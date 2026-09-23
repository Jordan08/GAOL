/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         gaol_parse
#define yylex           gaol_lex
#define yyerror         gaol_error
#define yydebug         gaol_debug
#define yynerrs         gaol_nerrs
#define yylval          gaol_lval
#define yychar          gaol_char

/* First part of user prologue.  */
#line 1 "gaol_interval_parser.ypp"

/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * CVS: $Id: gaol_interval_parser.ypp 191 2012-03-06 17:08:58Z goualard $
 * Last modified:
 * By:
 *--------------------------------------------------------------------------*/

#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <exception>
#include <string>
#include "gaol/gaol_common.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_expression.h"
#include "gaol/gaol_expr_eval.h"
#include "gaol/gaol_limits.h"
#include "gaol/gaol_exceptions.h"
#include "gaol/gaol_ieee1788.h"
#include "gaol/gaol_parser.h"
#include <cctype>

#undef yyerror
#define yyerror gaol_error_bison

int gaol_error_bison(const char *s ...);
int gaol_lex(void);

using namespace gaol;
extern interval *gaol_result_of_parsing;
extern bool gaol_global_parsing_flag;

/*
  One grammar, whose every string is an expression (GAOL v5)

  GAOL read a string with two grammars: the numbers, the constants and the
  functions of them built the tree of gaol/gaol_expression.h, while the
  intervals written between brackets, and the operations on them, were
  computed at once. Bison chose between the two at each token (124
  shift/reduce and 14 reduce/reduce conflicts): once a number had started the
  tree, no interval could come, and 1+[1,2], 2*cos([0,1]) and [1,2]+1+[1,2]
  were refused while [1,2]*2 was read, as was an interval in a bound,
  [cos([0,1]), 2]. Each literal set the flag of success, which the next one
  set again: [nth_root(8,1.5)]+[1,2] gave [-oo,+oo] rather than an error.

  The literals of intervals, the uncertain numbers, empty and <a,b> are now
  leaves of the tree, computed when they are read, and the operators and
  functions are nodes over any expression. The grammar has no conflict, and an
  error found in an action aborts the parsing at once.
*/

/*
  The parser holds one reference to each node it builds (gaol_node()), which
  it gives up when the node goes into another node, which holds its own, or
  when its value has been computed (GAOL v5): GAOL kept it, and the nodes of
  every expression read were never freed.
*/
static expr_node *gaol_node(expr_node *e)
{
  e->inc_refcount();
  return e;
}

static void gaol_release(expr_node *e)
{
  if (e->dec_refcount() == 0) {
    delete e;
  }
}

/*
  The value of the tree e: a literal, a bound, the exponent of nth_root(). The
  parser builds no null node, the only one whose evaluation fails.
*/
static interval gaol_value(expr_node *e)
{
  expr_eval eval;
  e->accept(eval);
  return eval.result();
}

/*
  The exception an action raises (GAOL v5): the action keeps it and aborts,
  the parser frees the nodes it holds, and parse_interval() throws it again.
  Thrown through the parser, it left them.
*/
std::exception_ptr gaol_parsing_exception;

#define GAOL_PARSING_ERROR(excep,msg)                                   \
  do {                                                                  \
    try {                                                               \
      gaol_ERROR(excep,msg);                                            \
    } catch (...) {                                                     \
      gaol_parsing_exception = std::current_exception();                \
    }                                                                   \
  } while (0)

/*
  1 if e is the infinity written inf, -1 if it is -inf, 0 otherwise (GAOL
  v5). An infinity read in an expression is [dmax, +oo] or [-oo, -dmax] (see
  gaol_expr_eval.h), but a lower bound written inf, or an upper bound written
  -inf, leaves no interval, as numsToInterval of IEEE 1788-2015 (10.5.8): [inf],
  [inf, inf] and [inf,] are the empty set.
*/
static int gaol_literal_infinity(expr_node *e)
{
  if (double_node *d = dynamic_cast<double_node*>(e)) {
    return (d->get_val() == GAOL_INFINITY) ? 1 : ((d->get_val() == -GAOL_INFINITY) ? -1 : 0);
  }
  if (unary_minus_node *m = dynamic_cast<unary_minus_node*>(e)) {
    return -gaol_literal_infinity(m->get_subexpr());
  }
  return 0;
}

