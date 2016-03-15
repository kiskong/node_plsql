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
	/* istanbul ignore next */
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
		/* istanbul ignore next */
		debug('process: pageReceived -> ' + (err === null) ? 'success' : 'error');

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
	try {
		message = parse.parseContent(page);
	} catch (err) {
		let errorTitle = 'Unable to parse the page returned form server';
		let errorMessage = '';
		try {
			errorMessage = 'Exception: ' + err.message + '<br><br>Page:<br>' + _lineBreaksToHtml(page);
		} catch (e) {/**/}
		log.error(new Error(errorTitle + '\n' + errorMessage));
		_errorPage(res, errorTitle, errorMessage);
	}

	// Send the response
	try {
		_sendResponse(message, cgiObj, req, res);
	} catch (err) {
		const errorTitle = 'Unable to send response to the client';
		let errorMessage = '';
		try {
			errorMessage = 'Exception: ' + err.message + '<br><br>Response:<br><pre>' + util.inspect(message, {showHidden: false, depth: null, colors: false}) + '</pre>';
		} catch (e) {/**/}
		log.error(new Error(errorTitle + '\n' + errorMessage));
		_errorPage(res, errorTitle, errorMessage);
	}
}

/**
* Send a response
*
* @param {Object} message - A message object
* @param {Object} cgiObj - CGI object
* @param {Object} req - express request object
* @param {Object} res - express response object
* @api public
*/
function _sendResponse(message, cgiObj, req, res) {
	// debug
	debug('send: START message=' + util.inspect(message, {showHidden: false, depth: null, colors: true}));

	// validate
	/* istanbul ignore next */
	if (arguments.length !== 4 || !_.isObject(message) || !_.isObject(cgiObj) || !_.isObject(req) || !_.isObject(res)) {
		log.exit(new Error('invalid arguments'));
	}

	// add "Server" header
	message.headers.Server = cgiObj.SERVER_SOFTWARE;

	// trace
	log.trace('D) A RESPONSE WILL BE SEND (response.send)', message);

	// Iterate over the array of cookies
	_.each(message.cookies, function (cookie) {
		debug('send: set cookie: ' + util.inspect(cookie, {showHidden: false, depth: null, colors: true}));
		res.cookie(cookie.name, cookie.value, _.omit(cookie, ['name', 'value']));
	});

	// Is the a "redirectLocation" header
	if (typeof message.redirectLocation === 'string' && message.redirectLocation.length > 0) {
		/*
		if (message.redirectLocation.indexOf('/') !== 0 && message.redirectLocation.indexOf('http://') !== 0) {
			debug('Converting relative path "' + message.redirectLocation + '" to absolute path');
			message.redirectLocation = cgiObj.SCRIPT_NAME + '/' + message.redirectLocation;
		}
		*/

		debug('send: redirecting to "' + message.redirectLocation + '"');
		res.redirect(302, message.redirectLocation);
		return;
	}

	// Is the a "contentType" header
	if (typeof message.contentType === 'string' && message.contentType.length > 0) {
		debug('send: set "Content-Type" to "' + message.contentType + '"');
		res.set('Content-Type', message.contentType);
	}

	// Iterate over the headers object
	_.each(message.headers, function (value, key) {
		debug('send: set header "' + key + '" to "' + value + '"');
		res.set(key, value);
	});

	// Process the body
	/* istanbul ignore else */
	if (typeof message.body === 'string') {
		debug('send: body "' + message.body.substring(0, 30).replace('\n', '\\n') + '"');
		res.send(message.body);
	}

	debug('send: END');
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
	// debug
	debug('_pageProcessError');

	// validate
	/* istanbul ignore next */
	if (arguments.length !== 3) {
		log.exit(new Error('_pageProcessError: invalid argument number: ' + arguments.length));
	}

	// trace
	log.trace('C) A STORED PROCEDURE REPORTED AN ERROR (request._pageProcessError)');

	// Show the error page
	let message = '';
	/* istanbul ignore else */
	if (typeof err.message === 'string') {
		message = err.message;
	}

	_errorPage(res, 'Failed to parse target procedure', message);
}

/**
* Generate error page
*
* @param {Object} res - express response object
* @param {String} title - The error title
* @param {String} message - The error message
* @api private
*/
function _errorPage(res, title, message) {
	// debug
	debug('_errorPage', title, message);

	// validate
	/* istanbul ignore next */
	if (arguments.length !== 3 || !_.isObject(res) || typeof title !== 'string' || typeof message !== 'string') {
		log.exit(new Error('_errorPage: invalid argument number: ' + arguments.length));
	}

	const html = `<html><head><title>${title}</title></head><body><h1>${title}</h1><p>${message}</p></body></html>`;

	// Send the response
	res.status(404).send(html);
}

/**
* Return the given text with line breaks converted to <br> tags
*
* @param {String} text - The text to convert
* @return {String} The converted text
* @api private
*/
function _lineBreaksToHtml(text) {
	return text.replace(/(?:\r\n|\r|\n)/g, '<br />');
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
