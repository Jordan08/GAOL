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

#line 241 "y.tab.c"

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
#line 165 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;

#line 374 "y.tab.c"

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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   466

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  5
/* YYNRULES -- Number of rules.  */
#define YYNRULES  50
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  146

#define YYUNDEFTOK  2
#define YYMAXUTOK   292


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
      42,    43,    38,    36,    46,    37,     2,    39,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      47,     2,    48,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    44,     2,    45,     2,     2,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    40,    41
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   202,   202,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   219,   221,   223,   225,   227,   228,   229,
     238,   246,   256,   257,   258,   259,   268,   277,   292,   293,
     294,   295,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   314,   315,   321,
     327
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EMPTY_STR", "ENTIRE_STR",
  "INFINITY_STR", "DMIN_STR", "DMAX_STR", "PI_STR", "EXP_STR", "LOG_STR",
  "POW_STR", "NTH_ROOT_STR", "SQRT_STR", "EXP2_STR", "LOG2_STR",
  "CBRT_STR", "SIGN_STR", "TRUNC_STR", "COS_STR", "SIN_STR", "TAN_STR",
  "ATAN2_STR", "COSH_STR", "SINH_STR", "TANH_STR", "ACOS_STR", "ASIN_STR",
  "ATAN_STR", "ACOSH_STR", "ASINH_STR", "ATANH_STR", "UNEXPECTED_CHAR",
  "NUMBER", "INTERVAL_CST", "UNCERTAIN_CST", "'+'", "'-'", "'*'", "'/'",
  "UMINUS", "UPLUS", "'('", "')'", "'['", "']'", "','", "'<'", "'>'",
  "$accept", "itv_expr", "expression", "literal", "function_call", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,    43,    45,    42,    47,
     291,   292,    40,    41,    91,    93,    44,    60,    62
};
# endif

#define YYPACT_NINF (-33)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     221,   -33,   -33,   -33,   -33,   -33,   -10,    -2,    12,    15,
      16,    17,    27,    28,    30,    38,    41,    42,    43,    45,
      46,    47,    48,    49,    80,    86,    87,    88,    89,   -33,
     -33,   -33,   221,   221,   221,    90,   221,   133,   -31,   -33,
     -33,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   -33,   -33,   223,    94,   -33,   135,
      10,    14,   -33,   221,   221,   221,   221,   247,   255,    25,
      29,   263,   271,   279,   287,   295,   303,   311,   319,   327,
      40,   335,   343,   351,   359,   367,   375,   383,   391,   399,
     -33,   -33,   -33,   233,   -33,   178,   221,     0,     0,   -33,
     -33,   -33,   -33,   221,   221,   -33,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,   221,   -33,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,   -33,   -33,   237,    -3,   407,   415,
     423,   -33,   -33,   -33,   -33,   -33
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    10,     6,     4,     5,     7,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       8,     9,     0,     0,     0,     0,     0,     0,     2,    11,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    16,     0,     0,    22,     0,
       0,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,    23,    24,     0,    20,     0,     0,    12,    13,    14,
      15,    41,    42,     0,     0,    47,    43,    44,    48,    45,
      46,    28,    29,    30,     0,    35,    36,    37,    32,    33,
      34,    38,    39,    40,    26,    25,     0,     0,     0,     0,
       0,    21,    27,    49,    50,    31
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -33,   -33,   -32,   -33,   -33
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    37,    38,    39,    40
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      64,    65,    66,    70,    71,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    41,    73,    74,    75,    76,   103,    75,    76,
      42,   107,   108,   109,   110,   142,    73,    74,    75,    76,
      73,    74,    75,    76,    43,   104,   105,    44,    45,    46,
     106,    73,    74,    75,    76,    73,    74,    75,    76,    47,
      48,   113,    49,   136,   137,   114,    73,    74,    75,    76,
      50,   138,   139,    51,    52,    53,   124,    54,    55,    56,
      57,    58,   140,     1,    67,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    59,    29,    30,    31,    32,    33,    60,    61,
      62,    63,    34,    72,    35,    68,    69,    36,     1,   101,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,     0,    34,     0,    35,
     102,     1,    36,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,     0,
      34,     0,    35,   135,     1,    36,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,    73,
      74,    75,    76,    34,     0,    35,   100,     0,    36,    73,
      74,    75,    76,    73,    74,    75,    76,     0,   134,     0,
       0,     0,   141,    73,    74,    75,    76,     0,     0,     0,
     111,    73,    74,    75,    76,     0,     0,     0,   112,    73,
      74,    75,    76,     0,     0,     0,   115,    73,    74,    75,
      76,     0,     0,     0,   116,    73,    74,    75,    76,     0,
       0,     0,   117,    73,    74,    75,    76,     0,     0,     0,
     118,    73,    74,    75,    76,     0,     0,     0,   119,    73,
      74,    75,    76,     0,     0,     0,   120,    73,    74,    75,
      76,     0,     0,     0,   121,    73,    74,    75,    76,     0,
       0,     0,   122,    73,    74,    75,    76,     0,     0,     0,
     123,    73,    74,    75,    76,     0,     0,     0,   125,    73,
      74,    75,    76,     0,     0,     0,   126,    73,    74,    75,
      76,     0,     0,     0,   127,    73,    74,    75,    76,     0,
       0,     0,   128,    73,    74,    75,    76,     0,     0,     0,
     129,    73,    74,    75,    76,     0,     0,     0,   130,    73,
      74,    75,    76,     0,     0,     0,   131,    73,    74,    75,
      76,     0,     0,     0,   132,    73,    74,    75,    76,     0,
       0,     0,   133,    73,    74,    75,    76,     0,     0,     0,
     143,    73,    74,    75,    76,     0,     0,     0,   144,    73,
      74,    75,    76,     0,     0,     0,   145
};

