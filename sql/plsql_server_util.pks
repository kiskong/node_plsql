CREATE OR REPLACE
PACKAGE plsql_server_util
IS
	PROCEDURE get_page(page OUT NOCOPY CLOB);
END plsql_server_util;
/
