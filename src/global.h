#ifndef GLOBAL__H
#define GLOBAL__H

///////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
# pragma warning(disable: 4530)		// C++ exception handler used ...
# pragma warning(disable: 4506)		// no definition for inline function ...
# pragma warning(disable: 4996)		// This function or variable may be unsafe.
#endif

///////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__APPLE__)
# include <malloc.h>
#endif

#if defined(__GNUC__)
inline size_t stricmp(const char* s1, const char* s2) {return strcasecmp(s1, s2);}
inline size_t _stricmp(const char* s1, const char* s2) {return strcasecmp(s1, s2);}
#endif

///////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <exception>
#include <stdexcept>

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
class parameterType
{
public:
	enum dataType {Null, Scalar, Array};

	parameterType() : m_type(Null) {}
	parameterType(const std::string& name, const std::string& value)
		:	m_type(Scalar)
		,	m_name(name)
	{
		m_values.push_back(value);
	}
	parameterType(const std::string& name, const std::list<std::string>& values)
		:	m_type(Array)
		,	m_name(name)
	{
		std::list<std::string>::const_iterator it;
		for (it = values.begin(); it != values.end(); ++it)
		{
			m_values.push_back(*it);
		}

		assert(values.size() == m_values.size());
	}

	dataType type() const {return m_type;}
	std::string name() const {return m_name;}
	std::string value() const;
	std::list<std::string> values() const;
	std::string to_string() const;

private:
	dataType				m_type;
	std::string 			m_name;
	std::list<std::string> 	m_values;
};
typedef std::list<parameterType> parameterListType;
typedef std::list<parameterType>::iterator parameterListIteratorType;
typedef std::list<parameterType>::const_iterator parameterListConstIteratorType;

///////////////////////////////////////////////////////////////////////////
bool isDebug();
void replace(std::string& str, const std::string& from, const std::string& to);
void hexDump(const char* desc, const void* addr, int len);
void convert(const propertyListType& properties, stringListType* names, stringListType* values);
std::string to_string(const std::list<std::string>& list);
std::string to_string(const propertyListType& list);
std::string to_string(const parameterListType& list);

#endif // GLOBAL__H
