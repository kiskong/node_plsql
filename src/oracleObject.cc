#include "global.h"

#include "oracleObject.h"

///////////////////////////////////////////////////////////////////////////
OracleObject::OracleObject(const Config& config)
	:	m_Config(config)
	,	m_environment(0)
	,	m_connectionPool(0)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::OracleObject" << std::endl << config.asString() << std::endl << std::flush;
	}
}

///////////////////////////////////////////////////////////////////////////
OracleObject::~OracleObject()
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::~OracleObject" << std::endl << std::flush;
	}

	destroy();
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::create()
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::create" << std::endl << std::flush;
	}

	// Create the Oracle enviroment
	m_environment = new ocip::Environment(OCI_THREADED);
	assert(m_environment);

	// Are we using a connection pool
	if (m_Config.m_conPool)
	{
		// Create the connection pool
		m_connectionPool = new ocip::ConnectionPool(m_environment);
		assert(m_connectionPool);

		if (m_Config.m_debug)
		{
			std::cout << "OracleObject::create: create connection pool. user=(" << m_Config.m_username << ") password=(" << m_Config.m_password << ") database=(" << m_Config.m_database << ")" << std::endl << std::flush;
		}

		// Create the connection pool
		if (!m_connectionPool->create(m_Config.m_username, m_Config.m_password, m_Config.m_database, m_Config.m_conMin, m_Config.m_conMax, m_Config.m_conIncr))
		{
			m_OracleError = m_connectionPool->reportError("create connection pool", __FILE__, __LINE__, m_Config.m_debug);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::destroy()
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::destroy" << std::endl << std::flush;
	}

	if (m_connectionPool)
	{
		// Destroy the connection pool
		m_connectionPool->destroy();
		delete m_connectionPool;
		m_connectionPool = 0;
	}

	// Destroy the connection object
	if (m_environment)
	{
		delete m_environment;
		m_environment = 0;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::execute(const std::string& username, const std::string& password, const std::string& sql)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::execute(" << sql << ") - BEGIN" << std::endl << std::flush;
	}

	// Create a new connection
	ocip::Connection* connection = createConnection();

	// Connect with database
	if (!connection->connect(username, password))
	{
		std::ostringstream s;
		s << "error when trying to connect. username: \"" << username << "\" password: \"" << password << "\"";
		m_OracleError = connection->reportError(s.str(), __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Prepare statement
	ocip::Statement statement(connection);
	if (!statement.prepare(sql))
	{
		m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Execute statement
	if (!statement.execute(1))
	{
		m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Disconnect from the connection pool
	if (!connection->disconnect())
	{
		m_OracleError = connection->reportError("disconnect from the connection pool", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	delete connection;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::request(const std::string& username, const std::string& password, const propertyListType& cgi, const std::string& procedure, const propertyListType& parameters, std::wstring* page)
{
	// Create a new connection
	ocip::Connection* connection = createConnection();

	// Connect with database
	if (!connection->connect(username, password))
	{
		m_OracleError = connection->reportError("connect", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// 1. Initialize the request
	if (!requestInit(connection, cgi))
	{
		return false;
	}

	// 2. Invoke the procedure
	if (!requestRun(connection, procedure, parameters))
	{
		return false;
	}

	// 3. Retrieve the page content
	if (!requestPage(connection, page))
	{
		return false;
	}

	// Disconnect from the connection pool
	if (!connection->disconnect())
	{
		connection->reportError("disconnect from the connection pool", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	delete connection;

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
// The gateway is no longer:
// * setting up the owa.ip_address record based on the client IP address (does not work with IPv6 addresses anyway)
// * setting up the hostname, user id and password for basic authentication (the Apex Listener does not do this either)
// * calling owa.initialize() before owa.init_cgi_env() (the Apex Listener does not do this either)
//
// htbuf_len: reduce this limit based on your worst-case character size.
// For most character sets, this will be 2 bytes per character, so the limit would be 127.
// For UTF8 Unicode, it's 3 bytes per character, meaning the limit should be 85.
// For the newer AL32UTF8 Unicode, it's 4 bytes per character, and the limit should be 63.
//
bool OracleObject::requestInit(ocip::Connection* connection, const propertyListType& cgi)
{
	propertyListConstIteratorType it;
	int i = 0;

	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestInit - BEGIN" << std::endl;
		for (it = cgi.begin(), i = 0; it != cgi.end(); ++it, ++i)
		{
			std::cout << "   " << i << ". '" << it->name << "': '" << it->value << "'" << std::endl;
		}
		std::cout << std::flush;
	}

	assert(cgi.size() > 0);

	// Convert the list of properties into two separate lists with names and values
	stringListType names;
	stringListType values;
	convert(cgi, &names, &values);

	// Prepare statement
	ocip::Statement statement(connection);
	if (!statement.prepare("BEGIN owa.init_cgi_env(:c, :n, :v); htp.init; htp.htbuf_len := 63; END;"))
	{
		m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Bind the number of cgi entries
	ocip::ParameterValue* bCount = new ocip::ParameterValue("c", ocip::Integer, ocip::Input);
	statement.addParameter(bCount);
	bCount->value(static_cast<long>(cgi.size()));

	// Bind array of CGI names
	ocip::ParameterArray* bNames = new ocip::ParameterArray("n", ocip::String, ocip::Input);
	statement.addParameter(bNames);
	bNames->value(names);

	// Bind array of CGI values
	ocip::ParameterArray* bValues = new ocip::ParameterArray("v", ocip::String, ocip::Input);
	statement.addParameter(bValues);
	bValues->value(values);

	// Execute statement
	if (!statement.execute(1))
	{
		m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestRun(ocip::Connection* connection, const std::string& procedure, const propertyListType& parameters)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestRun(" << procedure << ") - BEGIN" << std::flush << std::endl;
	}

	// Build the proper sql command
	std::string sql;
	if (procedure[0] == '!')
	{
		sql = "BEGIN " + procedure.substr(1) + "(name_array=>:n, value_array=>:v); END;";
	}
	else
	{
		propertyListConstIteratorType it;
		sql = "BEGIN " + procedure + "(";
		for (it = parameters.begin(); it != parameters.end(); ++it)
		{
			if (it != parameters.begin())
			{
				sql += ",";
			}
			sql += it->name + "=>\'" + it->value + "\'";
		}
		sql += "); END;";
	}

	// Prepare statement
	ocip::Statement statement(connection);
	if (!statement.prepare(sql))
	{
		m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Bind values
	if (procedure[0] == '!')
	{
		// Convert the list of properties into two separate lists with names and values
		stringListType names;
		stringListType values;
		convert(parameters, &names, &values);

		// Bind array of CGI names
		ocip::ParameterArray* bNames = new ocip::ParameterArray("n", ocip::String, ocip::Input);
		statement.addParameter(bNames);
		bNames->value(names);

		// Bind array of CGI values
		ocip::ParameterArray* bValues = new ocip::ParameterArray("v", ocip::String, ocip::Input);
		statement.addParameter(bValues);
		bValues->value(values);
	}

	// Execute statement
	if (!statement.execute(1))
	{
		m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestPage(ocip::Connection* connection, std::wstring* page)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestPage - BEGIN" << std::flush << std::endl;
	}

	sword status = 0;
	OCILobLocator* locp = 0;
	OCIBind* bindp = 0;

	ocip::Statement statement(connection);

	// Allocate lob descriptor
	status = oci_lob_descriptor_allocate(connection->hEnv(), &locp);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, 0, "oci_lob_descriptor_allocate", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Prepare statement
	if (!statement.prepare("BEGIN node_plsql.get_page(:page); END;"))
	{
		m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Bind CLOB descriptor
	if (!statement.bind(&bindp, "page", SQLT_CLOB, &locp, sizeof(OCILobLocator*)))
	{
		m_OracleError = statement.reportError("oci_bind_by_name", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Execute statement
	if (!statement.execute(1))
	{
		m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Open CLOB
	status = oci_open_lob(connection->hSvcCtx(), connection->hError(), locp);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_open_lob", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Get length of CLOB
	long lob_length = 0;
	status = oci_lob_gen_length(connection->hSvcCtx(), connection->hError(), locp, &lob_length);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_lob_gen_length", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Allocate buffer for CLOB
	wchar_t* lob_buffer = reinterpret_cast<wchar_t*>(malloc((lob_length + 1) * sizeof(wchar_t)));

	// Read the CLOB
	ub4 amt		= lob_length * sizeof(wchar_t);
	ub4 buflen	= lob_length * sizeof(wchar_t);
	status = oci_clob_read(connection->hSvcCtx(), connection->hError(), locp, &amt, 1, reinterpret_cast<void*>(lob_buffer), buflen, OCI_UTF16ID);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_clob_read", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Terminate buffer with 0
	lob_buffer[lob_length] = 0;

	// Convert into page
	*page = std::wstring(lob_buffer);

	// Free buffer for CLOB
	free(lob_buffer);

	// Close CLOB
	status = oci_close_lob(connection->hSvcCtx(), connection->hError(), locp);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_close_lob", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	// Free lob descriptor
	status = oci_lob_descriptor_free(locp);
	if (status != OCI_SUCCESS)
	{
		m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_lob_descriptor_free", __FILE__, __LINE__, m_Config.m_debug);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
ocip::Connection* OracleObject::createConnection()
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::createConnection" << std::endl << std::flush;
	}

	// Are we using a connection pool ?
	ocip::Connection* connection = 0;
	if (m_Config.m_conPool)
	{
		connection = new ocip::Connection(m_connectionPool);
	}
	else
	{
		connection = new ocip::Connection(m_environment);
	}
	assert(connection);

	return connection;
}
