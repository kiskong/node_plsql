#ifndef OCI_ORACLE_DUMMY_H
#define OCI_ORACLE_DUMMY_H

// types
typedef unsigned char  ub1;
typedef   signed char  sb1;
typedef unsigned short ub2;
typedef   signed short sb2;
typedef unsigned int   ub4;
typedef   signed int   sb4;
typedef unsigned char  oratext;
typedef          char  eb1;
typedef          short eb2;
typedef          int   eb4;
typedef oratext        text;
typedef oratext        OraText;
typedef          int   eword;
typedef unsigned int   uword;
typedef   signed int   sword;
typedef unsigned short utext;

// defines
#undef CONST
#define CONST const
#define dvoid void
#define boolean bool
#define OCIDuration int

// handles
typedef struct {int d;} OCISession;
typedef struct {int d;} OCIBind;
typedef struct {int d;} OCIDescribe;
typedef struct {int d;} OCIDefine;
typedef struct {int d;} OCIEnv;
typedef struct {int d;} OCIError;
typedef struct {int d;} OCICPool;
typedef struct {int d;} OCISPool;
typedef struct {int d;} OCIAuthInfo;
typedef struct {int d;} OCILob;
typedef struct {int d;} OCILobLength;
typedef struct {int d;} OCILobMode;
typedef struct {int d;} OCILobOffset;
typedef struct {int d;} OCILobLocator;
typedef struct {int d;} OCIBlobLocator;
typedef struct {int d;} OCIClobLocator;
typedef struct {int d;} OCILobRegion;
typedef struct {int d;} OCIBFileLocator;
typedef struct {int d;} OCIParam;
typedef struct {int d;} OCIResult;
typedef struct {int d;} OCISnapshot;
typedef struct {int d;} OCIServer;
typedef struct {int d;} OCIStmt;
typedef struct {int d;} OCISvcCtx;
typedef struct {int d;} OCITrans;

#define OCI_HTYPE_FIRST          1
#define OCI_HTYPE_ENV            1
#define OCI_HTYPE_ERROR          2
#define OCI_HTYPE_SVCCTX         3
#define OCI_HTYPE_STMT           4
#define OCI_HTYPE_BIND           5
#define OCI_HTYPE_DEFINE         6
#define OCI_HTYPE_DESCRIBE       7
#define OCI_HTYPE_SERVER         8
#define OCI_HTYPE_SESSION        9
#define OCI_HTYPE_AUTHINFO       OCI_HTYPE_SESSION

// Error Return Values
#define OCI_SUCCESS 0
#define OCI_SUCCESS_WITH_INFO 1
#define OCI_RESERVED_FOR_INT_USE 200
#define OCI_NO_DATA 100
#define OCI_ERROR -1
#define OCI_INVALID_HANDLE -2
#define OCI_NEED_DATA 99
#define OCI_STILL_EXECUTING -3123

