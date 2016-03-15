'use strict';

/**
 * @fileoverview Test for the module "request.js"
 * @author doberkofler
 */

/* global describe: false, it:false */

/**
* Module dependencies.
*/

//const debug = require('debug')('node_plsql:request:test');
const assert = require('chai').assert;
const _ = require('underscore');
const db = require('../lib/database');
const statistics = require('../lib/statistics');
const request = require('../lib/request');

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

	let res = {
		set: function () {
			return res;
		},
		status: function (status) {
			this.values.status = status;
			return res;
		},
		send: function () {
			return res;
		},
		cookie: function () {
			return res;
		},

		values: {
			status: null
		}
	};

	let application = {
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
		let req = {
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

		request.process(application, application.options.services[0], req, res);
	});

	it('scalar_para', function () {
		let req = {
			protocol: 'http',
			get: function () {
				return '';
			},
			connection: {},
			params: {
				name: 'scalar_para'
			},
			body: {},
			query: {
				p1: 'v1',
				p2: 'v2'
			}
		};

		request.process(application, application.options.services[0], req, res);
	});

	it('array_para', function () {
		let req = {
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

		request.process(application, application.options.services[0], req, res);
	});

	it('invalid number of arguments', function () {
		assert.throws(function () {
			request.process(0);
		});
	});

	it('invalid types of arguments', function () {
		let req = {
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

		request.process(application, application.options.services[0], req, res);
	});

	it('error in PL/SQL', function () {
		let req = {
			protocol: 'http',
			get: function () {
				return '';
			},
			connection: {},
			params: {
				name: 'error_in_plsql'
			},
			body: {}
		};

		initRes();
		request.process(application, application.options.services[0], req, res);
	});

	it('parse exception', function () {
		let req = {
			protocol: 'http',
			get: function () {
				return '';
			},
			connection: {},
			params: {
				name: 'parse_exception'
			},
			body: {}
		};

		initRes();
		request.process(application, application.options.services[0], req, res);
	});

	it('send exception', function () {
		let req = {
			protocol: 'http',
			get: function () {
				return '';
			},
			connection: {},
			params: {
				name: 'send_exception'
			},
			body: {}
		};


		initRes();
		request.process(application, application.options.services[0], req, res);
	});

	function initRes() {
		res.values.status = null;
	}

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
				callback(null, 'page');
				break;

			case 'scalar_para':
				assert.strictEqual(Object.keys(args).length, 2);
				assert.strictEqual(args.p1, 'v1');
				assert.strictEqual(args.p2, 'v2');
				callback(null, 'page');
				break;

			case 'array_para':
				assert.strictEqual(Object.keys(args).length, 3);
				assert.strictEqual(args.p1, 'v1');
				assert.strictEqual(args.p2, 'v2');
				assert.strictEqual(args.a1.length, 2);
				assert.strictEqual(args.a1[0], 'v1');
				assert.strictEqual(args.a1[1], 'v2');
				callback(null, 'page');
				break;

			case 'invalid_arguments':
				assert.strictEqual(Object.keys(args).length, 0);
				break;

			case 'error_in_plsql':
				callback(new Error('PL/SQL caused an error'), null);
				assert.strictEqual(res.values.status, 404);
				break;

			case 'parse_exception':
				callback(null); // missing page causes a parse exception!
				assert.strictEqual(res.values.status, 404);
				break;

			case 'send_exception':
				callback(null, 'Location: somewhere'); // using a location header will force the use of res.redirect and therefore cause an exception!
				assert.strictEqual(res.values.status, 404);
				break;

			default:
				throw new Error('Invalid procedure: ' + procedure);
		}
	}

});
