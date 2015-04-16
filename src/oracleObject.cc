#include "global.h"

#include "oracleObject.h"

///////////////////////////////////////////////////////////////////////////
static std::string getSql(const std::string& procedure, const parameterListType& parameters, bool isVariable);
static void bind(ocip::Statement& statement, bool isVariable, const parameterListType& parameters);
static parameterListType enhanceParameters(const parameterListType& parameters, procType& procData);
static std::string getParameterName(long position);
static bool loadFileContent(const std::string filename, std::vector<unsigned char>& fileContents);

///////////////////////////////////////////////////////////////////////////
static char* SQL_GET_PAGE =
"DECLARE\n"
"	MAX_LINES_TO_FETCH	CONSTANT	INTEGER			:=	2147483647;\n"
"	TRASHHOLD			CONSTANT	BINARY_INTEGER	:=	32767;\n"
"	iRows							INTEGER			:=	MAX_LINES_TO_FETCH;\n"
"	pageBuf							htp.htbuf_arr;\n"
"	textSize						BINARY_INTEGER;\n"
"	i								BINARY_INTEGER;\n"
"	pageLOB							CLOB;\n"
"	pageTXT							VARCHAR2(32767);\n"
"BEGIN\n"
"	owa.get_page(thepage=>pageBuf, irows=>iRows);\n"
"	IF (NOT iRows < MAX_LINES_TO_FETCH) THEN\n"
"		RAISE VALUE_ERROR;\n"
"	END IF;\n"
"	dbms_lob.createtemporary(lob_loc=>pageLOB, cache=>TRUE, dur=>dbms_lob.session);\n"
"	FOR i IN 1 .. iRows LOOP\n"
"		textSize := LENGTH(pageBuf(i));\n"
"		IF (textSize > 0) THEN\n"
"			IF (LENGTH(pageTXT) + textSize < TRASHHOLD) THEN\n"
"				pageTXT := pageTXT || pageBuf(i);\n"
"			ELSE\n"
"				IF (pageTXT IS NOT NULL) THEN\n"
"					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);\n"
"					pageTXT := NULL;\n"
"				END IF;\n"
"				IF (textSize < TRASHHOLD) THEN\n"
"					pageTXT := pageTXT || pageBuf(i);\n"
"				ELSE\n"
"					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>textSize, buffer=>pageBuf(i));\n"
"				END IF;\n"
"			END IF;\n"
"		END IF;\n"
"	END LOOP;\n"
"	IF (pageTXT IS NOT NULL) THEN\n"
"		dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);\n"
"	END IF;\n"
"	:page := pageLOB;\n"
"	dbms_lob.freetemporary(lob_loc=>pageLOB);\n"
"END;";

