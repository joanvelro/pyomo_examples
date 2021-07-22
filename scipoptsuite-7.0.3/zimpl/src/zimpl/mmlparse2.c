/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "src/zimpl/mmlparse2.y" /* yacc.c:339  */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mmlparse2.y                                                   */
/*   Name....: MML Parser                                                    */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2020 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#pragma clang diagnostic ignored "-Wunreachable-code"

#pragma GCC   diagnostic ignored "-Wstrict-prototypes"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "zimpl/lint.h"
#include "zimpl/mshell.h"
#include "zimpl/ratlptypes.h"
#include "zimpl/numb.h"
#include "zimpl/elem.h"
#include "zimpl/tuple.h"
#include "zimpl/mme.h"
#include "zimpl/set.h"
#include "zimpl/symbol.h"
#include "zimpl/entry.h"
#include "zimpl/idxset.h"
#include "zimpl/rdefpar.h"
#include "zimpl/bound.h"
#include "zimpl/define.h"
#include "zimpl/mono.h"
#include "zimpl/term.h"
#include "zimpl/list.h"
#include "zimpl/stmt.h"
#include "zimpl/local.h"
#include "zimpl/code.h"
#include "zimpl/inst.h"
        
#define YYERROR_VERBOSE 1

/* the function is actually getting a YYSTYPE* as argument, but the
 * type isn't available here, so it is decalred to accept any number of
 * arguments, i.e. yylex() and not yylex(void).
 */
extern int yylex();

/*lint -sem(yyerror, 1p, r_no) */ 
extern void yyerror(const char* s) is_NORETURN;
 

#line 143 "src/zimpl/mmlparse2.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "mmlparse2.h".  */
#ifndef YY_YY_SRC_ZIMPL_MMLPARSE2_H_INCLUDED
# define YY_YY_SRC_ZIMPL_MMLPARSE2_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DECLSET = 258,
    DECLPAR = 259,
    DECLVAR = 260,
    DECLMIN = 261,
    DECLMAX = 262,
    DECLSUB = 263,
    DECLSOS = 264,
    DEFNUMB = 265,
    DEFSTRG = 266,
    DEFBOOL = 267,
    DEFSET = 268,
    PRINT = 269,
    CHECK = 270,
    BINARY = 271,
    INTEGER = 272,
    REAL = 273,
    IMPLICIT = 274,
    ASGN = 275,
    DO = 276,
    WITH = 277,
    IN = 278,
    TO = 279,
    UNTIL = 280,
    BY = 281,
    FORALL = 282,
    EXISTS = 283,
    PRIORITY = 284,
    STARTVAL = 285,
    DEFAULT = 286,
    CMP_LE = 287,
    CMP_GE = 288,
    CMP_EQ = 289,
    CMP_LT = 290,
    CMP_GT = 291,
    CMP_NE = 292,
    INFTY = 293,
    AND = 294,
    OR = 295,
    XOR = 296,
    NOT = 297,
    SUM = 298,
    MIN = 299,
    MAX = 300,
    ARGMIN = 301,
    ARGMAX = 302,
    PROD = 303,
    IF = 304,
    THEN = 305,
    ELSE = 306,
    END = 307,
    INTER = 308,
    UNION = 309,
    CROSS = 310,
    SYMDIFF = 311,
    WITHOUT = 312,
    PROJ = 313,
    MOD = 314,
    DIV = 315,
    POW = 316,
    FAC = 317,
    CARD = 318,
    ROUND = 319,
    FLOOR = 320,
    CEIL = 321,
    RANDOM = 322,
    ORD = 323,
    ABS = 324,
    SGN = 325,
    LOG = 326,
    LN = 327,
    EXP = 328,
    SQRT = 329,
    SIN = 330,
    COS = 331,
    TAN = 332,
    ASIN = 333,
    ACOS = 334,
    ATAN = 335,
    POWER = 336,
    SGNPOW = 337,
    READ = 338,
    AS = 339,
    SKIP = 340,
    USE = 341,
    COMMENT = 342,
    MATCH = 343,
    SUBSETS = 344,
    INDEXSET = 345,
    POWERSET = 346,
    VIF = 347,
    VABS = 348,
    TYPE1 = 349,
    TYPE2 = 350,
    LENGTH = 351,
    SUBSTR = 352,
    NUMBSYM = 353,
    STRGSYM = 354,
    VARSYM = 355,
    SETSYM = 356,
    NUMBDEF = 357,
    STRGDEF = 358,
    BOOLDEF = 359,
    SETDEF = 360,
    DEFNAME = 361,
    NAME = 362,
    STRG = 363,
    NUMB = 364,
    SCALE = 365,
    SEPARATE = 366,
    CHECKONLY = 367,
    INDICATOR = 368
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 81 "src/zimpl/mmlparse2.y" /* yacc.c:355  */

   unsigned int bits;
   Numb*        numb;
   const char*  strg;
   const char*  name;
   Symbol*      sym;
   Define*      def;
   CodeNode*    code;

#line 307 "src/zimpl/mmlparse2.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void);

#endif /* !YY_YY_SRC_ZIMPL_MMLPARSE2_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 323 "src/zimpl/mmlparse2.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3473

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  126
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  309
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  925

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   368

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     121,   122,   116,   114,   120,   115,     2,   123,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   117,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   118,     2,   119,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   124,     2,   125,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   150,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   168,   175,   181,   187,   197,   198,   201,
     204,   207,   213,   222,   231,   240,   249,   258,   261,   271,
     274,   277,   280,   287,   290,   291,   298,   299,   307,   314,
     323,   333,   344,   353,   363,   367,   377,   378,   379,   383,
     386,   387,   388,   393,   401,   402,   403,   404,   409,   417,
     418,   422,   423,   431,   432,   435,   436,   440,   444,   448,
     451,   463,   466,   476,   482,   485,   488,   493,   498,   506,
     509,   514,   519,   526,   530,   535,   539,   545,   548,   553,
     558,   563,   567,   574,   581,   587,   593,   599,   604,   612,
     621,   630,   638,   649,   652,   656,   661,   669,   670,   673,
     676,   677,   680,   683,   684,   687,   690,   691,   694,   697,
     698,   701,   704,   705,   708,   711,   712,   713,   714,   715,
     719,   720,   724,   725,   726,   727,   731,   732,   733,   737,
     738,   739,   740,   741,   744,   745,   753,   754,   755,   759,
     760,   764,   765,   766,   772,   773,   776,   782,   785,   786,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   798,
     801,   804,   812,   818,   821,   827,   828,   829,   837,   841,
     842,   843,   844,   845,   846,   847,   857,   858,   865,   868,
     874,   875,   876,   879,   880,   883,   884,   887,   888,   892,
     893,   894,   897,   901,   904,   909,   910,   913,   916,   919,
     922,   925,   928,   931,   934,   935,   936,   937,   938,   939,
     940,   943,   946,   952,   953,   957,   958,   959,   960,   964,
     967,   970,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     992,   993,   998,  1004,  1005,  1009,  1012,  1018,  1021,  1027,
    1028,  1029,  1033,  1034,  1035,  1036,  1037,  1038,  1044,  1045,
    1046,  1050,  1051,  1052,  1055,  1058,  1061,  1064,  1070,  1071,
    1072,  1075,  1078,  1081,  1086,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1109,  1110,  1111,  1114,  1117,  1120,  1123,  1126
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DECLSET", "DECLPAR", "DECLVAR",
  "DECLMIN", "DECLMAX", "DECLSUB", "DECLSOS", "DEFNUMB", "DEFSTRG",
  "DEFBOOL", "DEFSET", "PRINT", "CHECK", "BINARY", "INTEGER", "REAL",
  "IMPLICIT", "ASGN", "DO", "WITH", "IN", "TO", "UNTIL", "BY", "FORALL",
  "EXISTS", "PRIORITY", "STARTVAL", "DEFAULT", "CMP_LE", "CMP_GE",
  "CMP_EQ", "CMP_LT", "CMP_GT", "CMP_NE", "INFTY", "AND", "OR", "XOR",
  "NOT", "SUM", "MIN", "MAX", "ARGMIN", "ARGMAX", "PROD", "IF", "THEN",
  "ELSE", "END", "INTER", "UNION", "CROSS", "SYMDIFF", "WITHOUT", "PROJ",
  "MOD", "DIV", "POW", "FAC", "CARD", "ROUND", "FLOOR", "CEIL", "RANDOM",
  "ORD", "ABS", "SGN", "LOG", "LN", "EXP", "SQRT", "SIN", "COS", "TAN",
  "ASIN", "ACOS", "ATAN", "POWER", "SGNPOW", "READ", "AS", "SKIP", "USE",
  "COMMENT", "MATCH", "SUBSETS", "INDEXSET", "POWERSET", "VIF", "VABS",
  "TYPE1", "TYPE2", "LENGTH", "SUBSTR", "NUMBSYM", "STRGSYM", "VARSYM",
  "SETSYM", "NUMBDEF", "STRGDEF", "BOOLDEF", "SETDEF", "DEFNAME", "NAME",
  "STRG", "NUMB", "SCALE", "SEPARATE", "CHECKONLY", "INDICATOR", "'+'",
  "'-'", "'*'", "';'", "'['", "']'", "','", "'('", "')'", "'/'", "'{'",
  "'}'", "$accept", "stmt", "decl_set", "set_entry_list", "set_entry",
  "def_numb", "def_strg", "def_bool", "def_set", "name_list", "decl_par",
  "par_singleton", "par_default", "decl_var", "var_type", "lower", "upper",
  "priority", "startval", "cexpr_entry_list", "cexpr_entry", "matrix_head",
  "matrix_body", "decl_obj", "decl_sub", "constraint_list", "constraint",
  "vbool", "con_attr_list", "con_attr", "con_type", "vexpr", "vproduct",
  "vfactor", "vexpo", "vval", "decl_sos", "soset", "sos_type", "exec_do",
  "command", "idxset", "pure_idxset", "sexpr", "sunion", "sproduct",
  "sval", "read", "read_par", "tuple_list", "lexpr", "tuple", "symidx",
  "cexpr_list", "cexpr", "cproduct", "cfactor", "cexpo", "cval", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,    43,    45,    42,    59,    91,    93,
      44,    40,    41,    47,   123,   125
};
# endif