// The leaf holding the interval of a literal
static expr_node *gaol_leaf(const interval& I)
{
  return gaol_node(new interval_node(I));
}

/*
  The leaf of an uncertain number, 3.56?1 (GAOL v5). IEEE 1788-2015 makes the
  uncertain form an interval literal of its own, not a number that can bound
  one: [5?1] is no interval literal (12.11.4). Its leaf is told apart from
  the other intervals so that the literals between brackets refuse it.
*/
class uncertain_node : public interval_node {
public:
  explicit uncertain_node(const interval& I) : interval_node(I) {}
};

// e is an uncertain number, alone or behind a minus sign
static bool gaol_uncertain(expr_node *e)
{
  if (unary_minus_node *m = dynamic_cast<unary_minus_node*>(e)) {
    return gaol_uncertain(m->get_subexpr());
  }
  return dynamic_cast<uncertain_node*>(e) != 0;
}

// e is an uncertain number written as a bound: the error the parsing aborts with
static bool gaol_uncertain_bound(expr_node *e)
{
  if (!gaol_uncertain(e)) {
    return false;
  }
  GAOL_PARSING_ERROR(input_format_error,
    "an uncertain number is no bound of an interval literal (IEEE 1788-2015, 12.11)");
  return true;
}

/*
  The functions a string calls, by their names (GAOL v5)

  A string is read with the names of GAOL, by interval(const char*),
  gaol::textToInterval() and operator>>, or with the names of IEEE 1788-2015,
  by gaol_ieee1788::textToInterval(), as a program opens the namespace gaol or
  gaol_ieee1788: parse_interval() sets gaol_parsing_names. The lexer looks each
  name up in the table of those names, whatever the case of its letters, and
  a name that is not in it is an error: nth_root(8,3) is read with the names
  of GAOL, rootn(8,3) with those of the standard, and pow(x,y) is the pow of
  the namespace of the names, gaol_pow_hybrid() or the pow of Table 9.1.

  Each name gives a function of intervals of one, two or three arguments, or
  of an interval and an int, whose second argument has to be an integer. A
  call is computed when it is read, its arguments being computed then too,
  and gives a leaf of the tree, as a literal does: GAOL built a node of
  gaol/gaol_expression.h for each of its functions, and read no other.
*/
::gaol::parsing_names gaol_parsing_names = ::gaol::parsing_names::gaol;

struct gaol_function {
  const char *name;
  unsigned int arity;
  interval (*f1)(const interval&);
  interval (*f2)(const interval&, const interval&);
  interval (*f3)(const interval&, const interval&, const interval&);
  interval (*fn)(const interval&, int);
};

typedef interval (*gaol_f1)(const interval&);
typedef interval (*gaol_f2)(const interval&, const interval&);
typedef interval (*gaol_fn)(const interval&, int);
#define GAOL_F1(name, f) { name, 1, static_cast<gaol_f1>(f), 0, 0, 0 }
#define GAOL_F2(name, f) { name, 2, 0, static_cast<gaol_f2>(f), 0, 0 }
#define GAOL_FN(name, f) { name, 2, 0, 0, 0, static_cast<gaol_fn>(f) }

// cbrt(x) is nth_root(x,3), as sqrt(x) is nth_root(x,2)
static interval gaol_cbrt(const interval& x)
{
  return ::gaol_core::nth_root(x, 3);
}

/* The names of GAOL: its functions of intervals, the functions of Tables 9.1
   and 10.5 of IEEE 1788-2015 under GAOL's names, and cbrt, integer and
   inverse. pow is gaol::pow, gaol_pow_hybrid(), which takes gaol_pown() for an
   exponent that is an integer: the parser took 1/x^|n| for n < 0, whose x^|n|
   overflowed, and 1 for n = 0, which is no value for an empty x. nth_root(x,
   q) takes an int q, which may be negative: rootn(x, q) of IEEE 1788-2015 */
