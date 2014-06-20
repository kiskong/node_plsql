#ifndef GLOBAL__H
#define GLOBAL__H

///////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
# pragma warning(disable: 4530)		// C++ exception handler used ...
# pragma warning(disable: 4506)		// no definition for inline function ...
#endif

///////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__APPLE__)
# include <malloc.h>
#endif

#if defined(__GNUC__)
inline size_t stricmp(const char* s1, const char* s2)
{
	return strcasecmp(s1, s2);
}
#endif

///////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <list>

///////////////////////////////////////////////////////////////////////////
typedef std::list<std::string> stringListType;
typedef std::list<std::string>::iterator stringListIteratorType;
typedef std::list<std::string>::const_iterator stringListConstIteratorType;

///////////////////////////////////////////////////////////////////////////
class propertyType
{
public:
	propertyType() {}
	propertyType(const std::string& n, const std::string& v) : name(n), value(v) {}
	std::string name;
	std::string value;
};
typedef std::list<propertyType> propertyListType;
typedef std::list<propertyType>::iterator propertyListIteratorType;
typedef std::list<propertyType>::const_iterator propertyListConstIteratorType;

///////////////////////////////////////////////////////////////////////////
bool isDebug();
void replace(std::string& str, const std::string& from, const std::string& to);
void hexDump(const char* desc, const void* addr, int len);
void convert(const propertyListType& properties, stringListType* names, stringListType* values);

#endif // GLOBAL__H
