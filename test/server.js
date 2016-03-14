'use strict';

/**
 * @fileoverview Test for the module "server.js"
 * @author doberkofler
 */

/* global describe: false, it:false, before: false, after: false */

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:server:test');
const assert = require('chai').assert;
const request = require('supertest');
const util = require('util');
const fs = require('fs');
const mkdirp = require('mkdirp');
const os = require('os');
const server = require('../lib/server');
const log = require('../lib/log');

/**
* Module constants.
*/

/**
* Module variables.
*/

/**
* Tests.
*/

describe('server.js', function () {

	describe('static resources', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('GET static resources (GET /test/server.js)', function () {
			it('should return the static file /test/server.js', function (done) {
				let test = request(application.expressApplication).get('/test/server.js');

				test.expect(200, new RegExp('.*should return the static file /test/server.js.*'), done);
			});
		});

		describe('static resources (GET /fileDoesNotExist)', function () {
			it('should report a 404 error', function (done) {
				let test = request(application.expressApplication).get('/fileDoesNotExist');

				test.expect(404, done);
			});
		});

	});

	describe('routes', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('default page (GET /sampleRoute)', function () {
			it('should return the default page', function (done) {
				let test = request(application.expressApplication).get('/sampleRoute');

				test.expect(302, 'Found. Redirecting to /sampleRoute/samplePage', done);
			});
		});

		describe('GET /sampleRoute/emptyPage', function () {
			it('GET /sampleRoute/emptyPage should return an empty page', function (done) {
				request(application.expressApplication).get('/sampleRoute/emptyPage')
					.expect(200, '', done);
			});
		});

		describe('GET /sampleRoute/samplePage', function () {
			it('GET /sampleRoute/samplePage should return the sample page', function (done) {
				request(application.expressApplication).get('/sampleRoute/samplePage')
					.expect(200, 'sample page', done);
			});
		});

		describe('GET /sampleRoute/completePage', function () {
			it('should return the complete page', function (done) {
				request(application.expressApplication).get('/sampleRoute/completePage?para=value')
					.expect(200, 'complete page', done);
			});
		});

		describe('GET /sampleRoute/arrayPage', function () {
			let args = {para: ['value1', 'value2']};

			it('should return the array page', function (done) {
				request(application.expressApplication).get('/sampleRoute/arrayPage?para=value1&para=value2')
					.expect(200, 'array page\n' + util.inspect(args), done);
			});
		});

		describe('GET /sampleRoute/redirect', function () {
			it('should redirect to another page', function (done) {
				request(application.expressApplication).get('/sampleRoute/redirect')
					.expect(302, done);
			});
		});

		describe('GET /sampleRoute/json', function () {
			it('should parse JSON', function (done) {
				request(application.expressApplication).get('/sampleRoute/json')
					.expect(200, '{"name":"johndoe"}', done);
			});
		});

		describe('POST /sampleRoute/form_urlencoded', function () {
			it('should return a form with fields', function (done) {
				let test = request(application.expressApplication).post('/sampleRoute/form_urlencoded');

				test.set('Content-Type', 'application/x-www-form-urlencoded');
				test.send('name=johndoe');

				test.expect(200, '{"name":"johndoe"}', done);
			});
		});

		describe('POST /sampleRoute/multipart_form_data', function () {
			it('should return a multipart form with files', function (done) {
				let test = request(application.expressApplication).post('/sampleRoute/multipart_form_data');

				test.set('Content-Type', 'multipart/form-data; boundary=foo');
				test.write('--foo\r\n');
				test.write('Content-Disposition: form-data; name="user_name"\r\n');
				test.write('\r\n');
				test.write('Tobi');
				test.write('\r\n--foo\r\n');
				test.write('Content-Disposition: form-data; name="text"; filename="test/server.js"\r\n');
				test.write('\r\n');
				test.write('some text here');
				test.write('\r\n--foo--');

				test.expect(200, 'server.js', done);
			});
		});

		describe('GET /sampleRoute/cgi', function () {
			it('GET /sampleRoute/cgi should validate the cgi', function (done) {
				request(application.expressApplication).get('/sampleRoute/cgi')
					.expect(200, 'cgi', done);
			});
		});

	});

	describe('file upload', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('Upload files (POST /sampleRoute/fileUpload)', function () {
			it('should upload files', function (done) {
				const FILENAME = 'temp/index.html';
				const CONTENT = 'content of index.html';
				let test;

				// create a static file
				mkdirp.sync('temp');
				fs.writeFileSync(FILENAME, CONTENT);

				// test the upload
				test = request(application.expressApplication).post('/sampleRoute/fileUpload');
				test.attach('file', FILENAME);
				test.expect(200, done);
			});
		});

	});

	describe('status page', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('GET /status', function () {
			it('should show the status page', function (done) {
				request(application.expressApplication)
					.get('/status')
					.expect(200)
					.end(function (err) {
						if (err) {
							return done(err);
						}
						return done();
					});
			});
		});

	});

	describe('errors', function () {
		let application;

		before('Start the server', function (done) {
			_startServer().then(function (app) {
				application = app;
				done();
			});
		});

		after('Stop the server', function (done) {
			server.stop(application, function () {
				application = null;
				done();
			});
		});

		describe('GET /invalidRoute', function () {
			it('should respond with 404', function (done) {
				let test = request(application.expressApplication).get('/invalidRoute');

				test.expect(404, new RegExp('.*404 Not Found.*'), done);
			});
		});

		describe('GET /sampleRoute/errorInPLSQL', function () {
			it('should respond with 404', function (done) {
				let test = request(application.expressApplication).get('/sampleRoute/errorInPLSQL');

				test.expect(404, new RegExp('.*Failed to parse target procedure.*'), done);
			});
		});

		describe('GET /sampleRoute/internalError', function () {
			it('should respond with 500', function (done) {
				let test = request(application.expressApplication).get('/sampleRoute/internalError');

				test.expect(500, done);
			});
		});

	});

	describe('start server with no routes', function () {
		let application;

		it('does start', function (done) {
			let OPTIONS = {
				server: {
					port: 8999,
					suppressOutput: true,
					requestLogging: true
				}
			};

			server.start(OPTIONS).then(function (app) {
				application = app;
				assert.ok(true);
				done();
			});
		});

		it('does stop', function (done) {
			server.stop(application, function () {
				application = null;
				assert.ok(true);
				done();
			});
		});
	});

	describe('start server with invalid options', function () {
		it('does not start', function (done) {
			server.start().then(function () {
			}, function (err) {
				assert.strictEqual(err, 'Configuration object must be an object');
				done();
			});
		});
	});

});

