/* jshint node: true */

/**
* Module dependencies.
*/

var oracleBindings = require('../build/Release/oracleBindings');
var debug = require('debug')('plsql-server:oracle');
var underscore = require('underscore');
var fs = require('fs');

/**
* Module variables.
*/

/**
* Install procedural code
*
* @api private
*/
function installCode(oracle) {
	'use strict';

	var code = '';

	debug('Installing "node_plsql.pks" ...');
	code = fs.readFileSync('sql/node_plsql.pks', 'utf8');
	oracle.execute(code);

	debug('Installing "node_plsql.pkb" ...');
	code = fs.readFileSync('sql/node_plsql.pkb', 'utf8');
	oracle.execute(code);
}

/**
* Create a new oracle connection
*
* @api private
*/
function createOracle(service) {
	'use strict';

	var defaults = {
		username: '',
		password: '',
		sysdba: false,
		hostname: 'localhost',
		port: 1521,
		database: '',
		debug: false
	};

	var oracleConfig = {
		username: service.oracleUsername,
		password: service.oraclePassword,
		sysdba: false,
		hostname: service.oracleHostname,
		port: service.oraclePort,
		database: service.oracleDatabase
	};

	oracleConfig = underscore.extend(defaults, oracleConfig);

	debug('Open Oracle connection for ' + oracleConfig.username + '...');

	// Create Oracle object
	service.oracleSession = new oracleBindings.OracleBindings(oracleConfig);

	// Prepare Oracle object
	service.oracleSession.prepare();

	// Connect with database
	service.oracleSession.connect();

	// Install procedural code
	installCode(service.oracleSession);
}

/**
* Create the needed Oracle connections
*
* @api public
*/
function createNeededOracleConnections(services) {
	'use strict';

	var i = 0,
		s = {};

	for (i = 0; i < services.length; i++) {
		s = services[i];
		createOracle(s);
	}
}

/**
* Invoke the PL/SQL code and return the page content
*
* @api public
*/
var invokeProcedure = function (oracleSession, procedure, args, cgi) {
	'use strict';

	// Invoke the procedure and return the page contents
	var page = oracleSession.request(procedure, args, cgi);

	return page;
};

module.exports = {
	createNeededOracleConnections: createNeededOracleConnections,
	invokeProcedure: invokeProcedure
};
