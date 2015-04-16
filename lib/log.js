/**
* Module dependencies.
*/

var fs = require('fs');
var colors = require('colors');

/**
* Module constants.
*/
var ERROR_FILE_NAME = 'error.log';

/**
* Module variables.
*/

function _line(size) {
	'use strict';

	var len = size,
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
	'use strict';

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
	'use strict';

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
	'use strict';

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
	'use strict';

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
	'use strict';

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
		console.log(colors.red('ERROR:' + ' ' + timestamp + ' - ' + txt + ' (details in error.log)'));
	}
}

module.exports = {
	getTimestamp: getTimestamp,
	enabled: enabled,
	enable: enable,
	log: log,
	error: error
};
