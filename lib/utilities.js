'use strict';

/**
* Module dependencies.
*/

const fs = require('fs');
const path = require('path');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* File copy
*
* @param {String} from Source file.
* @param {String} to Destination file.
* @api private
*/
function fileCopy(from, to) {
	let content = fs.readFileSync(from);

	fs.writeFileSync(to, content);
}

/**
* File delete
*
* @param {String} filename Filename.
* @api private
*/
function fileDelete(filename) {
	if (fs.existsSync(filename)) {
		fs.unlinkSync(filename);
	}
}

/**
* Copy the sample configuration file
*
* @param {String} filename Relative filename.
* @return {String} Absolute file path relative to the directory where this script resides.
* @api private
*/
function absoluteFilename(filename) {
	return path.resolve(path.join(__dirname, filename));
}

module.exports = {
	fileCopy: fileCopy,
	fileDelete: fileDelete,
	absoluteFilename: absoluteFilename
};
