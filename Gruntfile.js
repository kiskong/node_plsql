/* jshint node: true */

module.exports = function (grunt) {
	'use strict';

	// Explicitely set the DEBUG environment variable when running grunt
	process.env.DEBUG = '';
	process.env.NOLOG = '1';

	// Project configuration.
	grunt.initConfig({
		jscs: {
			src: [
				'Gruntfile.js',
				'lib/**/*.js',
				'test/**/*.js'
			],
			options: {
				config: '.jscsrc'
			}
		},
		jshint: {
			options: {
				jshintrc: '.jshintrc'
			},
			files: {
				src: [
					'Gruntfile.js',
					'lib/**/*.js',
					'test/**/*.js'
				]
			}
		},
		jsonlint: {
			package: {
				src: ['package.json']
			}
		},
		mochaTest: {
			test: {
				options: {
					require: 'test/support/env',
					reporter: 'spec'
				},
				src: ['test/**/*.js']
			}
		},
		mocha_istanbul: {
			coverage: {
				src: ['test', 'test/acceptance'], // the folder, not the files,
				options: {
				}
			},
			coveralls: {
				src: ['test', 'test/acceptance'], // the folder, not the files,
				options: {
					coverage: true,
					check: {
						lines: 90,
						statements: 90
					},
					root: './lib', // define where the cover task should consider the root of libraries that are covered by tests
					reportFormats: ['lcov']
				}
			}
		},
		watch: {
			gruntfile: {
				files: '<%= jshint.gruntfile.src %>',
				tasks: ['jshint:gruntfile']
			},
			lib: {
				files: '<%= jshint.lib.src %>',
				tasks: ['jshint:lib', 'nodeunit']
			},
			test: {
				files: '<%= jshint.test.src %>',
				tasks: ['jshint:test', 'nodeunit']
			}
		}
	});

	grunt.event.on('coverage', function (lcovFileContents, done) {
		done();
	});

	// These plugins provide necessary tasks.
	grunt.loadNpmTasks('grunt-contrib-clean');
	grunt.loadNpmTasks('grunt-contrib-copy');
	grunt.loadNpmTasks('grunt-contrib-jshint');
	grunt.loadNpmTasks('grunt-contrib-watch');
	grunt.loadNpmTasks('grunt-jscs-checker');
	grunt.loadNpmTasks('grunt-jsonlint');
	grunt.loadNpmTasks('grunt-mocha-test');

	grunt.loadNpmTasks('grunt-mocha-istanbul');

	// Default task.
	grunt.registerTask('default', ['lint', 'test']);

	// "lint" task.
	grunt.registerTask('lint', ['jscs', 'jshint', 'jsonlint']);

	// "test" task.
	grunt.registerTask('test', ['mochaTest', 'coveralls']);

	// "code coverage" tasks.
	grunt.registerTask('coveralls', ['mocha_istanbul:coveralls']);
	grunt.registerTask('coverage', ['mocha_istanbul:coverage']);
};
