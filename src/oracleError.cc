#include "global.h"

#include "oracleError.h"
#include "oci_includes.h"

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
const std::string oracleError::what() const
{
	std::ostringstream s;

	s	<< "Exception: " << itsMessage << " in " << itsFile << "(" << itsLine << ")" << std::endl
		<< "Oracle status code: " << itsOracleStatus << std::endl
		<< "Oracle error code: " << itsOracleErrorCode << std::endl
		<< "Oracle error message: " << itsOracleErrorMessage << std::endl;

	return s.str();
}
