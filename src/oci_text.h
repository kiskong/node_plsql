#ifndef OCI_TEXT__H
#define OCI_TEXT__H

#include "oci_includes.h"

///////////////////////////////////////////////////////////////////////////
class oci_text
{
public:
	oci_text() : ot(0), sz(0) {}
	oci_text(const std::wstring& s) : ws(s), ot(0), sz(0) {}
	oci_text(const std::string& s) : ws(std::wstring(s.begin(), s.end())), ot(0), sz(0) {}

	oci_text(const oci_text& t) {*this = t;}
	oci_text& operator=(const oci_text& t);

	std::wstring get_wstring() const {return ws;}
	std::string get_string() const {return std::string(ws.begin(), ws.end());}
	const wchar_t* data() const {return ws.c_str();}
	const OraText* text() const;
	ub4 size() const;
	void dump() const;

	static std::wstring to_wstring(const OraText* text);

	static size_t stringLength(const void* ptr, size_t size_elem);
	static int copy4to2bytes(const unsigned int* src, size_t src_size, unsigned short* dst, size_t dst_size);
	static int copy2to4bytes(const unsigned short* src, size_t src_size, unsigned int* dst, size_t dst_size);
	static void dump(const char* text, size_t size, const std::string& title = "");

private:
	std::wstring	ws;
	OraText*		ot;
	size_t			sz;	// the size allocated for ot
};

#endif // OCI_TEXT__H
