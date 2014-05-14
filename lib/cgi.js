/* jshint node: true */

/**
* Module dependencies.
*/

/**
* Module variables.
*/

/**
* Create a new oracle connection
*
* @api private
*/
/**
* Create a CGI object
*
* @api public
*/
var createCGI = function (serverConfig, req) {
	'use strict';

	var PROTOCOL = 'http'; // ???
	var CHARSET = 'UTF8';
	var IANA_CHARSET = 'UTF-8';

	var cgi = {
		'PLSQL_GATEWAY': 'node-plsql_server',
		'GATEWAY_IVERSION': '2',
		'SERVER_SOFTWARE': 'node.js',
		'GATEWAY_INTERFACE': 'CGI/1.1',
		'SERVER_PORT': serverConfig.port.toString(),
		//'SERVER_NAME': 'QLT400',
		'REQUEST_METHOD': req.method,
		'PATH_INFO': req.params.name,
		//'SCRIPT_NAME': '/pls/lj_unittest',
		'REMOTE_ADDR': req._remoteAddress,
		'SERVER_PROTOCOL': PROTOCOL + '/' + req.httpVersion,
		'REQUEST_PROTOCOL': PROTOCOL,
		//'REMOTE_USER': 'LJ_UNITTEST',
		'HTTP_USER_AGENT': req.headers['user-agent'],
		'HTTP_HOST': req.headers.host,
		'HTTP_ACCEPT': req.headers.accept,
		'HTTP_ACCEPT_ENCODING': req.headers['accept-encoding'],
		'HTTP_ACCEPT_LANGUAGE': req.headers['accept-language'],
		//'HTTP_REFERER': 'http://localhost:7777/pls/lj_unittest/LAS_DLG_Admin.MainAdministration',
		//'WEB_AUTHENT_PREFIX': '',
		//'DAD_NAME': '',
		//'DOC_ACCESS_PATH': 'docs',
		//'DOCUMENT_TABLE': 'LJP_Documents',
		//'PATH_ALIAS': '',
		'REQUEST_CHARSET': CHARSET,
		'REQUEST_IANA_CHARSET': IANA_CHARSET,
		'SCRIPT_PREFIX': ''
	};

	return cgi;
};

module.exports = {
	createCGI: createCGI
};
