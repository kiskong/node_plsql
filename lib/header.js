/**
* Module dependencies.
*/

var tough = require('tough-cookie');
// var debug = require('debug')('node_plsql:header');

/**
* Does the body contain a HTTP header
* @param {String} text - A body
* @return {Boolean} true if the body constains an HTTP header
* @api public
*/
var containsHttpHeader = function (text) {
	'use strict';

	if (!text) {
		return false;
	}
	var t = text.toUpperCase();
	return (t.indexOf('CONTENT-TYPE: ') !== -1 || t.indexOf('LOCATION: ') !== -1 || t.indexOf('STATUS: ') !== -1 || t.indexOf('X-DB-CONTENT-LENGTH: ') !== -1 || t.indexOf('WWW-AUTHENTICATE: ') !== -1);
};

/**
* Parse the text returned by the procedure and separate header and body
* @param {String} content - An http page
* @return {Object} An object with a header and body property
* @api public
*/
var getHeaderAndBody = function (content) {
	'use strict';

	var obj = {
		header: '',
		body: ''
	};

	// Split up the content in header and body
	if (containsHttpHeader(content)) {
		// Find the end of the header identified by \n\n
		var headerEndPosition = content.indexOf('\n\n');

		// If we find no end of header marker, we only received a header without actual body
		if (headerEndPosition === -1) {
			obj.header = content;
		} else {
			headerEndPosition += 2;
			obj.header = content.substring(0, headerEndPosition);
			obj.body = content.substring(headerEndPosition);
		}
	} else {
		obj.body = content;
	}

	return obj;
};

/**
* Parse the header and split it up into the individual components
*
* @param {String} headerText HTTP header
* @param {String} headerMain Main header
* @param {String} headerOther Other header
* @param {Array} headerCookies Array of cokies
* @api public
*/
var parseHeader = function (headerText, headerMain, headerOther, headerCookies) {
	'use strict';

	var headerLines = [],
		headerLine = '',
		s = '',
		t = '',
		i = 0;

	if (!headerText) {
		return;
	}

	headerLines = headerText.split('\n');

	for (i = 0; i < headerLines.length; i++) {
		headerLine = headerLines[i];
		if (headerLine.length > 0) {
			if (headerLine.indexOf('Location: ') === 0) {
				headerMain.redirectLocation = headerLine.substr(10);
			} else if (headerLine.indexOf('Status: ') === 0) {
				s = headerLine.substr(8);
				t = s.split(' ')[0];
				headerMain.statusCode = parseInt(t, 10);
				headerMain.statusDescription = s.substr(t.length + 1);
			/*
			} else if (headerLine.indexOf('WWW-Authenticate: ') === 0) {
				headerMain.statusCode = 401;
				headerMain.statusDescription = '';*/
			} else if (headerLine.indexOf('Content-type: ') === 0) {
				headerMain.contentType = headerLine.substr(14);
			} else if (headerLine.indexOf('X-DB-Content-length: ') === 0) {
				headerMain.contentLength = parseInt(headerLine.substr(21), 10);
			} else if (headerLine.indexOf('Set-Cookie: ') === 0) {
				var cookie = tough.parse(headerLine.substr(12), false);
				if (cookie) {
					headerCookies.push(cookie);
				}
			} else if (headerLine.indexOf(':') !== -1) {
				headerOther[headerLine.substr(0, headerLine.indexOf(':'))] = headerLine.substr(headerLine.indexOf(':') + 2);
			}
		}
	}
};

module.exports = {
	containsHttpHeader: containsHttpHeader,
	getHeaderAndBody: getHeaderAndBody,
	parseHeader: parseHeader
};