// input data types
#define SQLT_CHR  1                        /* (ORANET TYPE) character string */
#define SQLT_NUM  2                          /* (ORANET TYPE) oracle numeric */
#define SQLT_INT  3                                 /* (ORANET TYPE) integer */
#define SQLT_FLT  4                   /* (ORANET TYPE) Floating point number */
#define SQLT_STR  5                                /* zero terminated string */
#define SQLT_VNU  6                        /* NUM with preceding length byte */
#define SQLT_PDN  7                  /* (ORANET TYPE) Packed Decimal Numeric */
#define SQLT_LNG  8                                                  /* long */
#define SQLT_VCS  9                             /* Variable character string */
#define SQLT_NON  10                      /* Null/empty PCC Descriptor entry */
#define SQLT_RID  11                                                /* rowid */
#define SQLT_DAT  12                                /* date in oracle format */
#define SQLT_VBI  15                                 /* binary in VCS format */
#define SQLT_BFLOAT 21                                /* Native Binary float*/
#define SQLT_BDOUBLE 22                             /* NAtive binary double */
#define SQLT_BIN  23                                  /* binary data(DTYBIN) */
#define SQLT_LBI  24                                          /* long binary */
#define SQLT_UIN  68                                     /* unsigned integer */
#define SQLT_SLS  91                        /* Display sign leading separate */
#define SQLT_LVC  94                                  /* Longer longs (char) */
#define SQLT_LVB  95                                   /* Longer long binary */
#define SQLT_AFC  96                                      /* Ansi fixed char */
#define SQLT_AVC  97                                        /* Ansi Var char */
#define SQLT_IBFLOAT  100                          /* binary float canonical */
#define SQLT_IBDOUBLE 101                         /* binary double canonical */
#define SQLT_CUR  102                                        /* cursor  type */
#define SQLT_RDD  104                                    /* rowid descriptor */
#define SQLT_LAB  105                                          /* label type */
#define SQLT_OSL  106                                        /* oslabel type */
#define SQLT_NTY  108                                   /* named object type */
#define SQLT_REF  110                                            /* ref type */
#define SQLT_CLOB 112                                       /* character lob */
#define SQLT_BLOB 113                                          /* binary lob */
#define SQLT_BFILEE 114                                   /* binary file lob */
#define SQLT_CFILEE 115                                /* character file lob */
#define SQLT_RSET 116                                     /* result set type */
#define SQLT_NCO  122      /* named collection type (varray or nested table) */
#define SQLT_VST  155                                      /* OCIString type */
#define SQLT_ODT  156                                        /* OCIDate type */
#define SQLT_DATE                      184                      /* ANSI Date */
#define SQLT_TIME                      185                           /* TIME */
#define SQLT_TIME_TZ                   186            /* TIME WITH TIME ZONE */
#define SQLT_TIMESTAMP                 187                      /* TIMESTAMP */
#define SQLT_TIMESTAMP_TZ              188       /* TIMESTAMP WITH TIME ZONE */
#define SQLT_INTERVAL_YM               189         /* INTERVAL YEAR TO MONTH */
#define SQLT_INTERVAL_DS               190         /* INTERVAL DAY TO SECOND */
#define SQLT_TIMESTAMP_LTZ             232        /* TIMESTAMP WITH LOCAL TZ */

// Attribute Types
#define OCI_ATTR_SESSION 0
#define OCI_ATTR_PREFETCH_ROWS 0
#define OCI_ATTR_USERNAME 0
#define OCI_ATTR_PASSWORD 0

// more
#define OCI_DEFAULT 0
#define OCI_SYSDBA 0
#define OCI_UTF16ID 0
#define SQLCS_IMPLICIT 0
#define OCI_LOB_READONLY 0
#define OCI_ATTR_SERVER 0
#define OCI_CRED_RDBMS 0
#define OCI_THREADED 0
#define OCI_NTV_SYNTAX 0
#define OCI_FETCH_NEXT 0
#define OCI_DTYPE_LOB 0
#define OCI_TEMP_BLOB 0
#define OCI_ATTR_NOCACHE 0
#define OCI_DURATION_SESSION 0
#define OCI_TEMP_CLOB 0

// functions
typedef sb4 (*OCICallbackLobRead)(void  *ctxp, const void  *bufp, ub4 len, ub1 piece);
typedef sb4 (*OCICallbackLobWrite)(void  *ctxp, void  *bufp, ub4 *lenp, ub1 *piece);

inline void OCIClientVersion(sword *major_version,
                      sword *minor_version,
                      sword *update_num,
                      sword *patch_num,
                      sword *port_update_num) {}
inline sword   OCIErrorGet   (void  *hndlp, ub4 recordno, OraText *sqlstate,
                       sb4 *errcodep, OraText *bufp, ub4 bufsiz, ub4 type) {return OCI_SUCCESS;}
inline sword   OCIEnvNlsCreate (OCIEnv **envp, ub4 mode, void  *ctxp,
                 void  *(*malocfp)(void  *ctxp, size_t size),
                 void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
                 void   (*mfreefp)(void  *ctxp, void  *memptr),
                 size_t xtramem_sz, void  **usrmempp,
                 ub2 charset, ub2 ncharset) {return OCI_SUCCESS;}
inline sword   OCIFEnvCreate (OCIEnv **envp, ub4 mode, void  *ctxp,
                 void  *(*malocfp)(void  *ctxp, size_t size),
                 void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
                 void   (*mfreefp)(void  *ctxp, void  *memptr),
                 size_t xtramem_sz, void  **usrmempp, void  *fupg) {return OCI_SUCCESS;}
inline sword   OCIHandleAlloc(const void  *parenth, void  **hndlpp, const ub4 type, 
                       const size_t xtramem_sz, void  **usrmempp) {return OCI_SUCCESS;}
