/**
* Module dependencies.
*/

'use strict';

var debug = require('debug')('node_plsql:database_oracle');
// var oracleBindings = require('../build/Release/oracleBindings');
var binary = require('node-pre-gyp');
var path = require('path');
var bindingPath = binary.find(path.resolve(path.join(__dirname, '../package.json')));
var oracleBindings = require(bindingPath);
var log = require('./log');


/**
* Module variables.
*/

/**
* Connect with Oracle
*
* @param {Object} config Configuration object
* @return {Object} Database handle
* @api public
*/
function connect(config) {
	var databaseHandle = {callback: config.callback},
		Undefined,
		oracleConfig,
		text;

	// If we have no services, just return
	if (config.services.length <= 0) {
		return Undefined;
	}

	// Oracle configuration
	oracleConfig = {
		username: config.services[0].databaseUsername || '',
		password: config.services[0].databasePassword || '',
		database: config.services[0].databaseConnectString || '',
		oracleConnectionPool: config.server.oracleConnectionPool,
		oracleDebug: config.server.oracleDebug
	};

	// Show connect message
	text = 'Connect with Oracle as ' + oracleConfig.username;
	if (oracleConfig.database.length > 0) {
		text += '@' + oracleConfig.database;
	}
	if (debug.enabled) {
		debug(text);
	} else {
		console.log(text);
	}

	// Create Oracle binding object
	databaseHandle.oracle = new oracleBindings.OracleBindings(oracleConfig);

	// Initialize Oracle and create the connection pool
	try {
		databaseHandle.oracle.create();
	} catch (e) {
		log.error('Error when trying to create the connection pool', e);
		/*eslint-disable no-process-exit */
		process.exit(1);
		/*eslint-enable no-process-exit */
	}

	return databaseHandle;
}

/**
* Invoke the PL/SQL code and execute the callback when done
*
* @param {Object} databaseHandle Database handle object.
* @param {String} username Oracle username.
* @param {String} password Oracle password.
* @param {String} procedure PL/SQL procedure to execute.
* @param {Object} args Object with the arguments for the PL/SQL procedure as properties.
* @param {Array} cgi Array of cgi variables to send for the PL/SQL code.
* @param {Array} files Array of files to upload.
* @param {String} doctablename Document table name.
* @param {Function} callback Callback function (function cb(err, page)) to invoke when done.
* @api public
*/
function invoke(databaseHandle, username, password, procedure, args, cgi, files, doctablename, callback) {
	// trace
	log.trace('B) A STORED PROCEDURE WILL BE INVOKED (database.invoke)', {username: username, procedure: procedure, args: args, cgi: cgi, files: files, doctablename: doctablename});

	// Invoke the procedure and return the page contents
	try {
		databaseHandle.oracle.request(username, password, procedure, args, cgi, files, doctablename, callback);
	} catch (e) {
		log.error(e);
	}
}

module.exports = {
	connect: connect,
	invoke: invoke
};
