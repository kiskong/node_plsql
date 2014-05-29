/* jshint node: true */

module.exports = function (grunt) {
	'use strict';

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
		jshint: {
			options: {
				jshintrc: '.jshintrc'
			},
			gruntfile: {
				src: 'Gruntfile.js'
			},
			lib: {
				src: ['lib/**/*.js']
			},
			test: {
				src: ['test/**/*.js']
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
					reporter: 'spec'
				},
				src: ['coverage/test/**/*.js']
			},
			coverage: {
				options: {
					reporter: 'html-cov',
					quiet: true,
					captureFile: 'coverage/coverage.html'
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
	grunt.loadNpmTasks('grunt-jsonlint');
	grunt.loadNpmTasks('grunt-mocha-test');

	// Default task.
	grunt.registerTask('default', ['jshint', 'jsonlint', 'clean', 'blanket', 'copy', 'mochaTest']);

	// "lint" task.
	grunt.registerTask('lint', ['jshint', 'jsonlint']);

	// "test" task.
	grunt.registerTask('test', ['clean', 'blanket', 'copy', 'mochaTest']);
};
