/* jshint node: true */

var listener = require('../lib/listener').createListener;

var config = {
	server: {
		port: 8999,
		static_dir: __dirname + '/static'
	},
	services: [{
		route: 'demo',
		oracleUsername: 'plsql_server',
		oraclePassword: 'plsql_server',
		oracleHostname: 'localhost',
		oraclePort: 1521,
		oracleDatabase: 'TEST'
	}]
};

// Create a PL/SQL listener
listener(config);
