#include "global.h"

#include "nodeUtilities.h"

namespace nodeUtilities
{

///////////////////////////////////////////////////////////////////////////
bool objectAsStringList(v8::Local<v8::Array>& arr, stringListType* list)
{
	list->clear();

	const uint32_t length = arr->Length();

	for (uint32_t i = 0 ; i < length ; ++i)
	{
		const v8::Local<v8::Value> entry = arr->Get(i);

		if (!entry->IsString())
		{
			return false;
		}
		v8::String::Utf8Value utf8_value(entry->ToString());
		std::string value = std::string(*utf8_value);

		list->push_back(value);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool objectAsStringLists(const v8::Local<v8::Object>& object, propertyListType* list)
{
	list->clear();

	const v8::Local<v8::Array> props = object->GetPropertyNames();
	const uint32_t length = props->Length();

	// Iterate all properties of the object
	for (uint32_t i = 0 ; i < length ; ++i)
	{
		const v8::Local<v8::Value> key = props->Get(i);
		const v8::Local<v8::Value> value = object->Get(key);

		if (!key->IsString() || !value->IsString())
		{
			return false;
		}

		propertyType property;

		v8::String::Utf8Value utf8_key(key->ToString());
		property.name = std::string(*utf8_key);

		v8::String::Utf8Value utf8_value(value->ToString());
		property.value = std::string(*utf8_value);

		list->push_back(property);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool objectAsParameterList(const v8::Local<v8::Object>& object, parameterListType* list)
{
	list->clear();

	const v8::Local<v8::Array> props = object->GetPropertyNames();
	const uint32_t length = props->Length();

	// Iterate all properties of the object
	for (uint32_t i = 0 ; i < length ; ++i)
	{
		// make sure that the key is a string
		const v8::Local<v8::Value> key = props->Get(i);
		if (!key->IsString())
		{
			return false;
		}

		// is the value a string
		const v8::Local<v8::Value> value = object->Get(key);
		v8::String::Utf8Value utf8_key(key->ToString());
		std::string p_name(*utf8_key);
		if (value->IsString())
		{
			v8::String::Utf8Value utf8_value(value->ToString());
			std::string p_value(*utf8_value);
			parameterType parameter(p_name, p_value);
			list->push_back(parameter);
		}
		else if (value->IsArray())
		{
			stringListType value_list;
			v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(value);
			if (!objectAsStringList(arr, &value_list))
			{
				return false;
			}
			
			parameterType parameter(p_name, value_list);
			list->push_back(parameter);
		}
		else
		{
			// the value is neither a string nor an array
			return false;
		}
	}

	return true;
}

} // namespace
