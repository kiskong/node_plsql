'use strict';

/**
 * @fileoverview Test for the module "cgi.js"
 * @author doberkofler
 */

/* global describe: false, it:false */

// ------------------------------------------------------------------------------
// Requirements
// ------------------------------------------------------------------------------

const assert = require('chai').assert;
const cgi = require('../lib/cgi');
const os = require('os');

// ------------------------------------------------------------------------------
// Tests
// ------------------------------------------------------------------------------

describe('cgi.js', function () {

	describe('when calling createCGI()', function () {

		it('with an empty configuration object or request', function () {
			assert.throws(function () {
				cgi.createCGI({}, {});
			});
			assert.throws(function () {
				cgi.createCGI({a: 0}, {});
			});
			assert.throws(function () {
				cgi.createCGI({}, {a: 0});
			});
		});

		it('with a proper configuration object and request', function () {
			const ROUTE = 'route',
				DOCUMENT_TABLE_NAME = 'doc-table',
				REMOTE_ADDRESS = '127.0.0.1';

			const serverConfig = {
				server: {
					port: 4711
				}
			};

			const req = {
				protocol: 'http',
				method: 'GET',
				params: {
					name: 'index.html'
				},
				httpVersion: '1.1',
				ip: REMOTE_ADDRESS,
				get: function (name) {
					switch (name.toLowerCase())	{
						case 'user-agent':
							return 'USER-AGENT';
						case 'host':
							return 'HOST';
						case 'accept':
							return 'ACCEPT';
						case 'accept-encoding':
							return 'ACCEPT-ENCODING';
						case 'accept-language':
							return 'ACCEPT-LANGUAGE';
						case 'referer':
						case 'referrer':
							return 'HTTP-REFERER';
						default:
							return null;
					}
				},
				cookies: {
					cookie1: 'value1',
					cookie2: 'value2'
				},
				connection: {
					remoteAddress: REMOTE_ADDRESS
				}
			};

			const service = {
				route: ROUTE,
				documentTableName: DOCUMENT_TABLE_NAME
			};

			let cgiObject = cgi.createCGI(serverConfig, req, service);

			assert.strictEqual(28, Object.keys(cgiObject).length);
			assert.deepEqual(cgiObject, {
				'PLSQL_GATEWAY': 'WebDb',
				'GATEWAY_IVERSION': '2',
				'SERVER_SOFTWARE': 'NodeJS-PL/SQL-Gateway',
				'GATEWAY_INTERFACE': 'CGI/1.1',
				'SERVER_PORT': '4711',
				'SERVER_NAME': os.hostname(),
				'REQUEST_METHOD': 'GET',
				'PATH_INFO': 'index.html',
				'SCRIPT_NAME': ROUTE,
				'REMOTE_ADDR': REMOTE_ADDRESS,
				'SERVER_PROTOCOL': 'HTTP/1.1',
				'REQUEST_PROTOCOL': 'HTTP',
				'REMOTE_USER': '',
				'HTTP_USER_AGENT': 'USER-AGENT',
				'HTTP_HOST': 'HOST',
				'HTTP_ACCEPT': 'ACCEPT',
				'HTTP_ACCEPT_ENCODING': 'ACCEPT-ENCODING',
				'HTTP_ACCEPT_LANGUAGE': 'ACCEPT-LANGUAGE',
				'HTTP_REFERER': 'HTTP-REFERER',
				'WEB_AUTHENT_PREFIX': '',
				'DAD_NAME': ROUTE,
				'DOC_ACCESS_PATH': 'doc',
				'DOCUMENT_TABLE': DOCUMENT_TABLE_NAME,
				'PATH_ALIAS': '',
				'REQUEST_CHARSET': 'UTF8',
				'REQUEST_IANA_CHARSET': 'UTF-8',
				'SCRIPT_PREFIX': '/',
				'HTTP_COOKIE': 'cookie1=value1;cookie2=value2;'
			});

			service.documentTableName = '';
			cgiObject = cgi.createCGI(serverConfig, req, service);
			assert.strictEqual(cgiObject.DOCUMENT_TABLE, '');

		});
	});

});
