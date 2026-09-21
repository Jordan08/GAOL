/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_GAOL_Y_TAB_H_INCLUDED
# define YY_GAOL_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gaol_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EMPTY_STR = 258,
    ENTIRE_STR = 259,
    INFINITY_STR = 260,
    DMIN_STR = 261,
    DMAX_STR = 262,
    PI_STR = 263,
    EXP_STR = 264,
    LOG_STR = 265,
    POW_STR = 266,
    NTH_ROOT_STR = 267,
    SQRT_STR = 268,
    EXP2_STR = 269,
    LOG2_STR = 270,
    CBRT_STR = 271,
    SIGN_STR = 272,
    TRUNC_STR = 273,
    COS_STR = 274,
    SIN_STR = 275,
    TAN_STR = 276,
    ATAN2_STR = 277,
    COSH_STR = 278,
    SINH_STR = 279,
    TANH_STR = 280,
    ACOS_STR = 281,
    ASIN_STR = 282,
    ATAN_STR = 283,
    ACOSH_STR = 284,
    ASINH_STR = 285,
    ATANH_STR = 286,
    UNEXPECTED_CHAR = 287,
    NUMBER = 288,
    INTERVAL_CST = 289,
    UNCERTAIN_CST = 290,
    UMINUS = 291,
    UPLUS = 292
  };
#endif
/* Tokens.  */
#define EMPTY_STR 258
#define ENTIRE_STR 259
#define INFINITY_STR 260
#define DMIN_STR 261
#define DMAX_STR 262
#define PI_STR 263
#define EXP_STR 264
#define LOG_STR 265
#define POW_STR 266
#define NTH_ROOT_STR 267
#define SQRT_STR 268
#define EXP2_STR 269
#define LOG2_STR 270
#define CBRT_STR 271
#define SIGN_STR 272
#define TRUNC_STR 273
#define COS_STR 274
#define SIN_STR 275
#define TAN_STR 276
#define ATAN2_STR 277
#define COSH_STR 278
#define SINH_STR 279
#define TANH_STR 280
#define ACOS_STR 281
#define ASIN_STR 282
#define ATAN_STR 283
#define ACOSH_STR 284
#define ASINH_STR 285
#define ATANH_STR 286
#define UNEXPECTED_CHAR 287
#define NUMBER 288
#define INTERVAL_CST 289
#define UNCERTAIN_CST 290
#define UMINUS 291
#define UPLUS 292

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 99 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;

#line 138 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE gaol_lval;

int gaol_parse (void);

#endif /* !YY_GAOL_Y_TAB_H_INCLUDED  */
