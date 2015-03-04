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

	res.status(404).send('<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL ' + req.url + ' was not found.</p></body></html>');
};

module.exports = {
	errorHandler404: errorHandler404
};
