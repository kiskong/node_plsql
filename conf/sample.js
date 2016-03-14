'use strict';

const server = require('../lib/node_plsql');
const path = require('path');

const config = {
	server: {
		port: 8999,
		static: [{
			mountPath: '/',
			physicalDirectory: path.join(__dirname, '/static')
		}],
		suppressOutput: false,
		requestLogging: true,
		requestTracing: false
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
server.start(config);
