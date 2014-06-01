/**
 * @fileoverview Test for the module "config.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


/**
* Module dependencies.
*/

var fs = require('fs');
var path = require('path');
var assert = require('chai').assert;
var config = require('../lib/config');
var mkdirp = require('mkdirp');


/**
* Module constants.
*/

var TEST_CONFIGURATION_FILENAME = '_mocha.json';


/**
* Module variables.
*/


/**
* File copy
*
* @param {String} from Source file.
* @param {String} to Destination file.
* @api private
*/
function fileCopy(from, to)
{
	'use strict';

	var content = fs.readFileSync(from);
	fs.writeFileSync(to, content);
}

/**
* File delete
*
* @param {String} filename Filename.
* @api private
*/
function fileDelete(filename)
{
	'use strict';

	if (fs.existsSync(filename)) {
		fs.unlinkSync(filename);
	}
}

/**
* Make the given filename absolute
*
* @param {String} filename Relative filename.
* @return {String} Absolute file path relative to the directory where this script resides.
* @api private
*/
function absoluteFilename(filename)
{
	'use strict';

	return path.resolve(path.join(__dirname, filename));
}

/**
* Copy the sample configuration file
*
* @param {String} filename Relative filename.
* @return {String} Absolute file path relative to the directory where this script resides.
* @api private
*/
function absoluteFilename(filename)
{
	'use strict';

	return path.resolve(path.join(__dirname, filename));
}


/**
* Tests.
*/
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

	describe('when creating a new configuration file', function () {
		var SAMPLE_FILE_SRC = absoluteFilename('../../conf/sample.json'),
			SAMPLE_FILE_DST = absoluteFilename('../conf/sample.json');

		// Delete the sample configuration file
		if (fs.existsSync(SAMPLE_FILE_DST)) {
			fs.unlinkSync(SAMPLE_FILE_DST);
		}

		// Copy the sample file
		mkdirp(path.resolve(path.resolve(path.join(__dirname, '../conf'))));
		fileCopy(SAMPLE_FILE_SRC, SAMPLE_FILE_DST);

		it('should create and load a sample configuration file', function () {
			fileDelete(TEST_CONFIGURATION_FILENAME);

			config.createSample(TEST_CONFIGURATION_FILENAME);

			var obj = config.load(TEST_CONFIGURATION_FILENAME);
			assert.isObject(obj);

			fileDelete(TEST_CONFIGURATION_FILENAME);
		});

		it('should throw an error when trying to load an invalid configuration file', function () {
			fileDelete(TEST_CONFIGURATION_FILENAME);

			fs.writeFileSync(TEST_CONFIGURATION_FILENAME, '[this is no valid jason or yaml file}');

			assert.throws(function () {
				config.load(TEST_CONFIGURATION_FILENAME);
			});

			fileDelete(TEST_CONFIGURATION_FILENAME);
		});

		it('should throw an error when trying to create a sample configuration file without name', function () {
			fileDelete(TEST_CONFIGURATION_FILENAME);

			assert.throws(function () {
				config.createSample('undefined_folder/test.json');
			});

			fileDelete(TEST_CONFIGURATION_FILENAME);
		});

		it('should not be able to find the sample configuration file', function () {
			fileDelete(TEST_CONFIGURATION_FILENAME);

			if (fs.existsSync(SAMPLE_FILE_DST)) {
				fs.unlinkSync(SAMPLE_FILE_DST);
			}

			assert.throws(function () {
				config.createSample(TEST_CONFIGURATION_FILENAME);
			});

			fileDelete(TEST_CONFIGURATION_FILENAME);
		});

	});

});
