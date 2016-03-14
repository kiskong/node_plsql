'use strict';

/**
 * @fileoverview Test for the module "log.js"
 * @author doberkofler
 */


/* global describe: false, beforeEach:false, it:false */
/*eslint-disable no-console*/

/**
* Module dependencies.
*/

const assert = require('chai').assert;
const fs = require('fs');
const log = require('../lib/log');


/**
* Module constants.
*/

const ERROR_FILE_NAME = 'error.log';
const TRACE_FILE_NAME = 'trace.log';

/**
* Module variables.
*/


/**
* Tests.
*/
describe('log.js', function () {

	describe('trace', function () {
		beforeEach('Cleanup tracing', function () {
			log.tracing(false);
			if (fs.existsSync(TRACE_FILE_NAME)) {
				fs.unlink(TRACE_FILE_NAME);
			}
		});

		it('enable/disable', function () {
			let ok;

			assert.strictEqual(process.env.NODE_PLSQL_TRACE, '0');

			ok = log.tracing(true);
			assert.strictEqual(ok, false);
			assert.strictEqual(process.env.NODE_PLSQL_TRACE, '1');

			ok = log.tracing(false);
			assert.strictEqual(ok, true);
			assert.strictEqual(process.env.NODE_PLSQL_TRACE, '0');
		});

		it('trace when disabled', function () {
			log.trace('string');
			assert.isFalse(fs.existsSync(TRACE_FILE_NAME));
		});

		it('trace', function () {
			const TAG = 'TAG#' + log.getTimestamp();
			let data;

			log.tracing(true);
			log.trace(TAG, {object: 'object'}, ['array']);

			data = fs.readFileSync(TRACE_FILE_NAME, {encoding: 'utf8'});
			assert.ok(data.indexOf(TAG) !== -1);
			assert.ok(data.indexOf('{ object: \'object\' }') !== -1);
			assert.ok(data.indexOf('[ \'array\' ]') !== -1);
		});
	});

	describe('error', function () {
		it('does log an error', function () {
			const CONSOLE_LOG = console.log;
			const TIMESTAMP = 'timestamp:' + log.getTimestamp();
			let data;

			console.log = function () {};
			log.error();
			log.error(TIMESTAMP);
			console.log = CONSOLE_LOG;

			data = fs.readFileSync(ERROR_FILE_NAME, {encoding: 'utf8'});
			assert.ok(data.indexOf(TIMESTAMP) !== 0);
		});

		it('log an Error instance', function () {
			const CONSOLE_LOG = console.log;
			const TIMESTAMP = 'timestamp:' + log.getTimestamp();
			let data;

			console.log = function () {};
			log.error(new Error(TIMESTAMP));
			console.log = CONSOLE_LOG;

			data = fs.readFileSync(ERROR_FILE_NAME, {encoding: 'utf8'});
			assert.ok(data.indexOf(TIMESTAMP) !== 0);
		});

		it('log a string', function () {
			const CONSOLE_LOG = console.log;
			const TIMESTAMP = 'timestamp:' + log.getTimestamp();
			let data;

			console.log = function () {};
			log.error(TIMESTAMP);
			console.log = CONSOLE_LOG;

			data = fs.readFileSync(ERROR_FILE_NAME, {encoding: 'utf8'});
			assert.ok(data.indexOf(TIMESTAMP) !== 0);
		});

		it('log something else', function () {
			const CONSOLE_LOG = console.log;
			const TIMESTAMP = 'timestamp:' + log.getTimestamp();
			let data;

			console.log = function () {};
			log.error({text: TIMESTAMP});
			console.log = CONSOLE_LOG;

			data = fs.readFileSync(ERROR_FILE_NAME, {encoding: 'utf8'});
			assert.ok(data.indexOf(TIMESTAMP) !== 0);
		});
	});

	describe('enabled', function () {
		it('does enable', function () {
			let original = log.enabled(),
				Undefined;

			log.enable(true);
			assert.strictEqual(process.env.NOLOG, Undefined);

			assert.strictEqual(log.enable(false), true);
			assert.strictEqual(process.env.NOLOG, '1');

			assert.strictEqual(log.enable(), false);
			assert.strictEqual(process.env.NOLOG, Undefined);

			log.enable(original);
		});
	});

	describe('log', function () {
		it('does log', function () {
			let original = log.enabled();

			log.enable(true);
			log.log();

			log.enable(original);
		});
	});

});