#define YYPACT_NINF -534

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-534)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1124,   -84,   -70,   -52,   -44,   -26,    -5,    20,   -13,    33,
      37,    45,   239,   162,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,   -12,     2,    26,   147,   155,
     159,   192,    97,   105,   131,   179,  1726,  1833,   865,   163,
    -534,   481,   875,  1623,   865,   318,   349,  -534,   418,   865,
     349,  2836,  2836,  1144,    35,   252,   252,   252,   252,   304,
    2158,  1833,   865,    19,    29,  1002,  1214,   865,  1833,   865,
     865,   339,   345,   375,   382,   392,   427,   440,   444,   516,
     519,   528,   532,   536,   539,   554,   557,   560,   569,   575,
     579,   600,   607,   480,   480,  -534,   480,   625,   639,   642,
     645,  -534,  -534,  -534,  3352,  3352,  1833,  1925,   441,  -534,
      54,  -534,   901,   421,   493,    50,   313,  -534,  -534,   476,
     441,   901,   421,    50,  1833,   481,   710,  -534,   947,   753,
    -534,   510,   772,   665,  3026,  1833,  3026,  3026,   688,   693,
    -534,   797,  1339,  3026,   437,   705,  3026,   810,  3094,   795,
     730,  -534,   763,   795,   865,  1833,   756,   776,   780,   783,
     792,   815,   837,   843,   869,   870,   881,   890,   480,  2897,
    2897,  2836,   489,   -27,  -534,  -534,   809,   549,   424,   898,
     865,  1833,  2700,     7,  -534,   145,   278,   865,  -534,  -534,
     921,   318,  -534,   148,   198,   223,   235,   865,  -534,    -6,
     437,  -534,  1019,  2008,  1029,  2008,  1031,  2632,  1045,  2632,
    1070,  1074,   310,  1075,  1081,   481,   481,  3026,  3026,  3026,
    3026,   481,  3026,  3026,  3026,  3026,  3026,  3026,  3026,  3026,
    3026,  3026,  3026,  3026,  1023,  3026,  3026,  3026,  -534,  -534,
    -534,  3026,  3026,  3026,  3026,  -534,  -534,   435,   118,   199,
    1833,  2632,  -534,    -9,  1339,   -59,   753,   -17,    23,   481,
     481,   481,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   648,   648,  1833,  1833,  1833,   481,  3026,  3026,  3026,
    3026,  3026,  3026,  3026,  3026,  3026,  3291,  3291,  3291,  3291,
    3291,  -534,   969,   524,   239,   481,  -534,   -14,  1047,     6,
     989,   114,   514,  -534,  1106,  3026,   797,  3026,  3026,  3026,
    3026,  -534,   437,  1086,   437,  3026,  1027,  1833,  2429,   437,
    2226,   318,  -534,  1452,  1036,  1134,   992,  2836,  2836,  2836,
    2836,  2836,  2836,  2836,  2836,  2836,  2836,  2836,  2836,  -534,
    1362,  1362,  -534,  -534,   610,   675,  2836,  2836,  -534,  2958,
    3291,  3291,  2836,  2836,  2958,  -534,  1135,   995,  2700,  2700,
    1018,   344,   387,  2768,  -534,  -534,  -534,  -534,  2836,  2836,
    1141,  -534,  1149,  1078,  1166,  1170,  1174,  1185,  1082,  -534,
    3026,  1084,   277,  3291,  1107,   463,  3291,   812,  3026,   859,
    3026,  3291,  1833,   648,   612,   332,   666,   873,   979,  1003,
      89,   630,  1028,  1032,  1037,  1043,  1046,  1113,  1116,  1140,
    1142,  1155,  1159,  1164,  1110,  1184,   365,   623,   498,   583,
     617,   896,  -534,  -534,  -534,  1024,  2090,  -534,  1106,  -534,
    -534,  3026,  3026,   947,   947,   947,   947,   947,   947,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  1194,  1194,
     947,   437,   437,   437,   437,   437,   437,   437,   313,   313,
    -534,  -534,  -534,  -534,  -534,   481,  -534,    38,  1127,  1129,
       5,  -534,   481,   561,  -534,  3026,  3026,  -534,    11,  3026,
     437,   437,   437,   437,  1528,   437,  -534,  1059,  -534,  -534,
    1833,   437,   810,   318,   349,   779,   349,  -534,  2836,  2836,
    1186,  1188,  1198,  1202,  1211,  1217,  1222,  1226,  1228,  1231,
    1237,  1249,  1251,  1253,  1255,  1286,  1288,  1292,   473,   747,
    1298,   865,  1833,  1145,  1150,  1154,  1161,  1190,  1193,  1197,
    1200,  1201,  2836,  -534,   -27,   424,   -27,   424,  -534,  -534,
    -534,  -534,   -27,   424,   -27,   424,  -534,  1144,  1144,  -534,
     276,   253,   269,  2700,  2700,  2700,  2836,  2836,  2836,  2836,
    2836,  2836,  2836,  2836,  2836,  2836,  2836,  2836,  2836,  -534,
     773,   549,    40,   250,    35,  2836,  -534,  3026,  3026,  1833,
     481,  -534,   313,  -534,  -534,  -534,  -534,  -534,  -534,   865,
     437,   865,   437,  -534,   990,   966,   302,  -534,    54,  1106,
    -534,  -534,  -534,  -534,  3026,  3026,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    3026,  -534,  -534,  -534,  -534,  -534,  2632,  1109,   -56,  -534,
     -10,     3,   758,  1833,   481,   481,  -534,  1106,   947,   107,
     714,    46,   437,  -534,    27,  3026,    22,   999,  3162,  1071,
    1122,   810,   795,  1128,   795,   -27,   424,   256,   323,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  3026,  3026,
    -534,  1284,  1099,  2836,  2836,  2836,  2836,  2836,  2836,  2836,
    2836,  2836,  1268,   289,   216,   394,  -534,  -534,  1270,  1270,
     145,   278,   773,   549,   773,   549,   773,   549,   773,   549,
     773,   549,   773,   549,   773,   773,   773,   773,   773,   773,
    1156,  1156,  3026,  3026,  1156,  3026,  3026,  1156,  -534,   773,
    1005,  1052,    93,   729,  1302,  1306,   481,  1833,  3026,  1206,
    1301,   817,   847,  -534,  -534,  3026,  -534,  3026,  -534,   901,
     769,   764,  -534,  -534,  -534,  -534,  1349,  3026,  1230,  -534,
    2496,   400,  2294,  -534,  1232,   318,  -534,  1238,  2836,  1307,
    1327,  2836,  2836,  1144,  -534,  2836,  2836,  1368,   437,   437,
     437,   437,  -534,  -534,  -534,  -534,  3026,  3026,   842,   492,
     477,  -534,  -534,  3026,  3026,   316,   385,  3026,  -534,  -534,
     437,  -534,  1321,  3230,  1325,   431,  -534,   810,  -534,   507,
    -534,  -534,   424,    13,   502,   593,   620,   663,  -534,  -534,
    -534,  -534,  -534,  1156,  1156,  1156,  1156,   437,   437,  -534,
    -534,  -534,  1331,  1343,  -534,  -534,   342,  3026,  2563,  3026,
    2362,  1240,  -534,  -534,  2836,  -534,  2836,  -534,  2836,  -534,
    2836,  -534,  -534,  -534,  3026,  -534,   520,  1328,   527,  1337,
    -534,   145,   278,  1156,   145,   278,  1156,   145,   278,  1156,
     145,   278,  1156,  1345,  -534,  -534,  -534,  -534,  2836,  2836,
    2836,  2836,  2836,  2836,  2836,  2836,  -534,   559,   574,   595,
     660,   727,   749,   751,   778,   793,   824,   854,   857,   863,
     868,   878,   883,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  1156,
    1156,  1156,  1156,  1156,  1156,  1156,  1156,  1156,  1156,  1156,
    1156,  1156,  1156,  1156,  1156
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     8,     9,    10,    11,     3,
       4,     5,     6,     7,    12,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    59,    49,    47,     0,     0,
      49,     0,     0,     0,   175,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   255,   255,   183,   255,     0,     0,     0,
       0,   280,   279,   278,     0,     0,     0,     0,   181,   190,
     197,   199,   182,   180,   179,   257,   259,   262,   268,   271,
       0,   184,     0,     0,     0,     0,     0,   186,   187,     0,
     178,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      63,     0,    65,     0,    35,     0,     0,    61,     0,    54,
       0,    48,     0,    54,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,     0,
       0,     0,     0,   139,   146,   151,   154,     0,   259,     0,
       0,     0,     0,     0,    74,     0,     0,     0,   176,   177,
       0,    59,    27,     0,     0,     0,     0,     0,   253,     0,
     257,   247,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   281,   282,
     203,     0,     0,     0,     0,   269,   270,     0,     0,     0,
       0,     0,   205,     0,   231,     0,   229,     0,   257,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   285,     0,     0,     0,     0,    13,     0,     0,     0,
       0,     0,     0,    32,     0,     0,    66,     0,     0,     0,
       0,   224,    67,     0,    60,     0,     0,     0,     0,    50,
       0,    59,    41,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   157,
       0,     0,   152,   153,     0,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,    72,     0,     0,     0,     0,
       0,     0,     0,     0,    73,   136,   137,   138,     0,     0,
       0,   172,     0,     0,     0,     0,     0,     0,     0,   254,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,   248,   302,     0,     0,   217,     0,   215,
     216,     0,     0,   243,   241,   238,   242,   240,   239,   196,
     191,   193,   195,   192,   194,   200,   201,   244,   245,   246,
     249,   258,   237,   235,   232,   236,   234,   233,   260,   261,
     265,   266,   263,   264,   272,     0,   185,   189,     0,     0,
       0,    17,     0,     0,    68,     0,     0,    64,     0,     0,
     225,   226,   227,   228,     0,    62,    43,     0,    51,    56,
       0,    55,    61,    59,    49,     0,    49,    39,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   171,   140,   142,   141,   143,   150,   147,
     148,   155,   144,   260,   145,   261,   149,     0,     0,   128,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     130,   130,   130,   130,   175,     0,    28,     0,     0,     0,
       0,   250,   273,   276,   308,   274,   277,   309,   275,     0,
     210,     0,   212,   267,     0,     0,     0,   202,   198,     0,
     286,   289,   290,   291,     0,     0,   287,   288,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   221,   303,
       0,   256,   283,   284,   251,   204,     0,     0,     0,   230,
       0,     0,     0,     0,     0,     0,    16,     0,    22,     0,
       0,     0,   223,    69,     0,     0,    36,     0,     0,     0,
       0,    61,    54,     0,    54,   156,   273,     0,     0,   166,
     167,   160,   162,   161,   159,   163,   164,   165,     0,     0,
     158,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    76,     0,     0,     0,   129,   125,   126,   127,
       0,     0,   113,   115,   116,   118,   110,   112,   119,   121,
     122,   124,   107,   109,   114,   117,   111,   120,   123,   108,
      79,    80,     0,     0,    81,     0,     0,    82,   174,   173,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   218,     0,   207,     0,   209,   188,
       0,     0,    18,    15,    14,    70,     0,     0,     0,    30,
       0,     0,     0,    45,     0,    59,    40,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,   130,   130,
     130,   130,    23,    24,    25,    26,     0,     0,     0,     0,
       0,   220,   305,     0,     0,     0,     0,     0,    21,    31,
      37,    29,     0,     0,     0,     0,    42,    61,    38,     0,
     168,   169,     0,     0,     0,     0,     0,     0,   132,   133,
     134,   135,   131,    83,    85,    84,    86,   211,   213,   222,
     252,   306,     0,     0,   206,   208,     0,     0,     0,     0,
       0,     0,   170,    78,     0,   130,     0,   130,     0,   130,
       0,   130,   307,   304,     0,    19,     0,     0,     0,     0,
      44,     0,     0,   103,     0,     0,   105,     0,     0,   104,
       0,     0,   106,     0,    53,    52,    58,    57,     0,     0,
       0,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,    87,
      91,    90,    97,    89,    96,    95,   101,    88,    94,    93,
     100,    92,    99,    98,   102
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -534,  -534,  -534,   925,   767,  -534,  -534,  -534,  -534,  1015,
    -534,  -534,  -534,  -534,  1096,   -41,  -148,  -185,  -488,   946,
    1105,  -131,  -534,  -534,  -534,  -533,  1077,  -332,  -493,  -534,
     -96,   616,  -296,  -129,  -534,  -534,  -534,   877,  -534,  -534,
    1153,   -31,   853,  1293,  1118,  1067,  -252,  1365,  -534,  -534,
     381,   326,    17,    14,   -36,   522,   -25,  -102,  -534
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    13,    14,   470,   471,    15,    16,    17,    18,   193,
      19,   138,   748,    20,    50,   149,   321,   147,   316,   139,
     140,   141,   306,    21,    22,   183,   184,   360,   710,   812,
     368,   361,   173,   174,   175,   176,    23,   190,   191,    24,
      39,   126,   127,   128,   109,   110,   111,   142,   311,   255,
     112,   129,   238,   114,   177,   116,   117,   118,   119
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     115,   123,   245,   246,   650,   324,   372,   144,    41,   153,
     305,   133,   426,   145,   682,   683,   735,   186,   152,   445,
     446,    60,    43,    25,   200,   123,   549,   550,   474,   737,
     379,   202,   123,   643,   208,   210,   211,    26,   213,   214,
     342,   343,    45,    46,    47,    48,   363,   431,   432,   745,
     534,   536,   363,   747,    60,    27,   542,   544,   284,   285,
     633,   428,   187,    28,    60,   833,   429,   245,   246,   734,
     249,   258,   712,   713,   199,   468,   253,   469,   711,   714,
     717,    29,   278,   279,   280,   281,   282,   283,   123,   349,
     369,   265,   266,    32,   267,   268,   350,   728,   200,   123,
     301,   302,    30,   277,   284,   285,    42,   312,   430,   271,
     314,   239,   319,   240,   277,   736,   427,   284,   285,   123,
      44,   257,   636,   325,   364,   637,   277,    31,   738,   188,
     189,   277,   273,   274,   275,   345,   492,   284,   285,    33,
     203,   597,   304,    34,    49,   123,   362,   277,   299,   356,
     205,    35,   269,   270,   346,   347,   370,   273,   274,   275,
     284,   285,    40,   754,   284,   285,   378,   200,    51,   200,
     272,   387,   381,   389,   384,   479,    52,   365,   366,   367,
      53,   397,   398,   399,   400,   339,   402,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   476,   415,
     416,   200,   655,   284,   285,   200,   200,   200,   200,   604,
     774,   342,   343,    54,   123,   302,   246,   382,    55,   385,
     538,   687,   688,   689,   743,   546,    56,   637,   284,   285,
     803,   278,   279,   280,   281,   282,   283,   123,   123,   123,
     423,   451,   452,   453,   454,   455,   456,   457,   365,   366,
     367,   417,    57,    36,    37,   418,   419,   420,   421,   346,
     347,   460,   461,   462,   463,   464,    38,   758,   373,   200,
     374,   480,   481,   482,   483,   813,   814,   815,   816,   485,
     130,   123,   715,   716,   491,   557,   558,   559,   560,   561,
     562,   501,   503,   505,   507,   509,   511,   513,   515,   517,
      58,   563,   564,   565,   566,   567,   568,   758,   651,   831,
     365,   366,   367,   284,   285,   553,   554,   555,   373,   478,
     375,   424,   362,   552,   539,   540,   541,   186,   363,   462,
     346,   347,   571,   573,   278,   279,   280,   281,   282,   283,
     763,   764,   853,   373,   856,   376,   859,   146,   862,   273,
     274,   275,   590,   728,   592,   373,   596,   377,   585,   192,
     392,   588,   113,   122,   352,   353,   593,   346,   347,   143,
     346,   347,   286,   287,   728,   533,   557,   558,   559,   560,
     561,   562,   148,   352,   353,   265,   266,   122,   267,   268,
     628,   424,   352,   353,   122,   630,   631,   277,   686,   584,
     909,   910,   911,   912,   913,   914,   915,   916,   917,   918,
     919,   920,   921,   922,   923,   924,   284,   285,   121,   563,
     564,   565,   566,   567,   568,   197,   365,   366,   367,   288,
     284,   285,   122,   256,   150,   151,   289,   352,   353,   641,
     642,   824,   201,   200,   276,   728,   269,   270,   647,   212,
     122,   793,   599,   652,   123,   654,   284,   285,   346,   347,
     215,   122,   844,   658,   845,   655,   216,   259,   260,   261,
     262,   263,   264,   259,   260,   261,   262,   263,   264,   284,
     285,   122,   830,   286,   287,   620,   123,   248,   265,   266,
     671,   267,   268,   644,   265,   266,   217,   267,   268,   284,
     285,   352,   353,   218,   755,   292,   757,   122,   352,   353,
     825,   186,   685,   219,   284,   285,   300,   362,   362,   362,
     691,   693,   695,   697,   699,   701,   703,    65,    66,   821,
     124,   273,   274,   275,    69,    70,   326,   290,   291,    71,
     354,   720,   721,   123,   820,   284,   285,   289,   220,   269,
     270,   284,   285,   834,   835,   269,   270,   422,   724,   832,
     725,   221,   357,   265,   266,   222,   267,   268,   730,   731,
     797,    90,   864,   178,   178,   178,   122,   265,   266,   866,
     267,   268,    96,   277,   732,   587,   100,   346,   347,   369,
     641,   284,   285,   668,   765,   766,    60,   123,   237,   122,
     122,   122,   125,   346,   347,   107,   348,    65,    66,   746,
     124,   893,   751,   277,    69,    70,   346,   347,   277,    71,
     622,   346,   347,   472,   269,   270,   894,   296,   284,   285,
     143,   425,   759,   760,   284,   285,   424,   223,   269,   270,
     224,   284,   285,   122,   836,   837,   422,   895,   246,   225,
     468,    90,   469,   226,   447,   448,   449,   227,    65,    66,
     228,   124,    96,   352,   353,    69,   100,   172,   179,   185,
      71,   838,   839,   346,   347,   229,   768,   769,   230,   770,
     771,   231,   125,   265,   266,   107,   267,   268,   352,   353,
     232,   123,   780,   178,    65,    66,   233,   124,   487,   785,
     234,   786,    90,   277,   178,   623,    71,   352,   353,   346,
     347,   790,   896,    96,   840,   841,   795,   100,   122,   265,
     266,   235,   267,   268,   346,   347,   246,   186,   236,   805,
     807,   294,   533,   125,   346,   347,   107,   277,    90,   624,
     817,   818,   621,   277,   269,   270,   241,   822,   823,    96,
     605,   826,   627,   100,   629,   868,   869,   780,   870,   871,
     242,   872,   873,   243,   874,   875,   244,   265,   266,   125,
     267,   268,   107,   595,   352,   353,   295,   352,   353,   897,
     269,   270,   265,   266,   298,   267,   268,   344,   600,   352,
     353,   846,   297,   848,   780,   653,   151,   424,   852,   472,
     855,   898,   858,   899,   861,   303,   458,   459,   863,   726,
     143,   265,   266,   304,   267,   268,   122,   265,   266,   134,
     267,   268,   265,   266,   313,   267,   268,   320,   269,   270,
     900,   744,   878,   880,   882,   884,   886,   888,   890,   892,
     315,   346,   347,   269,   270,   901,   775,   322,   122,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   346,   347,   352,   353,   346,   347,   669,   535,   537,
     351,   649,   269,   270,   543,   545,   902,   327,   269,   270,
     178,   178,   323,   269,   270,   178,   788,   346,   347,   787,
     178,   178,   352,   353,   819,   265,   266,   328,   267,   268,
      60,   329,   582,   672,   330,   122,   903,   346,   347,   904,
      60,    65,    66,   331,   124,   905,   204,   206,    69,    70,
     906,    65,    66,    71,   124,   729,   284,   285,    69,    70,
     907,   284,   285,    71,   589,   908,   332,   783,   352,   353,
     273,   274,   275,   500,   502,   504,   506,   508,   510,   512,
     514,   516,   518,   519,   520,    90,   269,   270,   333,   122,
     722,   284,   285,   472,   334,    90,    96,   784,   346,   347,
     100,   352,   353,   284,   285,   551,    96,   346,   347,   185,
     100,   591,   352,   353,   570,   572,   125,   284,   285,   107,
     335,   336,   346,   347,   132,   601,   125,   352,   353,   107,
     265,   266,   337,   267,   268,   273,   274,   275,   273,   274,
     275,   338,   346,   347,   739,   355,   277,   727,   625,   465,
     656,   178,   259,   260,   261,   262,   263,   264,   273,   274,
     275,   273,   274,   275,   273,   274,   275,    60,   371,   475,
     380,   726,   499,   265,   266,   548,   267,   268,    65,    66,
     383,   124,   386,   122,   178,    69,    70,   553,   554,   555,
      71,   269,   270,   273,   274,   275,   388,   473,   556,   178,
     178,   194,   195,   196,   626,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   390,    90,   284,   285,   391,   393,   178,   273,   274,
     275,   602,   394,    96,   269,   270,   484,   100,   779,   648,
     273,   274,   275,   284,   285,   657,   749,   284,   285,   284,
     285,   752,   772,   207,   414,   603,   107,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,   273,   274,
     275,    60,   284,   285,   486,    12,   284,   285,   344,   762,
     606,   284,   285,   497,   607,   498,   547,   284,   285,   608,
     284,   285,   574,   185,   684,   609,   284,   285,   610,   773,
     575,   180,   690,   692,   694,   696,   698,   700,   702,   704,
     705,   706,   707,   708,   709,   576,   577,   154,    63,    64,
     578,   719,    67,   181,   579,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   581,   580,   583,    72,    73,    74,
      75,    76,    77,   156,   157,   158,   159,   160,   161,   162,
     163,   164,    87,    88,    89,   165,   166,   284,   285,   586,
     284,   285,   618,   273,   733,   611,   182,   167,   612,   753,
      91,    92,    93,    94,   168,   756,    97,    98,   634,    60,
     635,   101,   102,   103,   284,   285,   284,   285,   169,   170,
      65,    66,   613,   124,   614,   171,   673,    69,    70,   284,
     285,   674,    71,   284,   285,   675,   767,   615,   284,   285,
     178,   616,   676,   802,   178,   178,   617,   178,   178,   500,
     502,   504,   506,   508,   510,   512,   514,   516,   284,   285,
     346,   347,   352,   353,    90,   761,   619,   363,   659,   553,
     606,   677,   346,   347,   678,    96,   352,   353,   679,   100,
     660,   680,   681,   776,   607,   346,   347,   777,   781,   108,
     120,   352,   353,   661,   131,   209,   346,   347,   107,   608,
     352,   353,   346,   347,   662,   352,   353,   791,   609,   796,
     663,   346,   347,   610,   120,   798,   178,   850,   178,   664,
     178,   120,   178,   352,   353,   346,   347,   352,   353,   346,
     347,   611,   827,   665,   799,   612,   829,   666,   657,   185,
     865,   804,   806,   439,   440,   441,   442,   443,   444,   867,
     178,   178,   178,   178,   178,   178,   178,   178,   639,   247,
     352,   353,   346,   347,   742,   521,   352,   353,   613,   477,
     667,   522,   346,   347,   614,   284,   285,   120,   293,   496,
     670,   284,   285,   782,   307,   308,   309,   310,   120,   800,
     646,   523,   524,   525,   526,   527,   528,   529,   530,   531,
     569,   284,   285,   165,   166,   284,   285,   466,   120,   801,
     851,   718,   854,   842,   857,   167,   860,   284,   285,   284,
     285,   598,   168,   284,   285,   843,   789,   876,   493,   494,
      47,   495,   254,     0,   120,     0,   340,   341,   808,   809,
     810,   811,     0,   532,   877,   879,   881,   883,   885,   887,
     889,   891,     0,     0,     0,     0,     0,     0,     0,     0,
     293,     0,   293,     0,     0,     0,     0,     0,   395,   396,
       0,     0,     0,     0,   401,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   120,   293,     0,     0,     0,     0,     0,
     134,     0,   433,   434,   435,   436,   437,   438,     0,   645,
       0,     0,     0,    60,     0,     0,   120,   120,   120,   450,
       0,    62,    63,    64,     0,     0,    67,   135,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   467,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
     120,   136,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
      97,    98,     0,     0,     0,   101,   102,   103,     0,     0,
       0,     0,   104,   105,     0,   134,     0,     0,     0,   137,
       0,     0,     0,     0,     0,     0,     0,     0,    60,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,     0,
       0,    67,   135,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   594,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,     0,     0,   136,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,    97,    98,     0,     0,     0,
     101,   102,   103,     0,     0,     0,     0,   104,   105,     0,
       0,     0,     0,     0,   137,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,     0,     0,     0,   632,     0,
       0,    60,     0,     0,     0,   638,   640,     0,    61,    62,
      63,    64,    65,    66,    67,    68,     0,     0,     0,    69,
      70,     0,     0,   120,    71,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,   120,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,     0,   101,   102,   103,     0,     0,     0,     0,
     104,   105,     0,     0,     0,     0,     0,   106,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    59,     0,     0,     0,     0,     0,     0,    60,     0,
       0,     0,   120,   723,     0,    61,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,    69,    70,     0,     0,
       0,    71,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,     0,     0,     0,     0,     0,   632,
       0,     0,     0,    90,     0,     0,   120,   740,   741,    91,
      92,    93,    94,     0,    96,    97,    98,    99,   100,     0,
     101,   102,   103,     0,     0,     0,     0,   104,   105,     0,
       0,     0,     0,     0,   106,     0,     0,   107,     0,     0,
      60,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,   250,     0,     0,     0,    69,    70,
       0,     0,     0,    71,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,     0,     0,   136,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,   778,
     120,    91,    92,    93,    94,     0,    96,    97,    98,     0,
     100,     0,   101,   102,   103,     0,     0,     0,     0,   104,
     105,     0,     0,    60,     0,     0,   251,     0,     0,   107,
     252,    62,    63,    64,    65,    66,    67,   250,     0,     0,
       0,    69,    70,     0,     0,     0,    71,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,    96,
      97,    98,     0,   100,     0,   101,   102,   103,     0,     0,
       0,     0,   104,   105,     0,    60,     0,     0,     0,   251,
       0,     0,   107,    62,    63,    64,     0,     0,    67,   135,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,    97,    98,   198,     0,     0,   101,   102,   103,
       0,    62,    63,    64,   104,   105,    67,   135,     0,     0,
       0,   137,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
      97,    98,     0,     0,   489,   101,   102,   103,     0,    62,
      63,    64,   104,   105,    67,   490,     0,     0,     0,   137,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,    97,    98,
       0,     0,   794,   101,   102,   103,     0,    62,    63,    64,
     104,   105,    67,   135,     0,     0,     0,   137,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,    97,    98,     0,     0,
     849,   101,   102,   103,     0,    62,    63,    64,   104,   105,
      67,   135,     0,     0,     0,   137,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,    97,    98,     0,   488,     0,   101,
     102,   103,    62,    63,    64,     0,   104,   105,   135,     0,
       0,     0,     0,   137,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,    97,    98,     0,   792,     0,   101,   102,   103,    62,
      63,    64,     0,     0,     0,   135,     0,     0,     0,     0,
     137,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,    97,    98,
       0,   847,     0,   101,   102,   103,    62,    63,    64,     0,
       0,     0,   135,     0,     0,     0,     0,   137,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,    97,    98,     0,     0,     0,
     101,   102,   103,     0,     0,    62,    63,    64,    65,    66,
      67,   250,     0,     0,   137,    69,    70,     0,     0,     0,
      71,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,    96,    97,    98,     0,   100,     0,   101,
     102,   103,   358,   154,    63,    64,   104,   105,    67,   155,
       0,     0,     0,   251,     0,     0,   107,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,   156,
     157,   158,   159,   160,   161,   162,   163,   164,    87,    88,
      89,   165,   166,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   167,     0,     0,    91,    92,    93,    94,
     168,     0,    97,    98,     0,     0,     0,   101,   102,   103,
       0,   154,    63,    64,   169,   170,    67,   155,     0,     0,
       0,   359,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    87,    88,    89,   165,
     166,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,   167,     0,     0,    91,    92,    93,    94,   168,     0,
      97,    98,     0,     0,     0,   101,   102,   103,     0,   154,
      63,    64,   169,   170,    67,   155,     0,     0,     0,   171,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,   156,   157,   158,   159,   160,
     161,   162,   163,   164,    87,    88,    89,   165,   166,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   167,
       0,     0,    91,    92,    93,    94,   168,     0,    97,    98,
     154,    63,    64,   101,   102,   103,   155,     0,     0,     0,
     169,   170,     0,     0,     0,     0,     0,   171,     0,     0,
      72,    73,    74,    75,    76,    77,   156,   157,   158,   159,
     160,   161,   162,   163,   164,    87,    88,    89,   165,   166,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     167,     0,     0,    91,    92,    93,    94,   168,     0,    97,
      98,   154,    63,    64,   101,   102,   103,   155,     0,     0,
       0,   340,   341,     0,     0,     0,     0,     0,   171,     0,
       0,    72,    73,    74,    75,    76,    77,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    87,    88,    89,   165,
     166,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   167,     0,     0,    91,    92,    93,    94,   168,     0,
      97,    98,     0,     0,     0,   101,   102,   103,     0,    62,
      63,    64,   169,   170,    67,   135,     0,     0,     0,   171,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,    97,    98,
       0,     0,     0,   101,   102,   103,     0,    62,    63,    64,
     104,   105,    67,   317,     0,     0,     0,   137,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,    97,    98,     0,     0,
       0,   101,   102,   103,     0,    62,    63,    64,   104,   318,
      67,   135,     0,     0,     0,   137,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,    97,    98,     0,     0,     0,   101,
     102,   103,     0,    62,    63,    64,   104,   750,    67,   135,
       0,     0,     0,   137,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,    97,    98,    62,    63,    64,   101,   102,   103,
     135,     0,     0,     0,   104,   828,     0,     0,     0,     0,
       0,   137,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,    97,    98,    62,    63,    64,   101,   102,
     103,   135,     0,     0,     0,   104,   105,     0,     0,     0,
       0,     0,   137,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,    97,    98,     0,     0,     0,   101,
     102,   103,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   137
};

