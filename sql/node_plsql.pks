CREATE OR REPLACE
PACKAGE node_plsql
IS
	PROCEDURE get_page(page OUT NOCOPY CLOB);
END node_plsql;
/
