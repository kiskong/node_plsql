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
Connection::Connection(int mode /*= OCI_DEFAULT*/)
	:	m_mode(static_cast<ub4>(mode))
	,	m_envhp(0)
	,	m_errhp(0)
	,	m_svchp(0)
	,	m_srvhp(0)
	,	m_usrhp(0)
	,	m_oracle_status(OCI_SUCCESS)
	,	m_isConnected(false)

{
	create();
}

///////////////////////////////////////////////////////////////////////////
Connection::~Connection()
{
	destroy();
}

///////////////////////////////////////////////////////////////////////////
void Connection::create()
{
	m_oracle_status = OCI_SUCCESS;

	// Create the Oracle environment
	m_oracle_status = oci_connect_environment_create(&m_envhp, m_mode, OCI_UTF16ID, OCI_UTF16ID);
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate an error handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate a service handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid**>(&m_svchp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate a server handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_SERVER, reinterpret_cast<dvoid**>(&m_srvhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Allocate a authentication handle:
	m_oracle_status = oci_handle_allocate(m_envhp, OCI_HTYPE_SESSION, reinterpret_cast<dvoid**>(&m_usrhp));
	assert(m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
void Connection::destroy()
{
	m_oracle_status = OCI_SUCCESS;

	// Free a server handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_SERVER, reinterpret_cast<dvoid**>(&m_srvhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free a service handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid**>(&m_svchp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free a authentication handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_SESSION, reinterpret_cast<dvoid**>(&m_usrhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free a error handle:
	m_oracle_status = oci_handle_free(OCI_HTYPE_ERROR, reinterpret_cast<dvoid**>(&m_errhp));
	assert(m_oracle_status == OCI_SUCCESS);

	// Free the environment handle
	m_oracle_status = oci_handle_free(OCI_HTYPE_ENV, reinterpret_cast<dvoid**>(&m_envhp));
	assert(m_oracle_status == OCI_SUCCESS);

	m_envhp = 0;
	m_errhp = 0;
	m_svchp = 0;
	m_srvhp = 0;
	m_usrhp = 0;
}

///////////////////////////////////////////////////////////////////////////
bool Connection::connect(const std::string& username, const std::string& password, const std::string& server, bool isSYSDBA)
{
	m_oracle_status = OCI_SUCCESS;

	// Convert the strings to UTF16
	oci_text o_username(username);
	oci_text o_password(password);
	oci_text o_server(server);

	// Attach to server:
	m_oracle_status = OCIServerAttach(	m_srvhp,
										m_errhp,
										o_server.text(),
										o_server.size(),
										(ub4)OCI_DEFAULT
										);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	// Set the server handle in the service handle:
	m_oracle_status = oci_attribute_set(reinterpret_cast<dvoid*>(m_svchp), OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid*>(m_srvhp), 0, OCI_ATTR_SERVER, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	// Set attributes in the authentication handle:
	m_oracle_status = oci_attribute_set(reinterpret_cast<dvoid*>(m_usrhp), OCI_HTYPE_SESSION, reinterpret_cast<dvoid*>(const_cast<OraText*>(o_username.text())), o_username.size(), OCI_ATTR_USERNAME, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}
	m_oracle_status = oci_attribute_set(reinterpret_cast<dvoid*>(m_usrhp), OCI_HTYPE_SESSION, reinterpret_cast<dvoid*>(const_cast<OraText*>(o_password.text())), o_password.size(), OCI_ATTR_PASSWORD, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	// Logon:
	m_oracle_status = OCISessionBegin(	m_svchp,
										m_errhp,
										m_usrhp,
										OCI_CRED_RDBMS,
										isSYSDBA ? OCI_SYSDBA : OCI_DEFAULT
										);
	if (m_oracle_status != OCI_SUCCESS)
	{
		m_oracle_status = OCIServerDetach(m_srvhp, m_errhp, (ub4)OCI_DEFAULT);
		return false;
	}


	// Set the authentication handle in the Service handle:
	m_oracle_status = oci_attribute_set(reinterpret_cast<dvoid*>(m_svchp), OCI_HTYPE_SVCCTX, reinterpret_cast<dvoid*>(m_usrhp), 0, OCI_ATTR_SESSION, m_errhp);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	m_isConnected = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool Connection::disconnect()
{
	m_oracle_status = OCI_SUCCESS;

	if (!m_isConnected)
	{
		return true;
	}

	// Terminte the user session context:
	m_oracle_status = OCISessionEnd(m_svchp, m_errhp, m_usrhp, (ub4)0);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	// Delete access to a datasource:
	m_oracle_status = OCIServerDetach(m_srvhp, m_errhp, (ub4)OCI_DEFAULT);
	if (m_oracle_status != OCI_SUCCESS)
	{
		return false;
	}

	m_isConnected = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
Statement::Statement(Connection& connection)
	:	m_connection(connection)
	,	m_stmtp(0)
{
	m_oracle_status = oci_statement_allocate(m_connection.hEnv(), &m_stmtp);
	assert(m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
Statement::~Statement()
{
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
	m_oracle_status = oci_statement_prepare(m_stmtp, m_connection.hError(), sql);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::execute(int iterations)
{
	// Actually bind the parameter
	ParameterListIteratorType it;
	for (it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		sword status = (*it)->bind(m_stmtp, m_connection.hError());
		assert(status == OCI_SUCCESS);
	}

	// Execute the statement
	m_oracle_status = oci_statement_execute(m_stmtp, m_connection.hSvcCtx(), m_connection.hError(), static_cast<ub4>(iterations));
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
bool Statement::bind(OCIBind** bindpp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp, ub4 maxarr_len, ub4* curelen)
{
	m_oracle_status = oci_bind_by_name(m_stmtp, bindpp, m_connection.hError(), placeholder, dty, valuep, value_sz, indp, maxarr_len, curelen);
	return (m_oracle_status == OCI_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////
void Statement::addParameter(Parameter* parameter)
{
	parameter->m_statement = this;

	m_parameters.push_back(parameter);
}

///////////////////////////////////////////////////////////////////////////
ParameterValue::ParameterValue(const std::string& placeholder, DataType type, Direction direction)
	:	Parameter(placeholder, type, direction)
	,	m_value_sz(0)
	,	m_ind(0)
{
}

///////////////////////////////////////////////////////////////////////////
ParameterValue::~ParameterValue()
{
}

///////////////////////////////////////////////////////////////////////////
void ParameterValue::value(long value)
{
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
	assert(m_direction == Input);
	assert(m_type == String);

	// convert to wstring
	std::wstring ws(value.begin(), value.end());

	// get the size of the value to bind
	size_t size = (ws.length() + 1) * sizeof(wchar_t);
	m_value_sz = static_cast<sb4>(size);

	// copy the value
	assert(size < sizeof(m_value));
	memmove(m_value, ws.c_str(), size);
}

///////////////////////////////////////////////////////////////////////////
sword ParameterValue::bind(OCIStmt* stmtp, OCIError* errhp)
{
	assert(m_statement);
	assert(stmtp);
	assert(errhp);
	assert(m_value_sz > 0);

	//std::cout << "ParameterValue::bind: " << m_placeholder << " (m_type=" << getTypeName(m_type) << " m_value_sz=" << m_value_sz << ")" << std::endl << std::flush;

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
}

///////////////////////////////////////////////////////////////////////////
ParameterArray::~ParameterArray()
{
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
	//std::cout << "ParameterArray::value: BEGIN" << std::endl << std::flush;

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

	// Add the 0 terminator and reserve space for wchar_t wide characters
	max_string_size = (max_string_size + 1) * sizeof (wchar_t);

	// get the size of the value to bind
	m_value_sz = static_cast<sb4>(max_string_size);

	// allocate and initialize buffers
	allocate(max_string_size, LIST_SIZE);

	// copy the values
	unsigned char* p = reinterpret_cast<unsigned char*>(m_valuep);
	for (it = list.begin(); it != list.end(); ++it)
	{
		std::wstring ws(it->begin(), it->end());
		memmove(p, ws.c_str(), (ws.length() + 1) * sizeof(wchar_t));
		p += max_string_size;
	}

	// Set the array sizes
	m_maxarr_len = static_cast<ub4>(LIST_SIZE);
	m_curelen = m_maxarr_len;

	//std::cout << "ParameterArray::value: END" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
sword ParameterArray::bind(OCIStmt* stmtp, OCIError* errhp)
{
	assert(m_statement);
	assert(stmtp);
	assert(errhp);
	assert(m_valuep);
	assert(m_value_sz > 0);
	assert(m_indp);
	assert(m_maxarr_len > 0);
	assert(m_curelen == m_maxarr_len);

	//std::cout << "ParameterArray::bind: " << m_placeholder << " (m_type=" << getTypeName(m_type) << " m_value_sz=" << m_value_sz << ")" << std::endl << std::flush;

	return oci_bind_by_name(stmtp, &m_bindp, errhp, m_placeholder, static_cast<ub2>(m_type), m_valuep, m_value_sz, m_indp, m_maxarr_len, &m_curelen);
}

///////////////////////////////////////////////////////////////////////////
void ParameterArray::allocate(size_t value_size, size_t array_size)
{
	assert(array_size > 0);

	//std::cout << "ParameterArray::allocate: (value_size=" << value_size << " array_size=" << array_size << ")" << std::endl << std::flush;

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
