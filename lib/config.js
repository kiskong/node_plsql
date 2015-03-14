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
* Module constants.
*/


/**
* Module variables.
*/
var SAMPLE_FILENAME = 'sample.json';


/**
 * Validate configuration object.
 * @param {config} config The configuration object
 * @returns {String} Return undefined if valid or else the error message.
 * @api public
 */
function validate(config) {
	'use strict';

	debug('validate');

	var i = 0;

	// Validate "server"
	if (!config || !config.server) {
		return 'Configuration object must containt a "server" property';
	}

	// Validate port
	if (!underscore.isNumber(config.server.port) || config.server.port < 1 || config.server.port > 32767) {
		return 'Configuration object must containt a numeric property "server.port"';
	}

	// Suppress output
	if (typeof config.server.suppressOutput !== 'undefined' && typeof config.server.suppressOutput !== 'boolean') {
		return 'Configuration object property "server.suppressOutput" must be a boolean';
	}

	// Validate the services
	for (i = 0; i < config.services.length; i++) {

		// All services must use the same database
		if (config.services[i].databaseConnectString !== config.services[0].databaseConnectString) {
			return 'Configuration object is only allowed to containt a single database!';
		}

		// All services must contain a documentTableName
		if (!config.services[i].documentTableName || config.services[i].documentTableName.length === 0) {
			return 'Configuration object must contain a "documentTableName" for each service!';
		}
	}

	// return undefined;
}

/**
 * Load and parse a YAML or JSON configuration object from a file.
 * @param {string} filePath the path to the YAML or JSON config file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function load(filePath) {
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
	debug('Load configuration file "' + filePath + '"...');
	if (filePath) {
		try {
			config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
		} catch (e) {
			throw new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message);
		}
	}

	// Merge default options with the actual ones
	var serverConfig = underscore.extend(defaults, config);

	return serverConfig;
}

/**
 * Create a new sample configuration file
 * @param {String} outputFileName The name to configuration file.
 */
function createSample(outputFileName) {
	'use strict';

	var inputFileName = '',
		config = '';

	// Read the configuration file
	inputFileName = path.join(__dirname, '../conf', SAMPLE_FILENAME);
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

/**
 * Set a new sample filename and return the original one
 * @param {String} filename The name to sample configuration file.
 * @return {String} The name to sample configuration file.
 */
function setSampleFilename(filename) {
	'use strict';

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
