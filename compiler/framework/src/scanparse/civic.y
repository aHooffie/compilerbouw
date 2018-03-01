%{


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"

static node *parseresult = NULL;
extern int yylex();
static int yyerror( char *errname);

%}

%union {
 nodetype            nodetype;
 char                *id;
 int                 cint;
 float               cflt;
 binop               cbinop;
 monop               cmonop;
 node                *node;
 type                ctype;
}

%token BRACKET_L BRACKET_R COMMA SEMICOLON PAR_L PAR_R
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND NOT
%token TRUEVAL FALSEVAL LET IF ELSE WHILE DO RETURN FOR
%token INTTYPE FLOATTYPE BOOLTYPE VOIDTYPE
%token EXTERN EXPORT

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant exprs expr
%type <node> funbody vardec vardecs params param
%type <node> stmts stmt assign varlet program if while dowhile block return for
%type <node> binop monop
%type <ctype> type

%left MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND NOT NEG
%right BRACKET_R PAR_R
%left ELSE
%right BRACKET_L PAR_L

%start program

%%

program: funbody 
         {
           parseresult = $1;
         }
         ;

// [ export ] RetType Id ( [ Param [ , Param ]* ] ) { FunBody }


function: type ID BRACKET_L params BRACKET_R PAR_L funbody PAR_R
          {
            $$ = TBmakeFunction($1, $2, $7, $4);
          }
          |
          type ID BRACKET_L BRACKET_R PAR_L funbody PAR_R
          {
             $$ = TBmakeFunction($1, $2, $7, NULL);
          }
          ;

params: param params
        {
          PARAMETERS_NEXT($1) = $2;
        } 
        | param
        {
          $$ = $1;
        } 
        ;

param: ID type
        {
          TBmakeParameters($1, $2, NULL, NULL);
        }
        ;   

funbody: vardecs stmts
        {
          $$ = TBmakeFunctionbody($2, NULL, $1);
        }
        | vardecs
        {
          $$ = TBmakeFunctionbody(NULL, NULL, $1);
        }
        | stmts
        { 
          $$ = TBmakeFunctionbody($1, NULL, NULL);
        }
        |
        {
          $$ = TBmakeFunctionbody(NULL, NULL, NULL);
        }
      
        ;

vardecs: vardec vardecs // GEEN ARRAY
          {
            VARDECLARATION_NEXT($1) = $2;
          }
          | vardec
          {
            $$ = $1;
          }
          ;      

vardec: type ID LET expr SEMICOLON  // GEEN ARRAY GEEN NEXT
        {
          $$ = TBmakeVardeclaration($2, $1, NULL, $4, NULL);
        }
        | type ID SEMICOLON // GEEN ARRAY GEEN INIT GEEN NEXT
        {
          $$ = TBmakeVardeclaration($2, $1, NULL, NULL, NULL);
        }
        ;

stmts:  stmt stmts
        {
          $$ = TBmakeStmts( $1, $2);
        }
        | stmt
        {
          $$ = TBmakeStmts( $1, NULL);
        }
        ;

stmt:   assign
        {
           $$ = $1;
        }
        | ID BRACKET_L exprs BRACKET_R SEMICOLON
        {
          $$ = TBmakeFunctioncallstmt( STRcpy( $1), $3);
        }
        | ID BRACKET_L BRACKET_R SEMICOLON
        {
          $$ = TBmakeFunctioncallstmt( STRcpy( $1), NULL);
        } 
        | while
        {
           $$ = $1;
        }
        | if
        {
          $$ = $1;
        }
        | return
        {
          $$ = $1;
        }
        | dowhile
        {
          $$ = $1;
        }
        | for
        {
          $$ = $1;
        }
        ;

assign: varlet LET expr SEMICOLON
        {
          $$ = TBmakeAssign( $1, $3);
        }
        ;

if:     IF BRACKET_L expr BRACKET_R block
        {
          $$ = TBmakeIfelse($3, $5, NULL);
        }
        | IF BRACKET_L expr BRACKET_R block ELSE block
        {
          $$ = TBmakeIfelse($3, $5, $7);
        }
        ;

while:  WHILE BRACKET_L expr BRACKET_R block
        {
          $$ = TBmakeWhile($3, $5);
        }
        ;

return: RETURN SEMICOLON
        {
          $$ = TBmakeReturn(NULL);
        }
        | RETURN expr SEMICOLON
        {
          $$ = TBmakeReturn($2);
        }
        ;

dowhile: DO block WHILE BRACKET_L expr BRACKET_R SEMICOLON
        {
          $$ = TBmakeDowhile($2, $5);
        }
        ;

