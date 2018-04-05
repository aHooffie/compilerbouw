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

/*
 * Module: Scanning & Parsing of the input code. 
 * Authors: Aynel Gul & Andrea van den Hooff
 * Arrays not implemented.
 */


static node *parseresult = NULL;
extern int yylex();
static int yyerror( char *errname);
node *reverselist(node *vardecs);

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

%token BRACKET_L BRACKET_R COMMA SEMICOLON PAR_L PAR_R SQBR_L SQBR_R
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND NOT
%token TRUEVAL FALSEVAL LET IF ELSE WHILE DO RETURN FOR
%token INTTYPE FLOATTYPE BOOLTYPE VOIDTYPE
%token EXTERN EXPORT

%token <cint> NUM
%token <cflt> FLOAT
%token <id> ID

%type <node> intval floatval boolval constant exprs expr
%type <node> declarations declaration globaldec globaldef function 
%type <node> funbody vardec vardecs params param localfunctions localfunction
%type <node> stmts stmt assign varlet program if while dowhile block return for
%type <node> binop monop
%type <node> ids arrayexprs arrayexpr
%type <ctype> type

%right LET
%left OR
%left AND
%left EQ NE
%left LE LT GE GT
%left MINUS PLUS 
%left STAR SLASH PERCENT
%left NOT NEG

// UNSURE ABOUT BELOW
%left BRACKET_R PAR_R 
%left ELSE
%right BRACKET_L PAR_L

%start program

%%

program: declarations
         {
           parseresult = TBmakeProgram($1);
         }
         ;

declarations: declaration declarations
        {
          $$ = TBmakeDeclarations($1, $2);
        }
        | declaration
        {
          $$ = TBmakeDeclarations($1, NULL);
        }
        ;

declaration: globaldef
        {
          $$ = $1;
        }
        | globaldec
        {
          $$ = $1;
        }
        | function
        {
          $$ = $1;
        }
        ;

globaldec: EXTERN type ID SEMICOLON
        {
          $$ = TBmakeGlobaldec($2, $3, NULL);
        }
        | EXTERN type SQBR_L ids SQBR_R ID SEMICOLON
        {
          $$ = TBmakeGlobaldec($2, $6, $4);
        }
        ;

globaldef: type ID SEMICOLON
        {
          $$ = TBmakeGlobaldef($2, $1, NULL, NULL);
        }
        | type ID LET expr SEMICOLON
        {
          $$ = TBmakeGlobaldef($2, $1, NULL, $4);
        }
        | type SQBR_L exprs SQBR_R ID SEMICOLON
        {
          $$ = TBmakeGlobaldef($5, $1, $3, NULL);
        }
         | EXPORT type SQBR_L exprs SQBR_R ID SEMICOLON
        {
          $$ = TBmakeGlobaldef($6, $2, $4, NULL);
        }
        | type SQBR_L exprs SQBR_R ID LET expr SEMICOLON
        {
          $$ = TBmakeGlobaldef($5, $1, $3, $7);
        }
        | type SQBR_L exprs SQBR_R ID LET SQBR_L arrayexprs SQBR_R SEMICOLON
        {
          $$ = TBmakeGlobaldef($5, $1, $3, $8);
        }
        | EXPORT type ID SEMICOLON
        {
          $$ = TBmakeGlobaldef($3, $2, NULL, NULL);
          GLOBALDEF_ISEXPORT($$) = TRUE;
        }
        | EXPORT type ID LET expr SEMICOLON
        {
          $$ = TBmakeGlobaldef($3, $2, NULL, $5);
          GLOBALDEF_ISEXPORT($$) = TRUE;
        }
        | EXPORT type SQBR_L exprs SQBR_R ID LET expr SEMICOLON
        {
          $$ = TBmakeGlobaldef($6, $2, $4, $8);
          GLOBALDEF_ISEXPORT($$) = TRUE;
        }
        | EXPORT type SQBR_L exprs SQBR_R ID LET SQBR_L arrayexprs SQBR_R SEMICOLON
        {
          $$ = TBmakeGlobaldef($6, $2, $4, $9);
          GLOBALDEF_ISEXPORT($$) = TRUE;
        }
        ;

function: type ID BRACKET_L params BRACKET_R PAR_L funbody PAR_R
        {
          $$ = TBmakeFunction($1, $2, $4, $7);
        }
        |
        type ID BRACKET_L BRACKET_R PAR_L funbody PAR_R
        {
            $$ = TBmakeFunction($1, $2, NULL, $6);
        }
        | EXPORT type ID BRACKET_L params BRACKET_R PAR_L funbody PAR_R
        {
          $$ = TBmakeFunction($2, $3, $5, $8);
          FUNCTION_ISEXPORT($$) = TRUE;
        }
        | EXPORT type ID BRACKET_L BRACKET_R PAR_L funbody PAR_R
        {
            $$ = TBmakeFunction($2, $3, NULL, $7);
            FUNCTION_ISEXPORT($$) = TRUE;
        }
        | EXTERN type ID BRACKET_L BRACKET_R SEMICOLON
        {
          $$ = TBmakeFunction($2, $3, NULL, NULL);
          FUNCTION_ISEXTERN($$) = TRUE;
        }
        | EXTERN type ID BRACKET_L params BRACKET_R SEMICOLON
        {
          $$ = TBmakeFunction($2, $3, $5, NULL);
          FUNCTION_ISEXTERN($$) = TRUE;
        }
        ;

