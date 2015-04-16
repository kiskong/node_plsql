#ifndef ORACLEOBJECT__H
#define ORACLEOBJECT__H

#include "config.h"
#include "ocip_interface.h"

///////////////////////////////////////////////////////////////////////////
class fileType
{
public:
	fileType() {}
	fileType(const std::string& fieldname, const std::string& filename, const std::string& path, const std::string& encoding, const std::string& mimetype)
		:	m_fieldname(fieldname)
		,	m_filename(filename)
		,	m_path(path)
		,	m_encoding(encoding)
		,	m_mimetype(mimetype)
		{}
	std::string m_fieldname;
	std::string m_filename;
	std::string m_path;
	std::string m_encoding;
	std::string m_mimetype;
};
typedef std::list<fileType> fileListType;
typedef std::list<fileType>::iterator fileListIteratorType;
typedef std::list<fileType>::const_iterator fileListConstIteratorType;

///////////////////////////////////////////////////////////////////////////
class procType
{
public:
	enum ProcType
	{
		synonym = 5,
		procedure = 7,
		function = 8,
		package = 9
	};

	std::string				m_owner;
	long					m_type;
	std::string				m_name;
	std::string				m_part1;
	std::string				m_part2;
	std::list<std::string>	m_arguments;
	std::list<std::string>	m_dataTypes;
};

///////////////////////////////////////////////////////////////////////////
class OracleObject
{
public:
	// Constructor/Destructor
	OracleObject(const Config& config);
	~OracleObject();

	// Create
	bool create();
	bool destroy();

	// Execute SQL
	bool execute(const std::string& username, const std::string& password, const std::string& sql);

	// Request page
	bool request(const std::string& username, const std::string& password, const propertyListType& cgi, const fileListType& files, const std::string& tableName, const std::string& procedure, const parameterListType& parameters, std::wstring* page);

	// Return error object
	oracleError getOracleError() const {return m_OracleError;}

private:
	Config					m_Config;
	ocip::Environment*		m_environment;
	ocip::ConnectionPool*	m_connectionPool;
	oracleError				m_OracleError;

	// Request steps
	bool requestInit(ocip::Connection* connection, const propertyListType& cgi);
	bool requestUploadFiles(ocip::Connection* connection, const fileListType& files, const std::string& doctablename);
	bool uploadFile(ocip::Connection* connection, const fileType& file, const std::string& doctablename);
	bool requestRun(ocip::Connection* connection, const std::string& procedure, const parameterListType& parameters);
	bool requestPage(ocip::Connection* connection, std::wstring* page);

	bool findObjectToInvoke(ocip::Connection* connection, const std::string& name, procType* procData);

	ocip::Connection* createConnection();

	// Error handling
	void reportError(int oracleStatus, OCIError* errhp, const std::string& message, const std::string& file, int line);

	// Disable copy
	OracleObject(const OracleObject&);
	OracleObject &operator=(const OracleObject&);
};

#endif // ORACLEOBJECT__H
