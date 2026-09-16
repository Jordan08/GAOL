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
#include "gaol/gaol_limits.h"
#include "gaol/gaol_exceptions.h"

#undef yyerror
#define yyerror gaol_error_bison

int gaol_error_bison(const char *s ...);
int gaol_lex(void);

using namespace gaol;
extern interval *gaol_result_of_parsing;
extern interval gaol_tmp_itv;
extern bool gaol_global_parsing_flag;

/*
  The parser holds one reference to each node it builds ($$->inc_refcount()),
  which it gives up when the node goes into another node, which holds its own,
  or into an expression (fork of GAOL): GAOL kept it, and the nodes of every
  expression read were never freed. the_null_expr keeps the reference
  init() gives it.
*/
static void gaol_release(expr_node *e)
{
  if (e->dec_refcount() == 0) {
    delete e;
  }
}

/*
  The exception the evaluation of an exponent threw (fork of GAOL): the action
  keeps it and aborts, the parser frees the nodes it holds, and
  parse_interval() throws it again. Thrown through the parser, it left them.
*/
std::exception_ptr gaol_parsing_exception;

/*
  Evaluates e, the exponent of pow() or nth_root(), into v; aborted is set
  when the evaluation throws, as atan2() does
*/
static bool gaol_evaluate_exponent(expr_node *e, interval& v, bool& aborted)
{
  try {
    return evaluate_expr(expression(*e),v);
  } catch (...) {
    gaol_parsing_exception = std::current_exception();
    aborted = true;
    return false;
  }
}

