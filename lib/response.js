'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:response');
const util = require('util');
const _ = require('underscore');
const log = require('./log');

/**
* Send a response
*
* @param {Object} message - A message object
* @param {Object} cgiObj - CGI object
* @param {Object} req - express request object
* @param {Object} res - express response object
* @api public
*/
function send(message, cgiObj, req, res) {
	// debug
	debug('send');

	// validate
	if (arguments.length !== 4) {
		log.exit(new Error('invalid arguments'));
	}

	// add "Server" header
	message.headers.Server = cgiObj.SERVER_SOFTWARE;

	// trace
	log.trace('D) A RESPONSE WILL BE SEND (response.send)', message);

	// Iterate over the array of cookies
	_.each(message.cookies, function (cookie) {
		debug('Set cookies: ' + util.inspect(cookie, {showHidden: false, depth: null, colors: true}));
		res.cookie(cookie.name, cookie.value, _.omit(cookie, ['name', 'value']));
	});

	// Is the a "redirectLocation" header
	if (typeof message.redirectLocation === 'string' && message.redirectLocation.length > 0) {
		/*
		if (message.redirectLocation.indexOf('/') !== 0 && message.redirectLocation.indexOf('http://') !== 0) {
			debug('Converting relative path "' + message.redirectLocation + '" to absolute path');
			message.redirectLocation = cgiObj.SCRIPT_NAME + '/' + message.redirectLocation;
		}
		*/

		debug('Redirecting to "' + message.redirectLocation + '"');
		res.redirect(302, message.redirectLocation);
		return;
	}

	// Is the a "contentType" header
	if (typeof message.contentType === 'string' && message.contentType.length > 0) {
		debug('Set "Content-Type" to "' + message.contentType + '"');
		res.set('Content-Type', message.contentType);
	}

	// Iterate over the headers object
	_.each(message.headers, function (value, key) {
		debug('Set "' + key + '" to "' + value + '"');
		res.set(key, value);
	});

	// Process the body
	if (typeof message.body === 'string') {
		debug('Send body "' + message.body.substring(0, 30).replace('\n', '\\n') + '"');
		res.send(message.body);
	}
}

module.exports = {
	send: send
};
