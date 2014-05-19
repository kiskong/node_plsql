#include "global.h"

#include "config.h"

///////////////////////////////////////////////////////////////////////////
void Config::debug() const
{
	std::cout << "Con Min:  " << m_conMin << std::endl;
	std::cout << "Con Max:  " << m_conMax << std::endl;
	std::cout << "Con Inc:  " << m_conIncr << std::endl;
	std::cout << "Username: " << m_username << std::endl;
	std::cout << "Password: " << m_password << std::endl;
	std::cout << "Database: " << m_database << std::endl;
	std::cout << "Debug:    " << m_debug << std::endl;
	std::cout << std::flush;
}
