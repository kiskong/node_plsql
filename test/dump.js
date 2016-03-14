'use strict';

/**
 * @fileoverview Test for the module "dump.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

const assert = require('chai').assert;
const dump = require('../lib/dump');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/

describe('dump.js', function () {

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
			let debug = function () {};

			debug.enable = function () {};
			debug.disable = function () {};
			debug.enabled = true;

			dump.debug('title', 'text\ntext', debug);
		});
	});

});
