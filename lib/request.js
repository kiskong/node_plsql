'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:request');
const path = require('path');
const util = require('util');
const _ = require('underscore');
const log = require('./log');
const cgi = require('./cgi');
const parse = require('./parse');
const response = require('./response');
const database = require('./database');
const statistics = require('./statistics');

/**
* Module variables.
*/

let sequencialID = 0;

/**
* Process a request asynchroniously
*
* @param {Object} application - node_plsql application
* @param {Object} service Service configuration
* @param {Object} req Request object
* @param {Object} res Response object
* @api public
*/
function process(application, service, req, res) {
	let args = {},
		fields = {},
		files = [],
		requestStatus,
		cgiObj;

	// debug
	debug('process: start');

	// trace
	log.trace('A) A REQUEST WAS RECEIVED (request.process)', _.pick(req, ['_startTime', 'body', 'complete', 'cookies', 'files', 'headers', 'httpversion', 'method', 'originalUrl', 'params', 'query', 'signedCookies', 'url']));

	// validate
	if (arguments.length !== 4) {
		log.exit(new Error('process: invalid argument number: ' + arguments.length));
	}

	// Get the CGI
	cgiObj = cgi.createCGI(application.options, req, service);

	// Does the request contain any files
	if (req.hasOwnProperty('files') && _.keys(req.files).length > 0) {
		// Get the files
		_getFiles(req.files, fields, files);
		args = _.extend(args, fields);
	}

	// Does the request contain a body
	/* istanbul ignore else */
	if (req.hasOwnProperty('body')) {
		fields = _normalizeBody(req.body);
		args = _.extend(args, fields);
	}

	// Prepare on object only containing the actual arguments
	args = _.extend(args, _getParameter(req));

	// Mark a request as started in the statistics
	requestStatus = statistics.requestStarted(application);

	// Invoke the PL/SQL procedure
	database.invoke(service, req.params.name, args, cgiObj, files, service.documentTableName, function pageReceived(err, page) {
		debug('process: pageReceived');

		// Mark a request as completed in the statistics
		statistics.requestCompleted(application, requestStatus);

		// if we have an error object, we throw an exception as something most have gone wrong and we want to log it
		if (err) {
			_reportRequestError(err, req);
			_pageProcessError(err, req, res);
		} else {
			_pageProcessSuccess(page, cgiObj, req, res);
		}
	});
}

/**
* Process the successful page and send the response
*
* @param {String} page - Page content
* @param {Object} cgiObj - CGI object
* @param {Object} req - express request object
* @param {Object} res - express response object
* @api private
*/
function _pageProcessSuccess(page, cgiObj, req, res) {
	let message;

	// debug
	debug('_pageProcessSuccess');

	// validate
	/* istanbul ignore next */
	if (arguments.length !== 4) {
		log.exit(new Error('_pageProcessSuccess: invalid argument number: ' + arguments.length));
	}

	// trace
	log.trace('C) A STORED PROCEDURE RETURN A PAGE (request._pageProcessSuccess)', page);

	// parse the page returned by the PL/SQL procedure
	message = parse.parseContent(page);

	// Send the response
	response.send(message, cgiObj, req, res);
}

/**
* Process the error page and send the response
*
* @param {Error} err Error content
* @param {Object} req - express request object
* @param {Object} res - express response object
* @api private
*/
function _pageProcessError(err, req, res) {
	let lines,
		html,
		len,
		i;

	// debug
	debug('_pageProcessError');

	// validate
	/* istanbul ignore next */
	if (arguments.length !== 3) {
		log.exit(new Error('_pageProcessError: invalid argument number: ' + arguments.length));
	}
	/* istanbul ignore next */
	if (!(err instanceof Error)) {
		log.exit(new Error('_pageProcessError: err argument must be an instance of Error'));
	}

	// trace
	log.trace('C) A STORED PROCEDURE REPORTED AN ERROR (request._pageProcessError)');

	// Create html error message
	html = '<html><head><title>Failed to parse target procedure</title></head><body><h1>Failed to parse target procedure</h1>';

	try {
		html += '<p>';
		lines = err.message.split('\n');
		len = lines.length;
		for (i = 0; i < len; i++) {
			html += lines[i] + '<br/>';
		}
		html += '</p>';
	} catch (e) {
		// we do not care!
	}

	html += '</body></html>';

	// Send the response
	res.status(404).send(html);
}