/*
  1 if e is the infinity written inf, -1 if it is -inf, 0 otherwise (fork of
  GAOL). An infinity read in an expression is [dmax, +oo] or [-oo, -dmax] (see
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

#line 175 "y.tab.c"

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
    COS_STR = 269,
    SIN_STR = 270,
    TAN_STR = 271,
    ATAN2_STR = 272,
    COSH_STR = 273,
    SINH_STR = 274,
    TANH_STR = 275,
    ACOS_STR = 276,
    ASIN_STR = 277,
    ATAN_STR = 278,
    ACOSH_STR = 279,
    ASINH_STR = 280,
    ATANH_STR = 281,
    UNEXPECTED_CHAR = 282,
    NUMBER = 283,
    INTERVAL_CST = 284,
    UNCERTAIN_CST = 285,
    UMINUS = 286,
    UPLUS = 287
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
#define COS_STR 269
#define SIN_STR 270
#define TAN_STR 271
#define ATAN2_STR 272
#define COSH_STR 273
#define SINH_STR 274
#define TANH_STR 275
#define ACOS_STR 276
#define ASIN_STR 277
#define ATAN_STR 278
#define ACOSH_STR 279
#define ASINH_STR 280
#define ATANH_STR 281
#define UNEXPECTED_CHAR 282
#define NUMBER 283
#define INTERVAL_CST 284
#define UNCERTAIN_CST 285
#define UMINUS 286
#define UPLUS 287

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 99 "gaol_interval_parser.ypp"

  int i;
  double d;
  Interval_struct itv;
  expr_node* expr;

#line 298 "y.tab.c"

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
#define YYFINAL  89
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   632

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  73
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  244

#define YYUNDEFTOK  2
#define YYMAXUTOK   287


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
      37,    38,    33,    31,    39,    32,     2,    34,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      42,     2,    43,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    40,     2,    41,     2,     2,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    35,    36
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   133,   133,   137,   138,   140,   142,   144,   146,   148,
     149,   150,   155,   157,   159,   161,   164,   166,   168,   170,
     172,   174,   176,   178,   180,   182,   184,   186,   189,   191,
     204,   212,   218,   228,   239,   247,   252,   257,   262,   271,
     280,   285,   299,   301,   303,   305,   307,   309,   311,   314,
     317,   320,   323,   326,   327,   328,   332,   335,   338,   341,
     344,   347,   350,   353,   356,   359,   362,   365,   368,   371,
     374,   377,   419,   423
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EMPTY_STR", "ENTIRE_STR",
  "INFINITY_STR", "DMIN_STR", "DMAX_STR", "PI_STR", "EXP_STR", "LOG_STR",
  "POW_STR", "NTH_ROOT_STR", "SQRT_STR", "COS_STR", "SIN_STR", "TAN_STR",
  "ATAN2_STR", "COSH_STR", "SINH_STR", "TANH_STR", "ACOS_STR", "ASIN_STR",
  "ATAN_STR", "ACOSH_STR", "ASINH_STR", "ATANH_STR", "UNEXPECTED_CHAR",
  "NUMBER", "INTERVAL_CST", "UNCERTAIN_CST", "'+'", "'-'", "'*'", "'/'",
  "UMINUS", "UPLUS", "'('", "')'", "','", "'['", "']'", "'<'", "'>'",
  "$accept", "itv_expr", "itv_expr_aux", "itv_function_call",
  "parsed_interval", "expression", "function_call", YY_NULLPTR
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
     285,    43,    45,    42,    47,   286,   287,    40,    41,    44,
      91,    93,    60,    62
};
# endif

#define YYPACT_NINF (-28)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     158,   -28,   -28,   -28,   -28,   -28,    -4,     6,    17,    23,
      28,    29,    35,    36,    44,    45,    70,    71,    96,   117,
     118,   119,   120,   121,   -28,   -28,   -28,   158,   158,   158,
     233,   344,    40,   -26,   -28,   -28,    98,   -28,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   -28,   -28,   -28,   -28,
      46,    72,    42,    68,   122,   123,   148,   159,   173,   175,
     181,   182,   184,   187,   193,   194,   197,   223,   226,   234,
     260,   263,   344,   344,   344,   270,   -28,     3,    14,   -28,
     158,   158,   158,   158,   344,   344,   344,   344,   195,   235,
     272,   309,   110,   114,   160,   170,   346,   354,   362,   370,
     378,   386,   394,   402,   174,   183,   410,   418,   426,   434,
     442,   450,   458,   466,   474,   482,   490,   498,   506,   514,
     522,   530,   538,   546,   -28,   -28,   -28,   -28,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   -28,   -28,    72,   -28,
      18,   307,   -28,   344,   -24,   -24,   -28,   -28,     5,     5,
     -28,   -28,   -28,   -28,   -28,   -28,   158,   344,   158,   344,
     -28,   -28,   -28,   -28,   -28,   -28,   -28,   -28,   158,   344,
     -28,   -28,   -28,   -28,   -28,   -28,   -28,   -28,   -28,   -28,
     -28,   -28,   -28,   -28,   -28,   -28,   -28,   -28,   235,   309,
     114,   170,   354,   370,   386,   402,   183,   418,   434,   450,
     466,   482,   498,   514,   530,   546,   -28,   -28,    30,    -2,
     554,   562,   570,   578,   586,   594,   -28,   -28,   -28,   -28,
     -28,   -28,   -28,   -28
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    31,    47,    43,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    46,    40,     0,     0,     0,
       0,     0,     0,     2,    10,     3,    30,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     9,    30,     8,    30,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    35,     0,     0,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    55,    34,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    52,     0,    37,
       0,     0,    32,     0,     4,     5,     6,     7,    48,    49,
      50,    51,    25,    69,    26,    70,     0,     0,     0,     0,
      28,    72,    12,    56,    13,    57,    14,    58,     0,     0,
      19,    63,    20,    64,    21,    65,    16,    60,    17,    61,
      18,    62,    22,    66,    23,    67,    24,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    39,    38,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    41,    27,    71,
      29,    73,    15,    59
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,    47,   -28,   -28,   -27,   -28
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    32,    33,    34,    35,    36,    37
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      57,    59,    61,    87,    88,    90,    91,    92,    93,    92,
      93,    99,   101,   103,   105,   107,   109,   111,   113,   115,
     117,   119,   121,   123,   125,   127,   129,   131,   133,    94,
      95,    96,    97,    38,    94,    95,    96,    97,    96,    97,
      89,   237,   161,    39,   162,    94,    95,    96,    97,    94,
      95,    96,    97,   163,    40,   156,   157,   158,   160,   226,
      41,    94,    95,    96,    97,    42,    43,   168,   169,   170,
     171,   236,    44,    45,    56,    58,    60,    90,    91,    92,
      93,    46,    47,   136,   134,    98,   100,   102,   104,   106,
     108,   110,   112,   114,   116,   118,   120,   122,   124,   126,
     128,   130,   132,    94,    95,    96,    97,    48,    49,   137,
     135,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,    94,
      95,    96,    97,    50,   228,     0,   229,   164,   165,   166,
     167,    90,    91,    92,    93,    94,    95,    96,    97,   176,
     231,     0,   233,   177,    51,    52,    53,    54,    55,   138,
     139,     1,   235,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,   140,    24,    25,    26,    27,
      28,    90,    91,    92,    93,    29,   141,     0,    30,   178,
      31,    94,    95,    96,    97,    90,    91,    92,    93,   179,
     142,     0,   143,   188,    94,    95,    96,    97,   144,   145,
       0,   146,   189,   230,   147,   232,    90,    91,    92,    93,
     148,   149,     0,   172,   150,   234,    62,    63,     2,     3,
       4,     5,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
     151,    24,    25,   152,    82,    83,    94,    95,    96,    97,
      84,   153,    85,   173,    86,     2,     3,     4,     5,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,   154,    24,    25,
     155,    82,    83,    90,    91,    92,    93,    84,     0,     0,
     174,   159,     2,     3,     4,     5,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     0,    24,    25,     0,    82,    83,
      94,    95,    96,    97,    84,     0,     0,   175,   227,     2,
       3,     4,     5,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,    24,    25,     0,    82,    83,    90,    91,    92,
      93,    84,     0,     0,   180,    94,    95,    96,    97,     0,
       0,     0,   181,    90,    91,    92,    93,     0,     0,     0,
     182,    94,    95,    96,    97,     0,     0,     0,   183,    90,
      91,    92,    93,     0,     0,     0,   184,    94,    95,    96,
      97,     0,     0,     0,   185,    90,    91,    92,    93,     0,
       0,     0,   186,    94,    95,    96,    97,     0,     0,     0,
     187,    90,    91,    92,    93,     0,     0,     0,   190,    94,
      95,    96,    97,     0,     0,     0,   191,    90,    91,    92,
      93,     0,     0,     0,   192,    94,    95,    96,    97,     0,
       0,     0,   193,    90,    91,    92,    93,     0,     0,     0,
     194,    94,    95,    96,    97,     0,     0,     0,   195,    90,
      91,    92,    93,     0,     0,     0,   196,    94,    95,    96,
      97,     0,     0,     0,   197,    90,    91,    92,    93,     0,
       0,     0,   198,    94,    95,    96,    97,     0,     0,     0,
     199,    90,    91,    92,    93,     0,     0,     0,   200,    94,
      95,    96,    97,     0,     0,     0,   201,    90,    91,    92,
      93,     0,     0,     0,   202,    94,    95,    96,    97,     0,
       0,     0,   203,    90,    91,    92,    93,     0,     0,     0,
     204,    94,    95,    96,    97,     0,     0,     0,   205,    90,
      91,    92,    93,     0,     0,     0,   206,    94,    95,    96,
      97,     0,     0,     0,   207,    90,    91,    92,    93,     0,
       0,     0,   238,    94,    95,    96,    97,     0,     0,     0,
     239,    90,    91,    92,    93,     0,     0,     0,   240,    94,
      95,    96,    97,     0,     0,     0,   241,    90,    91,    92,
      93,     0,     0,     0,   242,    94,    95,    96,    97,     0,
       0,     0,   243
};

static const yytype_int16 yycheck[] =
{
      27,    28,    29,    30,    31,    31,    32,    33,    34,    33,
      34,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    31,
      32,    33,    34,    37,    31,    32,    33,    34,    33,    34,
       0,    43,    39,    37,    41,    31,    32,    33,    34,    31,
      32,    33,    34,    39,    37,    82,    83,    84,    85,    41,
      37,    31,    32,    33,    34,    37,    37,    94,    95,    96,
      97,    41,    37,    37,    27,    28,    29,    31,    32,    33,
      34,    37,    37,    41,    38,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    31,    32,    33,    34,    37,    37,    41,
      38,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,    31,
      32,    33,    34,    37,   161,    -1,   163,    90,    91,    92,
      93,    31,    32,    33,    34,    31,    32,    33,    34,    39,
     177,    -1,   179,    39,    37,    37,    37,    37,    37,    37,
      37,     3,   189,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    37,    28,    29,    30,    31,
      32,    31,    32,    33,    34,    37,    37,    -1,    40,    39,
      42,    31,    32,    33,    34,    31,    32,    33,    34,    39,
      37,    -1,    37,    39,    31,    32,    33,    34,    37,    37,
      -1,    37,    39,   176,    37,   178,    31,    32,    33,    34,
      37,    37,    -1,    38,    37,   188,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      37,    28,    29,    37,    31,    32,    31,    32,    33,    34,
      37,    37,    39,    38,    41,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    37,    28,    29,
      37,    31,    32,    31,    32,    33,    34,    37,    -1,    -1,
      38,    41,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    -1,    31,    32,
      31,    32,    33,    34,    37,    -1,    -1,    38,    41,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    -1,    31,    32,    31,    32,    33,
      34,    37,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    31,    32,    33,    34,    -1,    -1,    -1,    38,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    31,    32,    33,    34,    -1,
      -1,    -1,    38
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    28,    29,    30,    31,    32,    37,
      40,    42,    45,    46,    47,    48,    49,    50,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    46,    49,    46,    49,
      46,    49,     3,     4,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    31,    32,    37,    39,    41,    49,    49,     0,
      31,    32,    33,    34,    31,    32,    33,    34,    46,    49,
      46,    49,    46,    49,    46,    49,    46,    49,    46,    49,
      46,    49,    46,    49,    46,    49,    46,    49,    46,    49,
      46,    49,    46,    49,    46,    49,    46,    49,    46,    49,
      46,    49,    46,    49,    38,    38,    41,    41,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    49,    49,    49,    41,
      49,    39,    41,    39,    46,    46,    46,    46,    49,    49,
      49,    49,    38,    38,    38,    38,    39,    39,    39,    39,
      38,    38,    38,    38,    38,    38,    38,    38,    39,    39,
      38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,    38,    38,    38,    38,    38,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    41,    41,    49,    49,
      46,    49,    46,    49,    46,    49,    41,    43,    38,    38,
      38,    38,    38,    38
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    44,    45,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     3,     3,     3,     3,     2,     2,
       1,     3,     4,     4,     4,     6,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     6,     4,     6,
       1,     1,     3,     5,     3,     2,     3,     3,     4,     4,
       1,     5,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     2,     2,     1,     3,     4,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     6,     4,     6
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
    case 49: /* expression  */
