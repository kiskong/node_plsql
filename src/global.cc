#include "global.h"

///////////////////////////////////////////////////////////////////////////
void replace(std::string& str, const std::string& from, const std::string& to)
{
    for (;;)
    {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
		{
			return;
		}
		str.replace(start_pos, from.length(), to);
	}
}

///////////////////////////////////////////////////////////////////////////
void hexDump(const char* desc, const void* addr, int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char* pc = reinterpret_cast<const unsigned char*>(addr);

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

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