/*
 * Start server
 */
function _startServer() {
	const OPTIONS = {
		server: {
			port: 8999,
			static: [
				{
					mountPath: '/',
					physicalDirectory: './'
				},
				{
					mountPath: '/temp/',
					physicalDirectory: './temp'
				}
			],
			suppressOutput: true,
			requestLogging: true,
			requestTracing: true
		},
		services: [
			{
				route: 'sampleRoute',
				defaultPage: 'samplePage',
				databaseUsername: 'sampleUsername',
				databasePassword: 'samplePassword',
				databaseConnectString: 'sampleConnectString',
				documentTableName: 'sampleDoctable',
				invokeCallback: _invokeCallback
			},
			{
				route: 'secondRoute',
				databaseUsername: 'secondUsername',
				databasePassword: 'secondPassword',
				databaseConnectString: 'sampleConnectString',
				documentTableName: 'secondDoctable',
				invokeCallback: _invokeCallback
			}
		]
	};

	log.enable(false);

	// Start server
	return server.start(OPTIONS);
}

/*
 * Database callback when invoking a page
 */
function _invokeCallback(database, procedure, args, cgi, files, doctablename, callback) {
	debug('_invokeCallback: START\n' + util.inspect(arguments, {showHidden: false, depth: null, colors: true}) + '\"');

	switch (procedure.toLowerCase()) {
		case 'emptypage':
			callback(null, _getPage(''));
			break;
		case 'samplepage':
			callback(null, _getPage('sample page'));
			break;
		case 'completepage':
			callback(null, _getPage('complete page', {'Content-Type': 'text/html', 'Set-Cookie': 'C1=V1'}));
			break;
		case 'arraypage':
			callback(null, _getPage('array page\n' + util.inspect(args), {'Content-Type': 'text/html'}));
			break;
		case 'redirect':
			callback(null, _getPage('', {'Location': 'www.google.com'}));
			break;
		case 'json':
			callback(null, _getPage('{"name":"johndoe"}', {'Content-Type': 'application/json'}));
			break;
		case 'form_urlencoded':
			callback(null, _getPage('{"name":"johndoe"}', {'Content-Type': 'text/html'}));
			break;
		case 'multipart_form_data':
			callback(null, _getPage('server.js', {'Content-Type': 'text/html'}));
			break;
		case 'cgi':
			_validateCGI(cgi);
			callback(null, _getPage('cgi'), {'Content-Type': 'text/html'});
			break;
		case 'fileupload':
			callback(null, _getPage('File "server.js" has been uploaded', {'Content-Type': 'text/html'}));
			break;
		case 'errorinplsql':
			callback(new Error('procedure not found'));
			break;
		case 'internalerror':
			throw new Error('internal error');
		default:
			console.log('==========> FATAL ERROR IN server.js: _invokeCallback received an invalid procedure=' + procedure);
			break;
	}

	debug('_invokeCallback: END');
}

