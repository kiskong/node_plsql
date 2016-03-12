'use strict';

/**
* Module dependencies.
*/

var debug = require('debug')('node_plsql:config');
var fs = require('fs');
var path = require('path');
var stripComments = require('strip-json-comments');
var yaml = require('js-yaml');
var _ = require('underscore');
var log = require('./log');


/**
* Module constants.
*/


/**
* Module variables.
*/
var SAMPLE_FILENAME = 'sample.json';


/**
 * Validate configuration object.
 * @param {Object} options The configuration object
 * @returns {String} Return undefined if valid or else the error message.
 * @api public
 */
function validate(options) {
	let Undefined;
	var service,
		i = 0;

	debug('validate');

	// Validate "options"
	if (!_.isObject(options)) {
		return 'Configuration object must be an object';
	}

	// Validate "options.server"
	if (!_.isObject(options.server)) {
		return 'Configuration object must contain an object "server"';
	}

	// Validate "options.server.port"
	if (!_.isNumber(options.server.port) || options.server.port < 1 || options.server.port > 32767) {
		return 'Configuration object must containt a numeric property "server.port"';
	}

	// Suppress output
	if (typeof options.server.suppressOutput !== 'undefined' && typeof options.server.suppressOutput !== 'boolean') {
		return 'Configuration object property "server.suppressOutput" must be a boolean';
	}

	// Validate the services
	if (_.isArray(options.services)) {
		for (i = 0; i < options.services.length; i++) {
			service = options.services[i];

			if (typeof service.route !== 'string' || service.route.length === 0) {
				return 'Configuration object property "services[' + i + '].route" must be a non-empty string';
			}

			if (typeof options.services[i].databaseUsername !== 'string') {
				return 'Configuration object property "services[' + i + '].databaseUsername" must be a string';
			}

			if (typeof options.services[i].databasePassword !== 'string') {
				return 'Configuration object property "services[' + i + '].databasePassword" must be a string';
			}

			if (typeof service.databaseConnectString !== 'string') {
				return 'Configuration object property "services[' + i + '].databaseConnectString" must be a string';
			}

			if ( typeof service.defaultPage !== 'undefined' && typeof service.defaultPage !== 'string') {
				return 'Configuration object property "services[' + i + '].defaultPage" must be a string';
			}

			if ( typeof service.documentTableName !== 'undefined' && typeof service.documentTableName !== 'string') {
				return 'Configuration object property "services[' + i + '].documentTableName" must be a string';
			}
		}
	}

	return Undefined;
}

/**
 * Load and parse a YAML or JSON configuration object from a file.
 * @param {string} filePath the path to the YAML or JSON configuration file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function load(filePath) {
	var defaults = {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: path.join(process.cwd(), 'static')
			}],
			requestLogging: true
		},
		services: []
	};
	var config;

	// Load configuration
	debug('Load configuration file "' + filePath + '"...');
	if (filePath) {
		try {
			config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
		} catch (e) {
			log.exit(new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message));
		}
	}

	// Merge default options with the actual ones
	return _.extend(defaults, config);
}

/**
 * Create a new sample configuration file
 * @param {String} outputFileName The name to configuration file.
 */
function createSample(outputFileName) {
	var inputFileName = '',
		config = '',
		fileName;

	// Read the configuration file
	inputFileName = path.join(__dirname, '../conf', SAMPLE_FILENAME);
	path.resolve(inputFileName, inputFileName);
	try {
		config = fs.readFileSync(inputFileName, 'utf8');
	} catch (e) {
		log.exit(new Error('Could not load the sample configuration file: "' + inputFileName + '"'));
	}

	// Write sample configuration file
	fileName = path.join(process.cwd(), outputFileName);
	try {
		fs.writeFileSync(fileName, config);
	} catch (e) {
		log.exit(new Error('Could not save sample configuration file: "' + fileName + '"'));
	}
}

/**
 * Set a new sample filename and return the original one
 * @param {String} filename The name to sample configuration file.
 * @return {String} The name to sample configuration file.
 */
function setSampleFilename(filename) {
	var original = SAMPLE_FILENAME;

	SAMPLE_FILENAME = filename;

	return original;
}

module.exports = {
	validate: validate,
	load: load,
	createSample: createSample,
	setSampleFilename: setSampleFilename
};
