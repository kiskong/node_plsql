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

/**
* Create a listener
*
* @param {Object} serverConfig Server configuration
* @return {Object} Express application
* @api public
*/
function start(serverConfig) {
	'use strict';

	var valid,
		app,
		access,
		server,
		host = os.hostname(),
		i = 0;

	// Validate configuration
	debug('Validate configuration...');
	valid = config.validate(serverConfig);
	if (typeof valid !== 'undefined') {
		throw new TypeError(valid);
	}

	// Enable or disable the console output
	/* istanbul ignore else */
	if (serverConfig.server.suppressOutput) {
		log.enable(false);
	}

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
	mkdirp.sync(UPLOAD_DIRECTORY);

	// Middleware: for parsing multipart-form data requests which supports streams2.
	app.use(multipart({
		uploadDir: UPLOAD_DIRECTORY
	}));

	debug('Use "json parsing middleware"...');
	app.use(bodyParser.json());

	debug('Use "urlencoded parsing middleware"...');
	app.use(bodyParser.urlencoded({
		extended: true
	}));

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
		app.use(serverConfig.server.static[i].mountPath, express.static(serverConfig.server.static[i].physicalDirectory));
	}

	// Middleware: logging
	/* istanbul ignore else */
	if (serverConfig.server.requestLogging) {
		/* istanbul ignore if */
		log.log('Enable request logging to the file "' + REQUEST_LOGGING_FILENAME + '".');
		access = fs.createWriteStream('access.log', {flags: 'a'});
		app.use(morgan('combined', {
			stream: access
		}));
	}

	// Process the routes
	debug('Configuring services...');
	for (i = 0; i < serverConfig.services.length; i++) {
		route.createRoute(app, serverConfig.server, serverConfig.services[i]);
	}

	// Add a route for the status page
	app.route('/status').get(function (req, res/*, next*/) {
		statusPage.process(app, req, res);
	});

	// Error handler 404
	app.use(error.errorHandler404);

	// Bind and listen for connections on the given host and port
	/* istanbul ignore if */
	log.log('The host ' + host + ' is now listening on port ' + serverConfig.server.port + '.');
	log.log('Use http://' + host + ':' + serverConfig.server.port + '/status to see the server status page.');
	statistics.setStartup(app);
	server = app.listen(serverConfig.server.port);
	/* istanbul ignore if */
	if (app.hasOwnProperty('server')) {
		throw new Error('Invalid property "server" in the app object!');
	} else {
		app.server = server;
	}

	return app;
}

module.exports = {
	start: start
};
