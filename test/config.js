'use strict';

/**
 * @fileoverview Test for the module "config.js"
 * @author doberkofler
 */

/* global describe: false, it:false */

/**
* Module dependencies.
*/

const fs = require('fs');
const path = require('path');
const assert = require('chai').assert;
const utilities = require('../lib/utilities');
const config = require('../lib/config');

/**
* Module constants.
*/

const TEST_CONFIGURATION_FILENAME = '_mocha.json';

/**
* Module variables.
*/

/**
* Tests.
*/

describe('config.js', function () {

	describe('validate', function () {
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
			let conf = getValidConf();

			delete conf.server.port;
			assert.strictEqual(config.validate(conf), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because the port is invalid', function () {
			let conf = getValidConf();

			conf.server.port = 1.1;
			assert.strictEqual(config.validate(conf), 'Configuration object must containt a numeric property "server.port"');
		});

		it('is an invalid configuration because of an invalid "server.suppressOutput" property', function () {
			let conf = getValidConf();

			conf.server.suppressOutput = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "server.suppressOutput" must be a boolean');
		});

		it('is an invalid "route"', function () {
			let conf = getValidConf();

			conf.services[0].route = '';
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].route" must be a non-empty string');
		});

		it('is an invalid "databaseConnectString"', function () {
			let conf = getValidConf();

			conf.services[0].databaseConnectString = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databaseConnectString" must be a string');
		});

		it('is an invalid "databaseUsername"', function () {
			let conf = getValidConf();

			conf.services[0].databaseUsername = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databaseUsername" must be a string');
		});

		it('is an invalid "databasePassword"', function () {
			let conf = getValidConf();

			conf.services[0].databasePassword = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].databasePassword" must be a string');
		});

		it('is an invalid "defaultPage"', function () {
			let conf = getValidConf();

			conf.services[0].defaultPage = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].defaultPage" must be a string');
		});

		it('is an invalid "documentTableName"', function () {
			let conf = getValidConf();

			conf.services[0].documentTableName = 0;
			assert.strictEqual(config.validate(conf), 'Configuration object property "services[0].documentTableName" must be a string');
		});
	});

	describe('initialize', function () {
		let ANSWERS = {
			port: 8999,
			route: 'sample',
			defaultPage: 'myPage',
			databaseUsername: 'scott',
			databasePassword: 'tiger',
			databaseConnectString: 'localhost:1521/ORCL',
			documentTableName: 'doctable',
			mountPath: '/i/',
			physicalDirectory: './static',
			requestLogging: true,
			requestTracing: false
		};

		it('should create and load a sample configuration file', function () {
			let conf;

			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);
			config.processAnswers(ANSWERS, TEST_CONFIGURATION_FILENAME);
			conf = config.load(TEST_CONFIGURATION_FILENAME);

			assert.isObject(conf);
			assert.strictEqual(conf.server.port, ANSWERS.port);
			assert.strictEqual(conf.server.static[0].mountPath, ANSWERS.mountPath);
			assert.strictEqual(conf.server.static[0].physicalDirectory, ANSWERS.physicalDirectory);
			assert.strictEqual(conf.server.requestLogging, ANSWERS.requestLogging);
			assert.strictEqual(conf.server.requestTracing, ANSWERS.requestTracing);
			assert.strictEqual(conf.services[0].route, ANSWERS.route);
			assert.strictEqual(conf.services[0].defaultPage, ANSWERS.defaultPage);
			assert.strictEqual(conf.services[0].databaseUsername, ANSWERS.databaseUsername);
			assert.strictEqual(conf.services[0].databasePassword, ANSWERS.databasePassword);
			assert.strictEqual(conf.services[0].databaseConnectString, ANSWERS.databaseConnectString);
			assert.strictEqual(conf.services[0].documentTableName, ANSWERS.documentTableName);
		});

		it('should throw an error when trying to create a sample configuration file without name', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);
			assert.throws(function () {
				config.processAnswers(ANSWERS);
			});
		});
	});

	describe('load', function () {
		it('should load the default configuration file', function () {
			utilities.fileDelete(config.defaultConfigFileName());
			fs.writeFileSync(config.defaultConfigFileName(), JSON.stringify(getValidConf()));
			assert.isObject(config.load());
		});

		it('should throw an error when trying to load an invalid configuration file', function () {
			utilities.fileDelete(TEST_CONFIGURATION_FILENAME);
			fs.writeFileSync(TEST_CONFIGURATION_FILENAME, '[this is no valid json or yaml file}');
			assert.throws(function () {
				config.load(TEST_CONFIGURATION_FILENAME);
			});
		});
	});

	/*
	* Return a valid configuration object
	*/
	function getValidConf() {
		return {
			server: {
				port: 8999,
				static: [
					{
						mountPath: '/',
						physicalDirectory: path.join(__dirname, 'static')
					},
					{
						mountPath: '/i/',
						physicalDirectory: path.join(__dirname, 'apex')
					}
				],
				suppressOutput: true,
				requestLogging: true
			},
			services: [
				{
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
				}
			]
		};
	}
});
