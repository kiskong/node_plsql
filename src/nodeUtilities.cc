#include "global.h"

#include "nodeUtilities.h"

namespace nodeUtilities
{

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

} // namespace
