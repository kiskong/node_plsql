/* jshint node: true */

/**
* Module dependencies.
*/

//var debug = require('debug')('listener');

/**
* Module variables.
*/

var DEFAULT_LENGTH = 70;
var DEFAULT_STRING = '-';

/**
* Return a divider string
* @param {Number} n - The times to repeat the string.
* @param {String} s - A string to repeat.
* @api public
*/
function divider(n, s)
{
	'use strict';

	n = n || DEFAULT_LENGTH;
	s = s || DEFAULT_STRING;

	return new Array(n + 1).join(s);
}

/**
* Return a string starting and ending with a devider in the form of
*	"-<title> begin---------" CRLF <text> CRLF "-<title> end-----------"
* @param {String} title - The title of the comment.
* @param {String} text - The content of the comment.
* @api public
*/
function block(title, text) {
	'use strict';

	var begin = '',
		end = '';

	begin = DEFAULT_STRING + title + ' begin';
	begin += divider(DEFAULT_LENGTH - begin.length, DEFAULT_STRING);

	end = DEFAULT_STRING + title + ' end';
	end += divider(DEFAULT_LENGTH - end.length, DEFAULT_STRING);

	return begin + '\n' + text + '\n' + end;
}

/**
* Return a string starting and ending with a devider in the form of
*	"-<title> begin---------" CRLF <text> CRLF "-<title> end-----------"
* @param {String} title The title of the comment.
* @param {String} text The text of the comment.
* @param {Object} dbg The debug object.
* @api public
*/
function debug(title, text, dbg) {
	'use strict';

	var lines,
		len,
		i;

	lines = block(title, text).split('\n');

	len = lines.length;

	for (i = 0; i < len; i++) {
		dbg(lines[i]);
	}
}

module.exports = {
	divider: divider,
	block: block,
	debug: debug
};
