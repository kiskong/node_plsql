#ifndef GLOBAL__H
#define GLOBAL__H

///////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
# pragma warning(disable: 4506)		// no definition for inline function ...
#endif

///////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <string.h>
#include <malloc.h>

///////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <string>
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
void convert(const propertyListType& properties, stringListType* names, stringListType* values);

#endif // GLOBAL__H
