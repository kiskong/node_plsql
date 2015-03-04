/**
 * @fileoverview Test for the module "statistics.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var statistics = require('../lib/statistics');


/**
* Module constants.
*/


/**
* Module variables.
*/

/**
* Tests.
*/

describe('statistics', function () {
	'use strict';

	describe('setStartup', function () {
		it('should initialize the object and set the startup time', function () {

			var app = {},
				start = new Date();

			statistics.setStartup(app);
			assert.strictEqual(Object.prototype.toString.call(app.statistics.startup), '[object Date]');
			assert.isTrue(app.statistics.startup >= start && app.statistics.startup <= new Date());

			statistics.setStartup(app);
			assert.strictEqual(Object.prototype.toString.call(app.statistics.startup), '[object Date]');
			assert.isTrue(app.statistics.startup >= start && app.statistics.startup <= new Date());
		});
	});

	describe('addRequest', function () {
		it('should initialize the object and add the timeing for a request', function () {

			var app = {},
				req;

			req = statistics.requestStarted(app);
			assert.strictEqual(app.statistics.requestStartedCount, 1);
			assert.isUndefined(app.statistics.requestCompletedCount);
			assert.isUndefined(app.statistics.requestDuration, 0);
			req.start -= 10;
			statistics.requestCompleted(app, req);
			assert.strictEqual(app.statistics.requestStartedCount, 1);
			assert.strictEqual(app.statistics.requestCompletedCount, 1);
			assert.isTrue(app.statistics.requestDuration >= 10, 0);
		});
	});

	describe('get', function () {
		it('should get the statistics object', function () {

			var app = {},
				req,
				s;

			assert.throws(function () {
				s = statistics.get();
			});

			assert.throws(function () {
				s = statistics.get(app);
			});

			assert.throws(function () {
				s = statistics.requestCompleted(app);
			});

			app.statistics = {};
			s = statistics.get(app);
			assert.strictEqual(s.startup, '');
			assert.strictEqual(s.requestStartedCount, '');
			assert.strictEqual(s.requestCompletedCount, '');
			assert.strictEqual(s.averageRequestTime, '');

			statistics.setStartup(app);
			s = statistics.get(app);
			assert.strictEqual(Object.prototype.toString.call(s.startup), '[object String]');
			assert.strictEqual(s.requestStartedCount, '');
			assert.strictEqual(s.requestCompletedCount, '');
			assert.strictEqual(s.averageRequestTime, '');

			// a request that started but was never completed
			req = statistics.requestStarted(app);

			// a request that completed in 10ms or more
			req = statistics.requestStarted(app);
			req.start -= 10;
			statistics.requestCompleted(app, req);

			// a request that completed in 100ms or more
			req = statistics.requestStarted(app);
			req.start -= 100;
			statistics.requestCompleted(app, req);

			s = statistics.get(app);
			assert.strictEqual(s.requestStartedCount, '3');
			assert.strictEqual(s.requestCompletedCount, '2');
			assert.strictEqual(s.averageRequestTime, '55ms');
		});
	});

});
