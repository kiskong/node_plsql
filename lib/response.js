/**
* Module dependencies.
*/

'use strict';

var debug = require('debug')('plsql-server:response');
var log = require('./log');
var debugBlock = require('./dump').debug;

/**
* Send a response
*
* @param {Object} res Express result object
* @param {String} headerMain Main header
* @param {String} headerOther Other header
* @param {Array} headerCookies Array of cokies
* @param {String} body Body
* @api public
*/
function send(res, headerMain, headerOther, headerCookies, body) {
	var cookie = {},
		i = 0;

	// trace
	log.trace('D) A RESPONSE WILL BE SEND (response.send)', {contentType: headerMain.contentType, contentLength: headerMain.contentLength, headerCookies: headerCookies, redirectLocation: headerMain.redirectLocation, body: body});

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
