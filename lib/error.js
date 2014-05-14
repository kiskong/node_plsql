/* jshint node: true */

/**
* Module dependencies.
*/

/**
* Error Handler Middleware
*
* @api public
*/
var errorHandler = function (err, req, res, next) {
	'use strict';

	console.error(err.stack);

	res.send(500, 'Something broke!');
};

module.exports = {
	errorHandler: errorHandler
};
