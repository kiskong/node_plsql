#include "global.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#include "ocip_interface.h"

namespace ocip
{

///////////////////////////////////////////////////////////////////////////
bool Environment::m_debug = false;

///////////////////////////////////////////////////////////////////////////
std::string getTypeName(DataType type)
{
	switch (type)
	{
		case SQLT_INT: return "SQLT_INT";
		case SQLT_STR: return "SQLT_STR";
	}

	return "**UNKNOWN**";
}

///////////////////////////////////////////////////////////////////////////
Environment::Environment(int mode /*= OCI_DEFAULT*/, bool debug /*= false*/)
	:	m_mode(static_cast<ub4>(mode))
	,	m_envhp(0)
{
	m_debug = debug;

	if (Environment::debug())
	{
		std::cout << "Environment::Environment" << std::flush << std::endl;
	}

	// Create the Oracle environment
	sword status = oci_connect_environment_create(&m_envhp, m_mode, OCI_UTF16ID, OCI_UTF16ID);
	assert(status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
Environment::~Environment()
{
	if (Environment::debug())
	{
		std::cout << "Environment::~Environment" << std::flush << std::endl;
	}

	sword status = oci_handle_free(OCI_HTYPE_ENV, reinterpret_cast<dvoid**>(&m_envhp));
	// assert(status == OCI_SUCCESS);
	m_envhp = 0;
}

///////////////////////////////////////////////////////////////////////////
oracleError Environment::reportError(int oracleStatus, OCIError* errhp, const std::string& message, const std::string& file, int line)
{
	if (Environment::debug())
	{
		std::cerr << "ORACLE ERROR in " << file << "(" << line << "): " << message << std::endl << "Oracle status code: " << oracleStatus << std::flush << std::endl;
	}

	std::string	oracleErrorMessage;
	int oracleErrorCode = 0;

	// Try to retrive the oracle error code and message
	if (errhp)
	{
		/*sword status =*/ oci_error_get(errhp, &oracleErrorMessage, &oracleErrorCode);

		if (Environment::debug())
		{
			std::cerr << "Oracle error code: " << oracleErrorMessage << std::endl << "Oracle error message: " << oracleErrorCode << std::flush << std::endl;
		}
	}

	// Set the error status
	return oracleError(message, oracleStatus, oracleErrorCode, oracleErrorMessage, file, line);
}

///////////////////////////////////////////////////////////////////////////
ConnectionPool::ConnectionPool(Environment* environment)
	:	m_envhp(environment->hEnv())
	,	m_errhp(0)
	,	m_poolhp(0)
	,	m_poolName(0)
	,	m_poolNameLen(0)
	,	m_hasPool(false)
	,	m_oracle_status(0)
{
	if (Environment::debug())
	{
		std::cout << "ConnectionPool::ConnectionPool" << std::flush << std::endl;
	}

	// Allocate an error handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate an connection pool handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_CPOOL, reinterpret_cast<dvoid**>(&m_poolhp));
	assert(m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
ConnectionPool::~ConnectionPool()
{
	if (Environment::debug())
	{
		std::cout << "ConnectionPool::~ConnectionPool" << std::flush << std::endl;
	}

	// destroy the connection pool
	destroy();

	// It does not seem to be needed to destroy the handles because most likely they are destroy when deallocating the connection pool itself
#if 0
	// Free a connection pool handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_CPOOL, reinterpret_cast<dvoid**>(&m_poolhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free a error handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);
#endif
}

///////////////////////////////////////////////////////////////////////////
bool ConnectionPool::create(const std::string& username, const std::string& password, const std::string& database, int connMin, int connMax, int connIncr)
{
	if (Environment::debug())
	{
		std::cout << "Connection::create(" << username << ", " << password << ", " << connMin << ", " << connMax << ", " << connIncr << ")" << std::flush << std::endl;
	}

	assert(!m_hasPool);

	m_oracle_status = oci_connect_pool_create(m_envhp, m_errhp, m_poolhp, &m_poolName, &m_poolNameLen, username, password, database, connMin, connMax, connIncr);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	m_hasPool = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool ConnectionPool::destroy()
{
	if (Environment::debug())
	{
		std::cout << "ConnectionPool::destroy" << std::flush << std::endl;
	}

	if (m_hasPool)
	{
		m_oracle_status = oci_connect_pool_destroy(m_poolhp, m_errhp);
		if (m_oracle_status != OCI_SUCCESS)
		{
			return false;
		}
	}

	m_hasPool = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
Connection::Connection(Environment* environment)
	:	m_envhp(environment->hEnv())
	,	m_errhp(0)
	,	m_poolhp(0)
	,	m_poolName(0)
	,	m_poolNameLen(0)
	,	m_hasPool(false)
	,	m_svchp(0)
	,	m_isConnected(false)
	,	m_oracle_status(OCI_SUCCESS)
{
	if (Environment::debug())
	{
		std::cout << "Connection::Connection" << std::flush << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////
Connection::Connection(ConnectionPool* connectionPool)
	:	m_envhp(connectionPool->hEnv())
	,	m_errhp(0)
	,	m_poolhp(connectionPool->poolhp())
	,	m_poolName(connectionPool->poolName())
	,	m_poolNameLen(connectionPool->poolNameLen())
	,	m_hasPool(false)
	,	m_svchp(0)
	,	m_isConnected(false)
	,	m_oracle_status(OCI_SUCCESS)
{
	if (Environment::debug())
	{
		std::cout << "Connection::Connection" << std::flush << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////
Connection::~Connection()
{
	if (Environment::debug())
	{
		std::cout << "Connection::~Connection" << std::flush << std::endl;
	}

	disconnect();
}

///////////////////////////////////////////////////////////////////////////
bool Connection::connect(const std::string& username, const std::string& password, const std::string& server)
{
	if (Environment::debug())
	{
		std::cout << "Connection::connect(" << username << ", " << password << ", " << server << ")" << std::flush << std::endl;
	}

	m_oracle_status = OCI_SUCCESS;

	// Allocate an error handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate a service handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid**>(&m_svchp));
	assert(m_oracle_status == OCI_SUCCESS);

	if (hasPool())
	{
		assert(server.size() == 0);
		// Logon to connection pool
		m_oracle_status = oci_logon(m_envhp, m_errhp, &m_svchp, username, password, m_poolName, m_poolNameLen);
	}
	else
	{
		// Logon to server
		m_oracle_status = oci_logon(m_envhp, m_errhp, &m_svchp, username, password, server);
	}

	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	m_isConnected = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool Connection::connect(const std::string& username, const std::string& password)
{
	return connect(username, password, "");
}

///////////////////////////////////////////////////////////////////////////
bool Connection::disconnect()
{
	if (Environment::debug())
	{
		std::cout << "Connection::disconnect" << std::flush << std::endl;
	}

	m_oracle_status = OCI_SUCCESS;

	if (!isConnected())
	{
		return true;
	}

	// Logoff
	m_oracle_status = oci_logoff(m_svchp, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	// Free a error handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	//assert(m_oracle_status == OCI_SUCCESS);

	// Free a service handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid**>(&m_svchp));
	//assert(m_oracle_status == OCI_SUCCESS);

	m_isConnected = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool Connection::commit()
{
	if (Environment::debug())
	{
		std::cout << "Connection::commit" << std::flush << std::endl;
	}

	assert(isConnected());
	assert(m_svchp);
	assert(m_errhp);

	m_oracle_status = oci_commit(m_svchp, m_errhp);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
Statement::Statement(Connection* connection)
	:	m_connection(connection)
	,	m_stmtp(0)
{
	if (Environment::debug())
	{
		std::cout << "Statement::Statement" << std::flush << std::endl;
	}

	assert(connection->isConnected());

	m_oracle_status = oci_statement_allocate(m_connection->hEnv(), &m_stmtp);
	assert(m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
Statement::~Statement()
{
	if (Environment::debug())
	{
		std::cout << "Statement::~Statement" << std::flush << std::endl;
	}

	ParameterListIteratorType it;
	for (it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		delete *it;
	}	

	m_oracle_status = oci_statement_free(m_stmtp);
	assert(m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::prepare(const std::string& sql)
{
	if (Environment::debug())
	{
		std::string s(sql);
		replace(s, "\n", "\\n");
		if (s.size() > 1000) {
			s = s.substr(0, 1000) + "...";
		}
		std::cout << "Statement::prepare: sql=\"" << s << "\"" << std::flush << std::endl;
	}

	m_sql = sql;

	m_oracle_status = oci_statement_prepare(m_stmtp, m_connection->hError(), sql);

	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::execute(int iterations)
{
	if (Environment::debug())
	{
		std::cout << "Statement::execute: START iterations=" << iterations << std::flush << std::endl;
	}

	// Actually bind the parameter
	ParameterListIteratorType it;
	for (it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		m_oracle_status = (*it)->bind(m_stmtp, m_connection->hError());
		if (m_oracle_status != OCI_SUCCESS)
		{
			return (m_oracle_status == OCI_SUCCESS); 
		}
	}

	// Execute the statement
	m_oracle_status = oci_statement_execute(m_stmtp, m_connection->hSvcCtx(), m_connection->hError(), static_cast<ub4>(iterations));

	if (Environment::debug())
	{
		std::cout << "Statement::execute: END" << std::flush << std::endl;
	}

	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::bind(OCIBind** bindpp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp, ub4 maxarr_len, ub4* curelen)
{
	if (Environment::debug())
	{
		std::cout << "Statement::bind: m_placeholder=\"" << placeholder << "\"" << std::flush << std::endl;
	}

	assert(bindpp);

	m_oracle_status = oci_bind_by_name(m_stmtp, bindpp, m_connection->hError(), placeholder, dty, valuep, value_sz, indp, maxarr_len, curelen);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
void Statement::addParameter(Parameter* parameter)
{
	assert(parameter);

	if (Environment::debug())
	{
		std::cout << "Statement::addParameterValue: m_placeholder=\"" << parameter->m_placeholder << "\"" << std::flush << std::endl;
	}

	parameter->m_statement = this;

	m_parameters.push_back(parameter);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::openAndReadLOB(OCILobLocator* locp, std::wstring* lob)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::openAndReadLOB" << std::flush << std::endl;
	}

	assert(locp);
	assert(lob);

	// Open CLOB
	m_oracle_status = oci_open_lob(m_connection->hSvcCtx(), m_connection->hError(), locp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		std::cout << "ERROR in oci_open_lob" << std::flush << std::endl;
		return false;
	}

	// Get length of CLOB
	long lob_length = 0;
	m_oracle_status = oci_lob_gen_length(m_connection->hSvcCtx(), m_connection->hError(), locp, &lob_length);
	if (m_oracle_status != OCI_SUCCESS)
	{
		std::cout << "ERROR in oci_lob_gen_length" << std::flush << std::endl;
		return false;
	}

	// Allocate buffer for CLOB
	oci_text lob_buffer(lob_length);

	// Read the CLOB
	ub4 amt		= lob_buffer.size();
	ub4 buflen	= amt;
	m_oracle_status = oci_clob_read(m_connection->hSvcCtx(), m_connection->hError(), locp, &amt, 1, reinterpret_cast<void*>(lob_buffer.text()), buflen, OCI_UTF16ID);
	if (m_oracle_status != OCI_SUCCESS)
	{
		std::cout << "ERROR in oci_clob_read: m_oracle_status=" << m_oracle_status << std::flush << std::endl;
		return false;
	}

	// Convert into page
	*lob = lob_buffer.getWString();

	// Close CLOB
	m_oracle_status = oci_close_lob(m_connection->hSvcCtx(), m_connection->hError(), locp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		std::cout << "ERROR in oci_close_lob" << std::flush << std::endl;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool Statement::writeBLOB(OCILobLocator* locp, const std::vector<unsigned char>& lob)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::writeBLOB (size=" << lob.size() << ")" << std::flush << std::endl;
	}

	assert(locp);

	// Write the BLOB
	ub4 amt		= static_cast<ub4>(lob.size());
	ub4 buflen	= amt;
	m_oracle_status = oci_blob_write(m_connection->hSvcCtx(), m_connection->hError(), locp, &amt, 1, const_cast<void*>(reinterpret_cast<const void*>(&lob[0])), buflen, OCI_ONE_PIECE);
	if (m_oracle_status != OCI_SUCCESS)
	{
		std::cout << "ERROR in oci_blob_write: m_oracle_status=" << m_oracle_status << std::flush << std::endl;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
oracleError Statement::reportError(const std::string& message, const std::string& file, int line)
{
	std::string text("sql: " + m_sql);
	if (message.size() > 0)
	{
		text += "\n" + message;
	}

	return Environment::reportError(m_oracle_status, m_connection->hError(), text, file, line);
}

///////////////////////////////////////////////////////////////////////////
Parameter::Parameter(const std::string& placeholder, DataType type, Direction direction, size_t maxValueLen)
	:	m_statement(0)
	,	m_placeholder(placeholder)
	,	m_type(type)
	,	m_direction(direction)
	,	m_value_sz(0)
	,	m_value_sz_in_bytes(0)
{
	switch (type)
	{
		case String:
			if (maxValueLen <= 0)
			{
				throw std::runtime_error("The maxValueLen must be > 0 when binding an string parameter");
			}
			else
			{
				m_value_sz = static_cast<sb4>(maxValueLen);
				m_value_sz_in_bytes = static_cast<sb4>(oci_text::sizeInBytes(m_value_sz));
			}
			break;
		case Integer:
			m_value_sz_in_bytes = m_value_sz = static_cast<sb4>(sizeof(long));
			break;
		default:
			throw std::runtime_error("The parameter type (String or Integer)");
			break;
	}

	switch (direction)
	{
		case Input:
		case Output:
			break;
		default:
			throw std::runtime_error("The direction must be Input or Output");
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
ParameterValue::ParameterValue(const std::string& placeholder, DataType type, Direction direction, size_t maxValueLen/* = 2000*/)
	:	Parameter(placeholder, type, direction, maxValueLen)
	,	m_value_integer(0)
	,	m_ind(0)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::ParameterValue: m_placeholder=\"" << m_placeholder << "\" type=\"" << getTypeName(type) << "\" maxValueLen=\"" << maxValueLen << "\"" << std::flush << std::endl;
	}

	switch (type)
	{
		case String:
			m_value_string.allocate(maxValueLen);
			break;
		case Integer:
			break;
		default:
			throw std::runtime_error("Invalid value for parameter type");
	}
}

///////////////////////////////////////////////////////////////////////////
ParameterValue::~ParameterValue()
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::~ParameterValue m_placeholder=\"" << m_placeholder << "\"" << std::flush << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////
long ParameterValue::getInteger() const
{
	if (m_type != Integer)
	{
		throw std::runtime_error("Illegal operation for a non Integer type");
	}

	return m_value_integer;
}

///////////////////////////////////////////////////////////////////////////
std::string ParameterValue::getString() const
{
	if (m_type != String)
	{
		throw std::runtime_error("Illegal operation for a non String type");
	}

	return m_value_string.getString();
}

///////////////////////////////////////////////////////////////////////////
void ParameterValue::value(long value)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::value(long) m_placeholder=\"" << m_placeholder << "\" value=" << value << std::flush << std::endl;
	}

	switch (m_type)
	{
		case Integer:
			m_value_integer = value;
			break;
		case String:
		default:
			throw std::runtime_error("Illegal operation for a non Integer type");
	}
}

///////////////////////////////////////////////////////////////////////////
void ParameterValue::value(const std::string& value)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::value(std::string) m_placeholder=\"" << m_placeholder << "\" value=\"" << value << "\"" << std::flush << std::endl;
	}

	switch (m_type)
	{
		case String:
			m_value_string.replace(value);
			break;
		case Integer:
		default:
			throw std::runtime_error("Illegal operation for a non String type");
	}
}

///////////////////////////////////////////////////////////////////////////
sword ParameterValue::bind(OCIStmt* stmtp, OCIError* errhp)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::bind: START" << std::endl << "m_placeholder=\"" << m_placeholder << "\" m_type=\"" << getTypeName(m_type) << "\" m_value_sz=" << m_value_sz << std::endl << std::flush;
	}

	if (!stmtp || !m_statement || !errhp || m_value_sz <= 0)
	{
		std::ostringstream s;
		s <<	"Internal error in ParameterValue::bind" << std::endl <<
				"stmtp: " << (void*)stmtp << std::endl <<
				"errhp: " << (void*)errhp << std::endl <<
				"m_statement: " << (void*)m_statement << std::endl <<
				"errhp: " << (void*)errhp << std::endl <<
				"m_value_sz: " << m_value_sz << std::endl <<
				std::flush;
		throw std::runtime_error(s.str());
		throw std::runtime_error("Internal error in ParameterValue::bind");
	}

	dvoid* valuep = 0;
	sb4 value_sz_in_bytes = 0;
	switch (m_type)
	{
		case String:
			valuep = reinterpret_cast<dvoid*>(m_value_string.text());
			value_sz_in_bytes = m_value_string.size();
			break;
		case Integer:
			valuep = reinterpret_cast<dvoid*>(&m_value_integer);
			value_sz_in_bytes = m_value_sz;
			break;
		default:
			std::cerr << "Illegal data type in ParameterValue::bind" << std::endl << std::flush;
			throw std::runtime_error("Illegal data type in ParameterValue::bind");
	}

	sword status = oci_bind_by_name(stmtp, &m_bindp, errhp, m_placeholder, static_cast<ub2>(m_type), valuep, value_sz_in_bytes, &m_ind);

	if (Environment::debug())
	{
		std::cout << "ParameterValue::bind: END" << std::endl << std::flush;
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////
ParameterArray::ParameterArray(const std::string& placeholder, DataType type, Direction direction, size_t maxArrayLen, size_t maxValueLen /*= 2000*/)
	:	Parameter(placeholder, type, direction, maxValueLen)
	,	m_valuep(0)
	,	m_indp(0)
	,	m_maxarr_len(static_cast<ub4>(maxArrayLen))
	,	m_curelen(0)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::ParameterArray: m_placeholder=\"" << m_placeholder << "\" type=\"" << getTypeName(type) << "\" maxArrayLen=\"" << maxArrayLen << "\" maxValueLen=\"" << maxValueLen << "\"" << std::flush << std::endl;
	}

	// calculate the size in bytes for each individual entry
	m_value_sz_in_bytes = valueSizeInByte(static_cast<ub4>(maxValueLen));

	// allocate buffers
	allocate(m_value_sz_in_bytes, static_cast<ub4>(maxArrayLen));

	// initialize buffers
	initialize(m_value_sz_in_bytes, static_cast<ub4>(maxArrayLen));
}

///////////////////////////////////////////////////////////////////////////
ParameterArray::~ParameterArray()
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::~ParameterArray m_placeholder=\"" << m_placeholder << "\"" << std::flush << std::endl;
	}

	if (m_valuep)
	{
		free(m_valuep);
		m_valuep = 0;	
	}

	if (m_indp)
	{
		free(m_indp);
		m_indp = 0;	
	}
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::value(std::list<long> list)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::value(std::list<long>) m_placeholder=\"" << m_placeholder << "\" list.size()=" << list.size() << std::flush << std::endl;
	}

	if (m_type != Integer)
	{
		throw std::runtime_error("Illegal operation for a non Integer type");
	}

	const size_t LIST_SIZE = list.size();
	if (LIST_SIZE == 0 || LIST_SIZE > m_maxarr_len)
	{
		throw std::runtime_error("The list is not allowed to be empty or greater that the maximal number of array entries");
	}

	// initialize buffers
	initialize(m_value_sz_in_bytes, m_maxarr_len);

	// copy the values
	std::list<long>::iterator it;
	long* p = reinterpret_cast<long*>(m_valuep);
	for (it = list.begin(); it != list.end(); ++it)
	{
		*p++ = *it;
	}

	// Set the current array size
	m_curelen = static_cast<sb4>(LIST_SIZE);
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::value(std::list<std::string> list)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::value(std::list<string>) m_placeholder=\"" << m_placeholder << "\" list.size()=" << list.size() << std::flush << std::endl;
	}

	if (m_type != String)
	{
		throw std::runtime_error("Illegal operation for a non Integer type");
	}

	const size_t LIST_SIZE = list.size();
	if (LIST_SIZE == 0 || LIST_SIZE > m_maxarr_len)
	{
		throw std::runtime_error("The list is not allowed to be empty or greater that the maximal number of array entries");
	}

	// initialize buffers
	initialize(m_value_sz_in_bytes, m_maxarr_len);

	// copy the values
	std::list<std::string>::iterator it;
	unsigned char* p = reinterpret_cast<unsigned char*>(m_valuep);
	for (it = list.begin(); it != list.end(); ++it)
	{
		std::string s(*it);
		if (s.size() > m_value_sz)
		{
			s.erase(m_value_sz);
		}

		//std::cout << "Move \"" << s << "\" to " << reinterpret_cast<const void*>(p) << " " << reinterpret_cast<long>(p) << std::flush << std::endl;

		oci_text::convert(s, reinterpret_cast<oci_text::utf16_char_t*>(p), m_value_sz_in_bytes);
		
		p += m_value_sz_in_bytes;
	}

	// Set the current array size
	m_curelen = static_cast<sb4>(LIST_SIZE);
}

///////////////////////////////////////////////////////////////////////////
std::list<long> ParameterArray::getInteger() const
{
	std::list<long> integerArray;

	// copy the values
	long* p = reinterpret_cast<long*>(m_valuep);
	int i;
	for (i = 0; i < static_cast<int>(m_curelen); i++)
	{
		integerArray.push_back(*p++);
	}

	return integerArray;
}

///////////////////////////////////////////////////////////////////////////
std::list<std::string> ParameterArray::getString() const
{
	std::list<std::string> stringArray;

	// copy the values
	unsigned char* p = reinterpret_cast<unsigned char*>(m_valuep);
	int i;
	for (i = 0; i < static_cast<int>(m_curelen); i++)
	{
		stringArray.push_back(oci_text::convert_utf16_to_string(reinterpret_cast<const oci_text::utf16_char_t*>(p)));
		p += m_value_sz_in_bytes;
	}

	return stringArray;
}

///////////////////////////////////////////////////////////////////////////
sword ParameterArray::bind(OCIStmt* stmtp, OCIError* errhp)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::bind: START" << std::endl << "m_placeholder=\"" << m_placeholder << "\" m_type=\"" << getTypeName(m_type) << "\" m_value_sz=" << m_value_sz << std::endl << std::flush;
	}

	if (!stmtp || !errhp || !m_statement || !m_valuep || m_value_sz <= 0 || !m_indp || m_maxarr_len <= 0 || m_curelen > m_maxarr_len)
	{
		std::ostringstream s;
		s <<	"Internal error in ParameterArray::bind" << std::endl <<
				"stmtp: " << (void*)stmtp << std::endl <<
				"errhp: " << (void*)errhp << std::endl <<
				"m_statement: " << (void*)m_statement << std::endl <<
				"m_valuep: " << (void*)m_valuep << std::endl <<
				"m_value_sz: " << m_value_sz << std::endl <<
				"m_indp: " << (void*)m_indp << std::endl <<
				"m_maxarr_len: " << m_maxarr_len << std::endl <<
				"m_curelen: " << m_curelen << std::endl <<
				std::flush;
		throw std::runtime_error(s.str());
	}

	sword status = oci_bind_by_name(stmtp, &m_bindp, errhp, m_placeholder, static_cast<ub2>(m_type), m_valuep, m_value_sz_in_bytes, m_indp, m_maxarr_len, &m_curelen);

	if (Environment::debug())
	{
		std::cout << "ParameterArray::bind: END" << std::endl << std::flush;
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////
sb4 ParameterArray::valueSizeInByte(sb4 maxValueLen)
{
	sb4 size = 0;

	switch (m_type)
	{
		case String:
			size = static_cast<sb4>(oci_text::sizeInBytes(static_cast<size_t>(maxValueLen)));
			break;
		case Integer:
			size = maxValueLen;
			break;
		default:
			throw std::runtime_error("Illegal data type in ParameterArray::allocate");
	}

	return size;
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::allocate(sb4 valueSizeInByte, sb4 maxArrayLen)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::allocate m_placeholder=\"" << m_placeholder << "\" valueSizeInByte=" << valueSizeInByte << " maxArrayLen=" << maxArrayLen << std::endl << std::flush;
	}

	assert(valueSizeInByte > 0);
	assert(maxArrayLen > 0);
	assert(m_valuep == 0);
	assert(m_indp == 0);

	// allocate the value buffer
	m_valuep = reinterpret_cast<dvoid*>(malloc(valueSizeInByte * maxArrayLen));
	assert(m_valuep);

	// allocate the indicator buffer
	m_indp = reinterpret_cast<sb2*>(malloc(sizeof(sb2) * maxArrayLen));
	assert(m_indp);
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::initialize(sb4 valueSizeInByte, sb4 maxArrayLen)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::initialize m_placeholder=\"" << m_placeholder << "\" valueSizeInByte=" << valueSizeInByte << " maxArrayLen=" << maxArrayLen << std::endl << std::flush;
	}

	assert(valueSizeInByte > 0);
	assert(maxArrayLen > 0);
	assert(m_valuep != 0);
	assert(m_indp != 0);

	// initialize the value buffer
	memset(m_valuep, 0, valueSizeInByte * maxArrayLen);

	// initialize the indicator buffer
	memset(m_indp, 0, sizeof(sb2) * maxArrayLen);
}

} // namespace "ocip"
