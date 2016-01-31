/**
 * @fileoverview Test for the module "log.js"
 * @author doberkofler
 */

'use strict';


/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var fs = require('fs');
var log = require('../lib/log');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/
describe('log', function () {
	describe('enabled', function () {
		it('does enable', function () {
			var original = log.enabled(),
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
			var original = log.enabled();

			log.enable(true);
			log.log();

			log.enable(original);
		});
	});

	describe('error', function () {
		it('does log an error', function () {
			var timestamp = 'timestamp:' + log.getTimestamp(),
				data;

			log.error();
			log.error(timestamp);

			data = fs.readFileSync('error.log', {encoding: 'utf8'});
			assert.ok(data.indexOf(timestamp) !== 0);
		});
	});

});
