#ifndef ORACLEOBJECT__H
#define ORACLEOBJECT__H

#include "config.h"
#include "oracleError.h"
#include "ocip_interface.h"

///////////////////////////////////////////////////////////////////////////
class OracleObject
{
public:
	// Constructor/Destructor
	OracleObject(const Config& config);
	~OracleObject();

	// Connect and disconnect
	bool connect();
	bool disconnect();

	bool requestInit(const propertyListType& cgi);
	bool requestRun(const std::string& procedure, const propertyListType& parameters);
	bool requestPage(std::wstring* page);

	// Status
	oracleError getOracleError() const {return itsOracleError;}

private:
	Config				itsConfig;
	ocip::Connection*	connection;
	oracleError			itsOracleError;

	// Error handling
	std::string getErrorMessage();
	void reportError(int oracleStatus, const std::string& message, const std::string& file, int line);

	// Utilities
	std::string getConnectString() const;

	// Disable copy
	OracleObject(const OracleObject&);
	OracleObject &operator=(const OracleObject&);
};

#endif // ORACLEOBJECT__H
