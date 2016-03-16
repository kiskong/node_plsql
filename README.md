[![Downloads][downloads-image]][npm-url] [![Build Status][travis-image]][travis-url] [![Coverage Status](https://coveralls.io/repos/doberkofler/node_plsql/badge.svg)](https://coveralls.io/r/doberkofler/node_plsql)

# The Node.js PL/SQL Gateway for Oracle
The Node.js PL/SQL Server is a bridge between an Oracle database and a Node.js web server.
It is an open-source alternative to mod_plsql, the Embedded PL/SQL Gateway and ORDS,
allowing you to develop PL/SQL web applications using the PL/SQL Web Toolkit (OWA) and Oracle Application Express (Apex),
and serve the content using a Node.js HTTP Server (express).

Please feel free to try and suggest any improvements. Your thoughts and ideas are most welcome.

# Installation

## Prerequisites
There are several prerequisites needed to both compile and run the Oracle database driver.
Please visit the [node-oracledb INSTALL.md](https://github.com/oracle/node-oracledb/blob/master/INSTALL.md) page for more information.

## Installing node_plsql locally
* Prepare and install the prerequisites (Python, C++ compiler, Oracle, ...)
* Setup the environment (set the needed environment variables)
* Create and move to a new directory
* Install node_plsql (`npm install node_plsql`)
* The node_plsql client can be run using `node_modules/.bin/node_plsql` on MacOS/Linux or `node_modules\.bin\node_plsql` on Windows
* Run node_plsql `node_modules/.bin/node_plsql --init=sample.json` to create a new sample configuration file.
* Change the sample JSON configuration file (sample.json) as needed and especially make sure that the Oracle configuration information is correct.
* Install the PL/SQL examples using SQL*Plus and running `install.sql` in the `./node_modules/node_plsql/demo` directory while connected as a user with administrative privileges.
* Run `node_modules/.bin/node_plsql --config=sample.json` to start the server.
* Invoke a browser and open the page `http://localhost:8999/demo/demo.pageIndex`.

## Installing node_plsql globally
* Prepare and install the prerequisites (Python, C++ compiler, Oracle, ...)
* Setup the environment (set the needed environment variables)
* Create and move to a new directory
* Install node_plsql (`npm install node_plsql --global`)
* Use `npm list --global` to see if and where the node_plsql module has been installed
* The node_plsql client can be run using `node_plsql`
* Run node_plsql `node_plsql --init=sample.json` to create a new sample configuration file.
* Change the sample JSON configuration file (sample.json) as needed and especially make sure that the Oracle configuration information is correct.
* Install the PL/SQL examples using SQL*Plus and running `install.sql` from the sql directory (in the node_plsql module of the global npm repository) as user sys.
* Run `node_plsql --config=sample.json` to start the server.
* Invoke a browser and open the page `http://localhost:8999/demo/demo.pageIndex`.

# Configuration

## How does a mod_plsql DAD configuration compare to the node_plsql configuration file

```
<Location /pls/sample>
  SetHandler                    pls_handler
  Order                         deny,allow
  Allow                         from all
  PlsqlDatabaseUsername         scott
  PlsqlDatabasePassword         tiger
  PlsqlDatabaseConnectString    ORCL
  PlsqlAuthenticationMode       Basic
  PlsqlDefaultPage              demo.pageindex
  PlsqlDocumentTablename        doctable
  PlsqlErrorStyle               DebugStyle
  PlsqlNlsLanguage              AMERICAN_AMERICA.UTF8
</Location>
```

```json
{
  "port": 8999,
  "static": [{
      "mountPath": "/",
      "physicalDirectory": "./static"
    }
  ],
  "requestLogging": true,
  },
  "services": [{
    "route": "sample",
    "defaultPage": "demo.pageindex",
    "databaseUsername": "scott",
    "databasePassword": "tiger",
    "databaseConnectString": "ORCL",
    "documentTableName": "doctable"
  }]
}
```

# Documentation
This README is currently the only available documentation.

# Missing features

## Features in mod_plsql that are not yet available in node_plsql:
- Support for APEX 5.
- Default exclusion list.
- Basic and custom authentication methods, based on the OWA_SEC package and custom packages.
- Caching of PL/SQL-generated web pages, based on the OWA_CACHE package.
- Override CGI environment variables in DAD configuration file.

## Support matrix of mod_plsql configuration options:
- PlsqlDMSEnable -> no
- PlsqlLogEnable -> no
- PlsqlLogDirectory -> no
- PlsqlIdleSessionCleanupInterval -> no

- PlsqlAfterProcedure -> no
- PlsqlAlwaysDescribeProcedure -> no
- PlsqlAuthenticationMode -> not yet
- PlsqlBeforeProcedure -> no
- PlsqlBindBucketLengths -> no
- PlsqlBindBucketWidths -> no
- PlsqlCGIEnvironmentList -> no
- PlsqlCompatibilityMode -> no
- PlsqlConnectionTimeout -> no
- PlsqlConnectionValidation -> no
- PlsqlDatabaseConnectString -> yes
- PlsqlDatabasePassword -> yes
- PlsqlDatabaseUserName -> yes
- PlsqlDefaultPage -> yes
- PlsqlDocumentPath -> not yet
- PlsqlDocumentProcedure -> not yet
- PlsqlDocumentTablename -> yes
- PlsqlErrorStyle -> no
- PlsqlExclusionList -> not yet
- PlsqlFetchBufferSize -> no
- PlsqlInfoLogging -> no
- PlsqlMaxRequestsPerSession -> no
- PlsqlNLSLanguage -> not yet
- PlsqlPathAlias -> not yet
- PlsqlPathAliasProcedure -> not yet
- PlsqlRequestValidationFunction -> not yet
- PlsqlSessionCookieName -> no
- PlsqlSessionStateManagement -> no
- PlsqlTransferMode -> no
- PlsqlUploadAsLongRaw -> no

- PlsqlCacheCleanupTime -> no
- PlsqlCacheDirectory -> no
- PlsqlCacheEnable -> no
- PlsqlCacheMaxAge -> no
- PlsqlCacheMaxSize -> no
- PlsqlCacheTotalSize -> no

# Release History
See the [changelog](https://github.com/doberkofler/node_plsql/blob/master/CHANGELOG.md).

[downloads-image]: http://img.shields.io/npm/dm/node_plsql.svg
[npm-url]: https://npmjs.org/package/node_plsql

[travis-url]: http://travis-ci.org/doberkofler/node_plsql
[travis-image]: https://travis-ci.org/doberkofler/node_plsql.svg?branch=master