/*
 * Get database page
 */
function _getPage(body, header) {
	let text = '',
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

/*
 * Validate the CGI
 */
function _validateCGI(cgi) {
	const SERVER_PORT = '8999',
		ROUTE = 'sampleRoute';

	assert.strictEqual(cgi.PLSQL_GATEWAY, 'WebDb');
	assert.strictEqual(cgi.GATEWAY_IVERSION, '2');
	assert.strictEqual(cgi.SERVER_SOFTWARE, 'NodeJS-PL/SQL-Gateway');
	assert.strictEqual(cgi.GATEWAY_INTERFACE, 'CGI/1.1');
	assert.strictEqual(cgi.SERVER_PORT, SERVER_PORT);
	assert.strictEqual(cgi.SERVER_NAME, os.hostname());
	assert.strictEqual(cgi.REQUEST_METHOD, 'GET');
	assert.strictEqual(cgi.PATH_INFO, 'cgi');
	assert.strictEqual(cgi.SCRIPT_NAME, ROUTE);
//	assert.strictEqual(cgi.REMOTE_ADDR, REMOTE_ADDRESS);
	assert.strictEqual(cgi.SERVER_PROTOCOL, 'HTTP/1.1');
	assert.strictEqual(cgi.REQUEST_PROTOCOL, 'HTTP');
//	assert.strictEqual(cgi.REMOTE_USER, '');
//	assert.strictEqual(cgi.HTTP_USER_AGENT, 'USER-AGENT');
//	assert.strictEqual(cgi.HTTP_HOST, 'HOST');
//	assert.strictEqual(cgi.HTTP_ACCEPT, 'ACCEPT');
//	assert.strictEqual(cgi.HTTP_ACCEPT_ENCODING, 'ACCEPT-ENCODING');
//	assert.strictEqual(cgi.HTTP_ACCEPT_LANGUAGE, 'ACCEPT-LANGUAGE');
//	assert.strictEqual(cgi.HTTP_REFERER, '');
	assert.strictEqual(cgi.WEB_AUTHENT_PREFIX, '');
	assert.strictEqual(cgi.DAD_NAME, ROUTE);
	assert.strictEqual(cgi.DOC_ACCESS_PATH, 'doc');
	assert.strictEqual(cgi.DOCUMENT_TABLE, 'sampleDoctable');
	assert.strictEqual(cgi.PATH_ALIAS, '');
	assert.strictEqual(cgi.REQUEST_CHARSET, 'UTF8');
	assert.strictEqual(cgi.REQUEST_IANA_CHARSET, 'UTF-8');
	assert.strictEqual(cgi.SCRIPT_PREFIX, '/');
	assert.isUndefined(cgi.HTTP_COOKIE);
}
