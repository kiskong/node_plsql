#include "global.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#include "oci_includes.h"
#include "oci_text.h"

#ifdef USE_LINUX
# include <iconv.h>
#endif // USE_LINUX

///////////////////////////////////////////////////////////////////////////
//	unicode constants
#define UNI_SHIFT             ((int) 10 )
#define UNI_BASE              ((unsigned int) 0x0010000UL)
#define UNI_MASK              ((unsigned int) 0x3FFUL)
#define UNI_REPLACEMENT_CHAR  ((unsigned int) 0x0000FFFD)
#define UNI_MAX_BMP           ((unsigned int) 0x0000FFFF)
#define UNI_MAX_UTF16         ((unsigned int) 0x0010FFFF)
#define UNI_MAX_UTF32         ((unsigned int) 0x7FFFFFFF)
#define UNI_MAX_LEGAL_UTF32   ((unsigned int) 0x0010FFFF)
#define UNI_SUR_HIGH_START    ((unsigned int) 0xD800)
#define UNI_SUR_HIGH_END      ((unsigned int) 0xDBFF)
#define UNI_SUR_LOW_START     ((unsigned int) 0xDC00)
#define UNI_SUR_LOW_END       ((unsigned int) 0xDFFF)

#define CVT_SRC_ILLEGAL         0
#define CVT_SRC_EXHAUSTED      -1
#define CVT_DST_EXHAUSTED      -2

#define CVT_STRICT              0
#define CVT_LENIENT             1

#define UTF8_BYTES_PER_CHAR     4

