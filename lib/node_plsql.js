/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs');
var os = require('os');

var express = require('express');
var bodyParser = require('body-parser');
var multipart = require('connect-multiparty');
var cookieParser = require('cookie-parser');
var compression = require('compression');
var morgan = require('morgan');
var serveStatic = require('serve-static');
var mkdirp = require('mkdirp');

var error = require('./error');
var oracle = require('./oracle');
var request = require('./request');

var debug = require('debug')('node_plsql:listener');

/**
* Module constants.
*/
var REQUEST_LOGGING_FILENAME = 'access.log';
var UPLOAD_DIRECTORY = './uploads/';

/**
* Module variables.
*/

var app;

/**
* Create a new service
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service
* @api private
*/
var createService = function (app, serverConfig, service) {
	'use strict';

	// Define the route
	var path = '/' + service.route + '/:name',
		text = '';

	text = 'Configuring route "' + service.route + '"';
	if (debug.enabled) {
		debug(text);
	} else {
		console.log(text);
	}

	// Set route
	app.route(path)
	.get(function (req, res, next) {
		request.processRequest(serverConfig, app.oracleSession, service.databaseUsername, service.databasePassword, service.documentTableName, req, res);
	})
	.post(function (req, res, next) {
		request.processRequest(serverConfig, app.oracleSession, service.databaseUsername, service.databasePassword, service.documentTableName, req, res);
	});
};

/**
* Create a listener
*
* @param {Object} config Server configuration
* @return {Object}
* @api public
*/

function start(config)
{
	'use strict';

	var host = os.hostname(),
		text = '',
		i = 0;

	debug('-INIT: BEGIN----------------------------------------------------------');

	text = 'Starting the NodeJS PL/SQL Gateway';
	if (debug.enabled) {
		debug(text);
	} else {
		console.log(text);
	}

	// Create express application
	debug('Creating express application...');
	app = express();

	// Create the needed Oracle connections
	debug('Connecting with server...');
	app.oracleSession = oracle.connect(config);

	// Make sure the upload directory exists
	mkdirp(UPLOAD_DIRECTORY, function (err) {
		if (err) {
			throw err;
		}
	});

	// Middleware: for parsing multipart-form data requests which supports streams2.
	app.use(multipart({
		uploadDir: UPLOAD_DIRECTORY
	}));

	debug('Use "Body parsing middleware"...');
	app.use(bodyParser());

	// Middleware: Cookie Parser
	debug('Use "cookieParser"...');
	app.use(cookieParser());

	// Middleware: compression
	debug('Use "compression"...');
	app.use(compression({
		threshhold: 512
	}));

	// Middleware: serve static files
	for (i = 0; i < config.server.static.length; i++) {
		text = 'Serving static files for mount path "' + config.server.static[i].mountPath + '" from "' + config.server.static[i].physicalDirectory + '"';
		if (debug.enabled) {
			debug(text);
		} else {
			console.log(text);
		}
		app.use(config.server.static[i].mountPath, serveStatic(config.server.static[i].physicalDirectory));
	}

	// Middleware: logging
	if (config.server.requestLogging) {
		text = 'Enable request logging to the file "' + REQUEST_LOGGING_FILENAME + '".';
		if (debug.enabled) {
			debug(text);
		} else {
			console.log(text);
		}
		var access = fs.createWriteStream('access.log', {flags: 'a'});
		app.use(morgan({
			stream: access
		}));
	}

	// Process the routes
	debug('Configuring services...');
	for (i = 0; i < config.services.length; i++) {
		createService(app, config.server, config.services[i]);
	}

	// Error handler
	app.use(error.errorHandler);

	// Bind and listen for connections on the given host and port
	text = 'The host ' + host + ' is now listening on port ' + config.server.port + '.';
	if (debug.enabled) {
		debug(text);
	} else {
		console.log(text);
	}
	app.listen(config.server.port);

	debug('-INIT: END------------------------------------------------------------');

	return app;
}

module.exports = {
	start: start
};
