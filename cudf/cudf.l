
/*******************************************************/
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010         */
/* Distributed under GPLv3                             */
/*******************************************************/


/* A scanner for the CUDF */

%{

#include <cudf/cudf.h>
#include <stdio.h>
#include "cudf.tab.hpp"

#define STRLIMIT 5000
char astring[STRLIMIT];
int strsize = 0;

extern int is_pstring(char *pname);
extern int pidenttype(char *pname);

%}
%x anstr keepstate proptype linestring
%option bison-bridge
%option yylineno
%option noyywrap

%%

preamble:       return PREAMBLE;

property:       return PROPERTYDEF;

package:        return PACKAGE;
version:        return VERSION;
depends:        return DEPENDS;
conflicts:      return CONFLICTS;
provides:       return PROVIDES;
installed:      return INSTALLED;
was-installed:  return WASINSTALLED;
keep:           { BEGIN(keepstate); return KEEP; }

true            return TRUE;
false           return FALSE;

=               return EQ;
!=              return NEQ;
>               return SUP;
>=              return SUPEQ;
\<              return INF;
\<=             return INFEQ;


<keepstate>version         { BEGIN(INITIAL); return KEEPVERSION; }
<keepstate>package         { BEGIN(INITIAL); return KEEPPACKAGE; }
<keepstate>feature         { BEGIN(INITIAL); return KEEPFEATURE; }
<keepstate>none            { BEGIN(INITIAL); return KEEPNONE; }
<keepstate>[\t ]
<keepstate>.               fprintf(stderr, "CUDF scanner error: Unrecognized character in keep state value: %s\n", yytext);

request:[^\n]*      return PROBLEM;
install:            return INSTALL;
remove:             return REMOVE;
upgrade:            return UPGRADE;

univ-checksum:[^\n]* /* Ignore */
status-checksum:[^\n]* /* Ignore */
req-checksum:[^\n]* /* Ignore */

\"            strsize = 0; BEGIN(anstr);
<anstr>\"       { BEGIN(INITIAL); return STRING; }
<anstr>\\\"     { if (strsize < STRLIMIT) astring[strsize++] = yytext[1]; 
                      else { fprintf(stderr, "CUDF error (line %d): string are limited to %d characters.\n", cudflineno, STRLIMIT); exit(-1); } }
<anstr>[^"]     { if (strsize < STRLIMIT) astring[strsize++] = yytext[0]; 
                      else { fprintf(stderr, "CUDF error (line %d): string are limited to %d characters.\n", cudflineno, STRLIMIT); exit(-1); } }

<linestring>[^\n]* { BEGIN(INITIAL); strcpy(astring, (yytext + 1)); return STRING; }

[+-]?[0-9]+              { strcpy(cudflval.str, yytext); return INTEGER; }

[a-zA-Z0-9+*_%/\-\.@]*":" { strcpy(cudflval.str, yytext); 
                            switch(pidenttype(yytext)) {
                              case pt_bool:
                                return PIDENT_BOOL;
                              case pt_int: 
                                return PIDENT_INT;
                              case pt_nat: 
                                return PIDENT_NAT;
                              case pt_posint: 
                                return PIDENT_POSINT;
                              case pt_enum: 
                                return PIDENT_ENUM;
                              case pt_string: 
                                BEGIN(linestring);
                                return PIDENT_STRING;
                              case pt_vpkg: 
                                return PIDENT_VPKG;
                              case pt_veqpkg: 
                                return PIDENT_VEQPKG;
                              case pt_vpkglist: 
                                return PIDENT_VPKGLIST;
                              case pt_veqpkglist: 
                                return PIDENT_VEQPKGLIST;
                              case pt_vpkgformula:
                                return PIDENT_VPKGFORMULA;
                              case pt_none:
                              default:
                                return PIDENT; 
                            }
                          }

[a-zA-Z0-9+*_%/\-\.@()]* { strcpy(cudflval.str, yytext); return IDENT; }


#[^\n]*   /* comment */

[,:|\[\]]   return yytext[0];

[\t\n ]   /* Just ignore spaces */
.         fprintf(stderr, "CUDF scanner error: Unrecognized character: %s\n", yytext);

%%



