#include "global.h"

#include "oracleError.h"
#include "oci_includes.h"

///////////////////////////////////////////////////////////////////////////
oracleError::oracleError()
:	itsMessage("")
,	itsOracleStatus(OCI_SUCCESS)
,	itsOracleErrorCode(OCI_SUCCESS)
,	itsOracleErrorMessage("")
,	itsFile("")
,	itsLine(0)
{
}

///////////////////////////////////////////////////////////////////////////
oracleError::oracleError(const std::string& message, const std::string& file, int line)
:	itsMessage(message)
,	itsOracleStatus(OCI_SUCCESS)
,	itsOracleErrorCode(OCI_SUCCESS)
,	itsOracleErrorMessage("")
,	itsFile(file)
,	itsLine(line)
{
}

///////////////////////////////////////////////////////////////////////////
oracleError::oracleError(const std::string& message, int oracleStatus, int oracleErrorCode, const std::string& oracleErrorMessage, const std::string& file, int line)
:	itsMessage(message)
,	itsOracleStatus(oracleStatus)
,	itsOracleErrorCode(oracleErrorCode)
,	itsOracleErrorMessage(oracleErrorMessage)
,	itsFile(file)
,	itsLine(line)
{
}

///////////////////////////////////////////////////////////////////////////
const std::string oracleError::what() const
{
	std::ostringstream s;

	s	<< "Oracle error when: " << itsMessage << std::endl
		<< "Source location: " << itsFile << "(" << itsLine << ")" << std::endl
		<< "Oracle status code: " << itsOracleStatus << std::endl
		<< "Oracle error code: " << itsOracleErrorCode << std::endl
		<< "Oracle error message: " << itsOracleErrorMessage << std::endl;

	return s.str();
}
