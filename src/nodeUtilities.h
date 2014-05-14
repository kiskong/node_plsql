#ifndef NODEUTILITIES__H
#define NODEUTILITIES__H

#include <node.h>

namespace nodeUtilities
{

///////////////////////////////////////////////////////////////////////////
inline bool isArgString(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsString());
}

///////////////////////////////////////////////////////////////////////////
inline std::string getArgString(const v8::Arguments& args, int index)
{
	v8::String::Utf8Value param(args[index]->ToString());
	return std::string(*param);
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgInt32(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsInt32());
}

///////////////////////////////////////////////////////////////////////////
inline int getArgInt32(const v8::Arguments& args, int index)
{
	return args[index]->Int32Value();
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgBoolean(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsBoolean());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgBoolean(const v8::Arguments& args, int index)
{
	return args[index]->BooleanValue();
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgObject(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsObject());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgObject(const v8::Arguments& args, int index, v8::Local<v8::Object>* object)
{
	if (index >= 0 && index < args.Length() && args[index]->IsObject())
	{
		*object = v8::Local<v8::Object>::Cast(args[index]);
		return true;
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The argument \"" << index << "\" is not an object!" << std::endl;
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjString(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	return value->IsString();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjBoolean(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	return value->IsBoolean();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjInteger(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	return value->IsNumber();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjNumber(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	return value->IsNumber();
}

///////////////////////////////////////////////////////////////////////////
inline std::string getObjString(const v8::Local<v8::Object>& object, const std::string& key)
{
	std::string s;

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (value->IsString())
	{
		v8::String::Utf8Value utf8String(value);
		s = std::string(*utf8String);
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The key \"" << key << "\" is not a string!" << std::endl;
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////
inline bool getObjBoolean(const v8::Local<v8::Object>& object, const std::string& key)
{
	bool b(false);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (value->IsBoolean())
	{
		b = static_cast<int>(value->IsTrue());
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The key \"" << key << "\" is not a boolean!" << std::endl;
	}

	return b;
}

///////////////////////////////////////////////////////////////////////////
inline int getObjInteger(const v8::Local<v8::Object>& object, const std::string& key)
{
	int n(0);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (value->IsNumber())
	{
		n = static_cast<int>(value->ToNumber()->Value());
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The key \"" << key << "\" is not a number!" << std::endl;
	}

	return n;
}

///////////////////////////////////////////////////////////////////////////
inline double getObjNumber(const v8::Local<v8::Object>& object, const std::string& key)
{
	double n(0);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (value->IsNumber())
	{
		n = static_cast<double>(value->ToNumber()->Value());
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The key \"" << key << "\" is not a number!" << std::endl;
	}

	return n;
}

///////////////////////////////////////////////////////////////////////////
inline void ThrowError(const std::string& text)
{
	v8::ThrowException(v8::Exception::Error(v8::String::New(text.c_str())));
}

///////////////////////////////////////////////////////////////////////////
inline void ThrowTypeError(const std::string& text)
{
	v8::ThrowException(v8::Exception::TypeError(v8::String::New(text.c_str())));
}

///////////////////////////////////////////////////////////////////////////
// convert an object containing only properties of type string to a list of
//	propertyType entries.
bool objectAsStringLists(const v8::Local<v8::Object>& object, propertyListType* list);

} // namespace

#endif // NODEUTILITIES__H
