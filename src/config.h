#ifndef CONFIG__H
#define CONFIG__H

///////////////////////////////////////////////////////////////////////////
class Config
{
public:
	Config() : m_conMin(10), m_conMax(10000), m_conIncr(10), m_debug(false) {}
	~Config() {}
	void debug() const;

	int			m_conMin;
	int			m_conMax;
	int			m_conIncr;
	std::string	m_username;
	std::string m_password;
	std::string m_database;
	bool		m_debug;
};

#endif // CONFIG__H