static const gaol_function gaol_functions[] = {
  GAOL_F1("sqr", ::gaol_core::sqr), GAOL_F1("sqrt", ::gaol_core::sqrt), GAOL_F1("cbrt", gaol_cbrt),
  GAOL_F2("pow", ::gaol_core::gaol_pow_hybrid), GAOL_FN("nth_root", ::gaol_core::nth_root),
  GAOL_F1("exp", ::gaol_core::exp), GAOL_F1("exp2", ::gaol_core::exp2), GAOL_F1("exp10", ::gaol_core::exp10),
  GAOL_F1("expm1", ::gaol_core::expm1), GAOL_F1("exp2m1", ::gaol_core::exp2m1),
  GAOL_F1("exp10m1", ::gaol_core::exp10m1),
  GAOL_F1("log", ::gaol_core::log), GAOL_F1("log2", ::gaol_core::log2), GAOL_F1("log10", ::gaol_core::log10),
  GAOL_F1("log1p", ::gaol_core::log1p), GAOL_F1("log2p1", ::gaol_core::log2p1),
  GAOL_F1("log10p1", ::gaol_core::log10p1),
  GAOL_F1("sin", ::gaol_core::sin), GAOL_F1("cos", ::gaol_core::cos), GAOL_F1("tan", ::gaol_core::tan),
  GAOL_F1("asin", ::gaol_core::asin), GAOL_F1("acos", ::gaol_core::acos), GAOL_F1("atan", ::gaol_core::atan),
  GAOL_F2("atan2", ::gaol_core::atan2),
  GAOL_F1("sinh", ::gaol_core::sinh), GAOL_F1("cosh", ::gaol_core::cosh), GAOL_F1("tanh", ::gaol_core::tanh),
  GAOL_F1("asinh", ::gaol_core::asinh), GAOL_F1("acosh", ::gaol_core::acosh),
  GAOL_F1("atanh", ::gaol_core::atanh),
  GAOL_F1("sinpi", ::gaol_core::sinpi), GAOL_F1("cospi", ::gaol_core::cospi), GAOL_F1("tanpi", ::gaol_core::tanpi),
  GAOL_F1("asinpi", ::gaol_core::asinpi), GAOL_F1("acospi", ::gaol_core::acospi),
  GAOL_F1("atanpi", ::gaol_core::atanpi), GAOL_F2("atan2pi", ::gaol_core::atan2pi),
  GAOL_F2("hypot", ::gaol_core::hypot), GAOL_F1("rsqrt", ::gaol_core::rsqrt),
  GAOL_F1("sign", ::gaol_core::sign), GAOL_F1("ceil", ::gaol_core::ceil), GAOL_F1("floor", ::gaol_core::floor),
  GAOL_F1("trunc", ::gaol_core::trunc), GAOL_F1("integer", ::gaol_core::integer),
  GAOL_F1("round_ties_to_even", ::gaol_core::round_ties_to_even),
  GAOL_F1("round_ties_to_away", ::gaol_core::round_ties_to_away),
  GAOL_F1("abs", ::gaol_core::abs), GAOL_F2("min", ::gaol_core::min), GAOL_F2("max", ::gaol_core::max),
  GAOL_F1("inverse", ::gaol_core::inverse),
  { "fma", 3, 0, 0, ::gaol_core::fma, 0 },
};

/* The names of IEEE 1788-2015: the functions of Tables 9.1 and 10.5 that
   GAOL provides, as gaol_ieee1788 names them. pow is the pow of Table 9.1,
   pown(x, p) the power with an integer exponent, rootn(x, q) the root */
