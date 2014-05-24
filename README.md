[![Build Status](https://travis-ci.org/doberkofler/node_plsql.svg?branch=master)](http://travis-ci.org/doberkofler/node_plsql)

# Node PL/SQL Server: The Node.js PL/SQL Gateway for Oracle
The Node.js PL/SQL Server is a bridge between an Oracle database and a Node.js web server.
It is an open-source alternative to mod_plsql, the Embedded PL/SQL Gateway and the APEX Listener,
allowing you to develop PL/SQL web applications using the PL/SQL Web Toolkit (OWA) and Oracle Application Express (Apex),
and serve the content using a Node.js HTTP Server (express).

# Installation
See the [installation guide](https://github.com/doberkofler/node_plsql/blob/master/INSTALL.md) on how to install this package.

# Example

## Basic example
* Install the PL/SQL examples using SQL*Plus and running install.sql from the sql directory as user sys.
* Run `node_plsql --init=sample.json` to create a new sample configuration file.
* Change the sample JSON configuration file (sample.json) as needed and especially make sure that the Oracle configuration information is correct.
* Run `node_plsql --config=sample.json` to start the server.
* Invoke a browser and open the page `http://localhost:8999/demo/demo.pageIndex`.

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
See the [change log](https://github.com/doberkofler/node_plsql/blob/master/CHANGELOG.md).
