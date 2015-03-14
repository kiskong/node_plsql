/**
* Module dependencies.
*/

var os = require('os');

/**
* Module variables.
*/

/**
* Create the HTTP_COOKIE string
*
* @param {Object} req Request
* @return {String} Cookie string
* @api private
*/
function getCookieString(req) {
	'use strict';

	var cookieString = '',
		propName = '';

	for (propName in req.cookies) {
		/* istanbul ignore else */
		if (req.cookies.hasOwnProperty(propName)) {
			cookieString += propName + '=' + req.cookies[propName] + ';';
		}
	}

	return cookieString;
}

/**
* Create a CGI object
*
* @param {Object} serverConfig Server configuration
* @param {Object} req Request
* @param {Object} service Service configuration
* @return {String} CGI object
* @api public
*/
function createCGI(serverConfig, req, service) {
	'use strict';

	if (typeof serverConfig !== 'object' || typeof req !== 'object' || typeof service !== 'object') {
		throw new Error('Invalid arguments');
	}

	var PROTOCOL = req.protocol;
	var CHARSET = 'UTF8';
	var IANA_CHARSET = 'UTF-8';
	var cookieString = getCookieString(req);

	var cgi = {
		'PLSQL_GATEWAY': 'node-plsql_server',
		'GATEWAY_IVERSION': '2',
		'SERVER_SOFTWARE': 'node.js',
		'GATEWAY_INTERFACE': 'CGI/1.1',
		'SERVER_PORT': serverConfig.port.toString(),
		'SERVER_NAME': os.hostname(),
		'REQUEST_METHOD': req.method,
		'PATH_INFO': req.params.name,
		'SCRIPT_NAME': service.route,
		'REMOTE_ADDR': req.headers['X-Real-IP'] || req.connection.remoteAddress,
		'SERVER_PROTOCOL': PROTOCOL + '/' + req.httpVersion,
		'REQUEST_PROTOCOL': PROTOCOL,
		'REMOTE_USER': '',
		'HTTP_USER_AGENT': req.headers['user-agent'],
		'HTTP_HOST': req.headers.host,
		'HTTP_ACCEPT': req.headers.accept,
		'HTTP_ACCEPT_ENCODING': req.headers['accept-encoding'],
		'HTTP_ACCEPT_LANGUAGE': req.headers['accept-language'],
		'HTTP_REFERER': '',
		'WEB_AUTHENT_PREFIX': '',
		'DAD_NAME': service.route,
		'DOC_ACCESS_PATH': '',
		'DOCUMENT_TABLE': service.documentTableName || '',
		'PATH_ALIAS': '',
		'REQUEST_CHARSET': CHARSET,
		'REQUEST_IANA_CHARSET': IANA_CHARSET,
		'SCRIPT_PREFIX': ''
	};

	// Add cookies
	if (cookieString.length > 0) {
		cgi.HTTP_COOKIE = cookieString;
	}

	return cgi;
}

module.exports = {
	createCGI: createCGI
};
