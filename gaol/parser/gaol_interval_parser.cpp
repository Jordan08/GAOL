/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

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
 * Copyright (c) 2026 ENSTA, France
 * Modified by Jordan NININ (GAOL v5)
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <string>
#include "gaol/gaol_common.h"
#include "gaol/gaol_interval.h"
#include "gaol/parser/gaol_expression.h"
#include "gaol/parser/gaol_expr_eval.h"
#include "gaol/gaol_exceptions.h"
#include "gaol/gaol_ieee1788.h"
#include "gaol/parser/gaol_parser.h"
#include "gaol/gaol_port.h"

/*
  The depth of the stack of the parser: the 10000 entries that bison gives it
  when the grammar does not say (GAOL v5). An open parenthesis or bracket, and
  a sign, keep an entry until the expression they open is reduced, the name
  and the parenthesis of a call two, and an operand and the operator or the
  comma after it two, until the operand that follows them is read: a string
  nested deeper is refused, as a syntax error (input_format_error).
  The stack is in the heap beyond its first 200 entries (see
  gaol_parser_alloc()).
  The reading of a string uses no more of the stack of the program, whatever
  its length or its nesting: an operation is computed when it is read
  (gaol_operation()), and the signs, the only tree the parser holds that is
  more than a node deep, are evaluated and deleted without recursion
  (gaol/parser/gaol_expr_eval.h, gaol/parser/gaol_expression.cpp).
*/
#define YYMAXDEPTH 10000

#undef yyerror
#define yyerror gaol_error_bison

void gaol_error_bison(void *scanner, void *context, const char *msg);

using namespace gaol_core;

/*
  The state of a reading of a string (GAOL v5), which the parser, a pure one,
  and the lexer, a reentrant one, are given rather than globals: each reading
  has its own, and several threads read strings at once. GAOL kept the
  interval read and the success of the reading in globals, as flex and bison
  did their state: two threads reading a string each crashed.
*/
union gaol_flex_block;

struct gaol_parsing_context {
  interval result;                // the interval read, if ok is true
  bool ok;                        // the string is an expression, which was read
  std::exception_ptr exception;   // the exception an action or the lexer raised
  ::gaol_core::parsing_names names;    // the names of the functions
  const char *input;              // the characters the lexer has not read yet
  std::size_t input_left;         // their number, NUL characters included
  bool out_of_memory;             // an allocation of the stack of the parser failed
  gaol_flex_block *flex_blocks;   // the blocks flex allocated and has not freed
};

static gaol_parsing_context *gaol_context(void *context)
{
  return static_cast<gaol_parsing_context*>(context);
}

/*
  The characters of the string for the lexer, max_size at most at a time,
  which it asks for as it reads (YY_INPUT, gaol/parser/gaol_interval_lexer.lpp): the
  number given, 0 at the end of the string (GAOL v5). GAOL gave the lexer the
  string with yy_scan_string(), which takes its length from strlen() and holds
  it in an int: a std::string holding a NUL character was read up to it, so
  that "1\0junk" was 1, and a string of 2^32 + 3 characters was read as its
  first 3, "1+1" of "1+1+...", the value of another expression. The lexer
  now reads the n characters given, a NUL being no character of any token,
  whatever their number, and holds no copy of the string; a token holds 2^29
  characters at most (gaol_flex_alloc()).
*/
int gaol_read_input(void *context, char *buf, int max_size)
{
  gaol_parsing_context *c = gaol_context(context);
  const std::size_t n = std::min(c->input_left, static_cast<std::size_t>(max_size));
  std::memcpy(buf, c->input, n);
  c->input += n;
  c->input_left -= n;
  return static_cast<int>(n);
}

