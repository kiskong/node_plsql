/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs');
var path = require('path');
var stripComments = require('strip-json-comments');
var yaml = require('js-yaml');
var underscore = require('underscore');
var debug = require('debug')('node_plsql:config');

/**
 * Validate configuration object.
 * @param {config} config The configuration object
 * @returns {String} Return undefined if valid or else the error message.
 * @api private
 */
function validateConfig(config) {
	'use strict';

	var i = 0;

	// Validate configuration
	if (!underscore.isNumber(config.server.port) || config.server.port < 1 || config.server.port > 32767) {
		throw new TypeError('Configuration object must containt a numeric property "server.port"');
	}
	if (config.services.length > 1) {
		for (i = 0; i < config.services.length; i++) {
			if (config.services[i].database !== config.services[0].database) {
				throw new TypeError('Configuration object is only allowed to containt a single database!');
			}
			if (!config.services[i].documentTableName || config.services[i].documentTableName.length === 0) {
				throw new TypeError('Configuration object must contain a "documentTableName" for each service!');
			}
		}
	}

	return undefined;
}

/**
 * Load and parse a YAML or JSON configuration object from a file.
 * @param {string} filePath the path to the YAML or JSON config file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function loadConfig(filePath) {
	'use strict';

	var defaults = {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: path.join(process.cwd(), 'static')
			}],
			requestLogging: true,
			oracleConnectionPool: true,
			oracleDebug: false
		},
		services: []
	};
	var config;

	// Load configuration
	if (filePath) {
		try {
			config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
		} catch (e) {
			throw new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message);
		}
	}

	// Merge default options with the actual ones
	var extended = underscore.extend(defaults, config);
	debug('Config:' + JSON.stringify(extended));

	// Validate configuration
	var valid = validateConfig(extended);
	if (valid !== undefined) {
		throw new TypeError(valid);
	}

	return extended;
}

/**
 * Create a new sample configuration file
 * @param {String} outputFileName The name to configuration file.
 */
function createSampleConfig(outputFileName)
{
	'use strict';

	var inputFileName = '',
		config = '';

	// Read the configuration file
	inputFileName = path.join(__dirname, '../conf/sample.json');
	path.resolve(inputFileName, inputFileName);
	try {
		config = fs.readFileSync(inputFileName, 'utf8');
	} catch (e) {
		throw new Error('Could not load the sample configuration file: "' + inputFileName + '"');
	}

	// Write sample configuration file
	outputFileName = path.join(process.cwd(), outputFileName);
	try {
		fs.writeFileSync(outputFileName, config);
	} catch (e) {
		throw new Error('Could not save sample configuration file: "' + outputFileName + '"');
	}
}

module.exports = {
	loadConfig: loadConfig,
	createSampleConfig: createSampleConfig
};
