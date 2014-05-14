-- create user
@create_user.sql

-- change schema
ALTER SESSION SET CURRENT_SCHEMA=node_plsql;

-- install code
@node_plsql.pks
show errors
@node_plsql.pkb
show errors

-- install demo
@demo.pks
show errors
@demo.pkb
show errors
