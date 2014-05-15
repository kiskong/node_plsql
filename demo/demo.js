/* jshint node: true */

var node_plsql = require('../lib/node_plsql');

var config = {
	server: {
		port: 8999,
		static: [{
			mountPath: '/',
			physicalDirectory: __dirname + '/static'
		}]
	},
	services: [{
		route: 'demo',
		oracleUsername: 'node_plsql',
		oraclePassword: 'node_plsql',
		oracleHostname: 'localhost',
		oraclePort: 1521,
		oracleDatabase: 'TEST'
	}]
};

// Start the NODE.JS PL/SQL Server
node_plsql.start(config);
