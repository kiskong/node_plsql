/*
 * The CLI object should *not* call process.exit() directly. It should only return
 * exit codes. This allows other programs to use the CLI object and still control
 * when the program exits.
 */

/**
* Module dependencies.
*/

var nopt = require('nopt');
var path = require('path');
var version = require('./version');
var config = require('./config');
var server = require('./server');

/**
* Execute the application
*
* @param {Array} argv Command line arguments
* @return {Number} Exit code
* @api private
*/
function execute(argv) {
	'use strict';

	var knownOpts = {
		'version': Boolean,
		'help': null,
		'config': path,
		'init': String
	};
	var shortHands = {
		'v': ['--version'],
		'h': ['--help'],
		's': ['--silent'],
		'c': ['--config'],
		'i': ['--init']
	};
	var usage = [
		'node_plsql.js — The Node.js PL/SQL Gateway for Oracle',
		'Usage: node_plsql [options]',
		'Options:',
		'\t-v, --version       Outputs version number',
		'\t-h, --help          Outputs this help message',
		'\t-s, --silent        Suppress any console output',
		'\t-c, --config=<file> Path to your json config file',
		'\t-i, --init=<file>   Path to a new json config file to be created',
		'Examples:',
		'\tnode_plsql --init=sample.json',
		'\tnode_plsql --config=sample.json'
	].join('\n');
	var serverConfig;

	// Parse the command line arguments
	var parsed = nopt(knownOpts, shortHands, argv, 2);

	// Process the command line arguments
	if (parsed.version) {
		console.log('node_plsql.js — Version: ' + version.get());
		return 0;
	} else if (parsed.help) {
		console.log(usage);
		return 0;
	} else if (parsed.init) {
		config.createSample(parsed.init);
		console.log('The configuration file "' + parsed.init + '" has been created.');
		return 0;
	}

	// Check arguments
	if (!parsed.config) {
		console.log('Error: No configuration file (--config) has been specified\n');
		console.log(usage);
		return 1;
	}

	// Load configuration
	serverConfig = config.load(parsed.config);

	// Merge with any command line arguments
	if (parsed.suppress) {
		serverConfig.suppressOutput = true;
	}

	// Start the server
	server.start(serverConfig);

	return 0;
}

module.exports = {
	execute: execute
};