static const yytype_int16 yycheck[] =
{
      36,    37,   104,   105,   492,   153,   191,    43,    20,    50,
     141,    42,    21,    44,   547,   548,    26,    53,    49,   271,
     272,    35,    20,   107,    60,    61,   358,   359,    22,    26,
      36,    62,    68,    22,    65,    66,    67,   107,    69,    70,
     169,   170,    16,    17,    18,    19,    39,    24,    25,    22,
     346,   347,    39,    31,    35,   107,   352,   353,   114,   115,
      22,   120,    27,   107,    35,    52,   125,   169,   170,   125,
     106,   107,    32,    33,    60,    89,   107,    91,   571,   572,
     573,   107,    32,    33,    34,    35,    36,    37,   124,   116,
     186,    53,    54,   106,    56,    57,   123,    51,   134,   135,
     136,   137,   107,   120,   114,   115,   118,   143,   125,    55,
     146,    94,   148,    96,   120,   125,   125,   114,   115,   155,
     118,   107,   117,   154,   117,   120,   120,   107,   125,    94,
      95,   120,    39,    40,    41,   171,   321,   114,   115,   106,
     121,   393,   120,   106,   118,   181,   182,   120,   134,   180,
     121,   106,   114,   115,   114,   115,   187,    39,    40,    41,
     114,   115,     0,   651,   114,   115,   197,   203,    21,   205,
     116,   207,   203,   209,   205,   306,    21,    32,    33,    34,
      21,   217,   218,   219,   220,   168,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,    84,   235,
     236,   237,   498,   114,   115,   241,   242,   243,   244,   120,
     117,   340,   341,    21,   250,   251,   318,   203,   121,   205,
     349,   553,   554,   555,   117,   354,   121,   120,   114,   115,
     763,    32,    33,    34,    35,    36,    37,   273,   274,   275,
     122,   277,   278,   279,   280,   281,   282,   283,    32,    33,
      34,   237,   121,    14,    15,   241,   242,   243,   244,   114,
     115,   286,   287,   288,   289,   290,    27,    51,   120,   305,
     122,   307,   308,   309,   310,   768,   769,   770,   771,   315,
     117,   317,    32,    33,   320,    32,    33,    34,    35,    36,
      37,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     121,    32,    33,    34,    35,    36,    37,    51,   493,   797,
      32,    33,    34,   114,   115,    39,    40,    41,   120,   305,
     122,   122,   358,   359,   349,   350,   351,   363,    39,   354,
     114,   115,   368,   369,    32,    33,    34,    35,    36,    37,
      51,    52,   835,   120,   837,   122,   839,    29,   841,    39,
      40,    41,   388,    51,   390,   120,   392,   122,   383,   107,
      50,   386,    36,    37,   114,   115,   391,   114,   115,    43,
     114,   115,    59,    60,    51,   122,    32,    33,    34,    35,
      36,    37,    33,   114,   115,    53,    54,    61,    56,    57,
     426,   122,   114,   115,    68,   431,   432,   120,   122,   122,
     893,   894,   895,   896,   897,   898,   899,   900,   901,   902,
     903,   904,   905,   906,   907,   908,   114,   115,    37,    32,
      33,    34,    35,    36,    37,   121,    32,    33,    34,   116,
     114,   115,   106,   107,    16,    17,   123,   114,   115,   475,
     476,   125,    61,   479,    23,    51,   114,   115,   484,    68,
     124,    51,   120,   494,   490,   496,   114,   115,   114,   115,
     121,   135,   120,   499,   122,   761,   121,    32,    33,    34,
      35,    36,    37,    32,    33,    34,    35,    36,    37,   114,
     115,   155,    51,    59,    60,   120,   522,   106,    53,    54,
     521,    56,    57,   479,    53,    54,   121,    56,    57,   114,
     115,   114,   115,   121,   652,   124,   654,   181,   114,   115,
     125,   547,   548,   121,   114,   115,   135,   553,   554,   555,
     556,   557,   558,   559,   560,   561,   562,    46,    47,    52,
      49,    39,    40,    41,    53,    54,   155,    61,    62,    58,
     116,   577,   578,   579,    52,   114,   115,   123,   121,   114,
     115,   114,   115,    51,    52,   114,   115,   122,   589,    52,
     591,   121,   181,    53,    54,   121,    56,    57,   604,   605,
     755,    90,    52,    51,    52,    53,   250,    53,    54,    52,
      56,    57,   101,   120,   620,   122,   105,   114,   115,   685,
     626,   114,   115,   120,   690,   691,    35,   633,   118,   273,
     274,   275,   121,   114,   115,   124,   117,    46,    47,   645,
      49,    52,   648,   120,    53,    54,   114,   115,   120,    58,
     122,   114,   115,   297,   114,   115,    52,   117,   114,   115,
     304,   250,   668,   669,   114,   115,   122,   121,   114,   115,
     121,   114,   115,   317,    51,    52,   122,    52,   750,   121,
      89,    90,    91,   121,   273,   274,   275,   121,    46,    47,
     121,    49,   101,   114,   115,    53,   105,    51,    52,    53,
      58,    51,    52,   114,   115,   121,   712,   713,   121,   715,
     716,   121,   121,    53,    54,   124,    56,    57,   114,   115,
     121,   727,   728,   171,    46,    47,   121,    49,   317,   735,
     121,   737,    90,   120,   182,   122,    58,   114,   115,   114,
     115,   747,    52,   101,    51,    52,   752,   105,   392,    53,
      54,   121,    56,    57,   114,   115,   828,   763,   121,   765,
     766,    21,   122,   121,   114,   115,   124,   120,    90,   122,
     776,   777,   119,   120,   114,   115,   121,   783,   784,   101,
     120,   787,   426,   105,   428,   851,   852,   793,   854,   855,
     121,   857,   858,   121,   860,   861,   121,    53,    54,   121,
      56,    57,   124,   392,   114,   115,    23,   114,   115,    52,
     114,   115,    53,    54,   119,    56,    57,   171,   122,   114,
     115,   827,    20,   829,   830,    16,    17,   122,   834,   473,
     836,    52,   838,    52,   840,   117,   284,   285,   844,    51,
     484,    53,    54,   120,    56,    57,   490,    53,    54,    22,
      56,    57,    53,    54,   119,    56,    57,    32,   114,   115,
      52,   117,   868,   869,   870,   871,   872,   873,   874,   875,
      30,   114,   115,   114,   115,    52,   117,   117,   522,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   114,   115,   114,   115,   114,   115,   120,   346,   347,
      61,   490,   114,   115,   352,   353,    52,   121,   114,   115,
     358,   359,   119,   114,   115,   363,   122,   114,   115,   120,
     368,   369,   114,   115,    52,    53,    54,   121,    56,    57,
      35,   121,   380,   522,   121,   579,    52,   114,   115,    52,
      35,    46,    47,   121,    49,    52,    63,    64,    53,    54,
      52,    46,    47,    58,    49,   599,   114,   115,    53,    54,
      52,   114,   115,    58,   122,    52,   121,   120,   114,   115,
      39,    40,    41,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,    90,   114,   115,   121,   633,
     579,   114,   115,   637,   121,    90,   101,   120,   114,   115,
     105,   114,   115,   114,   115,   359,   101,   114,   115,   363,
     105,   122,   114,   115,   368,   369,   121,   114,   115,   124,
     121,   121,   114,   115,   119,   122,   121,   114,   115,   124,
      53,    54,   121,    56,    57,    39,    40,    41,    39,    40,
      41,   121,   114,   115,   633,   117,   120,    51,   122,    50,
     498,   499,    32,    33,    34,    35,    36,    37,    39,    40,
      41,    39,    40,    41,    39,    40,    41,    35,   117,    50,
      21,    51,    50,    53,    54,    50,    56,    57,    46,    47,
      21,    49,    21,   727,   532,    53,    54,    39,    40,    41,
      58,   114,   115,    39,    40,    41,    21,    20,    50,   547,
     548,    56,    57,    58,    50,   553,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,   564,   565,   566,   567,
     568,    21,    90,   114,   115,    21,    21,   575,    39,    40,
      41,   122,    21,   101,   114,   115,    20,   105,   727,    50,
      39,    40,    41,   114,   115,   499,   117,   114,   115,   114,
     115,    50,   117,   121,   101,   122,   124,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    39,    40,
      41,    35,   114,   115,   117,    21,   114,   115,   532,    50,
     122,   114,   115,   117,   122,    21,    21,   114,   115,   122,
     114,   115,    21,   547,   548,   122,   114,   115,   122,   117,
      21,    27,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   107,    20,    43,    44,    45,
      20,   575,    48,    49,    20,   673,   674,   675,   676,   677,
     678,   679,   680,   681,   122,    20,   122,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,   114,   115,   122,
     114,   115,   122,    39,   125,   122,    92,    93,   122,   117,
      96,    97,    98,    99,   100,   117,   102,   103,   121,    35,
     121,   107,   108,   109,   114,   115,   114,   115,   114,   115,
      46,    47,   122,    49,   122,   121,   121,    53,    54,   114,
     115,   121,    58,   114,   115,   121,   120,   122,   114,   115,
     758,   122,   121,   761,   762,   763,   122,   765,   766,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   114,   115,
     114,   115,   114,   115,    90,    21,   122,    39,   122,    39,
     122,   121,   114,   115,   121,   101,   114,   115,   121,   105,
     122,   121,   121,    21,   122,   114,   115,    21,   122,    36,
      37,   114,   115,   122,    41,   121,   114,   115,   124,   122,
     114,   115,   114,   115,   122,   114,   115,   117,   122,   117,
     122,   114,   115,   122,    61,   117,   834,   117,   836,   122,
     838,    68,   840,   114,   115,   114,   115,   114,   115,   114,
     115,   122,    51,   122,   758,   122,    51,   122,   762,   763,
      52,   765,   766,   265,   266,   267,   268,   269,   270,    52,
     868,   869,   870,   871,   872,   873,   874,   875,   473,   106,
     114,   115,   114,   115,   637,    43,   114,   115,   122,   304,
     122,    49,   114,   115,   122,   114,   115,   124,   125,   323,
     122,   114,   115,   122,    85,    86,    87,    88,   135,   122,
     484,    69,    70,    71,    72,    73,    74,    75,    76,    77,
     363,   114,   115,    81,    82,   114,   115,   294,   155,   122,
     834,   574,   836,   122,   838,    93,   840,   114,   115,   114,
     115,   394,   100,   114,   115,   122,   117,   122,    16,    17,
      18,    19,   107,    -1,   181,    -1,   114,   115,   110,   111,
     112,   113,    -1,   121,   868,   869,   870,   871,   872,   873,
     874,   875,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     207,    -1,   209,    -1,    -1,    -1,    -1,    -1,   215,   216,
      -1,    -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   250,   251,    -1,    -1,    -1,    -1,    -1,
      22,    -1,   259,   260,   261,   262,   263,   264,    -1,    31,
      -1,    -1,    -1,    35,    -1,    -1,   273,   274,   275,   276,
      -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   295,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
     317,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    97,    98,    99,    -1,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,    -1,    22,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      -1,    48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   392,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
      97,    98,    99,    -1,    -1,   102,   103,    -1,    -1,    -1,
     107,   108,   109,    -1,    -1,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,   465,    -1,
      -1,    35,    -1,    -1,    -1,   472,   473,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    -1,    -1,   490,    58,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   522,    90,    -1,    -1,    -1,
      -1,    -1,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,    -1,   107,   108,   109,    -1,    -1,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,   579,   580,    -1,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    -1,    -1,    53,    54,    -1,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,   626,
      -1,    -1,    -1,    90,    -1,    -1,   633,   634,   635,    96,
      97,    98,    99,    -1,   101,   102,   103,   104,   105,    -1,
     107,   108,   109,    -1,    -1,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,   124,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      45,    46,    47,    48,    49,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    -1,    83,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,   726,
     727,    96,    97,    98,    99,    -1,   101,   102,   103,    -1,
     105,    -1,   107,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,    -1,    -1,    35,    -1,    -1,   121,    -1,    -1,   124,
     125,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,   105,    -1,   107,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,    -1,    35,    -1,    -1,    -1,   121,
      -1,    -1,   124,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,    -1,   102,   103,    36,    -1,    -1,   107,   108,   109,
      -1,    43,    44,    45,   114,   115,    48,    49,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    97,    98,    99,    -1,    -1,
     102,   103,    -1,    -1,    38,   107,   108,   109,    -1,    43,
      44,    45,   114,   115,    48,    49,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    98,    99,    -1,    -1,   102,   103,
      -1,    -1,    38,   107,   108,   109,    -1,    43,    44,    45,
     114,   115,    48,    49,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      96,    97,    98,    99,    -1,    -1,   102,   103,    -1,    -1,
      38,   107,   108,   109,    -1,    43,    44,    45,   114,   115,
      48,    49,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,    -1,   102,   103,    -1,    38,    -1,   107,
     108,   109,    43,    44,    45,    -1,   114,   115,    49,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,    -1,
      -1,   102,   103,    -1,    38,    -1,   107,   108,   109,    43,
      44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    98,    99,    -1,    -1,   102,   103,
      -1,    38,    -1,   107,   108,   109,    43,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
      97,    98,    99,    -1,    -1,   102,   103,    -1,    -1,    -1,
     107,   108,   109,    -1,    -1,    43,    44,    45,    46,    47,
      48,    49,    -1,    -1,   121,    53,    54,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,   105,    -1,   107,
     108,   109,    42,    43,    44,    45,   114,   115,    48,    49,
      -1,    -1,    -1,   121,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    -1,    -1,    96,    97,    98,    99,
     100,    -1,   102,   103,    -1,    -1,    -1,   107,   108,   109,
      -1,    43,    44,    45,   114,   115,    48,    49,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    96,    97,    98,    99,   100,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,   109,    -1,    43,
      44,    45,   114,   115,    48,    49,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      -1,    -1,    96,    97,    98,    99,   100,    -1,   102,   103,
      43,    44,    45,   107,   108,   109,    49,    -1,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    -1,    -1,    96,    97,    98,    99,   100,    -1,   102,
     103,    43,    44,    45,   107,   108,   109,    49,    -1,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    -1,    -1,    96,    97,    98,    99,   100,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,   109,    -1,    43,
      44,    45,   114,   115,    48,    49,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    98,    99,    -1,    -1,   102,   103,
      -1,    -1,    -1,   107,   108,   109,    -1,    43,    44,    45,
     114,   115,    48,    49,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      96,    97,    98,    99,    -1,    -1,   102,   103,    -1,    -1,
      -1,   107,   108,   109,    -1,    43,    44,    45,   114,   115,
      48,    49,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,    -1,   102,   103,    -1,    -1,    -1,   107,
     108,   109,    -1,    43,    44,    45,   114,   115,    48,    49,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,    -1,   102,   103,    43,    44,    45,   107,   108,   109,
      49,    -1,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    98,
      99,    -1,    -1,   102,   103,    43,    44,    45,   107,   108,
     109,    49,    -1,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,    -1,   102,   103,    -1,    -1,    -1,   107,
     108,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    21,   127,   128,   131,   132,   133,   134,   136,
     139,   149,   150,   162,   165,   107,   107,   107,   107,   107,
     107,   107,   106,   106,   106,   106,    14,    15,    27,   166,
       0,    20,   118,    20,   118,    16,    17,    18,    19,   118,
     140,    21,    21,    21,    21,   121,   121,   121,   121,    28,
      35,    42,    43,    44,    45,    46,    47,    48,    49,    53,
      54,    58,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      90,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   107,   108,   109,   114,   115,   121,   124,   169,   170,
     171,   172,   176,   177,   179,   180,   181,   182,   183,   184,
     169,   176,   177,   180,    49,   121,   167,   168,   169,   177,
     117,   169,   119,   167,    22,    49,    83,   121,   137,   145,
     146,   147,   173,   177,   180,   167,    29,   143,    33,   141,
      16,    17,   167,   141,    43,    49,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    81,    82,    93,   100,   114,
     115,   121,   157,   158,   159,   160,   161,   180,   181,   157,
      27,    49,    92,   151,   152,   157,   180,    27,    94,    95,
     163,   164,   107,   135,   135,   135,   135,   121,    36,   179,
     180,   176,   167,   121,   168,   121,   168,   121,   167,   121,
     167,   167,   176,   167,   167,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   118,   178,   178,
     178,   121,   121,   121,   121,   183,   183,   169,   176,   180,
      49,   121,   125,   167,   173,   175,   177,   179,   180,    32,
      33,    34,    35,    36,    37,    53,    54,    56,    57,   114,
     115,    55,   116,    39,    40,    41,    23,   120,    32,    33,
      34,    35,    36,    37,   114,   115,    59,    60,   116,   123,
      61,    62,   176,   169,    21,    23,   117,    20,   119,   179,
     176,   180,   180,   117,   120,   147,   148,    85,    86,    87,
      88,   174,   180,   119,   180,    30,   144,    49,   115,   180,
      32,   142,   117,   119,   142,   167,   176,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   178,
     114,   115,   159,   159,   157,   180,   114,   115,   117,   116,
     123,    61,   114,   115,   116,   117,   167,   176,    42,   121,
     153,   157,   180,    39,   117,    32,    33,    34,   156,   156,
     167,   117,   143,   120,   122,   122,   122,   122,   167,    36,
      21,   167,   179,    21,   167,   179,    21,   180,    21,   180,
      21,    21,    50,    21,    21,   169,   169,   180,   180,   180,
     180,   169,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   101,   180,   180,   179,   179,   179,
     179,   179,   122,   122,   122,   176,    21,   125,   120,   125,
     125,    24,    25,   169,   169,   169,   169,   169,   169,   170,
     170,   170,   170,   170,   170,   172,   172,   176,   176,   176,
     169,   180,   180,   180,   180,   180,   180,   180,   181,   181,
     182,   182,   182,   182,   182,    50,   166,   169,    89,    91,
     129,   130,   177,    20,    22,    50,    84,   146,   179,   147,
     180,   180,   180,   180,    20,   180,   117,   176,    38,    38,
      49,   180,   143,    16,    17,    19,   140,   117,    21,    50,
     157,   180,   157,   180,   157,   180,   157,   180,   157,   180,
     157,   180,   157,   180,   157,   180,   157,   180,   157,   157,
     157,    43,    49,    69,    70,    71,    72,    73,    74,    75,
      76,    77,   121,   122,   158,   181,   158,   181,   159,   182,
     182,   182,   158,   181,   158,   181,   159,    21,    50,   153,
     153,   157,   180,    39,    40,    41,    50,    32,    33,    34,
      35,    36,    37,    32,    33,    34,    35,    36,    37,   152,
     157,   180,   157,   180,    21,    21,   107,    20,    20,    20,
      20,   122,   181,   122,   122,   182,   122,   122,   182,   122,
     180,   122,   180,   182,   169,   176,   180,   172,   171,   120,
     122,   122,   122,   122,   120,   120,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     120,   119,   122,   122,   122,   122,    50,   177,   180,   177,
     180,   180,   169,    22,   121,   121,   117,   120,   169,   129,
     169,   180,   180,    22,   179,    31,   145,   180,    50,   176,
     144,   143,   141,    16,   141,   158,   181,   157,   180,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   120,   120,
     122,   167,   176,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   151,   151,   157,   180,   122,   153,   153,   153,
     157,   180,   157,   180,   157,   180,   157,   180,   157,   180,
     157,   180,   157,   180,   157,   157,   157,   157,   157,   157,
     154,   154,    32,    33,   154,    32,    33,   154,   163,   157,
     180,   180,   176,   169,   167,   167,    51,    51,    51,   177,
     180,   180,   180,   125,   125,    26,   125,    26,   125,   176,
     169,   169,   130,   117,   117,    22,   180,    31,   138,   117,
     115,   180,    50,   117,   144,   142,   117,   142,    51,   180,
     180,    21,    50,    51,    52,   156,   156,   120,   180,   180,
     180,   180,   117,   117,   117,   117,    21,    21,   169,   176,
     180,   122,   122,   120,   120,   180,   180,   120,   122,   117,
     180,   117,    38,    51,    38,   180,   117,   143,   117,   157,
     122,   122,   181,   151,   157,   180,   157,   180,   110,   111,
     112,   113,   155,   154,   154,   154,   154,   180,   180,    52,
      52,    52,   180,   180,   125,   125,   180,    51,   115,    51,
      51,   144,    52,    52,    51,    52,    51,    52,    51,    52,
      51,    52,   122,   122,   120,   122,   180,    38,   180,    38,
     117,   157,   180,   154,   157,   180,   154,   157,   180,   154,
     157,   180,   154,   180,    52,    52,    52,    52,   156,   156,
     156,   156,   156,   156,   156,   156,   122,   157,   180,   157,
     180,   157,   180,   157,   180,   157,   180,   157,   180,   157,
     180,   157,   180,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   126,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   128,   128,   128,   128,   129,   129,   129,
     129,   129,   130,   131,   132,   133,   134,   135,   135,   136,
     136,   136,   136,   136,   137,   137,   138,   138,   139,   139,
     139,   139,   139,   139,   139,   139,   140,   140,   140,   141,
     141,   141,   141,   141,   142,   142,   142,   142,   142,   143,
     143,   144,   144,   145,   145,   145,   145,   146,   147,   148,
     148,   149,   149,   150,   151,   151,   151,   151,   151,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     154,   154,   155,   155,   155,   155,   156,   156,   156,   157,
     157,   157,   157,   157,   157,   157,   158,   158,   158,   158,
     158,   159,   159,   159,   160,   160,   160,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   162,   163,   163,   164,   164,   164,   165,   166,
     166,   166,   166,   166,   166,   166,   167,   167,   168,   168,
     169,   169,   169,   169,   169,   169,   169,   170,   170,   171,
     171,   171,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   173,   173,   174,   174,   174,   174,   175,
     175,   175,   176,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,   176,
     176,   176,   176,   177,   177,   178,   178,   179,   179,   180,
     180,   180,   181,   181,   181,   181,   181,   181,   182,   182,
     182,   183,   183,   183,   183,   183,   183,   183,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     5,     8,     8,     7,     1,     3,     6,
       8,     4,     2,     8,     8,     8,     8,     1,     3,     9,
       8,     9,     5,     1,     1,     1,     0,     2,     9,     6,
       8,     5,     9,     6,    11,     8,     0,     1,     2,     0,
       2,     3,     9,     9,     0,     2,     2,     8,     8,     0,
       2,     0,     2,     1,     3,     1,     2,     2,     3,     3,
       4,     5,     5,     5,     1,     3,     4,     5,     7,     4,
       4,     4,     4,     6,     6,     6,     6,    12,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
      12,    12,    12,     8,     8,     8,     8,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     1,     2,     2,     1,     3,     4,     2,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     6,     6,
       7,     3,     5,     4,     4,     0,     1,     1,     3,     2,
       2,     2,     2,     2,     2,     4,     1,     1,     5,     3,
       1,     3,     3,     3,     3,     3,     3,     1,     4,     1,
       3,     3,     4,     2,     4,     2,     7,     5,     7,     5,
       4,     7,     4,     7,     3,     3,     3,     3,     5,     5,
       6,     4,     7,     4,     2,     2,     2,     2,     2,     1,
       3,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     3,
       4,     4,     7,     2,     3,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     4,     1,     2,
       2,     1,     3,     4,     4,     4,     4,     4,     1,     1,
       1,     2,     2,     4,     4,     2,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     3,     4,     8,     6,     7,     8,     4,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

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
      int yyn = yypact[*yyssp];
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
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
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
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
          yyp++;
          yyformat++;
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
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

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
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
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
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
      yychar = yylex (&yylval);
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
#line 150 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2509 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 3:
#line 151 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2515 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 4:
#line 152 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2521 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 5:
#line 153 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2527 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 6:
#line 154 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2533 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 7:
#line 155 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2539 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 8:
#line 156 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2545 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 9:
#line 157 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2551 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 10:
#line 158 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2557 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 11:
#line 159 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2563 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 12:
#line 160 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { code_set_root((yyvsp[0].code)); }
#line 2569 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 13:
#line 168 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_set1, 3,
            code_new_name((yyvsp[-3].name)),                                       /* Name */
            code_new_inst(i_idxset_pseudo_new, 1,               /* index set */
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-1].code));                                              /* initial set */
      }
