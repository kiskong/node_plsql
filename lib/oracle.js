/* jshint node: true */

/**
* Module dependencies.
*/

var oracleBindings = require('../build/Release/oracleBindings');
var debug = require('debug')('plsql-server:oracle');
var underscore = require('underscore');

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
* @api private
*/
function installCode(oracle) {
	'use strict';

	debug('Installing procedural code ...');
	oracle.execute(NODE_PLSQL_PACKAGE);
	oracle.execute(NODE_PLSQL_PACKAGE_BODY);
}

/**
* Create a new oracle connection
*
* @api private
*/
function createOracle(service) {
	'use strict';

	var defaults = {
		username: '',
		password: '',
		sysdba: false,
		hostname: 'localhost',
		port: 1521,
		service: '',
		debug: false
	};

	var oracleConfig = {
		username: service.oracleUsername,
		password: service.oraclePassword,
		sysdba: false,
		hostname: service.oracleHostname,
		port: service.oraclePort,
		service: service.oracleService
	};

	oracleConfig = underscore.extend(defaults, oracleConfig);

	debug('Open Oracle connection for ' + oracleConfig.username + '...');

	// Create Oracle object
	service.oracleSession = new oracleBindings.OracleBindings(oracleConfig);

	// Prepare Oracle object
	service.oracleSession.prepare();

	// Connect with database
	service.oracleSession.connect();

	// Install procedural code
	installCode(service.oracleSession);
}

/**
* Create the needed Oracle connections
*
* @api public
*/
function createNeededOracleConnections(services) {
	'use strict';

	var i = 0,
		s = {};

	for (i = 0; i < services.length; i++) {
		s = services[i];
		createOracle(s);
	}
}

/**
* Invoke the PL/SQL code and return the page content
*
* @api public
*/
var invokeProcedure = function (oracleSession, procedure, args, cgi) {
	'use strict';

	// Invoke the procedure and return the page contents
	var page = oracleSession.request(procedure, args, cgi);

	return page;
};

module.exports = {
	createNeededOracleConnections: createNeededOracleConnections,
	invokeProcedure: invokeProcedure
};
