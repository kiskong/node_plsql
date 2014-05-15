/* jshint node: true */

/**
* Module dependencies.
*/

var debug = require('debug')('plsql-server:response');
var dumpBlock = require('./dump').block;

/**
* Send a response
*
* @api public
*/
var send = function (res, headerMain, headerOther, headerCookies, body) {
	'use strict';

	var cookie = {},
		i = 0;

	// Set the content type
	debug('Set Content-Type to "' + headerMain.contentType + '"');
	res.set('Content-Type', headerMain.contentType);

	// Set the content length
	debug('Set Content-Length to "' + headerMain.contentLength + '"');
	res.set('Content-Length', headerMain.contentLength);

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
	if (debug.enabled) {
		console.log(dumpBlock('body', body));
	}
	res.send(body);
};

module.exports = {
	send: send
};