#line 2581 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 14:
#line 175 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_set1, 3,
            code_new_name((yyvsp[-6].name)),                                       /* Name */
            (yyvsp[-4].code),                                                 /* index set */
            (yyvsp[-1].code));                                                      /* set */
      }
#line 2592 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 15:
#line 181 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_set2, 3,
            code_new_name((yyvsp[-6].name)),                                       /* Name */
            (yyvsp[-4].code),                                                 /* index set */
            (yyvsp[-1].code));                                   /* initial set_entry_list */
      }
#line 2603 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 16:
#line 187 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_set2, 3,
            code_new_name((yyvsp[-5].name)),                                       /* Name */
            code_new_inst(i_idxset_pseudo_new, 1,               /* index set */
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-1].code));                                   /* initial set_entry_list */
      }
#line 2615 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 17:
#line 197 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_entry_list_new, 1, (yyvsp[0].code)); }
#line 2621 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 18:
#line 198 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 2629 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 19:
#line 201 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_subsets, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(-1)));
      }
#line 2637 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 20:
#line 204 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_subsets, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 2645 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 21:
#line 207 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_powerset, 1, (yyvsp[-1].code));
      }
#line 2653 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 22:
#line 213 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_entry, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 2659 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 23:
#line 222 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2670 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 24:
#line 231 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2681 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 25:
#line 240 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2692 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 26:
#line 249 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2703 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 27:
#line 258 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_elem_list_new, 1, code_new_name((yyvsp[0].name)));
      }
