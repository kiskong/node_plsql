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

	create();
}

///////////////////////////////////////////////////////////////////////////
Environment::~Environment()
{
	if (Environment::debug())
	{
		std::cout << "Environment::~Environment" << std::flush << std::endl;
	}

	destroy();
}

///////////////////////////////////////////////////////////////////////////
void Environment::create()
{
	if (Environment::debug())
	{
		std::cout << "Environment::create" << std::flush << std::endl;
	}

	// Create the Oracle environment
	sword status = oci_connect_environment_create(&m_envhp, m_mode, OCI_UTF16ID, OCI_UTF16ID);
	assert(status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
void Environment::destroy()
{
	if (Environment::debug())
	{
		std::cout << "Environment::destroy" << std::flush << std::endl;
	}

	// Free the environment handle
	sword status = oci_handle_free(OCI_HTYPE_ENV, reinterpret_cast<dvoid**>(&m_envhp));
	assert(status == OCI_SUCCESS);

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

	// Free a connection pool handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_CPOOL, reinterpret_cast<dvoid**>(&m_poolhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free a error handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);
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

	assert(m_hasPool);

	m_oracle_status = oci_connect_pool_destroy(m_poolhp, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
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
	,	m_isLoggedOn(false)
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
	,	m_isLoggedOn(false)
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

	m_isLoggedOn = true;

	return true;
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

	m_isLoggedOn = false;

	return true;
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
		if (s.size() > 100) {
			s = s.substr(0, 100) + "...";
		}
		std::cout << "Statement::prepare: sql=\"" << s << "\"" << std::flush << std::endl;
	}

	m_oracle_status = oci_statement_prepare(m_stmtp, m_connection->hError(), sql);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::execute(int iterations)
{
	if (Environment::debug())
	{
		std::cout << "Statement::execute: iterations=" << iterations << std::flush << std::endl;
	}

	// Actually bind the parameter
	ParameterListIteratorType it;
	for (it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		sword status = (*it)->bind(m_stmtp, m_connection->hError());
		assert(status == OCI_SUCCESS);
	}

	// Execute the statement
	m_oracle_status = oci_statement_execute(m_stmtp, m_connection->hSvcCtx(), m_connection->hError(), static_cast<ub4>(iterations));
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::bind(OCIBind** bindpp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp, ub4 maxarr_len, ub4* curelen)
{
	if (Environment::debug())
	{
		std::cout << "Statement::bind: m_placeholder=\"" << placeholder << "\"" << std::flush << std::endl;
	}

	m_oracle_status = oci_bind_by_name(m_stmtp, bindpp, m_connection->hError(), placeholder, dty, valuep, value_sz, indp, maxarr_len, curelen);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
void Statement::addParameter(Parameter* parameter)
{
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
ParameterValue::ParameterValue(const std::string& placeholder, DataType type, Direction direction)
	:	Parameter(placeholder, type, direction)
	,	m_value_sz(0)
	,	m_ind(0)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::ParameterValue: m_placeholder=\"" << m_placeholder << "\" type=\"" << getTypeName(type) << "\"" << std::flush << std::endl;
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
void ParameterValue::value(long value)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::value(long) m_placeholder=\"" << m_placeholder << "\" value=" << value << std::flush << std::endl;
	}

	assert(m_direction == Input);
	assert(m_type == Integer);

	// get the size of the value to bind
	size_t size = sizeof(value);
	m_value_sz = static_cast<sb4>(size);

	// copy the value
	assert(size < sizeof(m_value));
	memmove(m_value, &value, size);
}

///////////////////////////////////////////////////////////////////////////
void ParameterValue::value(const std::string& value)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::value(std::string) m_placeholder=\"" << m_placeholder << "\" value=\"" << value << "\"" << std::flush << std::endl;
	}

	assert(m_direction == Input);
	assert(m_type == String);

	// convert
	m_value_sz = oci_text::convert(value, reinterpret_cast<oci_text::utf16_char_t*>(m_value), sizeof(m_value));
}

///////////////////////////////////////////////////////////////////////////
sword ParameterValue::bind(OCIStmt* stmtp, OCIError* errhp)
{
	if (Environment::debug())
	{
		std::cout << "ParameterValue::bind m_placeholder=\"" << m_placeholder << "\" m_type=\"" << getTypeName(m_type) << "\" m_value_sz=" << m_value_sz << std::endl << std::flush;
	}

	assert(m_statement);
	assert(stmtp);
	assert(errhp);
	assert(m_value_sz > 0);

	return oci_bind_by_name(stmtp, &m_bindp, errhp, m_placeholder, static_cast<ub2>(m_type), reinterpret_cast<dvoid*>(m_value), m_value_sz, &m_ind);
}

///////////////////////////////////////////////////////////////////////////
ParameterArray::ParameterArray(const std::string& placeholder, DataType type, Direction direction)
	:	Parameter(placeholder, type, direction)
	,	m_valuep(0)
	,	m_value_sz(0)
	,	m_indp(0)
	,	m_maxarr_len(0)
	,	m_curelen(0)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::ParameterArray: m_placeholder=\"" << m_placeholder << "\" type=\"" << getTypeName(type) << "\"" << std::flush << std::endl;
	}
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

	assert(m_direction == Input);
	assert(m_type == Integer);
	assert(list.size() > 0);

	// get the size of the value to bind
	m_value_sz = static_cast<sb4>(sizeof(long));

	// allocate and initialize buffers
	allocate(sizeof(long), list.size());

	// copy the values
	std::list<long>::iterator it;
	long* p = reinterpret_cast<long*>(m_valuep);
	for (it = list.begin(); it != list.end(); ++it)
	{
		*p++ = *it;
	}
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::value(std::list<std::string> list)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::value(std::list<string>) m_placeholder=\"" << m_placeholder << "\" list.size()=" << list.size() << std::flush << std::endl;
	}

	assert(m_direction == Input);
	assert(m_type == String);
	assert(list.size() > 0);

	// Get the array size
	const size_t LIST_SIZE = list.size();

	// Find the widest string
	size_t max_string_size = 0;
	std::list<std::string>::iterator it;
	for (it = list.begin(); it != list.end(); ++it)
	{
		size_t size = it->size();
		if (size > max_string_size)
		{
			max_string_size = size;
		}
	}

	// Add the 0 terminator and reserve space for UTF16 characters
	max_string_size = (max_string_size + 1) * sizeof (oci_text::utf16_char_t);

	// get the size of the value to bind
	m_value_sz = static_cast<sb4>(max_string_size);

	// allocate and initialize buffers
	allocate(max_string_size, LIST_SIZE);

	// copy the values
	unsigned char* p = reinterpret_cast<unsigned char*>(m_valuep);
	for (it = list.begin(); it != list.end(); ++it)
	{
		/*size_t bytes =*/ oci_text::convert(*it, reinterpret_cast<oci_text::utf16_char_t*>(p), max_string_size);
		p += max_string_size;
	}

	// Set the array sizes
	m_maxarr_len = static_cast<ub4>(LIST_SIZE);
	m_curelen = m_maxarr_len;
}