static const gaol_function ieee1788_functions[] = {
  GAOL_F1("neg", ::gaol_ieee1788::neg), GAOL_F2("add", ::gaol_ieee1788::add),
  GAOL_F2("sub", ::gaol_ieee1788::sub), GAOL_F2("mul", ::gaol_ieee1788::mul),
  GAOL_F2("div", ::gaol_ieee1788::div), GAOL_F1("recip", ::gaol_ieee1788::recip),
  GAOL_F1("sqr", ::gaol_core::sqr), GAOL_F1("sqrt", ::gaol_core::sqrt),
  { "fma", 3, 0, 0, ::gaol_core::fma, 0 },
  GAOL_FN("pown", ::gaol_ieee1788::pown), GAOL_F2("pow", ::gaol_ieee1788::pow),
  GAOL_F1("exp", ::gaol_core::exp), GAOL_F1("exp2", ::gaol_core::exp2), GAOL_F1("exp10", ::gaol_core::exp10),
  GAOL_F1("log", ::gaol_core::log), GAOL_F1("log2", ::gaol_core::log2), GAOL_F1("log10", ::gaol_core::log10),
  GAOL_F1("sin", ::gaol_core::sin), GAOL_F1("cos", ::gaol_core::cos), GAOL_F1("tan", ::gaol_core::tan),
  GAOL_F1("asin", ::gaol_core::asin), GAOL_F1("acos", ::gaol_core::acos), GAOL_F1("atan", ::gaol_core::atan),
  GAOL_F2("atan2", ::gaol_core::atan2),
  GAOL_F1("sinh", ::gaol_core::sinh), GAOL_F1("cosh", ::gaol_core::cosh), GAOL_F1("tanh", ::gaol_core::tanh),
  GAOL_F1("asinh", ::gaol_core::asinh), GAOL_F1("acosh", ::gaol_core::acosh),
  GAOL_F1("atanh", ::gaol_core::atanh),
  GAOL_F1("sign", ::gaol_core::sign), GAOL_F1("ceil", ::gaol_core::ceil), GAOL_F1("floor", ::gaol_core::floor),
  GAOL_F1("trunc", ::gaol_core::trunc),
  GAOL_F1("roundTiesToEven", ::gaol_ieee1788::roundTiesToEven),
  GAOL_F1("roundTiesToAway", ::gaol_ieee1788::roundTiesToAway),
  GAOL_F1("abs", ::gaol_core::abs), GAOL_F2("min", ::gaol_core::min), GAOL_F2("max", ::gaol_core::max),
  GAOL_FN("rootn", ::gaol_ieee1788::rootn),
  GAOL_F1("expm1", ::gaol_core::expm1), GAOL_F1("exp2m1", ::gaol_core::exp2m1),
  GAOL_F1("exp10m1", ::gaol_core::exp10m1),
  GAOL_F1("logp1", ::gaol_ieee1788::logp1), GAOL_F1("log2p1", ::gaol_core::log2p1),
  GAOL_F1("log10p1", ::gaol_core::log10p1),
  GAOL_F2("hypot", ::gaol_core::hypot), GAOL_F1("rSqrt", ::gaol_ieee1788::rSqrt),
  GAOL_F1("sinPi", ::gaol_ieee1788::sinPi), GAOL_F1("cosPi", ::gaol_ieee1788::cosPi),
  GAOL_F1("tanPi", ::gaol_ieee1788::tanPi), GAOL_F1("asinPi", ::gaol_ieee1788::asinPi),
  GAOL_F1("acosPi", ::gaol_ieee1788::acosPi), GAOL_F1("atanPi", ::gaol_ieee1788::atanPi),
  GAOL_F2("atan2Pi", ::gaol_ieee1788::atan2Pi),
};

// The function of a name, whatever the case of its letters, in the table of
// the names the string is read with; 0 for a name that is not in it
const void *gaol_lookup_function(const char *name)
{
  const bool ieee1788 = (gaol_parsing_names == ::gaol::parsing_names::ieee1788);
  const gaol_function *table = ieee1788 ? ieee1788_functions : gaol_functions;
  const std::size_t size = ieee1788 ? sizeof(ieee1788_functions)/sizeof(gaol_function)
                                    : sizeof(gaol_functions)/sizeof(gaol_function);
  for (std::size_t i = 0; i < size; ++i) {
    const char *a = table[i].name, *b = name;
    while (*a != '\0' && *b != '\0'
           && std::tolower(static_cast<unsigned char>(*a)) == std::tolower(static_cast<unsigned char>(*b))) {
      ++a;
      ++b;
    }
    if (*a == '\0' && *b == '\0') {
      return &table[i];
    }
  }
  return 0;
}

