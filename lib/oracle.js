'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:oracle');
const fs = require('fs');
const util = require('util');
const _ = require('underscore');
const async = require('async');
const Promise = require('es6-promise').Promise;
/* istanbul ignore next */
const oracledb = require(process.env.NODE_ENV === 'test' ? './oracledb_shim' : 'oracledb');
const log = require('./log');

/**
* Module variables.
*/

/**
* Create a connection pool and return a promise.
*	if resolved, the promise will have the connection pool as parameter
*	if rejected, the promise will have the error as parameter
*
* @param {String} user - The username
* @param {String} password - The password
* @param {String} connectString - The connect string
* @return {Object} Promise
*/
function createConnectionPool(user, password, connectString) {
	const OPTIONS = {
		user: user,
		password: password,
		connectString: connectString
	};

	// debug
	debug('createConnectionPool: user="' + user + '" password="' + password + '" connectString="' + connectString + '"');

	// validate arguments
	/* istanbul ignore if */
	if (arguments.length !== 3 || typeof user !== 'string' || typeof password !== 'string' || typeof connectString !== 'string') {
		log.exit(new Error('Invalid arguments'));
	}

	return new Promise(function (resolve, reject) {
		oracledb.createPool(OPTIONS, function (err, pool) {
			/* istanbul ignore if */
			if (err) {
				debug(err);
				reject(new Error(err));
			} else {
				resolve(pool);
			}
		});
	});
}

/**
* Destroy a connection pool and return promise.
*	if resolved, the promise will have no parameter
*	if rejected, the promise will have the error as parameter
*
* @param {Object} pool - A connection pool
* @return {Object} Promise
*/
function destroyConnectionPool(pool) {
	// debug
	debug('destroyConnectionPool');

	// validate arguments
	/* istanbul ignore if */
	if (arguments.length !== 1 || !_isConnectionPool(pool)) {
		log.exit(new Error('Invalid arguments'));
	}

	return new Promise(function (resolve, reject) {
		pool.terminate(function (err) {
			/* istanbul ignore if */
			if (err) {
				debug(err);
				reject(new Error(err));
			} else {
				resolve();
			}
		});
	});
}

/**
* Get a connection and return a promise.
*	if resolved, the promise will have the connection as parameter
*	if rejected, the promise will have the error as parameter
*
* @param {Object} pool - A connection pool
* @return {Object} Promise
*/
function openConnection(pool) {
	// debug
	debug('openConnection');

	// validate arguments
	/* istanbul ignore if */
	if (arguments.length !== 1 || !_isConnectionPool(pool)) {
		log.exit(new Error('Invalid arguments'));
	}

	return new Promise(function (resolve, reject) {
		pool.getConnection(function (err, connection) {
			/* istanbul ignore if */
			if (err) {
				debug(err);
				reject(new Error(err));
			} else {
				resolve(connection);
			}
		});
	});
}

/**
* Close connection and return a promise.
*	if resolved, the promise will have no parameter
*	if rejected, the promise will have the error as parameter
*
* @param {Object} connection - A node-oracledb connection
* @return {Object} Promise
*/
function closeConnection(connection) {
	// debug
	debug('closeConnection');

	// validate arguments
	/* istanbul ignore if */
	if (arguments.length !== 1 || !_isConnection(connection)) {
		log.exit(new Error('Invalid arguments'));
	}

	return new Promise(function (resolve, reject) {
		connection.release(function (err) {
			/* istanbul ignore if */
			if (err) {
				debug(err);
				reject(new Error(err));
			} else {
				resolve();
			}
		});
	});
}

/**
* Execute the code and return a promise.
*	if resolved, the promise will have the results of the operation as parameter
*	if rejected, the promise will have the error as parameter
*
* @param {Object} connection - A node-oracledb connection
* @param {String} sql - A sql statement
* @param {Object} bind - An object containing the bindings for the statement
* @return {Object} Promise
*/
function execute(connection, sql, bind) {
	// debug
	//debug('execute "' + sql + '"', util.inspect(bind, {showHidden: false, depth: null, colors: true}));
	debug('execute');

	// validate arguments
	/* istanbul ignore if */
	if (arguments.length < 3 || arguments.length > 4 || !_isConnection(connection) || typeof sql !== 'string' || !_.isObject(bind)) {
		log.exit(new Error('invalid arguments'));
	}

	return new Promise(function (resolve, reject) {
		connection.execute(sql, bind, function (err, result) {
			/* istanbul ignore if */
			if (err) {
				debug(err);
				reject(new Error(err));
			} else {
				resolve(result);
			}
		});
	});
}

/**
* Return the version of the node-oracledb library
*
* @return {String} Version
*/
function getNodeDriverVersion() {
	return versionAsString(oracledb.version);
}

/**
* Return the version of the oracle client
*
* @return {String} Version
*/
function getClientVersion() {
	return versionAsString(oracledb.oracleClientVersion);
}

