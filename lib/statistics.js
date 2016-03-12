'use strict';

/**
* Module dependencies.
*/
var humanize = require('ms');
var os = require('os');
var _ = require('underscore');
var log = require('./log');
var debug = require('debug')('node_plsql:statistics');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Set startup time
*
* @param {Object} application - node_plsql application
* @api public
*/
function setStartup(application) {
	application.statistics = application.statistics || {};
	application.statistics.serverStartupTime = new Date();
}

/**
* Mark a requets start
*
* @param {Object} application - node_plsql application
* @return {Object} Request stats object
* @api public
*/
function requestStarted(application) {
	if (!application.statistics.requestStartedCount) {
		application.statistics.requestStartedCount = 1;
	} else {
		application.statistics.requestStartedCount++;
	}

	return {
		id: application.statistics.requestStartedCount,
		start: new Date()
	};
}

/**
* Mark the completion of a request
*
* @param {Object} application - node_plsql application
* @param {Object} reqStatus - Request object
* @api public
*/
function requestCompleted(application, reqStatus) {
	var duration;

	/* istanbul ignore if */
	if (!application.statistics || !reqStatus || !reqStatus.start) {
		log.exit(new Error('Invalid state of statistics object!'));
	}

	duration = new Date() - reqStatus.start;

	if (!application.statistics.requestCompletedCount) {
		application.statistics.requestCompletedCount = 1;
	} else {
		application.statistics.requestCompletedCount++;
	}

	if (!application.statistics.requestDuration) {
		application.statistics.requestDuration = duration;
	} else {
		application.statistics.requestDuration += duration;
	}
}

/**
* Get stats
*
* @param {Object} application - node_plsql application
* @return {Object} Applications statistics
* @api public
*/
function get(application) {
	const NOW = new Date();
	var stats = {
			currentTime: NOW.toString(),
			serverStartupTime: '',
			running: '',
			requestStartedCount: '',
			requestCompletedCount: '',
			activeRequestCount: '',
			averageRequestTime: '',
			oracleDbVersion: '',
			oracleClientVersion: '',
			services: []
		},
		service,
		url,
		i;

	debug('get');

	if (_.isObject(application) && _.isObject(application.statistics)) {
		/* istanbul ignore else */
		if (_.isDate(application.statistics.serverStartupTime)) {
			stats.serverStartupTime = application.statistics.serverStartupTime.toString();
			stats.running = humanize(NOW - application.statistics.serverStartupTime);
		}

		if (_.isNumber(application.statistics.requestStartedCount)) {
			stats.requestStartedCount = application.statistics.requestStartedCount.toString();
		}
		if (_.isNumber(application.statistics.requestCompletedCount)) {
			stats.requestCompletedCount = application.statistics.requestCompletedCount.toString();
			stats.averageRequestTime = humanize(Math.round(application.statistics.requestDuration / application.statistics.requestCompletedCount));
		}
		if (_.isNumber(application.statistics.requestStartedCount) && _.isNumber(application.statistics.requestCompletedCount)) {
			stats.activeRequestCount = (application.statistics.requestCompletedCount - application.statistics.requestStartedCount).toString();
		}

		if (_.isString(application.statistics.oracleDbVersion)) {
			stats.oracleDbVersion = application.statistics.oracleDbVersion;
		}
		if (_.isString(application.statistics.oracleClientVersion)) {
			stats.oracleClientVersion = application.statistics.oracleClientVersion;
		}

		if (_.isObject(application.options) && _.isArray(application.options.services)) {
			for (i = 0; i < application.options.services.length; i++) {
				service = application.options.services[i];
				url = 'http://' + os.hostname() + ':' + application.options.server.port.toString() + '/' + service.route;

				try {
					stats.services.push({
						title: url,
						url: url,
						stats: _getServiceStats(service)
					});
				} catch (e) {/**/}
			}
		}
	}

	return stats;
}

/**
* Get stats for a service
*
* @param {Object} service - Service object
* @return {Array} Array of stats for a service
* @api public
*/
function _getServiceStats(service) {
	var stats = [];

	stats.push({
		name: 'Default page',
		value: service.defaultPage
	});

	stats.push({
		name: 'Oracle username',
		value: service.databaseUsername
	});

	stats.push({
		name: 'Oracle connect string',
		value: service.databaseConnectString
	});

	stats.push({
		name: 'Oracle server version',
		value: service.database.oracleServerVersion
	});

	stats.push({
		name: 'Oracle minimum number of connections a connection pool maintains',
		value: service.database.connectionPool.poolMin
	});

	stats.push({
		name: 'Oracle maximum number of connections that can be open in the connection pool',
		value: service.database.connectionPool.poolMax
	});

	stats.push({
		name: 'Oracle number of connections that are opened whenever a connection request exceeds the number of currently open connections',
		value: service.database.connectionPool.poolIncrement
	});

	stats.push({
		name: 'Oracle time (in seconds) after which the pool terminates idle connections',
		value: service.database.connectionPool.poolTimeout
	});

	stats.push({
		name: 'Oracle number of currently active connections in the connection pool',
		value: service.database.connectionPool.connectionsInUse
	});

	stats.push({
		name: 'Oracle The number of currently open connections in the underlying connection pool',
		value: service.database.connectionPool.connectionsOpen
	});

	return stats;
}

module.exports = {
	setStartup: setStartup,
	requestStarted: requestStarted,
	requestCompleted: requestCompleted,
	get: get
};
