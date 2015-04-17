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
static size_t stringLength(const void* ptr, size_t size_elem);
static int copy4to2bytes(const unsigned int* src, size_t src_size, unsigned short* dst, size_t dst_size);
static int copy2to4bytes(const unsigned short* src, size_t src_size, unsigned int* dst, size_t dst_size);

///////////////////////////////////////////////////////////////////////////
oci_text::oci_text(size_t max_size)
	:	m_text(0)
	,	m_size(0)
{
	// Allocate m_text
	_allocate(max_size);
}

///////////////////////////////////////////////////////////////////////////
oci_text::oci_text(const OraText* text)
	:	m_text(0)
	,	m_size(0)
{
	// Allocate m_text
	const size_t bytes = _allocate(stringLength(reinterpret_cast<const void*>(text), sizeof(utf16_char_t)));

	// Copy m_text
	memmove(m_text, text, bytes);
}

///////////////////////////////////////////////////////////////////////////
oci_text::oci_text(const std::wstring& s)
	:	m_text(0)
	,	m_size(0)
{
	_create(s);
}

///////////////////////////////////////////////////////////////////////////
oci_text::oci_text(const std::string& s)
	:	m_text(0)
	,	m_size(0)
{
	std::wstring ws(s.begin(), s.end());
	_create(ws);
}

///////////////////////////////////////////////////////////////////////////
oci_text::~oci_text()
{
	if (m_text)
	{
		free(m_text);
	}
}

///////////////////////////////////////////////////////////////////////////
oci_text& oci_text::operator=(const oci_text& t)
{
	// Copy myself
	if (this == &t) {
		return *this;
	}

	// Free any existing resources
	if (m_text)
	{
		free(m_text);
	}
	m_text = 0;
	m_size = 0;

	// Allocate m_text
	const size_t bytes = _allocate(t.m_size);

	// Copy m_text
	memmove(m_text, t.m_text, bytes);

	return *this;
}

///////////////////////////////////////////////////////////////////////////
void oci_text::replace(const std::wstring& s)
{
	assert(m_text);
	assert(m_size > 0);

	// Create a copy of s
	std::wstring ws(s);

	// Resize the given string to a length of maximal m_size characters
	if (ws.size() > m_size)
	{
		ws.erase(m_size);
	}

#ifdef USE_LINUX
	// Convert 4 bytes to 2 bytes
	copy4to2bytes(reinterpret_cast<const unsigned int*>(ws.c_str()), m_size, m_text, m_size);
#else // USE_LINUX
	// Calculate the buffer size in bytes (including the needed 0 terminator)
	const size_t bytes = sizeInBytes(ws.size());

	memmove(reinterpret_cast<void*>(m_text), reinterpret_cast<const void*>(ws.c_str()), bytes);
#endif // USE_LINUX
}

///////////////////////////////////////////////////////////////////////////
void oci_text::replace(const std::string& s)
{
	std::wstring ws(s.begin(), s.end());
	replace(ws);
}

///////////////////////////////////////////////////////////////////////////
void oci_text::allocate(size_t max_size)
{
	// free any existing buffer
	if (m_text)
	{
		free(m_text);
	}
	m_text = 0;
	m_size = 0;

	// allocate a new buffer
	_allocate(max_size);
}

///////////////////////////////////////////////////////////////////////////
std::wstring oci_text::getWString() const
{
#ifdef USE_LINUX
	// Size in bytes
	size_t bytes = (m_size + 1) * sizeof(wchar_t);

	// Allocate buffer
	unsigned int* temp_buffer = reinterpret_cast<unsigned int*>(malloc(bytes));
	memset(temp_buffer, 0, bytes);

	// Convert 2 bytes to 4 bytes
	copy2to4bytes(m_text, m_size, temp_buffer, m_size);
	
	// Create a wstring
	std::wstring ws(reinterpret_cast<wchar_t*>(temp_buffer));

	// Free the buffer
	free(temp_buffer);
#else
	std::wstring ws((wchar_t*)m_text);
#endif

	return ws;
}

