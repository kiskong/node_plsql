/**
 * @fileoverview Test for the module "server.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


/**
* Module dependencies.
*/

var debug = require('debug')('test/server');
var assert = require('chai').assert;
var request = require('supertest');
var util = require('util');
var fs = require('fs');
var mkdirp = require('mkdirp');
var server = require('../lib/server');


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
* @api private
*/
function databaseConnect(/*config*/) {
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
function getPage(body, header) {
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
function databaseInvoke(databaseHandle, username, password, procedure, args, cgi, files, doctablename, callback) {
	'use strict';

	var proc = procedure.toLowerCase();

	debug('databaseInvoke: \n' + util.inspect(arguments, {showHidden: false, depth: null, colors: true}) + '\"');

	switch (proc) {
	case 'emptypage':
		callback(null, '', {});
		break;
	case 'samplepage':
		callback(null, getPage('sample page'), {'Content-Type': 'text/html'});
		break;
	case 'completepage':
		callback(null, getPage('complete page', {'Content-Type': 'text/html', 'Set-Cookie': 'C1=V1'}));
		break;
	case 'redirect':
		callback(null, getPage('', {'Location': 'www.google.com'}));
		break;
	case 'json':
		callback(null, getPage('{"name":"johndoe"}', {'Content-Type': 'application/json'}));
		break;
	case 'form_urlencoded':
		callback(null, getPage('{"name":"johndoe"}', {'Content-Type': 'text/html'}));
		break;
	case 'multipart_form_data':
		callback(null, getPage('foo.txt', {'Content-Type': 'text/html'}));
		break;
	case 'invalidpage':
		callback('procedure not found');
		break;
	case 'internalerror':
		throw new Error('internal error');
	default:
		break;
	}
}

/**
* Start server
*
* @return {Object} Express application.
* @api private
*/
function startServer() {
	'use strict';

	var config = {
		server: {
			port: 8999,
			static: [{
				mountPath: '/',
				physicalDirectory: './'
			},
			{
				mountPath: '/temp/',
				physicalDirectory: './temp'
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

	// create a static file
	mkdirp.sync('temp');
	fs.writeFileSync('temp/index.html', 'content of index.html');

	// Start server
	var app = server.start(config);

	return app;
}


/**
* Tests.
*/

describe('start server', function () {
	'use strict';

	describe('with an valid configuration', function () {
		var app;
		it('does start', function () {
			app = startServer();
		});
		it('does stop', function () {
			app.server.close();
		});
	});

	describe('with an invalid configuration', function () {
		it('throws an exception', function () {
			assert.throws(function () {
				server.start({});
			});
		});
	});
});

describe('GET static resources', function () {
	'use strict';

	var app = startServer();

	describe('GET /test/server.js', function () {
		it('should return the static file /test/server.js', function (done) {
			var test = request(app).get('/test/server.js');
			test.expect(200, done);
		});
	});

	describe('GET /temp/index.html', function () {
		it('should return the static file /temp/index.html', function (done) {
			var test = request(app).get('/temp/index.html');
			test.expect(200, 'content of index.html', done);
		});
	});

	// Stop the server
	app.server.close();
});

describe('GET default page', function () {
	'use strict';

	var app = startServer();

	describe('GET /sampleRoute', function () {
		it('should return the default page', function (done) {
			var test = request(app).get('/sampleRoute');
			test.expect(302, 'Moved Temporarily. Redirecting to /sampleRoute/samplePage', done);
		});
	});

	// Stop the server
	app.server.close();
});

describe('GET', function () {
	'use strict';

	var app = startServer();

	describe('GET /sampleRoute/emptyPage', function () {
		it('GET /sampleRoute/emptyPage should return an empty page', function (done) {
			var test = request(app).get('/sampleRoute/emptyPage');
			test.expect(200, '', done);
		});
	});

	describe('GET /sampleRoute/samplePage', function () {
		it('GET /sampleRoute/samplePage should return the sample page', function (done) {
			var test = request(app).get('/sampleRoute/samplePage');
			test.expect(200, 'sample page', done);
		});
	});

	describe('GET /sampleRoute/completePage', function () {
		it('should return the complete page', function (done) {
			var test = request(app).get('/sampleRoute/completePage?para=value');
			test.expect(200, 'complete page', done);
		});
	});

	describe('GET /sampleRoute/redirect', function () {
		it('should redirect to another page', function (done) {
			var test = request(app).get('/sampleRoute/redirect');
			test.expect(302, done);
		});
	});

	describe('GET /sampleRoute/json', function () {
		it('should parse JSON', function (done) {
			var test = request(app).get('/sampleRoute/json');
			test.expect(200, '{"name":"johndoe"}', done);
		});
	});

	// Stop the server
	app.server.close();
});

describe('GET status', function () {
	'use strict';

	var app = startServer();

	describe('GET /status', function () {
		it('should show the status page', function (done) {
			var test = request(app).get('/status');
			test.expect(200, done);
		});
	});

	// Stop the server
	app.server.close();
});

describe('GET errors', function () {
	'use strict';

	var app = startServer();

	describe('GET /invalidRoute', function () {
		it('should respond with 404', function (done) {
			var test = request(app).get('/invalidRoute');
			test.expect(404, '<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL /invalidRoute was not found.</p></body></html>', done);
		});
	});

	describe('GET /sampleRoute/invalidPage', function () {
		it('should respond with 404', function (done) {
			var test = request(app).get('/sampleRoute/invalidPage');
			test.expect(404, '<html><head><title>Failed to parse target procedure</title></head><body><h1>Failed to parse target procedure</h1>\n<p>\nprocedure not found<br/>\n</p>\n</body></html>', done);
		});
	});

	describe('GET /sampleRoute/internalError', function () {
		it('should respond with 500', function (done) {
			var test = request(app).get('/sampleRoute/internalError');
			test.expect(500, /Error: internal error.*/, done);
		});
	});

	// Stop the server
	app.server.close();
});

describe('POST', function () {
	'use strict';

	var app = startServer();

	describe('POST /sampleRoute/form_urlencoded', function () {
		it('should return a form with fields', function (done) {
			var test = request(app).post('/sampleRoute/form_urlencoded');
			test.set('Content-Type', 'application/x-www-form-urlencoded');
			test.send('name=johndoe');
			test.expect(200, '{"name":"johndoe"}', done);
		});
	});

	describe('POST /sampleRoute/multipart_form_data', function () {
		it('should return a multipart form with files', function (done) {
			var test = request(app).post('/sampleRoute/multipart_form_data');
			test.set('Content-Type', 'multipart/form-data; boundary=foo');
			test.write('--foo\r\n');
			test.write('Content-Disposition: form-data; name="user_name"\r\n');
			test.write('\r\n');
			test.write('Tobi');
			test.write('\r\n--foo\r\n');
			test.write('Content-Disposition: form-data; name="text"; filename="foo.txt"\r\n');
			test.write('\r\n');
			test.write('some text here');
			test.write('\r\n--foo--');
			test.expect(200, 'foo.txt', done);
		});
	});

	// Stop the server
	app.server.close();
});
