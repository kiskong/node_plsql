'use strict';

/**
 * @fileoverview Test for the module "response.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

//var debug = require('debug')('node_plsql:response:test');
var assert = require('chai').assert;
//var _ = require('underscore');
var response = require('../lib/response');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/

describe('response.js', function () {
	it('body', function () {
		var result = {
			headers: {},
			body: null
		};
		var message = {
			redirectLocation: null,
			contentType: null,
			contentLength: null,
			statusCode: null,
			statusDescription: null,
			headers: {},
			cookies: []
		};
		const cgiObj = {},
			req = {},
			res = {
				set: function (key, value) {
					result.headers[key] = value;
				},
				send: function (body) {
					result.body = body;
				}
			};

		response.send(message, cgiObj, req, res);
		assert.strictEqual(result.body, null, 'has no body');

		message.body = 'body';
		response.send(message, cgiObj, req, res);
		assert.strictEqual(result.body, 'body', 'has body');
	});

	it('invalid number of arguments', function () {
		assert.throws(function () {
			response.send(0);
		});
	});
});
