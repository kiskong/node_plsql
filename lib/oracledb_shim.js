'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:oracledb_shim');
const _ = require('underscore');
const log = require('./log');

/**
* Module variables.
*/

/*
*	Constructor for "Pool"
*/
function Pool() {
	this.connectionsOpen = 0;
	this.executeCallback = null;
}

/*
*	Constructor for "Connection"
*/
function Connection(pool) {
	this.pool = pool;
	this.oracleServerVersion = 0;
}

/*
*	"oracledb.terminate"
*/
Pool.prototype.terminate = function (callback) {
	// debug
	debug('Pool.terminate');

	// validate
	if (arguments.length !== 1 || !_.isFunction(callback)) {
		log.exit(new Error('Pool.terminate: invalid arguments'));
	}

	callback(null, null);
};

/*
*	"Pool.getConnection"
*/
Pool.prototype.getConnection = function (callback) {
	// debug
	debug('Pool.getConnection');

	// validate
	if (arguments.length !== 1 || !_.isFunction(callback)) {
		log.exit(new Error('Pool.getConnection: invalid arguments'));
	}

	callback(null, new Connection(this));
};

/*
*	"Pool.registerExecuteCallback"
*/
Pool.prototype.registerExecuteCallback = function (callback) {
	// debug
	debug('Pool.setExecuteCallback');

	// validate
	if (arguments.length !== 1 || (!_.isUndefined(callback) && !_.isNull(callback) && !_.isFunction(callback))) {
		log.exit(new Error('Connection.registerExecuteCallback: invalid arguments'));
	}

	this.executeCallback = callback;
};

/*
*	"Connection.release"
*/
Connection.prototype.release = function (callback) {
	// debug
	debug('Connection.release');

	// validate
	if (arguments.length !== 1 || !_.isFunction(callback)) {
		log.exit(new Error('Connection.release: invalid arguments'));
	}

	callback(null, null);
};

/*
*	"Connection.execute"
*/
Connection.prototype.execute = function (sql, bind, callback) {
	let result = {};

	// debug
	debug('Connection.execute');

	// validate
	if (arguments.length !== 3 || typeof sql !== 'string' || !_.isObject(bind) || !_.isFunction(callback)) {
		log.exit(new Error('Connection.execute: invalid arguments'));
	}

	if (_.isFunction(this.pool.executeCallback)) {
		result = this.pool.executeCallback(sql, bind, result);
	}

	callback(null, result);
};

/*
*	"oracledb.createPool"
*/
function createPool(options, callback) {
	// debug
	debug('createPool', arguments);

	// validate
	if (arguments.length !== 2 || !_.isObject(options) || !_.isFunction(callback) || typeof options.user !== 'string' || typeof options.password !== 'string' || typeof options.connectString !== 'string') {
		log.exit(new Error('oracledb.createPool: invalid arguments'));
	}

	callback(null, new Pool());
}

module.exports = {
	BIND_IN: 1,
	BIND_INOUT: 2,
	BIND_OUT: 3,

	STRING: 4,
	NUMBER: 5,
	DATE: 6,
	CURSOR: 7,
	BUFFER: 8,
	CLOB: 9,
	BLOB: 10,

	poolMin: 1,
	poolMax: 1,
	poolIncrement: 1,
	poolTimeout: 1,
	prefetchRows: 1,
	stmtCacheSize: 1,

	version: 0,
	oracleClientVersion: 0,

	createPool: createPool
};