///////////////////////////////////////////////////////////////////////////
oci_text& oci_text::operator=(const oci_text& t)
{
	// Copy myself
	if (this == &t) {
		return *this;
	}

	// Free buffer
	if (ot) {
		free(ot);
	}
	ot = 0;
	sz = 0;

	// Copy string
	ws = t.ws;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
const OraText* oci_text::text() const
{
	// Calculate the size:
	const size_t SIZE = (ws.size() + 1) * sizeof(wchar_t);

	// Do we need more space
	if (SIZE > sz)
	{
		// (re)Allocate buffer
		OraText* buffer = reinterpret_cast<OraText*>(realloc(ot, SIZE));
		memset(buffer, 0, SIZE);
		(const_cast<oci_text*>(this))->ot = buffer;
		assert(ot);
		(const_cast<oci_text*>(this))->sz = SIZE;
	}

#ifdef USE_LINUX

	{
	const size_t SIZE_ELEM = sizeof(wchar_t);
	assert(SIZE_ELEM >= 2);

	// source
	const unsigned int* src = reinterpret_cast<const unsigned int*>(ws.c_str());
	size_t src_size = ws.size();
	
	// destination
	unsigned short* dst = reinterpret_cast<unsigned short*>((const_cast<oci_text*>(this))->ot);
	size_t dst_size = src_size;

	// convert 4 bytes to 2 bytes
	/*int size =*/ copy4to2bytes(src, src_size, dst, dst_size);
	}

#else // USE_LINUX

	memmove((const_cast<oci_text*>(this))->ot, reinterpret_cast<const OraText*>(ws.c_str()), SIZE);

#endif // USE_LINUX

	return ot;
}

///////////////////////////////////////////////////////////////////////////
ub4 oci_text::size() const
{
	return static_cast<ub4>(ws.length() * sizeof(short));
}

///////////////////////////////////////////////////////////////////////////
void oci_text::dump() const
{
	std::cout << "oci_text: ws=(";
	std::wcout << ws;
	std::cout << ") ot=(" << (void*)ot << ") sz=(" << sz << ")" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
std::wstring oci_text::to_wstring(const OraText* text)
{
	const size_t SIZE_ELEM = sizeof(short);
	assert(SIZE_ELEM == 2);

	// source
	const unsigned short* src = reinterpret_cast<const unsigned short*>(text);
	size_t src_size = oci_text::stringLength(reinterpret_cast<const void*>(text), SIZE_ELEM);
	
	// destination
	unsigned int* dst = reinterpret_cast<unsigned int*>(malloc((src_size + 1) * sizeof(wchar_t)));
	size_t dst_size = src_size;

	// convert 2 bytes to 4 bytes
	/*int size =*/ oci_text::copy2to4bytes(src, src_size, dst, dst_size);

	// convert to std:wstring
	std::wstring ws(reinterpret_cast<wchar_t*>(dst));

	// free temporary buffer
	free(dst);

	return ws;
}

///////////////////////////////////////////////////////////////////////////
void oci_text::dump(const char* text, size_t size, const std::string& title)
{
	std::cout << "---" << title << "-BEGIN------------------------------" << std::endl;

	for (size_t i = 0; i < size; i++)
	{
		std::cout << i << ": " << "(" << (int)text[i] << ") ... (" << (char)text[i] << ")" << std::endl;
	}

	std::cout << "---" << title << "-END--------------------------------" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////
size_t oci_text::stringLength(const void* ptr, size_t size_elem)
{
    int size = 0;

    assert(ptr);

    if (size_elem == sizeof(char))
    {
        const char *s = (const char *) ptr;
        const char *e = (const char *) ptr;

        while (*e++) ;

        size = (int) (e - s - (size_t) 1);
    }
    else if (size_elem == sizeof(short))
    {
        const short *s = (const short *) ptr;
        const short *e = (const short *) ptr;

        while (*e++) ;

        size = (int) (e - s - (size_t) 1);
    }
    else if (size_elem == sizeof(int))
    {
        const int *s = (const int *) ptr;
        const int *e = (const int *) ptr;

        while (*e++) ;

        size = (int) (e - s - (size_t) 1);
    }

    return size;
}

///////////////////////////////////////////////////////////////////////////
int oci_text::copy4to2bytes(const unsigned int* src, size_t src_size, unsigned short* dst, size_t dst_size)
{
    int cp_size = 0;

    const unsigned int *src_end   = NULL;
    const unsigned short *dst_end = NULL;

    assert(src);
    assert(dst);

    src_end = src + src_size;
    dst_end = dst + dst_size;

    while (src < src_end)
    {
        unsigned int c;

        if (dst >= dst_end) return -1;

        c = *src++;

        if (c <= UNI_MAX_BMP)
        {
            if ((c >= UNI_SUR_HIGH_START) && (c < UNI_SUR_LOW_END))
            {
                *dst++ = UNI_REPLACEMENT_CHAR;
            }
            else
            {
                *dst++ = (unsigned short) c;
            }

            cp_size++;
        }
        else if (c > UNI_MAX_LEGAL_UTF32)
        {
            *dst++ = UNI_REPLACEMENT_CHAR;

            cp_size++;
        }
        else
        {
            if ((dst +  (size_t) 1) >= dst_end)
            {
                return -2;
            }

            c -= UNI_BASE;

            if (dst)
            {
                *dst++ = (unsigned short)((c >> UNI_SHIFT) + UNI_SUR_HIGH_START);
                *dst++ = (unsigned short)((c &  UNI_MASK ) + UNI_SUR_LOW_START );
            }

            cp_size++;
            cp_size++;
        }
    }

    return cp_size;
}

///////////////////////////////////////////////////////////////////////////
int oci_text::copy2to4bytes(const unsigned short* src, size_t src_size, unsigned int* dst, size_t dst_size)
{
    int cp_size = 0;

    const unsigned short *src_end = NULL;
    const unsigned int *dst_end   = NULL;

    unsigned int c1, c2;

    assert(src);
    assert(dst);

    src_end = src + src_size;
    dst_end = dst + dst_size;

    while (src < src_end)
    {
        c1 = *src++;

        if ((c1 >= UNI_SUR_HIGH_START) && (c1 <= UNI_SUR_HIGH_END))
        {
            if (src < src_end)
            {
                c2 = *src;

                if ((c2 >= UNI_SUR_LOW_START) && (c2 <= UNI_SUR_LOW_END))
                {
                    c1 = ((c1 - UNI_SUR_HIGH_START) << UNI_SHIFT) +
                          (c2 - UNI_SUR_LOW_START )  + UNI_BASE;

                    ++src;
                }
            }
            else
            {
                return -1;
            }
        }

        if (dst >= dst_end)
        {
            return -2;
        }

        *dst++ = c1;

        cp_size++;
    }

    return cp_size;
}
