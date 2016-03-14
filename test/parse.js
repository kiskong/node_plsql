'use strict';

/**
 * @fileoverview Test for the module "header.js"
 * @author doberkofler
 */

/*global describe: false, it:false */
/*eslint-disable no-undefined */

// ------------------------------------------------------------------------------
// Requirements
// ------------------------------------------------------------------------------

const assert = require('chai').assert;
const _ = require('underscore');
const parse = require('../lib/parse');

// ------------------------------------------------------------------------------
// Tests
// ------------------------------------------------------------------------------

describe('parse.js', function () {

	describe('when parsing the text returned from PL/SQL', function () {
		const NOW = new Date();
		const TEST_DATA = [
			{
				title: 'empty',
				raw: '',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'only 2 CRLF',
				raw: '\n\n',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: '\n\n'
			},
			{
				title: 'Only body',
				raw: 'Only body',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: 'Only body'
			},
			{
				title: 'Multiline body',
				raw: 'Multiline\nbody',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: 'Multiline\nbody'
			},
			{
				title: 'Body, content type and a cookie',
				raw: 'Content-type: text/html\nSet-Cookie: c=v\n\nBody, content type and a cookie',
				cookies: [
					{name: 'c', value: 'v'}
				],
				redirectLocation: null,
				contentType: 'text/html',
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: 'Body, content type and a cookie'
			},
			{
				title: 'Location and content type',
				raw: 'Location: index.html\nContent-type: text/html',
				cookies: [],
				redirectLocation: 'index.html',
				contentType: 'text/html',
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'Only a content type',
				raw: '\nContent-type: text/html\n',
				cookies: [],
				redirectLocation: null,
				contentType: 'text/html',
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'No body, an invalid and a valid cookie',
				raw: 'Set-Cookie: =v91\nSet-Cookie: c2=v v',
				cookies: [
					{name: 'c2', value: 'v v'}
				],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'No body and 2 invalid cookies',
				raw: 'Set-Cookie: =v91\nSet-Cookie: c2:v92',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'No body but several header lines',
				raw: 'Status: 400 error status\nContent-type: text/html\nX-DB-Content-length: 4711\nSet-Cookie: c1=v1\nSet-Cookie: c2=another value\nsome attribute: some value',
				cookies: [
					{name: 'c1', value: 'v1'},
					{name: 'c2', value: 'another value'}
				],
				redirectLocation: null,
				contentType: 'text/html',
				contentLength: 4711,
				statusCode: 400,
				statusDescription: 'error status',
				headers: {
					'some attribute': 'some value'
				},
				body: ''
			},
			{
				title: 'all supported cookie options',
				raw: 'Set-Cookie: c=v; domain=d; secure=s; httponly; path=/p; expires=' + NOW.toGMTString(),
				cookies: [
					{name: 'c', value: 'v', domain: 'd', secure: 's', httpOnly: true, path: '/p', expires: NOW}
				],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'cookie with a path',
				raw: '\nSet-Cookie: c1=v1\nSet-Cookie: c2 = v2\nSet-Cookie: c3=v3; path=/path; HttpOnly\n',
				cookies: [
					{name: 'c1', value: 'v1'},
					{name: 'c2', value: 'v2'},
					{name: 'c3', value: 'v3', path: '/path', httpOnly: true}
				],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'cookie with a path not starting with /',
				raw: 'Set-Cookie: ORA_WWV_USER_69401584990494=ORA_WWV-l/MUz/hRgyDJIKAExswTqhY2; path=apex; HttpOnly',
				cookies: [
					{name: 'ORA_WWV_USER_69401584990494', value: 'ORA_WWV-l/MUz/hRgyDJIKAExswTqhY2', path: 'apex', httpOnly: true}
				],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'cookie with an illegal attribute',
				raw: 'Set-Cookie: c=v; illegalAttribute=true; HttpOnly',
				cookies: [
					{name: 'c', value: 'v', httpOnly: true}
				],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			},
			{
				title: 'empty "Set-Cookie: "',
				raw: 'Set-Cookie: ',
				cookies: [],
				redirectLocation: null,
				contentType: null,
				contentLength: null,
				statusCode: null,
				statusDescription: null,
				headers: {},
				body: ''
			}
		];

		_.each(TEST_DATA, function (test) {
			it(test.title + ': _parseHeader should return the following objects', function () {
				let message = parse.parseContent(test.raw);

				// cookies
				assert.strictEqual(message.cookies.length, test.cookies.length, 'same number of cookies');
				_testCookies(message.cookies, test.cookies);

				// headers
				assert.strictEqual(message.redirectLocation, test.redirectLocation, 'redirectLocation');
				assert.strictEqual(message.contentType, test.contentType, 'contentType');
				assert.strictEqual(message.contentLength, test.contentLength, 'contentLength');
				assert.strictEqual(message.statusCode, test.statusCode, 'statusCode');
				assert.strictEqual(message.statusDescription, test.statusDescription, 'statusDescription');

				// other headers
				assert.deepEqual(message.headers, test.headers, 'compare headers');

				// body
				assert.strictEqual(message.body, test.body, 'compare body');
			});
		});

		it('throws an error', function () {
			assert.throws(function () {
				parse.parseContent();
			});
			assert.throws(function () {
				parse.parseContent(0);
			});
			assert.throws(function () {
				parse.parseContent('1', '2');
			});
		});
	});

});

function _testCookies(resultCookies, expectedCookies) {
	// Make surte that every result cookie at least has a name and a value property
	_.each(resultCookies, function (cookie) {
		assert.property(cookie, 'name', 'cookie must have a "name" property');
		assert.property(cookie, 'value', 'cookie must have a "value" property');
	});

	// process the list of excepted cookies
	_.each(expectedCookies, function (expectedCookie) {
		// find the expected cookie in the list of cookies
		let resultCookie = _.find(resultCookies, function (cookie) {
			return cookie.name === expectedCookie.name;
		});

		assert.isDefined(resultCookie, 'find the cookie with key "' + expectedCookie.key + '"');

		_.each(expectedCookie, function (value, key) {
			assert.property(resultCookie, key, 'find property "' + key + '"');
			if (key === 'expires') {
				assert.strictEqual(resultCookie[key].getFullYear(), value.getFullYear(), 'compare value of property "' + key + '"');
				assert.strictEqual(resultCookie[key].getMonth(), value.getMonth(), 'compare value of property "' + key + '"');
				assert.strictEqual(resultCookie[key].getDate(), value.getDate(), 'compare value of property "' + key + '"');
				assert.strictEqual(resultCookie[key].getHours(), value.getHours(), 'compare value of property "' + key + '"');
				assert.strictEqual(resultCookie[key].getMinutes(), value.getMinutes(), 'compare value of property "' + key + '"');
				assert.strictEqual(resultCookie[key].getSeconds(), value.getSeconds(), 'compare value of property "' + key + '"');
			} else {
				assert.strictEqual(resultCookie[key], value, 'compare value of property "' + key + '"');
			}
		});
	});
}
