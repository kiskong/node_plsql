#include "global.h"

#include "config.h"

///////////////////////////////////////////////////////////////////////////
void Config::debug() const
{
	std::cout << "Username: " << itsUsername << std::endl;
	std::cout << "Password: " << itsPassword << std::endl;
	std::cout << "SYSDBA:   " << isSYSDBA << std::endl;
	std::cout << "Hostname: " << itsHostname << std::endl;
	std::cout << "Port:     " << itsPort << std::endl;
	std::cout << "Service:  " << itsService << std::endl;
	std::cout << "Debug:    " << isDebug << std::endl;
	std::cout << std::flush;
}
