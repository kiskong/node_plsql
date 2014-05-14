-- create user
CREATE USER plsql_server IDENTIFIED BY plsql_server;

-- assign privileges
GRANT create session TO plsql_server;
GRANT unlimited tablespace TO plsql_server;
GRANT create table TO plsql_server;
GRANT create view TO plsql_server;
GRANT create sequence TO plsql_server;
GRANT create procedure TO plsql_server;
GRANT execute on dbms_lob TO plsql_server;
GRANT execute on dbms_output TO plsql_server;
