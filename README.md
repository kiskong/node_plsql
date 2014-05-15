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
* Adapt the demo/server.js file and make sure that the Oracle configuration is correct.
* Run `node demo/server.js` in the root of the source directory.
* Invoke a browser and open the page http://localhost:8999/demo/demo.pageIndex.

#Development

##Debugging
* Set the environment variable DEBUG to * before starting node.

## Unit test
* Run `npm install` and `npm test` in the root of the source directory

# Documentation
This README and the [release goals](https://github.com/doberkofler/node_plsql/blob/master/RELEASE-GOALS.md) are currently the only available documentation.

# Release History
See the [change log](https://github.com/doberkofler/node_plsql/blob/master/CHANGELOG.md).