inline sword   OCIHandleFree(void  *hndlp, const ub4 type) {return OCI_SUCCESS;}
inline sword   OCIAttrGet (const void  *trgthndlp, ub4 trghndltyp, 
                    void  *attributep, ub4 *sizep, ub4 attrtype, 
                    OCIError *errhp) {return OCI_SUCCESS;}
inline sword   OCIAttrSet (void  *trgthndlp, ub4 trghndltyp, void  *attributep,
                    ub4 size, ub4 attrtype, OCIError *errhp) {return OCI_SUCCESS;}
inline sword   OCIStmtPrepare   (OCIStmt *stmtp, OCIError *errhp, const OraText *stmt,
                          ub4 stmt_len, ub4 language, ub4 mode) {return OCI_SUCCESS;}
inline sword   OCIBindByName   (OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
                         const OraText *placeholder, sb4 placeh_len, 
                         void  *valuep, sb4 value_sz, ub2 dty, 
                         void  *indp, ub2 *alenp, ub2 *rcodep, 
                         ub4 maxarr_len, ub4 *curelep, ub4 mode) {return OCI_SUCCESS;}
inline sword   OCIDefineByPos  (OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,
                         ub4 position, void  *valuep, sb4 value_sz, ub2 dty,
                         void  *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode) {return OCI_SUCCESS;}
inline sword   OCIStmtExecute  (OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp, 
                         ub4 iters, ub4 rowoff, const OCISnapshot *snap_in, 
                         OCISnapshot *snap_out, ub4 mode) {return OCI_SUCCESS;}
inline sword   OCIStmtFetch   (OCIStmt *stmtp, OCIError *errhp, ub4 nrows, 
                        ub2 orientation, ub4 mode) {return OCI_SUCCESS;}
inline sword   OCIDescriptorAlloc(const void  *parenth, void  **descpp, 
                           const ub4 type, const size_t xtramem_sz, 
                           void  **usrmempp) {return OCI_SUCCESS;}
inline sword   OCIDescriptorFree(void  *descp, const ub4 type) {return OCI_SUCCESS;}
inline sword OCILobCreateTemporary(OCISvcCtx          *svchp,
                            OCIError           *errhp,
                            OCILobLocator      *locp,
                            ub2                 csid,
                            ub1                 csfrm,
                            ub1                 lobtype,
                            boolean             cache,
                            OCIDuration         duration) {return OCI_SUCCESS;}
inline sword OCILobFreeTemporary(OCISvcCtx          *svchp,
                          OCIError           *errhp,
                          OCILobLocator      *locp) {return OCI_SUCCESS;}
inline sword   OCILobOpen( OCISvcCtx        *svchp,
                   OCIError         *errhp,
                   OCILobLocator    *locp,
                   ub1               mode ) {return OCI_SUCCESS;}
inline sword OCILobClose( OCISvcCtx        *svchp,
                   OCIError         *errhp,
                   OCILobLocator    *locp ) {return OCI_SUCCESS;}
inline sword   OCILobGetLength  (OCISvcCtx *svchp, OCIError *errhp, 
                          OCILobLocator *locp,
                          ub4 *lenp) {return OCI_SUCCESS;}
inline sword   OCILobRead  (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                     ub4 *amtp, ub4 offset, void  *bufp, ub4 bufl, void  *ctxp,
                     OCICallbackLobRead cbfp, ub2 csid, ub1 csfrm) {return OCI_SUCCESS;}
inline sword   OCILobWrite  (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                      ub4 *amtp, ub4 offset, void  *bufp, ub4 buflen,
                      ub1 piece,  void  *ctxp, OCICallbackLobWrite cbfp,
                      ub2 csid, ub1 csfrm) {return OCI_SUCCESS;}
inline sword   OCIServerAttach  (OCIServer *srvhp, OCIError *errhp,
                          const OraText *dblink, sb4 dblink_len, ub4 mode) {return OCI_SUCCESS;}

inline sword   OCIServerDetach  (OCIServer *srvhp, OCIError *errhp, ub4 mode) {return OCI_SUCCESS;}

inline sword   OCISessionBegin  (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp,
                          ub4 credt, ub4 mode) {return OCI_SUCCESS;}

inline sword   OCISessionEnd   (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp, 
                         ub4 mode) {return OCI_SUCCESS;}

#endif // OCI_ORACLE_DUMMY_H
