'use strict';

/**
 * @fileoverview Test for the module "database.js"
 * @author doberkofler
 */


/* global describe: false, it:false, before: false, after: false */


/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:database:test');
const _ = require('underscore');
const assert = require('chai').assert;
const request = require('supertest');
const server = require('../lib/server');
const oracle = require('../lib/oracle');
const database = require('../lib/database');


/**
* Module constants.
*/

const GETARGS = 'G';
const FIXED = 'F';
const VARIABLE = 'V';
const OTHER = 'O';


/**
* Module variables.
*/

let oracleExecuteHistory = [];


/**
* Tests.
*/

describe('database.js', function () {

	describe('invoke', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		it('should process a request with fixed arguments', function (done) {
			database.invoke(application.options.services[0], 'fixedArguments', {x: 'a', y: ['1', '1']}, {}, [], 'doctablename', function callback(err, page) {
				debug('err  -->', err);
				debug('page -->', page);

				assert.isNull(err);
				assert.strictEqual(page, 'page with fixed arguments');
				done();
			});
		});

		it('should process a request with variableArguments arguments', function () {
			database.invoke(application.options.services[0], '!variableArguments', {x: 'a', y: ['1', '1']}, {}, [], 'doctablename', function callback(err, page) {
				debug('err  -->', err);
				debug('page -->', page);

				assert.isNull(err);
				assert.strictEqual(page, 'page with variable arguments');
			});
		});

		it('should process a request with fixed arguments but use an array because of the PL/SQL definition', function (done) {
			database.invoke(application.options.services[0], 'fixedTableArguments', {x: 'a', y: '123'}, {}, [], 'doctablename', function callback(err, page) {
				debug('err  -->', err);
				debug('page -->', page);

				assert.isNull(err);
				assert.strictEqual(page, 'page with fixed table arguments');
				done();
			});
		});
	});

	describe('invoke throws an exception', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		it('should raise an exception when invoked with the incorrect arguments', function () {
			assert.throws(function () {
				database.invoke(application.options.services[0], 'procedure', {}, {}, [], 'doctablename');
			});
		});

	});

	describe('get default page', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('GET /sampleRoute/fixedArguments?x=a&y=1&y=1', function () {
			it('should invoke a page with fixed arguments', function (done) {
				let test = request(application.expressApplication).get('/sampleRoute/fixedArguments?x=a&y=1&y=1');

				test.expect(200, 'page with fixed arguments', done);
			});
		});

		describe('GET /sampleRoute/!variableArguments?x=a&y=1&y=1', function () {
			it('should invoke a page with variable arguments', function (done) {
				let test = request(application.expressApplication).get('/sampleRoute/!variableArguments?x=a&y=1&y=1');

				test.expect(200, 'page with variable arguments', done);
			});
		});
	});

});

/*
* Start server
*/
function _startServer() {
	const OPTIONS = {
		server: {
			port: 8999,
			suppressOutput: true,
			requestLogging: true
		},
		services: [
			{
				route: 'sampleRoute',
				databaseUsername: 'sampleUsername',
				databasePassword: 'samplePassword',
				databaseConnectString: 'sampleConnectString',
				documentTableName: 'sampleDoctable',
				oracleExecuteCallback: _oracleExecuteCallback
			}
		]
	};

	// Start server
	return server.start(OPTIONS);
}

/*
 * node-oracledb execute shim callback
 */
function _oracleExecuteCallback(sql, binds, result) {
	let operation = _getOperation(sql),
		lastOperation = (oracleExecuteHistory.length > 0) ? oracleExecuteHistory[oracleExecuteHistory.length - 1] : null;

	// save the history
	oracleExecuteHistory.push({operation: operation, sql: sql, binds: binds});

	// delegate
	if (operation === FIXED || operation === VARIABLE) {
		result = _executeProcedureAndGetPage(operation, lastOperation, sql, binds, result);
	} else if (operation === GETARGS) {
		result = _getArguments(operation, lastOperation, sql, binds);
	}

	return result;
}

/*
 * Get the operation (GETARGS, FIXED, VARIABLE, OTHER) we are interested in
 */