/*
  The call of the function fn on the n expressions given, which it releases:
  the leaf of its value, or 0 for an error, which the parsing aborts with.
  A wrong number of arguments is an input_format_error, and a second argument
  that is no integer, where it has to be one, an invalid_action_error.
*/
static expr_node *gaol_call(const void *fn, unsigned int n, expr_node *a, expr_node *b, expr_node *c)
{
  const gaol_function *f = static_cast<const gaol_function*>(fn);
  expr_node *result = 0;
  try {
    if (f->arity != n) {
      const std::string msg = std::string(f->name) + " called with a wrong number of arguments";
      GAOL_PARSING_ERROR(input_format_error, msg.c_str());
    } else if (f->fn != 0) {
      const interval q = gaol_value(b);
      if (!q.is_an_int()) {
        const std::string msg = std::string(f->name) + " used with non integral 2nd arg.";
        GAOL_PARSING_ERROR(invalid_action_error, msg.c_str());
      } else {
        result = gaol_leaf(f->fn(gaol_value(a), static_cast<int>(q.left())));
      }
    } else if (n == 1) {
      result = gaol_leaf(f->f1(gaol_value(a)));
    } else if (n == 2) {
      result = gaol_leaf(f->f2(gaol_value(a), gaol_value(b)));
    } else {
      result = gaol_leaf(f->f3(gaol_value(a), gaol_value(b), gaol_value(c)));
    }
  } catch (...) {
    gaol_parsing_exception = std::current_exception();
    result = 0;
  }
  gaol_release(a);
  if (b != 0) {
    gaol_release(b);
  }
  if (c != 0) {
    gaol_release(c);
  }
  return result;
}

#line 420 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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
    FUNCTION_NAME = 264,
    UNEXPECTED_CHAR = 265,
    NUMBER = 266,
    INTERVAL_CST = 267,
    UNCERTAIN_CST = 268,
    UMINUS = 269,
    UPLUS = 270
  };
#endif
/* Tokens.  */
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
#line 344 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;
  const void* fn;

#line 510 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE gaol_lval;

int gaol_parse (void);

#endif /* !YY_GAOL_Y_TAB_H_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   163

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  27
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  5
/* YYNRULES -- Number of rules.  */
#define YYNRULES  30
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  58

#define YYUNDEFTOK  2
#define YYMAXUTOK   270


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      20,    21,    16,    14,    24,    15,     2,    17,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      25,     2,    26,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    22,     2,    23,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    18,
      19
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   376,   376,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   393,   395,   397,   399,   401,   402,   403,
     412,   420,   430,   431,   432,   433,   442,   451,   466,   467,
     469
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EMPTY_STR", "ENTIRE_STR",
  "INFINITY_STR", "DMIN_STR", "DMAX_STR", "PI_STR", "FUNCTION_NAME",
  "UNEXPECTED_CHAR", "NUMBER", "INTERVAL_CST", "UNCERTAIN_CST", "'+'",
  "'-'", "'*'", "'/'", "UMINUS", "UPLUS", "'('", "')'", "'['", "']'",
  "','", "'<'", "'>'", "$accept", "itv_expr", "expression", "literal",
  "function_call", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    43,    45,    42,    47,   269,   270,
      40,    41,    91,    93,    44,    60,    62
};
# endif

