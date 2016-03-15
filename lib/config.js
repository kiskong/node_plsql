'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:config');
const fs = require('fs');
const path = require('path');
const stripComments = require('strip-json-comments');
const inquirer = require('inquirer');
const yaml = require('js-yaml');
const _ = require('underscore');
const log = require('./log');


/**
* Module constants.
*/

const DEFAULT_CONFIGURATION_FILENAME = 'node_plsql.json';
const CONFIG_TEMPLATE = [
	'{',
	'	// The server configuration',
	'	"server": {',
	'	    // The server port to listen at',
	'	    "port": ${port},',
	'',
	'	    // The mapping for servicing static files',
	'	    "static": [',
	'	        {',
	'	            "mountPath": "${mountPath}",',
	'	            "physicalDirectory": "${physicalDirectory}"',
	'	        }',
	'	    ],',
	'',
	'	    // Suppress any console output',
	'	    "suppressOutput": false,',
	'',
	'	    // Enable http request logging',
	'	    "requestLogging": ${requestLogging},',
	'',
	'	    // Enable tracing',
	'	    "requestTracing": ${requestTracing}',
	'	},',
	'   // Service configuration (used for each individual connection)',
	'   "services": [',
	'      {',
	'         // The route to be intercepted for this service',
	'         "route": "${route}",',
	'',
	'         // The default page',
	'         "defaultPage": "${defaultPage}",',
	'',
	'         // The database username',
	'         "databaseUsername": "${databaseUsername}",',
	'',
	'         // The database password',
	'         "databasePassword": "${databasePassword}",',
	'',
	'         // The database connect string',
	'         "databaseConnectString": "${databaseConnectString}",',
	'',
	'         // The name of the table for file uploads',
	'         "documentTableName": "${documentTableName}"',
	'      }',
	'   ]',
	'}'
].join('\n');

/**
* Module variables.
*/

/**
 * Validate configuration object.
 * @param {Object} options The configuration object
 * @returns {String} Return undefined if valid or else the error message.
 * @api public
 */
function validate(options) {
	let Undefined;
	let service,
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
	if (!_isValidPort(options.server.port)) {
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
 * @param {string} [filePath=DEFAULT_CONFIGURATION_FILENAME] the path to the YAML or JSON configuration file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function load(filePath) {
	let defaults = {
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
	let config;

	filePath = filePath || DEFAULT_CONFIGURATION_FILENAME;

	// Load configuration
	debug('Load configuration file "' + filePath + '"...');
	try {
		config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
	} catch (e) {
		log.exit(new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message));
	}

	// Merge default options with the actual ones
	return _.extend(defaults, config);
}

/* istanbul ignore next */
/**
 * Create a new sample configuration file
 * @param {String} outputFileName The name to configuration file.
 */
function initialize(outputFileName) {
	function nonEmptyString(s) {
		if (s.trim().length > 0) {
			return true;
		}
		return 'You must use a non-empty string. Try again.';
	}

	inquirer.prompt([
		{
			type: 'input',
			name: 'port',
			message: 'What port do you want to use?',
			default: 8999,
			validate: function (input) {
				if (_isValidPort(input)) {
					return true;
				}

				return 'You must use an integer between 1 and 32767. Try again.';
			}
		},
		{
			type: 'input',
			name: 'route',
			message: 'What is the name of the service to configure?',
			default: 'sample',
			validate: nonEmptyString
		},
		{
			type: 'input',
			name: 'defaultPage',
			message: 'What is the default page?',
			default: ''
		},
		{
			type: 'input',
			name: 'databaseUsername',
			message: 'What Oracle username should be used?',
			default: 'scott',
			validate: nonEmptyString
		},
		{
			type: 'password',
			name: 'databasePassword',
			message: 'What Oracle password should be used?',
			default: 'tiger',
			validate: nonEmptyString
		},
		{
			type: 'input',
			name: 'databaseConnectString',
			message: 'What Oracle connect string should be used?',
			default: 'localhost:1521/ORCL',
			validate: nonEmptyString
		},
		{
			type: 'input',
			name: 'documentTableName',
			message: 'What is the document table name?',
			default: 'doctable',
			validate: nonEmptyString
		},
		{
			type: 'input',
			name: 'mountPath',
			message: 'What mount path for static resources should be used?',
			default: '/i/',
			validate: nonEmptyString
		},
		{
			type: 'input',
			name: 'physicalDirectory',
			message: 'What physical directory for static resources should be used?',
			default: './static',
			validate: nonEmptyString
		},
		{
			type: 'confirm',
			name: 'requestLogging',
			message: 'Should logging be enabled?',
			default: true
		},
		{
			type: 'confirm',
			name: 'requestTracing',
			message: 'Should tracing be enabled?',
			default: false
		}
	], function (answers) {
		processAnswers(answers, outputFileName);
		console.log('The configuration file "' + outputFileName + '" has been created.');
	});
}

/**
 * Process the configuration answers.
 * @param {Object} answers The answers.
 * @param {String} outputFileName The name to configuration file.
 */
function processAnswers(answers, outputFileName) {
	let fileName = path.join(process.cwd(), outputFileName),
		config;

	// Define an interpolate regex to match expressions that should be interpolated verbatim
	_.templateSettings = {
		interpolate: /\$\{(.+?)\}/g
	};

	// Create configuration file
	try {
		config = _.template(CONFIG_TEMPLATE)(answers);
	} catch (e) {
		/* istanbul ignore next */
		log.exit(new Error('Could not create sample configuration file: "' + fileName + '"\n' + e.message));
	}

	// Write sample configuration file
	try {
		fs.writeFileSync(fileName, config);
	} catch (e) {
		/* istanbul ignore next */
		log.exit(new Error('Could not write sample configuration file: "' + fileName + '"\n' + e.message));
	}
}

/*
 * isValidPort
 */
function _isValidPort(value) {
	let port;

	if (typeof value === 'number') {
		port = value;
	/* istanbul ignore if */
	} else if (typeof value === 'string') {
		port = Number(value);
	} else {
		return false;
	}

	return (port % 1) === 0 && port >= 1 && port <= 32767;
}

module.exports = {
	load: load,
	initialize: initialize,
	validate: validate,
	processAnswers: processAnswers,
	defaultConfigFileName: function () {
		return DEFAULT_CONFIGURATION_FILENAME;
	}
};