function _getOperation(sql) {
	let operation = OTHER;

	// The sql command "dbms_utility.name_resolve" indicates that we are processing a request with fixed arguments
	//	and are retrieving the definition of a procedure
	if (/.*dbms_utility.name_resolve.*/.test(sql)) {
		operation = GETARGS;
	}

	// The sql command "owa.get_page" indicates that we are executing a procedure and retrieving the page
	if (/.*owa.get_page.*/.test(sql)) {
		// if it also includes "(:argnames, :argvalues)" we are processing a request with a variable arguments
		if (/.*(:argnames, :argvalues).*/.test(sql)) {
			operation = VARIABLE;
		} else {
			operation = FIXED;
		}
	}

	return operation;
}

/*
 * execute procedure and retrieve the page
 */
function _executeProcedureAndGetPage(operation, lastOperation, sql, binds, result) {
	if (/.*fixedArguments.*/.test(sql)) {
		debug('_executeProcedureAndGetPage: fixedArguments', binds);

		assert.strictEqual(operation, FIXED, 'current operation must be "FIXED"');
		assert.strictEqual(lastOperation.operation, GETARGS, 'previous operation must be "GETARGS"');

		assert.isTrue(_.has(binds, 'p1'), 'binding must include property "p1"');
		assert.deepEqual(binds.p1, {dir: oracle.BIND_IN, type: oracle.STRING, val: 'a'});

		assert.isTrue(_.has(binds, 'p2'), 'binding must include property "p2"');
		assert.deepEqual(binds.p2, {dir: oracle.BIND_IN, type: oracle.STRING, val: ['1', '1']});

		result = {
			outBinds: {
				irows: 1,
				page: ['page with fixed arguments']
			}
		};
	}

	if (/.*fixedTableArguments.*/.test(sql)) {
		debug('_executeProcedureAndGetPage: fixedTableArguments', binds);

		assert.strictEqual(operation, FIXED, 'current operation must be "FIXED"');
		assert.strictEqual(lastOperation.operation, GETARGS, 'previous operation must be "GETARGS"');

		assert.isTrue(_.has(binds, 'p1'), 'binding must include property "p1"');
		assert.deepEqual(binds.p1, {dir: oracle.BIND_IN, type: oracle.STRING, val: 'a'});

		assert.isTrue(_.has(binds, 'p2'), 'binding must include property "p2"');
		assert.deepEqual(binds.p2, {dir: oracle.BIND_IN, type: oracle.STRING, val: ['123']});

		result = {
			outBinds: {
				irows: 1,
				page: ['page with fixed table arguments']
			}
		};
	}

	if (/.*variableArguments.*/.test(sql)) {
		debug('_executeProcedureAndGetPage: variableArguments');

		assert.strictEqual(operation, VARIABLE);
		assert.isTrue(_.isUndefined(lastOperation) || lastOperation.operation !== GETARGS);

		assert.isTrue(_.has(binds, 'argnames'));
		assert.deepEqual(binds.argnames, {dir: oracle.BIND_IN, type: oracle.STRING, val: ['x', 'y', 'y']});

		assert.isTrue(_.has(binds, 'argvalues'));
		assert.deepEqual(binds.argvalues, {dir: oracle.BIND_IN, type: oracle.STRING, val: ['a', '1', '1']});

		result = {
			outBinds: {
				irows: 1,
				page: ['page with variable arguments']
			}
		};
	}

	return result;
}

/*
 * get the arguments
 */
function _getArguments(operation, lastOperation, sql, binds) {
	debug('_getArguments', sql);

	assert.strictEqual(operation, GETARGS, 'current operation must be "GETARGS"');
	assert.isTrue(_.has(binds, 'name'), 'binds must have a "name" property');

	if (binds.name.val === 'fixedTableArguments') {
		debug('_getArguments: fixedTableArguments');

		assert.deepEqual(binds.name, {dir: oracle.BIND_IN, type: oracle.STRING, val: 'fixedTableArguments'}, 'binds must have a proper "name" property');

		return {
			outBinds: {
				names: ['x', 'y'],
				types: ['VARCHAR2', 'PL/SQL TABLE']
			}
		};
	}
}
