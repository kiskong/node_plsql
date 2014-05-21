/* jshint node: true */

/**
* Module dependencies.
*/

var createCGI = require('./cgi').createCGI;
var oracle = require('./oracle');
var header = require('./header');
var response = require('./response');

var debug = require('debug')('node_plsql:request');
var dumpBlock = require('./dump').block;

/**
* Module variables.
*/

/**
* Get the parameter
*
* @param {Object} req Request
* @return {Object} Object with the parameters as properties.
* @api private
*/
function getParameter(req)
{
	'use strict';

	// Prepare on object only containing the actual arguments
	var args = {};
	var argName = '';

	for (argName in req.query) {
		if (req.query.hasOwnProperty(argName)) {
			args[argName] = req.query[argName];
		}
	}

	return args;
}

/**
* Parse and send the results of a request
*
* @param {Object} res Result
* @param {String} page Page content
* @api private
*/
function parseAndRespond(res, page)
{
	'use strict';

	// Parse the page content and return the headers and the body
	var headerAndBody = header.getHeaderAndBody(page);

	// Parse the header
	var headerMain = {},
		headerOther = {},
		headerCookies = [];

	debug('Parsing header...');
	header.parseHeader(headerAndBody.header, headerMain, headerOther, headerCookies);
	if (debug.enabled) {
		console.log(dumpBlock('header main', JSON.stringify(headerMain, null, '\t')));
		console.log(dumpBlock('header other', JSON.stringify(headerOther, null, '\t')));
		console.log(dumpBlock('header cookies', JSON.stringify(headerCookies, null, '\t')));
	}

	// Send the response
	response.send(res, headerMain, headerOther, headerCookies, headerAndBody.body);
}

/**
* Process a request asynchroniously
*
* @param {Object} serverConfig Server configuration
* @param {Object} oracleSession Oracle binding object
* @param {String} username Oracle username
* @param {String} password Oracle password
* @param {Object} req Request
* @param {Object} res Result
* @api public
*/
function processRequest(serverConfig, oracleSession, username, password, req, res)
{
	'use strict';

	// Prepare on object only containing the actual arguments
	var args = getParameter(req);

	// Get the CGI
	var cgi = createCGI(serverConfig, req);
	if (debug.enabled) {
		console.log(dumpBlock('cgi', JSON.stringify(cgi, null, '\t')));
	}

	// Invoke the PL/SQL procedure
	debug('Invoking async "' + req.params.name + '" with the arguments "' + JSON.stringify(args) + '...');
	oracle.invoke(oracleSession, username, password, req.params.name, args, cgi, function (err, page) {
		debug('Returned from invokeProcedure "' + req.params.name + '" with the arguments "' + JSON.stringify(args));

		if (err) {
			throw new Error(err);
		} else {
			if (debug.enabled) {
				console.log(dumpBlock('page', page));
			}

			// Parse and send the results of a request
			parseAndRespond(res, page);
		}
	});
}

module.exports = {
	processRequest: processRequest
};
