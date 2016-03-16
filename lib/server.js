'use strict';

/**
* Module dependencies.
*/

const fs = require('fs');
const os = require('os');
const path = require('path');

const debug = require('debug')('node_plsql:server');
const _ = require('underscore');
const express = require('express');
const bodyParser = require('body-parser');
const multipart = require('connect-multiparty');
const cookieParser = require('cookie-parser');
const compression = require('compression');
const morgan = require('morgan');
const mkdirp = require('mkdirp');
const Promise = require('es6-promise').Promise;

const log = require('./log');
const config = require('./config');
const error = require('./error');
const route = require('./route');
const database = require('./database');
const statistics = require('./statistics');
const statusPage = require('./statusPage');

/**
* Module constants.
*/

const REQUEST_LOGGING_FILENAME = 'access.log';
const UPLOAD_DIRECTORY = './uploads/';

/**
* Module variables.
*/

/**
* Start application
*
* @param {Object} options - Server configuration
* @param {Object} resolve - resolve method of the promise
* @param {Object} reject - resolve method of the promise
* @return {Promise} promise with the application as parameter
* @api public
*/
function start(options) {
	return new Promise(function (resolve, reject) {
		const HOST = os.hostname();
		let application = {
				options: options
			},
			promise,
			valid,
			access;

		// Log server startup
		log.log('Starting the NodeJS PL/SQL Gateway');

		// Validate configuration
		debug('Validate configuration...');
		valid = config.validate(options);
		if (typeof valid !== 'undefined') {
			log.error('Configuration error: ' + valid);
			reject(valid);
			return;
		}

		// Enable tracing
		if (application.options.server.requestTracing) {
			log.log('Enable request tracing to the file "trace.log".');
			log.tracing(true);
		}

		// Enable or disable the console output
		/* istanbul ignore else */
		if (application.options.server.suppressOutput) {
			log.enable(false);
		}

		// Create express application
		debug('Creating express application...');
		application.expressApplication = express();

		// Initialize the statistics
		statistics.setStartup(application);

		// Configure express views
		application.expressApplication.set('view engine', 'ejs');
		application.expressApplication.set('views', path.resolve(path.join(__dirname, '../views')));
		application.expressApplication.use(express.static(path.resolve(path.join(__dirname, '../views'))));

		// Disable some headers
		application.expressApplication.disable('x-powered-by');

		// Middleware: for parsing multipart-form data requests which supports streams2.
		mkdirp.sync(UPLOAD_DIRECTORY);
		application.expressApplication.use(multipart({uploadDir: UPLOAD_DIRECTORY}));

		debug('Use "json parsing middleware"...');
		application.expressApplication.use(bodyParser.json());

		debug('Use "urlencoded parsing middleware"...');
		application.expressApplication.use(bodyParser.urlencoded({extended: true}));

		// Middleware: Cookie Parser
		debug('Use "cookieParser"...');
		application.expressApplication.use(cookieParser());

		// Middleware: compression
		debug('Use "compression"...');
		application.expressApplication.use(compression({threshhold: 512}));

		// Middleware: serve static files
		_staticFiles(application.expressApplication, application.options.server.static);

		// Middleware: logging
		/* istanbul ignore next */
		if (application.options.server.requestLogging) {
			log.log('Enable request logging to the file "' + REQUEST_LOGGING_FILENAME + '".');
			access = fs.createWriteStream('access.log', {flags: 'a'});
			application.expressApplication.use(morgan('combined', {
				stream: access
			}));
		}

		// Add a route for the status page
		debug('Add route for the status page...');
		application.expressApplication.route('/status').get(function (req, res/*, next*/) {
			statusPage.process(application, req, res);
		});
		application.expressApplication.route('/shutdown').get(function (req, res/*, next*/) {
			debug('Shutting down');

			database.destroyConnectionPools(application).then(function () {
				res.send('Server has been shut down!');
				log.log('Server has been shut down!');
				process.exit(0);
			});

			/*	TODO: Investigate why this code takes very long to close the connection!
			stop(application, function () {
				debug('Shut down!');
				process.exit(0);
			});
			*/
		});

		// Process the routes
		debug('Configuring the routes...');
		route.createRoutes(application);

		// At last add a route for all remaining requests that cannot be handled
		//	and return a 404 error
		application.expressApplication.get('*', function (req, res, next) {
			let err = new Error();

			err.status = 404;

			//next(err);
			error.errorHandler(err, req, res, next);
		});

		// Error handler (must be added as the last service)
		debug('Add an error handler...');
		application.expressApplication.use(error.errorHandler);

		// Create the needed connection pools
		debug('Create needed database pools...');
		promise = database.createConnectionPools(application);

		// Bind and listen for connections on the given host and port
		promise.then(function () {
			const PORT = application.options.server.port;

			log.log('The host ' + HOST + ' is now listening on port ' + PORT + '.');
			log.log('Use http://' + HOST + ':' + PORT + '/status to see the server status page.');

			application.server = application.expressApplication.listen(PORT);

			resolve(application);
		});

		/* istanbul ignore next */
		promise.catch(function (err) {
			reject(err);
		});
	});
}

/**
* Start application
*
* @param {Object} application - node_plsql application
* @param {Function} callback - Callback function to be invoked when server connection has closed
* @api public
*/
function stop(application, callback) {
	debug('stop: START');

	// Destrpy the connection pools
	database.destroyConnectionPools(application).then(function () {
		debug('stop: the connection pools have been destroyed');

		/* istanbul ignore else */
		if (application && application.server && application.server.close) {
			debug('start: close the server');
			application.server.close(callback);
		} else {
			callback();
		}
	});
}

/*
* Serve static files
*/
function _staticFiles(expressApplication, staticResources) {
	if (_.isArray(staticResources)) {
		_.each(staticResources, function (staticResource) {
			log.log('Serving static files for mount path "' + staticResource.mountPath + '" from "' + staticResource.physicalDirectory + '"');
			expressApplication.use(staticResource.mountPath, express.static(staticResource.physicalDirectory));
		});
	}
}

module.exports = {
	start: start,
	stop: stop
};
