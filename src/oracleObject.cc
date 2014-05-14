#include "global.h"

#include "oracleObject.h"

///////////////////////////////////////////////////////////////////////////
#define REPORT_ERROR(oracleStatus, message) reportError((oracleStatus), (message), __FILE__, __LINE__);

///////////////////////////////////////////////////////////////////////////
OracleObject::OracleObject(const Config& config)
	:	itsConfig(config)
	,	connection(0)
{
	if (itsConfig.isDebug)
	{
		std::cout << "OracleObject::OracleObject" << std::endl;
	}

	connection = new ocip::Connection(OCI_THREADED);
	assert(connection);
}

///////////////////////////////////////////////////////////////////////////
OracleObject::~OracleObject()
{
	if (itsConfig.isDebug)
	{
		std::cout << "OracleObject::~OracleObject" << std::endl;
	}

	if (connection)
	{
		delete connection;
		connection = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::connect()
{
	if (itsConfig.isDebug)
	{
		std::cout << "OracleObject::connect" << std::endl << std::flush;
	}

	if (!connection->connect(itsConfig.itsUsername, itsConfig.itsPassword, getConnectString(), false))
	{
		REPORT_ERROR(connection->status(), "connect");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::disconnect()
{
	if (itsConfig.isDebug)
	{
		std::cout << "OracleObject::disconnect" << std::endl;
	}

	if (!connection->disconnect())
	{
		REPORT_ERROR(connection->status(), "disconnect");
		return false;
	}

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
bool OracleObject::requestInit(const propertyListType& cgi)
{
	propertyListConstIteratorType it;
	int i = 0;

	if (itsConfig.isDebug)
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
	ocip::Statement statement(*connection);
	if (!statement.prepare("BEGIN owa.init_cgi_env(:c, :n, :v); htp.init; htp.htbuf_len := 63; END;"))
	{
		REPORT_ERROR(statement.status(), "oci_statement_prepare");
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
		REPORT_ERROR(statement.status(), "oci_statement_execute");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestRun(const std::string& procedure, const propertyListType& parameters)
{
	if (itsConfig.isDebug)
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
	ocip::Statement statement(*connection);
	if (!statement.prepare(sql))
	{
		REPORT_ERROR(statement.status(), "oci_statement_prepare");
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
		REPORT_ERROR(statement.status(), "oci_statement_execute");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestPage(std::wstring* page)
{
	if (itsConfig.isDebug)
	{
		std::cout << "OracleObject::requestPage - BEGIN" << std::flush << std::endl;
	}

	sword status = 0;
	OCILobLocator* locp = 0;
	OCIBind* bindp = 0;

	ocip::Statement statement(*connection);

	// Allocate lob descriptor
	status = oci_lob_descriptor_allocate(connection->hEnv(), &locp);
	if (status != OCI_SUCCESS)
	{
		REPORT_ERROR(status, "oci_lob_descriptor_allocate");
		return false;
	}

	// Prepare statement
	if (!statement.prepare("BEGIN plsql_server_util.get_page(:page); END;"))
	if (status != OCI_SUCCESS)
	{
		REPORT_ERROR(statement.status(), "oci_statement_prepare");
		return false;
	}

	// Bind CLOB descriptor
	if (!statement.bind(&bindp, "page", SQLT_CLOB, &locp, sizeof(OCILobLocator*)))
	{
		REPORT_ERROR(statement.status(), "oci_bind_by_name");
		return false;
	}

	// Execute statement
	if (!statement.execute(1))
	{
		REPORT_ERROR(statement.status(), "oci_statement_execute");
		return false;
	}

	// Open CLOB
	status = oci_open_lob(connection->hSvcCtx(), connection->hError(), locp);
	if (status != OCI_SUCCESS)
	{
		REPORT_ERROR(status, "oci_open_lob");
		return false;
	}

	// Get length of CLOB
	long lob_length = 0;
	status = oci_lob_gen_length(connection->hSvcCtx(), connection->hError(), locp, &lob_length);
	if (status != OCI_SUCCESS)
	{
		REPORT_ERROR(status, "oci_lob_gen_length");
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
		REPORT_ERROR(status, "oci_clob_read");
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
		REPORT_ERROR(status, "oci_close_lob");
		return false;
	}

	// Free lob descriptor
	status = oci_lob_descriptor_free(locp);
	if (status != OCI_SUCCESS)
	{
		REPORT_ERROR(status, "oci_lob_descriptor_free");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
void OracleObject::reportError(int oracleStatus, const std::string& message, const std::string& file, int line)
{
	if (itsConfig.isDebug)
	{
		std::cerr << "ORACLE ERROR in " << file << "(" << line << "): " << message << std::endl << "Oracle status code: " << oracleStatus << std::flush << std::endl;
	}

	std::string	oracleErrorMessage;
	int oracleErrorCode = 0;

	// Try to retrive the oracle error code and message
	if (connection->hError())
	{
		/*sword status =*/ oci_error_get(connection->hError(), &oracleErrorMessage, &oracleErrorCode);

		if (itsConfig.isDebug)
		{
			std::cerr << "Oracle error code: " << oracleErrorMessage << std::endl << "Oracle error message: " << oracleErrorCode << std::flush << std::endl;
		}
	}

	// Set the error status
	itsOracleError = oracleError(message, oracleStatus, oracleErrorCode, oracleErrorMessage, file, line);
}

///////////////////////////////////////////////////////////////////////////
std::string OracleObject::getConnectString() const
{
	std::ostringstream connectionString;
	connectionString << itsConfig.itsHostname << ":" << itsConfig.itsPort << "/" << itsConfig.itsDatabase;
	return connectionString.str();
}
