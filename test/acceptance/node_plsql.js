/**
 * @fileoverview Test for the module "node_plsql.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


/**
* Module dependencies.
*/

var debug = require('debug')('test/node_plsql');
var request = require('supertest');
var util = require('util');
var node_plsql = require('../../lib/node_plsql');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Database callback when connecting to the database
*
* @param {Object} config Server configuration.
* @return {Object} Database handle.
* @api private
*/
function databaseConnect(config)
{
	'use strict';

	debug('databaseConnect: \n' + util.inspect(arguments, {showHidden: false, depth: null, colors: true}) + '\"');
}

/**
* Get database page
*
* @param {String} body Page body.
* @param {Object} header Header object where the properties are the header names.
* @return {String} Text returned by database engine.
* @api private
*/
function getPage(body, header)
{
	'use strict';

	var text = '',
		name;

	if (header) {
		for (name in header) {
			if (header.hasOwnProperty(name)) {
				text += name + ': ' + header[name] + '\n';
			}
		}
	}

	text += 'Content-type: text/html; charset=UTF-8\nX-DB-Content-length: ' + body.length + '\n\n' + body;

	return text;
}

/**
* Database callback when invoking a page
*
* @param {Object} databaseHandle Database handle object.
* @param {String} username Oracle username.
* @param {String} password Oracle password.
* @param {String} procedure PL/SQL procedure to execute.
* @param {Object} args Object with the arguments for the PL/SQL procedure as properties.
* @param {Array} cgi Array of cgi variables to send for the PL/SQL code.
* @param {Array} files Array of files to upload.
* @param {String} doctablename Document table name.
* @param {Function} callback Callback function (function cb(err, page)) to invoke when done.
* @api private
*/
function databaseInvoke(databaseHandle, username, password, procedure, args, cgi, files, doctablename, callback)
{
	'use strict';

	var proc = procedure.toLowerCase();

	debug('databaseInvoke: \n' + util.inspect(arguments, {showHidden: false, depth: null, colors: true}) + '\"');

	if (proc === 'samplepage') {
		callback(null, getPage('sample page'));
	} else if (proc === 'completepage') {
		callback(null, getPage('complete page', {'Set-Cookie': 'C1=V1'}));
	} else if (proc === 'invalidpage') {
		callback('procedure not found');
	} else if (proc === 'internalerror') {
		throw new Error('internal error');
	}
}

/**
* Start server
*
* @return {Object} Express application.
* @api private
*/
function startServer()
{
	'use strict';

	var config = {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: __dirname + '/static'
			}],
			suppressOutput: true,
			requestLogging: true,
			oracleConnectionPool: true,
			oracleDebug: false
		},
		services: [{
			route: 'sampleRoute',
			defaultPage: 'samplePage',
			databaseUsername: 'sampleUsername',
			databasePassword: 'samplePassword',
			databaseConnectString: 'sampleConnectString',
			documentTableName: 'sampleDoctable'
		},
		{
			route: 'secondRoute',
			defaultPage: 'secondPage',
			databaseUsername: 'secondUsername',
			databasePassword: 'secondPassword',
			databaseConnectString: 'sampleConnectString',
			documentTableName: 'secondDoctable'
		}],
		callbacks: {
			databaseConnect: databaseConnect,
			databaseInvoke: databaseInvoke
		}
	};

	var app = node_plsql.start(config);

	return app;
}


/**
* Tests.
*/
describe('route-map', function () {
	'use strict';

	var app = startServer();

	describe('GET /sampleRoute/samplePage', function () {
		it('should return the sample page', function (done) {
			request(app)
			.get('/sampleRoute/samplePage')
			.expect('sample page', done);
		});
	});

	describe('GET /sampleRoute/completePage', function () {
		it('should return the complete page', function (done) {
			request(app)
			.get('/sampleRoute/completePage')
			.expect('complete page', done);
		});
	});

	describe('GET /sampleRoute/samplePage?p1=v1', function () {
		it('should return the sample page with parameters', function (done) {
			request(app)
			.get('/sampleRoute/samplePage?p1=v1')
			.expect('sample page', done);
		});
	});

	describe('GET /sampleRoute', function () {
		it('should return the default page', function (done) {
			request(app)
			.get('/sampleRoute')
			.expect('Moved Temporarily. Redirecting to /sampleRoute/samplePage', done);
		});
	});

	describe('GET /invalidRoute', function () {
		it('should respond with 404', function (done) {
			request(app)
			.get('/invalidRoute')
			.expect('<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL /invalidRoute was not found.</p></body></html>', done);
		});
	});

	describe('GET /sampleRoute/invalidPage', function () {
		it('should respond with 404', function (done) {
			request(app)
			.get('/sampleRoute/invalidPage')
			.expect('<html><head><title>Failed to parse target procedure</title></head><body><h1>Failed to parse target procedure</h1>\n<p>\nprocedure not found<br/>\n</p>\n</body></html>', done);
		});
	});

	describe('GET /sampleRoute/internalError', function () {
		it('should respond with 500', function (done) {
			request(app)
			.get('/sampleRoute/internalError')
			.expect('{}', done);
		});
	});

});
