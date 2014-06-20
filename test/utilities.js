/**
 * @fileoverview Test for the module "utilities.js"
 * @author doberkofler
 */


/* jshint node: true */
/* global describe: false, it:false */


/**
* Module dependencies.
*/

var assert = require('chai').assert;
var utilities = require('../lib/utilities');

var mkdirp = require('mkdirp');
var path = require('path');
var fs = require('fs');


/**
* Module constants.
*/


/**
* Module variables.
*/


/**
* Tests.
*/
describe('absoluteFilename', function () {
	'use strict';

	describe('when getting the absolute filename', function () {
		var FILENAME = '../test.file';

		it('does return it', function () {
			assert.strictEqual(utilities.absoluteFilename(FILENAME), path.resolve(path.join(__dirname, FILENAME)));
		});
	});

});

describe('fileCopy', function () {
	'use strict';

	describe('when copying an existing file', function () {
		var SRC = 'temp/test.old',
			DST = 'temp/test.new';

		mkdirp('temp', function (err) {
			/* istanbul ignore if */
			if (err) {
				throw err;
			}
		});

		utilities.fileDelete(SRC);
		utilities.fileDelete(DST);
		fs.writeFileSync(SRC, 'content');

		it('does copy the file', function () {
			utilities.fileCopy(SRC, DST);

			assert.strictEqual(fs.existsSync(DST), true);
		});
	});

});