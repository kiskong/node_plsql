#include "global.h"

///////////////////////////////////////////////////////////////////////////
void convert(const propertyListType& properties, stringListType* names, stringListType* values)
{
	assert(names && values);

	names->clear();
	values->clear();

	propertyListConstIteratorType it;
	for (it = properties.begin(); it != properties.end(); ++it)
	{
		names->push_back(it->name);
		values->push_back(it->value);
	}

	assert(properties.size() == names->size());
	assert(properties.size() == values->size());
}