/**
* Return the version of the oracle server
*
* @param {Object} connection - A node-oracledb connection
* @return {String} Version
*/
function getServerVersion(connection) {
	// validate arguments
	/* istanbul ignore if */
	if (arguments.length !== 1 || !_isConnection(connection)) {
		log.exit(new Error('Invalid arguments'));
	}

	return versionAsString(connection.oracleServerVersion);
}

/**
* Load and install the scripts in the scripts given by an array of file names and return a promise.
*	if resolved, the promise will have no parameter
*	if rejected, the promise will have the error as parameter
*
* @param {Object} connection - A node-oracledb connection
* @param {Array} scriptFileNames - An array of file names
* @return {Object} Promise
*/
/* istanbul ignore next */
function installScripts(connection, scriptFileNames) {
	let i;

	// debug
	debug('installScripts', util.inspect(scriptFileNames, {showHidden: false, depth: null, colors: true}));

	// validate arguments
	if (arguments.length !== 2 || !_isConnection(connection) || !_.isArray(scriptFileNames)) {
		log.exit(new Error('Invalid arguments'));
	}
	for (i = 0; i < scriptFileNames; i++) {
		if (typeof scriptFileNames[i] !== 'string') {
			log.exit(new Error('Invalid arguments'));
		}
	}

	return new Promise(function (resolve, reject) {

		let sqlScripts = [];

		// Load the scripts
		try {
			for (i = 0; i < scriptFileNames; i++) {
				sqlScripts.push(fs.readFileSync(scriptFileNames[i], 'utf8'));
			}
		} catch (err) {
			debug(err);
			reject(new Error(err));
			return;
		}

		// Execute the scripts
		async.eachSeries(
			sqlScripts,
			function (sql, callback) {
				connection.execute(sql, function (err) {
					callback(err);
				});
			},
			function (err) {
				if (err) {
					debug(err);
					reject(new Error(err));
					return;
				}

				resolve();
			}
		);

		resolve();
	});
}

/**
*	Return a string representation of a version in numeric representation
*	For version a.b.c.d.e, this property gives the number: (100000000 * a) + (1000000 * b) + (10000 * c) + (100 * d) + e)
*	For version a.b.c, this property gives the number: (10000 * a) + (100 * b) + c)
*
* @param {Number} numericVersion - A number representing a 3-part or 5-part Oracle version
* @return {String} The string representation of the given version
*/

/*
 */
function versionAsString(numericVersion) {
	let version = [],
		n;

	if (typeof numericVersion !== 'number') {
		return null;
	}

	// This is the 5-part version
	if (numericVersion > 100000000) {
		n = Math.floor(numericVersion / 100000000);
		numericVersion = numericVersion - n * 100000000;
		version.push(n.toString());

		n = Math.floor(numericVersion / 1000000);
		numericVersion = numericVersion - n * 1000000;
		version.push(n.toString());
	}

	// This is the 3-part version
	n = Math.floor(numericVersion / 10000);
	numericVersion = numericVersion - n * 10000;
	version.push(n.toString());

	n = Math.floor(numericVersion / 100);
	numericVersion = numericVersion - n * 100;
	version.push(n.toString());

	version.push(numericVersion.toString());

	return version.join('.');
}

/*
*	Validate if the given argument is a node-oracledb connection pool object
*/
function _isConnectionPool(pool) {
	return _.isObject(pool) && _.isNumber(pool.connectionsOpen);
}

/*
*	Validate if the given argument is a node-oracledb connection object
*/
function _isConnection(connection) {
	return _.isObject(connection) && _.isNumber(connection.oracleServerVersion);
}

// Initialize the node-oracledb library
oracledb.poolMin = 5;
oracledb.poolMax = 100;
oracledb.poolIncrement = 10;
oracledb.poolTimeout = 60;
oracledb.prefetchRows = 100;
oracledb.stmtCacheSize = 100;

module.exports = {
	BIND_IN: oracledb.BIND_IN,
	BIND_INOUT: oracledb.BIND_INOUT,
	BIND_OUT: oracledb.BIND_OUT,
	STRING: oracledb.STRING,
	NUMBER: oracledb.NUMBER,
	DATE: oracledb.DATE,
	CURSOR: oracledb.CURSOR,
	BUFFER: oracledb.BUFFER,
	CLOB: oracledb.CLOB,
	BLOB: oracledb.BLOB,
	createConnectionPool: createConnectionPool,
	destroyConnectionPool: destroyConnectionPool,
	openConnection: openConnection,
	closeConnection: closeConnection,
	execute: execute,
	installScripts: installScripts,
	getNodeDriverVersion: getNodeDriverVersion,
	getClientVersion: getClientVersion,
	getServerVersion: getServerVersion,
	versionAsString: versionAsString
};