#line 2711 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 28:
#line 261 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_elem_list_add, 2, (yyvsp[-2].code), code_new_name((yyvsp[0].name)));
      }
#line 2719 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 29:
#line 271 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_para1, 4, code_new_name((yyvsp[-7].name)), (yyvsp[-5].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2727 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 30:
#line 274 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_para2, 4, code_new_name((yyvsp[-6].name)), (yyvsp[-4].code), (yyvsp[-1].code), code_new_inst(i_nop, 0));
      }
#line 2735 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 31:
#line 277 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_para2, 4, code_new_name((yyvsp[-7].name)), (yyvsp[-5].code), (yyvsp[-1].code), code_new_inst(i_nop, 0));
      }
#line 2743 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 32:
#line 280 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_para1, 4,
            code_new_name((yyvsp[-3].name)),
            code_new_inst(i_idxset_pseudo_new, 1, code_new_inst(i_bool_true, 0)),
            (yyvsp[-1].code),
            code_new_inst(i_nop, 0));
      }
#line 2755 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 33:
#line 287 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_nop, 0); }
#line 2761 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 34:
#line 290 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 2767 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 35:
#line 291 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_new, 1,
            code_new_inst(i_entry, 2, code_new_inst(i_tuple_empty, 0), (yyvsp[0].code)));
      }