///////////////////////////////////////////////////////////////////////////
std::string oci_text::getString() const
{
	std::wstring ws(getWString());
	return std::string(ws.begin(), ws.end());
}

///////////////////////////////////////////////////////////////////////////
void oci_text::_create(const std::wstring& s)
{
	assert(m_text == 0);
	assert(m_size == 0);

	// Allocate m_text
	const size_t bytes = _allocate(s.size());
	assert(bytes >= s.size());

#ifdef USE_LINUX
	// Convert 4 bytes to 2 bytes
	copy4to2bytes(reinterpret_cast<const unsigned int*>(s.c_str()), m_size, m_text, m_size);
#else // USE_LINUX
	memmove(reinterpret_cast<void*>(m_text), reinterpret_cast<const void*>(s.c_str()), bytes);
#endif // USE_LINUX
}

///////////////////////////////////////////////////////////////////////////
size_t oci_text::_allocate(size_t max_size)
{
	assert(m_text == 0);
	assert(m_size == 0);

	m_size = max_size;

	// Calculate the buffer size in bytes
	const size_t bytes = sizeInBytes(m_size);

	// Allocate m_text
	m_text = reinterpret_cast<utf16_char_t*>(malloc(bytes));
	assert(m_text);

	// Initialize buffer
	memset(m_text, 0, bytes);

	// Return size in bytes
	return bytes;
}

///////////////////////////////////////////////////////////////////////////
void oci_text::dump(const std::string& desc) const
{
	std::wcout << L"oci_text: m_text=(" << m_text << ") m_size=(" << m_size << ")" << std::endl << std::flush;

	const size_t bytes = (m_size + 1) * sizeof(utf16_char_t);
	hexDump(desc.c_str(), reinterpret_cast<const void*>(m_text), static_cast<int>(bytes));
}

///////////////////////////////////////////////////////////////////////////
size_t oci_text::convert(const std::wstring& s, utf16_char_t* buffer, size_t max_bytes)
{
	// Initialize the buffer
	memset(buffer, 0, max_bytes);

	// Calculate the number of characters
	const size_t size = s.size();

	// Calculate the buffer size in bytes
	const size_t bytes = (size + 1) * sizeof(utf16_char_t);
	assert(bytes <= max_bytes);

#ifdef USE_LINUX
	// Convert 4 bytes to 2 bytes
	copy4to2bytes(reinterpret_cast<const unsigned int*>(s.c_str()), size, buffer, size);
#else // USE_LINUX
	memmove(reinterpret_cast<void*>(buffer), reinterpret_cast<const void*>(s.c_str()), bytes);
#endif // USE_LINUX

	return bytes;
}

///////////////////////////////////////////////////////////////////////////
std::wstring oci_text::convert_utf16_to_wstring(const utf16_char_t* buffer)
{
#ifdef USE_LINUX
	// Size in bytes
	size_t size = stringLength(buffer, sizeof(wchar_t));

	size_t bytes = (size + 1) * sizeof(wchar_t);

	// Allocate buffer
	unsigned int* temp_buffer = reinterpret_cast<unsigned int*>(malloc(bytes));
	memset(temp_buffer, 0, bytes);

	// Convert 2 bytes to 4 bytes
	copy2to4bytes(buffer, size, temp_buffer, size);
	
	// Create a wstring
	std::wstring ws(reinterpret_cast<wchar_t*>(temp_buffer));

	// Free the buffer
	free(temp_buffer);
#else
	std::wstring ws((wchar_t*)buffer);
#endif

	return ws;
}

///////////////////////////////////////////////////////////////////////////
size_t stringLength(const void* ptr, size_t size_elem)
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
int copy4to2bytes(const unsigned int* src, size_t src_size, unsigned short* dst, size_t dst_size)
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
int copy2to4bytes(const unsigned short* src, size_t src_size, unsigned int* dst, size_t dst_size)
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
