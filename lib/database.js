/* jshint node: true */

/**
* Module dependencies.
*/

/**
* Module variables.
*/

/**
* Load a database driver and return it
*
* @param {Object} config Server configuration.
* @return {Object} Database connection handle.
* @api private
*/
function database(config)
{
	'use strict';

	if (process.env.NODE_ENV === 'test') {
		return require('./database_shim');
	} else {
		return require('./database_oracle');
	}
}

module.exports = {
	database: database()
};