#ifdef _WIN32
# include <Windows.h>
#else
# include <unistd.h>
#endif

#include <oci.h>

#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <time.h> 

static char			appusername[1024];
static char			apppassword[1024];
static char			username[1024];
static char			password[1024];
static char			database[1024];

static OCIError		*errhp			= 0;
static OCIEnv		*envhp			= 0;
static OCICPool		*poolhp			= 0;
static OCISvcCtx	*svchp			= 0;
static OraText		*poolName		= 0;
static sb4			poolNameLen		= 0;

/* Max, min and increment connections */
static ub4			conMin			= 1;
static ub4			conMax			= 3;
static ub4			conIncr			= 1;
 
/* Local functions */
static double get_time();
static void elapsed_time(const char* text, double start_time);
static void checkerr(OCIError *errhp, sword status);
static sb4 olen(CONST OraText * string);
static void sleep(int milliseconds);
static void get_command_line_arguments(int argc, char* argv[]);
static void usage(const char* app);

/* main */
int main(int argc, char* argv[])
{
	double start;
	sword status;

	printf("START\n");

	get_command_line_arguments(argc, argv);

	start = get_time();
	OCIEnvCreate(&envhp, OCI_THREADED | OCI_OBJECT, (dvoid *)0,  NULL, NULL, NULL, 0, (dvoid *)0);
	elapsed_time("OCIEnvCreate", start);

	start = get_time();
	OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
	OCIHandleAlloc((dvoid *) envhp, (dvoid **) &poolhp, OCI_HTYPE_CPOOL, (size_t) 0, (dvoid **) 0);
	elapsed_time("OCIHandleAlloc", start);
 
	/* CONNECT DIRECTLY TO THE SERVER */
	start = get_time();
	status = OCILogon2(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		&svchp,					// svchp    (IN/OUT) Address of an OCI service context pointer. This is filled with a server and session handle.
		username,				// username     (IN) The user name used to authenticate the session. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		olen(username),			// uname_len    (IN) The length of username, in number of bytes, regardless of the encoding.
		password,				// password     (IN) The user's password. For connection pooling, if this parameter is NULL then OCILogon2() assumes that the logon is for a proxy user. It implicitly creates a proxy connection in such a case, using the pool user to authenticate the proxy user. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		olen(password),			// passwd_len   (IN) The length of password, in number of bytes, regardless of the encoding.
		database,				// dbname       (IN) For the default case, this indicates the connect string to use to connect to the Oracle Database.
		olen(database),			// dbname_len   (IN) The length of dbname. For session pooling and connection pooling, this value is returned by the OCISessionPoolCreate() or OCIConnectionPoolCreate() call respectively.
		OCI_DEFAULT				// mode         (IN) The values accepted are: OCI_DEFAULT, OCI_LOGON2_CPOOL, OCI_LOGON2_SPOOL, OCI_LOGON2_STMTCACHE, OCI_LOGON2_PROXY
		);
	elapsed_time("OCILogon2 to the server", start);
	if (status)
	{
		checkerr(errhp, status);
		exit(1);
	}

	/* DISCONNECT */
	start = get_time();
	checkerr(errhp, OCILogoff((dvoid *) svchp, errhp));
	elapsed_time("OCILogoff from the server", start);

	/* sleep for 1 minute */
	sleep(60000);

	/* CREATE THE CONNECTION POOL */
	start = get_time();
	status = OCIConnectionPoolCreate(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		poolhp,					// poolhp       (IN) An allocated pool handle.
		&poolName,				// poolName    (OUT) The name of the connection pool connected to.
		&poolNameLen,			// poolNameLen (OUT) The length of the string pointed to by poolName.
		database,				// dblink       (IN) Specifies the database (server) to connect to.
		olen(database),			// dblinkLen    (IN) The length of the string pointed to by dblink.
		conMin,					// connMin      (IN) Specifies the minimum number of connections in the connection pool. Valid values are 0 and above.
		conMax,					// connMax      (IN) Specifies the maximum number of connections that can be opened to the database. Once this value is reached, no more connections are opened. Valid values are 1 and above.
		conIncr,				// connIncr     (IN) Allows the application to set the next increment for connections to be opened to the database if the current number of connections are less than connMax. Valid values are 0 and above.
		appusername,			// poolUsername (IN) Connection pooling requires an implicit primary session and this attribute provides a username for that session.
		olen(appusername),		// poolUserLen  (IN) The length of poolUsername.
		apppassword,			// poolPassword (IN) The password for the username poolUsername.
		olen(apppassword),		// poolPassLen  (IN) The length of poolPassword.
		OCI_DEFAULT				// mode         (IN) The modes supported are
		);
	elapsed_time("OCIConnectionPoolCreate", start);
	if (status)
	{
		checkerr(errhp, status);
		exit(1);
	}

	/* CONNECT USING THE CONNECTION POOL */
	start = get_time();
	status = OCILogon2(
		envhp,					// envhp        (IN) A pointer to the environment where the connection pool is to be created.
		errhp,					// errhp    (IN/OUT) An error handle you can pass to OCIErrorGet() for diagnostic information in the event of an error.
		&svchp,					// svchp    (IN/OUT) Address of an OCI service context pointer. This is filled with a server and session handle.
		username,				// username     (IN) The user name used to authenticate the session. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		olen(username),			// uname_len    (IN) The length of username, in number of bytes, regardless of the encoding.
		password,				// password     (IN) The user's password. For connection pooling, if this parameter is NULL then OCILogon2() assumes that the logon is for a proxy user. It implicitly creates a proxy connection in such a case, using the pool user to authenticate the proxy user. Must be in the encoding specified by the charset parameter of a previous call to OCIEnvNlsCreate().
		olen(password),			// passwd_len   (IN) The length of password, in number of bytes, regardless of the encoding.
		poolName,				// dbname       (IN) For the default case, this indicates the connect string to use to connect to the Oracle Database.
		poolNameLen,			// dbname_len   (IN) The length of dbname. For session pooling and connection pooling, this value is returned by the OCISessionPoolCreate() or OCIConnectionPoolCreate() call respectively.
		OCI_LOGON2_CPOOL		// mode         (IN) The values accepted are: OCI_DEFAULT, OCI_LOGON2_CPOOL, OCI_LOGON2_SPOOL, OCI_LOGON2_STMTCACHE, OCI_LOGON2_PROXY
		);
	elapsed_time("OCILogon2 using the connection pool", start);
	if (status)
	{
		checkerr(errhp, status);
		exit(1);
	}

	/* DISCONNECT */
	start = get_time();
	checkerr(errhp, OCILogoff((dvoid *) svchp, errhp));
	elapsed_time("OCILogoff from the connection pool", start);

	/* DESTROY THE CONNECTION POOL */
	start = get_time();
	checkerr(errhp, OCIConnectionPoolDestroy(poolhp, errhp, OCI_DEFAULT));
	elapsed_time("OCIConnectionPoolDestroy", start);

	/* FREE HANDLES */  
	start = get_time();
	checkerr(errhp, OCIHandleFree((dvoid *)poolhp, OCI_HTYPE_CPOOL));
	checkerr(errhp, OCIHandleFree((dvoid *)errhp, OCI_HTYPE_ERROR));
	elapsed_time("OCIHandleFree", start);

	printf("END\n");

	return 0;
} 

