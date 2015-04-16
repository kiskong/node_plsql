#ifndef ORACLEERROR__H
#define ORACLEERROR__H

///////////////////////////////////////////////////////////////////////////
class oracleError
{
public:
	oracleError();
	oracleError(const std::string& message, const std::string& file, int line);
	oracleError(const std::string& message, int oracleStatus, int oracleErrorCode, const std::string& oracleErrorMessage, const std::string& file, int line);

	const std::string what() const;

	std::string getMessage() const {return itsMessage;}
	int getOracleStatus() const {return itsOracleStatus;}
	int getOracleErrorCode() const {return itsOracleErrorCode;}
	std::string getOracleErrorMessage() const {return itsOracleErrorMessage;}
	std::string getFile() const {return itsFile;}
	int getLine() const {return itsLine;}

private:
	std::string	itsMessage;
	int			itsOracleStatus;
	int			itsOracleErrorCode;
	std::string	itsOracleErrorMessage;
	std::string	itsFile;
	int			itsLine;
};

#endif // ORACLEERROR__H
