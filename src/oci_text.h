#ifndef OCI_TEXT__H
#define OCI_TEXT__H

#include "oci_includes.h"

///////////////////////////////////////////////////////////////////////////
class oci_text
{
public:
	// This is the representation of the UTF16 (2 byte) type used in Oracle
	typedef unsigned short utf16_char_t;

	// Contruct an empty text buffer
	oci_text() : m_text(0), m_size(0) {}

	// Construct a text buffer that can hold up to "max_size" characters (the 0 terminator will also added internally)
	oci_text(size_t max_size);

	// Construct a text buffer from an existing Oracle text buffer
	oci_text(const OraText* text);

	// Construct a text buffer from a std::wstring
	oci_text(const std::wstring& s);

	// Construct a text buffer from a std::sstring
	oci_text(const std::string& s);

	// Destruct a text buffer and free all allocated resources
	~oci_text();

	// Copy constructor and assignment operator
	oci_text(const oci_text& t) {*this = t;}
	oci_text& operator=(const oci_text& t);

	// Replace the text in the buffer but keep the buffer size
	void replace(const std::wstring& s);
	void replace(const std::string& s);

	// Allocate a text buffer that can hold up to "max_size" characters (the 0 terminator will also added internally)
	void allocate(size_t max_size);

	OraText* text() const {assert(m_text); return reinterpret_cast<OraText*>(m_text);}
	ub4 size() const {return static_cast<ub4>(m_size * sizeof(utf16_char_t));}

	std::wstring getWString() const;
	std::string getString() const;

	void dump(const std::string& desc = "") const;

	// Static convertion utilities

	// Convert a std::string to a std::wstring
	static std::wstring convert(const std::string& s) {return std::wstring(s.begin(), s.end());}

	// Convert a std::wstring to a std::string
	static std::string convert(const std::wstring& s) {return std::string(s.begin(), s.end());}

	// Convert a std::wstring to the Oracle UTF16 representation and return the number of bytes that have been used
	static size_t convert(const std::wstring& s, utf16_char_t* buffer, size_t max_bytes);

	// Convert a std::string to the Oracle UTF16 representation and return the number of bytes that have been used
	static size_t convert(const std::string& s, utf16_char_t* buffer, size_t max_bytes) {return convert(convert(s), buffer, max_bytes);}

	// Convert a string in Oracle UTF16 representation to a std::wstring
	static std::wstring convert_utf16_to_wstring(const utf16_char_t* buffer);

	// Convert a string in Oracle UTF16 representation to a std::string
	static std::string convert_utf16_to_string(const utf16_char_t* buffer) {return convert(convert_utf16_to_wstring(buffer));}

	// Convert the size in characters to the size in bytes (the 0 terminator will also be added)
	static size_t sizeInBytes(size_t size) {return (size + 1) * sizeof(utf16_char_t);}

private:
	// Create a new string buffer based on the existing wide string
	void _create(const std::wstring& s);

	// Allocate a buffer for max_size characters ad return the size in bytes
	size_t _allocate(size_t max_size);

	utf16_char_t*	m_text;	//	the text
	size_t			m_size;	//	the size of the text in UTF16 character (not byte)
};

#endif // OCI_TEXT__H
