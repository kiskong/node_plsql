-- create user
@create_user.sql

-- change schema
ALTER SESSION SET CURRENT_SCHEMA=node_plsql;

-- install demo
@demo.pks
show errors
@demo.pkb
show errors
