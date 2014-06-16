/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs');
var os = require('os');
var path = require('path');

var debug = require('debug')('node_plsql:listener');
var express = require('express');
var bodyParser = require('body-parser');
var multipart = require('connect-multiparty');
var cookieParser = require('cookie-parser');
var compression = require('compression');
var morgan = require('morgan');
var serveStatic = require('serve-static');
var mkdirp = require('mkdirp');

var log = require('./log');
var config = require('./config');
var error = require('./error');
var route = require('./route');
var database = require('./database').database;
var statistics = require('./statistics');
var statusPage = require('./statusPage');

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
* Create a listener
*
* @param {Object} serverConfig Server configuration
* @return {Object}
* @api public
*/
function start(serverConfig)
{
	'use strict';

	var host = os.hostname(),
		i = 0;

	// Validate configuration
	debug('Validate configuration...');
	var valid = config.validate(serverConfig);
	if (valid !== undefined) {
		throw new TypeError(valid);
	}

	// Enable or disable the console output
	log.enable(serverConfig.server.suppressOutput === false);

	// Log server startup
	log.log('Starting the NodeJS PL/SQL Gateway');

	// Create express application
	debug('Creating express application...');
	app = express();

	// Configure express
	app.set('view engine', 'ejs');
	app.set('views', path.resolve(path.join(__dirname, '../views')));

	// Create the needed database connection
	debug('Connecting with server...');
	app.databaseHandle = database.connect(serverConfig);

	// Make sure the upload directory exists
	mkdirp(UPLOAD_DIRECTORY, function (err) {
		/* istanbul ignore if */
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
	for (i = 0; i < serverConfig.server.static.length; i++) {
		log.log('Serving static files for mount path "' + serverConfig.server.static[i].mountPath + '" from "' + serverConfig.server.static[i].physicalDirectory + '"');
		app.use(serverConfig.server.static[i].mountPath, serveStatic(serverConfig.server.static[i].physicalDirectory));
	}

	// Middleware: logging
	/* istanbul ignore else */
	if (serverConfig.server.requestLogging) {
		/* istanbul ignore if */
		log.log('Enable request logging to the file "' + REQUEST_LOGGING_FILENAME + '".');
		var access = fs.createWriteStream('access.log', {flags: 'a'});
		app.use(morgan({
			stream: access
		}));
	}

	// Process the routes
	debug('Configuring services...');
	for (i = 0; i < serverConfig.services.length; i++) {
		route.createRoute(app, serverConfig.server, serverConfig.services[i]);
	}

	// Add a route for the status page
	app.route('/status').get(function (req, res, next) {
		statusPage.process(app, req, res);
	});

	// Error handler 404
	app.use(error.errorHandler404);

	// Error handler 500
	app.use(error.errorHandler500);

	// Bind and listen for connections on the given host and port
	/* istanbul ignore if */
	log.log('The host ' + host + ' is now listening on port ' + serverConfig.server.port + '.');
	statistics.setStartup(app);
	app.listen(serverConfig.server.port);

	return app;
}

module.exports = {
	start: start
};
