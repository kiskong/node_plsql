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

var TEST_CONFIGURATION_FILENAME = '_mocha.json';


/**
* Module variables.
*/


/*
* Return a valid configuration object
*/
function getValidConf() {
	'use strict';

	return {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: path.join(__dirname, 'static')
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
}


/**
* Tests.
*/
describe('config', function () {
	'use strict';

	describe('when validating a configuration', function () {
		it('is a valid configuration', function () {
			var conf = getValidConf(),
				Undefined;

			assert.strictEqual(config.validate(conf), Undefined);
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

		it('is an invalid configuration because we miss a documentTableName', function () {
			var conf = getValidConf();

			conf.services[0].documentTableName = '';
			assert.strictEqual(config.validate(conf), 'Configuration object must contain a "documentTableName" for each service!');
		});

		it('is an invalid configuration because not all services have the same databaseConnectString', function () {
			var conf = getValidConf();

			conf.services[0].databaseConnectString = 'abc';
			assert.strictEqual(config.validate(conf), 'Configuration object is only allowed to containt a single database!');
		});
	});

	describe('configuration file', function () {

		it('should load no configuration file', function () {
			var obj = config.load();
			assert.isObject(obj);
		});

		it('should create and load a sample configuration file', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			config.createSample(TEST_CONFIGURATION_FILENAME);

			var obj = config.load(TEST_CONFIGURATION_FILENAME);
			assert.isObject(obj);
		});

		it('should throw an error when trying to load an invalid configuration file', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			fs.writeFileSync(TEST_CONFIGURATION_FILENAME, '[this is no valid jason or yaml file}');

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
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);

			var original = config.setSampleFilename('this_does_not_exist_for_sure');

			assert.throws(function () {
				config.createSample(TEST_CONFIGURATION_FILENAME);
				config.setSampleFilename(original);
			});
		});

	});

});
