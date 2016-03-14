'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:route');
const _ = require('underscore');
const log = require('./log');
const request = require('./request');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Create the routes to the different services
*
* @param {Object} application - The node_plsql application itself
* @api private
*/
function createRoutes(application) {
	let service,
		i;

	debug('createRoutes');

	if (_.isArray(application.options.services)) {
		for (i = 0; i < application.options.services.length; i++) {
			service = application.options.services[i];

			// Create the page route
			_createPageRoute(application, service);

			// Create the default page route
			if (service.hasOwnProperty('defaultPage') && service.defaultPage.length > 0) {
				_createDefaultPageRoute(application, service);
			}
		}
	}
}

/*
 * Create a new page route
 */
function _createPageRoute(application, service) {
	// Define the route
	let path = '/' + service.route + '/:name';

	log.log('Create page route for "' + path + '"');

	// Create service route
	application.expressApplication.route(path).get(function (req, res/*, next*/) {
		request.process(application, service, req, res);
	}).post(function (req, res/*, next*/) {
		request.process(application, service, req, res);
	});
}

/*
 * Create a new default page route
 */
function _createDefaultPageRoute(application, service) {
	// Define the route
	let path = '/' + service.route;

	log.log('Create default page route for "' + path + '" to "' + service.defaultPage + '"');

	// Add a route that only redirects to the actual default page
	application.expressApplication.route(path).get(function (req, res/*, next*/) {
		debug('_createDefaultPageRoute: redirecting to the default page "' + service.defaultPage + '"');
		res.redirect(path + '/' + service.defaultPage);
	});
}

module.exports = {
	createRoutes: createRoutes
};
