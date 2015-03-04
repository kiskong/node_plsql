/**
 * @fileoverview Test for the module "header.js"
 * @author doberkofler
 */


/* global describe: false, it:false */


// ------------------------------------------------------------------------------
// Requirements
// ------------------------------------------------------------------------------

var assert = require('chai').assert;
var tough = require('tough-cookie');
var header = require('../lib/header');


// ------------------------------------------------------------------------------
// Tests
// ------------------------------------------------------------------------------
describe('header', function () {
	'use strict';

	describe('when calling tough-cookie.parse()', function () {
		it('the cookie string should return an object', function () {
			var cookie = {};

			assert.isUndefined(tough.Cookie.parse('', true));
			assert.isUndefined(tough.Cookie.parse('a', true));
			assert.isUndefined(tough.Cookie.parse('=a', true));

			cookie = tough.Cookie.parse('c1=v1', true);
			assert.strictEqual(cookie.key, 'c1');
			assert.strictEqual(cookie.value, 'v1');

			cookie = tough.Cookie.parse('c1="this is the value"', true);
			assert.isUndefined(cookie);

			cookie = tough.Cookie.parse('c1=this is the value', false);
			assert.strictEqual(cookie.key, 'c1');
			assert.strictEqual(cookie.value, 'this is the value');

			cookie = tough.Cookie.parse('c1="this is the value"', false);
			assert.strictEqual(cookie.key, 'c1');
			assert.strictEqual(cookie.value, 'this is the value');
		});
	});

	describe('when calling containsHttpHeader()', function () {
		it('should find text containing with one header line', function () {
			var code = header.containsHttpHeader;
			assert.equal(code('Content-type: '), true);
			assert.equal(code('CONTENT-TYPE: '), true);
			assert.equal(code('content-type: '), true);
			assert.equal(code('Location: '), true);
			assert.equal(code('Status: '), true);
			assert.equal(code('X-DB-Content-length: '), true);
			assert.equal(code('WWW-Authenticate: '), true);
			assert.equal(code(' Location: '), true);
			assert.equal(code(' Location:  '), true);
			assert.equal(code('\nLocation: \n'), true);
		});
		it('should find text containing with multiple header lines', function () {
			var code = header.containsHttpHeader;
			assert.equal(code('Location: Status: '), true);
			assert.equal(code('Location: \nStatus: '), true);
			assert.equal(code('\nContent-type: \nLocation: \nStatus: \n'), true);
		});
		it('should not find text containing without any header lines', function () {
			var code = header.containsHttpHeader;
			assert.equal(code(''), false);
			assert.equal(code(null), false);
			assert.equal(code(), false);
			assert.equal(code('Content-type:'), false);
			assert.equal(code('Content type: '), false);
			assert.equal(code('Location:Status: '), true);
		});
	});

	describe('when calling getHeaderAndBody()', function () {
		it('the header and the body should be split', function () {
			var testData = [{
				text: 'Content-type: text/html\n\n<html>',
				header: 'Content-type: text/html\n\n',
				body: '<html>'
			},
			{
				text: 'Content-type: text/html',
				header: 'Content-type: text/html',
				body: ''
			},
			{
				text: '<html>',
				header: '',
				body: '<html>'
			},
			{
				text: '',
				header: '',
				body: ''
			}],
			i;

			for (i = 0; i < testData.length; i++) {
				var result = header.getHeaderAndBody(testData[i].text);
				assert.equal(result.header, testData[i].header);
				assert.equal(result.body, testData[i].body);
			}
		});
	});

	describe('when calling parseHeader() with an invalid cookie header', function () {
		it('should return the following object', function () {
			var testHeaders = [{
				text: '',
				header: {},
				other: {},
				cookie: []
			},
			{
				text: '\n\n',
				header: {},
				other: {},
				cookie: []
			},
			{
				text: 'this is not a header line because the is no colon',
				header: {},
				other: {},
				cookie: []
			},
			{
				text: 'Location: index.html\nContent-type: text/html',
				header: {
					'contentType': 'text/html',
					'redirectLocation': 'index.html'
				},
				other: {},
				cookie: []
			},
			{
				text: '\nContent-type: text/html\n',
				header: {
					'contentType': 'text/html'
				},
				other: {},
				cookie: []
			},
			{
				text: 'Status: 400 error status\nContent-type: text/html\nX-DB-Content-length: 4711',
				header: {
					'statusCode': 400,
					'statusDescription': 'error status',
					'contentType': 'text/html',
					'contentLength': 4711
				},
				other: {},
				cookie: []
			},
			{
				text: 'Set-Cookie: c1=v1\nSet-Cookie: c2=another value',
				header: {},
				other: {},
				cookie: [{
					'key': 'c1',
					'value': 'v1'
				},
				{
					'key': 'c2',
					'value': 'another value'
				}]
			},
			{
				text: 'Set-Cookie: =v1\nSet-Cookie: c2:another value',
				header: {},
				other: {},
				cookie: []
			},
			{
				text: 'Status: 400 error status\nContent-type: text/html\nX-DB-Content-length: 4711\nSet-Cookie: c1=v1\nSet-Cookie: c2=another value\nsome attribute: some value',
				header: {
					'statusCode': 400,
					'statusDescription': 'error status',
					'contentType': 'text/html',
					'contentLength': 4711
				},
				other: {
					'some attribute': 'some value'
				},
				cookie: [{
					'key': 'c1',
					'value': 'v1'
				},
				{
					'key': 'c2',
					'value': 'another value'
				}]
			},
			{
				text: '\nSet-Cookie: correctKey=correctValue\nSet-Cookie: illegalKey = illigalValue\nSet-Cookie: key=value\n',
				header: {},
				other: {},
				cookie: [{
					'key': 'correctKey',
					'value': 'correctValue'
				},
				{
					'key': 'key',
					'value': 'value'
				}]
			}];

			function findCookie(cookies, key) {
				var i;

				for (i = 0; i < cookies.length; i++) {
					if (cookies[i].key === key) {
						return i;
					}
				}

				return -1;
			}

			function testCookies(expectedCookies, resultCookies) {
				var i,
					f,
					k;

				for (i = 0; i < expectedCookies.length; i++) {
					for (k in expectedCookies[i]) {
						if (expectedCookies.hasOwnProperty(k)) {
							f = findCookie(resultCookies, k);
							assert.notEqual(f, -1);
							assert.property(resultCookies[f], k);
							assert.propertyVal(resultCookies[f], k, expectedCookies[k]);
						}
					}
				}
			}

			var headerMain = {},
				headerOther = {},
				headerCookies = [],
				l = 0;

			for (l = 0; l < testHeaders.length; l++) {
				headerMain = {};
				headerOther = {};
				headerCookies = [];
				header.parseHeader(testHeaders[l].text, headerMain, headerOther, headerCookies);
				assert.deepEqual(headerMain, testHeaders[l].header);
				assert.deepEqual(headerOther, testHeaders[l].other);
				testCookies(testHeaders[l].cookie, headerCookies);
			}
		});
	});

});
