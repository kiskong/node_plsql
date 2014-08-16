/**
 * @fileoverview Test for the module "database_shim.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var database_shim = require('../lib/database_shim');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/
describe('database_shim', function () {
	'use strict';

	describe('callbacks', function () {

		it('invokes callback when available', function () {
			var connectRun = false,
				invokeRun = false,
				CONFIG = {
					callbacks: {
						databaseConnect: function () {
							connectRun = true;
						},
						databaseInvoke: function () {
							invokeRun = true;
						}
					}
				};

			database_shim.connect(CONFIG);
			assert.strictEqual(connectRun, true);

			database_shim.invoke(CONFIG);
			assert.strictEqual(invokeRun, true);
		});

		it('does not invokes callback when not available', function () {
			var connectRun = false,
				invokeRun = false,
				CONFIG = {
					callbacks: {
						databaseConnect: 0
					}
				};

			database_shim.connect(CONFIG);
			assert.strictEqual(connectRun, false);

			database_shim.invoke(CONFIG);
			assert.strictEqual(invokeRun, false);
		});

	});

});
