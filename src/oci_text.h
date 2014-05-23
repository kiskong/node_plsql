#ifndef OCI_TEXT__H
#define OCI_TEXT__H

#include "oci_includes.h"

///////////////////////////////////////////////////////////////////////////
class oci_text
{
public:
	oci_text() : m_text(0), m_size(0) {}
	oci_text(size_t bytes);
	oci_text(const OraText* text);
	oci_text(const std::wstring& s);
	oci_text(const std::string& s);
	~oci_text();

	oci_text(const oci_text& t) {*this = t;}
	oci_text& operator=(const oci_text& t);

	OraText* text() const {return reinterpret_cast<OraText*>(m_text);}
	ub4 size() const {return static_cast<ub4>(m_size * sizeof(unsigned short));}

	std::wstring getWString() const;
	std::string getString() const;

	void dump(const std::string& desc = "") const;

private:
	void create(const std::wstring& s);

	// Alocate a buffer for max_size characters ad return the size in bytes
	size_t allocate(size_t max_size);

	unsigned short*	m_text;	//	the text
	size_t			m_size;	//	the size of the text in UTF16 character (not byte)
};

#endif // OCI_TEXT__H
