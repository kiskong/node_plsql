/*eslint strict:[2, "global"]*/

'use strict';

//
//	Requirements
//
var gulp = require('gulp'),
	glob = require('glob'),
	coveralls = require('gulp-coveralls'),
	gutil = require('gulp-util'),
	istanbul = require('gulp-istanbul'),
	eslint = require('gulp-eslint'),
	jsonlint = require('gulp-jsonlint'),
	mocha = require('gulp-mocha'),
	runSequence = require('run-sequence'),
	through = require('through');

//
//	Constants
//
var JS_SRC_FILES = [
	'Gulpfile.js',
	'lib/**/*.js',
	'test/**/*.js'
];
var MOCHA_OPTIONS = {
	require: 'test/support/env',
	reporter: 'spec'
};

process.env.NODE_ENV = 'test';

//
//	Variables
//

//
//	Local functions
//

/**
 * Count the number of files in a stream
 * @function	numberOfFiles
 * @param		{String}	message		The message to show
 * @returns		{*}						The stream
 * @example		gulp.src(*.js').numberOfFiles(' files processed.')
 */
function numberOfFiles(message) {
	var count = 0;

	function countFiles(/*file*/) {
		count++;
	}

	function endStream() {
		gutil.log(gutil.colors.magenta(count), message);
		/*jshint validthis:true */
		this.emit('end');
	}

	return through(countFiles, endStream);
}

//
//	Main
//

//
//	The task "eslint" will lint the js files
//
gulp.task('eslint', function () {
	return gulp.src(JS_SRC_FILES)
		.pipe(eslint())
		.pipe(eslint.formatEach())
		.pipe(eslint.failOnError())
		.pipe(numberOfFiles('files lint free.'));
});

//
// This is the "jsonlint" task that Gulp will use to lint the json files
//
gulp.task('jsonlint', function (/*callback*/) {
	return gulp.src('package.json')
		.pipe(jsonlint())
		.pipe(jsonlint.reporter());
});

//
//	The tasks "test" will run all unit tests (no database is needed) in the mocha test runner
//
glob.sync('test/*.js', {}).forEach(function (file) {
	var task = file.toLowerCase();

	task = task.replace(/test\//gi, 'test:');
	task = task.replace(/\.js/gi, '');
	task = task.replace(/\//gi, ':');

	gulp.task(task, function (/*callback*/) {
		return gulp.src(file, {read: false})
			.pipe(mocha(MOCHA_OPTIONS));
	});
});
gulp.task('test', function (/*callback*/) {
	return gulp.src(['test/*.js'], {read: false})
		.pipe(mocha(MOCHA_OPTIONS));
});

//
//	The tasks "coverage" will analyze the coverage of the unit tests
//
gulp.task('coverage', function (callback) {
	gulp.src(['lib/**/*.js'])
		.pipe(istanbul())				// Covering files
		.pipe(istanbul.hookRequire())	// Force 'require' to return covered files
		.on('finish', function () {
			gulp.src(['test/*.js'])
			.pipe(mocha())
			.pipe(istanbul.writeReports()) // Creating the reports after tests runned
			.on('end', callback);
		});
});

//
//	The tasks "coveralls" will upload the coverage to te coveralls service
//
gulp.task('coveralls', function (/*callback*/) {
	return gulp.src('./coverage/lcov.info')
		.pipe(coveralls());
});

//
// Meta tasks
//
gulp.task('lint', function (callback) {
	runSequence('eslint',
				'jsonlint',
				callback
				);
});
gulp.task('default', function (callback) {
	runSequence('lint',
				'test',
				'coverage',
				callback
				);
});
