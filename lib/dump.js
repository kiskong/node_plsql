/* jshint node: true */

/**
* Module dependencies.
*/

//var debug = require('debug')('listener');

/**
* Module variables.
*/

var DEFAULT_LENGTH = 60;
var DEFAULT_STRING = '-';

/**
* Return a divider string
* @param {Number} n - The times to repeat the string.
* @param {String} s - A string to repeat.
* @api public
*/
var divider = function (n, s) {
	'use strict';

	n = n || DEFAULT_LENGTH;
	s = s || DEFAULT_STRING;

	return new Array(n + 1).join(s);
};

/**
* Return a block starting and ending with a devider in the form of
*	"-<title> begin---------" CRLF <block> CRLF "-<title> end-----------"
* @param {String} title - The title of the comment.
* @param {String} block - The content of the comment.
* @api public
*/
var block = function (title, block) {
	'use strict';

	var begin = '',
		end = '';

	begin = DEFAULT_STRING + title + ' begin';
	begin += divider(DEFAULT_LENGTH - begin.length, DEFAULT_STRING);

	end = DEFAULT_STRING + title + ' end';
	end += divider(DEFAULT_LENGTH - end.length, DEFAULT_STRING);

	return begin + '\n' + block + '\n' + end;
};

module.exports = {
	divider: divider,
	block: block
};
