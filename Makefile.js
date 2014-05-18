/**
 * @fileoverview Build file
 * @author doberkofler
 */


/* jshint node: true */
/* global target, exec:false, echo:false, find:false */

//------------------------------------------------------------------------------
// Requirements
//------------------------------------------------------------------------------

require('shelljs/make');
var nodeCLI = require('shelljs-nodecli');
//var path = require('path');
//var os = require('os');

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------

/**
 * Generates a function that matches files with a particular extension.
 * @param {string} extension The file extension (i.e. 'js')
 * @returns {Function} The function to pass into a filter method.
 * @private
 */
function fileType(extension) {
	'use strict';

	return function (filename) {
		return filename.substring(filename.lastIndexOf('.') + 1) === extension;
	};
}

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------

var NODE = 'node',
	NODE_MODULES = './node_modules/',
	MOCHA = NODE_MODULES + 'mocha/bin/_mocha',
	JSHINT = 'jshint',
	JSONLINT = 'jsonlint',

	// Files
	TEST_FILES = find('test/').filter(fileType('js')).join(' '),
	JS_FILES = find('lib/').filter(fileType('js')).join(' ') + ' ' + find('./Makefile.js').filter(fileType('js')).join(' ') + ' ' + TEST_FILES,
	JSON_FILES = 'package.json';

//------------------------------------------------------------------------------
// Tasks
//------------------------------------------------------------------------------

target.all = function () {
	'use strict';

	target.lint();
	target.test();
};

target.lint = function () {
	'use strict';

	echo('Validating JSON Files');
	exec(JSONLINT + ' -q -c ' + JSON_FILES);

	echo('Validating JavaScript files');
	exec(JSHINT + ' ' + JS_FILES);
};

target.test = function () {
	'use strict';

	nodeCLI.exec('istanbul', 'cover', MOCHA, '-- -c', TEST_FILES);
	nodeCLI.exec(NODE, MOCHA, TEST_FILES);
	//nodeCLI.exec('istanbul', 'check-coverage', '--statement 99 --branch 98 --function 99 --lines 99');
	//nodeCLI.exec('mocha-phantomjs', '-R dot', 'tests/tests.htm');
};
