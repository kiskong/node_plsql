/* jshint node: true */

/**
* Module dependencies.
*/

var fs = require('fs');

var express = require('express');
var compression = require('compression');
var morgan = require('morgan');
var serveStatic = require('serve-static');

var error = require('./error');
var createCGI = require('./cgi').createCGI;
var oracle = require('./oracle');
var header = require('./header');
var response = require('./response');

var debug = require('debug')('plsql-server:listener');
var dumpBlock = require('./dump').block;

/**
* Module variables.
*/

var app;

/**
* Invoke the PL/SQL code and return the page content
*
* @api private
*/
var processRequest = function (serverConfig, service, req, res) {
	'use strict';

	// Prepare on object only containing the actual properties
	var properties = {};
	var propName = '';
	for (propName in req.query) {
		if (req.query.hasOwnProperty(propName)) {
			properties[propName] = req.query[propName];
		}
	}

	// Get the CGI
	var cgi = createCGI(serverConfig, req);

	// Invoke the PL/SQL procedure
	debug('Invoking "' + req.params.name + '" with the arguments "' + JSON.stringify(req.query) + '...');
	var page = oracle.invokeProcedure(service.oracleSession, req.params.name, properties, cgi);
	if (debug.enabled) {
		console.log(dumpBlock('page', page));
	}

	// Parse the page content and return the headers and the body
	var headerAndBody = header.getHeaderAndBody(page);

	// Parse the header
	var headerMain = {},
		headerOther = {},
		headerCookies = [];
	debug('Parsing header...');
	header.parseHeader(headerAndBody.header, headerMain, headerOther, headerCookies);
	if (debug.enabled) {
		console.log(dumpBlock('header main', JSON.stringify(headerMain, null, '\t')));
		console.log(dumpBlock('header other', JSON.stringify(headerOther, null, '\t')));
		console.log(dumpBlock('header cookies', JSON.stringify(headerCookies, null, '\t')));
	}

	// Send the response
	response.send(res, headerMain, headerOther, headerCookies, headerAndBody.body);
};

/**
* Create a new service
*
* @api private
*/
var createService = function (app, serverConfig, service) {
	'use strict';

	// Define the route
	var path = '/' + service.route + '/:name';
	app.route(path)
	.get(function (req, res, next) {
		processRequest(serverConfig, service, req, res);
	})
	.post(function (req, res, next) {
		processRequest(serverConfig, service, req, res);
	});
};

/**
* Create a listener
*
* @return {Function}
* @api public
*/

exports.createListener = function (config) {
	'use strict';

	var i = 0;

	// Create the access log stream
	var access = fs.createWriteStream('access.log', {flags: 'a'});

	// Create the needed Oracle connections
	debug('Connecting with server...');
	oracle.createNeededOracleConnections(config.services);

	// Create express application
	debug('Creating express application...');
	app = express();

	// Middleware: compression
	app.use(compression({
		threshhold: 512
	}));

	// Middleware: serve static files
	app.use(serveStatic(__dirname + '/static'));

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
};