#line 2776 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 36:
#line 298 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_nop, 0); }
#line 2782 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 37:
#line 299 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_entry, 2, code_new_inst(i_tuple_empty, 0), (yyvsp[0].code)); }
#line 2788 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 38:
#line 307 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-7].name)),
            (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2800 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 39:
#line 314 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-4].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2814 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 40:
#line 323 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-6].name)),
            (yyvsp[-4].code),
            code_new_varclass(VAR_IMP),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2829 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 41:
#line 333 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-3].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_IMP),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2845 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 42:
#line 344 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-7].name)),
            (yyvsp[-5].code),
            code_new_varclass(VAR_INT),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2859 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 43:
#line 353 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-4].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_INT),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2874 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 44:
#line 363 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-9].name)), (yyvsp[-7].code), code_new_varclass(VAR_INT), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2883 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 45:
#line 367 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-6].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_INT), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2895 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 46:
#line 377 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_varclass(VAR_CON); }
#line 2901 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 47:
#line 378 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_varclass(VAR_CON); }
#line 2907 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 48:
#line 379 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_varclass(VAR_IMP); }
#line 2913 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 49:
#line 383 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0)));
      }
#line 2921 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 50:
#line 386 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bound_new, 1, (yyvsp[0].code)); }
#line 2927 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 51:
#line 387 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_bound(BOUND_MINUS_INFTY); }
#line 2933 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 52:
#line 388 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-6].code),
            code_new_inst(i_bound_new, 1, (yyvsp[-4].code)),
            code_new_bound(BOUND_MINUS_INFTY));
      }
