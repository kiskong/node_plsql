/**
 * @fileoverview Expose the node_plsql API and CLI to require.
 * @author Dieter Oberkofler
 */

module.exports = {
	/*eslint-disable camelcase */
	node_plsql: require('./node_plsql'),
	/*eslint-enable camelcase */
	cli: require('./cli')
};
