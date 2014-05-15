/* jshint node: true */

/*
 * The CLI object should *not* call process.exit() directly. It should only return
 * exit codes. This allows other programs to use the CLI object and still control
 * when the program exits.
 */

/**
* Module dependencies.
*/

var nopt = require('nopt');
var fs = require('fs');
var path = require('path');
var node_plsql = require('./node_plsql');

/**
 * Load and parse configuration file
 * @param {String} filePath The path to configuration file.
 * @returns {Object} AConfiguration object.
 */
function loadConfigurationFile(filePath)
{
	'use strict';

	var configuration,
		text;

	if (filePath) {
		// Read .eslintignore file
		try {
			text = fs.readFileSync(filePath, 'utf8');
			try {
				// Attempt to parse as JSON
				configuration = JSON.parse(text);
			} catch (e) {
				throw new TypeError('Could not parse JSON configuration file file: ' + filePath);
			}
		} catch (e) {
			throw new Error('Could not parse JSON configuration file file: ' + filePath);
		}
	}

	return configuration;
}

/**
* Execute the application
*
* @param {Array} argv Command line arguments
* @return {Number}
* @api private
*/
function execute(argv)
{
	'use strict';

	var knownOpts = {
		'version': Boolean,
		'help': null,
		'config': path
	};
	var shortHands = {
		'v': ['--version'],
		'h': ['--help'],
		'c': ['--config']
	};
	var usage = [
		'node_plsql.js — The Node.js PL/SQL Gateway for Oracle',
		'Usage: node_plsql [options]',
		'Options:',
		'\t-v, --version \t\tOutputs version number',
		'\t-h, --help \t\tOutputs this help message',
		'\t-c, --config \t\tPath to your json config file'
	].join('\n');
	var base = path.join(__dirname, '..');
	var version = JSON.parse(fs.readFileSync(base + '/package.json', 'utf8')).version;
	var configuration;

	// Parse the command line arguments
	var parsed = nopt(knownOpts, shortHands, argv, 2);

	// Process the command line arguments
	if (parsed.version) {
		console.log('node_plsql.js — Version: ' + version);
		return 0;
	} else if (parsed.help) {
		console.log(usage);
		return 0;
	} else if (parsed.config) {
		configuration = loadConfigurationFile(parsed.config);
	}

	// Check arguments
	if (!configuration) {
		console.log('Error: No configuration file (--config) has been specified\n');
		console.log(usage);
		return 1;
	}

	// Start the server
	node_plsql.start(configuration);

	return 0;
}

module.exports = {
	execute: execute
};