static const yytype_int8 yycheck[] =
{
      32,    33,    34,    35,    36,    36,    37,    38,    39,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    42,    36,    37,    38,    39,    69,    38,    39,
      42,    73,    74,    75,    76,    48,    36,    37,    38,    39,
      36,    37,    38,    39,    42,    45,    46,    42,    42,    42,
      46,    36,    37,    38,    39,    36,    37,    38,    39,    42,
      42,    46,    42,   105,   106,    46,    36,    37,    38,    39,
      42,   113,   114,    42,    42,    42,    46,    42,    42,    42,
      42,    42,   124,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    42,    33,    34,    35,    36,    37,    42,    42,
      42,    42,    42,     0,    44,    45,    46,    47,     3,    45,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    -1,    42,    -1,    44,
      45,     3,    47,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    -1,
      42,    -1,    44,    45,     3,    47,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    33,    34,    35,    36,    37,    36,
      37,    38,    39,    42,    -1,    44,    43,    -1,    47,    36,
      37,    38,    39,    36,    37,    38,    39,    -1,    45,    -1,
      -1,    -1,    45,    36,    37,    38,    39,    -1,    -1,    -1,
      43,    36,    37,    38,    39,    -1,    -1,    -1,    43,    36,
      37,    38,    39,    -1,    -1,    -1,    43,    36,    37,    38,
      39,    -1,    -1,    -1,    43,    36,    37,    38,    39,    -1,
      -1,    -1,    43,    36,    37,    38,    39,    -1,    -1,    -1,
      43,    36,    37,    38,    39,    -1,    -1,    -1,    43,    36,
      37,    38,    39,    -1,    -1,    -1,    43,    36,    37,    38,
      39,    -1,    -1,    -1,    43,    36,    37,    38,    39,    -1,
      -1,    -1,    43,    36,    37,    38,    39,    -1,    -1,    -1,
      43,    36,    37,    38,    39,    -1,    -1,    -1,    43,    36,
      37,    38,    39,    -1,    -1,    -1,    43,    36,    37,    38,
      39,    -1,    -1,    -1,    43,    36,    37,    38,    39,    -1,
      -1,    -1,    43,    36,    37,    38,    39,    -1,    -1,    -1,
      43,    36,    37,    38,    39,    -1,    -1,    -1,    43,    36,
      37,    38,    39,    -1,    -1,    -1,    43,    36,    37,    38,
      39,    -1,    -1,    -1,    43,    36,    37,    38,    39,    -1,
      -1,    -1,    43,    36,    37,    38,    39,    -1,    -1,    -1,
      43,    36,    37,    38,    39,    -1,    -1,    -1,    43,    36,
      37,    38,    39,    -1,    -1,    -1,    43
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    33,
      34,    35,    36,    37,    42,    44,    47,    50,    51,    52,
      53,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    51,    51,    51,     4,    45,    46,
      51,    51,     0,    36,    37,    38,    39,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      43,    45,    45,    51,    45,    46,    46,    51,    51,    51,
      51,    43,    43,    46,    46,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    46,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    45,    45,    51,    51,    51,    51,
      51,    45,    48,    43,    43,    43
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    49,    50,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      52,    52,    52,    52,    52,    52,    52,    52,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     2,     2,     1,     3,
       3,     5,     2,     3,     3,     4,     4,     5,     4,     4,
       4,     6,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     6,
       6
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
    case 51: /* expression  */
#line 191 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1440 "y.tab.c"
        break;

    case 52: /* literal  */
#line 191 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1446 "y.tab.c"
        break;

    case 53: /* function_call  */
#line 191 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1452 "y.tab.c"
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
#line 202 "gaol_interval_parser.ypp"
                                                        { *gaol_result_of_parsing = gaol_value((yyvsp[0].expr));
							  gaol_release((yyvsp[0].expr));
							  gaol_global_parsing_flag = true; }
#line 1724 "y.tab.c"
    break;

  case 3:
#line 208 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node((yyvsp[0].d))); }
#line 1730 "y.tab.c"
    break;

  case 4:
#line 209 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(std::numeric_limits<double>::min())); }
#line 1736 "y.tab.c"
    break;

  case 5:
#line 210 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(std::numeric_limits<double>::max())); }
#line 1742 "y.tab.c"
    break;

  case 6:
#line 211 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new double_node(GAOL_INFINITY)); }
#line 1748 "y.tab.c"
    break;

  case 7:
#line 212 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::pi()); }
#line 1754 "y.tab.c"
    break;

  case 8:
#line 213 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval((yyvsp[0].itv).l,(yyvsp[0].itv).r)); }
#line 1760 "y.tab.c"
    break;

  case 9:
#line 214 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new uncertain_node(interval((yyvsp[0].itv).l,(yyvsp[0].itv).r))); }
#line 1766 "y.tab.c"
    break;

  case 10:
#line 215 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::emptyset()); }
#line 1772 "y.tab.c"
    break;

  case 11:
#line 216 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1778 "y.tab.c"
    break;

  case 12:
#line 217 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new add_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1785 "y.tab.c"
    break;

  case 13:
#line 219 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new sub_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1792 "y.tab.c"
    break;

  case 14:
#line 221 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new mult_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1799 "y.tab.c"
    break;

  case 15:
#line 223 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new div_node(*(yyvsp[-2].expr),*(yyvsp[0].expr)));
							  gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 1806 "y.tab.c"
    break;

  case 16:
#line 225 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new unary_minus_node(*(yyvsp[0].expr)));
							  gaol_release((yyvsp[0].expr)); }
