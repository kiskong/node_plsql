CREATE OR REPLACE
PACKAGE BODY sample IS

PROCEDURE openPage(title IN VARCHAR2);
PROCEDURE closePage;

PROCEDURE pageIndex
IS
BEGIN
	openPage('PLSQL-SERVER - Index Page');
	htp.p('<ul>');
	htp.p('<li><a href="sample.pageSimple?text=some-text">Simple page</a></li>');
	htp.p('<li><a href="!sample.pageFlexible?p1=v1'||CHR(38)||'p2=v2'||CHR(38)||'p3=v3">Flexible parameter passing</a></li>');
	htp.p('<li><a href="sample.pageCGI">CGI</a></li>');
	htp.p('<li><a href="sample.pageCookie">Cookies</a></li>');
	htp.p('<li><a href="sample.pageRedirect">Redirect</a></li>');
	htp.p('<li><a href="sample.pageLocation">Change location</a></li>');
	htp.p('</ul>');
	closePage();
END pageIndex;

PROCEDURE pageSimple(text IN VARCHAR2 DEFAULT NULL)
IS
BEGIN
	openPage('PLSQL-SERVER - Simple page');
	htp.p('<p>'||text||'</p>');
	closePage();
END pageSimple;

PROCEDURE pageFlexible(name_array IN owa.vc_arr, value_array IN owa.vc_arr)
IS
BEGIN
	openPage('PLSQL-SERVER - Flexible parameter passing');
	htp.p('<table>');
	htp.p('<tr><th>name</th><th>value</th></tr>');
	FOR i IN 1 .. name_array.COUNT LOOP
		htp.p('<tr><td>'||name_array(i)||'</td><td>'||value_array(i)||'</td></tr>');
	END LOOP;
	htp.p('</table>');
	closePage();
END pageFlexible;

PROCEDURE pageCGI
IS
BEGIN
	openPage('PLSQL-SERVER - CGI');
	htp.p('<table>');
	htp.p('<tr><th>name</th><th>value</th></tr>');
	FOR i IN 1 .. owa.num_cgi_vars LOOP
		htp.p('<tr><td>'||owa.cgi_var_name(i)||'</td><td>'||owa.cgi_var_val(i)||'</td></tr>');
	END LOOP;
	htp.p('</table>');
	closePage();
END pageCGI;

PROCEDURE pageCookie
IS
	names		owa_cookie.vc_arr;
	vals		owa_cookie.vc_arr;
	num_vals	INTEGER;
BEGIN
	owa_cookie.get_all(names=>names, vals=>vals, num_vals=>num_vals);

	owa_util.mime_header('text/html', FALSE);
	owa_cookie.send('demoCookie', TO_CHAR(SYSDATE, 'YYYY.MM.DD HH24:MI:SS'));
	owa_util.http_header_close;

	openPage('PLSQL-SERVER - Cookies');
	htp.p('<table>');
	htp.p('<tr><th>name</th><th>value</th></tr>');
	FOR i IN 1 .. num_vals LOOP
		htp.p('<tr><td>'||names(i)||'</td><td>'||vals(i)||'</td></tr>');
	END LOOP;
	htp.p('</table>');
	closePage();
END pageCookie;

PROCEDURE pageRedirect
IS
BEGIN
	owa_util.mime_header('text/html', FALSE);
	owa_cookie.send('fromPage', 'sample.pageRedirect');
	owa_util.redirect_url(curl=>'sample.pageOther', bclose_header=>FALSE);
	owa_util.http_header_close;
END pageRedirect;

PROCEDURE pageLocation
IS
BEGIN
	openPage('PLSQL-SERVER - pageLocation');
	htp.p('<script>self.top.location.replace("sample.pageOther");</script>');
	closePage();
END pageLocation;

PROCEDURE pageOther
IS
BEGIN
	openPage('PLSQL-SERVER - Other page');
	closePage();
END pageOther;

PROCEDURE openPage(title IN VARCHAR2)
IS
BEGIN
	htp.p('<html><head><title>'||title||'</title></head><body><h1>'||title||'</h1>');
	htp.p('<p>'||TO_CHAR(SYSDATE, 'YYYY.MM.DD HH24:MI:SS')||'</p>');
	htp.p('<p><a href="sample.pageIndex">Menu</a></p>');
END openPage;

PROCEDURE closePage
IS
BEGIN
	htp.p('</body></html>');
END closePage;

END sample;
/