/* get_time */
double get_time()
{
	return (double)clock();
}

/* elapsed_time */
void elapsed_time(const char* text, double start_time)
{
	double end_time = get_time();
	double diff = (end_time - start_time) / (double)CLOCKS_PER_SEC;
	printf("%s (%.4lf seconds)\n", text, diff);
}

/* Handle oci error */
void checkerr(OCIError *errhp, sword status)
{
	text errbuf[512];
	sb4 errcode = 0;

	switch (status)
	{
		case OCI_SUCCESS:
			break;
		case OCI_SUCCESS_WITH_INFO:
			printf("Error - OCI_SUCCESS_WITH_INFO\n");
			break;
		case OCI_NEED_DATA:
			printf("Error - OCI_NEED_DATA\n");
			break;
		case OCI_NO_DATA:
			printf("Error - OCI_NODATA\n");
			break;
		case OCI_ERROR:
			OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
			printf("Error - %.*s\n", 512, errbuf);
		break;
		case OCI_INVALID_HANDLE:
			printf("Error - OCI_INVALID_HANDLE\n");
			break;
		case OCI_STILL_EXECUTING:
			printf("Error - OCI_STILL_EXECUTE\n");
			break;
		case OCI_CONTINUE:
			printf("Error - OCI_CONTINUE\n");
			break;
		default:
			break;
	}
}

/* get the command line arguments */
void get_command_line_arguments(int argc, char* argv[])
{
	int i;

	#define eq(a,n) strncmp((a), (n), strlen(n))

	for (i = 1; i < argc; i++)
	{
		if (eq(argv[i], "--") == 0)
		{
			if (eq(argv[i], "--appusername=") == 0)
			{
				strcpy(appusername, argv[i] + strlen("--appusername="));
			}
			else if (eq(argv[i], "--apppassword=") == 0)
			{
				strcpy(apppassword, argv[i] + strlen("--apppassword="));

			}
			else if (eq(argv[i], "--username=") == 0)
			{
				strcpy(username, argv[i] + strlen("--username="));

			}
			else if (eq(argv[i], "--password=") == 0)
			{
				strcpy(password, argv[i] + strlen("--password="));

			}
			else if (eq(argv[i], "--database=") == 0)
			{
				strcpy(database, argv[i] + strlen("--database="));

			}
			else
			{
				printf("-- with no proper argument\n");
				usage(argv[0]);
			}
		}
		else
		{
			printf("argument not starting with --\n");
			usage(argv[0]);
		}
	}
}

/* usage */
void usage(const char* app)
{
	printf("Usage: %s [--appusername<username>] [--apppassword<password>] [--username<username>] [--password<password>] [--database<database>]\n", app);
	exit(1);
}

/* Get the length of a OraText* string */
sb4 olen(CONST OraText * string)
{
	return (sb4) strlen((const signed char *) string);
}

/* sleep */
void sleep(int milliseconds)
{
	printf("Sleeping for (%.4lf seconds)\n", (double)milliseconds / 1000.0);
#ifdef _WIN32
	Sleep(milliseconds);
#else
	usleep(milliseconds * 1000);
#endif
}
