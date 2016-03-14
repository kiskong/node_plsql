'use strict';

/**
 * @fileoverview Test for the module "oracledb_shim.js"
 * @author doberkofler
 */


/* global describe: false, it:false, before:false, after:false */


/**
* Module dependencies.
*/

//const debug = require('debug')('node_plsql:oracledb_shim:test');
/*eslint-disable no-unused-vars, camelcase */
const assert = require('chai').assert;
const _ = require('underscore');
const oracledb = require('../lib/oracledb_shim');
/*eslint-enable no-unused-vars, camelcase */


/**
* Module constants.
*/


/**
* Module variables.
*/

/**
* Tests.
*/

describe('oracle_shim.js', function () {

	describe('createPool', function () {
		it('works with proper arguments', function (done) {
			oracledb.createPool({user: 'user', password: 'password', connectString: 'connectString'}, function (err, pool) {
				assert.isNull(err);
				assert.isObject(pool);
				pool.terminate(function (err2) {
					assert.isNull(err2);
					done();
				});
			});
		});

		it('createPool throws an exception with invalid arguments', function () {
			assert.throws(function () {
				oracledb.createPool({password: 'password', connectString: 'connectString'}, function () {});
			});
		});

		it('terminate throws an exception with invalid arguments', function () {
			assert.throws(function () {
				oracledb.createPool({user: 'user', password: 'password', connectString: 'connectString'}, function (err, pool) {
					pool.terminate();
				});
			});
		});
	});

	describe('getConnection', function () {
		let connectionPool;

		before('Create a connection pool', function (done) {
			oracledb.createPool({user: 'user', password: 'password', connectString: 'connectString'}, function (err, pool) {
				connectionPool = pool;
				done();
			});
		});

		after('Close the connection pool', function (done) {
			connectionPool.terminate(function () {
				connectionPool = null;
				done();
			});
		});

		it('works with proper arguments', function (done) {
			connectionPool.getConnection(function (err, connection) {
				assert.isNull(err);
				assert.isObject(connection);
				connection.release(function (err2) {
					assert.isNull(err2);
					done();
				});
			});
		});

		it('getConnection throws an exception with invalid arguments', function () {
			assert.throws(function () {
				connectionPool.getConnection();
			});
		});

		it('release throws an exception with invalid arguments', function () {
			assert.throws(function () {
				connectionPool.getConnection(function (err, connection) {
					connection.release();
				});
			});
		});

	});

	describe('execute', function () {
		let connectionPool,
			connection;

		before('Create a connection', function (done) {
			oracledb.createPool({user: 'user', password: 'password', connectString: 'connectString'}, function (err, pool) {
				connectionPool = pool;
				connectionPool.getConnection(function (err2, conn) {
					connection = conn;
					done();
				});
			});
		});

		after('Close the connection', function (done) {
			connection.release(function () {
				connection = null;
				connectionPool.terminate(function () {
					connectionPool = null;
					done();
				});
			});
		});

		it('works with proper arguments', function (done) {
			connection.execute('sql', {}, function (err, result) {
				assert.isNull(err);
				assert.deepEqual(result, {});
				done();
			});
		});

		it('throws an exception with invalid arguments', function () {
			assert.throws(function () {
				connection.execute('sql', function () {});
			});
		});

	});

	describe('registerExecuteCallback', function () {
		let connectionPool,
			connection;

		before('Create a connection', function (done) {
			oracledb.createPool({user: 'user', password: 'password', connectString: 'connectString'}, function (err, pool) {
				pool.registerExecuteCallback(function registerExecuteCallback(sql, bind, result) {
					result = {hello: 'world'};
					return result;
				});
				connectionPool = pool;
				connectionPool.getConnection(function (err2, conn) {
					connection = conn;
					done();
				});
			});
		});

		after('Close the connection', function (done) {
			connection.release(function () {
				connection = null;
				connectionPool.registerExecuteCallback(null);
				connectionPool.terminate(function () {
					connectionPool = null;
					done();
				});
			});
		});

		it('works with proper arguments', function (done) {
			connection.execute('sql', {}, function (err, result) {
				assert.isNull(err);
				assert.deepEqual(result, {hello: 'world'});
				done();
			});
		});

		it('throws an exception with invalid arguments', function () {
			assert.throws(function () {
				connectionPool.registerExecuteCallback();
			});
		});
	});

});
