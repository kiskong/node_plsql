'use strict';

/**
 * @fileoverview Test for the module "request.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

//var debug = require('debug')('node_plsql:request:test');
var assert = require('chai').assert;
var _ = require('underscore');
var db = require('../lib/database');
var statistics = require('../lib/statistics');
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

describe('request.js', function () {

	var application = {
		options: {
			server: {
				port: 8999,
				static: [
					{
						mountPath: '/',
						physicalDirectory: './'
					},
					{
						mountPath: '/temp/',
						physicalDirectory: './temp'
					}
				],
				suppressOutput: true,
				requestLogging: true
			},
			services: [{
				route: 'sampleRoute',
				defaultPage: 'samplePage',
				databaseUsername: 'sampleUsername',
				databasePassword: 'samplePassword',
				databaseConnectString: 'sampleConnectString',
				documentTableName: 'sampleDoctable',
				invokeCallback: invokeCallback
			}]
		}
	};

	statistics.setStartup(application);

	// Create the connection pool
	db.createConnectionPools(application);

	it('no_para', function () {
		var req = {
			protocol: 'http',
			get: function () {
				return '';
			},
			connection: {},
			params: {
				name: 'no_para'
			},
			body: {},
			query: {}
		};
		var res = {};

		request.process(application, application.options.services[0], req, res);
	});

	it('scalar_para', function () {
		var req = {
			protocol: 'http',
			get: function () {
				return '';
			},
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

		request.process(application, application.options.services[0], req, res);
	});

	it('array_arguments', function () {
		var req = {
			protocol: 'http',
			get: function () {
				return '';
			},
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

		request.process(application, application.options.services[0], req, res);
	});

	it('invalid number of arguments', function () {
		assert.throws(function () {
			request.process(0);
		});
	});

	it('invalid types of arguments', function () {
		const CONSOLE_LOG = console.log;
		var req = {
			protocol: 'http',
			get: function () {
				return '';
			},
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

		console.log = function () {};
		request.process(application, application.options.services[0], req, res);
		console.log = CONSOLE_LOG;
	});
});

function invokeCallback(database, procedure, args, cgi, files, doctablename, callback) {
	assert.ok(	arguments.length === 7 &&
				_.isObject(database) &&
				_.isString(procedure) &&
				_.isObject(args) &&
				_.isObject(cgi) &&
				_.isArray(files) &&
				(_.isUndefined(doctablename) || _.isString(doctablename)) &&
				_.isFunction(callback)
				);

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
