/* jshint node: true */

var node_plsql = require('../lib/node_plsql');

var config = {
	server: {
		port: 8999,
		static: [{
			mountPath: '/',
			physicalDirectory: __dirname + '/static'
		}],
		suppressOutput: false,
		requestLogging: true,
		oracleConnectionPool: true,
		oracleDebug: false
	},
	services: [{
		route: 'sample',
		defaultPage: '',
		databaseUsername: 'sample',
		databasePassword: 'sample',
		databaseConnectString: '',
		documentTableName: 'doctable'
	}]
};

// Start the NODE.JS PL/SQL Server
node_plsql.start(config);
