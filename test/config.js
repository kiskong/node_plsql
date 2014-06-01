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
var path = require('path');
var assert = require('chai').assert;
var config = require('../lib/config');
var mkdirp = require('mkdirp');


function fileCopy(from, to)
{
	'use strict';

	var content = fs.readFileSync(from);
	fs.writeFileSync(to, content);
}


//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
describe('config', function () {
	'use strict';

	describe('when validating a configuration', function () {
		it('is a valid configuration', function () {
			var TEST_CONF = {
				server: {
					port: 8999,
					static: [{
						mountPath: '/',
						physicalDirectory: __dirname + '/static'
					}],
					suppressOutput: true,
					requestLogging: true,
					oracleConnectionPool: true,
					oracleDebug: false
				},
				services: [{
					route: 'sampleRoute',
					defaultPage: 'samplePage',
					databaseUsername: 'sampleUsername',
					databasePassword: 'samplePassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: 'sampleDoctable'
				},
				{
					route: 'secondRoute',
					defaultPage: 'secondPage',
					databaseUsername: 'secondUsername',
					databasePassword: 'secondPassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: 'secondDoctable'
				}],
				callbacks: {
					databaseConnect: function () {},
					databaseInvoke: function () {}
				}
			};

			assert.strictEqual(config.validate(TEST_CONF), undefined);
		});

		it('is an invalid configuration because of the missing port', function () {
			var TEST_CONF = {
				server: {
				},
				services: [{
					route: 'sampleRoute',
					defaultPage: 'samplePage',
					databaseUsername: 'sampleUsername',
					databasePassword: 'samplePassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: 'sampleDoctable'
				}]
			};

			assert.strictEqual(config.validate(TEST_CONF), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because of the port that is not of type number', function () {
			var TEST_CONF = {
				server: {
					port: true,
				},
				services: [{
					route: 'sampleRoute',
					defaultPage: 'samplePage',
					databaseUsername: 'sampleUsername',
					databasePassword: 'samplePassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: 'sampleDoctable'
				}]
			};

			assert.strictEqual(config.validate(TEST_CONF), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because we miss a documentTableName', function () {
			var TEST_CONF = {
				server: {
					port: 8999,
				},
				services: [{
					route: 'sampleRoute',
					defaultPage: 'samplePage',
					databaseUsername: 'sampleUsername',
					databasePassword: 'samplePassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: ''
				}]
			};

			assert.strictEqual(config.validate(TEST_CONF), 'Configuration object must contain a "documentTableName" for each service!');
		});

		it('is an invalid configuration because not all services have the same databaseConnectString', function () {
			var TEST_CONF = {
				server: {
					port: 8999,
				},
				services: [{
					route: 'sampleRoute',
					defaultPage: 'samplePage',
					databaseUsername: 'sampleUsername',
					databasePassword: 'samplePassword',
					databaseConnectString: 'sampleConnectString',
					documentTableName: 'sampleDoctable'
				},
				{
					route: 'secondRoute',
					defaultPage: 'secondPage',
					databaseUsername: 'secondUsername',
					databasePassword: 'secondPassword',
					databaseConnectString: 'otherConnectString',
					documentTableName: 'secondDoctable'
				}]
			};

			assert.strictEqual(config.validate(TEST_CONF), 'Configuration object is only allowed to containt a single database!');
		});
	});

	/*

	describe('when creating a new configuration file', function () {
		var SAMPLE_FILE_SRC = path.resolve(path.join(__dirname, '../../conf/sample.json')),
			SAMPLE_FILE_DST = path.resolve(path.join(__dirname, '../conf/sample.json')),
			FILE_PATH = '_mocha.json';

		// Delete the sample configuration file
		if (fs.existsSync(SAMPLE_FILE_DST)) {
			fs.unlinkSync(SAMPLE_FILE_DST);
		}

		// Copy the sample file
		mkdirp(path.resolve(path.resolve(path.join(__dirname, '../conf'))));
		fileCopy(SAMPLE_FILE_SRC, SAMPLE_FILE_DST);

		// Remove any remaining test files
		if (fs.existsSync(FILE_PATH)) {
			fs.unlinkSync(FILE_PATH);
		}

		it('should create a sample configuration file', function () {
			if (fs.existsSync(FILE_PATH)) {
				fs.unlinkSync(FILE_PATH);
			}

			config.createSample(FILE_PATH);

			var obj = config.load(FILE_PATH);
			assert.isObject(obj);

			if (fs.existsSync(FILE_PATH)) {
				fs.unlinkSync(FILE_PATH);
			}
		});

		it('should not be able to create a sample configuration file without name', function () {
			if (fs.existsSync(FILE_PATH)) {
				fs.unlinkSync(FILE_PATH);
			}

			assert.throws(function () {
				config.createSample('undefined_folder/test.json');
			});

			if (fs.existsSync(FILE_PATH)) {
				fs.unlinkSync(FILE_PATH);
			}
		});

		it('should not be able to find the sample configuration file', function () {
			// Delete the sample configuration file
			if (fs.existsSync(SAMPLE_FILE_DST)) {
				fs.unlinkSync(SAMPLE_FILE_DST);
			}

			assert.throws(function () {
				config.createSample(FILE_PATH);
			});

			if (fs.existsSync(FILE_PATH)) {
				fs.unlinkSync(FILE_PATH);
			}
		});

	});

	*/

});
