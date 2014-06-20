/* jshint node: true */

/**
* Module dependencies.
*/

var debug = require('debug')('plsql-server:response');
var debugBlock = require('./dump').debug;

/**
* Send a response
*
* @api public
*/
function send(res, headerMain, headerOther, headerCookies, body)
{
	'use strict';

	var cookie = {},
		i = 0;

	// Set the content type
	if (headerMain.contentType) {
		debug('Set Content-Type to "' + headerMain.contentType + '"');
		res.set('Content-Type', headerMain.contentType);
	}

	// Set the content length
	if (headerMain.contentLength) {
		debug('Set Content-Length to "' + headerMain.contentLength + '"');
		res.set('Content-Length', headerMain.contentLength);
	}

	// Set the cookies
	for (i = 0; i < headerCookies.length; i++) {
		cookie = headerCookies[i];
		debug('Set cookie to "' + JSON.stringify(headerCookies[i]) + '"');
		res.cookie(cookie.key, cookie.value, {'path': ''});
	}

	// Set a redirect
	if (headerMain.redirectLocation && headerMain.redirectLocation.length > 0) {
		debug('Redirecting to "' + headerMain.redirectLocation + '"');
		res.redirect(302, headerMain.redirectLocation);
		return;
	}

	// Send the body
	debugBlock('BODY', body, debug);
	res.send(body);
}

module.exports = {
	send: send
};
