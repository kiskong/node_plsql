'use strict';

/*
 * The CLI object should *not* call process.exit() directly. It should only return
 * exit codes. This allows other programs to use the CLI object and still control
 * when the program exits.
 */

/**
* Module dependencies.
*/

const nopt = require('nopt');
const path = require('path');
const log = require('./log');
const version = require('./version');
const config = require('./config');
const server = require('./server');

/**
* Execute the application
*
* @param {Array} argv Command line arguments
* @return {Number} Exit code
* @api private
*/
function execute(argv) {
	let configPath = config.defaultConfigFileName();
	const knownOpts = {
		'version': Boolean,
		'help': Boolean,
		'silent': Boolean,
		'config': path,
		'init': Boolean
	};
	const shortHands = {
		'v': ['--version'],
		'h': ['--help'],
		's': ['--silent'],
		'c': ['--config'],
		'i': ['--init']
	};
	const usage = [
		'node_plsql.js — The Node.js PL/SQL Gateway for Oracle',
		'Usage: node_plsql [options]',
		'Options:',
		'\t-v, --version       Outputs version number',
		'\t-h, --help          Outputs this help message',
		'\t-s, --silent        Suppress any console output',
		'\t-c, --config=<file> Use configuration from this file (default: ' + configPath + ')',
		'\t-i, --init          Run configuration initialization wizard',
		'Examples:',
		'\tnode_plsql --init',
		'\tnode_plsql --config=sample.json'
	].join('\n');
	let promise;
	let serverConfig;

	// Parse the command line arguments
	let parsed = nopt(knownOpts, shortHands, argv, 2);

	// Check arguments
	if (parsed.config) {
		configPath = parsed.config;
	}

	// Process the command line arguments
	if (parsed.version) {
		console.log('node_plsql.js — Version: ' + version.get());
		return 0;
	} else if (parsed.help) {
		console.log(usage);
		return 0;
	} else if (parsed.init) {
		config.initialize(configPath);
		return 0;
	}

	// Load configuration
	serverConfig = config.load(configPath);

	// Merge with any command line arguments
	if (parsed.suppress) {
		serverConfig.server.suppressOutput = true;
	}

	// Start the server
	promise = server.start(serverConfig);
	promise.catch(function (err) {
		log.error(err);
	});

	return 0;
}

module.exports = {
	execute: execute
};
