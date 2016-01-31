'use strict';

/**
* Module dependencies.
*/

var path = require('path');
var fs = require('fs');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Get the version
*
* @return {String} Version
* @api public
*/
function get() {
	var filePath = path.join(__dirname, '../package.json');
	var version = '';

	try {
		version = JSON.parse(fs.readFileSync(filePath, 'utf8')).version;
	} catch (e) {
		throw new TypeError('Could not parse file: ' + filePath + ' Error: ' + e.message);
	}

	return version;
}

module.exports = {
	get: get
};
