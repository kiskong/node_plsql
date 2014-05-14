#ifndef OCIP_INTERFACE__H
#define OCIP_INTERFACE__H

#include "oci_interface.h"

namespace ocip
{

///////////////////////////////////////////////////////////////////////////
enum DataType
{
	String	= SQLT_STR,
	Integer = SQLT_INT
};
enum Direction
{
	Input,
	Output
};

///////////////////////////////////////////////////////////////////////////
std::string getTypeName(DataType type);

///////////////////////////////////////////////////////////////////////////
class Parameter;
class ParameterArray;

///////////////////////////////////////////////////////////////////////////
class Connection
{
public:
	Connection(int mode = OCI_DEFAULT);
	~Connection();

	bool connect(const std::string& username, const std::string& password, const std::string& server, bool isSYSDBA);
	bool disconnect();

	int status() const {return static_cast<int>(m_oracle_status);}
	bool isConnected() const {return m_isConnected;}

	OCIEnv* hEnv() const {return m_envhp;}
	OCIError* hError() const {return m_errhp;}
	OCISvcCtx* hSvcCtx() const {return m_svchp;}
	OCIServer* hServer() const {return m_srvhp;}
	OCISession* hSession() const {return m_usrhp;}

private:
	void create();
	void destroy();

	sb4					m_mode;
	OCIEnv*				m_envhp;
	OCIError*			m_errhp;
	OCISvcCtx*			m_svchp;
	OCIServer*			m_srvhp;
	OCISession*			m_usrhp;

	sword				m_oracle_status;
	bool				m_isConnected;

	// Disable copy/assign
	Connection(const Connection&);
	Connection &operator=(const Connection&);
};

///////////////////////////////////////////////////////////////////////////
class Statement
{
public:
	Statement(Connection& connection);
	~Statement();

	bool prepare(const std::string& sql);
	bool execute(int iterations);
	bool bind(OCIBind** bindpp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp = 0, ub4 maxarr_len = 0, ub4* curelen = 0);
	int status() const {return static_cast<int>(m_oracle_status);}
	OCIStmt* hStmt() const {return m_stmtp;}
	OCIError* hError() const {return m_connection.hError();}

	void addParameter(Parameter* parameter);

private:
	Connection&			m_connection;
	OCIStmt*			m_stmtp;
	sword				m_oracle_status;

	typedef std::list<Parameter*> ParameterListType;
	typedef std::list<Parameter*>::iterator ParameterListIteratorType;
	ParameterListType	m_parameters;

	// Disable copy/assign
	Statement(const Statement&);
	Statement &operator=(const Statement&);
};

///////////////////////////////////////////////////////////////////////////
class Parameter
{
public:
	friend class Statement;

	Parameter(const std::string& placeholder, DataType type, Direction direction) :	m_statement(0), m_placeholder(placeholder), m_type(type), m_direction(direction), m_bindp(0) {}
	virtual ~Parameter() {}

protected:
	virtual sword bind(OCIStmt* stmtp, OCIError* errhp) = 0;

	Statement*			m_statement;
	std::string			m_placeholder;
	DataType			m_type;
	Direction			m_direction;

	OCIBind*			m_bindp;

private:
	// Disable copy/assign
	Parameter(const Parameter&);
	Parameter &operator=(const Parameter&);
};

///////////////////////////////////////////////////////////////////////////
class ParameterValue : public Parameter
{
public:
	friend class Statement;

	ParameterValue(const std::string& placeholder, DataType type, Direction direction);
	virtual ~ParameterValue();

	void value(long value);
	void value(const std::string& value);

private:
	virtual sword bind(OCIStmt* stmtp, OCIError* errhp);

	unsigned char		m_value[32767];
	sb4					m_value_sz;
	sb2					m_ind;

	// Disable copy/assign
	ParameterValue(const ParameterValue&);
	ParameterValue &operator=(const ParameterValue&);
};

///////////////////////////////////////////////////////////////////////////
class ParameterArray : public Parameter
{
public:
	friend class Statement;

	ParameterArray(const std::string& placeholder, DataType type, Direction direction);
	virtual ~ParameterArray();

	void value(std::list<long> list);
	void value(std::list<std::string> list);

private:
	virtual sword bind(OCIStmt* stmtp, OCIError* errhp);
	void allocate(size_t value_size, size_t array_size);

	dvoid*				m_valuep;
	sb4					m_value_sz;
	sb2*				m_indp;
	ub4					m_maxarr_len;
	ub4					m_curelen;

	// Disable copy/assign
	ParameterArray(const ParameterArray&);
	ParameterArray &operator=(const ParameterArray&);
};

} // namespace "ocip"

#endif // OCIP_INTERFACE__H
