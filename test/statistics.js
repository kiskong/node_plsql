'use strict';

/**
 * @fileoverview Test for the module "statistics.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

const assert = require('chai').assert;
const _ = require('underscore');
const statistics = require('../lib/statistics');


/**
* Module constants.
*/


/**
* Module variables.
*/

/**
* Tests.
*/

describe('statistics.js', function () {

	describe('setStartup', function () {
		it('thows an exception if the statistics have not yet been initialized', function () {
			let application = {};

			assert.throws(function () {
				statistics.requestStarted(application);
			});

			statistics.setStartup(application);

			assert.doesNotThrow(function () {
				statistics.requestStarted(application);
			});
		});

		it('should initialize the object and set the startup time', function () {
			const START = new Date();
			let application = {};

			statistics.setStartup(application);
			assert.isTrue(_.isDate(application.statistics.serverStartupTime));
			assert.isTrue(application.statistics.serverStartupTime >= START && application.statistics.serverStartupTime <= new Date());

			statistics.setStartup(application);
			assert.isTrue(_.isDate(application.statistics.serverStartupTime));
			assert.isTrue(application.statistics.serverStartupTime >= START && application.statistics.serverStartupTime <= new Date());
		});
	});

	describe('addRequest', function () {
		it('should initialize the object and add the timing for a request', function () {
			let application = {},
				req;

			statistics.setStartup(application);

			req = statistics.requestStarted(application);
			assert.strictEqual(application.statistics.requestStartedCount, 1);
			assert.isUndefined(application.statistics.requestCompletedCount);
			assert.isUndefined(application.statistics.requestDuration, 0);
			req.start -= 10;
			statistics.requestCompleted(application, req);
			assert.strictEqual(application.statistics.requestStartedCount, 1);
			assert.strictEqual(application.statistics.requestCompletedCount, 1);
			assert.isTrue(application.statistics.requestDuration >= 10, 0);
		});
	});

	describe('get', function () {
		it('should get the statistics object', function () {
			let application = {},
				req,
				s;

			// even the application parameter is missing but we still get some results
			s = statistics.get();
			assert.ok(_.isString(s.currentTime) && s.currentTime.length > 0);
			assert.strictEqual(s.serverStartupTime, '');
			assert.strictEqual(s.requestStartedCount, '');
			assert.strictEqual(s.requestCompletedCount, '');
			assert.strictEqual(s.averageRequestTime, '');

			// no statistics at all
			s = statistics.get(application);
			assert.ok(_.isString(s.currentTime) && s.currentTime.length > 0);
			assert.strictEqual(s.serverStartupTime, '');
			assert.strictEqual(s.requestStartedCount, '');
			assert.strictEqual(s.requestCompletedCount, '');
			assert.strictEqual(s.averageRequestTime, '');

			statistics.setStartup(application);

			statistics.setStartup(application);
			s = statistics.get(application);
			assert.ok(_.isString(s.serverStartupTime) && s.serverStartupTime.length > 0);
			assert.strictEqual(s.requestStartedCount, '');
			assert.strictEqual(s.requestCompletedCount, '');
			assert.strictEqual(s.averageRequestTime, '');

			// a request that started but was never completed
			req = statistics.requestStarted(application);

			// a request that completed in 10ms or more
			req = statistics.requestStarted(application);
			req.start -= 10;
			statistics.requestCompleted(application, req);

			// a request that completed in 100ms or more
			req = statistics.requestStarted(application);
			req.start -= 100;
			statistics.requestCompleted(application, req);

			s = statistics.get(application);
			assert.strictEqual(s.requestStartedCount, '3');
			assert.strictEqual(s.requestCompletedCount, '2');
			assert.strictEqual(s.averageRequestTime, '55ms');
		});
	});

});
