/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs'),
	path = require('path'),
	stripComments = require('strip-json-comments'),
	yaml = require('js-yaml'),
	underscore = require('underscore');

/**
 * Load and parse a YAML or JSON configuration object from a file.
 * @param {string} filePath the path to the YAML or JSON config file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function loadConfig(filePath) {
	'use strict';

	var config = {},
		i = 0;

	// Load configuration
	if (filePath) {
		try {
			config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
		} catch (e) {
			throw new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message);
		}
	}

	// Validate configuration
	if (!config.hasOwnProperty('server')) {
		throw new TypeError('Configuration object must containt a property "server"');
	}
	if (!config.server.hasOwnProperty('port') || !underscore.isNumber(config.server.port) || config.server.port < 1 || config.server.port > 32767) {
		throw new TypeError('Configuration object must containt a numeric property "server.port"');
	}
	if (config.services.length > 1) {
		for (i = 0; i < config.services.length; i++) {
			if (config.services[i].database !== config.services[0].database) {
				throw new TypeError('Configuration object is only allowed to containt a single database');
			}
		}
	}

	return config;
}

/**
 * Create a new sample configuration file
 * @param {String} fileName The name to configuration file.
 */
function createSampleConfig(fileName)
{
	'use strict';

	var physicalDirectory = path.join(process.cwd(), 'static');
	physicalDirectory = physicalDirectory.replace(/\\/g, '/');

	var CONFIGURATION = [
		'{',
		'  // Server configuration (used for the entire server)',
		'  "server": {',
		'    // The server port to listen at',
		'    "port": 8999,',
		'    // The mapping for servicing static files',
		'    "static": [',
		'    {',
		'      "mountPath": "/",',
		'      "physicalDirectory": "' + physicalDirectory + '"',
		'      }',
		'    ]',
		'  },',
		'  // Service configuration (used for each individual connection)',
		'  "services": [',
		'    {',
		'      // The route to be intercepted for this service',
		'      "route": "sample",',
		'      // The Oracle username',
		'      "username": "sample",',
		'      // The Oracle password',
		'      "password": "sample",',
		'      // The Oracle database name',
		'      "database": "",',
		'    }',
		'  ]',
		'}'
	].join('\n');

	fileName = path.join(process.cwd(), fileName);

	// Write sample configuration file
	try {
		fs.writeFileSync(fileName, CONFIGURATION);
	} catch (e) {
		throw new Error('Could not save sample configuration file: ' + fileName);
	}
}

module.exports = {
	loadConfig: loadConfig,
	createSampleConfig: createSampleConfig
};