#line 2943 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 53:
#line 393 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-6].code),
            code_new_bound(BOUND_MINUS_INFTY),
            code_new_inst(i_bound_new, 1, (yyvsp[-1].code)));
      }
#line 2953 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 54:
#line 401 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_bound(BOUND_INFTY); }
#line 2959 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 55:
#line 402 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bound_new, 1, (yyvsp[0].code)); }
#line 2965 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 56:
#line 403 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_bound(BOUND_INFTY); }
#line 2971 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 57:
#line 404 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code),
            code_new_inst(i_bound_new, 1, (yyvsp[-3].code)),
            code_new_bound(BOUND_INFTY));
      }
#line 2981 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 58:
#line 409 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code),
            code_new_bound(BOUND_INFTY),
            code_new_inst(i_bound_new, 1, (yyvsp[-1].code)));
      }
#line 2991 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 59:
#line 417 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb(numb_new_integer(0)); }
#line 2997 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 60:
#line 418 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3003 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 61:
#line 422 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb(numb_copy(numb_unknown())); }
#line 3009 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 62:
#line 423 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3015 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 63:
#line 431 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_entry_list_new, 1, (yyvsp[0].code)); }
#line 3021 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 64:
#line 432 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_entry_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3029 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 65:
#line 435 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read, 1, (yyvsp[0].code)); }
#line 3035 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 66:
#line 436 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_list_matrix, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 3041 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 67:
#line 440 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_entry, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 3047 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 68:
#line 444 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 3053 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 69:
#line 448 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_matrix_list_new, 2, (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 3061 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 70:
#line 451 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_matrix_list_add, 3, (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 3069 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 71:
#line 463 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_object_min, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
      }
#line 3077 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 72:
#line 466 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_object_max, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
      }
#line 3085 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 73:
#line 476 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_subto, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
     }
#line 3093 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 74:
#line 482 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, (yyvsp[0].code), code_new_inst(i_nop, 0));
     }
#line 3101 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 75:
#line 485 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, (yyvsp[-2].code), (yyvsp[0].code));
     }
#line 3109 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 76:
#line 488 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code)),
           code_new_inst(i_nop, 0));
     }
#line 3119 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 77:
#line 493 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_expr_if_else, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_inst(i_nop, 0)),
           code_new_inst(i_nop, 0));
      }
#line 3129 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 78:
#line 498 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code)),
           code_new_inst(i_nop, 0));
      }
#line 3139 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 79:
#line 506 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint, 4, (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code), code_new_bits((yyvsp[0].bits)));
     }
#line 3147 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 80:
#line 509 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint, 4, (yyvsp[-3].code), (yyvsp[-2].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-1].code)),
           code_new_bits((yyvsp[0].bits)));
     }
#line 3157 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 81:
#line 514 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_constraint, 4,
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-2].code), (yyvsp[-1].code), code_new_bits((yyvsp[0].bits)));
     }
#line 3167 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 82:
#line 519 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
        (yyval.code) = code_new_inst(i_constraint, 4,
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-2].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-1].code)),
           code_new_bits((yyvsp[0].bits)));
     }
#line 3179 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 83:
#line 526 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code), (yyvsp[-4].code),
           code_new_contype(CON_RHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3188 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 84:
#line 530 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-5].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)), (yyvsp[-1].code), (yyvsp[-4].code),
           code_new_contype(CON_RHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3198 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 85:
#line 535 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-1].code), (yyvsp[-3].code), (yyvsp[-5].code), (yyvsp[-4].code),
           code_new_contype(CON_LHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3207 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 86:
#line 539 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-1].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-5].code), (yyvsp[-4].code),
           code_new_contype(CON_LHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3218 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 87:
#line 545 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3226 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 88:
#line 548 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3236 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 89:
#line 553 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3246 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 90:
#line 558 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3256 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 91:
#line 563 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3265 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 92:
#line 567 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3277 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 93:
#line 574 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3289 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 94:
#line 581 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3300 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 95:
#line 587 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3311 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 96:
#line 593 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3322 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 97:
#line 599 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3332 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 98:
#line 604 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3345 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 99:
#line 612 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3359 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 100:
#line 621 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3373 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 101:
#line 630 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3386 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 102:
#line 638 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3401 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 103:
#line 649 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3409 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 104:
#line 652 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), 
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3418 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 105:
#line 656 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), 
            (yyvsp[-4].code), (yyvsp[-3].code), code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3428 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 106:
#line 661 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code), 
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3438 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 107:
#line 669 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_ne, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3444 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 108:
#line 670 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_ne, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3452 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 109:
#line 673 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_ne, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3460 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 110:
#line 676 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_eq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3466 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 111:
#line 677 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_eq, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3474 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 112:
#line 680 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_eq, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3482 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 113:
#line 683 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_le, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3488 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 114:
#line 684 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_le, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3496 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 115:
#line 687 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_le, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3504 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 116:
#line 690 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_ge, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3510 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 117:
#line 691 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_ge, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3518 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 118:
#line 694 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_ge, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3526 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 119:
#line 697 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_lt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3532 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 120:
#line 698 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_lt, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3540 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 121:
#line 701 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_lt, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3548 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 122:
#line 704 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_gt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3554 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 123:
#line 705 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_gt, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3562 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 124:
#line 708 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vbool_gt, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3570 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 125:
#line 711 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_and, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3576 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 126:
#line 712 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_or,  2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3582 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 127:
#line 713 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_xor, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3588 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 128:
#line 714 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vbool_not, 1, (yyvsp[0].code)); }
#line 3594 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 129:
#line 715 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 3600 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 130:
#line 719 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = 0; }
#line 3606 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 131:
#line 720 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = (yyvsp[-2].bits) | (yyvsp[0].bits); }
#line 3612 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 132:
#line 724 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = LP_FLAG_CON_SCALE; }
#line 3618 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 133:
#line 725 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = LP_FLAG_CON_SEPAR; }
#line 3624 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 134:
#line 726 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = LP_FLAG_CON_CHECK; }
#line 3630 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 135:
#line 727 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.bits) = LP_FLAG_CON_INDIC; }
#line 3636 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 136:
#line 731 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_contype(CON_RHS); }
#line 3642 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 137:
#line 732 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_contype(CON_LHS); }
#line 3648 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 138:
#line 733 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_contype(CON_EQUAL); }
#line 3654 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 139:
#line 737 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3660 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 140:
#line 738 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_add, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3666 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 141:
#line 739 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_sub, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3672 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 142:
#line 740 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_const, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3678 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 143:
#line 741 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_term_sub, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3686 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 144:
#line 744 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_const, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 3692 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 145:
#line 745 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_term_sub, 2,
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)),
            (yyvsp[0].code));
      }
