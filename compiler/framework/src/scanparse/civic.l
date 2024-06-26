/*
 * Module: civic.l
 * Description: Scanning of the input code. 
 * Authors: Aynel Gul & Andrea van den Hooff
 * Arrays implemented.
 */

%{

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "types.h"
#include "node_basic.h"
#include "str.h"
#include "free.h"
#include "civic.tab.h"
#include "globals.h"
#include "ctinfo.h"


#define FILTER(token) \
  global.col += yyleng;  \
  return( token);

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT
#define YY_NO_INPUT


%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */
%option noyywrap
%option nounput
%x C_COMMENT


%%

"("                        { FILTER( BRACKET_L);}
")"                        { FILTER( BRACKET_R);}
","                        { FILTER( COMMA);    }
";"                        { FILTER( SEMICOLON);}
"{"                        { FILTER( PAR_L);    }
"}"                        { FILTER( PAR_R);    }
"["                        { FILTER( SQBR_L);    }
"]"                        { FILTER( SQBR_R);    }

"-"                        { FILTER( MINUS);    }
"+"                        { FILTER( PLUS);     }
"*"                        { FILTER( STAR);     } 
"/"                        { FILTER( SLASH);    } 
"%"                        { FILTER( PERCENT);  } 

"<="                       { FILTER( LE);       }
"<"                        { FILTER( LT);       } 
">="                       { FILTER( GE);       }
">"                        { FILTER( GT);       }
"=="                       { FILTER( EQ);       }
"!="                       { FILTER( NE);       }
"&&"                       { FILTER( AND);      }
"||"                       { FILTER( OR);       }
"!"                        { FILTER( NOT);      }

"="                        { FILTER( LET);      }
"if"                       { FILTER( IF);       }
"else"                     { FILTER( ELSE);     }
"while"                    { FILTER( WHILE);    }
"do"                       { FILTER( DO);       }
"for"                      { FILTER( FOR);      }

"return"                   { FILTER( RETURN);   }
"extern"                   { FILTER( EXTERN);   }
"export"                   { FILTER( EXPORT);   }

"true"                     { FILTER( TRUEVAL);  }
"false"                    { FILTER( FALSEVAL); }

"int"                      { FILTER( INTTYPE);  }
"float"                    { FILTER( FLOATTYPE);}
"bool"                     { FILTER( BOOLTYPE); }
"void"                     { FILTER( VOIDTYPE); }

"\/\/".*                   { }

"/*"                       { BEGIN(C_COMMENT); }
<C_COMMENT>"*/"            { BEGIN(INITIAL); }
<C_COMMENT>\n              { }
<C_COMMENT>.               { }



[A-Za-z][A-Za-z0-9_]*      { 
                            yylval.id = STRcpy(yytext);
                            FILTER( ID);
                           }

[0-9]+                     { 
                              long long int test = atoll(yytext);
                              
                              if (test < INT_MIN || test > INT_MAX)
                                CTIabort("Integer overflow! Make sure all ints are between -2147483648 and 2147483647");
                              
                              yylval.cint = atoi(yytext);
                              FILTER( NUM);
                           }

[0-9]*[\.][0-9]*            { yylval.cflt = strtof(yytext, NULL);
                              if (errno)  CTIabort("Float value out of tange.");
                             FILTER( FLOAT);
                           }

\n.*                       { global.line += 1;
                             global.col = 0;
                             yyless(1);
                           }

[ \t]                      { global.col += yyleng;
                           } 
