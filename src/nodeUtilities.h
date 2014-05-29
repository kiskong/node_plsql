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
inline bool isArgBoolean(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsBoolean());
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgInt32(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsInt32());
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgObject(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsObject());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgString(const v8::Arguments& args, int index, std::string* result)
{
	assert(result);

	v8::Local<v8::Value> value = args[index];
	if (!value->IsString())
	{
		return false;
	}
	
	v8::String::Utf8Value utf8String(value);
	*result = std::string(*utf8String);

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgBoolean(const v8::Arguments& args, int index, bool* result)
{
	assert(result);

	v8::Local<v8::Value> value = args[index];
	if (!value->IsBoolean())
	{
		return false;
	}

	*result = static_cast<int>(value->IsTrue());

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgInteger(const v8::Arguments& args, int index, long* result)
{
	assert(result);

	v8::Local<v8::Value> value = args[index];
	if (!value->IsNumber())
	{
		return false;
	}

	*result = static_cast<long>(value->ToNumber()->Value());

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgInteger(const v8::Arguments& args, int index, double* result)
{
	assert(result);

	v8::Local<v8::Value> value = args[index];
	if (!value->IsNumber())
	{
		return false;
	}

	*result = static_cast<double>(value->ToNumber()->Value());

	return true;
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
inline bool isArgArray(const v8::Arguments& args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsArray());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgArray(const v8::Arguments& args, int index, v8::Local<v8::Array>* array)
{
	if (index >= 0 && index < args.Length() && args[index]->IsArray())
	{
		*array = v8::Local<v8::Array>::Cast(args[index]);
		return true;
	}
	else
	{
		std::cerr << "ERROR in " << __FILE__ << "(" << __LINE__ << "): The argument \"" << index << "\" is not an array!" << std::endl;
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
inline bool getObjString(const v8::Local<v8::Object>& object, const std::string& key, std::string* result)
{
	assert(result);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (!value->IsString())
	{
		return false;
	}
	
	v8::String::Utf8Value utf8String(value);
	*result = std::string(*utf8String);

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline bool getObjBoolean(const v8::Local<v8::Object>& object, const std::string& key, bool* result)
{
	assert(result);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (!value->IsBoolean())
	{
		return false;
	}

	*result = static_cast<int>(value->IsTrue());

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline int getObjInteger(const v8::Local<v8::Object>& object, const std::string& key, long* result)
{
	assert(result);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (!value->IsNumber())
	{
		return false;
	}

	*result = static_cast<long>(value->ToNumber()->Value());

	return true;
}

///////////////////////////////////////////////////////////////////////////
inline double getObjNumber(const v8::Local<v8::Object>& object, const std::string& key, double* result)
{
	assert(result);

	v8::Local<v8::Value> value = object->Get(v8::String::New(key.c_str()));
	if (!value->IsNumber())
	{
		return false;
	}


	*result = static_cast<double>(value->ToNumber()->Value());

	return true;
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