/*
  The memory of the lexer (GAOL v5)

  flex allocates the state of a scanner, the buffer of the characters read,
  16 KiB doubled for a token longer, and the stack of its buffers. Where an
  allocation failed, it wrote "out of dynamic memory" and ended the program
  with exit(2). It throws std::bad_alloc now (YY_FATAL_ERROR,
  gaol/parser/gaol_interval_lexer.lpp), but leaves a block it holds in no variable:
  the state of a buffer whose characters it could not allocate, or the
  characters that a failed realloc() does not free while flex overwrites
  their address. Each block it allocates for a reading is therefore linked to
  the context of the reading, and those flex has not freed when the reading
  ends are freed with it. The blocks come from the nothrow operator new, as
  the other memory of GAOL, so that tests/expressions.cpp makes their
  allocation fail too.
*/
struct gaol_flex_links {
  gaol_flex_block *previous;
  gaol_flex_block *next;
  std::size_t size;               // the bytes flex asked for
};

// The head of a block, before the bytes flex asked for, aligned as malloc() aligns
union gaol_flex_block {
  gaol_flex_links links;
  std::max_align_t alignment;
};

/*
  size bytes for flex, or 0 if they could not be allocated. flex holds the
  size of its buffer in an int, which it doubles for a token longer than the
  buffer with a multiplication that overflows from 2^30 on: no block is larger
  than INT_MAX / 2 bytes, so that the buffer stops at 2^29 characters and a
  token longer than that throws std::bad_alloc (GAOL v5)
*/
void *gaol_flex_alloc(void *context, std::size_t size)
{
  if (size > static_cast<std::size_t>(std::numeric_limits<int>::max() / 2)) {
    return 0;
  }
  gaol_flex_block *const b =
    static_cast<gaol_flex_block*>(::operator new(sizeof(gaol_flex_block) + size, std::nothrow));
  if (b == 0) {
    return 0;
  }
  gaol_parsing_context *c = gaol_context(context);
  b->links.previous = 0;
  b->links.next = c->flex_blocks;
  b->links.size = size;
  if (c->flex_blocks != 0) {
    c->flex_blocks->links.previous = b;
  }
  c->flex_blocks = b;
  return b + 1;
}

void gaol_flex_free(void *context, void *p)
{
  if (p == 0) {
    return;
  }
  gaol_flex_block *const b = static_cast<gaol_flex_block*>(p) - 1;
  if (b->links.previous != 0) {
    b->links.previous->links.next = b->links.next;
  } else {
    gaol_context(context)->flex_blocks = b->links.next;
  }
  if (b->links.next != 0) {
    b->links.next->links.previous = b->links.previous;
  }
  ::operator delete(b);
}

// As realloc(): p is left as it is when the new block cannot be allocated
void *gaol_flex_realloc(void *context, void *p, std::size_t size)
{
  void *const q = gaol_flex_alloc(context, size);
  if (q != 0 && p != 0) {
    std::memcpy(q, p, std::min(size, (static_cast<gaol_flex_block*>(p) - 1)->links.size));
    gaol_flex_free(context, p);
  }
  return q;
}

// The error of flex, an allocation that failed (YY_FATAL_ERROR)
void gaol_flex_error()
{
  throw std::bad_alloc();
}

/*
  The exception the lexer raised, an allocation that failed, for
  gaol_parse_string() to throw again, as one an action raises: thrown through
  the parser, it left the nodes on its stack (GAOL v5). Called in the handler.
*/
void gaol_lexer_failed(void *context)
{
  gaol_context(context)->exception = std::current_exception();
}

/*
  The stack of the parser, beyond its 200 first entries, in blocks of the
  heap (YYMALLOC, YYFREE), from the nothrow operator new as the blocks of
  flex: a block that cannot be allocated makes the parser stop, as a string
  nested deeper than YYMAXDEPTH does, and gaol_parse_string() throws
  std::bad_alloc rather than input_format_error (GAOL v5)
*/
static void *gaol_parser_alloc(void *context, std::size_t size)
{
  void *const p = ::operator new(size, std::nothrow);
  if (p == 0) {
    gaol_context(context)->out_of_memory = true;
  }
  return p;
}

static void gaol_parser_free(void *p)
{
  ::operator delete(p);
}

#define YYMALLOC(size) gaol_parser_alloc(context, size)
#define YYFREE(p) gaol_parser_free(p)

