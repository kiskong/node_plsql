'use strict';

/**
* Module dependencies.
*/

const debug = require('debug')('node_plsql:log');
const fs = require('fs');
const util = require('util');
const colors = require('colors');
const _ = require('underscore');

/**
* Module constants.
*/

const ERROR_FILE_NAME = 'error.log';
const TRACE_FILE_NAME = 'trace.log';

/**
* Module variables.
*/

/**
* Is logging enabled
*
* @param {date} [date] - Optional date.
* @return {string} Timestamp string.
* @api public
*/
function getTimestamp(date) {
	let d = date || new Date();

	function lz(value, length) {
		let s = value.toString(),
			l = length || 2;

		/* istanbul ignore next */
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
	let original = enabled();

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
* Write an trace message
*
* @param {...*} var_args
* @api public
*/
function trace(/*var_args*/) {
	let text,
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

/**
* Enable tracing
*
* @param {Boolean} ok - True to enable or else disable
* @return {Boolean} Previos status
* @api public
*/
function tracing(ok) {
	let original = process.env.NODE_PLSQL_TRACE === '1';

	process.env.NODE_PLSQL_TRACE = (ok === true) ? '1' : '0';

	return original;
}

/**
* Write an error message
*
* @param {*} err - The error value
* @api public
*/
function error(err) {
	const timestamp = getTimestamp();
	let text,
		stack,
		fd,
		lines,
		i;

	// Depending on the type of error, we either just display a text or convert the object
	if (err instanceof Error) {
		text = err.toString();
		stack = err.stack;
	} else if (typeof err === 'string') {
		text = err;
		stack = new Error().stack;
	} else {
		text = util.inspect(arguments, {showHidden: false, depth: null, colors: false});
		stack = new Error().stack;
	}

	debug(colors.red('ERROR: ' + text + '\nSTACK: ' + stack));

	// write to the error log
	fd = fs.openSync(ERROR_FILE_NAME, 'a');
	fs.writeSync(fd, '\n\n' + _line() + '\nERROR:\n' + text + '\n\nTIMESTAMP:\n' + timestamp);
	lines = Array.prototype.slice.call(arguments).slice(1);
	if (lines.length > 0) {
		fs.writeSync(fd, '\n');
	}
	for (i = 0; i < lines.length; i++) {
		fs.writeSync(fd, '\n' + lines[i]);
	}
	fs.writeSync(fd, '\n\nSTACK:\n' + stack + '\n');
	fs.closeSync(fd);

	// write to the console
	/* istanbul ignore if */
	if (process.env.NOLOG !== '1' && process.env.NODE_ENV !== 'test') {
		console.log(colors.red('ERROR: ' + timestamp + ' - ' + text + ' (details in error.log)'));
	}
}

/**
* Write an error message and exit the node application
*
* @param {*} err - The error value
* @api public
*/
/* istanbul ignore next */
function exit(err) {
	error.apply(this, arguments);

	if (process.env.NODE_ENV === 'test'/* || typeof global.it === 'function'*/) {
		throw (err instanceof Error) ? err : new Error('Fatal error');
	} else {
		process.exit(9);
	}
}

/*
* Return a line in the given length
*/
function _line(size) {
	let len = size || 80,
		str = '';

	while (len--) {
		str += '-';
	}

	return str;
}

module.exports = {
	getTimestamp: getTimestamp,
	enabled: enabled,
	enable: enable,
	log: log,
	trace: trace,
	tracing: tracing,
	error: error,
	exit: exit
};
