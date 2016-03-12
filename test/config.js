'use strict';

/**
 * @fileoverview Test for the module "config.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

var fs = require('fs');
var path = require('path');
var assert = require('chai').assert;
var utilities = require('../lib/utilities');
var config = require('../lib/config');


/**
* Module constants.
*/

const TEST_CONFIGURATION_FILENAME = '_mocha.json';


/**
* Module variables.
*/


/*
* Return a valid configuration object
*/
function getValidConf() {
	return {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: path.join(__dirname, 'static')
			}],
			suppressOutput: true,
			requestLogging: true
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
		}]
	};
}


/**
* Tests.
*/

describe('config.js', function () {

	describe('when validating a configuration', function () {

		it('is a valid configuration', function () {
			assert.isUndefined(config.validate(getValidConf()));
		});

		it('is not an object', function () {
			assert.strictEqual(config.validate('options'), 'Configuration object must be an object');
		});

		it('has no server object', function () {
			assert.strictEqual(config.validate({}), 'Configuration object must contain an object "server"');
		});

		it('is an invalid configuration because of the missing port', function () {
			var conf = getValidConf();

			delete conf.server.port;
			assert.strictEqual(config.validate(conf), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because of the port that is not of type number', function () {
			var conf = getValidConf();

			conf.server.port = true;
			assert.strictEqual(config.validate(conf), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because of an invalid "server.suppressOutput" property', function () {
			var conf = getValidConf();

			conf.server.suppressOutput = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "server.suppressOutput" must be a boolean');
		});

		it('is an invalid "route"', function () {
			var conf = getValidConf();

			conf.services[0].route = '';
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].route" must be a non-empty string');
		});

		it('is an invalid "databaseConnectString"', function () {
			var conf = getValidConf();

			conf.services[0].databaseConnectString = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databaseConnectString" must be a string');
		});

		it('is an invalid "databaseUsername"', function () {
			var conf = getValidConf();

			conf.services[0].databaseUsername = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databaseUsername" must be a string');
		});

		it('is an invalid "databasePassword"', function () {
			var conf = getValidConf();

			conf.services[0].databasePassword = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databasePassword" must be a string');
		});

		it('is an invalid "defaultPage"', function () {
			var conf = getValidConf();

			conf.services[0].defaultPage = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].defaultPage" must be a string');
		});

		it('is an invalid "documentTableName"', function () {
			var conf = getValidConf();

			conf.services[0].documentTableName = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].documentTableName" must be a string');
		});
	});

	describe('configuration file', function () {

		it('should load no configuration file', function () {
			assert.isObject(config.load());
		});

		it('should create and load a sample configuration file', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);
			config.createSample(TEST_CONFIGURATION_FILENAME);
			assert.isObject(config.load(TEST_CONFIGURATION_FILENAME));
		});

		it('should throw an error when trying to load an invalid configuration file', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			fs.writeFileSync(TEST_CONFIGURATION_FILENAME, '[this is no valid json or yaml file}');

			assert.throws(function () {
				config.load(TEST_CONFIGURATION_FILENAME);
			});
		});

		it('should throw an error when trying to create a sample configuration file without name', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			assert.throws(function () {
				config.createSample('undefined_folder/test.json');
			});
		});

		it('should not be able to find the sample configuration file', function () {
			var original;

			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			original = config.setSampleFilename('this_does_not_exist_for_sure');

			assert.throws(function () {
				config.createSample(TEST_CONFIGURATION_FILENAME);
				config.setSampleFilename(original);
			});
		});

	});

});