/*
  One grammar, whose every string is an expression (GAOL v5)

  GAOL read a string with two grammars: the numbers, the constants and the
  functions of them built the tree of gaol/parser/gaol_expression.h, while the
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
  The exception an action raises (GAOL v5): the action keeps it in the
  context of the reading and aborts, the parser frees the nodes it holds, and
  gaol_parse_string() throws it again. Thrown through the parser, it left them.

  Without exceptions, the action aborts and the reading fails, as for a
  syntax error (GAOL v5): gaol_ERROR wrote the error and called abort(), so
  that gaol_ieee1788::textToInterval() ended the program on a wrong call, as
  pown([2,5],2.5) or sin(1,2), where IEEE 1788-2015 has it give the empty set
  (12.12.7) as it does for any other string that is no literal. The callers
  that throw on a syntax error, gaol::textToInterval() and operator>>, still
  end the program then.
*/
#if GAOL_EXCEPTIONS_ENABLED
#  define GAOL_PARSING_ERROR(context,excep,msg)                         \
  do {                                                                  \
    try {                                                               \
      gaol_ERROR(excep,msg);                                            \
    } catch (...) {                                                     \
      gaol_context(context)->exception = std::current_exception();      \
    }                                                                   \
  } while (0)
#else
#  define GAOL_PARSING_ERROR(context,excep,msg) static_cast<void>(context)
#endif

/*
  1 if e is the infinity written inf, -1 if it is -inf, 0 otherwise (GAOL
  v5). An infinity read in an expression is [dmax, +oo] or [-oo, -dmax] (see
  gaol/parser/gaol_expr_eval.h), but a lower bound written inf, or an upper bound written
  -inf, leaves no interval, as numsToInterval of IEEE 1788-2015 (10.5.8): [inf],
  [inf, inf] and [inf,] are the empty set.
*/
static int gaol_literal_infinity(expr_node *e)
{
  // A loop, not a recursion: the signs of a bound, thousands of them, are a chain of nodes (GAOL v5)
  int sign = 1;
  while (unary_minus_node *m = dynamic_cast<unary_minus_node*>(e)) {
    sign = -sign;
    e = m->get_subexpr();
  }
  if (double_node *d = dynamic_cast<double_node*>(e)) {
    return sign * ((d->get_val() == GAOL_INFINITY) ? 1 : ((d->get_val() == -GAOL_INFINITY) ? -1 : 0));
  }
  return 0;
}

// The leaf holding the interval of a literal
static expr_node *gaol_leaf(const interval& I)
{
  return gaol_node(new interval_node(I));
}

/*
  The leaf of a number or of a constant, which an action gives: the node
  built, or 0 when it could not be allocated, the parsing aborting then with
  std::bad_alloc (GAOL v5). An action lets no exception through the parser,
  which left the nodes on its stack.
*/
static expr_node *gaol_number(void *context, double d)
{
  try {
    return gaol_node(new double_node(d));
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
    return 0;
  }
}

