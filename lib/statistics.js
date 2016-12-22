'use strict';

/**
* Module dependencies.
*/
const humanize = require('ms');
const os = require('os');
const _ = require('underscore');
const log = require('./log');
const debug = require('debug')('node_plsql:statistics');

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
	let duration;

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
	let stats = {
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

				stats.services.push({
					title: url,
					url: url,
					stats: _getServiceStats(service)
				});
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
	let stats = [];

	function add(name, value) {
		stats.push({name: name, value: value});
	}

	add('Default page', service.defaultPage);

	add('Authentication mode', service.authenticationMode);

	add('Oracle username', service.databaseUsername);

	add('Oracle connect string', service.databaseConnectString);

	try {
		add('Oracle server version', service.database.oracleServerVersion);
	} catch (e) {/**/}

	try {
		add('Minimum number of connections in pool', service.database.connectionPool.poolMin);
	} catch (e) {/**/}

	try {
		add('Maximum number of connections in pool', service.database.connectionPool.poolMax);
	} catch (e) {/**/}

	try {
		add('Connection number increase in pool', service.database.connectionPool.poolIncrement);
	} catch (e) {/**/}

	try {
		add('Timeout in seconds for idle connections', service.database.connectionPool.poolTimeout);
	} catch (e) {/**/}

	try {
		add('Number of active connections in pool', service.database.connectionPool.connectionsInUse);
	} catch (e) {/**/}

	try {
		add('Number of open connections in pool', service.database.connectionPool.connectionsOpen);
	} catch (e) {/**/}

	return stats;
}

module.exports = {
	setStartup: setStartup,
	requestStarted: requestStarted,
	requestCompleted: requestCompleted,
	get: get
};
