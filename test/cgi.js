/**
 * @fileoverview Test for the module "cgi.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


// ------------------------------------------------------------------------------
// Requirements
// ------------------------------------------------------------------------------

var assert = require('chai').assert;
var createCGI = require('../lib/cgi').createCGI;

var os = require('os');

// ------------------------------------------------------------------------------
// Tests
// ------------------------------------------------------------------------------
describe('cgi', function () {
	'use strict';

	describe('when calling createCGI()', function () {
		it('with an empty configuration object or request', function () {
			assert.throws(function () {
				createCGI({}, {});
			});
			assert.throws(function () {
				createCGI({a: 0}, {});
			});
			assert.throws(function () {
				createCGI({}, {a: 0});
			});
		});
		it('with a proper configuration object and request', function () {
			var ROUTE = 'route',
				DOCUMENT_TABLE_NAME = 'doc-table',
				REMOTE_ADDRESS = '';

			var serverConfig = {
				port: 4711
			},
			req = {
				protocol: 'http',
				method: 'GET',
				params: {
					name: 'index.html'
				},
				httpVersion: '1.1',
				headers: {
					'user-agent': 'USER-AGENT',
					host: 'HOST',
					accept: 'ACCEPT',
					'accept-encoding': 'ACCEPT-ENCODING',
					'accept-language': 'ACCEPT-LANGUAGE'
				},
				cookies: {
					cookie1: 'value1',
					cookie2: 'value2'
				},
				connection: {
					remoteAddress: REMOTE_ADDRESS
				}
			},
			service = {
				route: ROUTE,
				documentTableName: DOCUMENT_TABLE_NAME
			};

			var cgi = createCGI(serverConfig, req, service);
			assert.strictEqual(28, Object.keys(cgi).length);
			assert.deepEqual(cgi, {
				'PLSQL_GATEWAY': 'node-plsql_server',
				'GATEWAY_IVERSION': '2',
				'SERVER_SOFTWARE': 'node.js',
				'GATEWAY_INTERFACE': 'CGI/1.1',
				'SERVER_PORT': '4711',
				'SERVER_NAME': os.hostname(),
				'REQUEST_METHOD': 'GET',
				'PATH_INFO': 'index.html',
				'SCRIPT_NAME': ROUTE,
				'REMOTE_ADDR': REMOTE_ADDRESS,
				'SERVER_PROTOCOL': 'http/1.1',
				'REQUEST_PROTOCOL': 'http',
				'REMOTE_USER': '',
				'HTTP_USER_AGENT': 'USER-AGENT',
				'HTTP_HOST': 'HOST',
				'HTTP_ACCEPT': 'ACCEPT',
				'HTTP_ACCEPT_ENCODING': 'ACCEPT-ENCODING',
				'HTTP_ACCEPT_LANGUAGE': 'ACCEPT-LANGUAGE',
				'HTTP_REFERER': '',
				'WEB_AUTHENT_PREFIX': '',
				'DAD_NAME': ROUTE,
				'DOC_ACCESS_PATH': '',
				'DOCUMENT_TABLE': DOCUMENT_TABLE_NAME,
				'PATH_ALIAS': '',
				'REQUEST_CHARSET': 'UTF8',
				'REQUEST_IANA_CHARSET': 'UTF-8',
				'SCRIPT_PREFIX': '',
				'HTTP_COOKIE': 'cookie1=value1;cookie2=value2;'
			});
		});
	});

});
