/**
 * @fileoverview Test for the module "dump.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var dump = require('../lib/dump');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/
describe('dump', function () {
	'use strict';

	describe('when calling dump.divider()', function () {
		it('should work with defaults', function () {
			assert.strictEqual(dump.divider(), '----------------------------------------------------------------------');
		});
		it('should work with 1 parameter', function () {
			assert.strictEqual(dump.divider(10), '----------');
		});
		it('should work with 2 parameters', function () {
			assert.strictEqual(dump.divider(10, '*'), '**********');
		});
	});

	describe('when calling dump.block()', function () {
		it('it should work with 2 parameters', function () {
			assert.strictEqual(dump.block('title', 'content'), '-title begin----------------------------------------------------------\ncontent\n-title end------------------------------------------------------------');
		});
	});

	describe('when calling dump.debug()', function () {
		it('it should at least be there and not break', function () {
			// Emulate the debug module
			var debug = function () {};
			debug.enable = function () {};
			debug.disable = function () {};
			debug.enabled = true;

			dump.debug('title', 'text\ntext', debug);
		});
	});

});