static expr_node *gaol_constant(void *context, const interval& I)
{
  try {
    return gaol_leaf(I);
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
    return 0;
  }
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

// The leaf of an uncertain number, as gaol_number()
static expr_node *gaol_uncertain_number(void *context, const interval& I)
{
  try {
    return gaol_node(new uncertain_node(I));
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
    return 0;
  }
}

// e is an uncertain number, alone or behind a minus sign
static bool gaol_uncertain(expr_node *e)
{
  // A loop, as gaol_literal_infinity() (GAOL v5)
  while (unary_minus_node *m = dynamic_cast<unary_minus_node*>(e)) {
    e = m->get_subexpr();
  }
  return dynamic_cast<uncertain_node*>(e) != 0;
}

// e, a bound or 0 for none, is an uncertain number: the error the parsing aborts with
static bool gaol_uncertain_bound(expr_node *e, void *context)
{
  if (e == 0 || !gaol_uncertain(e)) {
    return false;
  }
  GAOL_PARSING_ERROR(context, input_format_error,
    "an uncertain number is no bound of an interval literal (IEEE 1788-2015, 12.11)");
  return true;
}

/*
  The functions a string calls, by their names (GAOL v5)

  A string is read with the names of GAOL, by gaol::textToInterval() and
  operator>>, or with the names of IEEE 1788-2015, by
  gaol_ieee1788::textToInterval(), as a program opens the namespace gaol or
  gaol_ieee1788: parse_interval() gives them to the context of the reading.
  The lexer looks each
  name up in the table of those names, whatever the case of its letters, and
  a name that is not in it is an error: nth_root(8,3) is read with the names
  of GAOL, rootn(8,3) with those of the standard, and pow(x,y) is the pow of
  the namespace of the names, gaol_pow_hybrid() or the pow of Table 9.1.

  Each name gives a function of intervals of one, two or three arguments, or
  of an interval and an int, whose second argument has to be an integer. A
  call is computed when it is read, its arguments being computed then too,
  and gives a leaf of the tree, as a literal does: GAOL built a node of
  gaol/parser/gaol_expression.h for each of its functions, and read no other.
*/
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

// The letter c in lower case, if it is an ASCII capital, whatever the locale
// (GAOL v5): the names are of ASCII letters, which the lexer takes in either
// case (%option caseless). std::tolower() depended on the locale of the C
// library, where 'I' need not be 'i': under the Turkish locales of glibc,
// std::tolower('I') is 'I' (tr_TR.UTF-8) or the dotless i, 0xfd
// (tr_TR.ISO-8859-9), and "SIN(0)" was refused.
static char ascii_lower(char c)
{
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// The function of a name, whatever the case of its letters, in the table of
// the names the string is read with, which its context gives; 0 for a name
// that is not in it
const void *gaol_lookup_function(const char *name, void *context)
{
  const bool ieee1788 = (gaol_context(context)->names == ::gaol_core::parsing_names::ieee1788);
  const gaol_function *table = ieee1788 ? ieee1788_functions : gaol_functions;
  const std::size_t size = ieee1788 ? sizeof(ieee1788_functions)/sizeof(gaol_function)
                                    : sizeof(gaol_functions)/sizeof(gaol_function);
  for (std::size_t i = 0; i < size; ++i) {
    const char *a = table[i].name, *b = name;
    while (*a != '\0' && *b != '\0' && ascii_lower(*a) == ascii_lower(*b)) {
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
static expr_node *gaol_call(void *context, const void *fn, unsigned int n,
                            expr_node *a, expr_node *b, expr_node *c)
{
  const gaol_function *f = static_cast<const gaol_function*>(fn);
  expr_node *result = 0;
  try {
    if (f->arity != n) {
      const std::string msg = std::string(f->name) + " called with a wrong number of arguments";
      GAOL_PARSING_ERROR(context, input_format_error, msg.c_str());
    } else if (f->fn != 0) {
      const interval q = gaol_value(b);
      if (!q.is_an_int()) {
        const std::string msg = std::string(f->name) + " used with non integral 2nd arg.";
        GAOL_PARSING_ERROR(context, invalid_action_error, msg.c_str());
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
    gaol_context(context)->exception = std::current_exception();
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

/*
  The operation op, one of + - * /, on the two expressions given, which it
  releases: the leaf of its value, or 0 for an error, which the parsing aborts
  with (GAOL v5). An operation is computed when it is read, as the call of a
  function is, and is no node of the tree: GAOL built a node for it, so that
  a sum of 100000 terms was a tree 100000 nodes deep, which the evaluation and
  the deletion of the tree walked with one call within another per node and
  overflowed the stack. The value is that of the nodes it replaces, the
  operands being computed in the same order.
*/
static expr_node *gaol_operation(void *context, char op, expr_node *a, expr_node *b)
{
  expr_node *result = 0;
  try {
    const interval x = gaol_value(a);
    const interval y = gaol_value(b);
    switch (op) {
    case '+':
      result = gaol_leaf(x + y);
      break;
    case '-':
      result = gaol_leaf(x - y);
      break;
    case '*':
      result = gaol_leaf(x * y);
      break;
    default:
      result = gaol_leaf(x / y);
      break;
    }
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
    result = 0;
  }
  gaol_release(a);
  gaol_release(b);
  return result;
}

/*
  The minus sign before the expression e, which it releases: its node, or 0
  when it could not be allocated (GAOL v5). A sign is the one operation that
  stays a node, so that a bound written -inf, or an uncertain number behind
  signs, is still told apart: the parser holds no other tree than a chain of
  signs over a leaf, which is evaluated and deleted without recursion.
*/
static expr_node *gaol_negation(void *context, expr_node *e)
{
  expr_node *result = 0;
  try {
    result = gaol_node(new unary_minus_node(*e));
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
  }
  gaol_release(e);
  return result;
}

// The forms of the literals with bounds
enum gaol_literal_form {
  gaol_point,                     // [a]
  gaol_bounded,                   // [a, b]
  gaol_left_only,                 // [a, ]
  gaol_right_only,                // [, b]
  gaol_degenerate                 // <a, b>
};

/*
  The literal of the form given with the bounds l and r, 0 for one left out,
  which it releases: the leaf of its interval, or 0 for an error, which the
  parsing aborts with (GAOL v5). No bound is an uncertain number. A left bound
  written inf, or a right bound written -inf, gives the empty set (see
  gaol_literal_infinity()). The two bounds of <a, b> are the same double: they
  are compared by their bits, -0 and +0 being the same bound, as
  gaol_detail::bound_equal() does, so that two subnormals that are equal as
  doubles under denormals-are-zero are told apart (GAOL v5, point Q of
  TODO.md): compared as doubles, <1e-310, 1e-309> was read [1e-310, 1e-309]
  under that mode, and refused without it.
*/
static expr_node *gaol_literal(void *context, gaol_literal_form form, expr_node *l, expr_node *r)
{
  expr_node *result = 0;
  try {
    if (!gaol_uncertain_bound(l, context) && !gaol_uncertain_bound(r, context)) {
      const interval lv = (l != 0) ? gaol_value(l) : interval::universe();
      const interval rv = (r != 0) ? gaol_value(r) : interval::universe();
      const bool infinite = (l != 0 && (form == gaol_point ? gaol_literal_infinity(l) != 0
                                                           : gaol_literal_infinity(l) > 0))
                            || (r != 0 && gaol_literal_infinity(r) < 0);
      if (form == gaol_point) {
        result = gaol_leaf(infinite ? interval::emptyset() : lv);
      } else if (form != gaol_degenerate) {
        result = gaol_leaf(infinite ? interval::emptyset() : interval(lv.left(), rv.right()));
      } else if (::gaol_detail::bound_equal(lv.left(), rv.right())) {
        result = gaol_leaf(interval(lv.left(), rv.right()));
      } else {
        GAOL_PARSING_ERROR(context, input_format_error,
          "bounds of degenerate interval do not evaluate to the same value");
      }
    }
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
    result = 0;
  }
  if (l != 0) {
    gaol_release(l);
  }
  if (r != 0) {
    gaol_release(r);
  }
  return result;
}

/*
  The interval of the whole string, the value of e, which it releases: false
  for an allocation that failed, which the parsing aborts with (GAOL v5)
*/
static bool gaol_result(void *context, expr_node *e)
{
  bool computed = false;
  try {
    gaol_context(context)->result = gaol_value(e);
    gaol_context(context)->ok = true;
    computed = true;
  } catch (...) {
    gaol_context(context)->exception = std::current_exception();
  }
  gaol_release(e);
  return computed;
}

#line 799 "y.tab.c"

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
#line 724 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;
  const void* fn;

#line 890 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int gaol_parse (void *scanner, void *context);


#endif /* !YY_GAOL_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_EMPTY_STR = 3,                  /* EMPTY_STR  */
  YYSYMBOL_ENTIRE_STR = 4,                 /* ENTIRE_STR  */
  YYSYMBOL_INFINITY_STR = 5,               /* INFINITY_STR  */
  YYSYMBOL_DMIN_STR = 6,                   /* DMIN_STR  */
  YYSYMBOL_DMAX_STR = 7,                   /* DMAX_STR  */
  YYSYMBOL_PI_STR = 8,                     /* PI_STR  */
  YYSYMBOL_FUNCTION_NAME = 9,              /* FUNCTION_NAME  */
  YYSYMBOL_UNEXPECTED_CHAR = 10,           /* UNEXPECTED_CHAR  */
  YYSYMBOL_NUMBER = 11,                    /* NUMBER  */
  YYSYMBOL_INTERVAL_CST = 12,              /* INTERVAL_CST  */
  YYSYMBOL_UNCERTAIN_CST = 13,             /* UNCERTAIN_CST  */
  YYSYMBOL_14_ = 14,                       /* '+'  */
  YYSYMBOL_15_ = 15,                       /* '-'  */
  YYSYMBOL_16_ = 16,                       /* '*'  */
  YYSYMBOL_17_ = 17,                       /* '/'  */
  YYSYMBOL_UMINUS = 18,                    /* UMINUS  */
  YYSYMBOL_UPLUS = 19,                     /* UPLUS  */
  YYSYMBOL_20_ = 20,                       /* '('  */
  YYSYMBOL_21_ = 21,                       /* ')'  */
  YYSYMBOL_22_ = 22,                       /* '['  */
  YYSYMBOL_23_ = 23,                       /* ']'  */
  YYSYMBOL_24_ = 24,                       /* ','  */
  YYSYMBOL_25_ = 25,                       /* '<'  */
  YYSYMBOL_26_ = 26,                       /* '>'  */
  YYSYMBOL_YYACCEPT = 27,                  /* $accept  */
  YYSYMBOL_itv_expr = 28,                  /* itv_expr  */
  YYSYMBOL_expression = 29,                /* expression  */
  YYSYMBOL_literal = 30,                   /* literal  */
  YYSYMBOL_function_call = 31              /* function_call  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 741 "gaol_interval_parser.ypp"

// The lexer of flex, a reentrant one given the value of the token to set
// (%option bison-bridge in gaol/parser/gaol_interval_lexer.lpp)
int gaol_lex(YYSTYPE *lval, void *scanner);

/* gaol_parse() counts the syntax errors in yynerrs, which nothing reads:
   Clang warned that the variable is set but not used (GAOL v5) */
#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wunused-but-set-variable")
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
#  endif
#endif

#line 960 "y.tab.c"


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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   270


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

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
       0,   779,   779,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     812,   813,   814,   815,   816,   817,   818,   819,   823,   824,
     826
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "EMPTY_STR",
  "ENTIRE_STR", "INFINITY_STR", "DMIN_STR", "DMAX_STR", "PI_STR",
  "FUNCTION_NAME", "UNEXPECTED_CHAR", "NUMBER", "INTERVAL_CST",
  "UNCERTAIN_CST", "'+'", "'-'", "'*'", "'/'", "UMINUS", "UPLUS", "'('",
  "')'", "'['", "']'", "','", "'<'", "'>'", "$accept", "itv_expr",
  "expression", "literal", "function_call", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

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
       0,    15,    16,    17,    18
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

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     6,     7,     8,     9,    11,    12,    13,
      14,    15,    20,    22,    25,    28,    29,    30,    31,    20,
      29,    29,    29,     4,    23,    24,    29,    29,     0,    14,
      15,    16,    17,    29,    21,    23,    23,    29,    23,    24,
      24,    29,    29,    29,    29,    21,    24,    23,    23,    29,
      29,    29,    23,    26,    21,    24,    29,    21
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    27,    28,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    29,    29,    29,    29,    29,    29,
      30,    30,    30,    30,    30,    30,    30,    30,    31,    31,
      31
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     2,     2,     1,     3,
       3,     5,     2,     3,     3,     4,     4,     5,     4,     6,
       8
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
        yyerror (scanner, context, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner, context); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, void *context)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  YY_USE (context);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, void *context)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner, context);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner, void *context)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner, context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner, context); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner, void *context)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  YY_USE (context);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_expression: /* expression  */
#line 768 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1707 "y.tab.c"
        break;

    case YYSYMBOL_literal: /* literal  */
#line 768 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1713 "y.tab.c"
        break;

    case YYSYMBOL_function_call: /* function_call  */
#line 768 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1719 "y.tab.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, void *context)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  case 2: /* itv_expr: expression  */
#line 779 "gaol_interval_parser.ypp"
                                                        { if (!gaol_result(context,(yyvsp[0].expr))) YYABORT; }
#line 1995 "y.tab.c"
    break;

  case 3: /* expression: NUMBER  */
#line 787 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_number(context,(yyvsp[0].d))) == 0) YYABORT; }
#line 2001 "y.tab.c"
    break;

  case 4: /* expression: DMIN_STR  */
#line 788 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_number(context,std::numeric_limits<double>::min())) == 0) YYABORT; }
#line 2007 "y.tab.c"
    break;

  case 5: /* expression: DMAX_STR  */
#line 789 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_number(context,std::numeric_limits<double>::max())) == 0) YYABORT; }
#line 2013 "y.tab.c"
    break;

  case 6: /* expression: INFINITY_STR  */
#line 790 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_number(context,GAOL_INFINITY)) == 0) YYABORT; }
#line 2019 "y.tab.c"
    break;

  case 7: /* expression: PI_STR  */
#line 791 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval::pi())) == 0) YYABORT; }
#line 2025 "y.tab.c"
    break;

  case 8: /* expression: INTERVAL_CST  */
#line 792 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval((yyvsp[0].itv).l,(yyvsp[0].itv).r))) == 0) YYABORT; }
#line 2031 "y.tab.c"
    break;

  case 9: /* expression: UNCERTAIN_CST  */
#line 793 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_uncertain_number(context,interval((yyvsp[0].itv).l,(yyvsp[0].itv).r))) == 0) YYABORT; }
#line 2037 "y.tab.c"
    break;

  case 10: /* expression: EMPTY_STR  */
#line 794 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval::emptyset())) == 0) YYABORT; }
#line 2043 "y.tab.c"
    break;

  case 11: /* expression: literal  */
#line 795 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 2049 "y.tab.c"
    break;

  case 12: /* expression: expression '+' expression  */
#line 796 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_operation(context,'+',(yyvsp[-2].expr),(yyvsp[0].expr))) == 0) YYABORT; }
#line 2055 "y.tab.c"
    break;

  case 13: /* expression: expression '-' expression  */
#line 797 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_operation(context,'-',(yyvsp[-2].expr),(yyvsp[0].expr))) == 0) YYABORT; }
#line 2061 "y.tab.c"
    break;

  case 14: /* expression: expression '*' expression  */
#line 798 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_operation(context,'*',(yyvsp[-2].expr),(yyvsp[0].expr))) == 0) YYABORT; }
#line 2067 "y.tab.c"
    break;

  case 15: /* expression: expression '/' expression  */
#line 799 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_operation(context,'/',(yyvsp[-2].expr),(yyvsp[0].expr))) == 0) YYABORT; }
#line 2073 "y.tab.c"
    break;

  case 16: /* expression: '-' expression  */
#line 800 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_negation(context,(yyvsp[0].expr))) == 0) YYABORT; }
#line 2079 "y.tab.c"
    break;

  case 17: /* expression: '+' expression  */
#line 801 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 2085 "y.tab.c"
    break;

  case 18: /* expression: function_call  */
#line 802 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 2091 "y.tab.c"
    break;

  case 19: /* expression: '(' expression ')'  */
#line 803 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 2097 "y.tab.c"
    break;

  case 20: /* literal: '[' expression ']'  */
#line 812 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_literal(context,gaol_point,(yyvsp[-1].expr),0)) == 0) YYABORT; }
#line 2103 "y.tab.c"
    break;

  case 21: /* literal: '[' expression ',' expression ']'  */
#line 813 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_literal(context,gaol_bounded,(yyvsp[-3].expr),(yyvsp[-1].expr))) == 0) YYABORT; }
#line 2109 "y.tab.c"
    break;

  case 22: /* literal: '[' ']'  */
#line 814 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval::emptyset())) == 0) YYABORT; }
#line 2115 "y.tab.c"
    break;

  case 23: /* literal: '[' ENTIRE_STR ']'  */
#line 815 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval::universe())) == 0) YYABORT; }
#line 2121 "y.tab.c"
    break;

  case 24: /* literal: '[' ',' ']'  */
#line 816 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_constant(context,interval::universe())) == 0) YYABORT; }
#line 2127 "y.tab.c"
    break;

  case 25: /* literal: '[' expression ',' ']'  */
#line 817 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_literal(context,gaol_left_only,(yyvsp[-2].expr),0)) == 0) YYABORT; }
#line 2133 "y.tab.c"
    break;

  case 26: /* literal: '[' ',' expression ']'  */
#line 818 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_literal(context,gaol_right_only,0,(yyvsp[-1].expr))) == 0) YYABORT; }
#line 2139 "y.tab.c"
    break;

  case 27: /* literal: '<' expression ',' expression '>'  */
#line 819 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_literal(context,gaol_degenerate,(yyvsp[-3].expr),(yyvsp[-1].expr))) == 0) YYABORT; }
#line 2145 "y.tab.c"
    break;

  case 28: /* function_call: FUNCTION_NAME '(' expression ')'  */
#line 823 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call(context,(yyvsp[-3].fn),1,(yyvsp[-1].expr),0,0)) == 0) YYABORT; }
#line 2151 "y.tab.c"
    break;

  case 29: /* function_call: FUNCTION_NAME '(' expression ',' expression ')'  */
#line 825 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call(context,(yyvsp[-5].fn),2,(yyvsp[-3].expr),(yyvsp[-1].expr),0)) == 0) YYABORT; }
#line 2157 "y.tab.c"
    break;

  case 30: /* function_call: FUNCTION_NAME '(' expression ',' expression ',' expression ')'  */
#line 827 "gaol_interval_parser.ypp"
                                                        { if (((yyval.expr) = gaol_call(context,(yyvsp[-7].fn),3,(yyvsp[-5].expr),(yyvsp[-3].expr),(yyvsp[-1].expr))) == 0) YYABORT; }
#line 2163 "y.tab.c"
    break;


#line 2167 "y.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, context, YY_("syntax error"));
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
                      yytoken, &yylval, scanner, context);
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner, context);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, context, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner, context);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner, context);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 830 "gaol_interval_parser.ypp"