#line 122 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1433 "y.tab.c"
        break;

    case 50: /* function_call  */
#line 122 "gaol_interval_parser.ypp"
            { gaol_release(((*yyvaluep).expr)); }
#line 1439 "y.tab.c"
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
#line 133 "gaol_interval_parser.ypp"
                                                { *gaol_result_of_parsing = interval((yyvsp[0].itv).l,(yyvsp[0].itv).r); }
#line 1709 "y.tab.c"
    break;

  case 3:
#line 137 "gaol_interval_parser.ypp"
                                                                { (yyval.itv) = (yyvsp[0].itv); }
#line 1715 "y.tab.c"
    break;

  case 4:
#line 138 "gaol_interval_parser.ypp"
                                                { gaol_tmp_itv=interval((yyvsp[-2].itv).l,(yyvsp[-2].itv).r)+interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
						  					(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1722 "y.tab.c"
    break;

  case 5:
#line 140 "gaol_interval_parser.ypp"
                                                { gaol_tmp_itv=interval((yyvsp[-2].itv).l,(yyvsp[-2].itv).r)-interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
						  					(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1729 "y.tab.c"
    break;

  case 6:
#line 142 "gaol_interval_parser.ypp"
                                                { gaol_tmp_itv=interval((yyvsp[-2].itv).l,(yyvsp[-2].itv).r)*interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
						  					(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1736 "y.tab.c"
    break;

  case 7:
#line 144 "gaol_interval_parser.ypp"
                                                { gaol_tmp_itv=interval((yyvsp[-2].itv).l,(yyvsp[-2].itv).r)/interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
						  					(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1743 "y.tab.c"
    break;

  case 8:
#line 146 "gaol_interval_parser.ypp"
                                                { gaol_tmp_itv= -interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
						  					(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1750 "y.tab.c"
    break;

  case 9:
#line 148 "gaol_interval_parser.ypp"
                                                { (yyval.itv) = (yyvsp[0].itv); }
#line 1756 "y.tab.c"
    break;

  case 10:
#line 149 "gaol_interval_parser.ypp"
                                                                { (yyval.itv) = (yyvsp[0].itv); }
#line 1762 "y.tab.c"
    break;

  case 11:
#line 150 "gaol_interval_parser.ypp"
                                                        { (yyval.itv) = (yyvsp[-1].itv); }
#line 1768 "y.tab.c"
    break;

  case 12:
#line 155 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=cos(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1775 "y.tab.c"
    break;

  case 13:
#line 157 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=sin(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1782 "y.tab.c"
    break;

  case 14:
#line 159 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=tan(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1789 "y.tab.c"
    break;

  case 15:
#line 161 "gaol_interval_parser.ypp"
                                                                { gaol_tmp_itv=atan2(interval((yyvsp[-3].itv).l,(yyvsp[-3].itv).r),
							          						interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						    								(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1797 "y.tab.c"
    break;

  case 16:
#line 164 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=acos(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1804 "y.tab.c"
    break;

  case 17:
#line 166 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=asin(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1811 "y.tab.c"
    break;

  case 18:
#line 168 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=atan(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1818 "y.tab.c"
    break;

  case 19:
#line 170 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=cosh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1825 "y.tab.c"
    break;

  case 20:
#line 172 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=sinh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1832 "y.tab.c"
    break;

  case 21:
#line 174 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=tanh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1839 "y.tab.c"
    break;

  case 22:
#line 176 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=acosh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1846 "y.tab.c"
    break;

  case 23:
#line 178 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=asinh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1853 "y.tab.c"
    break;

  case 24:
#line 180 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=atanh(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1860 "y.tab.c"
    break;

  case 25:
#line 182 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=exp(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1867 "y.tab.c"
    break;

  case 26:
#line 184 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=log(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1874 "y.tab.c"
    break;

  case 27:
#line 186 "gaol_interval_parser.ypp"
                                                                { gaol_tmp_itv=
						   									pow(interval((yyvsp[-3].itv).l,(yyvsp[-3].itv).r),interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						   									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1882 "y.tab.c"
    break;

  case 28:
#line 189 "gaol_interval_parser.ypp"
                                                                                { gaol_tmp_itv=sqrt(interval((yyvsp[-1].itv).l,(yyvsp[-1].itv).r));
						  									(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
#line 1889 "y.tab.c"
    break;

  case 29:
#line 191 "gaol_interval_parser.ypp"
                                                             { interval tmp((yyvsp[-1].itv).l,(yyvsp[-1].itv).r);
														   if (!tmp.is_an_int()) {
					                                       		// not an int?
																(yyval.itv).l=1; (yyval.itv).r=-1;
																gaol_ERROR(invalid_action_error,
																	"nth_root used with non integral 2nd arg.");
														    } else {
					     										gaol_tmp_itv=nth_root(interval((yyvsp[-3].itv).l,(yyvsp[-3].itv).r),static_cast<unsigned int>((yyvsp[-1].itv).l));
						       										(yyval.itv).l=gaol_tmp_itv.left(); (yyval.itv).r=gaol_tmp_itv.right(); }
														    }
#line 1904 "y.tab.c"
    break;

  case 30:
#line 204 "gaol_interval_parser.ypp"
                                                                { 	const expression e(*(yyvsp[0].expr));
											gaol_release((yyvsp[0].expr));
											gaol_global_parsing_flag = evaluate_left_right(e,
																gaol_result_of_parsing);
							  				(yyval.itv).l=gaol_result_of_parsing->left();
					          				(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1916 "y.tab.c"
    break;

  case 31:
#line 212 "gaol_interval_parser.ypp"
                                                                        { 	*gaol_result_of_parsing = interval::emptyset();
					  						gaol_global_parsing_flag = true;
											(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1926 "y.tab.c"
    break;

  case 32:
#line 218 "gaol_interval_parser.ypp"
                                                        {	const expression e(*(yyvsp[-1].expr));
											gaol_release((yyvsp[-1].expr));
											gaol_global_parsing_flag = evaluate_left_right(e,
																gaol_result_of_parsing);
					  						if (gaol_literal_infinity(e.get_root()) != 0) {
					  							*gaol_result_of_parsing = interval::emptyset();
					  						}
											(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1941 "y.tab.c"
    break;

  case 33:
#line 228 "gaol_interval_parser.ypp"
                                                { 	const expression el(*(yyvsp[-3].expr)), er(*(yyvsp[-1].expr));
											gaol_release((yyvsp[-3].expr));
											gaol_release((yyvsp[-1].expr));
											gaol_global_parsing_flag = evaluate_left_right(el, er,
																			gaol_result_of_parsing);
					  						if (gaol_literal_infinity(el.get_root()) > 0 || gaol_literal_infinity(er.get_root()) < 0) {
					  							*gaol_result_of_parsing = interval::emptyset();
					  						}
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1957 "y.tab.c"
    break;

  case 34:
#line 239 "gaol_interval_parser.ypp"
                                                                { 	*gaol_result_of_parsing = interval::emptyset();
					  						gaol_global_parsing_flag = true;
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1967 "y.tab.c"
    break;

  case 35:
#line 247 "gaol_interval_parser.ypp"
                                                                        { 	*gaol_result_of_parsing = interval::emptyset();
					  						gaol_global_parsing_flag = true;
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1977 "y.tab.c"
    break;

  case 36:
#line 252 "gaol_interval_parser.ypp"
                                                        { 	*gaol_result_of_parsing = interval::universe();
					  						gaol_global_parsing_flag = true;
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1987 "y.tab.c"
    break;

  case 37:
#line 257 "gaol_interval_parser.ypp"
                                                                { 	*gaol_result_of_parsing = interval::universe();
					  						gaol_global_parsing_flag = true;
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 1997 "y.tab.c"
    break;

  case 38:
#line 262 "gaol_interval_parser.ypp"
                                                        { 	const expression e(*(yyvsp[-2].expr));
											gaol_release((yyvsp[-2].expr));
											gaol_global_parsing_flag = evaluate_left_right(e,
																gaol_result_of_parsing);
					  						*gaol_result_of_parsing = (gaol_literal_infinity(e.get_root()) > 0) ? interval::emptyset()
					  							: interval(gaol_result_of_parsing->left(), GAOL_INFINITY);
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 2011 "y.tab.c"
    break;

  case 39:
#line 271 "gaol_interval_parser.ypp"
                                                        { 	const expression e(*(yyvsp[-1].expr));
											gaol_release((yyvsp[-1].expr));
											gaol_global_parsing_flag = evaluate_left_right(e,
																gaol_result_of_parsing);
					  						*gaol_result_of_parsing = (gaol_literal_infinity(e.get_root()) < 0) ? interval::emptyset()
					  							: interval(-GAOL_INFINITY, gaol_result_of_parsing->right());
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 2025 "y.tab.c"
    break;

  case 40:
#line 280 "gaol_interval_parser.ypp"
                                                                { 	*gaol_result_of_parsing = interval((yyvsp[0].itv).l,(yyvsp[0].itv).r);
					  						gaol_global_parsing_flag = true;
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
										}
#line 2035 "y.tab.c"
    break;

  case 41:
#line 285 "gaol_interval_parser.ypp"
                                            { 	const expression el(*(yyvsp[-3].expr)), er(*(yyvsp[-1].expr));
											gaol_release((yyvsp[-3].expr));
											gaol_release((yyvsp[-1].expr));
											gaol_global_parsing_flag = evaluate_left_right(el, er,
																							gaol_result_of_parsing);
                                          	(yyval.itv).l=gaol_result_of_parsing->left();
					  						(yyval.itv).r=gaol_result_of_parsing->right();
											if ((yyval.itv).l != (yyval.itv).r) {
												gaol_ERROR(input_format_error, std::string("bounds of degenerate interval do not evaluate to the same value"));
											}
										}
#line 2051 "y.tab.c"
    break;

  case 42:
#line 299 "gaol_interval_parser.ypp"
                                                        { 	(yyval.expr) = new double_node((yyvsp[0].d));
					  				(yyval.expr)->inc_refcount(); }
#line 2058 "y.tab.c"
    break;

  case 43:
#line 301 "gaol_interval_parser.ypp"
                                                        { 	(yyval.expr) = new double_node(std::numeric_limits<double>::min());
					  				(yyval.expr)->inc_refcount(); }
#line 2065 "y.tab.c"
    break;

  case 44:
#line 303 "gaol_interval_parser.ypp"
                                                        { 	(yyval.expr) = new double_node(std::numeric_limits<double>::max());
					  				(yyval.expr)->inc_refcount(); }
#line 2072 "y.tab.c"
    break;

  case 45:
#line 305 "gaol_interval_parser.ypp"
                                                        { 	(yyval.expr) = new interval_node(interval::pi());
	                                (yyval.expr)->inc_refcount(); }
#line 2079 "y.tab.c"
    break;

  case 46:
#line 307 "gaol_interval_parser.ypp"
                                    { 	(yyval.expr) = new interval_node(interval((yyvsp[0].itv).l,(yyvsp[0].itv).r));
					  				(yyval.expr)->inc_refcount(); }
#line 2086 "y.tab.c"
    break;

  case 47:
#line 309 "gaol_interval_parser.ypp"
                                                { 	(yyval.expr) = new double_node(GAOL_INFINITY);
					  				(yyval.expr)->inc_refcount(); }
#line 2093 "y.tab.c"
    break;

  case 48:
#line 311 "gaol_interval_parser.ypp"
                                        { 	(yyval.expr) = new add_node(*(yyvsp[-2].expr),*(yyvsp[0].expr));
					  				(yyval.expr)->inc_refcount();
					  				gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 2101 "y.tab.c"
    break;

  case 49:
#line 314 "gaol_interval_parser.ypp"
                                        { 	(yyval.expr) = new sub_node(*(yyvsp[-2].expr),*(yyvsp[0].expr));
					  				(yyval.expr)->inc_refcount();
					  				gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 2109 "y.tab.c"
    break;

  case 50:
#line 317 "gaol_interval_parser.ypp"
                                    { 	(yyval.expr) = new mult_node(*(yyvsp[-2].expr),*(yyvsp[0].expr));
					  				(yyval.expr)->inc_refcount();
					  				gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 2117 "y.tab.c"
    break;

  case 51:
#line 320 "gaol_interval_parser.ypp"
                                    { 	(yyval.expr) = new div_node(*(yyvsp[-2].expr),*(yyvsp[0].expr));
					  				(yyval.expr)->inc_refcount();
					  				gaol_release((yyvsp[-2].expr)); gaol_release((yyvsp[0].expr)); }
#line 2125 "y.tab.c"
    break;

  case 52:
#line 323 "gaol_interval_parser.ypp"
                                      { (yyval.expr) = new unary_minus_node(*(yyvsp[0].expr));
					  				(yyval.expr)->inc_refcount();
					  				gaol_release((yyvsp[0].expr)); }
#line 2133 "y.tab.c"
    break;

  case 53:
#line 326 "gaol_interval_parser.ypp"
                                      { (yyval.expr) = (yyvsp[0].expr); }
#line 2139 "y.tab.c"
    break;

  case 54:
#line 327 "gaol_interval_parser.ypp"
                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 2145 "y.tab.c"
    break;

  case 55:
#line 328 "gaol_interval_parser.ypp"
                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 2151 "y.tab.c"
    break;

  case 56:
#line 332 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new cos_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2159 "y.tab.c"
    break;

  case 57:
#line 335 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new sin_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2167 "y.tab.c"
    break;

  case 58:
#line 338 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new tan_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2175 "y.tab.c"
    break;

  case 59:
#line 341 "gaol_interval_parser.ypp"
                                                        { 	(yyval.expr) = new atan2_node(*(yyvsp[-3].expr), *(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-3].expr)); gaol_release((yyvsp[-1].expr)); }
#line 2183 "y.tab.c"
    break;

  case 60:
#line 344 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new acos_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2191 "y.tab.c"
    break;

  case 61:
#line 347 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new asin_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2199 "y.tab.c"
    break;

  case 62:
#line 350 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new atan_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2207 "y.tab.c"
    break;

  case 63:
#line 353 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new cosh_node(*(yyvsp[-1].expr));
														(yyval.expr)->inc_refcount();
														gaol_release((yyvsp[-1].expr)); }
#line 2215 "y.tab.c"
    break;

  case 64:
#line 356 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new sinh_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2223 "y.tab.c"
    break;

  case 65:
#line 359 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new tanh_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2231 "y.tab.c"
    break;

  case 66:
#line 362 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new acosh_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2239 "y.tab.c"
    break;

  case 67:
#line 365 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new asinh_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2247 "y.tab.c"
    break;

  case 68:
#line 368 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new atanh_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2255 "y.tab.c"
    break;

  case 69:
#line 371 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new exp_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2263 "y.tab.c"
    break;

  case 70:
#line 374 "gaol_interval_parser.ypp"
                                                                        { 	(yyval.expr) = new log_node(*(yyvsp[-1].expr));
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr)); }
#line 2271 "y.tab.c"
    break;

  case 71:
#line 377 "gaol_interval_parser.ypp"
                                                        { 	interval tmp;
														bool aborted = false;
					  									gaol_global_parsing_flag =
					    									gaol_evaluate_exponent((yyvsp[-1].expr),tmp,aborted);
														if (aborted) {
															gaol_release((yyvsp[-3].expr));
															gaol_release((yyvsp[-1].expr));
															YYABORT;
														}
														if (!gaol_global_parsing_flag) { // Error?
					     									(yyval.expr) = the_null_expr;
					     									(yyval.expr)->inc_refcount();
                                          				} else {
					     									if (!tmp.is_an_int()) {
					        									// not an int in disguise?
																(yyval.expr) = new pow_itv_node(*(yyvsp[-3].expr),*(yyvsp[-1].expr));
																(yyval.expr)->inc_refcount();
       					     								} else {
					        									if (tmp.left() > 0) {
					           										(yyval.expr) = new pow_node(*(yyvsp[-3].expr),static_cast<int>(tmp.left()));
	 				           										(yyval.expr)->inc_refcount();
                                               					} else {
						  											expr_node *one = new double_node(1.0);
					          										one->inc_refcount();
						  											if (tmp.left() < 0) {
					           											// translating x^(-a) into 1/(x^a)
						     											expr_node *e = new pow_node(*(yyvsp[-3].expr),
																							static_cast<int>(-tmp.left()));
						     											e->inc_refcount();
						     											(yyval.expr) = new div_node(*one,*e);
						     											(yyval.expr)->inc_refcount();
						     											gaol_release(one);
						     											gaol_release(e);
																	} else {  // tmp == 0.0
						     											(yyval.expr) = one;
						 											}
                                                				}
					     									}
                                          				}
														gaol_release((yyvsp[-3].expr));
														gaol_release((yyvsp[-1].expr));
                                        			}
#line 2318 "y.tab.c"
    break;

  case 72:
#line 419 "gaol_interval_parser.ypp"
                                                                        {	(yyval.expr) = new nth_root_node(*(yyvsp[-1].expr),2);
					  									(yyval.expr)->inc_refcount();
					  									gaol_release((yyvsp[-1].expr));
													}
#line 2327 "y.tab.c"
    break;

  case 73:
#line 423 "gaol_interval_parser.ypp"
                                                         { 	interval tmp;
														bool aborted = false;
					  									gaol_global_parsing_flag =
					    									gaol_evaluate_exponent((yyvsp[-1].expr),tmp,aborted);
														if (aborted) {
															gaol_release((yyvsp[-3].expr));
															gaol_release((yyvsp[-1].expr));
															YYABORT;
														}
														if (!tmp.is_an_int() || !gaol_global_parsing_flag) {
					     									// not an int?
															(yyval.expr) = the_null_expr;
															(yyval.expr)->inc_refcount();
       					  								} else {
					     									(yyval.expr) = new nth_root_node(*(yyvsp[-3].expr),static_cast<unsigned int>(tmp.left()));
					     									(yyval.expr)->inc_refcount();
					 									}
														gaol_release((yyvsp[-3].expr));
														gaol_release((yyvsp[-1].expr));
													  }
#line 2352 "y.tab.c"
    break;


#line 2356 "y.tab.c"

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
#line 445 "gaol_interval_parser.ypp"


int gaol_error_bison(const char *s ...)
{
  gaol_global_parsing_flag = false;
  return 0; // An error occurred
}
