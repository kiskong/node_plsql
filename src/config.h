#ifndef CONFIG__H
#define CONFIG__H

///////////////////////////////////////////////////////////////////////////
class Config
{
public:
	Config() : isSYSDBA(false), itsPort(1521), isDebug(false) {}
	~Config() {}
	void debug() const;

	std::string	itsUsername;
	std::string itsPassword;
	bool		isSYSDBA;
	std::string itsHostname;
	int			itsPort;
	std::string itsDatabase;
	bool		isDebug;
};

#endif // CONFIG__H
