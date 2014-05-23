#include "global.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#include "oci_interface.h"

///////////////////////////////////////////////////////////////////////////
const char* oci_get_client_version()
{
	sword major_version = 0, minor_version = 0, update_num = 0, patch_num = 0, port_update_num = 0;

	OCIClientVersion(&major_version, &minor_version, &update_num, &patch_num, &port_update_num);

	static char client_version[128];
	sprintf(client_version, "%d.%d.%d.%d.%d", (int)major_version, (int)minor_version, (int)update_num, (int)patch_num, (int)port_update_num);

	return client_version;
}

///////////////////////////////////////////////////////////////////////////
sword oci_error_get(dvoid* hndlp, ub4 type, std::string* error_message, int* error_code)
{
	sb4	errcode = 0;
	oci_text o_error(32767);

	sword status = OCIErrorGet(
		hndlp,					// hndlp        (IN) The error handle, usually, or the environment handle (for errors on OCIEnvCreate(), OCIHandleAlloc()).
		1,						// recordno     (IN) Indicates the status record from which the application seeks information. Starts from 1.
		(OraText*) 0,			// sqlstate    (OUT) Not supported in release 8.x or later.
		&errcode,				// errcodep    (OUT) The error code returned.
		o_error.text(),			// bufp        (OUT) The error message text returned.
		o_error.size(),			// bufsiz       (IN) The size of the buffer provided for the error message, in number of bytes.
		type					// type         (IN) The type of the handle (OCI_HTYPE_ERROR or OCI_HTYPE_ENV).
		);

	*error_message = o_error.getString();
	*error_code	= static_cast<int>(errcode);

	return status;
}

///////////////////////////////////////////////////////////////////////////
sword oci_error_get(OCIError* errhp, std::string* error_message, int* error_code)
{
	return oci_error_get(reinterpret_cast<dvoid*>(errhp), OCI_HTYPE_ERROR, error_message, error_code);
}

///////////////////////////////////////////////////////////////////////////
sword oci_error_get(OCIEnv* envhp, std::string* error_message, int* error_code)
{
	return oci_error_get(reinterpret_cast<dvoid*>(envhp), OCI_HTYPE_ENV, error_message, error_code);
}