/**
* Normalize the body by making sure that only "simple" parameters and no nested objects are submitted
*
* @param {Object} body The body of the request with all the parameters.
* @return {Object} Object with the parameters as properties.
* @api private
*/
function _normalizeBody(body) {
	// Prepare on object only containing the actual arguments
	let argName = '',
		type = '',
		args = {};

	// This more generic type check is taken from "http://javascriptweblog.wordpress.com/2011/08/08/fixing-the-javascript-typeof-operator"
	function toType(obj) {
		return ({}).toString.call(obj).match(/\s([a-zA-Z]+)/)[1].toLowerCase();
	}

	function isArrayOfString(obj) {
		let i;

		if (util.isArray(obj) !== true) {
			return false;
		}

		for (i = 0; i < obj.length; i++) {
			if (typeof obj[i] !== 'string') {
				return false;
			}
		}

		return true;
	}

	for (argName in body) {
		/* istanbul ignore else */
		if (body.hasOwnProperty(argName)) {
			type = toType(body[argName]);
			/* istanbul ignore else */
			if (type === 'string' || isArrayOfString(body[argName])) {
				args[argName] = body[argName];
			} else {
				log.error('The element "' + argName + '" in the request is not a string or an array of strings but rather of type "' + type + '" and cannot be processed!', util.inspect(body, {showHidden: false, depth: null, colors: false}));
			}
		}
	}

	return args;
}

/**
* Get the parameter
*
* @param {Object} req Request
* @return {Object} Object with the parameters as properties.
* @api private
*/
function _getParameter(req) {
	// Prepare on object only containing the actual arguments
	let args = {};
	let argName = '';

	for (argName in req.query) {
		/* istanbul ignore else */
		if (req.query.hasOwnProperty(argName)) {
			args[argName] = req.query[argName];
		}
	}

	return args;
}

/**
* Get the files
*
* @param {Object} files The "files" property in the request.
* @param {Object} fieldList An object with the fields.
* @param {Array} fileList An array of file objects to be uploaded.
* @api private
*/
function _getFiles(files, fieldList, fileList) {
	// Prepare on object only containing the actual arguments
	let propName = '',
		file,
		filename = '';

	for (propName in files) {
		/* istanbul ignore else */
		if (files.hasOwnProperty(propName)) {
			file = files[propName];
			/* istanbul ignore else */
			if (file.originalFilename && file.originalFilename.length > 0) {
				// Create a new proper filename for Oracle
				filename = _getRandomizedFilename(file.originalFilename);

				// Add the field
				fieldList[propName] = filename;

				// Add the file to upload
				fileList.push({
					fieldValue: filename,
					filename: file.originalFilename,
					physicalFilename: path.normalize(path.resolve(file.path)),
					encoding: '',
					mimetype: file.type,
					size: file.size
				});
			}
		}
	}
}

/**
* Get a randomized filename
*
* @param {String} filename The original filename.
* @return {String} The randomized filename.
* @api private
*/
function _getRandomizedFilename(filename) {
	let randomString;

	++sequencialID;
	randomString = (Date.now() + sequencialID).toString();

	return 'F' + randomString + '/' + path.basename(filename);
}

/**
* Report an error when trying to invoke the Oracle procedure
*
* @param {String} err Error description
* @param {Object} req Request object
* @api private
*/

function _reportRequestError(err, req) {
	let request = {
		headers: req.headers,
		url: req.url,
		method: req.method,
		params: req.params,
		query: req.query,
		body: req.body,
		files: req.files,
		cookies: req.cookies,
		route: req.route
	};

	log.error('Error processing a request "' + req.url + '"', err, util.inspect(request, {showHidden: false, depth: null, colors: false}));
}

module.exports = {
	process: process
};
