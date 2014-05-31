/* jshint node: true */

/**
* Module dependencies.
*/
var util = require('util');
var debug = require('debug')('node_plsql:error');

/**
* Error handler for 404 errors
*
* @param {Object} req Request object.
* @param {Object} res Response object.
* @api public
*/
var errorHandler404 = function (req, res) {
	'use strict';

	debug(util.inspect(req, {showHidden: false, depth: 1, colors: true}));

	res.send(404, '<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL ' + req.url + ' was not found.</p></body></html>');
};

/**
* Error handler for 500 errors
*
* @param {Object} err Error object.
* @param {Object} req Request object.
* @param {Object} res Response object.
* @param {Function} next
* @api public
*/
var errorHandler500 = function (err, req, res, next) {
	'use strict';

	debug(util.inspect(err, {showHidden: false, depth: null, colors: true}));
	debug(util.inspect(req, {showHidden: false, depth: 1, colors: true}));

	res.send(500, JSON.stringify(err));
};

module.exports = {
	errorHandler404: errorHandler404,
	errorHandler500: errorHandler500
};