#line 3702 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 146:
#line 753 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3708 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 147:
#line 754 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[-2].code), (yyvsp[0].code));  }
#line 3714 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 148:
#line 755 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[-2].code),
            code_new_inst(i_expr_div, 2, code_new_numb(numb_new_integer(1)), (yyvsp[0].code)));
      }
#line 3723 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 149:
#line 759 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 3729 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 150:
#line 760 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_term_mul, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3735 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 152:
#line 765 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3741 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 153:
#line 766 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[0].code), code_new_numb(numb_new_integer(-1)));
      }
#line 3749 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 154:
#line 772 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3755 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 155:
#line 773 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_term_power, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3763 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 156:
#line 776 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_term_sum, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3771 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 157:
#line 782 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 3779 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 158:
#line 785 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vabs, 1, (yyvsp[-1].code)); }
#line 3785 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 159:
#line 786 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(-2)), (yyvsp[-1].code)); }
#line 3791 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 160:
#line 787 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(3)), (yyvsp[-1].code)); }
#line 3797 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 161:
#line 788 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(4)), (yyvsp[-1].code)); }
#line 3803 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 162:
#line 789 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(5)), (yyvsp[-1].code)); }
#line 3809 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 163:
#line 790 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(6)), (yyvsp[-1].code)); }
#line 3815 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 164:
#line 791 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(7)), (yyvsp[-1].code)); }
#line 3821 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 165:
#line 792 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(8)), (yyvsp[-1].code)); }
#line 3827 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 166:
#line 793 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(9)), (yyvsp[-1].code)); }
#line 3833 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 167:
#line 794 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(10)), (yyvsp[-1].code)); }
#line 3839 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 168:
#line 795 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vexpr_fun, 3, code_new_numb(numb_new_integer(11)), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3847 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 169:
#line 798 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_vexpr_fun, 3, code_new_numb(numb_new_integer(12)), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3855 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 170:
#line 801 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3863 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 171:
#line 804 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 3869 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 172:
#line 812 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_sos, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
     }
#line 3877 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 173:
#line 818 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_soset, 3, (yyvsp[0].code), (yyvsp[-3].code), (yyvsp[-2].code));
     }
#line 3885 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 174:
#line 821 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3893 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 175:
#line 827 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb(numb_new_integer(1)); }
#line 3899 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 176:
#line 828 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb(numb_new_integer(1)); }
#line 3905 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 177:
#line 829 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb(numb_new_integer(2)); }
#line 3911 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 178:
#line 837 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 3917 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 179:
#line 841 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3923 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 180:
#line 842 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3929 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 181:
#line 843 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3935 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 182:
#line 844 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3941 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 183:
#line 845 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_print, 1, code_new_symbol((yyvsp[0].sym))); }
#line 3947 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 184:
#line 846 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_check, 1, (yyvsp[0].code)); }
#line 3953 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 185:
#line 847 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code));
     }
#line 3961 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 186:
#line 857 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 3967 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 187:
#line 858 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_idxset_new, 3,
            code_new_inst(i_tuple_empty, 0), (yyvsp[0].code), code_new_inst(i_bool_true, 0));
      }
#line 3976 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 188:
#line 865 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_idxset_new, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3984 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 189:
#line 868 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_idxset_new, 3, (yyvsp[-2].code), (yyvsp[0].code), code_new_inst(i_bool_true, 0));
      }
#line 3992 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 191:
#line 875 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_union, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3998 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 192:
#line 876 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_union, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4006 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 193:
#line 879 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_sdiff, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4012 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 194:
#line 880 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_minus, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4020 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 195:
#line 883 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_minus, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4026 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 196:
#line 884 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_inter, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4032 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 198:
#line 888 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_union2, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4038 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 200:
#line 893 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_cross, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4044 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 201:
#line 894 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_cross, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4052 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 202:
#line 897 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_inter2, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4058 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 203:
#line 901 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4066 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 204:
#line 904 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4076 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 205:
#line 909 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_empty, 1, code_new_size(0)); }
#line 4082 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 206:
#line 910 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_range2, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4090 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 207:
#line 913 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_range2, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(1)));
      }
#line 4098 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 208:
#line 916 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_range, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4106 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 209:
#line 919 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_range, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(1)));
      }
#line 4114 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 210:
#line 922 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_argmin, 3, code_new_numb(numb_new_integer(1)), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4122 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 211:
#line 925 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_argmin, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4130 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 212:
#line 928 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_argmax, 3, code_new_numb(numb_new_integer(1)), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4138 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 213:
#line 931 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_argmax, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4146 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 214:
#line 934 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 4152 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 215:
#line 935 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_new_tuple, 1, (yyvsp[-1].code)); }
#line 4158 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 216:
#line 936 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_new_elem, 1, (yyvsp[-1].code)); }
#line 4164 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 217:
#line 937 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_idxset, 1, (yyvsp[-1].code)); }
#line 4170 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 218:
#line 938 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_expr, 2, (yyvsp[-3].code), (yyvsp[-1].code)); }
#line 4176 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 219:
#line 939 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_set_expr, 2, (yyvsp[-3].code), (yyvsp[-1].code)); }
#line 4182 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 220:
#line 940 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_set_proj, 2, (yyvsp[-3].code), (yyvsp[-1].code));
       }
#line 4190 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 221:
#line 943 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
          (yyval.code) = code_new_inst(i_set_indexset, 1, code_new_symbol((yyvsp[-1].sym)));
       }
#line 4198 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 222:
#line 946 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4206 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 223:
#line 952 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_new, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4212 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 224:
#line 953 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_param, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 4218 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 225:
#line 957 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_skip, 1, (yyvsp[0].code)); }
#line 4224 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 226:
#line 958 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_use, 1, (yyvsp[0].code)); }
#line 4230 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 227:
#line 959 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_comment, 1, (yyvsp[0].code)); }
#line 4236 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 228:
#line 960 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read_match, 1, (yyvsp[0].code)); }
#line 4242 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 229:
#line 964 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_tuple_list_new, 1, (yyvsp[0].code));
      }
#line 4250 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 230:
#line 967 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_tuple_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4258 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 231:
#line 970 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_read, 1, (yyvsp[0].code)); }
#line 4264 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 232:
#line 974 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_eq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4270 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 233:
#line 975 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_ne, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4276 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 234:
#line 976 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_gt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4282 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 235:
#line 977 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_ge, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4288 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 236:
#line 978 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_lt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4294 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 237:
#line 979 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_le, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4300 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 238:
#line 980 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_seq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4306 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 239:
#line 981 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_sneq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4312 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 240:
#line 982 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_subs, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 4318 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 241:
#line 983 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_sseq, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 4324 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 242:
#line 984 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_subs, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4330 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 243:
#line 985 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_sseq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4336 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 244:
#line 986 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_and, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4342 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 245:
#line 987 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_or,  2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4348 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 246:
#line 988 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_xor, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4354 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 247:
#line 989 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_not, 1, (yyvsp[0].code)); }
#line 4360 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 248:
#line 990 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 4366 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 249:
#line 991 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_is_elem, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4372 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 250:
#line 992 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_bool_exists, 1, (yyvsp[-1].code)); }
#line 4378 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 251:
#line 993 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4388 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 252:
#line 998 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
        (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
     }
#line 4396 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1004 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_tuple_empty, 0); }
#line 4402 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1005 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_tuple_new, 1, (yyvsp[-1].code));  }
#line 4408 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1009 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_tuple_empty, 0);
      }
#line 4416 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1012 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_tuple_new, 1, (yyvsp[-1].code));
      }
#line 4424 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1018 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_elem_list_new, 1, (yyvsp[0].code));
      }
#line 4432 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1021 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_elem_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4440 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1027 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 4446 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1028 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_add, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4452 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1029 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_sub, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4458 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1033 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 4464 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1034 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_mul, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4470 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1035 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_div, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4476 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1036 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_mod, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4482 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1037 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_intdiv, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4488 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1038 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_prod, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4496 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1045 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[0].code); }
#line 4502 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1046 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_neg, 1, (yyvsp[0].code)); }
#line 4508 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1051 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_pow, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4514 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1052 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_sum, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4522 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1055 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_min, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4530 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1058 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_max, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4538 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1061 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_sglmin, 1, (yyvsp[-1].code));
         }
#line 4546 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1064 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_sglmax, 1, (yyvsp[-1].code));
      }
#line 4554 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1070 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_numb((yyvsp[0].numb)); }
#line 4560 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1071 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_strg((yyvsp[0].strg));  }
#line 4566 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1072 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_local_deref, 1, code_new_name((yyvsp[0].name)));
      }
#line 4574 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1075 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4582 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1078 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { 
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4590 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1081 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4600 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1086 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4610 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1091 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_fac, 1, (yyvsp[-1].code)); }
#line 4616 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1092 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_card, 1, (yyvsp[-1].code)); }
#line 4622 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1093 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_abs, 1, (yyvsp[-1].code)); }
#line 4628 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1094 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_sgn, 1, (yyvsp[-1].code)); }
#line 4634 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1095 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_round, 1, (yyvsp[-1].code)); }
#line 4640 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 290:
#line 1096 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_floor, 1, (yyvsp[-1].code)); }
#line 4646 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 291:
#line 1097 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_ceil, 1, (yyvsp[-1].code)); }
#line 4652 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 292:
#line 1098 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_log, 1, (yyvsp[-1].code)); }
#line 4658 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 293:
#line 1099 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_ln, 1, (yyvsp[-1].code)); }
#line 4664 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 294:
#line 1100 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_exp, 1, (yyvsp[-1].code)); }
#line 4670 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 295:
#line 1101 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_sqrt, 1, (yyvsp[-1].code)); }
#line 4676 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 296:
#line 1102 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_sin, 1, (yyvsp[-1].code)); }
#line 4682 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 297:
#line 1103 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_cos, 1, (yyvsp[-1].code)); }
#line 4688 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 298:
#line 1104 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_tan, 1, (yyvsp[-1].code)); }
#line 4694 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1105 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_asin, 1, (yyvsp[-1].code)); }
#line 4700 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1106 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_acos, 1, (yyvsp[-1].code)); }
#line 4706 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 301:
#line 1107 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_atan, 1, (yyvsp[-1].code)); }
#line 4712 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1109 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = (yyvsp[-1].code); }
#line 4718 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1110 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    { (yyval.code) = code_new_inst(i_expr_length, 1, (yyvsp[-1].code)); }
#line 4724 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1111 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_substr, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4732 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1114 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_rand, 2, (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4740 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1117 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4748 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1120 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_ord, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4756 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1123 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_min2, 1, (yyvsp[-1].code));
      }
#line 4764 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1126 "src/zimpl/mmlparse2.y" /* yacc.c:1646  */
    {
         (yyval.code) = code_new_inst(i_expr_max2, 1, (yyvsp[-1].code));
      }
#line 4772 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
    break;


#line 4776 "src/zimpl/mmlparse2.c" /* yacc.c:1646  */
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

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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
                  yystos[*yyssp], yyvsp);
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
