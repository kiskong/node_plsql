/**
 * @fileoverview Test for the module "cgi.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


//------------------------------------------------------------------------------
// Requirements
//------------------------------------------------------------------------------

var assert = require('chai').assert;
var createCGI = require('../lib/cgi').createCGI;


//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
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
			var serverConfig = {
				port: 4711
			},
			req = {
				method: 'GET',
				params: {
					name: 'index.html'
				},
				_remoteAddress: '1.2.3.4',
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
				}
			};
			var cgi = createCGI(serverConfig, req);
			assert.deepEqual(cgi, {
				'PLSQL_GATEWAY': 'node-plsql_server',
				'GATEWAY_IVERSION': '2',
				'SERVER_SOFTWARE': 'node.js',
				'GATEWAY_INTERFACE': 'CGI/1.1',
				'SERVER_PORT': '4711',
				'REQUEST_METHOD': 'GET',
				'PATH_INFO': 'index.html',
				'REMOTE_ADDR': '1.2.3.4',
				'SERVER_PROTOCOL': 'http/1.1',
				'REQUEST_PROTOCOL': 'http',
				'HTTP_USER_AGENT': 'USER-AGENT',
				'HTTP_HOST': 'HOST',
				'HTTP_ACCEPT': 'ACCEPT',
				'HTTP_ACCEPT_ENCODING': 'ACCEPT-ENCODING',
				'HTTP_ACCEPT_LANGUAGE': 'ACCEPT-LANGUAGE',
				'REQUEST_CHARSET': 'UTF8',
				'REQUEST_IANA_CHARSET': 'UTF-8',
				'SCRIPT_PREFIX': '',
				'HTTP_COOKIE': 'cookie1=value1;cookie2=value2;'
			});
		});
	});

});
