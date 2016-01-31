/**
 * @fileoverview Test for the module "request.js"
 * @author doberkofler
 */

'use strict';


/* global describe: false, it:false */


/**
* Module dependencies.
*/

// var debug = require('debug')('test/server');
var assert = require('chai').assert;
// var util = require('util');
var request = require('../lib/request');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/

describe('request', function () {
	var config = {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: './'
			}, {
				mountPath: '/temp/',
				physicalDirectory: './temp'
			}],
			suppressOutput: true,
			requestLogging: true,
			oracleConnectionPool: true,
			oracleDebug: false
		},
		services: [{
			route: 'sampleRoute',
			defaultPage: 'samplePage',
			databaseUsername: 'sampleUsername',
			databasePassword: 'samplePassword',
			databaseConnectString: 'sampleConnectString',
			documentTableName: 'sampleDoctable'
		}]
	};
	var app = {
		databaseHandle: {
			callbacks: {
				databaseInvoke: function (databaseHandle, username, password, procedure, args/*, cgi, files, doctablename, callback*/) {
					switch (procedure) {
						case 'no_para':
							assert.strictEqual(Object.keys(args).length, 0);
							break;
						case 'scalar_para':
							assert.strictEqual(Object.keys(args).length, 1);
							assert.strictEqual(args.p1, 'v1');
							assert.strictEqual(args.p2, 'v2');
							break;
						case 'array_arguments':
							assert.strictEqual(Object.keys(args).length, 2);
							assert.strictEqual(args.p1, 'v1');
							assert.strictEqual(args.p2, 'v2');
							assert.strictEqual(args.a1.length, 2);
							assert.strictEqual(args.a1[0], 'v1');
							assert.strictEqual(args.a1[1], 'v2');
							break;
						case 'invalid_arguments':
							assert.strictEqual(Object.keys(args).length, 0);
							break;
						default:
							break;
					}
				}
			}
		}
	};

	it('no_para', function (done) {
		var req = {
			headers: {},
			connection: {},
			params: {
				name: 'no_para'
			},
			body: {},
			query: {}
		};
		var res = {};

		request.processRequest(app, config.server, config.services[0], req, res);
		done();
	});

	it('scalar_para', function (done) {
		var req = {
			headers: {},
			connection: {},
			params: {
				name: 'single_para'
			},
			body: {},
			query: {
				p1: 'v1',
				p2: 'v2'
			}
		};
		var res = {};

		request.processRequest(app, config.server, config.services[0], req, res);
		done();
	});

	it('array_arguments', function (done) {
		var req = {
			headers: {},
			connection: {},
			params: {
				name: 'array_para'
			},
			body: {
				a1: ['v1', 'v2']
			},
			query: {
				p1: 'v1',
				p2: 'v2'
			}
		};
		var res = {};

		request.processRequest(app, config.server, config.services[0], req, res);
		done();
	});

	it('invalid_arguments', function (done) {
		var req = {
			headers: {},
			connection: {},
			params: {
				name: 'invalid_arguments'
			},
			body: {
				a1: true,
				a2: {},
				a3: 1,
				a4: [1, 2]
			}
		};
		var res = {};

		request.processRequest(app, config.server, config.services[0], req, res);
		done();
	});
});
