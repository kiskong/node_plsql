/**
 * @fileoverview Test for the module "version.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


//------------------------------------------------------------------------------
// Requirements
//------------------------------------------------------------------------------

var assert = require('chai').assert;
var version = require('../lib/version');


//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
describe('version', function () {
	'use strict';

	describe('when calling version.get()', function () {
		it('we get a string', function () {
			assert.typeOf(version.get(), 'string');
			assert.isTrue(version.get().length > 0);
		});
	});

});
