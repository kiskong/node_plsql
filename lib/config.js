/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs'),
	path = require('path'),
	stripComments = require('strip-json-comments'),
	yaml = require('js-yaml');

/**
 * Load and parse a YAML or JSON configuration object from a file.
 * @param {string} filePath the path to the YAML or JSON config file
 * @returns {Object} the parsed config object (empty object if there was a parse error)
 * @api private
 */
function loadConfig(filePath) {
	'use strict';

	var config = {};

	if (filePath) {
		try {
			config = yaml.safeLoad(stripComments(fs.readFileSync(filePath, 'utf8')));
		} catch (e) {
			throw new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message);
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
		'	// Server configuration (used for the entire server)',
		'	"server": {',
		'		// The server port to listen at',
		'		"port": 8999,',
		'		// The mapping for servicing static files',
		'		"static": [',
		'			{',
		'				"mountPath": "/",',
		'				"physicalDirectory": "' + physicalDirectory + '"',
		'			}',
		'		]',
		'	},',
		'	// Service configuration (used for each individual connection)',
		'	"services": [',
		'		{',
		'			// The route to be intercepted for this service',
		'			"route": "sample",',
		'			// The Oracle username',
		'			"oracleUsername": "sample",',
		'			// The Oracle password',
		'			"oraclePassword": "sample",',
		'			// The Oracle host name or IP address',
		'			"oracleHostname": "localhost",',
		'			// The Oracle port',
		'			"oraclePort": 1521,',
		'			// The Oracle service name or SID',
		'			"oracleService": "ORCL"',
		'		}',
		'	]',
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