#define YYPACT_NINF (-11)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     100,   -11,   -11,   -11,   -11,   -11,    -4,   -11,   -11,   -11,
     100,   100,   100,    35,   100,    10,    37,   -11,   -11,   100,
     -11,   -11,    -3,     8,   -11,    58,     9,   124,   -11,   100,
     100,   100,   100,   102,   -11,   -11,   -11,   128,   -11,    79,
     100,    11,    11,   -11,   -11,   -11,   100,   -11,   -11,   138,
      -9,   113,   -11,   -11,   -11,   100,   142,   -11
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    10,     6,     4,     5,     7,     0,     3,     8,     9,
       0,     0,     0,     0,     0,     0,     2,    11,    18,     0,
      17,    16,     0,     0,    22,     0,     0,     0,     1,     0,
       0,     0,     0,     0,    19,    23,    24,     0,    20,     0,
       0,    12,    13,    14,    15,    28,     0,    26,    25,     0,
       0,     0,    21,    27,    29,     0,     0,    30
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -11,   -11,   -10,   -11,   -11
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    15,    16,    17,    18
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      20,    21,    22,    26,    27,    29,    30,    31,    32,    33,
      28,    29,    30,    31,    32,    37,    19,    53,    34,    41,
      42,    43,    44,    29,    30,    31,    32,    31,    32,    49,
      50,    35,    38,    39,     0,     0,    51,     0,     1,    23,
       2,     3,     4,     5,     6,    56,     7,     8,     9,    10,
      11,    29,    30,    31,    32,    12,     0,    13,    24,    25,
      14,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,     0,    12,     0,
      13,    36,     1,    14,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,     0,    12,
       0,    13,    48,     1,    14,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,    29,    30,    31,    32,
      12,     0,    13,    45,     0,    14,    46,    29,    30,    31,
      32,     0,     0,     0,    54,     0,     0,    55,    29,    30,
      31,    32,    29,    30,    31,    32,     0,     0,    40,     0,
       0,    47,    29,    30,    31,    32,    29,    30,    31,    32,
       0,    52,     0,    57
};

static const yytype_int8 yycheck[] =
{
      10,    11,    12,    13,    14,    14,    15,    16,    17,    19,
       0,    14,    15,    16,    17,    25,    20,    26,    21,    29,
      30,    31,    32,    14,    15,    16,    17,    16,    17,    39,
      40,    23,    23,    24,    -1,    -1,    46,    -1,     3,     4,
       5,     6,     7,     8,     9,    55,    11,    12,    13,    14,
      15,    14,    15,    16,    17,    20,    -1,    22,    23,    24,
      25,     3,    -1,     5,     6,     7,     8,     9,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    -1,    20,    -1,
      22,    23,     3,    25,     5,     6,     7,     8,     9,    -1,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    20,
      -1,    22,    23,     3,    25,     5,     6,     7,     8,     9,
      -1,    11,    12,    13,    14,    15,    14,    15,    16,    17,
      20,    -1,    22,    21,    -1,    25,    24,    14,    15,    16,
      17,    -1,    -1,    -1,    21,    -1,    -1,    24,    14,    15,
      16,    17,    14,    15,    16,    17,    -1,    -1,    24,    -1,
      -1,    23,    14,    15,    16,    17,    14,    15,    16,    17,
      -1,    23,    -1,    21
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     6,     7,     8,     9,    11,    12,    13,
      14,    15,    20,    22,    25,    28,    29,    30,    31,    20,
      29,    29,    29,     4,    23,    24,    29,    29,     0,    14,
      15,    16,    17,    29,    21,    23,    23,    29,    23,    24,
      24,    29,    29,    29,    29,    21,    24,    23,    23,    29,
      29,    29,    23,    26,    21,    24,    29,    21
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    27,    28,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    29,    29,    29,    29,    29,    29,
      30,    30,    30,    30,    30,    30,    30,    30,    31,    31,
      31
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     2,     2,     1,     3,
       3,     5,     2,     3,     3,     4,     4,     5,     4,     6,
       8
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
    case 29: /* expression  */
#line 365 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1476 "y.tab.c"
        break;

    case 30: /* literal  */
#line 365 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1482 "y.tab.c"
        break;

    case 31: /* function_call  */
#line 365 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1488 "y.tab.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 376 "gaol_interval_parser.ypp"
                                                        { *gaol_result_of_parsing = gaol_value((yyvsp[0].expr));
							  gaol_release((yyvsp[0].expr));
							  gaol_global_parsing_flag = true; }
#line 1760 "y.tab.c"
    break;

  case 3:
#line 382 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node((yyvsp[0].d))); }
#line 1766 "y.tab.c"
    break;

  case 4:
#line 383 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(std::numeric_limits<double>::min())); }
#line 1772 "y.tab.c"
    break;

  case 5:
#line 384 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(std::numeric_limits<double>::max())); }
#line 1778 "y.tab.c"
    break;

  case 6:
#line 385 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(GAOL_INFINITY)); }
#line 1784 "y.tab.c"
    break;

  case 7:
#line 386 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::pi()); }
#line 1790 "y.tab.c"
    break;

  case 8:
#line 387 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval((yyvsp[0].itv).l,(yyvsp[0].itv).r)); }
#line 1796 "y.tab.c"
    break;

  case 9:
#line 388 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new uncertain_node(interval((yyvsp[0].itv).l,(yyvsp[0].itv).r))); }
#line 1802 "y.tab.c"
    break;

  case 10:
#line 389 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::emptyset()); }
#line 1808 "y.tab.c"
    break;

  case 11:
#line 390 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1814 "y.tab.c"
    break;

  case 12:
#line 391 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new add_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1821 "y.tab.c"
    break;

  case 13:
#line 393 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new sub_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1828 "y.tab.c"
    break;

  case 14:
#line 395 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new mult_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1835 "y.tab.c"
    break;

  case 15:
#line 397 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new div_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1842 "y.tab.c"
    break;

  case 16:
#line 399 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new unary_minus_node(*(yyvsp[0].expr)));
							  gaol_release((yyvsp[0].expr)); }