#line 1813 "y.tab.c"
    break;

  case 17:
#line 227 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1819 "y.tab.c"
    break;

  case 18:
#line 228 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[0].expr); }
#line 1825 "y.tab.c"
    break;

  case 19:
#line 229 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 1831 "y.tab.c"
    break;

  case 20:
#line 238 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval v = gaol_value((yyvsp[-1].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-1].expr)) != 0);
							  gaol_release((yyvsp[-1].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset() : v); }
#line 1844 "y.tab.c"
    break;

  case 21:
#line 246 "gaol_interval_parser.ypp"
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
#line 1859 "y.tab.c"
    break;

  case 22:
#line 256 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::emptyset()); }
#line 1865 "y.tab.c"
    break;

  case 23:
#line 257 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::universe()); }
#line 1871 "y.tab.c"
    break;

  case 24:
#line 258 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_leaf(interval::universe()); }
#line 1877 "y.tab.c"
    break;

  case 25:
#line 259 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-2].expr))) {
							    gaol_release((yyvsp[-2].expr));
							    YYABORT;
							  }
							  const interval l = gaol_value((yyvsp[-2].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-2].expr)) > 0);
							  gaol_release((yyvsp[-2].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset()
									 : interval(l.left(),GAOL_INFINITY)); }
#line 1891 "y.tab.c"
    break;

  case 26:
