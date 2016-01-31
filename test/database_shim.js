/**
 * @fileoverview Test for the module "databaseShim.js"
 * @author doberkofler
 */


'use strict';


/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var databaseShim = require('../lib/database_shim');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/
describe('databaseShim', function () {
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

			databaseShim.connect(CONFIG);
			assert.strictEqual(connectRun, true);

			databaseShim.invoke(CONFIG);
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

			databaseShim.connect(CONFIG);
			assert.strictEqual(connectRun, false);

			databaseShim.invoke(CONFIG);
			assert.strictEqual(invokeRun, false);
		});

	});

});