params: param COMMA params
        {
          PARAMETERS_NEXT($1) = $3;
        } 
        | param
        {
          $$ = $1;
        } 
        ;

param: type ID
        {
          $$ = TBmakeParameters($2, $1, NULL, NULL);
        }
        | type SQBR_L ids SQBR_R ID
        {
          $$ = TBmakeParameters($5, $1, NULL, $3);
        }
        ;

ids: ID COMMA ids
    {
      $$ = TBmakeIds( STRcpy( $1), $3);
    } 
    | ID
    {
      $$ = TBmakeIds( STRcpy( $1), NULL);
    }  
    ;

funbody: vardecs localfunctions stmts
        {
          $1 = reverselist($1);
          $$ = TBmakeFunctionbody($1, $2, $3);
        }
        | vardecs localfunctions 
        {
          $1 = reverselist($1);
          $$ = TBmakeFunctionbody($1, $2, NULL);
        }
        | vardecs stmts
        {
          $1 = reverselist($1);
          $$ = TBmakeFunctionbody($1, NULL, $2);
        }
        | localfunctions stmts
        {
          $$ = TBmakeFunctionbody(NULL, $1, $2);
        }
        | vardecs
        {
          $1 = reverselist($1);
          $$ = TBmakeFunctionbody($1, NULL, NULL);
        }
        | localfunctions
        {
          $$ = TBmakeFunctionbody(NULL, $1, NULL);
        }
        | stmts
        { 
          $$ = TBmakeFunctionbody(NULL, NULL, $1);
        }
        |
        {
          $$ = TBmakeFunctionbody(NULL, NULL, NULL);
        }
        ;

vardecs: vardecs vardec 
          {
            VARDECLARATION_NEXT($2) = $1;
            $$ = $2;
          }
          | vardec
          {
            $$ = $1;
          }
          ;      

vardec: type ID LET expr SEMICOLON
        {
          $$ = TBmakeVardeclaration($2, $1, NULL, $4, NULL);
        }
        | type ID SEMICOLON
        {
          $$ = TBmakeVardeclaration($2, $1, NULL, NULL, NULL);
        }
        | type SQBR_L exprs SQBR_R ID SEMICOLON
        {
          $$ = TBmakeVardeclaration($5, $1, $3, NULL, NULL);
        }
        | type SQBR_L exprs SQBR_R ID LET SQBR_L arrayexprs SQBR_R SEMICOLON
        {
          $$ = TBmakeVardeclaration($5, $1, $3, $8, NULL);
        }
        | type SQBR_L exprs SQBR_R ID LET expr SEMICOLON
        {
          $$ = TBmakeVardeclaration($5, $1, $3, $7, NULL);
        }
        ;

localfunctions: localfunction localfunctions
          {
            LOCALFUNCTION_NEXT($1) = $2;
          }
          | localfunction
          {
            $$ = $1;
          }
          ;   

localfunction: function
        {
          $$ = TBmakeLocalfunction($1, NULL);
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
          $$ = TBmakeAssign( $3, $1);
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
        | ID SQBR_L exprs SQBR_R
        {
          $$ = TBmakeVarlet( STRcpy ($1), NULL, $3);
        }
        ;

arrayexprs: arrayexpr COMMA arrayexprs
        {
          ARRAYEXPR_NEXT($1) = $3;
        }
        | arrayexpr
        {
          $$ = $1;
        }
        ;

arrayexpr: SQBR_L arrayexpr SQBR_R 
        {
          $$ = $2;
        }
        | expr
        {
          $$ = TBmakeArrayexpr($1, NULL);
        }
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
    | ID SQBR_L exprs SQBR_R
    {
      $$ = TBmakeVar( STRcpy( $1), $3);
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

monop: MINUS expr          { $$ = TBmakeMonop(MO_neg, $2); }     
      | NOT expr       { $$ = TBmakeMonop(MO_not, $2); }     
      ;

type: INTTYPE              { $$ = T_int; }     
      | FLOATTYPE          { $$ = T_float; } 
      | BOOLTYPE           { $$ = T_bool; } 
      | VOIDTYPE           { $$ = T_void; } 
      ;
%%

static int yyerror( char *error)
{
  CTIabort( "line %d, col %d\nError parsing source code: %s\n", 
            global.line, global.col, error);

  return( 0);
}

node *reverselist(node *vardecs) {
  node* prev = NULL;
  node* current = vardecs;
  node* next;

  while (current != NULL)
  {
      next  = VARDECLARATION_NEXT(current);  
      VARDECLARATION_NEXT(current) = prev;   
      prev = current;
      current = next;
  }
  return prev;
}

node *YYparseTree( void)
{
  DBUG_ENTER("YYparseTree");

  yyparse();

  DBUG_RETURN( parseresult);
}

