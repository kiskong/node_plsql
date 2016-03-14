'use strict';

/**
 * @fileoverview Test for the module "version.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

const assert = require('chai').assert;
const version = require('../lib/version');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/

describe('version.js', function () {

	describe('when calling version.get()', function () {
		it('we get a string with the version', function () {
			assert.typeOf(version.get(), 'string');
			assert.isTrue(version.get().length > 0);
		});
	});

});
