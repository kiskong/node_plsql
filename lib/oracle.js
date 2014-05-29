/* jshint node: true */

/**
* Module dependencies.
*/

var oracleBindings = require('../build/Release/oracleBindings');
var debug = require('debug')('node_plsql:oracle');

/**
* Module variables.
*/
var NODE_PLSQL_PACKAGE = [
	'CREATE OR REPLACE',
	'PACKAGE node_plsql',
	'IS',
	'	PROCEDURE get_page(page OUT NOCOPY CLOB);',
	'END node_plsql;'
].join('\n');

var NODE_PLSQL_PACKAGE_BODY = [
	'CREATE OR REPLACE',
	'PACKAGE BODY node_plsql',
	'IS',
	'',
	'PROCEDURE get_page(page OUT NOCOPY CLOB)',
	'IS',
	'	MAX_LINES_TO_FETCH	CONSTANT	INTEGER			:=	2147483647;',
	'	TRASHHOLD			CONSTANT	BINARY_INTEGER	:=	32767;',
	'',
	'	iRows							INTEGER			:=	MAX_LINES_TO_FETCH;',
	'	pageBuf							htp.htbuf_arr;',
	'	textSize						BINARY_INTEGER;',
	'	i								BINARY_INTEGER;',
	'',
	'	pageLOB							CLOB;',
	'	pageTXT							VARCHAR2(32767);',
	'BEGIN',
	'	--	get the content of the page',
	'	owa.get_page(thepage=>pageBuf, irows=>iRows);',
	'	IF (NOT iRows < MAX_LINES_TO_FETCH) THEN',
	'		RAISE VALUE_ERROR;',
	'	END IF;',
	'',
	'	--	allocate a temporary CLOB',
	'	dbms_lob.createtemporary(lob_loc=>pageLOB, cache=>TRUE, dur=>dbms_lob.session);',
	'',
	'	--	append the page rows to the CLOB',
	'	FOR i IN 1 .. iRows LOOP',
	'		textSize := LENGTH(pageBuf(i));',
	'		IF (textSize > 0) THEN',
	'			-- can we just concatenate strings ?',
	'			IF (LENGTH(pageTXT) + textSize < TRASHHOLD) THEN',
	'				pageTXT := pageTXT || pageBuf(i);',
	'			ELSE',
	'				-- append the existing string buffer',
	'				IF (pageTXT IS NOT NULL) THEN',
	'					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);',
	'					pageTXT := NULL;',
	'				END IF;',
	'				-- should we use the string or rather already switch to the CLOB ?',
	'				IF (textSize < TRASHHOLD) THEN',
	'					pageTXT := pageTXT || pageBuf(i);',
	'				ELSE',
	'					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>textSize, buffer=>pageBuf(i));',
	'				END IF;',
	'			END IF;',
	'		END IF;',
	'	END LOOP;',
	'',
	'	--	make sure that we also flushed the last text buffer',
	'	IF (pageTXT IS NOT NULL) THEN',
	'		dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);',
	'		pageTXT := NULL;',
	'	END IF;',
	'',
	'	--	copy the CLOB',
	'	page := pageLOB;',
	'',
	'	--	free the temporary CLOB',
	'	dbms_lob.freetemporary(lob_loc=>pageLOB);',
	'END get_page;',
	'',
	'END node_plsql;'
].join('\n');

/**
* Install procedural code
*
* @param {Object} oracle Oracle binding object
* @param {String} username Oracle username
* @param {String} password Oracle password
* @api private
*/
function installCode(oracle, username, password) {
	'use strict';

	debug('Installing procedural code ...');

	// Install procedural code
	oracle.executeSync(username, password, NODE_PLSQL_PACKAGE);
	oracle.executeSync(username, password, NODE_PLSQL_PACKAGE_BODY);
}

/**
* Connect with Oracle
*
* @param {Object} config Configuration object
* @api public
*/
function connect(config) {
	'use strict';

	// If we have no services, just return
	if (config.services.length <= 0) {
		return;
	}

	// Oracle configuration
	var oracleConfig = {
		username: config.services[0].databaseUsername || '',
		password: config.services[0].databasePassword || '',
		database: config.services[0].databaseConnectString || '',
		oracleConnectionPool: config.server.oracleConnectionPool,
		oracleDebug: config.server.oracleDebug
	};

	// Show connect message
	var text = 'Connect with Oracle as ' + oracleConfig.username;
	if (oracleConfig.database.length > 0) {
		text += '@' + oracleConfig.database;
	}
	if (debug.enabled) {
		debug(text);
	} else {
		console.log(text);
	}

	// Create Oracle binding object
	var oracle = new oracleBindings.OracleBindings(oracleConfig);

	// Initialize Oracle and create the connection pool
	oracle.create();

	// Install procedural code needed in each schema
	var i = 0;
	for (i = 0; i < config.services.length; i++) {
		installCode(oracle, config.services[i].databaseUsername, config.services[i].databasePassword);
	}

	return oracle;
}

/**
* Invoke the PL/SQL code and execute the callback when done
*
* @param {Object} oracle Oracle binding object.
* @param {String} username Oracle username.
* @param {String} password Oracle password.
* @param {String} procedure PL/SQL procedure to execute.
* @param {Object} args Object with the arguments for the PL/SQL procedure as properties.
* @param {Array} cgi Array of cgi variables to send for the PL/SQL code.
* @param {Array} files Array of files to upload.
* @param {String} doctablename Document table name.
* @param {Function} callback Callback function (function cb(err, page)) to invoke when done.
* @api public
*/
function invoke(oracle, username, password, procedure, args, cgi, files, doctablename, callback)
{
	'use strict';

	// Invoke the procedure and return the page contents
	oracle.request(username, password, procedure, args, cgi, files, doctablename, callback);
}

module.exports = {
	connect: connect,
	invoke: invoke
};
