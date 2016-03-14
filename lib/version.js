'use strict';

/**
* Module dependencies.
*/

const path = require('path');
const fs = require('fs');
const log = require('./log');

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
	const PACKAGE_FILENAME = path.join(__dirname, '../package.json');
	let content;
	let version = '';

	try {
		content = fs.readFileSync(PACKAGE_FILENAME);
	} catch (e) {
		/* istanbul ignore next */
		log.exit(new TypeError('Could not read file: ' + PACKAGE_FILENAME + ' Error: ' + e.message));
	}

	try {
		version = JSON.parse(content).version;
	} catch (e) {
		/* istanbul ignore next */
		log.exit(new TypeError('Could not parse file: ' + PACKAGE_FILENAME + ' Error: ' + e.message));
	}

	return version;
}

module.exports = {
	get: get
};