///////////////////////////////////////////////////////////////////////////
sword oci_handle_allocate(OCIEnv* envhp, ub4 type, dvoid** hndlpp)
{
	return OCIHandleAlloc(
		(dvoid *) envhp,		// parenth      (IN) An environment handle.
		hndlpp,					// hndlpp      (OUT) Returns a handle.
		type,					// type         (IN) Specifies the type of handle to be allocated.
		(CONST size_t) 0,		// xtramem_sz   (IN) Specifies an amount of user memory to be allocated.
		(dvoid **) 0			// usrmempp    (OUT) Returns a pointer to the user memory of size xtramem_sz allocated by the call for the user.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_handle_free(ub4 type, dvoid** hndlp)
{
	return OCIHandleFree(
		(dvoid*) hndlp,			// hndlp        (IN) A handle allocated by OCIHandleAlloc().
		type					// type         (IN) Specifies the type of storage to be freed.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_connect_environment_create(OCIEnv** envhpp, ub4 mode, ub2 charset, ub2 ncharset)
{
	return OCIEnvNlsCreate(
		envhpp,					// envhpp      (OUT) A pointer to an environment handle whose encoding setting is specified by mode.
		mode,					// mode         (IN) Specifies initialization of the mode.
		0,						// ctxp         (IN) Specifies the user-defined context for the memory callback routines.
		0,						// malocfp      (IN) Specifies the user-defined memory allocation function. If mode is OCI_THREADED, this memory allocation routine must be thread-safe.
		0,						// ralocfp      (IN) Specifies the user-defined memory reallocation function. If the mode is OCI_THREADED, this memory allocation routine must be thread-safe.
		0,						// mfreefp      (IN) Specifies the user-defined memory free function. If the mode is OCI_THREADED, this memory free routine must be thread-safe.
		0,						// xtramemsz    (IN) Specifies the amount of user memory to be allocated for the duration of the environment.
		0,						// usrmempp    (OUT) Returns a pointer to the user memory of size xtramemsz allocated by the call for the user.
		charset,				// charset      (IN) The client-side character set for the current environment handle. If it is 0, the NLS_LANG setting is used. OCI_UTF16ID is a valid setting; it is used by the metadata and the CHAR data.
		ncharset				// ncharset     (IN) The client-side national character set for the current environment handle. If it is 0, NLS_NCHAR setting is used. OCI_UTF16ID is a valid setting; it is used by the NCHAR data.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_connect_pool_create(OCIEnv* envhp, OCIError* errhp, OCICPool* poolhp, OraText** poolName, sb4* poolNameLen, const std::string& username, const std::string& password, const std::string& server, int connMin, int connMax, int connIncr)
{
	assert(connMin > 0);
	assert(connMax > connMin);
	assert(connIncr > 0);

	oci_text	o_username(username);
	oci_text	o_password(password);
	oci_text	o_server(server);

	sword status = OCIConnectionPoolCreate(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		poolhp,					// poolhp       (IN) An allocated pool handle.
		poolName,				// poolName    (OUT) The name of the connection pool connected to.
		poolNameLen,			// poolNameLen (OUT) The length of the string pointed to by poolName.
		o_server.text(),		// dblink       (IN) Specifies the database (server) to connect to.
		o_server.size(),		// dblinkLen    (IN) The length of the string pointed to by dblink.
		(ub4)connMin,			// connMin      (IN) Specifies the minimum number of connections in the connection pool. Valid values are 0 and above.
		(ub4)connMax,			// connMax      (IN) Specifies the maximum number of connections that can be opened to the database. Once this value is reached, no more connections are opened. Valid values are 1 and above.
		(ub4)connIncr,			// connIncr     (IN) Allows the application to set the next increment for connections to be opened to the database if the current number of connections are less than connMax. Valid values are 0 and above.
		o_username.text(),		// poolUsername (IN) Connection pooling requires an implicit primary session and this attribute provides a username for that session.
		o_username.size(),		// poolUserLen  (IN) The length of poolUsername.
		o_password.text(),		// poolPassword (IN) The password for the username poolUsername.
		o_password.size(),		// poolPassLen  (IN) The length of poolPassword.
		OCI_DEFAULT				// mode         (IN) The modes supported are
		);

	return status;
}

///////////////////////////////////////////////////////////////////////////
sword oci_connect_pool_destroy(OCICPool* poolhp, OCIError* errhp)
{
	return OCIConnectionPoolDestroy(
		poolhp,					// poolhp       (IN) A pool handle for which a pool has been created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		OCI_DEFAULT				// mode         (IN) Currently, this function will support only the OCI_DEFAULT mode.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_logon(OCIEnv* envhp, OCIError* errhp, OCISvcCtx** svchp, const std::string& username, const std::string& password, const std::string& server)
{
	oci_text	o_username(username);
	oci_text	o_password(password);
	oci_text	o_server(server);

	return OCILogon2(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		svchp,					// svchp    (IN/OUT) Address of an OCI service context pointer. This is filled with a server and session handle.
		o_username.text(),		// username     (IN) The user name used to authenticate the session. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		o_username.size(),		// uname_len    (IN) The length of username, in number of bytes, regardless of the encoding.
		o_password.text(),		// password     (IN) The user's password. For connection pooling, if this parameter is NULL then OCILogon2() assumes that the logon is for a proxy user. It implicitly creates a proxy connection in such a case, using the pool user to authenticate the proxy user. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		o_password.size(),		// passwd_len   (IN) The length of password, in number of bytes, regardless of the encoding.
		o_server.text(),		// dbname       (IN) For the default case, this indicates the connect string to use to connect to the Oracle Database.
		o_server.size(),		// dbname_len   (IN) The length of dbname. For session pooling and connection pooling, this value is returned by the OCISessionPoolCreate() or OCIConnectionPoolCreate() call respectively.
		OCI_DEFAULT				// mode         (IN) The values accepted are: OCI_DEFAULT, OCI_LOGON2_CPOOL, OCI_LOGON2_SPOOL, OCI_LOGON2_STMTCACHE, OCI_LOGON2_PROXY
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_logon(OCIEnv* envhp, OCIError* errhp, OCISvcCtx** svchp, const std::string& username, const std::string& password, OraText* poolName, sb4 poolNameLen)
{
	oci_text	o_username(username);
	oci_text	o_password(password);

	return OCILogon2(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		svchp,					// svchp    (IN/OUT) Address of an OCI service context pointer. This is filled with a server and session handle.
		o_username.text(),		// username     (IN) The user name used to authenticate the session. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		o_username.size(),		// uname_len    (IN) The length of username, in number of bytes, regardless of the encoding.
		o_password.text(),		// password     (IN) The user's password. For connection pooling, if this parameter is NULL then OCILogon2() assumes that the logon is for a proxy user. It implicitly creates a proxy connection in such a case, using the pool user to authenticate the proxy user. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		o_password.size(),		// passwd_len   (IN) The length of password, in number of bytes, regardless of the encoding.
		poolName,				// dbname       (IN) For the default case, this indicates the connect string to use to connect to the Oracle Database.
		poolNameLen,			// dbname_len   (IN) The length of dbname. For session pooling and connection pooling, this value is returned by the OCISessionPoolCreate() or OCIConnectionPoolCreate() call respectively.
		OCI_LOGON2_CPOOL		// mode         (IN) The values accepted are: OCI_DEFAULT, OCI_LOGON2_CPOOL, OCI_LOGON2_SPOOL, OCI_LOGON2_STMTCACHE, OCI_LOGON2_PROXY
		);
}


///////////////////////////////////////////////////////////////////////////
sword oci_logoff(OCISvcCtx* svchp, OCIError* errhp)
{
	return OCILogoff(
		svchp,					// svchp        (IN) The service context handle that was used in the call to OCILogon() or OCILogon2().
		errhp					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_attribute_set(dvoid* trgthndlp, ub4 trghndltyp, dvoid* attributep, ub4 size, ub4 attrtype, OCIError* errhp)
{
	return OCIAttrSet(
		trgthndlp,				// trgthndlp  (IN/OUT) Pointer to a handle type whose attribute gets modified.
		trghndltyp,				// trghndltyp (IN/OUT) The handle type.
		attributep,				// attributep     (IN) Pointer to an attribute value. The attribute value is copied into the target handle. If the attribute value is a pointer, then only the pointer is copied, not the contents of the pointer. String attributes must be in UTF-16 in OCI_UTF16 environment.
		size,					// size           (IN) The size of an attribute value. This can be passed in as 0 for most attributes as the size is already known by the OCI library. For text* attributes, a ub4 must be passed in set to the length of the string in bytes, regardless of encoding.
		attrtype,				// attrtype       (IN) The type of attribute being set:
		errhp					// errhp      (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_allocate(OCIEnv* envhp, OCIStmt** stmtpp)
{
	return OCIHandleAlloc(
		(dvoid *) envhp,		// parenth      (IN) An environment handle.
		(dvoid**) stmtpp,		// hndlpp      (OUT) Returns a handle.
		OCI_HTYPE_STMT,			// type         (IN) Specifies the type of handle to be allocated.
		(CONST size_t) 0,		// xtramem_sz   (IN) Specifies an amount of user memory to be allocated.
		(dvoid **) 0			// usrmempp    (OUT) Returns a pointer to the user memory of size xtramem_sz allocated by the call for the user.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_free(OCIStmt* stmtp)
{
	return OCIHandleFree(
		(dvoid*) stmtp,			// hndlp        (IN) A handle allocated by OCIHandleAlloc().
		OCI_HTYPE_STMT			// type         (IN) Specifies the type of storage to be freed.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_prepare(OCIStmt* stmtp, OCIError* errhp, const std::string& sql)
{
	oci_text o_sql(sql);

	return OCIStmtPrepare(
		stmtp,					// stmtp    (IN/OUT) The statement handle to the SQL or PL/SQL statement being processed.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		o_sql.text(),			// stmt     (IN)     SQL or PL/SQL statement to be executed. Must be a NULL-terminated string.
		o_sql.size(),			// stmt_len (IN)     Length of the statement in characters or in number of bytes, depending on the encoding. Must not be zero.
		OCI_NTV_SYNTAX,			// language (IN)     Specifies V7, or native syntax.
		OCI_DEFAULT				// mode     (IN)     Similar to the mode in the OCIEnvCreate() call, but this one has higher priority because it can override the "naturally" inherited mode setting.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_bind_by_name(OCIStmt* stmtp, OCIBind** bindpp, OCIError* errhp, const std::string& placeholder, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp /*= 0*/, ub4 maxarr_len /*= 0*/, ub4* curelen /*= 0*/)
{
	oci_text o_placeholder(placeholder);

	return OCIBindByName(
		stmtp,					// stmtp    (IN/OUT) The statement handle to the SQL or PL/SQL statement being processed.
		bindpp,					// bindpp   (IN/OUT) A pointer to save the pointer of a bind handle which is implicitly allocated by this call.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		o_placeholder.text(),	// placeholder  (IN) The placeholder, specified by its name, which maps to a variable in the statement associated with the statement handle.
		o_placeholder.size(),	// placeh_len   (IN) The length of the name specified in placeholder, in number of bytes regardless of the encoding.
		valuep,					// valuep   (IN/OUT) The pointer to a data value or an array of data values of the type specified in the dty parameter.
		value_sz,				// value_sz     (IN) The size in bytes of the data value pointed to by dvoid pointer valuep.
		dty,					// dty          (IN) The datatype of the value(s) being bound.
		(dvoid *) indp,			// indp     (IN/OUT) Pointer to an indicator variable or array.
		(ub2 *) 0,				// alenp    (IN/OUT) Pointer to array of actual lengths of array elements. Each element in alenp is the length of the data in the corresponding element in the bind value array before and after the execute.
		(ub2) 0,  				// rcodep      (OUT) Pointer to array of column level return codes. This parameter is ignored for dynamic binds.
		maxarr_len,				// maxarr_len   (IN) The maximum possible number of elements of type dty in a PL/SQL binds.
		curelen,				// curelep  (IN/OUT) A pointer to the actual number of elements. This parameter is only required for PL/SQL binds.
		OCI_DEFAULT				// mode         (IN) Similar to the mode in the OCIEnvCreate() call, but this one has higher priority because it can override the "naturally" inherited mode setting.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_bind_by_pos(OCIStmt* stmtp, OCIDefine** defnpp, OCIError* errhp, ub4 position, ub2 dty, dvoid* valuep, sb4 value_sz, sb2* indp /*=0*/)
{
	return OCIDefineByPos(
		stmtp,					// svchp    (IN/OUT) Service context handle.
		defnpp,					// defnpp   (IN/OUT) A pointer to a pointer to a define handle. If this parameter is passed as NULL, this call implicitly allocates the define handle. In the case of a redefine, a non-NULL handle can be passed in this parameter. This handle is used to store the define information for this column.. 
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		position,				// position     (IN) The position of this value in the select list. Positions are 1-based and are numbered from left to right.
		valuep,					// valuep   (IN/OUT) The pointer to a data value or an array of data values of the type specified in the dty parameter.
		value_sz,				// value_sz     (IN) The size in bytes of the data value pointed to by dvoid pointer valuep.
		dty,					// dty          (IN) The datatype of the value(s) being bound.
		(dvoid *) indp,			// indp     (IN/OUT) Pointer to an indicator variable or array.
		(ub2 *) 0,				// rlenp    (IN/OUT) Pointer to array of length of data fetched. Each element in rlenp is the length of the data (in bytes, unless the data in valuep is in Unicode, when it is in codepoints) in the corresponding element in the row after the fetch.
		(ub2 *) 0,  			// rcodep      (OUT) Pointer to array of column-level return codes.
		(ub4)OCI_DEFAULT
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_execute(OCIStmt* stmtp, OCISvcCtx* svchp, OCIError* errhp, ub4 iters)
{
	return OCIStmtExecute(
		svchp,					// svchp    (IN/OUT) Service context handle.
		stmtp,					// stmtp    (IN/OUT) The statement handle to the SQL or PL/SQL statement being processed.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		iters,					// iters        (IN) For non-SELECT statements, the number of times this statement is executed is equal to iters - rowoff.
		(ub4) 0,				// rowoff       (IN) The starting index from which the data in an array bind is relevant for this multiple row execution.
		(OCISnapshot *) 0,		// snap_in      (IN) This parameter is optional. If it is supplied, it must point to a snapshot descriptor of type OCI_DTYPE_SNAP.
		(OCISnapshot *) 0,		// snap_out    (OUT) This parameter is optional. If it is supplied, it must point to a descriptor of type OCI_DTYPE_SNAP.
		OCI_DEFAULT				// mode         (IN) Similar to the mode in the OCIEnvCreate() call, but this one has higher priority because it can override the "naturally" inherited mode setting.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_fetch(OCIStmt* stmtp, OCIError* errhp, ub4 nrows /*= 1*/)
{
	return OCIStmtFetch(
		stmtp,					// stmtp       (IN/OUT) An statement handle.
		errhp,					// errhp       (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		nrows,					// nrows           (IN) Number of rows to be fetched from the current position.
		OCI_FETCH_NEXT,			// orientation     (IN)	Prior to release 9.0, the only acceptable value is OCI_FETCH_NEXT, which is also the default value. 
		(ub4) OCI_DEFAULT		// mode            (IN)	Pass as OCI_DEFAULT.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_statement_set_prefetch_count(OCIStmt* stmtp, OCIError* errhp, ub4 nrows)
{
	return OCIAttrSet(
		stmtp,					// trgthndlp  (IN/OUT) Pointer to a handle type whose attribute gets modified.
		(ub4)OCI_HTYPE_STMT,	// trghndltyp (IN/OUT) The handle type.
		(dvoid*) &nrows,		// attributep     (IN) Pointer to an attribute value. The attribute value is copied into the target handle. If the attribute value is a pointer, then only the pointer is copied, not the contents of the pointer. String attributes must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		0,						// size           (IN) The size of an attribute value. This can be passed in as 0 for most attributes as the size is already known by the OCI library. For text* attributes, a ub4 must be passed in set to the length of the string in bytes, regardless of encoding.
		OCI_ATTR_PREFETCH_ROWS,	// attrtype       (IN) The type of attribute being set.
		errhp					// errhp      (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_lob_descriptor_allocate(OCIEnv* envhp, OCILobLocator** locpp)
{
	return OCIDescriptorAlloc(
		envhp,
		(dvoid**) locpp,
		(ub4)OCI_DTYPE_LOB,
		(size_t) 0,
		(dvoid**) 0
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_lob_descriptor_free(OCILobLocator* locp)
{
	return OCIDescriptorFree(
		(dvoid**) locp,
		(ub4) OCI_DTYPE_LOB
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_create_temporary_blob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp)
{
	return OCILobCreateTemporary(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) A locator that points to the temporary LOB. You must allocate the locator using OCIDescriptorAlloc() before passing it to this function.
		OCI_DEFAULT,			// csid         (IN) The LOB character set ID. For Oracle8i or later, pass as OCI_DEFAULT.
		SQLCS_IMPLICIT,			// csfrm        (IN) The LOB character set form of the buffer data. 
		OCI_TEMP_BLOB,			// lobtype      (IN) The type of LOB to create. Valid values include: OCI_TEMP_BLOB For a temporary BLOB or OCI_TEMP_CLOB For a temporary CLOB or NCLOB
		OCI_ATTR_NOCACHE,		// cache        (IN) Pass TRUE if the temporary LOB should be read into the cache; pass FALSE if it should not. The default is FALSE for NOCACHE functionality.
		OCI_DURATION_SESSION	// duration     (IN) The duration of the temporary LOB. The following are valid values: OCI_DURATION_SESSION or OCI_DURATION_CALL.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_create_temporary_clob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp)
{
	return OCILobCreateTemporary(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) A locator that points to the temporary LOB. You must allocate the locator using OCIDescriptorAlloc() before passing it to this function.
		OCI_DEFAULT,			// csid         (IN) The LOB character set ID. For Oracle8i or later, pass as OCI_DEFAULT.
		SQLCS_IMPLICIT,			// csfrm        (IN) The LOB character set form of the buffer data. 
		OCI_TEMP_CLOB,			// lobtype      (IN) The type of LOB to create. Valid values include: OCI_TEMP_BLOB For a temporary BLOB or OCI_TEMP_CLOB For a temporary CLOB or NCLOB
		OCI_ATTR_NOCACHE,		// cache        (IN) Pass TRUE if the temporary LOB should be read into the cache; pass FALSE if it should not. The default is FALSE for NOCACHE functionality.
		OCI_DURATION_SESSION	// duration     (IN) The duration of the temporary LOB. The following are valid values: OCI_DURATION_SESSION or OCI_DURATION_CALL.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_free_temporary_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp)
{
	return OCILobFreeTemporary(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp					// locp     (IN/OUT) A locator uniquely referencing the LOB to be freed.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_open_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp)
{
	return OCILobOpen(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) A locator uniquely referencing the LOB to be freed.
		OCI_LOB_READONLY		// ???
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_close_lob(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp)
{
	return OCILobClose(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp					// locp     (IN/OUT) A locator uniquely referencing the LOB to be freed.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_lob_gen_length(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator* locp, long* length)
{
	sword status = 0;
	ub4 l = 0;

	status = OCILobGetLength(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) A locator uniquely referencing the LOB to be freed.
		(ub4 *) &l				// ???
		);

	*length = static_cast<long>(l);

	return status;
}
	
///////////////////////////////////////////////////////////////////////////
sword oci_blob_read(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen)
{
	memset(bufp, 0, buflen);

	return OCILobRead(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) An internal LOB locator that uniquely references the LOB. This locator must have been a locator that was obtained from the server specified by svchp.
		amtp,					// amtp     (IN/OUT) The value in amtp is the amount in either bytes or characters.
		offset,					// offset       (IN) On input, this is the absolute offset from the beginning of the LOB value. For character LOBs (CLOBs, NCLOBs) it is the number of characters from the beginning of the LOB, for binary LOBs or BFILEs it is the number of bytes. The first position is 1.
		bufp,					// bufp     (IN/OUT) The pointer to a buffer into which the piece is read. The length of the allocated memory is assumed to be bufl.
		buflen,					// buflen       (IN) The length of the buffer in octets. This value differs from the amtp value for CLOBs and for NCLOBs (csfrm=SQLCS_NCHAR) when the amtp parameter is specified in terms of characters, and the bufl parameter is specified in terms of bytes.
		0,						// ctxp         (IN) The context for the callback function. Can be NULL.
		0,						// cbfp         (IN) A callback that can be registered to be called for each piece. If this is NULL, then OCI_NEED_DATA is returned for each piece.
		0,						// csid         (IN) The character set ID of the data in the buffer. If this value is 0, then csid is set to the client's NLS_LANG or NLS_CHAR value, depending on the value of csfrm.
		SQLCS_IMPLICIT			// csfrm        (IN) The character set form of the buffer data. The csfrm parameter must be consistent with the type of the LOB.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_blob_write(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub1 piece)
{
	return OCILobWrite(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) An internal LOB locator that uniquely references the LOB. This locator must have been a locator that was obtained from the server specified by svchp.
		amtp,					// amtp     (IN/OUT) The value in amtp is the amount in either bytes or characters.
		offset,					// offset       (IN) On input, it is the absolute offset from the beginning of the LOB value. For character LOBs, it is the number of characters from the beginning of the LOB; for binary LOBs, it is the number of bytes. The first position is 1.
		bufp,					// bufp         (IN) The pointer to a buffer from which the piece is written. The length of the data in the buffer is assumed to be the value passed in buflen. Even if the data is being written in pieces using the polling method, bufp must contain the first piece of the LOB when this call is invoked. If a callback is provided, bufp must not be used to provide data or an error results.
		buflen,					// buflen       (IN) The length, in bytes, of the data in the buffer. This value differs from the amtp value for CLOBs and NCLOBs when the amtp parameter is specified in terms of characters, and the buflen parameter is specified in terms of bytes.
		piece,					// piece        (IN) Which piece of the buffer is being written. The default value for this parameter is OCI_ONE_PIECE, indicating that the buffer is written in a single piece. The following other values are also possible for piecewise or callback mode: OCI_FIRST_PIECE, OCI_NEXT_PIECE, and OCI_LAST_PIECE.
		0,						// ctxp         (IN) The context for the callback function. Can be NULL.
		0,						// cbfp         (IN) A callback that can be registered to be called for each piece in a piecewise write. If this is NULL, the standard polling method is used.
		0,						// csid         (IN) The character set ID of the data in the buffer. If this value is 0, then csid is set to the client's NLS_LANG or NLS_CHAR value, depending on the value of csfrm.
		SQLCS_IMPLICIT			// csfrm        (IN) The character set form of the buffer data. The csfrm parameter must be consistent with the type of the LOB.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_clob_read(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub2 csid)
{
	memset(bufp, 0, buflen);

	return OCILobRead(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) An internal LOB locator that uniquely references the LOB. This locator must have been a locator that was obtained from the server specified by svchp.
		amtp,					// amtp     (IN/OUT) The value in amtp is the amount in either bytes or characters.
		offset,					// offset       (IN) On input, this is the absolute offset from the beginning of the LOB value. For character LOBs (CLOBs, NCLOBs) it is the number of characters from the beginning of the LOB, for binary LOBs or BFILEs it is the number of bytes. The first position is 1.
		bufp,					// bufp     (IN/OUT) The pointer to a buffer into which the piece is read. The length of the allocated memory is assumed to be bufl.
		buflen,					// buflen       (IN) The length of the buffer in octets. This value differs from the amtp value for CLOBs and for NCLOBs (csfrm=SQLCS_NCHAR) when the amtp parameter is specified in terms of characters, and the bufl parameter is specified in terms of bytes.
		0,						// ctxp         (IN) The context for the callback function. Can be NULL.
		0,						// cbfp         (IN) A callback that can be registered to be called for each piece. If this is NULL, then OCI_NEED_DATA is returned for each piece.
		csid,					// csid         (IN) The character set ID of the data in the buffer. If this value is 0, then csid is set to the client's NLS_LANG or NLS_CHAR value, depending on the value of csfrm.
		SQLCS_IMPLICIT			// csfrm        (IN) The character set form of the buffer data. The csfrm parameter must be consistent with the type of the LOB.
		);
}

///////////////////////////////////////////////////////////////////////////
sword oci_clob_write(OCISvcCtx* svchp, OCIError* errhp, OCILobLocator *locp, ub4 *amtp, ub4 offset, void *bufp, ub4 buflen, ub1 piece, ub2 csid)
{
	return OCILobWrite(
		svchp,					// svchp    (IN/OUT) Service context handle.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		locp,					// locp     (IN/OUT) An internal LOB locator that uniquely references the LOB. This locator must have been a locator that was obtained from the server specified by svchp.
		amtp,					// amtp     (IN/OUT) The value in amtp is the amount in either bytes or characters.
		offset,					// offset       (IN) On input, it is the absolute offset from the beginning of the LOB value. For character LOBs, it is the number of characters from the beginning of the LOB; for binary LOBs, it is the number of bytes. The first position is 1.
		bufp,					// bufp         (IN) The pointer to a buffer from which the piece is written. The length of the data in the buffer is assumed to be the value passed in buflen. Even if the data is being written in pieces using the polling method, bufp must contain the first piece of the LOB when this call is invoked. If a callback is provided, bufp must not be used to provide data or an error results.
		buflen,					// buflen       (IN) The length, in bytes, of the data in the buffer. This value differs from the amtp value for CLOBs and NCLOBs when the amtp parameter is specified in terms of characters, and the buflen parameter is specified in terms of bytes.
		piece,					// piece        (IN) Which piece of the buffer is being written. The default value for this parameter is OCI_ONE_PIECE, indicating that the buffer is written in a single piece. The following other values are also possible for piecewise or callback mode: OCI_FIRST_PIECE, OCI_NEXT_PIECE, and OCI_LAST_PIECE.
		0,						// ctxp         (IN) The context for the callback function. Can be NULL.
		0,						// cbfp         (IN) A callback that can be registered to be called for each piece in a piecewise write. If this is NULL, the standard polling method is used.
		csid,					// csid         (IN) The character set ID of the data in the buffer. If this value is 0, then csid is set to the client's NLS_LANG or NLS_CHAR value, depending on the value of csfrm.
		SQLCS_IMPLICIT			// csfrm        (IN) The character set form of the buffer data. The csfrm parameter must be consistent with the type of the LOB.
		);
}
