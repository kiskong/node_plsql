[![Downloads][downloads-image]][npm-url] [![Build Status][travis-image]][travis-url] [![Coveralls Status][coveralls-image]][coveralls-url]

# Node PL/SQL Server: The Node.js PL/SQL Gateway for Oracle
The Node.js PL/SQL Server is a bridge between an Oracle database and a Node.js web server.
It is an open-source alternative to mod_plsql, the Embedded PL/SQL Gateway and the APEX Listener,
allowing you to develop PL/SQL web applications using the PL/SQL Web Toolkit (OWA) and Oracle Application Express (Apex),
and serve the content using a Node.js HTTP Server (express).

Please feel free to try and suggest any improvements. Your thoughts and ideas are most welcome.

# Installation

IMPORTANT: Make sure that you read the [installation guide](https://github.com/doberkofler/node_plsql/blob/master/INSTALL.md) before trying to install!

Prerequisites:
* Node.js
* Python 2.7 (not v3.x), needed by node-gyp
* C++ Compiler toolchain (GCC, Visual Studio or similar)
* Oracle Instant Client and SDK
* Special environment variables
* Special symlinks and libaio on Linux

See the [installation guide](https://github.com/doberkofler/node_plsql/blob/master/INSTALL.md) on how to install this package.

# Example

## Basic example (when installing node_plsql locally)
* Prepare and install the prerequisites (Python, C++ compiler, Oracle, ...)
* Setup the environment (set the needed environment variables)
* Create and move to a new directory
* Install node_plsql (`npm install node_plsql`)
* The node_plsql client can be run using `node_modules/.bin/node_plsql` on MacOS/Linux or `node_modules\.bin\node_plsql` on Windows
* Run node_plsql `node_modules/.bin/node_plsql --init=sample.json` to create a new sample configuration file.
* Change the sample JSON configuration file (sample.json) as needed and especially make sure that the Oracle configuration information is correct.
* Install the PL/SQL examples using SQL*Plus and running `install.sql` in the `./node_modules/node_plsql/sql` directory while connected as a user with administrative privileges.
* Run `node_modules/.bin/node_plsql --config=sample.json` to start the server.
* Invoke a browser and open the page `http://localhost:8999/demo/demo.pageIndex`.

## Basic example (when installing node_plsql globally)
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
  "server": {
    "port": 8999,
    "static": [{
        "mountPath": "/",
        "physicalDirectory": "./static"
      }
    ],
    "requestLogging": true,
    "oracleConnectionPool": true,
    "oracleDebug": false
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
This README and the [release goals](https://github.com/doberkofler/node_plsql/blob/master/RELEASE-GOALS.md) are currently the only available documentation.

# Release History
See the [history](https://github.com/doberkofler/node_plsql/blob/master/HISTORY.md).

[downloads-image]: http://img.shields.io/npm/dm/node_plsql.svg
[npm-url]: https://npmjs.org/package/node_plsql

[travis-url]: http://travis-ci.org/doberkofler/node_plsql
[travis-image]: https://travis-ci.org/doberkofler/node_plsql.svg?branch=master

[coveralls-url]: https://coveralls.io/r/doberkofler/node_plsql
[coveralls-image]: https://coveralls.io/repos/doberkofler/node_plsql/badge.svg