block:  PAR_L stmts PAR_R
        {
          $$ = $2;
        }
        | PAR_L PAR_R
        {
          $$ = NULL;
        }
        | stmt
        {
          $$ = TBmakeStmts( $1, NULL);
        }
        ;

for:    FOR BRACKET_L INTTYPE ID LET expr COMMA expr COMMA expr BRACKET_R block
        {
          $$ = TBmakeFor($4, $6, $8, $10, $12);
        }
        | FOR BRACKET_L INTTYPE ID LET expr COMMA expr BRACKET_R block
        {
          $$ = TBmakeFor($4, $6, $8, NULL, $10);
        }
        ;

varlet: ID
        {
          $$ = TBmakeVarlet( STRcpy( $1), NULL, NULL);
        }
        | ID varlet
        {
          $$ = TBmakeVarlet( STRcpy ($1), $2, NULL);
        }
            // Mogelijk nog Array indices als input ($3).
            // Hoe VarLet testen?
        ;


exprs: expr COMMA exprs
      {
        $$ = TBmakeExpressions( $1, $3);
      }
      | expr
      {
        $$ = TBmakeExpressions( $1, NULL);
      }
      ;

expr: BRACKET_L expr BRACKET_R  
      {
        $$ = $2;
      }
    | binop
      {
        $$ = $1;
      }
    | monop 
      {
        $$ = $1;
      } 
    | BRACKET_L type BRACKET_R expr
      {
        $$ = TBmakeCast( $2, $4);
      }
    | ID BRACKET_L exprs BRACKET_R
    {
      $$ = TBmakeFunctioncallexpr( STRcpy( $1), $3);
    }
    | ID BRACKET_L BRACKET_R
    {
      $$ = TBmakeFunctioncallexpr( STRcpy( $1), NULL);
    } 
    | ID
      {
        $$ = TBmakeVar( STRcpy( $1), NULL);
      }
    | constant
      {
        $$ = $1;
      }  
    ;

constant: floatval
          {
            $$ = $1;
          }
        | intval
          {
            $$ = $1;
          }
        | boolval
          {
            $$ = $1;
          }
        ;

floatval: FLOAT
           {
             $$ = TBmakeFloat( $1);
           }
         ;

intval: NUM
        {
          $$ = TBmakeNum( $1);
        }
      ;

boolval: TRUEVAL
         {
           $$ = TBmakeBool( TRUE);
         }
       | FALSEVAL
         {
           $$ = TBmakeBool( FALSE);
         }
       ;

binop: expr PLUS expr      { $$ = TBmakeBinop(BO_add, $1, $3); }
     | expr MINUS expr     { $$ = TBmakeBinop(BO_sub, $1, $3); }
     | expr STAR expr      { $$ = TBmakeBinop(BO_mul, $1, $3); }
     | expr SLASH expr     { $$ = TBmakeBinop(BO_div, $1, $3); }
     | expr PERCENT expr   { $$ = TBmakeBinop(BO_mod, $1, $3); }
     | expr LE expr        { $$ = TBmakeBinop(BO_le, $1, $3); }
     | expr LT expr        { $$ = TBmakeBinop(BO_lt, $1, $3); }
     | expr GE expr        { $$ = TBmakeBinop(BO_ge, $1, $3); }
     | expr GT expr        { $$ = TBmakeBinop(BO_gt, $1, $3); }
     | expr EQ expr        { $$ = TBmakeBinop(BO_eq, $1, $3); }
     | expr NE expr        { $$ = TBmakeBinop(BO_ne, $1, $3); }
     | expr OR expr        { $$ = TBmakeBinop(BO_or, $1, $3); }
     | expr AND expr       { $$ = TBmakeBinop(BO_and, $1, $3); }
     ;

monop: MINUS expr      { $$ = TBmakeMonop(MO_neg, $2); }     
      | NOT expr       { $$ = TBmakeMonop(MO_not, $2); }     
      ;

type: INTTYPE            { $$ = T_int; }     
      | FLOATTYPE       { $$ = T_float; } 
      | BOOLTYPE        { $$ = T_bool; } 
      | VOIDTYPE         { $$ = T_void; } 
      ;
%%

static int yyerror( char *error)
{
  CTIabort( "line %d, col %d\nError parsing source code: %s\n", 
            global.line, global.col, error);

  return( 0);
}

node *YYparseTree( void)
{
  DBUG_ENTER("YYparseTree");

  yyparse();

  DBUG_RETURN( parseresult);
}

