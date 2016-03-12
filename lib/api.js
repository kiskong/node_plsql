'use strict';

/**
 * @fileoverview Expose the node_plsql API and CLI to require.
 * @author Dieter Oberkofler
 */

module.exports = {
	/*eslint-disable camelcase, global-require */
	node_plsql: require('./node_plsql'),
	cli: require('./cli')
	/*eslint-enable camelcase, global-require */
};
