/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.
   Copyright (c) 2026 ENSTA, France
   Modified by Jordan NININ (GAOL v5)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_GAOL_Y_TAB_H_INCLUDED
# define YY_GAOL_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gaol_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    EMPTY_STR = 258,               /* EMPTY_STR  */
    ENTIRE_STR = 259,              /* ENTIRE_STR  */
    INFINITY_STR = 260,            /* INFINITY_STR  */
    DMIN_STR = 261,                /* DMIN_STR  */
    DMAX_STR = 262,                /* DMAX_STR  */
    PI_STR = 263,                  /* PI_STR  */
    FUNCTION_NAME = 264,           /* FUNCTION_NAME  */
    UNEXPECTED_CHAR = 265,         /* UNEXPECTED_CHAR  */
    NUMBER = 266,                  /* NUMBER  */
    INTERVAL_CST = 267,            /* INTERVAL_CST  */
    UNCERTAIN_CST = 268,           /* UNCERTAIN_CST  */
    UMINUS = 269,                  /* UMINUS  */
    UPLUS = 270                    /* UPLUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define EMPTY_STR 258
#define ENTIRE_STR 259
#define INFINITY_STR 260
#define DMIN_STR 261
#define DMAX_STR 262
#define PI_STR 263
#define FUNCTION_NAME 264
#define UNEXPECTED_CHAR 265
#define NUMBER 266
#define INTERVAL_CST 267
#define UNCERTAIN_CST 268
#define UMINUS 269
#define UPLUS 270

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 717 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;
  const void* fn;

#line 105 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int gaol_parse (void *scanner, void *context);


#endif /* !YY_GAOL_Y_TAB_H_INCLUDED  */
