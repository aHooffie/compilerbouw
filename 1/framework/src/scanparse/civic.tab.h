/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     BRACKET_L = 258,
     BRACKET_R = 259,
     COMMA = 260,
     SEMICOLON = 261,
     MINUS = 262,
     PLUS = 263,
     STAR = 264,
     SLASH = 265,
     PERCENT = 266,
     LE = 267,
     LT = 268,
     GE = 269,
     GT = 270,
     EQ = 271,
     NE = 272,
     OR = 273,
     AND = 274,
     TRUEVAL = 275,
     FALSEVAL = 276,
     LET = 277,
     NUM = 278,
     FLOAT = 279,
     ID = 280
   };
#endif
/* Tokens.  */
#define BRACKET_L 258
#define BRACKET_R 259
#define COMMA 260
#define SEMICOLON 261
#define MINUS 262
#define PLUS 263
#define STAR 264
#define SLASH 265
#define PERCENT 266
#define LE 267
#define LT 268
#define GE 269
#define GT 270
#define EQ 271
#define NE 272
#define OR 273
#define AND 274
#define TRUEVAL 275
#define FALSEVAL 276
#define LET 277
#define NUM 278
#define FLOAT 279
#define ID 280




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 23 "src/scanparse/civic.y"
{
 nodetype            nodetype;
 char               *id;
 int                 cint;
 float               cflt;
 binop               cbinop;
 node               *node;
}
/* Line 1529 of yacc.c.  */
#line 108 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

