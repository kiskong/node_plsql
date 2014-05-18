/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs');

var express = require('express');
var cookieParser = require('cookie-parser');
var compression = require('compression');
var morgan = require('morgan');
var serveStatic = require('serve-static');

var error = require('./error');
var oracle = require('./oracle');
var request = require('./request');

var debug = require('debug')('node_plsql:listener');

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
	var path = '/' + service.route + '/:name';
	app.route(path)
	.get(function (req, res, next) {
		request.processRequest(serverConfig, service, req, res);
	})
	.post(function (req, res, next) {
		request.processRequest(serverConfig, service, req, res);
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

	debug('Starting server ...');

	var i = 0;

	// Create the access log stream
	var access = fs.createWriteStream('access.log', {flags: 'a'});

	// Create the needed Oracle connections
	debug('Connecting with server...');
	oracle.createNeededOracleConnections(config.services);

	// Create express application
	debug('Creating express application...');
	app = express();

	// Middleware: Cookie Parser
	app.use(cookieParser());

	// Middleware: compression
	app.use(compression({
		threshhold: 512
	}));

	// Middleware: serve static files
	for (i = 0; i < config.server.static.length; i++) {
		debug('Serve static files for "' + config.server.static[i].mountPath + '"" from "' + config.server.static[i].physicalDirectory + '" ...');
		app.use(config.server.static[i].mountPath, serveStatic(config.server.static[i].physicalDirectory));
	}

	// Middleware: logging
	app.use(morgan({
		stream: access
	}));

	// Process the routes
	debug('Configuring services...');
	for (i = 0; i < config.services.length; i++) {
		createService(app, config.server, config.services[i]);
	}

	// Error handler
	app.use(error.errorHandler);

	// Bind and listen for connections on the given host and port
	debug('Start listening on port ' + config.server.port + '...');
	app.listen(config.server.port);

	return app;
}

module.exports = {
	start: start
};
