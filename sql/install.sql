-- create user
@create_user.sql

-- change schema
ALTER SESSION SET CURRENT_SCHEMA=plsql_server;

-- install code
@plsql_server_util.pks
show errors
@plsql_server_util.pkb
show errors

-- install demo
@demo.pks
show errors
@demo.pkb
show errors
