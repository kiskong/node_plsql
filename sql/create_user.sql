-- create user
CREATE USER node_plsql IDENTIFIED BY node_plsql;

-- assign privileges
GRANT create session TO node_plsql;
GRANT unlimited tablespace TO node_plsql;
GRANT create table TO node_plsql;
GRANT create view TO node_plsql;
GRANT create sequence TO node_plsql;
GRANT create procedure TO node_plsql;
GRANT execute on dbms_lob TO node_plsql;
GRANT execute on dbms_output TO node_plsql;
