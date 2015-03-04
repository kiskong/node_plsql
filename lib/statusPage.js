/**
* Module dependencies.
*/

var statistics = require('./statistics');
var debug = require('debug')('node_plsql:statusPage');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Process the status page request
*
* @param {Object} app Express application
* @param {Object} req Request object
* @param {Object} res Response object
* @api public
*/
function process(app, req, res) {
	'use strict';

	debug('process');

	var stats = statistics.get(app);

	res.render('status', {
		stats: stats,
		title: 'node_plsql - Status Page',
		header: 'node_plsql - Status Page'
	});
}

module.exports = {
	process: process
};
