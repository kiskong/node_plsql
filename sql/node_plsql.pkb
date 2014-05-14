CREATE OR REPLACE
PACKAGE BODY node_plsql
IS

PROCEDURE get_page(page OUT NOCOPY CLOB)
IS
	MAX_LINES_TO_FETCH	CONSTANT	INTEGER			:=	2147483647;
	TRASHHOLD			CONSTANT	BINARY_INTEGER	:=	2000;

	iRows							INTEGER			:=	MAX_LINES_TO_FETCH;
	pageBuf							htp.htbuf_arr;
	textSize						BINARY_INTEGER;
	i								BINARY_INTEGER;

	pageLOB							CLOB;
	pageTXT							VARCHAR2(32767);
BEGIN
	--	get the content of the page
	owa.get_page(thepage=>pageBuf, irows=>iRows);
	IF (NOT iRows < MAX_LINES_TO_FETCH) THEN
		RAISE VALUE_ERROR;
	END IF;

	--	allocate a temporary CLOB
	dbms_lob.createtemporary(lob_loc=>pageLOB, cache=>TRUE, dur=>dbms_lob.session);

	--	append the page rows to the CLOB
	FOR i IN 1 .. iRows LOOP
		textSize := LENGTH(pageBuf(i));
		IF (textSize > 0) THEN
			-- can we just concatenate strings ?
			IF (LENGTH(pageTXT) + textSize < TRASHHOLD) THEN
				pageTXT := pageTXT || pageBuf(i);
			ELSE
				-- append the existing string buffer
				IF (pageTXT IS NOT NULL) THEN
					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);
					pageTXT := NULL;
				END IF;
				-- should we use the string or rather already switch to the CLOB ?
				IF (textSize < TRASHHOLD) THEN
					pageTXT := pageTXT || pageBuf(i);
				ELSE
					dbms_lob.writeappend(lob_loc=>pageLOB, amount=>textSize, buffer=>pageBuf(i));
				END IF;
			END IF;
		END IF;
	END LOOP;

	--	make sure that we also flushed the last text buffer
	IF (pageTXT IS NOT NULL) THEN
		dbms_lob.writeappend(lob_loc=>pageLOB, amount=>LENGTH(pageTXT), buffer=>pageTXT);
		pageTXT := NULL;
	END IF;

	--	copy the CLOB
	page := pageLOB;

	--	free the temporary CLOB
	dbms_lob.freetemporary(lob_loc=>pageLOB);
END get_page;

END node_plsql;
/
