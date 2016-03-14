'use strict';

/**
 * @fileoverview Test for the module "oracle.js"
 * @author doberkofler
 */


/* global describe: false, it:false, before: false, after: false */


/**
* Module dependencies.
*/

//const debug = require('debug')('node_plsql:oracle:test');
const assert = require('chai').assert;
const _ = require('underscore');
const oracle = require('../lib/oracle');


/**
* Module constants.
*/


/**
* Module variables.
*/

const BIND = {
	p1: {dir: oracle.BIND_IN, type: oracle.NUMBER, val: 0},
	p2: {dir: oracle.BIND_IN, type: oracle.STRING, val: 'string'}
};

function _executeCallback(sql, bind, result) {
	assert.strictEqual(typeof sql, 'string');
	assert.deepEqual(bind, BIND, 'binds');
	assert.deepEqual(result, {}, 'result');

	return {page: sql};
}

/**
* Tests.
*/

describe('oracle.js', function () {

	describe('Connection pool', function () {
		let connectionPool;

		before('Create a connection pool', function (done) {
			oracle.createConnectionPool('user', 'password', 'connectString').then(function (pool) {
				connectionPool = pool;
				done();
			});
		});

		after('Close the connection pool', function (done) {
			oracle.destroyConnectionPool(connectionPool).then(function () {
				connectionPool = null;
				done();
			});
		});

		describe('Open connection', function () {
			it('should return the new connection', function (done) {
				assert.isTrue(_.isObject(connectionPool));
				oracle.openConnection(connectionPool).then(function (conn) {
					assert.isTrue(_.isObject(conn));
					oracle.closeConnection(conn);
					done();
				});
			});
		});
	});

	describe('Connection', function () {
		let connectionPool,
			connection;

		before('Create a connection', function (done) {
			oracle.createConnectionPool('user', 'password', 'connectString').then(function (pool) {
				connectionPool = pool;
				oracle.openConnection(connectionPool).then(function (conn) {
					connection = conn;
					done();
				});
			});
		});

		after('Close the connection', function (done) {
			oracle.closeConnection(connection).then(function () {
				connection = null;
				oracle.destroyConnectionPool(connectionPool).then(function () {
					connectionPool = null;
					done();
				});
			});
		});

		describe('Connections', function () {
			it('should be available', function () {
				assert.isTrue(_.isObject(connectionPool));
				assert.isTrue(_.isObject(connection));
			});
		});

		it('should return the server version', function () {
			assert.strictEqual(oracle.getServerVersion(connection), '0.0.0');
		});
	});

	describe('Execute', function () {
		let connectionPool,
			connection;

		before('Create a connection', function (done) {
			oracle.createConnectionPool('user', 'password', 'connectString').then(function (pool) {
				connectionPool = pool;
				oracle.openConnection(connectionPool).then(function (conn) {
					connection = conn;
					done();
				});
			});
		});

		after('Close the connection', function (done) {
			connectionPool.registerExecuteCallback(null);

			oracle.closeConnection(connection).then(function () {
				connection = null;
				oracle.destroyConnectionPool(connectionPool).then(function () {
					connectionPool = null;
					done();
				});
			});
		});

		describe('Execute', function () {
			it('should return an empty object as result', function (done) {
				oracle.execute(connection, 'sql', {}).then(function (results) {
					assert.deepEqual(results, {});
					done();
				});
			});
		});

		describe('Execute with registerExecuteCallback', function () {
			it('should return the modified object as result', function (done) {
				connectionPool.registerExecuteCallback(_executeCallback);
				oracle.execute(connection, 'SELECT * FROM DUAL', BIND).then(function (result) {
					assert.deepEqual(result, {page: 'SELECT * FROM DUAL'});
					done();
				});
			});
		});
	});

	describe('versionAsString', function () {
		const TESTS = [
			{version: '', result: null},
			{version: null, result: null},
			{version: 0, result: '0.0.0'},
			{version: 10203, result: '1.2.3'},
			{version: 102030405, result: '1.2.3.4.5'}
		];

		it('should return correct string', function () {
			let i;

			for (i = 0; i < TESTS.length; i++) {
				assert.strictEqual(oracle.versionAsString(TESTS[i].version), TESTS[i].result, 'test #' + i);
			}
		});
	});

});
