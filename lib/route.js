/* jshint node: true */

/**
* Module dependencies.
*/

var log = require('./log');
var request = require('./request');
var debug = require('debug')('node_plsql:listener');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Process request
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service Service configuration
* @param {Object} req Request object
* @param {Object} res Response object
* @api private
*/
var processRequest = function (app, serverConfig, service, req, res) {
	'use strict';

	debug('Process request (route="' + service.route + '", type="' + req.type + '")');

	request.processRequest(serverConfig, app.databaseHandle, service, req, res);
};

/**
* Create a new page route
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service Service configuration
* @api private
*/
function createPageRoute(app, serverConfig, service) {
	'use strict';

	// Define the route
	var path = '/' + service.route + '/:name';

	log.log('Create page route for "' + path + '"');

	// Create service route
	app.route(path).get(function (req, res, next) {
		processRequest(app, serverConfig, service, req, res);
	}).post(function (req, res, next) {
		processRequest(app, serverConfig, service, req, res);
	});
}

/**
* Create a new default page route
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service Service configuration
* @api private
*/
function createDefaultPageRoute(app, serverConfig, service) {
	'use strict';

	// Define the route
	var path = '/' + service.route;

	log.log('Create default page route for "' + path + '" to "' + service.defaultPage + '"');

	// Add a route that only redirects to the actual default page
	app.route(path).get(function (req, res, next) {
		debug('createDefaultPageRoute: redirecting to the default page "' + service.defaultPage + '"');
		res.redirect(302, path + '/' + service.defaultPage);
	});
}

/**
* Create a new route
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service Service configuration
* @api private
*/
function createRoute(app, serverConfig, service) {
	'use strict';

	// Create the page route
	createPageRoute(app, serverConfig, service);

	// Create the default page route
	if (service.hasOwnProperty('defaultPage') && service.defaultPage.length > 0) {
		createDefaultPageRoute(app, serverConfig, service);
	}
}

module.exports = {
	createRoute: createRoute
};