void gaol_error_bison(void *, void *context, const char *)
{
  gaol_context(context)->ok = false; // An error occurred
}

/*
  Reads the n characters at s, NUL characters included, with the names of
  functions given: returns true, with out set to the interval read, if they
  are an expression, and false, with out unchanged, otherwise. Throws again
  the exception an action or the lexer raised, and std::bad_alloc when the
  stack of the parser could not grow (GAOL v5). Its scanner and its context
  are its own, and several threads call it at once.
*/
bool gaol_core::gaol_parse_string(const char *const s, std::size_t n, interval& out, ::gaol_core::parsing_names names)
{
  gaol_parsing_context context;
  context.ok = false;
  context.names = names;
  context.input = s;
  context.input_left = n;
  context.out_of_memory = false;
  context.flex_blocks = 0;
  // Frees the blocks flex has not freed, after the scanner, whatever is thrown
  struct flex_blocks_guard {
    gaol_parsing_context& c;
    ~flex_blocks_guard()
    {
      while (c.flex_blocks != 0) {
        gaol_flex_free(&c, c.flex_blocks + 1);
      }
    }
  } guard = { context };
  void *scanner = gaol_initialize_parsing(&context);
  if (scanner == 0) {
    throw std::bad_alloc();
  }
  int status;
  try {
    status = gaol_parse(scanner, &context);
  } catch (...) {
    // The scanner is freed whatever the parser throws
    gaol_cleanup_parsing(scanner);
    throw;
  }
  gaol_cleanup_parsing(scanner);
  if (context.exception) {
    // Thrown in the parser, which aborted to free its nodes
    std::rethrow_exception(context.exception);
  }
  if (status == 2 && context.out_of_memory) {
    throw std::bad_alloc();
  }
  if (context.ok) {
    // The infinities read give [dmax, +oo] and [-oo, -dmax] already
    // (gaol/parser/gaol_expr_eval.h): interval(+oo) and interval(-oo) are the empty set
    out = context.result;
  }
  return context.ok;
}
