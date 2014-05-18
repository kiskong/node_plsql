CREATE OR REPLACE
PACKAGE sample IS

PROCEDURE pageIndex;
PROCEDURE pageSimple(text IN VARCHAR2 DEFAULT NULL);
PROCEDURE pageFlexible(name_array IN owa.vc_arr, value_array IN owa.vc_arr);
PROCEDURE pageCGI;
PROCEDURE pageCookie;
PROCEDURE pageRedirect;
PROCEDURE pageLocation;
PROCEDURE pageOther;

END sample;
/