///////////////////////////////////////////////////////////////////////////
sword ParameterArray::bind(OCIStmt* stmtp, OCIError* errhp)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::bind m_placeholder=\"" << m_placeholder << "\" m_type=\"" << getTypeName(m_type) << "\" m_value_sz=" << m_value_sz << std::endl << std::flush;
	}

	assert(m_statement);
	assert(stmtp);
	assert(errhp);
	assert(m_valuep);
	assert(m_value_sz > 0);
	assert(m_indp);
	assert(m_maxarr_len > 0);
	assert(m_curelen == m_maxarr_len);

	return oci_bind_by_name(stmtp, &m_bindp, errhp, m_placeholder, static_cast<ub2>(m_type), m_valuep, m_value_sz, m_indp, m_maxarr_len, &m_curelen);
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::allocate(size_t value_size, size_t array_size)
{
	if (Environment::debug())
	{
		std::cout << "ParameterArray::allocate m_placeholder=\"" << m_placeholder << "\" value_size=" << value_size << " array_size=" << array_size << std::endl << std::flush;
	}

	assert(array_size > 0);

	// set the value size
	m_value_sz = static_cast<sb4>(value_size);

	// allocate and initialize the value buffer
	m_valuep = reinterpret_cast<dvoid*>(malloc(value_size * array_size));
	assert(m_valuep);
	memset(m_valuep, 0, value_size * array_size);

	// allocate and initialize the indicator buffer
	m_indp = reinterpret_cast<sb2*>(malloc(sizeof(sb2) * array_size));
	assert(m_indp);
	memset(m_indp, 0, sizeof(sb2) * array_size);
}

} // namespace "ocip"
