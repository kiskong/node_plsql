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

var path = require('path');
var fs = require('fs');

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
describe('version', function () {
	'use strict';

	describe('when calling version.get()', function () {
		it('we get a string with the version', function () {
			assert.typeOf(version.get(), 'string');
			assert.isTrue(version.get().length > 0);
		});
	});

	describe('when calling version.get() but there is no "package.json" file', function () {
		var filename = path.join(__dirname, '../package.json'),
			tempname = 'temp.json';

		it('we should throw an error', function () {
			fs.renameSync(filename, tempname);
			assert.throws(function () {
				version.get();
			});
			fs.renameSync(tempname, filename);
		});
	});

});
