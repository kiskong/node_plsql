#ifndef CONFIG__H
#define CONFIG__H

///////////////////////////////////////////////////////////////////////////
class Config
{
public:
	Config() : m_conMin(10), m_conMax(10000), m_conIncr(10), m_conPool(true), m_debug(false) {}
	~Config() {}
	std::string asString() const;

	int			m_conMin;
	int			m_conMax;
	int			m_conIncr;
	std::string	m_username;
	std::string m_password;
	std::string m_database;
	bool		m_conPool;
	bool		m_debug;
};

#endif // CONFIG__H
