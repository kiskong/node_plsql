/**
 * @fileoverview Test for the module "config.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


//------------------------------------------------------------------------------
// Requirements
//------------------------------------------------------------------------------

var fs = require('fs');
var assert = require('chai').assert;
var config = require('../lib/config');


//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
describe('config', function () {
	'use strict';

	var filePath = './test.json';

	if (fs.existsSync(filePath)) {
		fs.unlinkSync(filePath);
	}

	describe('when calling config.createSampleConfig()', function () {
		it('we should find a sample configuration file', function () {
			config.createSampleConfig(filePath);
			var obj = config.loadConfig(filePath);
			assert.isObject(obj);
		});
	});

	if (fs.existsSync(filePath)) {
		fs.unlinkSync(filePath);
	}

});
