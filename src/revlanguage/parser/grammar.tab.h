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
     REAL = 258,
     INT = 259,
     NAME = 260,
     STRING = 261,
     RBNULL = 262,
     FALSE = 263,
     TRUE = 264,
     FUNCTION = 265,
     PROCEDURE = 266,
     CLASS = 267,
     FOR = 268,
     IN = 269,
     IF = 270,
     ELSE = 271,
     WHILE = 272,
     NEXT = 273,
     BREAK = 274,
     RETURN = 275,
     MOD_CONST = 276,
     MOD_DYNAMIC = 277,
     MOD_STOCHASTIC = 278,
     MOD_DETERMINISTIC = 279,
     PROTECTED = 280,
     ARROW_ASSIGN = 281,
     TILDE_ASSIGN = 282,
     EQUATION_ASSIGN = 283,
     CONTROL_ASSIGN = 284,
     REFERENCE_ASSIGN = 285,
     ADDITION_ASSIGN = 286,
     SUBTRACTION_ASSIGN = 287,
     MULTIPLICATION_ASSIGN = 288,
     DIVISION_ASSIGN = 289,
     DECREMENT = 290,
     INCREMENT = 291,
     EQUAL = 292,
     AND = 293,
     OR = 294,
     AND2 = 295,
     OR2 = 296,
     GT = 297,
     GE = 298,
     LT = 299,
     LE = 300,
     EQ = 301,
     NE = 302,
     END_OF_INPUT = 303,
     UNOT = 304,
     UPLUS = 305,
     UMINUS = 306,
     UAND = 307
   };
#endif
/* Tokens.  */
#define REAL 258
#define INT 259
#define NAME 260
#define STRING 261
#define RBNULL 262
#define FALSE 263
#define TRUE 264
#define FUNCTION 265
#define PROCEDURE 266
#define CLASS 267
#define FOR 268
#define IN 269
#define IF 270
#define ELSE 271
#define WHILE 272
#define NEXT 273
#define BREAK 274
#define RETURN 275
#define MOD_CONST 276
#define MOD_DYNAMIC 277
#define MOD_STOCHASTIC 278
#define MOD_DETERMINISTIC 279
#define PROTECTED 280
#define ARROW_ASSIGN 281
#define TILDE_ASSIGN 282
#define EQUATION_ASSIGN 283
#define CONTROL_ASSIGN 284
#define REFERENCE_ASSIGN 285
#define ADDITION_ASSIGN 286
#define SUBTRACTION_ASSIGN 287
#define MULTIPLICATION_ASSIGN 288
#define DIVISION_ASSIGN 289
#define DECREMENT 290
#define INCREMENT 291
#define EQUAL 292
#define AND 293
#define OR 294
#define AND2 295
#define OR2 296
#define GT 297
#define GE 298
#define LT 299
#define LE 300
#define EQ 301
#define NE 302
#define END_OF_INPUT 303
#define UNOT 304
#define UPLUS 305
#define UMINUS 306
#define UAND 307




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 79 "./grammar.y"
{
    char*                                           c_string;
    std::string*                                    string;
    double                                          realValue;
    int                                             intValue;
    bool                                            boolValue;
    RevLanguage::SyntaxElement*                     syntaxElement;
    RevLanguage::SyntaxVariable*                    syntaxVariable;
    RevLanguage::SyntaxFunctionCall*                syntaxFunctionCall;
    RevLanguage::SyntaxLabeledExpr*                 syntaxLabeledExpr;
    RevLanguage::SyntaxFormal*                      syntaxFormal;
    std::list<RevLanguage::SyntaxElement*>*         syntaxElementList;
    std::list<RevLanguage::SyntaxLabeledExpr*>*     argumentList;
    std::list<RevLanguage::SyntaxFormal*>*          formalList;
}
/* Line 1529 of yacc.c.  */
#line 169 "./grammar.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;
