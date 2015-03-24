#ifndef NODEUTILITIES__H
#define NODEUTILITIES__H

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4267)		// '=' : conversion from ... to ..., possible loss of data
# pragma warning(disable: 4244)		// 'return' : conversion from ... to ..., possible loss of data
#endif

#include <nan.h>

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

namespace nodeUtilities
{

///////////////////////////////////////////////////////////////////////////
inline bool isArgString(_NAN_METHOD_ARGS_TYPE args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsString());
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgBoolean(_NAN_METHOD_ARGS_TYPE args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsBoolean());
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgInt32(_NAN_METHOD_ARGS_TYPE args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsInt32());
}

///////////////////////////////////////////////////////////////////////////
inline bool isArgObject(_NAN_METHOD_ARGS_TYPE args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsObject());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgString(_NAN_METHOD_ARGS_TYPE args, int index, std::string* result)
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
inline bool getArgBoolean(_NAN_METHOD_ARGS_TYPE args, int index, bool* result)
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
inline bool getArgInteger(_NAN_METHOD_ARGS_TYPE args, int index, long* result)
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
inline bool getArgInteger(_NAN_METHOD_ARGS_TYPE args, int index, double* result)
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
inline bool getArgObject(_NAN_METHOD_ARGS_TYPE args, int index, v8::Local<v8::Object>* object)
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
inline bool isArgArray(_NAN_METHOD_ARGS_TYPE args, int index)
{
	return (index >= 0 && index < args.Length() && args[index]->IsArray());
}

///////////////////////////////////////////////////////////////////////////
inline bool getArgArray(_NAN_METHOD_ARGS_TYPE args, int index, v8::Local<v8::Array>* array)
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
	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
	return value->IsString();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjBoolean(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
	return value->IsBoolean();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjInteger(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
	return value->IsNumber();
}

///////////////////////////////////////////////////////////////////////////
inline bool isObjNumber(const v8::Local<v8::Object>& object, const std::string& key)
{
	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
	return value->IsNumber();
}

///////////////////////////////////////////////////////////////////////////
inline bool getObjString(const v8::Local<v8::Object>& object, const std::string& key, std::string* result)
{
	assert(result);

	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
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

	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
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

	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
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

	v8::Local<v8::Value> value = object->Get(NanNew<v8::String>(key.c_str()));
	if (!value->IsNumber())
	{
		return false;
	}


	*result = static_cast<double>(value->ToNumber()->Value());

	return true;
}

///////////////////////////////////////////////////////////////////////////
// convert an object containing an array of strings to a stringListType
bool objectAsStringList(const v8::Local<v8::Array>& arr, stringListType* list);


///////////////////////////////////////////////////////////////////////////
// convert an object containing only properties of type string to a list of
//	propertyType entries.
bool objectAsStringLists(const v8::Local<v8::Object>& object, propertyListType* list);

///////////////////////////////////////////////////////////////////////////
// convert an object containing properties of type string or list of string
// to a list of parameterType entries.
bool objectAsParameterList(const v8::Local<v8::Object>& object, parameterListType* list);

} // namespace

#endif // NODEUTILITIES__H
