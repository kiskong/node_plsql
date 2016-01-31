'use strict';

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
function database(/*config*/) {
	/*eslint-disable global-require */
	/* istanbul ignore next */
	if (process.env.NODE_ENV === 'test') {
		return require('./database_shim');
	}

	/* istanbul ignore next */
	return require('./database_oracle');
	/*eslint-enable global-require */
}

module.exports = {
	database: database()
};
