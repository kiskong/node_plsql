/* jshint node: true */

/**
* Module dependencies.
*/


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Is logging enabled
*
* @return {Boolean} Return true when logging is enabled or else false.
* @api public
*/
function enabled()
{
	'use strict';

	if (process.env.NOLOG) {
		return false;
	} else {
		return true;
	}
}

/**
* Enable or disable logging
*
* @param {Boolean} flag True to enable or false to disable logging.
* @return {Boolean} Return the original logging status.
* @api public
*/
function enable(flag)
{
	'use strict';

	var original = enabled();

	if (flag === undefined || flag === true) {
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
function log(var_args)
{
	'use strict';

	/* istanbul ignore if */
	if (process.env.NOLOG !== '1' && arguments.length > 0) {
		console.log.apply(null, arguments);
	}
}

module.exports = {
	enabled: enabled,
	enable: enable,
	log: log
};
