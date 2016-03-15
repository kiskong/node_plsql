'use strict';

/**
* Module dependencies.
*/

const log = require('./log');

/**
* Error handler
*
* @param {Object} err Error object.
* @param {Object} req Request object.
* @param {Object} res Response object.
* @param {Object} next Response object.
* @api public
*/
function errorHandler(err, req, res/*, next*/) {
	let error = {
		title: '500 Internal Error',
		status: 500,
		error: err.message || /* istanbul ignore next */ '',
		url: req.url || /* istanbul ignore next */ '',
		stack: err.stack || /* istanbul ignore next */ ''
	};

	error.stack = error.stack.replace('\n\r', '\n').replace('\r\n', '\n').replace('\n', '<br>');

	/* istanbul ignore else */
	if (err.status === 404) {
		error.title = '404 Not Found';
		error.status = 404;
		error.error = 'Page not found';
	}

	log.error(error.title + ': ' + req.url);

	res.status(error.status);
	res.render('error', error);
}

module.exports = {
	errorHandler: errorHandler
};