#line 1849 "y.tab.c"
    break;

  case 17:
#line 401 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1855 "y.tab.c"
    break;

  case 18:
#line 402 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1861 "y.tab.c"
    break;

  case 19:
#line 403 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 1867 "y.tab.c"
    break;

  case 20:
#line 412 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval v = gaol_value((yyvsp[-1].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-1].expr)) != 0);
							  gaol_release((yyvsp[-1].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset() : v); }
#line 1880 "y.tab.c"
    break;

  case 21:
#line 420 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-3].expr)) || gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval l = gaol_value((yyvsp[-3].expr)), r = gaol_value((yyvsp[-1].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-3].expr)) > 0
										 || gaol_literal_infinity((yyvsp[-1].expr)) < 0);
							  gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset()
									 : interval(l.left(),r.right())); }
#line 1895 "y.tab.c"
    break;

  case 22:
#line 430 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::emptyset()); }
#line 1901 "y.tab.c"
    break;

  case 23:
#line 431 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::universe()); }
#line 1907 "y.tab.c"
    break;

  case 24:
#line 432 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::universe()); }
#line 1913 "y.tab.c"
    break;

  case 25:
#line 433 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-2].expr))) {
							    gaol_release((yyvsp[-2].expr));
							    YYABORT;
							  }
							  const interval l = gaol_value((yyvsp[-2].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-2].expr)) > 0);
							  gaol_release((yyvsp[-2].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset()
									 : interval(l.left(),GAOL_INFINITY)); }
#line 1927 "y.tab.c"
    break;

  case 26:
#line 442 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval r = gaol_value((yyvsp[-1].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-1].expr)) < 0);
							  gaol_release((yyvsp[-1].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset()
									 : interval(-GAOL_INFINITY,r.right())); }
#line 1941 "y.tab.c"
    break;

  case 27:
#line 451 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-3].expr)) || gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval l = gaol_value((yyvsp[-3].expr)), r = gaol_value((yyvsp[-1].expr));
							  gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr));
							  if (l.left() != r.right()) {
							    GAOL_PARSING_ERROR(input_format_error,
							      "bounds of degenerate interval do not evaluate to the same value");
							    YYABORT;
							  }
							  (yyval.expr) = gaol_leaf(interval(l.left(),r.right())); }
#line 1958 "y.tab.c"
    break;

  case 28:
#line 466 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call((yyvsp[-3].fn),1,(yyvsp[-1].expr),0,0)) == 0) YYABORT; }
#line 1964 "y.tab.c"
    break;

  case 29:
#line 468 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call((yyvsp[-5].fn),2,(yyvsp[-3].expr),(yyvsp[-1].expr),0)) == 0) YYABORT; }
#line 1970 "y.tab.c"
    break;

  case 30:
#line 470 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call((yyvsp[-7].fn),3,(yyvsp[-5].expr),(yyvsp[-3].expr),(yyvsp[-1].expr))) == 0) YYABORT; }
#line 1976 "y.tab.c"
    break;


#line 1980 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 473 "gaol_interval_parser.ypp"


int gaol_error_bison(const char *s ...)
{
  gaol_global_parsing_flag = false;
  return 0; // An error occurred
}
