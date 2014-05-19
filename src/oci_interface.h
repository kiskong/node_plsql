#ifndef OCI_INTERFACE__H
#define OCI_INTERFACE__H

#include "oci_includes.h"

///////////////////////////////////////////////////////////////////////////
class oci_text
{
public:
	oci_text() {}
	oci_text(const std::wstring& s) : ws(s) {}
	oci_text(const std::string& s) : ws(std::wstring(s.begin(), s.end())) {}
	oci_text(const oci_text& t) : ws(reinterpret_cast<const wchar_t*>(&t)) {}
	std::wstring get_wstring() const {return ws;}
	std::string get_string() const {return std::string(ws.begin(), ws.end());}
	const wchar_t* data() const {return ws.c_str();}
	const OraText* text() const {return reinterpret_cast<const OraText*>(ws.c_str());}
	ub4 size() const {return static_cast<ub4>(ws.length() * sizeof(wchar_t));}

private:
	std::wstring	ws;
};

///////////////////////////////////////////////////////////////////////////
const char* oci_get_client_version();

///////////////////////////////////////////////////////////////////////////
sword oci_error_get(dvoid* hndlp, ub4 type, std::string* error_message, int* error_code);
sword oci_error_get(OCIError* errhp, std::string* error_message, int* error_code);
sword oci_error_get(OCIEnv* envhp, std::string* error_message, int* error_code);

///////////////////////////////////////////////////////////////////////////
sword oci_handle_allocate(OCIEnv* envhp, ub4 type, dvoid** hndlp);
sword oci_handle_free(ub4 type, dvoid** hndlp);

///////////////////////////////////////////////////////////////////////////
sword oci_connect_environment_create(OCIEnv** envhpp, ub4 mode, ub2 charset, ub2 ncharset);
sword oci_connect_pool_create(OCIEnv* envhp, OCIError* errhp, OCICPool* poolhp, OraText** poolName, sb4* poolNameLen, const std::string& username, const std::string& password, const std::string& server, int connMin, int connMax, int connIncr);
sword oci_connect_pool_destroy(OCICPool* poolhp, OCIError* errhp);
sword oci_logon(OCIEnv* envhp, OCIError* errhp, OCISvcCtx** svchp, const std::string& username, const std::string& password, const std::string& server);
sword oci_logon(OCIEnv* envhp, OCIError* errhp, OCISvcCtx** svchp, const std::string& username, const std::string& password, OraText* poolName, sb4 poolNameLen);
sword oci_logoff(OCISvcCtx* svchp, OCIError* errhp);

///////////////////////////////////////////////////////////////////////////
sword oci_attribute_set(dvoid* trgthndlp, ub4 trghndltyp, dvoid* attributep, ub4 size, ub4 attrtype, OCIError* errhp);

///////////////////////////////////////////////////////////////////////////
sword oci_statement_allocate(OCIEnv* envhp, OCIStmt** stmtpp);
sword oci_statement_free(OCIStmt* stmtp);
sword oci_statement_prepare(OCIStmt* stmtp, OCIError* errhp, const std::string& sql);
sword oci_bind_by_name(OCIStmt* stmtp, OCIBind** bindpp, OCIError* errhp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp = 0, ub4 maxarr_len = 0, ub4* curelen = 0);
sword oci_bind_by_pos(OCIStmt* stmtp, OCIDefine** defnpp, OCIError* errhp, ub4 position, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp = 0);
sword oci_statement_execute(OCIStmt* stmtp, OCISvcCtx* svchp, OCIError* errhp, ub4 iters);
sword oci_statement_fetch(OCIStmt* stmtp, OCIError* errhp, ub4 nrows = 1);
sword oci_statement_set_prefetch_count(OCIStmt* stmtp, OCIError* errhp, ub4 nrows);

///////////////////////////////////////////////////////////////////////////
sword oci_lob_descriptor_allocate(OCIEnv* envhp, OCILobLocator** locpp);
sword oci_lob_descriptor_free(OCILobLocator* locp);
sword oci_create_temporary_blob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp);
sword oci_create_temporary_clob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp);
sword oci_free_temporary_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp);
sword oci_open_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp);
sword oci_close_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp);
sword oci_lob_gen_length(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp, long* length);
sword oci_blob_read(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen);
sword oci_blob_write(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub1 piece);
sword oci_clob_read(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub2 csid);
sword oci_clob_write(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub1 piece, ub2 csid);

#endif // OCI_INTERFACE__H