static char* SQL_FIND_OBJECT =
"DECLARE\n"
"	TYPE vc_arr IS TABLE OF VARCHAR2(2000) INDEX BY BINARY_INTEGER;\n"
"\n"
"	argumentArray vc_arr;\n"
"	dataTypeArray vc_arr;\n"
"	schemaName    VARCHAR2(32767);\n"
"	part1         VARCHAR2(32767);\n"
"	part2         VARCHAR2(32767);\n"
"	dblink        VARCHAR2(32767);\n"
"	objectType    NUMBER;\n"
"	objectID      NUMBER;\n"
"BEGIN\n"
"	dbms_utility.name_resolve(name=>:p_name, context=>1, schema=>schemaName, part1=>part1, part2=>part2, dblink=>dblink, part1_type=>objectType, object_number=>objectID);\n"
"\n"
"	IF (part1 IS NOT NULL) THEN\n"
"		SELECT argument_name, data_type BULK COLLECT INTO argumentArray, dataTypeArray FROM all_arguments WHERE owner = schemaName AND package_name = part1 AND object_name = part2 AND argument_name IS NOT NULL ORDER BY overload, sequence;\n"
"	ELSE\n"
"		SELECT argument_name, data_type BULK COLLECT INTO argumentArray, dataTypeArray FROM all_arguments WHERE owner = schemaName AND package_name IS NULL AND object_name = part2 AND argument_name IS NOT NULL ORDER BY overload, sequence;\n"
"	END IF;\n"
"\n"
"	:p_owner      := schemaName;\n"
"	:p_objectType := objectType;\n"
"	:p_part1      := part1;\n"
"	:p_part2      := part2;\n"
"	:p_arguments  := argumentArray;\n"
"	:p_dataTypes  := dataTypeArray;\n"
"\n"
"END;";

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
	m_environment = new ocip::Environment(OCI_THREADED, m_Config.m_debug);
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
			m_OracleError = m_connectionPool->reportError("create connection pool", __FILE__, __LINE__);
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
		std::cout << "OracleObject::execute" << std::endl << std::flush;
	}

	// Create a new connection
	ocip::Connection* connection = createConnection();

	// Connect with database
	if (!connection->connect(username, password))
	{
		std::ostringstream s;
		s << "error when trying to connect. username: \"" << username << "\" password: \"" << password << "\"";
		m_OracleError = connection->reportError(s.str(), __FILE__, __LINE__);
		return false;
	}

	// Prepare statement
	ocip::Statement statement(connection);
	if (!statement.prepare(sql))
	{
		m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
		return false;
	}

	// Execute statement
	if (!statement.execute(1))
	{
		m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
		return false;
	}

	// Disconnect from the connection pool
	if (!connection->disconnect())
	{
		m_OracleError = connection->reportError("disconnect from the connection pool", __FILE__, __LINE__);
		return false;
	}

	delete connection;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::request(const std::string& username, const std::string& password, const propertyListType& cgi, const fileListType& files, const std::string& doctablename, const std::string& procedure, const parameterListType& parameters, std::wstring* page)
{
	// 1. Connect with database
	ocip::Connection* connection = createConnection();
	if (!connection->connect(username, password))
	{
		m_OracleError = connection->reportError("connect", __FILE__, __LINE__);
		return false;
	}

	// 2. Initialize the request
	if (!requestInit(connection, cgi))
	{
		return false;
	}

	// 3. Upload files
	if (!requestUploadFiles(connection, files, doctablename))
	{
		return false;
	}

	// 4. Invoke the procedure
	if (!requestRun(connection, procedure, parameters))
	{
		return false;
	}

	// 5. Retrieve the page content
	if (!requestPage(connection, page))
	{
		return false;
	}

	// 6. Commit
	connection->commit();

	// 7. Disconnect from the connection pool
	if (!connection->disconnect())
	{
		connection->reportError("disconnect from the connection pool", __FILE__, __LINE__);
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
		std::cout << "OracleObject::requestInit: START" << std::endl;
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

	try {

		// Prepare statement
		ocip::Statement statement(connection);
		if (!statement.prepare("BEGIN owa.init_cgi_env(:c, :n, :v); htp.init; htp.htbuf_len := 63; END;"))
		{
			m_OracleError = statement.reportError("Statement::execute", __FILE__, __LINE__);
			return false;
		}

		// Bind the number of cgi entries
		ocip::ParameterValue* bCount = new ocip::ParameterValue("c", ocip::Integer, ocip::Input);
		statement.addParameter(bCount);
		bCount->value(static_cast<long>(cgi.size()));

		// Bind array of CGI names
		ocip::ParameterArray* bNames = new ocip::ParameterArray("n", ocip::String, ocip::Input, names.size());
		statement.addParameter(bNames);
		bNames->value(names);

		// Bind array of CGI values
		ocip::ParameterArray* bValues = new ocip::ParameterArray("v", ocip::String, ocip::Input, values.size());
		statement.addParameter(bValues);
		bValues->value(values);

		// Execute statement
		if (!statement.execute(1))
		{
			m_OracleError = statement.reportError("Statement::execute", __FILE__, __LINE__);
			return false;
		}

	} catch (const std::exception& e) {
		m_OracleError = oracleError(e.what(), __FILE__, __LINE__);
		return false;
	}

	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestInit: END" << std::endl << std::flush;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestUploadFiles(ocip::Connection* connection, const fileListType& files, const std::string& doctablename)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestUploadFiles (" << files.size() << " files)" << std::endl << std::flush;
	}

	// Process the files
	fileListConstIteratorType it;
	for (it = files.begin(); it != files.end(); ++it)
	{
		if (!uploadFile(connection, *it, doctablename))
		{
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::uploadFile(ocip::Connection* connection, const fileType& file, const std::string& doctablename)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::uploadFile: START (" << file.m_path << ")" << std::endl << std::flush;
	}

	sword status = 0;
	OCILobLocator* locp = 0;
	OCIBind* bindp = 0;

	try {

		ocip::Statement statement(connection);

		// Allocate lob descriptor
		status = oci_lob_descriptor_allocate(connection->hEnv(), &locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, 0, "oci_lob_descriptor_allocate", __FILE__, __LINE__);
			return false;
		}

		// Create temporary BLOB
		status = oci_create_temporary_blob(connection->hSvcCtx(), connection->hError(), locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, 0, "oci_create_temporary_blob", __FILE__, __LINE__);
			return false;
		}

		// Prepare statement
		std::string sql = "INSERT INTO " + doctablename + " (name, mime_type, doc_size, dad_charset, last_updated, content_type, blob_content) VALUES (:1, :2, :3, 'ascii', SYSDATE, 'BLOB', :4)";
		if (m_Config.m_debug)
		{
			std::cout << "OracleObject::uploadFile: insert blob. sql=\"" << sql << "\"." << std::endl << std::flush;
		}
		if (!statement.prepare(sql))
		{
			m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
			return false;
		}

		// Bind parameter
		ocip::ParameterValue* bFilename	= new ocip::ParameterValue("1", ocip::String, ocip::Input);
		statement.addParameter(bFilename);

		ocip::ParameterValue* bMimetype	= new ocip::ParameterValue("2", ocip::String, ocip::Input);
		statement.addParameter(bMimetype);

		ocip::ParameterValue* bSize	= new ocip::ParameterValue("3", ocip::Integer, ocip::Input);
		statement.addParameter(bSize);

		if (!statement.bind(&bindp, "4", SQLT_BLOB, &locp, sizeof(OCILobLocator*)))
		{
			m_OracleError = statement.reportError("oci_bind_by_name", __FILE__, __LINE__);
			return false;
		}

		// Load file
		std::vector<unsigned char> fileContents;
		if (!loadFileContent(file.m_path, fileContents))
		{
			std::string error = "Unable to load file \"" + file.m_path + "\"";
			if (m_Config.m_debug)
			{
				std::cout << "OracleObject::uploadFile: Unable to load file \"" << file.m_path << "\"!" << std::endl << std::flush;
			}
			m_OracleError = oracleError(error, 0, 0, "", __FILE__, __LINE__);
			return false;
		}

		if (m_Config.m_debug)
		{
			std::cout << "OracleObject::uploadFile: file loaded. name= \"" << file.m_fieldname << "\" path= \"" << file.m_path << "\" size=\"" << fileContents.size() << "\"." << std::endl << std::flush;
		}

		bFilename->value(file.m_fieldname);
		bMimetype->value(file.m_mimetype);
		bSize->value(static_cast<long>(fileContents.size()));

		// write BLOB
		if (!statement.writeBLOB(locp, fileContents))
		{
			m_OracleError = statement.reportError("write BLOB content", __FILE__, __LINE__);
			return false;
		}

		// Execute statement
		if (!statement.execute(1))
		{
			m_OracleError = statement.reportError("insert BLOB content\nsql: " + sql, __FILE__, __LINE__);
			return false;
		}

		// Free temporary BLOB
		/*
		status = oci_free_temporary_lob(connection->hSvcCtx(), connection->hError(), locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, 0, "oci_free_temporary_lob", __FILE__, __LINE__);
			return false;
		}
		*/

		// Free lob descriptor
		status = oci_lob_descriptor_free(locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_lob_descriptor_free", __FILE__, __LINE__);
			return false;
		}

	} catch (const std::exception& e) {
		m_OracleError = oracleError(e.what(), __FILE__, __LINE__);
		return false;
	}

	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::uploadFile: END" << std::endl << std::flush;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestRun(ocip::Connection* connection, const std::string& procedure, const parameterListType& parameters)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestRun: START" << std::endl <<
			"   procedure: " << procedure << std::endl <<
			"   parameter: " << std::endl << ::to_string(parameters) << std::endl << std::flush;
	}

	// Is the URL prefixed with a "!"-sign to indicate the use of dynamic parameters
	std::string name(procedure);
	bool isVariable = false;
	if (name[0] == '!')
	{
		name = procedure.substr(1);
		isVariable = true;
	}

	// Resolve the object to invoke
	procType procData;
	if (!findObjectToInvoke(connection, name, &procData))
	{
		return false;
	}
	if (m_Config.m_debug)
	{
		std::cout << "findObjectToInvoke:" << std::endl <<
			"m_owner: " << procData.m_owner << std::endl <<
			"m_type: " << procData.m_type << std::endl <<
			"m_name: " << procData.m_name << std::endl <<
			"m_part1: " << procData.m_part1 << std::endl <<
			"m_part2: " << procData.m_part2 << std::endl <<
			"m_arguments: " << std::endl << ::to_string(procData.m_arguments) << std::endl <<
			"m_dataTypes: " << std::endl << ::to_string(procData.m_dataTypes) << std::endl << std::flush;
	}

	// Eventually change the list of parameters based on the data types in the procedure to execute
	parameterListType newParameters = enhanceParameters(parameters, procData);

	try {

		// Create statement
		ocip::Statement statement(connection);

		// Prepare statement
		std::string sql(getSql(name, newParameters, isVariable));
		if (!statement.prepare(sql))
		{
			m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
			return false;
		}

		// Bind values for statement
		bind(statement, (procedure[0] == '!'), newParameters);

		// Execute statement
		if (!statement.execute(1))
		{
			m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
			return false;
		}

	} catch (const std::exception& e) {
		m_OracleError = oracleError(e.what(), __FILE__, __LINE__);
		return false;
	}

	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestRun: END" << std::endl << std::flush;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool OracleObject::requestPage(ocip::Connection* connection, std::wstring* page)
{
	if (m_Config.m_debug)
	{
		std::cout << "OracleObject::requestPage" << std::endl << std::flush;
	}

	sword status = 0;
	OCILobLocator* locp = 0;
	OCIBind* bindp = 0;

	try {

		ocip::Statement statement(connection);

		// Allocate lob descriptor
		status = oci_lob_descriptor_allocate(connection->hEnv(), &locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, 0, "oci_lob_descriptor_allocate", __FILE__, __LINE__);
			return false;
		}

		// Prepare statement
		if (!statement.prepare(SQL_GET_PAGE))
		{
			m_OracleError = statement.reportError("oci_statement_prepare", __FILE__, __LINE__);
			return false;
		}

		// Bind CLOB descriptor
		if (!statement.bind(&bindp, "page", SQLT_CLOB, &locp, sizeof(OCILobLocator*)))
		{
			m_OracleError = statement.reportError("oci_bind_by_name", __FILE__, __LINE__);
			return false;
		}

		// Execute statement
		if (!statement.execute(1))
		{
			m_OracleError = statement.reportError("oci_statement_execute", __FILE__, __LINE__);
			return false;
		}

		// Open CLOB and read the contents
		if (!statement.openAndReadLOB(locp, page))
		{
			m_OracleError = statement.reportError("open and read CLOB content", __FILE__, __LINE__);
			return false;
		}

		// Free lob descriptor
		status = oci_lob_descriptor_free(locp);
		if (status != OCI_SUCCESS)
		{
			m_OracleError = ocip::Environment::reportError(status, connection->hError(), "oci_lob_descriptor_free", __FILE__, __LINE__);
			return false;
		}

	} catch (const std::exception& e) {
		m_OracleError = oracleError(e.what(), __FILE__, __LINE__);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
// resolve the name of the procedure to invoke
bool OracleObject::findObjectToInvoke(ocip::Connection* connection, const std::string& name, procType* procData)
{
	assert(procData);

	try {

		// Create statement
		ocip::Statement statement(connection);

		// Prepare statement
		if (!statement.prepare(SQL_FIND_OBJECT))
		{
			m_OracleError = statement.reportError("prepare", __FILE__, __LINE__);
			return false;
		}

		// input "name"
		ocip::ParameterValue* pName = new ocip::ParameterValue("p_name", ocip::String, ocip::Input);
		statement.addParameter(pName);
		pName->value(name);

		// output "p_owner"
		ocip::ParameterValue* pOwner = new ocip::ParameterValue("p_owner", ocip::String, ocip::Input);
		statement.addParameter(pOwner);

		// output "p_objectType"
		ocip::ParameterValue* pObjectType = new ocip::ParameterValue("p_objectType", ocip::Integer, ocip::Input);
		statement.addParameter(pObjectType);

		// output "p_part1"
		ocip::ParameterValue* pPart1 = new ocip::ParameterValue("p_part1", ocip::String, ocip::Input);
		statement.addParameter(pPart1);

		// output "p_part2"
		ocip::ParameterValue* pPart2 = new ocip::ParameterValue("p_part2", ocip::String, ocip::Input);
		statement.addParameter(pPart2);

		// output "arguments"
		ocip::ParameterArray* pArguments = new ocip::ParameterArray("p_arguments", ocip::String, ocip::Output, 1000, 60);
		statement.addParameter(pArguments);

		// output "dataTypes"
		ocip::ParameterArray* pDataTypes = new ocip::ParameterArray("p_dataTypes", ocip::String, ocip::Output, 1000, 60);
		statement.addParameter(pDataTypes);

		// Execute statement
		if (!statement.execute(1))
		{
			m_OracleError = statement.reportError("execute", __FILE__, __LINE__);
			return false;
		}

		// get the results
		procData->m_owner = pOwner->getString();
		procData->m_type = pObjectType->getInteger();
		procData->m_part1 = pPart1->getString();
		procData->m_part2 = pPart2->getString();
		procData->m_arguments = pArguments->getString();
		procData->m_dataTypes = pDataTypes->getString();
		assert(procData->m_arguments.size() == procData->m_dataTypes.size());

	} catch (const std::exception& e) {
		m_OracleError = oracleError(e.what(), __FILE__, __LINE__);
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

///////////////////////////////////////////////////////////////////////////
static std::string getSql(const std::string& procedure, const parameterListType& parameters, bool isVariable)
{
	std::string sql;

	if (isVariable)
	{
		sql = "BEGIN " + procedure + "(name_array=>:n, value_array=>:v); END;";
	}
	else
	{
		parameterListConstIteratorType it;
		long pos = 1;
		for (it = parameters.begin(); it != parameters.end(); ++it)
		{
			switch (it->type())
			{
				case parameterType::Scalar:
				case parameterType::Array:
					if (pos > 1)
					{
						sql += ",";
					}
					sql += it->name() + "=>:" + getParameterName(pos);
					++pos;
					break;
				case parameterType::Null:
				default:
					break;
			}
		}
		sql = "BEGIN " + procedure + "(" + sql + "); END;";
	}

	return sql;
}

///////////////////////////////////////////////////////////////////////////
static void pushParameter(const parameterType& parameter, stringListType* names, stringListType* values)
{
	stringListType strings;
	stringListConstIteratorType it;

	switch (parameter.type())
	{
		case parameterType::Scalar:
			names->push_back(parameter.name());
			values->push_back(parameter.value());
			break;
		case parameterType::Array:
			strings = parameter.values();
			for (it = strings.begin(); it != strings.end(); ++it)
			{
				names->push_back(parameter.name());
				values->push_back(*it);
			}
			break;
		case parameterType::Null:
		default:
			names->push_back(parameter.name());
			values->push_back("");
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
static parameterListType enhanceParameters(const parameterListType& parameters, procType& procData)
{
	assert(parameters.size() == procData.m_dataTypes.size());

	parameterListType newParameters;

	parameterListConstIteratorType itp;
	stringListConstIteratorType its;
	for (itp = parameters.begin(), its = procData.m_dataTypes.begin(); itp != parameters.end(); ++itp, ++its)
	{
		if (itp->type() == parameterType::Scalar && *its == "PL/SQL TABLE")
		{
			stringListType strings;
			strings.push_back(itp->value());
			newParameters.push_back(parameterType(itp->name(), strings));
		}
		else
		{
			newParameters.push_back(*itp);
		}
	}

	assert(newParameters.size() == parameters.size());

	return newParameters;
}

///////////////////////////////////////////////////////////////////////////
static void bind(ocip::Statement& statement, bool isVariable, const parameterListType& parameters)
{
	if (isVariable)
	{
		stringListType names;
		stringListType values;
		parameterListConstIteratorType it;
		for (it = parameters.begin(); it != parameters.end(); ++it)
		{
			pushParameter(*it, &names, &values);
		}
		assert(names.size() == values.size());

		// Bind array of parameter names
		ocip::ParameterArray* bNames = new ocip::ParameterArray("n", ocip::String, ocip::Input, names.size());
		assert(bNames);
		statement.addParameter(bNames);
		bNames->value(names);

		// Bind array of parameter values
		ocip::ParameterArray* bValues = new ocip::ParameterArray("v", ocip::String, ocip::Input, values.size());
		assert(bValues);
		statement.addParameter(bValues);
		bValues->value(values);
	}
	else
	{
		parameterListConstIteratorType it;
		long pos = 1;
		for (it = parameters.begin(); it != parameters.end(); ++it)
		{
			switch (it->type())
			{
				case parameterType::Scalar:
					{
					// std::cout << "OracleObject::bind: " << getParameterName(pos) << " is a scalar = " << it->to_string() << std::endl << std::flush;
					ocip::ParameterValue* bValue = new ocip::ParameterValue(getParameterName(pos), ocip::String, ocip::Input);
					assert(bValue);
					statement.addParameter(bValue);
					bValue->value(it->value());
					++pos;
					}
					break;
				case parameterType::Array:
					{
					// std::cout << "OracleObject::bind: " << getParameterName(pos) << " is an array = " << it->to_string() << std::endl << std::flush;
					ocip::ParameterArray* bValues = new ocip::ParameterArray(getParameterName(pos), ocip::String, ocip::Input, it->values().size());
					assert(bValues);
					statement.addParameter(bValues);
					bValues->value(it->values());
					++pos;
					}
					break;
				case parameterType::Null:
				default:
					break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
static std::string getParameterName(long position)
{
	std::ostringstream s;
	s << position;

	return "p" + s.str();
}

///////////////////////////////////////////////////////////////////////////
static bool loadFileContent(const std::string filename, std::vector<unsigned char>& fileContents)
{
	std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		return false;
	}

	// Get the file size
	file.seekg(0, file.end);
	int length = static_cast<int>(file.tellg());
	file.seekg(0, file.beg);

    fileContents.resize(length);

	if (!file.read(reinterpret_cast<char*>(&fileContents[0]), length))
	{
		return false;
	}

	return true;
}
