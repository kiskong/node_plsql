'use strict';

/**
* Module dependencies.
*/

const statistics = require('./statistics');
const log = require('./log');
const debug = require('debug')('node_plsql:statusPage');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Process the status page request
*
* @param {Object} application - node_plsql application
* @param {Object} req Request object
* @param {Object} res Response object
* @api public
*/
function process(application, req, res) {
	let stats;

	debug('process');

	try {
		stats = statistics.get(application);
	} catch (e) {
		/* istanbul ignore next */
		log.error(e);
	}

	res.render('status', {
		stats: stats
	});
}

module.exports = {
	process: process
};
