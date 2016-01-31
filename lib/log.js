/**
* Module dependencies.
*/

'use strict';

var fs = require('fs');
var colors = require('colors');
var _ = require('underscore');
var util = require('util');

/**
* Module constants.
*/
var ERROR_FILE_NAME = 'error.log';
var TRACE_FILE_NAME = 'trace.log';

/**
* Module variables.
*/

function _line(size) {
	var len = size || 80,
		str = '';

	while (len--) {
		str += '-';
	}

	return str;
}

/**
* Is logging enabled
*
* @param {date} [date] Optional date.
* @return {string} Timestamp string.
* @api public
*/
function getTimestamp(date) {
	var d = date || new Date();

	function lz(value, length) {
		var s = value.toString(),
			l = length || 2;

		while (s.length < l) {
			s = '0' + s;
		}

		return s;
	}

	return	d.getFullYear().toString() + '.' + lz(d.getMonth() + 1) + '.' + lz(d.getDate()) + ' ' + lz(d.getHours() + 1) + ':' + lz(d.getMinutes()) + ':' + lz(d.getSeconds()) + '.' + lz(d.getMilliseconds(), 3);
}

/**
* Is logging enabled
*
* @return {Boolean} Return true when logging is enabled or else false.
* @api public
*/
function enabled() {
	if (process.env.NOLOG) {
		return false;
	}

	return true;
}

/**
* Enable or disable logging
*
* @param {Boolean} flag True to enable or false to disable logging.
* @return {Boolean} Return the original logging status.
* @api public
*/
function enable(flag) {
	var original = enabled();

	if (typeof flag === 'undefined' || flag === true) {
		delete process.env.NOLOG;
	} else {
		process.env.NOLOG = '1';
	}

	return original;
}

/**
* Write an log message
*
* @param {...*} var_args
* @api public
*/
function log(/*var_args*/) {
	/* istanbul ignore if */
	if (process.env.NOLOG !== '1' && arguments.length > 0) {
		console.log.apply(null, arguments);
	}
}

/**
* Write an error message
*
* @param {string} text The error message
* @api public
*/
function error(text) {
	var txt = text || '',
		timestamp = getTimestamp(),
		stack = new Error().stack,
		fd,
		lines,
		i;

	// write to the error log
	fd = fs.openSync(ERROR_FILE_NAME, 'a');
	fs.writeSync(fd, '\n\n' + _line(80) + '\nERROR: ' + txt + '\nTIMESTAMP: ' + timestamp);
	lines = Array.prototype.slice.call(arguments).slice(1);
	if (lines.length > 0) {
		fs.writeSync(fd, '\n');
	}
	for (i = 0; i < lines.length; i++) {
		fs.writeSync(fd, '\n' + lines[i]);
	}
	fs.writeSync(fd, '\n' + stack + '\n');
	fs.closeSync(fd);

	// write to the console
	/* istanbul ignore if */
	if (process.env.NOLOG !== '1') {
		console.log(colors.red('ERROR: ' + timestamp + ' - ' + txt + ' (details in error.log)'));
	}
}

/**
* Write an trace message
*
* @param {...*} var_args
* @api public
*/
function trace(/*var_args*/) {
	var text,
		fd;

	if (process.env.NODE_PLSQL_TRACE !== '1') {
		return;
	}

	// create a text to be logged
	text = _line() + '\nTIMESTAMP: ' + getTimestamp() + '\n\n';
	_.each(arguments, function (argument, index) {
		if (index > 0) {
			text += '\n';
		}

		if (typeof argument === 'string') {
			text += argument;
		} else {
			text += util.inspect(argument, {showHidden: false, depth: null, colors: false});
		}
	});
	text += '\n\n';

	// write to the trace file
	fd = fs.openSync(TRACE_FILE_NAME, 'a');
	fs.writeSync(fd, text);
	fs.closeSync(fd);
}

module.exports = {
	getTimestamp: getTimestamp,
	enabled: enabled,
	enable: enable,
	log: log,
	error: error,
	trace: trace
};