#line 268 "gaol_interval_parser.ypp"
                                                        { if (gaol_uncertain_bound((yyvsp[-1].expr))) {
							    gaol_release((yyvsp[-1].expr));
							    YYABORT;
							  }
							  const interval r = gaol_value((yyvsp[-1].expr));
							  const bool infinite = (gaol_literal_infinity((yyvsp[-1].expr)) < 0);
							  gaol_release((yyvsp[-1].expr));
							  (yyval.expr) = gaol_leaf(infinite ? interval::emptyset()
									 : interval(-GAOL_INFINITY,r.right())); }
#line 1905 "y.tab.c"
    break;

  case 27:
#line 277 "gaol_interval_parser.ypp"
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
#line 1922 "y.tab.c"
    break;

  case 28:
#line 292 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new cos_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1928 "y.tab.c"
    break;

  case 29:
#line 293 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new sin_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1934 "y.tab.c"
    break;

  case 30:
#line 294 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new tan_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1940 "y.tab.c"
    break;

  case 31:
#line 295 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new atan2_node(*(yyvsp[-3].expr),*(yyvsp[-1].expr)));
							  gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr)); }
#line 1947 "y.tab.c"
    break;

  case 32:
#line 297 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new acos_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1953 "y.tab.c"
    break;

  case 33:
#line 298 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new asin_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1959 "y.tab.c"
    break;

  case 34:
#line 299 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new atan_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1965 "y.tab.c"
    break;

  case 35:
#line 300 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new cosh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1971 "y.tab.c"
    break;

  case 36:
#line 301 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new sinh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1977 "y.tab.c"
    break;

  case 37:
#line 302 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new tanh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1983 "y.tab.c"
    break;

  case 38:
#line 303 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new acosh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1989 "y.tab.c"
    break;

  case 39:
#line 304 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new asinh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 1995 "y.tab.c"
    break;

  case 40:
#line 305 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new atanh_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2001 "y.tab.c"
    break;

  case 41:
#line 306 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new exp_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2007 "y.tab.c"
    break;

  case 42:
#line 307 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new log_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2013 "y.tab.c"
    break;

  case 43:
#line 308 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new exp2_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2019 "y.tab.c"
    break;

  case 44:
#line 309 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new log2_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2025 "y.tab.c"
    break;

  case 45:
#line 310 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new sign_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2031 "y.tab.c"
    break;

  case 46:
#line 311 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new trunc_node(*(yyvsp[-1].expr))); gaol_release((yyvsp[-1].expr)); }
#line 2037 "y.tab.c"
    break;

  case 47:
#line 314 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new nth_root_node(*(yyvsp[-1].expr),2)); gaol_release((yyvsp[-1].expr)); }
#line 2043 "y.tab.c"
    break;

  case 48:
#line 315 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new nth_root_node(*(yyvsp[-1].expr),3)); gaol_release((yyvsp[-1].expr)); }
#line 2049 "y.tab.c"
    break;

  case 49:
#line 321 "gaol_interval_parser.ypp"
                                                        { (yyval.expr) = gaol_node(new pow_itv_node(*(yyvsp[-3].expr),*(yyvsp[-1].expr)));
							  gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr)); }
#line 2056 "y.tab.c"
    break;

  case 50:
#line 327 "gaol_interval_parser.ypp"
                                                         { const interval q = gaol_value((yyvsp[-1].expr));
							  gaol_release((yyvsp[-1].expr));
							  if (!q.is_an_int()) {
							    gaol_release((yyvsp[-3].expr));
							    GAOL_PARSING_ERROR(invalid_action_error,
							      "nth_root used with non integral 2nd arg.");
							    YYABORT;
							  }
							  const int n = static_cast<int>(q.left());
							  if (n < 0) {
							    expr_node *one = gaol_node(new double_node(1.0));
							    expr_node *root = gaol_node(new nth_root_node(*(yyvsp[-3].expr),
							      static_cast<unsigned int>(-static_cast<long>(n))));
							    (yyval.expr) = gaol_node(new div_node(*one,*root));
							    gaol_release(one);
							    gaol_release(root);
							  } else {
							    (yyval.expr) = gaol_node(new nth_root_node(*(yyvsp[-3].expr),static_cast<unsigned int>(n)));
							  }
							  gaol_release((yyvsp[-3].expr)); }
#line 2081 "y.tab.c"
    break;


#line 2085 "y.tab.c"

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
#line 349 "gaol_interval_parser.ypp"


int gaol_error_bison(const char *s ...)
{
  gaol_global_parsing_flag = false;
  return 0; // An error occurred
}
