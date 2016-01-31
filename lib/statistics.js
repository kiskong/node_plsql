'use strict';

/**
* Module dependencies.
*/

var humanize = require('ms');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Set startup time
*
* @param {Object} app Express application
* @api public
*/
function setStartup(app) {
	if (!app.statistics) {
		app.statistics = {};
	}

	app.statistics.startup = new Date();
}

/**
* Mark a requets start
*
* @param {Object} app Express application
* @return {Object} Request stats object
* @api public
*/
function requestStarted(app) {
	if (!app.statistics) {
		app.statistics = {};
	}

	if (!app.statistics.requestStartedCount) {
		app.statistics.requestStartedCount = 1;
	} else {
		app.statistics.requestStartedCount++;
	}

	return {
		id: app.statistics.requestStartedCount,
		start: new Date()
	};
}

/**
* Mark the completion of a request
*
* @param {Object} app Express application
* @param {Object} reqStatus status information
* @api public
*/
function requestCompleted(app, reqStatus) {
	var duration;

	if (!app || !app.statistics || !reqStatus || !reqStatus.start) {
		throw new Error('Invalid state of statistics object!');
	}

	duration = new Date() - reqStatus.start;

	if (!app.statistics.requestCompletedCount) {
		app.statistics.requestCompletedCount = 1;
	} else {
		app.statistics.requestCompletedCount++;
	}

	if (!app.statistics.requestDuration) {
		app.statistics.requestDuration = duration;
	} else {
		app.statistics.requestDuration += duration;
	}
}

/**
* Get statistcal values
*
* @param {Object} app Express application
* @return {Object} Applications statistics
* @api public
*/
function get(app) {
	var s;

	if (app && app.statistics) {
		s = app.statistics;
	} else {
		throw new Error('Invalid app parameter');
	}

	return {
		startup: s.startup ? s.startup.toString() : '',
		running: s.startup ? humanize(new Date() - s.startup) : '',
		requestStartedCount: s.requestStartedCount ? s.requestStartedCount.toString() : '',
		requestCompletedCount: s.requestCompletedCount ? s.requestCompletedCount.toString() : '',
		activeRequestCount: s.requestCompletedCount ? (s.requestCompletedCount - s.requestStartedCount).toString() : '',
		averageRequestTime: s.requestCompletedCount ? humanize(Math.round(s.requestDuration / s.requestCompletedCount)) : ''
	};
}

module.exports = {
	setStartup: setStartup,
	requestStarted: requestStarted,
	requestCompleted: requestCompleted,
	get: get
};
