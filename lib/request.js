/**
* Module dependencies.
*/

var debug = require('debug')('node_plsql:request');
var path = require('path');
var crypto = require('crypto');
var util = require('util');
var underscore = require('underscore');

var log = require('./log');
var createCGI = require('./cgi').createCGI;
var header = require('./header');
var response = require('./response');
var database = require('./database').database;
var statistics = require('./statistics');
var debugBlock = require('./dump').debug;

/**
* Module variables.
*/

/**
* Normalize the body by making sure that only "simple" parameters and no nested objects are submitted
*
* @param {Object} body The body of the request with all the parameters.
* @return {Object} Object with the parameters as properties.
* @api private
*/
function normalizeBody(body) {
	'use strict';

	// Prepare on object only containing the actual arguments
	var argName = '',
		type = '',
		args = {};

	// This more generic type check is taken from "http://javascriptweblog.wordpress.com/2011/08/08/fixing-the-javascript-typeof-operator"
	function toType(obj) {
		return ({}).toString.call(obj).match(/\s([a-zA-Z]+)/)[1].toLowerCase();
	}

	function isArrayOfString(obj) {
		var i;

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
function getParameter(req) {
	'use strict';

	// Prepare on object only containing the actual arguments
	var args = {};
	var argName = '';

	for (argName in req.query) {
		/* istanbul ignore else */
		if (req.query.hasOwnProperty(argName)) {
			args[argName] = req.query[argName];
		}
	}

	return args;
}

/**
* Get a randomized filename
*
* @param {String} filename The original filename.
* @return {String} The randomized filename.
* @api private
*/
function getRandomizedFilename(filename) {
	'use strict';

	// Create a random string
	var randomString = (Date.now() + Math.random()).toString();
	randomString = crypto.createHash('md5').update(randomString).digest('hex');

	return 'F' + randomString + '/' + path.basename(filename);
}

/**
* Get the files
*
* @param {Object} files The "files" property in the request.
* @param {Object} fieldList An object with the fields.
* @param {Array} fileList An array of file objects to be uploaded.
* @api private
*/
function getFiles(files, fieldList, fileList) {
	'use strict';

	// Prepare on object only containing the actual arguments
	var propName = '',
		file,
		filename = '';

	for (propName in files) {
		/* istanbul ignore else */
		if (files.hasOwnProperty(propName)) {
			file = files[propName];
			/* istanbul ignore else */
			if (file.originalFilename && file.originalFilename.length > 0) {
				// Create a new proper filename for Oracle
				filename = getRandomizedFilename(file.originalFilename);

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
* Process the successful page and send the response
*
* @param {Object} res Result
* @param {String} page Page content
* @param {String} err Page content
* @api private
*/
function pageProcessSuccess(res, page/*, err*/) {
	'use strict';

	/* istanbul ignore if */
	if (debug.enabled) {
		debug('-PAGE SUCCESS: BEGIN--------------------------------------------------');
		debugBlock('PAGE', page, debug);
	}

	// Parse the page content and return the headers and the body
	debug('Get header and body...');
	var headerAndBody = header.getHeaderAndBody(page);

	// Parse the header
	var headerMain = {},
		headerOther = {},
		headerCookies = [];

	debug('Parsing header...');
	header.parseHeader(headerAndBody.header, headerMain, headerOther, headerCookies);

	/* istanbul ignore if */
	if (debug.enabled) {
		debug('-SEND: BEGIN----------------------------------------------------------');
		debugBlock('Header main', JSON.stringify(headerMain, null, '\t'), debug);
		debugBlock('Header other', JSON.stringify(headerOther, null, '\t'), debug);
		debugBlock('Header cookies', JSON.stringify(headerCookies, null, '\t'), debug);
		debugBlock('Body', headerAndBody.body, debug);
	}

	// Send the response
	response.send(res, headerMain, headerOther, headerCookies, headerAndBody.body);
}

/**
* Process the error page and send the response
*
* @param {Object} res Result
* @param {String} err Error content
* @api private
*/
function pageProcessError(res, err) {
	'use strict';

	var lines,
		html,
		len,
		i;

	/* istanbul ignore if */
	if (debug.enabled) {
		debug('-PAGE ERROR: BEGIN--------------------------------------------------');
		debugBlock('PAGE', err, debug);
	}

	// Create html error message
	lines = err.split('\n');
	html = '<html><head><title>Failed to parse target procedure</title></head><body><h1>Failed to parse target procedure</h1>\n';
	html += '<p>\n';
	len = lines.length;
	for (i = 0; i < len; i++) {
		html += lines[i] + '<br/>\n';
	}
	html += '</p>\n';
	html += '</body></html>';

	// Send the response
	res.status(404).send(html);
}

/**
* Process the page and send the response
*
* @param {Object} res Result
* @param {String} page Page content
* @param {Object} err Error
* @api private
*/
function pageProcess(res, page, err) {
	'use strict';

	debug('pageProcess');

	if (err) {
		pageProcessError(res, err);
	} else {
		pageProcessSuccess(res, page);
	}
}

/**
* Process a request asynchroniously
*
* @param {Object} app Express application
* @param {Object} serverConfig Server configuration
* @param {Object} service Service configuration
* @param {Object} req Request object
* @param {Object} res Response object
* @api public
*/

function processRequest(app, serverConfig, service, req, res) {
	'use strict';

	debug('processRequest: BEGIN');

	var databaseHandle = app.databaseHandle,
		args = {},
		fields = {},
		files = [],
		requestStatus;

	// Does the request contain any files
	if (req.hasOwnProperty('files') && underscore.keys(req.files).length > 0) {
		// Get the files
		getFiles(req.files, fields, files);

		/* istanbul ignore if */
		if (debug.enabled) {
			debugBlock('req.files', util.inspect(req.files, {showHidden: false, depth: null, colors: true}), debug);
			debug(' Fields: ' + util.inspect(fields, {showHidden: false, depth: null, colors: true}));
			debug(' Files: ' + util.inspect(files, {showHidden: false, depth: null, colors: true}));
		}

		args = underscore.extend(args, fields);
	}

	// Does the request contain a body
	/* istanbul ignore else */
	if (req.hasOwnProperty('body')) {
		debug('Request has a body: ' + util.inspect(req.body, {showHidden: false, depth: null, colors: true}));

		fields = normalizeBody(req.body);
		debug(' Fields: ' + util.inspect(fields, {showHidden: false, depth: null, colors: true}));

		args = underscore.extend(args, fields);
	}

	// Prepare on object only containing the actual arguments
	args = underscore.extend(args, getParameter(req));

	// Get the CGI
	var cgi = createCGI(serverConfig, req, service);

	/* istanbul ignore if */
	if (debug.enabled) {
		debug('-INVOKE: BEGIN--------------------------------------------------------');
		debug('Procedure name: "' + req.params.name + '"');
		debug('Parameter: "' + JSON.stringify(args) + '"');
		debug('Files: "' + JSON.stringify(files) + '"');
		debug('CGI: "' + JSON.stringify(cgi) + '"');
	}

	// Mark a request as started in the statistics
	requestStatus = statistics.requestStarted(app);

	// Invoke the PL/SQL procedure
	database.invoke(databaseHandle, service.databaseUsername, service.databasePassword, req.params.name, args, cgi, files, service.documentTableName, function pageReceived(err, page) {
		// if we have an error object, we throw an exception as something most have gone wrong and we want to log it
		if (err) {
			log.error('Error when processing a request "' + req.url + '"', err, util.inspect(req, {showHidden: false, depth: 2, colors: false}));
		}

		// Process the page returned by Oracle and render the page
		pageProcess(res, page, err);

		// Mark a request as completed in the statistics
		statistics.requestCompleted(app, requestStatus);
	});

	debug('processRequest: END');
}

module.exports = {
	processRequest: processRequest
};
