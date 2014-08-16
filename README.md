[![Build Status](https://travis-ci.org/doberkofler/node_plsql.svg?branch=master)](http://travis-ci.org/doberkofler/node_plsql)

# Node PL/SQL Server: The Node.js PL/SQL Gateway for Oracle
The Node.js PL/SQL Server is a bridge between an Oracle database and a Node.js web server.
It is an open-source alternative to mod_plsql, the Embedded PL/SQL Gateway and the APEX Listener,
allowing you to develop PL/SQL web applications using the PL/SQL Web Toolkit (OWA) and Oracle Application Express (Apex),
and serve the content using a Node.js HTTP Server (express).

Please feel free to try and suggest any improvements. Your thoughts and ideas are most welcome.

# Installation

IMPORTANT: Make sure that you read the [installation guide](https://github.com/doberkofler/node_plsql/blob/master/INSTALL.md) before trying to install!

Prerequisites:
* Python 2.7 (not v3.x), needed by node-gyp
* C++ Compiler toolchain (GCC, Visual Studio or similar)
* Oracle Instant Client and SDK
* Special environment variables
* Special symlinks and libaio on Linux

See the [installation guide](https://github.com/doberkofler/node_plsql/blob/master/INSTALL.md) on how to install this package.

# Example

## Basic example
* Install the PL/SQL examples using SQL*Plus and running install.sql from the sql directory as user sys.
* Run `node_plsql --init=sample.json` to create a new sample configuration file.
* Change the sample JSON configuration file (sample.json) as needed and especially make sure that the Oracle configuration information is correct.
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

#Development

##Debugging
* Set the environment variable DEBUG to the module to debug or * for all modules before starting the server.

Windows:
```bat
set DEBUG=*
```

MacOS/Linux:
```bash
export DEBUG=*
```

## Unit test
* Run `npm install` and `npm test` in the root of the source directory

# Documentation
This README and the [release goals](https://github.com/doberkofler/node_plsql/blob/master/RELEASE-GOALS.md) are currently the only available documentation.

# Release History
See the [history](https://github.com/doberkofler/node_plsql/blob/master/HISTORY.md).
