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
		route: 'sample',
		username: 'sample',
		password: 'sample',
		database: ''
	}]
};

// Start the NODE.JS PL/SQL Server
node_plsql.start(config);
