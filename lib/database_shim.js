'use strict';

/**
* Module dependencies.
*/

var debug = require('debug')('node_plsql:database');
var underscore = require('underscore');

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
	var databaseHandle = {
		callbacks: config.callbacks
	};

	debug('connect');

	if (databaseHandle.callbacks && databaseHandle.callbacks.databaseConnect && underscore.isFunction(databaseHandle.callbacks.databaseConnect)) {
		databaseHandle.callbacks.databaseConnect(config);
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
	debug('invoke');

	if (databaseHandle.callbacks && databaseHandle.callbacks.databaseInvoke && underscore.isFunction(databaseHandle.callbacks.databaseInvoke)) {
		databaseHandle.callbacks.databaseInvoke(databaseHandle, username, password, procedure, args, cgi, files, doctablename, callback);
	}
}

module.exports = {
	connect: connect,
	invoke: invoke
};
