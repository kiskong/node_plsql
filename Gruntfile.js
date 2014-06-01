/* jshint node: true */

module.exports = function (grunt) {
	'use strict';

	// Explicitely set the DEBUG environment variable when running grunt
	process.env.DEBUG = '';

	// Project configuration.
	grunt.initConfig({
		clean: {
			coverage: {
				src: ['coverage/']
			}
		},
		copy: {
			coverage: {
				src: ['test/**'],
				dest: 'coverage/'
			},
			needed_files_for_tests: {
				src: ['package.json', 'conf/*'],
				dest: 'coverage/'
			}
		},
		blanket: {
			coverage: {
				src: ['lib/'],
				dest: 'coverage/lib/'
			}
		},
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
			},
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
				src: ['coverage/test/**/*.js']
			},
			coverage: {
				options: {
					reporter: 'html-cov',
					quiet: true,
					captureFile: 'coverage.html'
				},
				src: ['coverage/test/**/*.js']
			},
			'travis-cov': {
				options: {
					reporter: 'travis-cov'
				},
				src: ['coverage/test/**/*.js']
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

	// These plugins provide necessary tasks.
	grunt.loadNpmTasks('grunt-blanket');
	grunt.loadNpmTasks('grunt-contrib-clean');
	grunt.loadNpmTasks('grunt-contrib-copy');
	grunt.loadNpmTasks('grunt-contrib-jshint');
	grunt.loadNpmTasks('grunt-contrib-watch');
	grunt.loadNpmTasks('grunt-jscs-checker');
	grunt.loadNpmTasks('grunt-jsonlint');
	grunt.loadNpmTasks('grunt-mocha-test');

	// Default task.
	grunt.registerTask('default', ['lint', 'test']);

	// "lint" task.
	grunt.registerTask('lint', ['jscs', 'jshint', 'jsonlint']);

	// "test" task.
	grunt.registerTask('test', ['clean', 'blanket', 'copy', 'mochaTest']);
};
