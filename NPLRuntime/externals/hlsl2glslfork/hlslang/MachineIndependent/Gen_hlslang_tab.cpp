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
#line 8 "hlslang.y" /* yacc.c:339  */


/* Based on:
ANSI C Yacc grammar

In 1985, Jeff Lee published his Yacc grammar (which is accompanied by a 
matching Lex specification) for the April 30, 1985 draft version of the 
ANSI C standard.  Tom Stockfisch reposted it to net.sources in 1987; that
original, as mentioned in the answer to question 17.25 of the comp.lang.c
FAQ, can be ftp'ed from ftp.uu.net, file usenet/net.sources/ansi.c.grammar.Z.
 
I intend to keep this version as close to the current C Standard grammar as 
possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

#include "SymbolTable.h"
#include "ParseHelper.h"
#include "../../include/hlsl2glsl.h"

extern void yyerror(TParseContext&, const char*);

#define FRAG_ONLY(S, L) {                                                       \
    if (parseContext.language != EShLangFragment) {                             \
        parseContext.error(L, " supported in fragment shaders only ", S, "", "");          \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define NONSQUARE_MATRIX_CHECK(S, L) { \
	if (parseContext.targetVersion < ETargetGLSL_120) { \
		parseContext.error(L, " not supported in pre-GLSL1.20", S, "", ""); \
		parseContext.recover(); \
	} \
}

#define UNSUPPORTED_FEATURE(S, L) {                                                       \
    parseContext.error(L, " not supported ", S, "", "");              \
    parseContext.recover();                                                            \
}

#define SET_BASIC_TYPE(RES,PAR,T,PREC) \
	TQualifier qual = parseContext.getDefaultQualifier(); \
	(RES).setBasic(T, qual, (PAR).line); \
	(RES).precision = PREC



#line 116 "hlslang.tab.c" /* yacc.c:339  */

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
   by #include "hlslang.tab.h".  */
#ifndef YY_YY_HLSLANG_TAB_H_INCLUDED
# define YY_YY_HLSLANG_TAB_H_INCLUDED
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
    CONST_QUAL = 258,
    STATIC_QUAL = 259,
    BOOL_TYPE = 260,
    FLOAT_TYPE = 261,
    INT_TYPE = 262,
    STRING_TYPE = 263,
    FIXED_TYPE = 264,
    HALF_TYPE = 265,
    BREAK = 266,
    CONTINUE = 267,
    DO = 268,
    ELSE = 269,
    FOR = 270,
    IF = 271,
    DISCARD = 272,
    RETURN = 273,
    BVEC2 = 274,
    BVEC3 = 275,
    BVEC4 = 276,
    IVEC2 = 277,
    IVEC3 = 278,
    IVEC4 = 279,
    VEC2 = 280,
    VEC3 = 281,
    VEC4 = 282,
    HVEC2 = 283,
    HVEC3 = 284,
    HVEC4 = 285,
    FVEC2 = 286,
    FVEC3 = 287,
    FVEC4 = 288,
    MATRIX2x2 = 289,
    MATRIX2x3 = 290,
    MATRIX2x4 = 291,
    MATRIX3x2 = 292,
    MATRIX3x3 = 293,
    MATRIX3x4 = 294,
    MATRIX4x2 = 295,
    MATRIX4x3 = 296,
    MATRIX4x4 = 297,
    HMATRIX2x2 = 298,
    HMATRIX2x3 = 299,
    HMATRIX2x4 = 300,
    HMATRIX3x2 = 301,
    HMATRIX3x3 = 302,
    HMATRIX3x4 = 303,
    HMATRIX4x2 = 304,
    HMATRIX4x3 = 305,
    HMATRIX4x4 = 306,
    FMATRIX2x2 = 307,
    FMATRIX2x3 = 308,
    FMATRIX2x4 = 309,
    FMATRIX3x2 = 310,
    FMATRIX3x3 = 311,
    FMATRIX3x4 = 312,
    FMATRIX4x2 = 313,
    FMATRIX4x3 = 314,
    FMATRIX4x4 = 315,
    IN_QUAL = 316,
    OUT_QUAL = 317,
    INOUT_QUAL = 318,
    UNIFORM = 319,
    STRUCT = 320,
    VOID_TYPE = 321,
    WHILE = 322,
    SAMPLER1D = 323,
    SAMPLER2D = 324,
    SAMPLER3D = 325,
    SAMPLERCUBE = 326,
    SAMPLER1DSHADOW = 327,
    SAMPLER2DSHADOW = 328,
    SAMPLER2DARRAY = 329,
    SAMPLERRECTSHADOW = 330,
    SAMPLERRECT = 331,
    SAMPLER2D_HALF = 332,
    SAMPLER2D_FLOAT = 333,
    SAMPLERCUBE_HALF = 334,
    SAMPLERCUBE_FLOAT = 335,
    SAMPLERGENERIC = 336,
    VECTOR = 337,
    MATRIX = 338,
    REGISTER = 339,
    TEXTURE = 340,
    SAMPLERSTATE = 341,
    IDENTIFIER = 342,
    TYPE_NAME = 343,
    FLOATCONSTANT = 344,
    INTCONSTANT = 345,
    BOOLCONSTANT = 346,
    STRINGCONSTANT = 347,
    FIELD_SELECTION = 348,
    LEFT_OP = 349,
    RIGHT_OP = 350,
    INC_OP = 351,
    DEC_OP = 352,
    LE_OP = 353,
    GE_OP = 354,
    EQ_OP = 355,
    NE_OP = 356,
    AND_OP = 357,
    OR_OP = 358,
    XOR_OP = 359,
    MUL_ASSIGN = 360,
    DIV_ASSIGN = 361,
    ADD_ASSIGN = 362,
    MOD_ASSIGN = 363,
    LEFT_ASSIGN = 364,
    RIGHT_ASSIGN = 365,
    AND_ASSIGN = 366,
    XOR_ASSIGN = 367,
    OR_ASSIGN = 368,
    SUB_ASSIGN = 369,
    LEFT_PAREN = 370,
    RIGHT_PAREN = 371,
    LEFT_BRACKET = 372,
    RIGHT_BRACKET = 373,
    LEFT_BRACE = 374,
    RIGHT_BRACE = 375,
    DOT = 376,
    COMMA = 377,
    COLON = 378,
    EQUAL = 379,
    SEMICOLON = 380,
    BANG = 381,
    DASH = 382,
    TILDE = 383,
    PLUS = 384,
    STAR = 385,
    SLASH = 386,
    PERCENT = 387,
    LEFT_ANGLE = 388,
    RIGHT_ANGLE = 389,
    VERTICAL_BAR = 390,
    CARET = 391,
    AMPERSAND = 392,
    QUESTION = 393
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 57 "hlslang.y" /* yacc.c:355  */

    struct {
        TSourceLoc line;
        union {
            TString *string;
            float f;
            int i;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TSourceLoc line;
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
			TIntermTyped* intermDeclaration;
        };
        union {
            TPublicType type;
            TQualifier qualifier;
            TFunction* function;
            TParameter param;
            TTypeLine typeLine;
            TTypeList* typeList;
	    TAnnotation* ann;
	    TTypeInfo* typeInfo;
        };
    } interm;

#line 329 "hlslang.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (TParseContext& parseContext);

#endif /* !YY_YY_HLSLANG_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 91 "hlslang.y" /* yacc.c:358  */

    extern int yylex(YYSTYPE*, TParseContext&);

#line 348 "hlslang.tab.c" /* yacc.c:358  */

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
#define YYFINAL  106
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2692

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  139
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  93
/* YYNRULES -- Number of rules.  */
#define YYNRULES  325
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  497

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   393

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
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   175,   175,   213,   216,   221,   226,   231,   237,   240,
     304,   307,   394,   404,   417,   431,   547,   550,   568,   572,
     579,   583,   590,   599,   611,   619,   646,   658,   668,   671,
     681,   691,   709,   744,   745,   746,   747,   753,   754,   755,
     756,   760,   761,   762,   766,   767,   768,   772,   773,   774,
     775,   776,   780,   781,   782,   786,   787,   791,   792,   796,
     797,   801,   802,   806,   807,   811,   812,   816,   817,   832,
     833,   847,   848,   849,   850,   851,   852,   853,   854,   855,
     856,   857,   861,   864,   875,   883,   884,   888,   921,   958,
     961,   968,   976,   997,  1029,  1040,  1053,  1065,  1077,  1095,
    1124,  1129,  1139,  1144,  1154,  1157,  1160,  1163,  1169,  1176,
    1179,  1197,  1221,  1249,  1276,  1309,  1343,  1346,  1364,  1387,
    1413,  1439,  1469,  1517,  1520,  1537,  1540,  1543,  1546,  1549,
    1557,  1560,  1575,  1578,  1581,  1584,  1587,  1590,  1593,  1604,
    1615,  1626,  1630,  1634,  1638,  1642,  1646,  1650,  1654,  1658,
    1662,  1666,  1670,  1674,  1678,  1682,  1686,  1690,  1695,  1700,
    1705,  1709,  1714,  1719,  1724,  1728,  1732,  1737,  1742,  1747,
    1751,  1756,  1761,  1766,  1770,  1774,  1779,  1784,  1789,  1793,
    1798,  1803,  1808,  1812,  1815,  1818,  1821,  1824,  1827,  1830,
    1833,  1836,  1839,  1842,  1845,  1848,  1851,  1854,  1857,  1861,
    1873,  1883,  1891,  1894,  1909,  1942,  1946,  1952,  1957,  1963,
    1973,  1989,  1990,  1991,  1995,  1999,  2000,  2006,  2007,  2008,
    2009,  2010,  2014,  2015,  2015,  2015,  2023,  2024,  2029,  2032,
    2040,  2043,  2049,  2050,  2054,  2062,  2066,  2076,  2081,  2098,
    2098,  2103,  2103,  2110,  2110,  2123,  2126,  2132,  2135,  2141,
    2145,  2152,  2159,  2166,  2173,  2193,  2204,  2208,  2215,  2218,
    2221,  2225,  2225,  2314,  2317,  2324,  2328,  2332,  2336,  2343,
    2347,  2353,  2357,  2364,  2370,  2371,  2372,  2373,  2374,  2375,
    2376,  2377,  2378,  2379,  2380,  2381,  2382,  2383,  2384,  2385,
    2386,  2387,  2388,  2389,  2390,  2394,  2395,  2396,  2397,  2401,
    2404,  2407,  2413,  2417,  2418,  2422,  2426,  2432,  2436,  2437,
    2438,  2439,  2440,  2441,  2442,  2443,  2447,  2452,  2457,  2458,
    2462,  2463,  2464,  2465,  2466,  2467
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CONST_QUAL", "STATIC_QUAL", "BOOL_TYPE",
  "FLOAT_TYPE", "INT_TYPE", "STRING_TYPE", "FIXED_TYPE", "HALF_TYPE",
  "BREAK", "CONTINUE", "DO", "ELSE", "FOR", "IF", "DISCARD", "RETURN",
  "BVEC2", "BVEC3", "BVEC4", "IVEC2", "IVEC3", "IVEC4", "VEC2", "VEC3",
  "VEC4", "HVEC2", "HVEC3", "HVEC4", "FVEC2", "FVEC3", "FVEC4",
  "MATRIX2x2", "MATRIX2x3", "MATRIX2x4", "MATRIX3x2", "MATRIX3x3",
  "MATRIX3x4", "MATRIX4x2", "MATRIX4x3", "MATRIX4x4", "HMATRIX2x2",
  "HMATRIX2x3", "HMATRIX2x4", "HMATRIX3x2", "HMATRIX3x3", "HMATRIX3x4",
  "HMATRIX4x2", "HMATRIX4x3", "HMATRIX4x4", "FMATRIX2x2", "FMATRIX2x3",
  "FMATRIX2x4", "FMATRIX3x2", "FMATRIX3x3", "FMATRIX3x4", "FMATRIX4x2",
  "FMATRIX4x3", "FMATRIX4x4", "IN_QUAL", "OUT_QUAL", "INOUT_QUAL",
  "UNIFORM", "STRUCT", "VOID_TYPE", "WHILE", "SAMPLER1D", "SAMPLER2D",
  "SAMPLER3D", "SAMPLERCUBE", "SAMPLER1DSHADOW", "SAMPLER2DSHADOW",
  "SAMPLER2DARRAY", "SAMPLERRECTSHADOW", "SAMPLERRECT", "SAMPLER2D_HALF",
  "SAMPLER2D_FLOAT", "SAMPLERCUBE_HALF", "SAMPLERCUBE_FLOAT",
  "SAMPLERGENERIC", "VECTOR", "MATRIX", "REGISTER", "TEXTURE",
  "SAMPLERSTATE", "IDENTIFIER", "TYPE_NAME", "FLOATCONSTANT",
  "INTCONSTANT", "BOOLCONSTANT", "STRINGCONSTANT", "FIELD_SELECTION",
  "LEFT_OP", "RIGHT_OP", "INC_OP", "DEC_OP", "LE_OP", "GE_OP", "EQ_OP",
  "NE_OP", "AND_OP", "OR_OP", "XOR_OP", "MUL_ASSIGN", "DIV_ASSIGN",
  "ADD_ASSIGN", "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
  "XOR_ASSIGN", "OR_ASSIGN", "SUB_ASSIGN", "LEFT_PAREN", "RIGHT_PAREN",
  "LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DOT",
  "COMMA", "COLON", "EQUAL", "SEMICOLON", "BANG", "DASH", "TILDE", "PLUS",
  "STAR", "SLASH", "PERCENT", "LEFT_ANGLE", "RIGHT_ANGLE", "VERTICAL_BAR",
  "CARET", "AMPERSAND", "QUESTION", "$accept", "variable_identifier",
  "primary_expression", "postfix_expression", "int_expression",
  "function_call", "function_call_or_method", "function_call_generic",
  "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "unary_expression", "unary_operator",
  "mul_expression", "add_expression", "shift_expression", "rel_expression",
  "eq_expression", "and_expression", "xor_expression", "or_expression",
  "log_and_expression", "log_xor_expression", "log_or_expression",
  "cond_expression", "assign_expression", "assignment_operator",
  "expression", "const_expression", "declaration", "function_prototype",
  "function_declarator", "function_header_with_parameters",
  "function_header", "parameter_declarator", "parameter_declaration",
  "parameter_qualifier", "parameter_type_specifier",
  "init_declarator_list", "single_declaration", "fully_specified_type",
  "type_qualifier", "type_specifier", "type_specifier_nonarray",
  "struct_specifier", "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "initializer",
  "declaration_statement", "statement", "simple_statement",
  "compound_statement", "$@1", "$@2", "statement_no_new_scope",
  "compound_statement_no_new_scope", "statement_list",
  "expression_statement", "selection_statement",
  "selection_rest_statement", "condition", "iteration_statement", "$@3",
  "$@4", "$@5", "for_init_statement", "conditionopt", "for_rest_statement",
  "jump_statement", "translation_unit", "external_declaration",
  "function_definition", "$@6", "initialization_list", "initializer_list",
  "annotation", "annotation_list", "annotation_item", "ann_type",
  "ann_literal", "ann_numerical_constant", "ann_literal_constructor",
  "ann_value_list", "ann_literal_init_list", "register_specifier",
  "semantic", "type_info", "sampler_initializer", "sampler_init_list",
  "sampler_init_item", YY_NULLPTR
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
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393
};
# endif

#define YYPACT_NINF -387

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-387)))

#define YYTABLE_NINF -90

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2208,    86,    36,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,   -12,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,   -14,  -387,  -387,
    -387,  -387,    32,    48,    38,    67,    11,  -387,    88,  2604,
    -387,    54,  -387,  1191,  -387,  -387,  -387,  -387,    72,  2604,
      45,  -387,    85,   118,    84,  -387,  -387,  -387,  -387,  2604,
     137,   202,  -387,    64,  -387,  1984,  -387,  -387,  2604,   207,
    2315,  -387,   174,   177,   189,   457,  -387,   225,  -387,  -387,
    -387,   227,  2604,    44,  -387,  1534,    39,     1,  -387,   182,
     -85,   192,   203,  -387,  -387,  -387,  -387,  1984,  1984,  1984,
    -387,  -387,  -387,  -387,  -387,  -387,    42,  -387,  -387,  -387,
     201,    46,  2095,   206,  -387,  1984,   115,   155,    58,   -22,
      40,   185,   187,   190,   222,   223,   -34,  -387,   208,  -387,
    2417,    57,    71,  -387,  -387,  -387,   238,   239,   240,   209,
     210,  -387,   216,   217,   215,  1648,   218,   221,  -387,  -387,
     196,  -387,  -387,    83,  -387,    32,   228,  -387,  -387,  -387,
    -387,   584,  -387,  -387,  -387,  -387,  -387,    26,  -387,  -387,
    1759,   220,   -32,   224,   230,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,    35,  -387,
     259,  -387,   263,  -387,   182,  1298,  -387,  -387,    66,   113,
    -387,  -387,  1984,  2519,  -387,  -387,  1984,   232,  -387,  -387,
    -387,  1984,  1984,  1984,  1984,  1984,  1984,  1984,  1984,  1984,
    1984,  1984,  1984,  1984,  1984,  1984,  1984,  1984,  1984,  1984,
    1984,  -387,  -387,  1984,   262,   207,  -387,   219,   226,   229,
    -387,  -387,   711,  -387,  1984,  -387,  -387,   114,  -387,  -387,
     711,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  1984,  1984,  -387,  -387,  -387,  1984,   156,  1298,
    -387,   -32,   233,  1298,   231,   -32,   265,  -387,  -387,   234,
    -387,   235,  1873,  -387,  -387,  -387,  -387,  -387,  1984,   241,
     242,  -387,   246,  -387,  -387,  -387,  -387,  -387,   115,   115,
     155,   155,    58,    58,    58,    58,   -22,   -22,    40,   185,
     187,   190,   222,   223,   136,   244,  -387,  -387,  -387,  -387,
    -387,   283,   965,    68,  -387,  1078,   711,  -387,  -387,   249,
    -387,  -387,   245,   -32,  -387,  1298,   250,   252,   247,    -3,
    -387,  -387,   122,  -387,  -387,  1984,   248,   255,  -387,  -387,
    1078,   711,   242,   269,   257,   237,   254,  1298,   256,  -387,
    1298,  -387,  -387,  -387,  -387,  -387,   160,   264,   253,  -387,
    -387,  -387,   258,   260,  -387,    31,  -387,  -387,  1409,  -387,
     278,  1984,  -387,   261,   267,   367,  -387,   266,   838,  -387,
     298,  -387,  1298,  -387,  -387,   163,   160,  -387,   -41,    -9,
    -387,  -387,  -387,  -387,  -387,  -387,    70,  1984,   838,   711,
    1298,  -387,  -387,  -387,  -387,  -387,  -387,   160,    73,   268,
     300,   301,   270,   302,   304,   271,   242,  -387,  -387,  -387,
    -387,  -387,  -387,   276,   272,  -387,   281,   273,  -387,   274,
     277,   279,   280,  -387,  -387,  -387,  -387
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,   125,   126,   137,   133,   136,   135,   134,   150,   151,
     152,   153,   154,   155,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     129,     0,   132,   185,   186,   189,   190,   195,   196,   197,
     194,   193,   187,   188,   191,   192,   184,     0,   183,   199,
     260,   259,   261,     0,    90,   104,     0,   109,   116,     0,
     123,   130,   198,     0,   256,   258,   128,   127,     0,     0,
       0,    85,     0,    87,   104,   105,   106,   107,    91,     0,
     104,     0,    86,   308,   124,     0,     1,   257,     0,     0,
       0,   202,     0,     0,     0,     0,   262,     0,    92,   101,
     103,   108,     0,   308,    93,     0,     0,     0,   311,   310,
     309,   117,     2,     5,     4,     6,    27,     0,     0,     0,
      35,    34,    36,    33,     3,     8,    28,    10,    15,    16,
       0,     0,    21,     0,    37,     0,    41,    44,    47,    52,
      55,    57,    59,    61,    63,    65,    67,    84,     0,    25,
       0,   207,     0,   205,   201,   203,     0,     0,     0,     0,
       0,   241,     0,     0,     0,     0,     0,   223,   228,   232,
      37,    69,    82,     0,   214,     0,   123,   217,   230,   216,
     215,     0,   218,   219,   220,   221,    88,    94,   100,   102,
       0,   110,   308,     0,     0,   307,   278,   274,   277,   279,
     276,   275,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   269,     0,   271,
       0,   314,     0,   312,   313,     0,    29,    30,     0,   130,
      12,    13,     0,     0,    19,    18,     0,   132,    22,    24,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   131,   200,     0,     0,     0,   204,     0,     0,     0,
     252,   251,     0,   243,     0,   255,   253,     0,   239,   222,
       0,    72,    73,    75,    74,    77,    78,    79,    80,    81,
      76,    71,     0,     0,   233,   229,   231,     0,     0,     0,
      96,   308,     0,     0,   118,   308,     0,   270,   272,     0,
     315,     0,     0,   211,   122,   212,   213,     7,     0,     0,
      14,    26,    11,    17,    23,    38,    39,    40,    43,    42,
      45,    46,    50,    51,    48,    49,    53,    54,    56,    58,
      60,    62,    64,    66,     0,     0,   208,   206,   140,   138,
     139,     0,     0,     0,   254,     0,   224,    70,    83,     0,
      97,    95,   111,   308,   115,     0,   119,     0,     0,     0,
     265,   266,     0,    32,     9,     0,   209,     0,   246,   245,
     248,     0,   237,     0,     0,     0,    98,     0,   112,   120,
       0,   306,   301,   299,   300,   296,     0,     0,     0,   295,
     297,   298,     0,     0,   317,     0,   318,   263,     0,    68,
       0,     0,   247,     0,     0,   236,   234,     0,     0,   225,
       0,   113,     0,   121,   303,     0,     0,   273,     0,     0,
     316,   319,   264,   267,   268,   210,     0,   249,     0,     0,
       0,   227,   240,   226,    99,   114,   305,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   250,   244,   235,   238,
     304,   302,   323,     0,     0,   320,     0,     0,   242,     0,
       0,     0,     0,   325,   324,   322,   321
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -387,  -387,  -387,  -387,  -387,  -387,  -387,   141,  -387,  -387,
    -387,  -387,   -52,  -387,    23,    24,   -36,    25,   124,   128,
     132,   133,   131,   134,  -387,   -89,  -147,  -387,  -138,  -110,
      37,    49,  -387,  -387,  -387,   289,   318,   314,   293,  -387,
    -387,  -303,    50,     0,   282,  -387,   308,   -75,  -387,   135,
    -305,    47,  -198,  -304,  -387,  -387,  -387,   -40,   325,   123,
      52,  -387,  -387,    22,  -387,  -387,  -387,  -387,  -387,  -387,
    -387,  -387,  -387,   342,  -387,  -387,  -315,  -387,  -117,  -387,
     188,    41,  -387,  -386,  -387,   -19,  -387,  -111,  -387,  -119,
    -387,  -387,     3
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   144,   145,   146,   339,   147,   148,   149,   150,   151,
     152,   153,   190,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   191,   192,   312,   193,   168,
     194,   195,    73,    74,    75,   119,    98,    99,   120,    76,
      77,    78,    79,   169,    81,    82,   110,   111,   172,   173,
     334,   197,   198,   199,   200,   300,   405,   462,   463,   201,
     202,   203,   436,   404,   204,   375,   292,   372,   400,   433,
     434,   205,    83,    84,    85,    92,   335,   392,   128,   238,
     239,   240,   418,   444,   420,   445,   421,   129,   130,   131,
     336,   425,   426
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      80,   248,   419,   316,   211,   258,   216,   217,   218,   219,
     220,   221,   241,   243,   381,   213,   167,   391,   384,   244,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   175,   167,    71,   242,    87,
     216,   217,   218,   219,   220,   221,   469,   297,   127,    72,
     112,   113,   114,   154,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   279,
       1,     2,   403,   154,   470,    88,   268,   269,   472,   104,
     409,   480,   422,    80,   423,   246,   247,     1,     2,   109,
      86,   126,   471,   324,   371,   175,   320,   403,   333,   121,
     322,   127,   441,   260,   280,   443,   473,    89,   109,   344,
     109,   270,   271,   454,   340,   196,   422,   424,   423,    90,
      71,   167,   121,   214,   474,   100,   215,   330,    95,    96,
      97,    50,    72,   101,   461,   237,   102,   465,   250,   251,
     272,   273,   364,   317,   100,    95,    96,    97,    50,   318,
     319,   450,   266,   267,   461,   479,   373,    91,   154,   252,
      94,   210,   255,   253,    93,   377,   378,   126,   256,   327,
     109,   105,   333,   365,   283,   103,   333,   127,   316,   124,
     284,   125,   337,   -89,   401,   390,   475,   126,   313,   481,
     313,   108,   313,   285,   167,   467,   286,   127,    95,    96,
      97,   196,   382,   435,   115,   313,   386,   379,   314,   345,
     346,   347,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   167,   338,
     105,   154,   352,   353,   354,   355,   313,   402,   333,   374,
     214,   117,   427,   380,   428,   261,   262,   263,   429,   412,
     413,   414,   216,   217,   218,   219,   220,   221,   313,   395,
     333,   478,   402,   333,   408,   154,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   453,   264,   466,   265,   467,   393,   348,   349,   123,
     350,   351,   196,   456,   171,   333,   176,   356,   357,   177,
     196,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   178,   206,   333,   207,   127,   245,   254,   -26,   476,
     311,   259,   274,   275,   277,   276,   281,   278,   287,   288,
     289,   293,   294,   298,   290,   291,   412,   413,   414,   415,
     295,   299,   325,   -25,   323,   326,   329,   214,   -20,   366,
     397,   383,   387,   368,   389,   385,   437,   439,   388,   394,
     369,   -27,   396,   370,   313,   455,   416,   406,   411,   407,
     431,   430,   196,   438,   410,   196,   196,   440,   447,   446,
     442,   459,   448,   458,   449,   464,   457,   483,   484,   486,
     460,   487,   489,   482,   343,   485,   488,   491,   358,   493,
     196,   196,   494,   359,   495,   496,   490,   492,   360,   362,
     361,   208,   118,   363,   122,   209,   170,   116,   477,   398,
     367,   249,   432,   376,   399,   107,   328,   468,   451,   417,
       0,     0,     0,     0,     0,     0,     0,     0,   196,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   196,   196,
       1,     2,     3,     4,     5,     0,     6,     7,   179,   180,
     181,     0,   182,   183,   184,   185,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,     0,
       0,    50,    51,    52,   186,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,     0,    68,     0,   132,    69,   133,   134,   135,     0,
     136,     0,     0,   137,   138,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   139,     0,     0,     0,   187,   188,     0,     0,
       0,     0,   189,   140,   141,   142,   143,     1,     2,     3,
       4,     5,     0,     6,     7,   179,   180,   181,     0,   182,
     183,   184,   185,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,     0,     0,    50,    51,
      52,   186,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    68,
       0,   132,    69,   133,   134,   135,     0,   136,     0,     0,
     137,   138,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   139,
       0,     0,     0,   187,   315,     0,     0,     0,     0,   189,
     140,   141,   142,   143,     1,     2,     3,     4,     5,     0,
       6,     7,   179,   180,   181,     0,   182,   183,   184,   185,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,     0,     0,    50,    51,    52,   186,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,     0,    68,     0,   132,    69,
     133,   134,   135,     0,   136,     0,     0,   137,   138,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,     0,
     187,     0,     0,     0,     0,     0,   189,   140,   141,   142,
     143,     1,     2,     3,     4,     5,     0,     6,     7,   179,
     180,   181,     0,   182,   183,   184,   185,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
       0,     0,    50,    51,    52,   186,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,     0,   132,    69,   133,   134,   135,
       0,   136,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   139,     0,     0,     0,   115,     0,     0,
       0,     0,     0,   189,   140,   141,   142,   143,     1,     2,
       3,     4,     5,     0,     6,     7,     0,     0,     0,     0,
       0,     0,     0,     0,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,     0,     0,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
      68,     0,   132,    69,   133,   134,   135,     0,   136,     0,
       0,   137,   138,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     1,     2,     3,     4,     5,     0,     6,     7,     0,
     189,   140,   141,   142,   143,     0,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
       0,     0,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,     0,   132,    69,   133,   134,   135,
       0,   136,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   106,     0,   139,     1,     2,     3,     4,     5,     0,
       6,     7,     0,     0,   140,   141,   142,   143,     0,     0,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,     0,     0,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,     0,    68,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     4,     5,     0,     6,     7,     0,
       0,     0,     0,     0,     0,     0,    70,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
       0,     0,     0,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,   331,   132,    69,   133,   134,   135,
       0,   136,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   139,     3,     4,     5,   332,     6,     7,
       0,     0,     0,     0,   140,   141,   142,   143,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,     0,     0,     0,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,     0,    68,     0,   132,    69,   133,   134,
     135,     0,   136,     0,     0,   137,   138,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,     0,   332,   452,
       0,     0,     0,     0,     0,   140,   141,   142,   143,     3,
       4,     5,     0,     6,     7,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,     0,     0,     0,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    68,
       0,   132,    69,   133,   134,   135,     0,   136,     0,     0,
     137,   138,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   139,
       0,     0,   212,     3,     4,     5,     0,     6,     7,     0,
     140,   141,   142,   143,     0,     0,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
       0,     0,     0,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,     0,   132,    69,   133,   134,   135,
       0,   136,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   139,     3,     4,     5,     0,     6,     7,
       0,     0,     0,   296,   140,   141,   142,   143,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,     0,     0,     0,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,     0,    68,     0,   132,    69,   133,   134,
     135,     0,   136,     0,     0,   137,   138,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,   321,     3,     4,
       5,     0,     6,     7,     0,   140,   141,   142,   143,     0,
       0,     0,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,     0,     0,     0,    51,    52,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,     0,    68,     0,
     132,    69,   133,   134,   135,     0,   136,     0,     0,   137,
     138,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,     3,
       4,     5,   332,     6,     7,     0,     0,     0,     0,   140,
     141,   142,   143,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,     0,     0,     0,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    68,
       0,   132,    69,   133,   134,   135,     0,   136,     0,     0,
     137,   138,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   139,
       3,     4,     5,     0,     6,     7,     0,     0,     0,     0,
     140,   141,   142,   143,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,     0,     0,     0,
      51,   257,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
      68,     0,   132,    69,   133,   134,   135,     0,   136,     0,
       0,   137,   138,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     1,     2,     3,     4,     5,     0,     6,     7,     0,
       0,   140,   141,   142,   143,     0,     0,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
       0,     0,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,     4,     5,     0,     6,     7,     0,     0,     0,     0,
       0,     0,     0,    70,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,     0,     0,     0,
      51,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
      68,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     4,     5,     0,     6,     7,     0,     0,
       0,     0,     0,     0,     0,   174,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,     0,
       0,     0,    51,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,     0,    68,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,     4,     5,     0,     6,     7,
       0,     0,     0,     0,     0,     0,     0,   282,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,     0,     0,     0,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,     0,    68,     0,   341,    69,     0,     3,
       4,     5,   342,     6,     7,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,     0,     0,     0,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    68,
       0,     0,    69
};

static const yytype_int16 yycheck[] =
{
       0,   139,   388,   201,   123,   152,     5,     6,     7,     8,
       9,    10,   129,   130,   319,   125,   105,   332,   323,   130,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,   110,   125,     0,   123,     3,
       5,     6,     7,     8,     9,    10,    87,   185,   133,     0,
       5,     6,     7,   105,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,   103,
       3,     4,   375,   125,   115,    87,    98,    99,    87,    79,
     385,   467,    85,    83,    87,   137,   138,     3,     4,    89,
       4,   123,   133,   212,   292,   170,   207,   400,   245,    99,
     210,   133,   407,   155,   138,   410,   115,   119,   108,   256,
     110,   133,   134,   428,   252,   115,    85,   120,    87,   133,
      83,   210,   122,    84,   133,    75,    87,   244,    61,    62,
      63,    64,    83,   122,   438,   134,   125,   442,    96,    97,
     100,   101,   280,   117,    94,    61,    62,    63,    64,   123,
     124,   120,    94,    95,   458,   460,   294,   125,   210,   117,
     122,   117,   116,   121,   116,   312,   313,   123,   122,   134,
     170,   117,   319,   283,   117,    87,   323,   133,   376,   115,
     123,   117,   116,   116,   116,   332,   116,   123,   122,   116,
     122,   119,   122,   122,   283,   122,   125,   133,    61,    62,
      63,   201,   321,   401,   119,   122,   325,   317,   125,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   317,   116,
     117,   283,   268,   269,   270,   271,   122,   375,   385,   125,
      84,   123,   120,    87,   122,   130,   131,   132,   395,    89,
      90,    91,     5,     6,     7,     8,     9,    10,   122,   123,
     407,   459,   400,   410,   383,   317,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,   428,   127,   120,   129,   122,   338,   264,   265,    87,
     266,   267,   292,   431,    87,   442,   122,   272,   273,   122,
     300,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   122,    87,   460,    87,   133,   124,   116,   115,   457,
     124,   115,   137,   136,   102,   135,   118,   104,    90,    90,
      90,   115,   115,   115,   125,   125,    89,    90,    91,    92,
     125,   120,   118,   115,   124,   115,    87,    84,   116,    87,
      67,   118,    87,   134,   119,   124,    87,   120,   124,   118,
     134,   115,   118,   134,   122,    87,   119,   118,   116,   124,
     115,   123,   372,   116,   124,   375,   376,   123,   125,   115,
     124,    14,   124,   116,   124,    87,   125,    87,    87,    87,
     124,    87,   116,   125,   253,   125,   125,   116,   274,   125,
     400,   401,   125,   275,   125,   125,   134,   134,   276,   278,
     277,   122,    94,   279,   100,   122,   108,    92,   458,   372,
     285,   139,   400,   300,   372,    83,   238,   446,   425,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   438,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   458,   459,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    -1,
      -1,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      -1,    -1,    85,    -1,    87,    88,    89,    90,    91,    -1,
      93,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,   127,   128,   129,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    -1,    -1,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,   127,   128,   129,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    -1,    -1,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    -1,    -1,    85,    -1,    87,    88,
      89,    90,    91,    -1,    93,    -1,    -1,    96,    97,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,   125,   126,   127,   128,
     129,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    85,    -1,    87,    88,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,   125,   126,   127,   128,   129,     3,     4,
       5,     6,     7,    -1,     9,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    -1,    -1,
      85,    -1,    87,    88,    89,    90,    91,    -1,    93,    -1,
      -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,     3,     4,     5,     6,     7,    -1,     9,    10,    -1,
     125,   126,   127,   128,   129,    -1,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    85,    -1,    87,    88,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     0,    -1,   115,     3,     4,     5,     6,     7,    -1,
       9,    10,    -1,    -1,   126,   127,   128,   129,    -1,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    -1,    -1,    85,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,     6,     7,    -1,     9,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,     5,     6,     7,   119,     9,    10,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    -1,    -1,    85,    -1,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,   120,
      -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,     5,
       6,     7,    -1,     9,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,   118,     5,     6,     7,    -1,     9,    10,    -1,
     126,   127,   128,   129,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    -1,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    85,    -1,    87,    88,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,     5,     6,     7,    -1,     9,    10,
      -1,    -1,    -1,   125,   126,   127,   128,   129,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    -1,    -1,    85,    -1,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,    -1,   118,     5,     6,
       7,    -1,     9,    10,    -1,   126,   127,   128,   129,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    -1,    -1,    -1,    65,    66,
      -1,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    -1,    -1,    85,    -1,
      87,    88,    89,    90,    91,    -1,    93,    -1,    -1,    96,
      97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,     5,
       6,     7,   119,     9,    10,    -1,    -1,    -1,    -1,   126,
     127,   128,   129,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
       5,     6,     7,    -1,     9,    10,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,
      65,    66,    -1,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    -1,    -1,
      85,    -1,    87,    88,    89,    90,    91,    -1,    93,    -1,
      -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,     3,     4,     5,     6,     7,    -1,     9,    10,    -1,
      -1,   126,   127,   128,   129,    -1,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    85,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       5,     6,     7,    -1,     9,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,
      65,    66,    -1,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    -1,    -1,
      85,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,     6,     7,    -1,     9,    10,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    -1,
      -1,    -1,    65,    66,    -1,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      -1,    -1,    85,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     5,     6,     7,    -1,     9,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    -1,    -1,    -1,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    -1,    -1,    85,    -1,    87,    88,    -1,     5,
       6,     7,    93,     9,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,    65,
      66,    -1,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,    -1,    88
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     9,    10,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      64,    65,    66,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    85,    88,
     125,   169,   170,   171,   172,   173,   178,   179,   180,   181,
     182,   183,   184,   211,   212,   213,     4,     3,    87,   119,
     133,   125,   214,   116,   122,    61,    62,    63,   175,   176,
     181,   122,   125,    87,   182,   117,     0,   212,   119,   182,
     185,   186,     5,     6,     7,   119,   197,   123,   175,   174,
     177,   182,   176,    87,   115,   117,   123,   133,   217,   226,
     227,   228,    87,    89,    90,    91,    93,    96,    97,   115,
     126,   127,   128,   129,   140,   141,   142,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   168,   182,
     185,    87,   187,   188,   120,   186,   122,   122,   122,    11,
      12,    13,    15,    16,    17,    18,    67,   119,   120,   125,
     151,   164,   165,   167,   169,   170,   182,   190,   191,   192,
     193,   198,   199,   200,   203,   210,    87,    87,   174,   177,
     117,   228,   118,   168,    84,    87,     5,     6,     7,     8,
       9,    10,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,   134,   218,   219,
     220,   217,   123,   217,   226,   124,   151,   151,   167,   183,
      96,    97,   117,   121,   116,   116,   122,    66,   165,   115,
     151,   130,   131,   132,   127,   129,    94,    95,    98,    99,
     133,   134,   100,   101,   137,   136,   135,   102,   104,   103,
     138,   118,   120,   117,   123,   122,   125,    90,    90,    90,
     125,   125,   205,   115,   115,   125,   125,   167,   115,   120,
     194,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   124,   166,   122,   125,   120,   191,   117,   123,   124,
     226,   118,   168,   124,   228,   118,   115,   134,   219,    87,
     217,    86,   119,   165,   189,   215,   229,   116,   116,   143,
     167,    87,    93,   146,   165,   151,   151,   151,   153,   153,
     154,   154,   155,   155,   155,   155,   156,   156,   157,   158,
     159,   160,   161,   162,   167,   168,    87,   188,   134,   134,
     134,   191,   206,   167,   125,   204,   198,   165,   165,   168,
      87,   189,   228,   118,   189,   124,   228,    87,   124,   119,
     165,   215,   216,   151,   118,   123,   118,    67,   190,   199,
     207,   116,   167,   180,   202,   195,   118,   124,   228,   189,
     124,   116,    89,    90,    91,    92,   119,   220,   221,   222,
     223,   225,    85,    87,   120,   230,   231,   120,   122,   165,
     123,   115,   202,   208,   209,   191,   201,    87,   116,   120,
     123,   189,   124,   189,   222,   224,   115,   125,   124,   124,
     120,   231,   120,   165,   215,    87,   167,   125,   116,    14,
     124,   192,   196,   197,    87,   189,   120,   122,   224,    87,
     115,   133,    87,   115,   133,   116,   167,   196,   191,   189,
     222,   116,   125,    87,    87,   125,    87,    87,   125,   116,
     134,   116,   134,   125,   125,   125,   125
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   139,   140,   141,   141,   141,   141,   141,   142,   142,
     142,   142,   142,   142,   143,   144,   145,   145,   146,   146,
     147,   147,   148,   148,   149,   150,   150,   150,   151,   151,
     151,   151,   151,   152,   152,   152,   152,   153,   153,   153,
     153,   154,   154,   154,   155,   155,   155,   156,   156,   156,
     156,   156,   157,   157,   157,   158,   158,   159,   159,   160,
     160,   161,   161,   162,   162,   163,   163,   164,   164,   165,
     165,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   167,   167,   168,   169,   169,   170,   170,   171,
     171,   172,   172,   173,   174,   174,   174,   174,   174,   174,
     175,   175,   175,   175,   176,   176,   176,   176,   177,   178,
     178,   178,   178,   178,   178,   178,   179,   179,   179,   179,
     179,   179,   179,   180,   180,   181,   181,   181,   181,   181,
     182,   182,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     184,   184,   185,   185,   186,   187,   187,   188,   188,   188,
     188,   189,   189,   189,   190,   191,   191,   192,   192,   192,
     192,   192,   193,   194,   195,   193,   196,   196,   197,   197,
     198,   198,   199,   199,   200,   201,   201,   202,   202,   204,
     203,   205,   203,   206,   203,   207,   207,   208,   208,   209,
     209,   210,   210,   210,   210,   210,   211,   211,   212,   212,
     212,   214,   213,   215,   215,   216,   216,   216,   216,   217,
     217,   218,   218,   219,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   220,   220,   221,   221,   221,   221,   222,
     222,   222,   223,   224,   224,   225,   226,   227,   228,   228,
     228,   228,   228,   228,   228,   228,   229,   229,   230,   230,
     231,   231,   231,   231,   231,   231
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     3,     1,     4,
       1,     3,     2,     2,     1,     1,     1,     3,     2,     2,
       2,     1,     2,     3,     2,     1,     1,     1,     1,     2,
       2,     2,     4,     1,     1,     1,     1,     1,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     5,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     2,     2,     4,     1,
       1,     2,     3,     3,     2,     4,     3,     4,     5,     7,
       3,     2,     3,     2,     0,     1,     1,     1,     1,     1,
       4,     6,     7,     8,     9,     6,     1,     3,     5,     6,
       7,     8,     5,     1,     2,     1,     1,     2,     2,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     6,     6,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     4,     1,     2,     3,     1,     3,     1,     3,     4,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     0,     5,     1,     1,     2,     3,
       1,     2,     1,     2,     5,     3,     1,     1,     4,     0,
       6,     0,     8,     0,     7,     1,     1,     1,     0,     2,
       3,     2,     2,     2,     3,     2,     1,     2,     1,     1,
       1,     0,     3,     3,     4,     1,     1,     3,     3,     2,
       3,     1,     2,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     3,     3,     5,     2,     0,     1,
       1,     1,     2,     2,     2,     3,     4,     3,     1,     2,
       4,     6,     6,     4,     6,     6
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
      yyerror (parseContext, YY_("syntax error: cannot back up")); \
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
                  Type, Value, parseContext); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, TParseContext& parseContext)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (parseContext);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, TParseContext& parseContext)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parseContext);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, TParseContext& parseContext)
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
                                              , parseContext);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, parseContext); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, TParseContext& parseContext)
{
  YYUSE (yyvaluep);
  YYUSE (parseContext);
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
yyparse (TParseContext& parseContext)
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
      yychar = yylex (&yylval, parseContext);
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
#line 175 "hlslang.y" /* yacc.c:1646  */
    {
        // The symbol table search was done in the lexical phase
        const TSymbol* symbol = (yyvsp[0].lex).symbol;
        const TVariable* variable;
        if (symbol == 0) {
            parseContext.error((yyvsp[0].lex).line, "undeclared identifier", (yyvsp[0].lex).string->c_str(), "");
            parseContext.recover();
            TType type(EbtFloat, EbpUndefined);
            TVariable* fakeVariable = new TVariable((yyvsp[0].lex).string, type);
            parseContext.symbolTable.insert(*fakeVariable);
            variable = fakeVariable;
        } else {
            // This identifier can only be a variable type symbol 
            if (! symbol->isVariable()) {
                parseContext.error((yyvsp[0].lex).line, "variable expected", (yyvsp[0].lex).string->c_str(), "");
                parseContext.recover();
            }
            variable = static_cast<const TVariable*>(symbol);
        }

        // don't delete $1.string, it's used by error recovery, and the pool
        // pop will reclaim the memory
		
		if (variable->getType().getQualifier() == EvqConst && variable->constValue)
		{
			TIntermConstant* c = ir_add_constant(variable->getType(), (yyvsp[0].lex).line);
			c->copyValuesFrom(*variable->constValue);
			(yyval.interm.intermTypedNode) = c;
		}
		else
		{
			TIntermSymbol* sym = ir_add_symbol(variable, (yyvsp[0].lex).line);
			(yyval.interm.intermTypedNode) = sym;
		}
    }
#line 2333 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 213 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 2341 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 216 "hlslang.y" /* yacc.c:1646  */
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[0].lex).line);
		constant->setValue((yyvsp[0].lex).i);
		(yyval.interm.intermTypedNode) = constant;
    }
#line 2351 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 221 "hlslang.y" /* yacc.c:1646  */
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[0].lex).line);
		constant->setValue((yyvsp[0].lex).f);
		(yyval.interm.intermTypedNode) = constant;
    }
#line 2361 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 226 "hlslang.y" /* yacc.c:1646  */
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtBool, EbpUndefined, EvqConst), (yyvsp[0].lex).line);
		constant->setValue((yyvsp[0].lex).b);
		(yyval.interm.intermTypedNode) = constant;
    }
#line 2371 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 231 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermTypedNode);
    }
#line 2379 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 237 "hlslang.y" /* yacc.c:1646  */
    { 
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 2387 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 240 "hlslang.y" /* yacc.c:1646  */
    {
        if (!(yyvsp[-3].interm.intermTypedNode)) {
            parseContext.error((yyvsp[-2].lex).line, " left of '[' is null ", "expression", "");
            YYERROR;
        }
        if (!(yyvsp[-3].interm.intermTypedNode)->isArray() && !(yyvsp[-3].interm.intermTypedNode)->isMatrix() && !(yyvsp[-3].interm.intermTypedNode)->isVector()) {
            if ((yyvsp[-3].interm.intermTypedNode)->getAsSymbolNode())
                parseContext.error((yyvsp[-2].lex).line, " left of '[' is not of type array, matrix, or vector ", (yyvsp[-3].interm.intermTypedNode)->getAsSymbolNode()->getSymbol().c_str(), "");
            else
                parseContext.error((yyvsp[-2].lex).line, " left of '[' is not of type array, matrix, or vector ", "expression", "");
            parseContext.recover();
        }
		if ((yyvsp[-1].interm.intermTypedNode)->getQualifier() == EvqConst) {
			if (((yyvsp[-3].interm.intermTypedNode)->isVector() || (yyvsp[-3].interm.intermTypedNode)->isMatrix()) && (yyvsp[-3].interm.intermTypedNode)->getType().getRowsCount() <= (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt() && !(yyvsp[-3].interm.intermTypedNode)->isArray() ) {
				parseContext.error((yyvsp[-2].lex).line, "", "[", "field selection out of range '%d'", (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt());
				parseContext.recover();
			} else {
				if ((yyvsp[-3].interm.intermTypedNode)->isArray()) {
					if ((yyvsp[-3].interm.intermTypedNode)->getType().getArraySize() == 0) {
						if ((yyvsp[-3].interm.intermTypedNode)->getType().getMaxArraySize() <= (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt()) {
							if (parseContext.arraySetMaxSize((yyvsp[-3].interm.intermTypedNode)->getAsSymbolNode(), (yyvsp[-3].interm.intermTypedNode)->getTypePointer(), (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt(), true, (yyvsp[-2].lex).line))
								parseContext.recover(); 
						} else {
							if (parseContext.arraySetMaxSize((yyvsp[-3].interm.intermTypedNode)->getAsSymbolNode(), (yyvsp[-3].interm.intermTypedNode)->getTypePointer(), 0, false, (yyvsp[-2].lex).line))
								parseContext.recover(); 
						}
					} else if ( (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt() >= (yyvsp[-3].interm.intermTypedNode)->getType().getArraySize()) {
						parseContext.error((yyvsp[-2].lex).line, "", "[", "array index out of range '%d'", (yyvsp[-1].interm.intermTypedNode)->getAsConstant()->toInt());
						parseContext.recover();
					}
				}
				(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexDirect, (yyvsp[-3].interm.intermTypedNode), (yyvsp[-1].interm.intermTypedNode), (yyvsp[-2].lex).line);
			}
		} else {
			if ((yyvsp[-3].interm.intermTypedNode)->isArray() && (yyvsp[-3].interm.intermTypedNode)->getType().getArraySize() == 0) {
				parseContext.error((yyvsp[-2].lex).line, "", "[", "array must be redeclared with a size before being indexed with a variable");
				parseContext.recover();
			}
			
			(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexIndirect, (yyvsp[-3].interm.intermTypedNode), (yyvsp[-1].interm.intermTypedNode), (yyvsp[-2].lex).line);
		}
        if ((yyval.interm.intermTypedNode) == 0) {
            TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[-2].lex).line);
			constant->setValue(0.f);
			(yyval.interm.intermTypedNode) = constant;
        } else if ((yyvsp[-3].interm.intermTypedNode)->isArray()) {
            if ((yyvsp[-3].interm.intermTypedNode)->getType().getStruct())
                (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getType().getStruct(), (yyvsp[-3].interm.intermTypedNode)->getType().getTypeName(), EbpUndefined, (yyvsp[-3].interm.intermTypedNode)->getLine()));
            else
                (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getBasicType(), (yyvsp[-3].interm.intermTypedNode)->getPrecision(), EvqTemporary, (yyvsp[-3].interm.intermTypedNode)->getColsCount(),(yyvsp[-3].interm.intermTypedNode)->getRowsCount(),  (yyvsp[-3].interm.intermTypedNode)->isMatrix()));
                
            if ((yyvsp[-3].interm.intermTypedNode)->getType().getQualifier() == EvqConst)
                (yyval.interm.intermTypedNode)->getTypePointer()->changeQualifier(EvqConst);
        } else if ((yyvsp[-3].interm.intermTypedNode)->isMatrix() && (yyvsp[-3].interm.intermTypedNode)->getType().getQualifier() == EvqConst)         
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getBasicType(), (yyvsp[-3].interm.intermTypedNode)->getPrecision(), EvqConst, 1, (yyvsp[-3].interm.intermTypedNode)->getColsCount()));
        else if ((yyvsp[-3].interm.intermTypedNode)->isMatrix())            
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getBasicType(), (yyvsp[-3].interm.intermTypedNode)->getPrecision(), EvqTemporary, 1, (yyvsp[-3].interm.intermTypedNode)->getColsCount()));
        else if ((yyvsp[-3].interm.intermTypedNode)->isVector() && (yyvsp[-3].interm.intermTypedNode)->getType().getQualifier() == EvqConst)          
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getBasicType(), (yyvsp[-3].interm.intermTypedNode)->getPrecision(), EvqConst));
        else if ((yyvsp[-3].interm.intermTypedNode)->isVector())       
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-3].interm.intermTypedNode)->getBasicType(), (yyvsp[-3].interm.intermTypedNode)->getPrecision(), EvqTemporary));
        else
            (yyval.interm.intermTypedNode)->setType((yyvsp[-3].interm.intermTypedNode)->getType());
    }
#line 2456 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 304 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 2464 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 307 "hlslang.y" /* yacc.c:1646  */
    {
        if (!(yyvsp[-2].interm.intermTypedNode)) {
            parseContext.error((yyvsp[0].lex).line, "field selection on null object", ".", "");
            YYERROR;
        }
        if ((yyvsp[-2].interm.intermTypedNode)->isArray()) {
            parseContext.error((yyvsp[0].lex).line, "cannot apply dot operator to an array", ".", "");
            parseContext.recover();
        }

        if ((yyvsp[-2].interm.intermTypedNode)->isVector()) {
            TVectorFields fields;
            if (! parseContext.parseVectorFields(*(yyvsp[0].lex).string, (yyvsp[-2].interm.intermTypedNode)->getRowsCount(), fields, (yyvsp[0].lex).line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

			(yyval.interm.intermTypedNode) = ir_add_vector_swizzle(fields, (yyvsp[-2].interm.intermTypedNode), (yyvsp[-1].lex).line, (yyvsp[0].lex).line);
        } else if ((yyvsp[-2].interm.intermTypedNode)->isMatrix()) {
            TVectorFields fields;
            if (!parseContext.parseMatrixFields(*(yyvsp[0].lex).string, (yyvsp[-2].interm.intermTypedNode)->getColsCount(), (yyvsp[-2].interm.intermTypedNode)->getRowsCount(), fields, (yyvsp[0].lex).line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            TString vectorString = *(yyvsp[0].lex).string;
            TIntermTyped* index = ir_add_swizzle(fields, (yyvsp[0].lex).line);                
            (yyval.interm.intermTypedNode) = ir_add_index(EOpMatrixSwizzle, (yyvsp[-2].interm.intermTypedNode), index, (yyvsp[-1].lex).line);
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[-2].interm.intermTypedNode)->getBasicType(), (yyvsp[-2].interm.intermTypedNode)->getPrecision(), EvqTemporary, 1, fields.num));
                    
        } else if ((yyvsp[-2].interm.intermTypedNode)->getBasicType() == EbtStruct) {
            bool fieldFound = false;
            TTypeList* fields = (yyvsp[-2].interm.intermTypedNode)->getType().getStruct();
            if (fields == 0) {
                parseContext.error((yyvsp[-1].lex).line, "structure has no fields", "Internal Error", "");
                parseContext.recover();
                (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
            } else {
                unsigned int i;
                for (i = 0; i < fields->size(); ++i) {
                    if ((*fields)[i].type->getFieldName() == *(yyvsp[0].lex).string) {
                        fieldFound = true;
                        break;
                    }
                }
                if (fieldFound) {
					TIntermConstant* index = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[0].lex).line);
					index->setValue(i);
					(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexDirectStruct, (yyvsp[-2].interm.intermTypedNode), index, (yyvsp[-1].lex).line);                
					(yyval.interm.intermTypedNode)->setType(*(*fields)[i].type);
                } else {
                    parseContext.error((yyvsp[-1].lex).line, " no such field in structure", (yyvsp[0].lex).string->c_str(), "");
                    parseContext.recover();
                    (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
                }
            }
        } else if ((yyvsp[-2].interm.intermTypedNode)->isScalar()) {

            // HLSL allows ".xxxx" field selection on single component floats.  Handle that here.
            TVectorFields fields;

            // Check to make sure only the "x" component is accessed.
            if (! parseContext.parseVectorFields(*(yyvsp[0].lex).string, 1, fields, (yyvsp[0].lex).line))
			{
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
				(yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
            }
			else
			{
				// Create the appropriate constructor based on the number of ".x"'s there are in the selection field
				TString vectorString = *(yyvsp[0].lex).string;
				TQualifier qualifier = (yyvsp[-2].interm.intermTypedNode)->getType().getQualifier() == EvqConst ? EvqConst : EvqTemporary;
				TType type((yyvsp[-2].interm.intermTypedNode)->getBasicType(), (yyvsp[-2].interm.intermTypedNode)->getPrecision(), qualifier, 1, (int) vectorString.size());
				(yyval.interm.intermTypedNode) = parseContext.constructBuiltIn(&type, parseContext.getConstructorOp(type),
												   (yyval.interm.intermTypedNode), (yyvsp[-2].interm.intermTypedNode)->getLine(), false);
			}
        } else {
            parseContext.error((yyvsp[-1].lex).line, " field selection requires structure, vector, or matrix on left hand side", (yyvsp[0].lex).string->c_str(), "");
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
        }
        // don't delete $3.string, it's from the pool
    }
#line 2556 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 394 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.lValueErrorCheck((yyvsp[0].lex).line, "++", (yyvsp[-1].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPostIncrement, (yyvsp[-1].interm.intermTypedNode), (yyvsp[0].lex).line, parseContext);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.unaryOpError((yyvsp[0].lex).line, "++", (yyvsp[-1].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermTypedNode);
        }
    }
#line 2571 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 404 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.lValueErrorCheck((yyvsp[0].lex).line, "--", (yyvsp[-1].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPostDecrement, (yyvsp[-1].interm.intermTypedNode), (yyvsp[0].lex).line, parseContext);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.unaryOpError((yyvsp[0].lex).line, "--", (yyvsp[-1].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermTypedNode);
        }
    }
#line 2586 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 417 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.scalarErrorCheck((yyvsp[0].interm.intermTypedNode), "[]"))
            parseContext.recover();
        TType type(EbtInt, EbpUndefined);
        (yyval.interm.intermTypedNode) = parseContext.constructBuiltIn(&type, EOpConstructInt, (yyvsp[0].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode)->getLine(), true);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.error((yyvsp[0].interm.intermTypedNode)->getLine(), "cannot convert to index", "[]", "");
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 2602 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 431 "hlslang.y" /* yacc.c:1646  */
    {
        TFunction* fnCall = (yyvsp[0].interm).function;
        TOperator op = fnCall->getBuiltInOp();

        if (op == EOpArrayLength) {
            if ((yyvsp[0].interm).intermNode->getAsTyped() == 0 || (yyvsp[0].interm).intermNode->getAsTyped()->getType().getArraySize() == 0) {
                parseContext.error((yyvsp[0].interm).line, "", fnCall->getName().c_str(), "array must be declared with a size before using this method");
                parseContext.recover();
            }

			TIntermConstant* constant = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[0].interm).line);
			constant->setValue((yyvsp[0].interm).intermNode->getAsTyped()->getType().getArraySize());
            (yyval.interm.intermTypedNode) = constant;
        } else if (op != EOpNull) {
            //
            // Then this should be a constructor.
            // Don't go through the symbol table for constructors.
            // Their parameters will be verified algorithmically.
            //
            TType type(EbtVoid, EbpUndefined);  // use this to get the type back
            if (parseContext.constructorErrorCheck((yyvsp[0].interm).line, (yyvsp[0].interm).intermNode, *fnCall, op, &type)) {
                (yyval.interm.intermTypedNode) = 0;
            } else {
                //
                // It's a constructor, of type 'type'.
                //
                (yyval.interm.intermTypedNode) = parseContext.addConstructor((yyvsp[0].interm).intermNode, &type, op, fnCall, (yyvsp[0].interm).line);
            }

            if ((yyval.interm.intermTypedNode) == 0) {
                parseContext.recover();
                (yyval.interm.intermTypedNode) = ir_set_aggregate_op(0, op, (yyvsp[0].interm).line);
				(yyval.interm.intermTypedNode)->setType(type);
            }
        } else {
            //
            // Not a constructor.  Find it in the symbol table.
            //
            const TFunction* fnCandidate;
            bool builtIn;
            fnCandidate = parseContext.findFunction((yyvsp[0].interm).line, fnCall, &builtIn);

            if ( fnCandidate && fnCandidate->getMangledName() != fnCall->getMangledName()) {
                //add constructors to arguments to ensure that they have proper types
                TIntermNode *temp = parseContext.promoteFunctionArguments( (yyvsp[0].interm).intermNode,
                                      fnCandidate);
                if (temp)
                    (yyvsp[0].interm).intermNode = temp;
                else {
                    parseContext.error( (yyvsp[0].interm).intermNode->getLine(), " unable to suitably promote arguments to function",
                                        fnCandidate->getName().c_str(), "");
                    fnCandidate = 0;
                }
            }

            if (fnCandidate) {
                //
                // A declared function.  But, it might still map to a built-in
                // operation.
                //
                op = fnCandidate->getBuiltInOp();
                if (builtIn && op != EOpNull) {
                    //
                    // A function call mapped to a built-in operation.
                    //
                    if (fnCandidate->getParamCount() == 1) {
                        //
                        // Treat it like a built-in unary operator.
                        //
                        (yyval.interm.intermTypedNode) = ir_add_unary_math(op, (yyvsp[0].interm).intermNode, gNullSourceLoc, parseContext);
                        if ((yyval.interm.intermTypedNode) == 0)  {
                            parseContext.error((yyvsp[0].interm).intermNode->getLine(), " wrong operand type", "Internal Error",
                                "built in unary operator function.  Type: %s",
                                static_cast<TIntermTyped*>((yyvsp[0].interm).intermNode)->getCompleteString().c_str());
                            YYERROR;
                        }
                    } else {
                        (yyval.interm.intermTypedNode) = ir_set_aggregate_op((yyvsp[0].interm).intermAggregate, op, (yyvsp[0].interm).line);
						(yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());
                    }
                } else {
                    // This is a real function call
                    
                    (yyval.interm.intermTypedNode) = ir_set_aggregate_op((yyvsp[0].interm).intermAggregate, EOpFunctionCall, (yyvsp[0].interm).line);
                    (yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());                   
                    
                    (yyval.interm.intermTypedNode)->getAsAggregate()->setName(fnCandidate->getMangledName());
                    (yyval.interm.intermTypedNode)->getAsAggregate()->setPlainName(fnCandidate->getName());

                    TQualifier qual;
                    for (int i = 0; i < fnCandidate->getParamCount(); ++i) {
                        qual = (*fnCandidate)[i].type->getQualifier();
                        if (qual == EvqOut || qual == EvqInOut) {
                            if (parseContext.lValueErrorCheck((yyval.interm.intermTypedNode)->getLine(), "assign", (yyval.interm.intermTypedNode)->getAsAggregate()->getNodes()[i]->getAsTyped())) {
                                parseContext.error((yyvsp[0].interm).intermNode->getLine(), "Constant value cannot be passed for 'out' or 'inout' parameters.", "Error", "");
                                parseContext.recover();
                            }
                        }
                    }
                }
                (yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());
            } else {
                // error message was put out by PaFindFunction()
                // Put on a dummy node for error recovery
                
				TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[0].interm).line);
				constant->setValue(0.f);
				(yyval.interm.intermTypedNode) = constant;
                parseContext.recover();
            }
        }
        delete fnCall;
    }
#line 2720 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 547 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[0].interm);
    }
#line 2728 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 550 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-2].interm.intermTypedNode)->isArray() && (yyvsp[0].interm).function->getName() == "length") {
            //
            // implement array.length()
            //
            (yyval.interm) = (yyvsp[0].interm);
            (yyval.interm).intermNode = (yyvsp[-2].interm.intermTypedNode);
            (yyval.interm).function->relateToOperator(EOpArrayLength);

        } else {
            parseContext.error((yyvsp[0].interm).line, "methods are not supported", "", "");
            parseContext.recover();
            (yyval.interm) = (yyvsp[0].interm);
        }
    }
#line 2748 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 568 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[-1].interm);
        (yyval.interm).line = (yyvsp[0].lex).line;
    }
#line 2757 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 572 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[-1].interm);
        (yyval.interm).line = (yyvsp[0].lex).line;
    }
#line 2766 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 579 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm).function = (yyvsp[-1].interm.function);
        (yyval.interm).intermNode = 0;
    }
#line 2775 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 583 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm).function = (yyvsp[0].interm.function);
        (yyval.interm).intermNode = 0;
    }
#line 2784 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 590 "hlslang.y" /* yacc.c:1646  */
    {
		if (!(yyvsp[0].interm.intermTypedNode)) {
          YYERROR;
		}
		TParameter param = { 0, 0, new TType((yyvsp[0].interm.intermTypedNode)->getType()) };
        (yyvsp[-1].interm.function)->addParameter(param);
        (yyval.interm).function = (yyvsp[-1].interm.function);
        (yyval.interm).intermNode = (yyvsp[0].interm.intermTypedNode);
    }
#line 2798 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 599 "hlslang.y" /* yacc.c:1646  */
    {
		if (!(yyvsp[0].interm.intermTypedNode)) {
          YYERROR;
		}
        TParameter param = { 0, 0, new TType((yyvsp[0].interm.intermTypedNode)->getType()) };
        (yyvsp[-2].interm).function->addParameter(param);
        (yyval.interm).function = (yyvsp[-2].interm).function;
        (yyval.interm).intermNode = ir_grow_aggregate((yyvsp[-2].interm).intermNode, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line);
    }
#line 2812 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 611 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.function) = (yyvsp[-1].interm.function);
    }
#line 2820 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 619 "hlslang.y" /* yacc.c:1646  */
    {
        //
        // Constructor
        //
        if ((yyvsp[0].interm.type).array) {
            //TODO : figure out how to deal with array constructors
        }

        if ((yyvsp[0].interm.type).userDef) {
            TString tempString = "";
            TType type((yyvsp[0].interm.type));
            TFunction *function = new TFunction(&tempString, type, EOpConstructStruct);
            (yyval.interm.function) = function;
        } else {
            TOperator op = ir_get_constructor_op((yyvsp[0].interm.type), parseContext, false);
            if (op == EOpNull) {
                parseContext.error((yyvsp[0].interm.type).line, "cannot construct this type", TType::getBasicString((yyvsp[0].interm.type).type), "");
                parseContext.recover();
                (yyvsp[0].interm.type).type = EbtFloat;
                op = EOpConstructFloat;
            }
            TString tempString = "";
            TType type((yyvsp[0].interm.type));
            TFunction *function = new TFunction(&tempString, type, op);
            (yyval.interm.function) = function;
        }
    }
#line 2852 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 646 "hlslang.y" /* yacc.c:1646  */
    {
		if (parseContext.reservedErrorCheck((yyvsp[0].lex).line, *(yyvsp[0].lex).string)) 
			parseContext.recover();
		TType type(EbtVoid, EbpUndefined);
		const TString *mangled;
		if ( *(yyvsp[0].lex).string == "main")
			mangled = NewPoolTString("xlat_main");
		else
			mangled = (yyvsp[0].lex).string;
		TFunction *function = new TFunction( mangled, type);
		(yyval.interm.function) = function;
	}
#line 2869 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 658 "hlslang.y" /* yacc.c:1646  */
    {
		if (parseContext.reservedErrorCheck((yyvsp[0].lex).line, *(yyvsp[0].lex).string)) 
			parseContext.recover();
		TType type(EbtVoid, EbpUndefined);
		TFunction *function = new TFunction((yyvsp[0].lex).string, type);
		(yyval.interm.function) = function;
    }
#line 2881 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 668 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 2889 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 671 "hlslang.y" /* yacc.c:1646  */
    {
		if (parseContext.lValueErrorCheck((yyvsp[-1].lex).line, "++", (yyvsp[0].interm.intermTypedNode)))
			parseContext.recover();
		(yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPreIncrement, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, parseContext);
		if ((yyval.interm.intermTypedNode) == 0) {
			parseContext.unaryOpError((yyvsp[-1].lex).line, "++", (yyvsp[0].interm.intermTypedNode)->getCompleteString());
			parseContext.recover();
			(yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
		}
    }
#line 2904 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 681 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.lValueErrorCheck((yyvsp[-1].lex).line, "--", (yyvsp[0].interm.intermTypedNode)))
            parseContext.recover();
		(yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPreDecrement, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, parseContext);
		if ((yyval.interm.intermTypedNode) == 0) {
			parseContext.unaryOpError((yyvsp[-1].lex).line, "--", (yyvsp[0].interm.intermTypedNode)->getCompleteString());
			parseContext.recover();
			(yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
		}
    }
#line 2919 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 691 "hlslang.y" /* yacc.c:1646  */
    {
		if ((yyvsp[-1].interm).op != EOpNull) {
			(yyval.interm.intermTypedNode) = ir_add_unary_math((yyvsp[-1].interm).op, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].interm).line, parseContext);
			if ((yyval.interm.intermTypedNode) == 0) {
				const char* errorOp = "";
				switch((yyvsp[-1].interm).op) {
					case EOpNegative:   errorOp = "-"; break;
					case EOpLogicalNot: errorOp = "!"; break;
					case EOpBitwiseNot: errorOp = "~"; break;
					default: break;
				}
				parseContext.unaryOpError((yyvsp[-1].interm).line, errorOp, (yyvsp[0].interm.intermTypedNode)->getCompleteString());
				parseContext.recover();
				(yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
			}
		} else
			(yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 2942 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 709 "hlslang.y" /* yacc.c:1646  */
    {
        // cast operator, insert constructor
        TOperator op = ir_get_constructor_op((yyvsp[-2].interm.type), parseContext, true);
        if (op == EOpNull) {
            parseContext.error((yyvsp[-2].interm.type).line, "cannot cast this type", TType::getBasicString((yyvsp[-2].interm.type).type), "");
            parseContext.recover();
            (yyvsp[-2].interm.type).type = EbtFloat;
            op = EOpConstructFloat;
        }
        TString tempString = "";
        TType type((yyvsp[-2].interm.type));
        TFunction *function = new TFunction(&tempString, type, op);
        TParameter param = { 0, 0, new TType((yyvsp[0].interm.intermTypedNode)->getType()) };
        function->addParameter(param);
        TType type2(EbtVoid, EbpUndefined);  // use this to get the type back
        if (parseContext.constructorErrorCheck((yyvsp[-2].interm.type).line, (yyvsp[0].interm.intermTypedNode), *function, op, &type2)) {
            (yyval.interm.intermTypedNode) = 0;
        } else {
            //
            // It's a constructor, of type 'type'.
            //
            (yyval.interm.intermTypedNode) = parseContext.addConstructor((yyvsp[0].interm.intermTypedNode), &type2, op, function, (yyvsp[-2].interm.type).line);
        }

        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.recover();
            (yyval.interm.intermTypedNode) = ir_set_aggregate_op(0, op, (yyvsp[-2].interm.type).line);
        } else {
			(yyval.interm.intermTypedNode)->setType(type2);
		}
	}
#line 2978 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 744 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpNull; }
#line 2984 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 745 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpNegative; }
#line 2990 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 746 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpLogicalNot; }
#line 2996 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 747 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("~", (yyvsp[0].lex).line);
              (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpBitwiseNot; }
#line 3003 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 753 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3009 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 754 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpMul, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "*", false); }
#line 3015 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 755 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpDiv, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "/", false); }
#line 3021 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 756 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpMod, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "%", false); }
#line 3027 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 760 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3033 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 761 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpAdd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "+", false); }
#line 3039 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 762 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpSub, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "-", false); }
#line 3045 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 766 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3051 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 767 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLeftShift, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "<<", false); }
#line 3057 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 768 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpRightShift, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, ">>", false); }
#line 3063 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 772 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3069 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 773 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLessThan, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "<", true); }
#line 3075 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 774 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpGreaterThan, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, ">", true); }
#line 3081 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 775 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLessThanEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "<=", true); }
#line 3087 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 776 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpGreaterThanEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, ">=", true); }
#line 3093 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 780 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3099 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 781 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "==", true); }
#line 3105 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 782 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpNotEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "!=", true); }
#line 3111 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 786 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3117 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 787 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpAnd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "&", false); }
#line 3123 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 791 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3129 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 792 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpExclusiveOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "^", false); }
#line 3135 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 796 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3141 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 797 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpInclusiveOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "|", false); }
#line 3147 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 801 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3153 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 802 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalAnd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "&&", true); }
#line 3159 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 806 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3165 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 807 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalXor, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "^^", true); }
#line 3171 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 811 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3177 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 812 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, "||", true); }
#line 3183 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 816 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3189 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 817 "hlslang.y" /* yacc.c:1646  */
    {
       if (parseContext.boolOrVectorErrorCheck((yyvsp[-3].lex).line, (yyvsp[-4].interm.intermTypedNode)))
            parseContext.recover();
       
		(yyval.interm.intermTypedNode) = ir_add_selection((yyvsp[-4].interm.intermTypedNode), (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-3].lex).line, parseContext.infoSink);
           
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[-3].lex).line, ":", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(), (yyvsp[0].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 3206 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 832 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 3212 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 833 "hlslang.y" /* yacc.c:1646  */
    {        
        if (parseContext.lValueErrorCheck((yyvsp[-1].interm).line, "assign", (yyvsp[-2].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = parseContext.addAssign((yyvsp[-1].interm).op, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].interm).line);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.assignError((yyvsp[-1].interm).line, "assign", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(), (yyvsp[0].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
        } else if (((yyvsp[-2].interm.intermTypedNode)->isArray() || (yyvsp[0].interm.intermTypedNode)->isArray()))
            parseContext.recover();
    }
#line 3228 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 847 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpAssign; }
#line 3234 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 848 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpMulAssign; }
#line 3240 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 849 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpDivAssign; }
#line 3246 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 850 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("%=", (yyvsp[0].lex).line);  (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpModAssign; }
#line 3252 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 851 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpAddAssign; }
#line 3258 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 852 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpSubAssign; }
#line 3264 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 853 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("<<=", (yyvsp[0].lex).line); (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpLeftShiftAssign; }
#line 3270 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 854 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("<<=", (yyvsp[0].lex).line); (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpRightShiftAssign; }
#line 3276 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 855 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("&=",  (yyvsp[0].lex).line); (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpAndAssign; }
#line 3282 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 856 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("^=",  (yyvsp[0].lex).line); (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpExclusiveOrAssign; }
#line 3288 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 857 "hlslang.y" /* yacc.c:1646  */
    { UNSUPPORTED_FEATURE("|=",  (yyvsp[0].lex).line); (yyval.interm).line = (yyvsp[0].lex).line; (yyval.interm).op = EOpInclusiveOrAssign; }
#line 3294 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 861 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 3302 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 864 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = ir_add_comma((yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[-1].lex).line, ",", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(), (yyvsp[0].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 3315 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 875 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.constErrorCheck((yyvsp[0].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 3325 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 883 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermDeclaration) = 0; }
#line 3331 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 884 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermDeclaration) = (yyvsp[-1].interm.intermDeclaration); }
#line 3337 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 888 "hlslang.y" /* yacc.c:1646  */
    {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find((yyvsp[-1].interm.function)->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != (yyvsp[-1].interm.function)->getReturnType()) {
                parseContext.error((yyvsp[0].lex).line, "overloaded functions must have the same return type", (yyvsp[-1].interm.function)->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*(yyvsp[-1].interm.function))[i].type->getQualifier()) {
                    parseContext.error((yyvsp[0].lex).line, "overloaded functions must have the same parameter qualifiers", (*(yyvsp[-1].interm.function))[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }

        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        (yyval.interm).function = (yyvsp[-1].interm.function);
        (yyval.interm).line = (yyvsp[0].lex).line;

        parseContext.symbolTable.insert(*(yyval.interm).function);
    }
#line 3375 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 921 "hlslang.y" /* yacc.c:1646  */
    {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find((yyvsp[-3].interm.function)->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != (yyvsp[-3].interm.function)->getReturnType()) {
                parseContext.error((yyvsp[-2].lex).line, "overloaded functions must have the same return type", (yyvsp[-3].interm.function)->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*(yyvsp[-3].interm.function))[i].type->getQualifier()) {
                    parseContext.error((yyvsp[-2].lex).line, "overloaded functions must have the same parameter qualifiers", (*(yyvsp[-3].interm.function))[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }

        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        (yyval.interm).function = (yyvsp[-3].interm.function);
        (yyval.interm).line = (yyvsp[-2].lex).line;
        (yyval.interm).function->setInfo(new TTypeInfo(*(yyvsp[0].lex).string, 0));

        parseContext.symbolTable.insert(*(yyval.interm).function);
    }
#line 3414 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 958 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.function) = (yyvsp[0].interm.function);
    }
#line 3422 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 961 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.function) = (yyvsp[0].interm.function);
    }
#line 3430 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 968 "hlslang.y" /* yacc.c:1646  */
    {
        // Add the parameter
        (yyval.interm.function) = (yyvsp[-1].interm.function);
        if ((yyvsp[0].interm).param.type->getBasicType() != EbtVoid)
            (yyvsp[-1].interm.function)->addParameter((yyvsp[0].interm).param);
        else
            delete (yyvsp[0].interm).param.type;
    }
#line 3443 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 976 "hlslang.y" /* yacc.c:1646  */
    {
        //
        // Only first parameter of one-parameter functions can be void
        // The check for named parameters not being void is done in parameter_declarator
        //
        if ((yyvsp[0].interm).param.type->getBasicType() == EbtVoid) {
            //
            // This parameter > first is void
            //
            parseContext.error((yyvsp[-1].lex).line, "cannot be an argument type except for '(void)'", "void", "");
            parseContext.recover();
            delete (yyvsp[0].interm).param.type;
        } else {
            // Add the parameter
            (yyval.interm.function) = (yyvsp[-2].interm.function);
            (yyvsp[-2].interm.function)->addParameter((yyvsp[0].interm).param);
        }
    }
#line 3466 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 997 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-2].interm.type).qualifier != EvqGlobal && (yyvsp[-2].interm.type).qualifier != EvqTemporary) {
			if ((yyvsp[-2].interm.type).qualifier == EvqConst || (yyvsp[-2].interm.type).qualifier == EvqStatic)
			{
				(yyvsp[-2].interm.type).qualifier = EvqTemporary;
			}
			else
			{
				parseContext.error((yyvsp[-1].lex).line, "no qualifiers allowed for function return", getQualifierString((yyvsp[-2].interm.type).qualifier), "");
				parseContext.recover();
			}
        }
        // make sure a sampler is not involved as well...
        if (parseContext.structQualifierErrorCheck((yyvsp[-1].lex).line, (yyvsp[-2].interm.type)))
            parseContext.recover();

        // Add the function as a prototype after parsing it (we do not support recursion)
        TFunction *function;
        TType type((yyvsp[-2].interm.type));
    const TString* mangled = 0;
    if ( *(yyvsp[-1].lex).string == "main")
        mangled = NewPoolTString( "xlat_main");
    else
        mangled = (yyvsp[-1].lex).string;

        function = new TFunction(mangled, type);
        (yyval.interm.function) = function;
    }
#line 3499 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1029 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-1].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[0].lex).line, "illegal use of type 'void'", (yyvsp[0].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[0].lex).line, *(yyvsp[0].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[0].lex).string, 0, new TType((yyvsp[-1].interm.type))};
        (yyval.interm).line = (yyvsp[0].lex).line;
        (yyval.interm).param = param;
    }
#line 3515 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1040 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-3].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[-2].lex).line, "illegal use of type 'void'", (yyvsp[-2].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[-2].lex).line, *(yyvsp[-2].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[-2].lex).string, 0, new TType((yyvsp[-3].interm.type))};
        (yyval.interm).line = (yyvsp[-2].lex).line;
        (yyval.interm).param = param;

        //TODO: add initializer support
    }
#line 3533 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 1053 "hlslang.y" /* yacc.c:1646  */
    {
        // Parameter with register
        if ((yyvsp[-2].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[-1].lex).line, "illegal use of type 'void'", (yyvsp[-1].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[-1].lex).line, *(yyvsp[-1].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[-1].lex).string, new TTypeInfo("", *(yyvsp[0].lex).string, 0), new TType((yyvsp[-2].interm.type))};
        (yyval.interm).line = (yyvsp[-1].lex).line;
        (yyval.interm).param = param; 
    }
#line 3550 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1065 "hlslang.y" /* yacc.c:1646  */
    {
        //Parameter with semantic
        if ((yyvsp[-3].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[-2].lex).line, "illegal use of type 'void'", (yyvsp[-2].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[-2].lex).line, *(yyvsp[-2].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[-2].lex).string, new TTypeInfo(*(yyvsp[0].lex).string, 0), new TType((yyvsp[-3].interm.type))};
        (yyval.interm).line = (yyvsp[-2].lex).line;
        (yyval.interm).param = param;
    }
#line 3567 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1077 "hlslang.y" /* yacc.c:1646  */
    {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-4].interm.type)))
            parseContext.recover();

        if (parseContext.reservedErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string))
            parseContext.recover();

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-1].interm.intermTypedNode), size))
            parseContext.recover();
        (yyvsp[-4].interm.type).setArray(true, size);

        TType* type = new TType((yyvsp[-4].interm.type));
        TParameter param = { (yyvsp[-3].lex).string, 0, type };
        (yyval.interm).line = (yyvsp[-3].lex).line;
        (yyval.interm).param = param;
    }
#line 3590 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1095 "hlslang.y" /* yacc.c:1646  */
    {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck((yyvsp[-4].lex).line, (yyvsp[-6].interm.type)))
            parseContext.recover();

        if (parseContext.reservedErrorCheck((yyvsp[-5].lex).line, *(yyvsp[-5].lex).string))
            parseContext.recover();

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[-4].lex).line, (yyvsp[-3].interm.intermTypedNode), size))
            parseContext.recover();
        (yyvsp[-6].interm.type).setArray(true, size);

        TType* type = new TType((yyvsp[-6].interm.type));
        TParameter param = { (yyvsp[-5].lex).string, new TTypeInfo(*(yyvsp[0].lex).string, 0), type };
        (yyval.interm).line = (yyvsp[-5].lex).line;
        (yyval.interm).param = param;
    }
#line 3613 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1124 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[0].interm);
        if (parseContext.paramErrorCheck((yyvsp[0].interm).line, (yyvsp[-2].interm.type).qualifier, (yyvsp[-1].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    }
#line 3623 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1129 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[0].interm);
        if (parseContext.parameterSamplerErrorCheck((yyvsp[0].interm).line, (yyvsp[-1].interm.qualifier), *(yyvsp[0].interm).param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck((yyvsp[0].interm).line, EvqTemporary, (yyvsp[-1].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    }
#line 3635 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1139 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[0].interm);
        if (parseContext.paramErrorCheck((yyvsp[0].interm).line, (yyvsp[-2].interm.type).qualifier, (yyvsp[-1].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    }
#line 3645 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1144 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm) = (yyvsp[0].interm);
        if (parseContext.parameterSamplerErrorCheck((yyvsp[0].interm).line, (yyvsp[-1].interm.qualifier), *(yyvsp[0].interm).param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck((yyvsp[0].interm).line, EvqTemporary, (yyvsp[-1].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    }
#line 3657 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1154 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.qualifier) = EvqIn;
    }
#line 3665 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1157 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.qualifier) = EvqIn;
    }
#line 3673 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1160 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.qualifier) = EvqOut;
    }
#line 3681 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1163 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.qualifier) = EvqInOut;
    }
#line 3689 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1169 "hlslang.y" /* yacc.c:1646  */
    {
        TParameter param = { 0, 0, new TType((yyvsp[0].interm.type)) };
        (yyval.interm).param = param;
    }
#line 3698 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1176 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermDeclaration) = (yyvsp[0].interm.intermDeclaration);
    }
#line 3706 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1179 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-3].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-1].lex).line, type))
            parseContext.recover();
        
        if (parseContext.nonInitConstErrorCheck((yyvsp[-1].lex).line, *(yyvsp[-1].lex).string, type))
            parseContext.recover();

        if (parseContext.nonInitErrorCheck((yyvsp[-1].lex).line, *(yyvsp[-1].lex).string, (yyvsp[0].interm.typeInfo), type))
            parseContext.recover();
		
		TSymbol* sym = parseContext.symbolTable.find(*(yyvsp[-1].lex).string);
		if (!sym)
			(yyval.interm.intermDeclaration) = (yyvsp[-3].interm.intermDeclaration);
		else
			(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-3].interm.intermDeclaration), sym, NULL, parseContext);
    }
#line 3729 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1197 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-5].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, type))
            parseContext.recover();
        
        if (parseContext.arrayTypeErrorCheck((yyvsp[-2].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[-2].lex).line, type))
            parseContext.recover();
        else {
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[-2].lex).line, *(yyvsp[-3].lex).string, (yyvsp[0].interm.typeInfo), type, variable))
                parseContext.recover();
		
			if (!variable)
				(yyval.interm.intermDeclaration) = (yyvsp[-5].interm.intermDeclaration);
			else {
				variable->getType().setArray(true);
				(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-5].interm.intermDeclaration), variable, NULL, parseContext);
			}
        }
    }
#line 3758 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1221 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-6].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-4].lex).line, type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck((yyvsp[-4].lex).line, *(yyvsp[-4].lex).string, type))
            parseContext.recover();

        if (parseContext.arrayTypeErrorCheck((yyvsp[-3].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[-3].lex).line, type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[-3].lex).line, (yyvsp[-2].interm.intermTypedNode), size))
                parseContext.recover();
            type.setArray(true, size);
			
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-4].lex).string, (yyvsp[0].interm.typeInfo), type, variable))
                parseContext.recover();
			
			if (!variable)
				(yyval.interm.intermDeclaration) = (yyvsp[-6].interm.intermDeclaration);
			else {
				(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-6].interm.intermDeclaration), variable, NULL, parseContext);
			}
        }
    }
#line 3791 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1249 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-7].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-5].lex).line, type))
            parseContext.recover();
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[-4].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[-4].lex).line, type))
            parseContext.recover();
        else if (parseContext.arrayErrorCheck((yyvsp[-4].lex).line, *(yyvsp[-5].lex).string, type, variable))
			parseContext.recover();
		
        {
            TIntermSymbol* symbol;
            type.setArray(true, (yyvsp[0].interm.intermTypedNode)->getType().getArraySize());
            if (!parseContext.executeInitializer((yyvsp[-5].lex).line, *(yyvsp[-5].lex).string, (yyvsp[-2].interm.typeInfo), type, (yyvsp[0].interm.intermTypedNode), symbol, variable)) {
                if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[-7].interm.intermDeclaration);
				else {
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-7].interm.intermDeclaration), variable, (yyvsp[0].interm.intermTypedNode), parseContext);
				}
            } else {
                parseContext.recover();
                (yyval.interm.intermDeclaration) = 0;
            }
        }
    }
#line 3823 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1276 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-8].interm.intermDeclaration));
		int array_size;
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-6].lex).line, type))
            parseContext.recover();
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[-5].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[-5].lex).line, type))
            parseContext.recover();
        else {
            if (parseContext.arraySizeErrorCheck((yyvsp[-5].lex).line, (yyvsp[-4].interm.intermTypedNode), array_size))
                parseContext.recover();
			
            type.setArray(true, array_size);
            if (parseContext.arrayErrorCheck((yyvsp[-5].lex).line, *(yyvsp[-6].lex).string, (yyvsp[-2].interm.typeInfo), type, variable))
                parseContext.recover();
        }

        {
            TIntermSymbol* symbol;
            if (!parseContext.executeInitializer((yyvsp[-6].lex).line, *(yyvsp[-6].lex).string, (yyvsp[-2].interm.typeInfo), type, (yyvsp[0].interm.intermTypedNode), symbol, variable)) {
				if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[-8].interm.intermDeclaration);
				else {
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-8].interm.intermDeclaration), variable, (yyvsp[0].interm.intermTypedNode), parseContext);
				}
            } else {
                parseContext.recover();
                (yyval.interm.intermDeclaration) = 0;
            }
        }
    }
#line 3861 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1309 "hlslang.y" /* yacc.c:1646  */
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[-5].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, type))
            parseContext.recover();
			
        TIntermSymbol* symbol;
		if ( !IsSampler(type.type)) {
			if (!parseContext.executeInitializer((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-2].interm.typeInfo), type, (yyvsp[0].interm.intermTypedNode), symbol)) {
				TSymbol* variable = parseContext.symbolTable.find(*(yyvsp[-3].lex).string);
				if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[-5].interm.intermDeclaration);
				else 				
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[-5].interm.intermDeclaration), variable, (yyvsp[0].interm.intermTypedNode), parseContext);
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		} else {
			//Special code to skip initializers for samplers
			(yyval.interm.intermDeclaration) = (yyvsp[-5].interm.intermDeclaration);
			if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, type))
				parseContext.recover();
			
			if (parseContext.nonInitConstErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, type))
				parseContext.recover();
			
			if (parseContext.nonInitErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-2].interm.typeInfo), type))
				parseContext.recover();
		}
	}
#line 3897 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1343 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.interm.intermDeclaration) = 0;
    }
#line 3905 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1346 "hlslang.y" /* yacc.c:1646  */
    {				
		bool error = false;
        if (error &= parseContext.structQualifierErrorCheck((yyvsp[-1].lex).line, (yyvsp[-2].interm.type)))
            parseContext.recover();
        
        if (error &= parseContext.nonInitConstErrorCheck((yyvsp[-1].lex).line, *(yyvsp[-1].lex).string, (yyvsp[-2].interm.type)))
            parseContext.recover();

        if (error &= parseContext.nonInitErrorCheck((yyvsp[-1].lex).line, *(yyvsp[-1].lex).string, (yyvsp[0].interm.typeInfo), (yyvsp[-2].interm.type)))
            parseContext.recover();
		
		TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[-1].lex).string);
		if (!error && symbol) {
			(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, NULL, (yyvsp[-1].lex).line, parseContext);
		} else {
			(yyval.interm.intermDeclaration) = 0;
		}
    }
#line 3928 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1364 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, (yyvsp[-4].interm.type)))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-4].interm.type)))
            parseContext.recover();

        if (parseContext.arrayTypeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-4].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[-2].lex).line, (yyvsp[-4].interm.type)))
            parseContext.recover();
        else {
            (yyvsp[-4].interm.type).setArray(true);
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[-2].lex).line, *(yyvsp[-3].lex).string, (yyvsp[0].interm.typeInfo), (yyvsp[-4].interm.type), variable))
                parseContext.recover();
        }
		
		TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[-3].lex).string);
		if (symbol) {
			(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, NULL, (yyvsp[-3].lex).line, parseContext);
		} else {
			(yyval.interm.intermDeclaration) = 0;
		}
    }
#line 3956 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1387 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[-4].lex).line, (yyvsp[-5].interm.type)))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck((yyvsp[-4].lex).line, *(yyvsp[-4].lex).string, (yyvsp[-5].interm.type)))
			parseContext.recover();
		
		TVariable* variable;
        if (parseContext.arrayTypeErrorCheck((yyvsp[-3].lex).line, (yyvsp[-5].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[-3].lex).line, (yyvsp[-5].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[-3].lex).line, (yyvsp[-2].interm.intermTypedNode), size))
                parseContext.recover();

            (yyvsp[-5].interm.type).setArray(true, size);
            if (parseContext.arrayErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-4].lex).string, (yyvsp[0].interm.typeInfo), (yyvsp[-5].interm.type), variable))
                parseContext.recover();
			
			if (variable) {
				(yyval.interm.intermDeclaration) = ir_add_declaration(variable, NULL, (yyvsp[-4].lex).line, parseContext);
			} else {
				(yyval.interm.intermDeclaration) = 0;
			}
        }
	}
#line 3987 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1413 "hlslang.y" /* yacc.c:1646  */
    {
		if (parseContext.structQualifierErrorCheck((yyvsp[-5].lex).line, (yyvsp[-6].interm.type)))
			parseContext.recover();

		TVariable* variable = 0;
		if (parseContext.arrayTypeErrorCheck((yyvsp[-4].lex).line, (yyvsp[-6].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[-4].lex).line, (yyvsp[-6].interm.type)))
			parseContext.recover();
		else {
			(yyvsp[-6].interm.type).setArray(true, (yyvsp[0].interm.intermTypedNode)->getType().getArraySize());
			if (parseContext.arrayErrorCheck((yyvsp[-4].lex).line, *(yyvsp[-5].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-6].interm.type), variable))
				parseContext.recover();
		}

		{        
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[-5].lex).line, *(yyvsp[-5].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-6].interm.type), (yyvsp[0].interm.intermTypedNode), symbol, variable)) {
				if (variable)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		}
    }
#line 4018 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1439 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[-6].lex).line, (yyvsp[-7].interm.type)))
            parseContext.recover();

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[-5].lex).line, (yyvsp[-7].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[-5].lex).line, (yyvsp[-7].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[-5].lex).line, (yyvsp[-4].interm.intermTypedNode), size))
                parseContext.recover();

            (yyvsp[-7].interm.type).setArray(true, size);
            if (parseContext.arrayErrorCheck((yyvsp[-5].lex).line, *(yyvsp[-6].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-7].interm.type), variable))
                parseContext.recover();
        }
        
		{        
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[-6].lex).line, *(yyvsp[-6].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-7].interm.type), (yyvsp[0].interm.intermTypedNode), symbol, variable)) {
				if (variable)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		}       
    }
#line 4053 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1469 "hlslang.y" /* yacc.c:1646  */
    {
		if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, (yyvsp[-4].interm.type)))
			parseContext.recover();
		
		if (!IsSampler((yyvsp[-4].interm.type).type)) {
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-4].interm.type), (yyvsp[0].interm.intermTypedNode), symbol)) {
				if (symbol)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		} else {
			if (parseContext.structQualifierErrorCheck((yyvsp[-3].lex).line, (yyvsp[-4].interm.type)))
				parseContext.recover();

			if (parseContext.nonInitConstErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-4].interm.type)))
				parseContext.recover();

			if (parseContext.nonInitErrorCheck((yyvsp[-3].lex).line, *(yyvsp[-3].lex).string, (yyvsp[-2].interm.typeInfo), (yyvsp[-4].interm.type)))
				parseContext.recover();
				
			TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[-3].lex).string);
			if (symbol) {
				(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, NULL, (yyvsp[-3].lex).line, parseContext);
			} else {
				(yyval.interm.intermDeclaration) = 0;
			}
		}
    }
#line 4091 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1517 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 4099 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1520 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[0].interm.type).array && parseContext.arrayQualifierErrorCheck((yyvsp[0].interm.type).line, (yyvsp[-1].interm.type))) {
            parseContext.recover();
            (yyvsp[0].interm.type).setArray(false);
        }

        if ((yyvsp[-1].interm.type).qualifier == EvqAttribute &&
            ((yyvsp[0].interm.type).type == EbtBool || (yyvsp[0].interm.type).type == EbtInt)) {
            parseContext.error((yyvsp[0].interm.type).line, "cannot be bool or int", getQualifierString((yyvsp[-1].interm.type).qualifier), "");
            parseContext.recover();
        }
        (yyval.interm.type) = (yyvsp[0].interm.type); 
        (yyval.interm.type).qualifier = (yyvsp[-1].interm.type).qualifier;
    }
#line 4118 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1537 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[0].lex).line);
    }
#line 4126 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1540 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqStatic, (yyvsp[0].lex).line);
    }
#line 4134 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1543 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[-1].lex).line); // same as "const" really
    }
#line 4142 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1546 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[-1].lex).line); // same as "const" really
    }
#line 4150 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1549 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.globalErrorCheck((yyvsp[0].lex).line, parseContext.symbolTable.atGlobalLevel(), "uniform"))
            parseContext.recover();
        (yyval.interm.type).setBasic(EbtVoid, EvqUniform, (yyvsp[0].lex).line);
    }
#line 4160 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1557 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 4168 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1560 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type) = (yyvsp[-3].interm.type);

        if (parseContext.arrayTypeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-3].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-1].interm.intermTypedNode), size))
                parseContext.recover();
            (yyval.interm.type).setArray(true, size);
        }
    }
#line 4185 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1575 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtVoid,EbpUndefined);
    }
#line 4193 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1578 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
    }
#line 4201 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1581 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
    }
#line 4209 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1584 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
    }
#line 4217 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1587 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtInt,EbpHigh);
    }
#line 4225 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1590 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtBool,EbpHigh);
    }
#line 4233 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1593 "hlslang.y" /* yacc.c:1646  */
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[-1].lex).i > 4 || (yyvsp[-1].lex).i < 1 ) {
            parseContext.error((yyvsp[-4].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtFloat, qual, (yyvsp[-5].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtFloat, qual, (yyvsp[-5].lex).line);
            (yyval.interm.type).setVector((yyvsp[-1].lex).i);
        }
    }
#line 4249 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1604 "hlslang.y" /* yacc.c:1646  */
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[-1].lex).i > 4 || (yyvsp[-1].lex).i < 1 ) {
            parseContext.error((yyvsp[-4].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtInt, qual, (yyvsp[-5].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtInt, qual, (yyvsp[-5].lex).line);
            (yyval.interm.type).setVector((yyvsp[-1].lex).i);
        }
    }
#line 4265 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1615 "hlslang.y" /* yacc.c:1646  */
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[-1].lex).i > 4 || (yyvsp[-1].lex).i < 1 ) {
            parseContext.error((yyvsp[-4].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtBool, qual, (yyvsp[-5].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtBool, qual, (yyvsp[-5].lex).line);
            (yyval.interm.type).setVector((yyvsp[-1].lex).i);
        }
    }
#line 4281 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1626 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(2);
    }
#line 4290 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1630 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(3);
    }
#line 4299 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1634 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(4);
    }
#line 4308 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1638 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(2);
    }
#line 4317 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1642 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(3);
    }
#line 4326 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1646 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(4);
    }
#line 4335 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1650 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(2);
    }
#line 4344 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1654 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(3);
    }
#line 4353 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1658 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(4);
    }
#line 4362 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1662 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(2);
    }
#line 4371 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1666 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(3);
    }
#line 4380 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1670 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(4);
    }
#line 4389 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1674 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(2);
    }
#line 4398 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1678 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(3);
    }
#line 4407 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1682 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(4);
    }
#line 4416 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1686 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 4425 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1690 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float2x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 4435 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1695 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float2x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 4445 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1700 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float3x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 4455 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1705 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 4464 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1709 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float3x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 4474 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1714 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float4x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 4484 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1719 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("float4x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 4494 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1724 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 4503 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1728 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 4512 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1732 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half2x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 4522 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1737 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half2x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 4532 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1742 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half3x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 4542 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1747 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 4551 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1751 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half3x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 4561 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1756 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half4x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 4571 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1761 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("half4x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 4581 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1766 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 4590 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1770 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 4599 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1774 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed2x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 4609 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1779 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed2x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 4619 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1784 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed3x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 4629 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1789 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 4638 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1793 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed3x4", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 4648 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1798 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed4x2", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 4658 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1803 "hlslang.y" /* yacc.c:1646  */
    {
		NONSQUARE_MATRIX_CHECK("fixed4x3", (yyvsp[0].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 4668 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1808 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 4677 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1812 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtTexture,EbpUndefined);
    }
#line 4685 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1815 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerGeneric,EbpUndefined);
    }
#line 4693 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1818 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler1D,EbpUndefined);
    }
#line 4701 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1821 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler2D,EbpUndefined);
    }
#line 4709 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1824 "hlslang.y" /* yacc.c:1646  */
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler2D,EbpMedium);
	}
#line 4717 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1827 "hlslang.y" /* yacc.c:1646  */
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler2D,EbpHigh);
	}
#line 4725 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1830 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler3D,EbpLow);
    }
#line 4733 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1833 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerCube,EbpUndefined);
    }
#line 4741 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1836 "hlslang.y" /* yacc.c:1646  */
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerCube,EbpMedium);
	}
#line 4749 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1839 "hlslang.y" /* yacc.c:1646  */
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerCube,EbpHigh);
	}
#line 4757 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1842 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerRect,EbpUndefined);
    }
#line 4765 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1845 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSamplerRectShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    }
#line 4773 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1848 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler1DShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    }
#line 4781 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1851 "hlslang.y" /* yacc.c:1646  */
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler2DShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    }
#line 4789 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1854 "hlslang.y" /* yacc.c:1646  */
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtSampler2DArray,EbpLow);
	}
#line 4797 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1857 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.type) = (yyvsp[0].interm.type);
        (yyval.interm.type).qualifier = parseContext.getDefaultQualifier();
    }
#line 4806 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1861 "hlslang.y" /* yacc.c:1646  */
    {
        //
        // This is for user defined type names.  The lexical phase looked up the
        // type.
        //
        TType& structure = static_cast<TVariable*>((yyvsp[0].lex).symbol)->getType();
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[0].lex),EbtStruct,EbpUndefined);
        (yyval.interm.type).userDef = &structure;
    }
#line 4820 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1873 "hlslang.y" /* yacc.c:1646  */
    {
        TType* structure = new TType((yyvsp[-1].interm.typeList), *(yyvsp[-3].lex).string, EbpUndefined, (yyvsp[-3].lex).line);
        TVariable* userTypeDef = new TVariable((yyvsp[-3].lex).string, *structure, true);
        if (! parseContext.symbolTable.insert(*userTypeDef)) {
            parseContext.error((yyvsp[-3].lex).line, "redefinition", (yyvsp[-3].lex).string->c_str(), "struct");
            parseContext.recover();
        }
        (yyval.interm.type).setBasic(EbtStruct, EvqTemporary, (yyvsp[-4].lex).line);
        (yyval.interm.type).userDef = structure;
    }
#line 4835 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1883 "hlslang.y" /* yacc.c:1646  */
    {
        TType* structure = new TType((yyvsp[-1].interm.typeList), TString(""), EbpUndefined, (yyvsp[-3].lex).line);
        (yyval.interm.type).setBasic(EbtStruct, EvqTemporary, (yyvsp[-3].lex).line);
        (yyval.interm.type).userDef = structure;
    }
#line 4845 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1891 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeList) = (yyvsp[0].interm.typeList);
    }
#line 4853 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1894 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);
        for (unsigned int i = 0; i < (yyvsp[0].interm.typeList)->size(); ++i) {
            for (unsigned int j = 0; j < (yyval.interm.typeList)->size(); ++j) {
                if ((*(yyval.interm.typeList))[j].type->getFieldName() == (*(yyvsp[0].interm.typeList))[i].type->getFieldName()) {
                    parseContext.error((*(yyvsp[0].interm.typeList))[i].line, "duplicate field name in structure:", "struct", (*(yyvsp[0].interm.typeList))[i].type->getFieldName().c_str());
                    parseContext.recover();
                }
            }
            (yyval.interm.typeList)->push_back((*(yyvsp[0].interm.typeList))[i]);
        }
    }
#line 4870 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1909 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);

        if (parseContext.voidErrorCheck((yyvsp[-2].interm.type).line, (*(yyvsp[-1].interm.typeList))[0].type->getFieldName(), (yyvsp[-2].interm.type))) {
            parseContext.recover();
        }
        for (unsigned int i = 0; i < (yyval.interm.typeList)->size(); ++i) {
            //
            // Careful not to replace already know aspects of type, like array-ness
            //
            TType* type = (*(yyval.interm.typeList))[i].type;
            type->setBasicType((yyvsp[-2].interm.type).type);
            type->setPrecision((yyvsp[-2].interm.type).precision);
            type->setColsCount((yyvsp[-2].interm.type).matcols);
            type->setRowsCount((yyvsp[-2].interm.type).matrows);
            type->setMatrix((yyvsp[-2].interm.type).matrix);
            
            // don't allow arrays of arrays
            if (type->isArray()) {
                if (parseContext.arrayTypeErrorCheck((yyvsp[-2].interm.type).line, (yyvsp[-2].interm.type)))
                    parseContext.recover();
            }
            if ((yyvsp[-2].interm.type).array)
                type->setArraySize((yyvsp[-2].interm.type).arraySize);
            if ((yyvsp[-2].interm.type).userDef) {
                type->setStruct((yyvsp[-2].interm.type).userDef->getStruct());
                type->setTypeName((yyvsp[-2].interm.type).userDef->getTypeName());
            }
        }
    }
#line 4905 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1942 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeList) = NewPoolTTypeList();
        (yyval.interm.typeList)->push_back((yyvsp[0].interm.typeLine));
    }
#line 4914 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1946 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeList)->push_back((yyvsp[0].interm.typeLine));
    }
#line 4922 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1952 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[0].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[0].lex).string);
    }
#line 4932 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1957 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[-2].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[-2].lex).string);
        (yyval.interm.typeLine).type->setSemantic(*(yyvsp[0].lex).string);
    }
#line 4943 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1963 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[-3].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[-3].lex).string);

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[-2].lex).line, (yyvsp[-1].interm.intermTypedNode), size))
            parseContext.recover();
        (yyval.interm.typeLine).type->setArraySize(size);
    }
#line 4958 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1973 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[-5].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[-5].lex).string);

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[-4].lex).line, (yyvsp[-3].interm.intermTypedNode), size))
            parseContext.recover();
        (yyval.interm.typeLine).type->setArraySize(size);
        (yyval.interm.typeLine).type->setSemantic(*(yyvsp[0].lex).string);
    }
#line 4974 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1989 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 4980 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1990 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 4986 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1991 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 4992 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1995 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermDeclaration); }
#line 4998 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1999 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermAggregate); }
#line 5004 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 2000 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5010 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 2006 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5016 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 2007 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5022 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 2008 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5028 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 2009 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5034 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 2010 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5040 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 2014 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermAggregate) = 0; }
#line 5046 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 2015 "hlslang.y" /* yacc.c:1646  */
    { parseContext.symbolTable.push(); }
#line 5052 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 2015 "hlslang.y" /* yacc.c:1646  */
    { parseContext.symbolTable.pop(); }
#line 5058 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 2015 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-2].interm.intermAggregate) != 0)
            (yyvsp[-2].interm.intermAggregate)->setOperator(EOpSequence);
        (yyval.interm.intermAggregate) = (yyvsp[-2].interm.intermAggregate);
    }
#line 5068 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 2023 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5074 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 2024 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 5080 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 2029 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = 0;
    }
#line 5088 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 2032 "hlslang.y" /* yacc.c:1646  */
    {
        if ((yyvsp[-1].interm.intermAggregate))
            (yyvsp[-1].interm.intermAggregate)->setOperator(EOpSequence);
        (yyval.interm.intermNode) = (yyvsp[-1].interm.intermAggregate);
    }
#line 5098 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 2040 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermAggregate) = ir_make_aggregate((yyvsp[0].interm.intermNode), gNullSourceLoc); 
    }
#line 5106 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 2043 "hlslang.y" /* yacc.c:1646  */
    { 
        (yyval.interm.intermAggregate) = ir_grow_aggregate((yyvsp[-1].interm.intermAggregate), (yyvsp[0].interm.intermNode), gNullSourceLoc);
    }
#line 5114 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 2049 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = 0; }
#line 5120 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 2050 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = static_cast<TIntermNode*>((yyvsp[-1].interm.intermTypedNode)); }
#line 5126 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 2054 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.boolErrorCheck((yyvsp[-4].lex).line, (yyvsp[-2].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermNode) = ir_add_selection((yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.nodePair), (yyvsp[-4].lex).line, parseContext.infoSink);
    }
#line 5136 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 2062 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.nodePair).node1 = (yyvsp[-2].interm.intermNode);
        (yyval.interm.nodePair).node2 = (yyvsp[0].interm.intermNode);
    }
#line 5145 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 2066 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.nodePair).node1 = (yyvsp[0].interm.intermNode);
        (yyval.interm.nodePair).node2 = 0;
    }
#line 5154 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 2076 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        if (parseContext.boolErrorCheck((yyvsp[0].interm.intermTypedNode)->getLine(), (yyvsp[0].interm.intermTypedNode)))
            parseContext.recover();
    }
#line 5164 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 2081 "hlslang.y" /* yacc.c:1646  */
    {
        TIntermSymbol* symbol;
        if (parseContext.structQualifierErrorCheck((yyvsp[-2].lex).line, (yyvsp[-3].interm.type)))
            parseContext.recover();
        if (parseContext.boolErrorCheck((yyvsp[-2].lex).line, (yyvsp[-3].interm.type)))
            parseContext.recover();

        if (!parseContext.executeInitializer((yyvsp[-2].lex).line, *(yyvsp[-2].lex).string, (yyvsp[-3].interm.type), (yyvsp[0].interm.intermTypedNode), symbol)) {
			(yyval.interm.intermTypedNode) = ir_add_declaration(symbol, (yyvsp[0].interm.intermTypedNode), (yyvsp[-2].lex).line, parseContext);
        } else {
            parseContext.recover();
            (yyval.interm.intermTypedNode) = 0;
        }
    }
#line 5183 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 2098 "hlslang.y" /* yacc.c:1646  */
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; }
#line 5189 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 2098 "hlslang.y" /* yacc.c:1646  */
    {
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_add_loop(ELoopWhile, (yyvsp[-2].interm.intermTypedNode), 0, (yyvsp[0].interm.intermNode), (yyvsp[-5].lex).line);
        --parseContext.loopNestingLevel;
    }
#line 5199 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 2103 "hlslang.y" /* yacc.c:1646  */
    { ++parseContext.loopNestingLevel; }
#line 5205 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 2103 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.boolErrorCheck((yyvsp[0].lex).line, (yyvsp[-2].interm.intermTypedNode)))
            parseContext.recover();
                    
        (yyval.interm.intermNode) = ir_add_loop(ELoopDoWhile, (yyvsp[-2].interm.intermTypedNode), 0, (yyvsp[-5].interm.intermNode), (yyvsp[-4].lex).line);
        --parseContext.loopNestingLevel;
    }
#line 5217 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 2110 "hlslang.y" /* yacc.c:1646  */
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; }
#line 5223 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 2110 "hlslang.y" /* yacc.c:1646  */
    {
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_make_aggregate((yyvsp[-3].interm.intermNode), (yyvsp[-5].lex).line);
        (yyval.interm.intermNode) = ir_grow_aggregate(
                (yyval.interm.intermNode),
                ir_add_loop(ELoopFor, reinterpret_cast<TIntermTyped*>((yyvsp[-2].interm.nodePair).node1), reinterpret_cast<TIntermTyped*>((yyvsp[-2].interm.nodePair).node2), (yyvsp[0].interm.intermNode), (yyvsp[-6].lex).line),
                (yyvsp[-6].lex).line);
        (yyval.interm.intermNode)->getAsAggregate()->setOperator(EOpSequence);
        --parseContext.loopNestingLevel;
    }
#line 5238 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 2123 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 5246 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 2126 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 5254 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 2132 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 5262 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 2135 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermTypedNode) = 0;
    }
#line 5270 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 2141 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.nodePair).node1 = (yyvsp[-1].interm.intermTypedNode);
        (yyval.interm.nodePair).node2 = 0;
    }
#line 5279 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 2145 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.nodePair).node1 = (yyvsp[-2].interm.intermTypedNode);
        (yyval.interm.nodePair).node2 = (yyvsp[0].interm.intermTypedNode);
    }
#line 5288 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 2152 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error((yyvsp[-1].lex).line, "continue statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        (yyval.interm.intermNode) = ir_add_branch(EOpContinue, (yyvsp[-1].lex).line);
    }
#line 5300 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 2159 "hlslang.y" /* yacc.c:1646  */
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error((yyvsp[-1].lex).line, "break statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        (yyval.interm.intermNode) = ir_add_branch(EOpBreak, (yyvsp[-1].lex).line);
    }
#line 5312 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 2166 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = ir_add_branch(EOpReturn, (yyvsp[-1].lex).line);
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid) {
            parseContext.error((yyvsp[-1].lex).line, "non-void function must return a value", "return", "");
            parseContext.recover();
        }
    }
#line 5324 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 2173 "hlslang.y" /* yacc.c:1646  */
    {
        TIntermTyped *temp = (yyvsp[-1].interm.intermTypedNode);
        if (parseContext.currentFunctionType->getBasicType() == EbtVoid) {
            parseContext.error((yyvsp[-2].lex).line, "void function cannot return a value", "return", "");
            parseContext.recover();
        } else if (*(parseContext.currentFunctionType) != (yyvsp[-1].interm.intermTypedNode)->getType()) {
            TOperator op = parseContext.getConstructorOp(*(parseContext.currentFunctionType));
            if (op != EOpNull)
                temp = parseContext.constructBuiltIn((parseContext.currentFunctionType), op, (yyvsp[-1].interm.intermTypedNode), (yyvsp[-2].lex).line, false);
            else
                temp = 0;
            if (temp == 0) {
                parseContext.error((yyvsp[-2].lex).line, "function return is not matching type:", "return", "");
                parseContext.recover();
                temp = (yyvsp[-1].interm.intermTypedNode);
            }
        }
        (yyval.interm.intermNode) = ir_add_branch(EOpReturn, temp, (yyvsp[-2].lex).line);
        parseContext.functionReturnsValue = true;
    }
#line 5349 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 2193 "hlslang.y" /* yacc.c:1646  */
    {
		// Jim: using discard when compiling vertex shaders should not be considered a syntactic error, instead,
		// we should issue a semantic error only if the code path is actually executed. (Not yet implemented)
        //FRAG_ONLY("discard", $1.line);
        (yyval.interm.intermNode) = ir_add_branch(EOpKill, (yyvsp[-1].lex).line);
    }
#line 5360 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 2204 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
        parseContext.treeRoot = (yyval.interm.intermNode);
    }
#line 5369 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 2208 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = ir_grow_aggregate((yyvsp[-1].interm.intermNode), (yyvsp[0].interm.intermNode), gNullSourceLoc);
        parseContext.treeRoot = (yyval.interm.intermNode);
    }
#line 5378 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 2215 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 5386 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 2218 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermDeclaration);
    }
#line 5394 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 2221 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.intermNode) = 0; }
#line 5400 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 2225 "hlslang.y" /* yacc.c:1646  */
    {
        TFunction& function = *((yyvsp[0].interm).function);
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find(function.getMangledName()));
        //
        // Note:  'prevDec' could be 'function' if this is the first time we've seen function
        // as it would have just been put in the symbol table.  Otherwise, we're looking up
        // an earlier occurance.
        //
        if (prevDec->isDefined()) {
            //
            // Then this function already has a body.
            //
            parseContext.error((yyvsp[0].interm).line, "function already has a body", function.getName().c_str(), "");
            parseContext.recover();
        }
        prevDec->setDefined();

        //
        // New symbol table scope for body of function plus its arguments
        //
        parseContext.symbolTable.push();

        //
        // Remember the return type for later checking for RETURN statements.
        //
        parseContext.currentFunctionType = &(prevDec->getReturnType());
        parseContext.functionReturnsValue = false;

        //
        // Insert parameters into the symbol table.
        // If the parameter has no name, it's not an error, just don't insert it
        // (could be used for unused args).
        //
        // Also, accumulate the list of parameters into the HIL, so lower level code
        // knows where to find parameters.
        //
        TIntermAggregate* paramNodes = new TIntermAggregate;
        for (int i = 0; i < function.getParamCount(); i++) {
            TParameter& param = function[i];
            if (param.name != 0) {
                TVariable *variable = new TVariable(param.name, param.info, *param.type);
                //
                // Insert the parameters with name in the symbol table.
                //
                if (! parseContext.symbolTable.insert(*variable)) {
                    parseContext.error((yyvsp[0].interm).line, "redefinition", variable->getName().c_str(), "");
                    parseContext.recover();
                    delete variable;
                }
                //
                // Transfer ownership of name pointer to symbol table.
                //
                param.name = 0;

                //
                // Add the parameter to the HIL
                //                
                paramNodes = ir_grow_aggregate(
                                               paramNodes, 
                                               ir_add_symbol(variable, (yyvsp[0].interm).line),
                                               (yyvsp[0].interm).line);
            } else {
                paramNodes = ir_grow_aggregate(paramNodes, ir_add_symbol_internal(0, "", param.info, *param.type, (yyvsp[0].interm).line), (yyvsp[0].interm).line);
            }
        }
        ir_set_aggregate_op(paramNodes, EOpParameters, (yyvsp[0].interm).line);
        (yyvsp[0].interm).intermAggregate = paramNodes;
        parseContext.loopNestingLevel = 0;
    }
#line 5474 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 2294 "hlslang.y" /* yacc.c:1646  */
    {
        //?? Check that all paths return a value if return type != void ?
        //   May be best done as post process phase on intermediate code
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid && ! parseContext.functionReturnsValue) {
            parseContext.error((yyvsp[-2].interm).line, "function does not return a value:", "", (yyvsp[-2].interm).function->getName().c_str());
            parseContext.recover();
        }
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_grow_aggregate((yyvsp[-2].interm).intermAggregate, (yyvsp[0].interm.intermNode), gNullSourceLoc);
        ir_set_aggregate_op((yyval.interm.intermNode), EOpFunction, (yyvsp[-2].interm).line);
        (yyval.interm.intermNode)->getAsAggregate()->setName((yyvsp[-2].interm).function->getMangledName().c_str());
        (yyval.interm.intermNode)->getAsAggregate()->setPlainName((yyvsp[-2].interm).function->getName().c_str());
        (yyval.interm.intermNode)->getAsAggregate()->setType((yyvsp[-2].interm).function->getReturnType());
        
	if ( (yyvsp[-2].interm).function->getInfo())
	    (yyval.interm.intermNode)->getAsAggregate()->setSemantic((yyvsp[-2].interm).function->getInfo()->getSemantic());
    }
#line 5496 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 2314 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermAggregate);
    }
#line 5504 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 2317 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermAggregate);
    }
#line 5512 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 2324 "hlslang.y" /* yacc.c:1646  */
    {
        //create a new aggNode
       (yyval.interm.intermAggregate) = ir_make_aggregate( (yyvsp[0].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode)->getLine());       
    }
#line 5521 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 2328 "hlslang.y" /* yacc.c:1646  */
    {
       //take the inherited aggNode and return it
       (yyval.interm.intermAggregate) = (yyvsp[0].interm.intermTypedNode)->getAsAggregate();
    }
#line 5530 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 2332 "hlslang.y" /* yacc.c:1646  */
    {
        // append to the aggNode
       (yyval.interm.intermAggregate) = ir_grow_aggregate( (yyvsp[-2].interm.intermAggregate), (yyvsp[0].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode)->getLine());       
    }
#line 5539 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 2336 "hlslang.y" /* yacc.c:1646  */
    {
       // append all children or $3 to $1
       (yyval.interm.intermAggregate) = parseContext.mergeAggregates( (yyvsp[-2].interm.intermAggregate), (yyvsp[0].interm.intermTypedNode)->getAsAggregate());
    }
#line 5548 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 2343 "hlslang.y" /* yacc.c:1646  */
    {
        //empty annotation
      (yyval.interm.ann) = 0;
    }
#line 5557 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 2347 "hlslang.y" /* yacc.c:1646  */
    {
      (yyval.interm.ann) = (yyvsp[-1].interm.ann);
    }
#line 5565 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 2353 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.interm.ann) = new TAnnotation;
		(yyval.interm.ann)->addKey( *(yyvsp[0].lex).string);
    }
#line 5574 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 2357 "hlslang.y" /* yacc.c:1646  */
    {
        (yyvsp[-1].interm.ann)->addKey( *(yyvsp[0].lex).string);
		(yyval.interm.ann) = (yyvsp[-1].interm.ann);
    }
#line 5583 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 2364 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.lex).string = (yyvsp[-3].lex).string;
    }
#line 5591 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 2370 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5597 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 2371 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5603 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 2372 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5609 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 2373 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5615 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 2374 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5621 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 2375 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5627 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 2376 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5633 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 2377 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5639 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 2378 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5645 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 2379 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5651 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 2380 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5657 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 2381 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5663 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 2382 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5669 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 2383 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5675 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 2384 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5681 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 2385 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5687 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 2386 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5693 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 2387 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5699 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 2388 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5705 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 2389 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5711 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 2390 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5717 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 2394 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5723 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 2395 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5729 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 2396 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5735 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 2397 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5741 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 2401 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.lex).f = (float)(yyvsp[0].lex).i;
	}
#line 5749 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 2404 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.lex).f = ((yyvsp[0].lex).b) ? 1.0f : 0.0f;
	}
#line 5757 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 2407 "hlslang.y" /* yacc.c:1646  */
    {
		(yyval.lex).f = (yyvsp[0].lex).f;
	}
#line 5765 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 2413 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5771 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 2417 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5777 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 2418 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5783 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 2422 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5789 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 2426 "hlslang.y" /* yacc.c:1646  */
    {
        (yyval.lex) = (yyvsp[-1].lex);
    }
#line 5797 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 2432 "hlslang.y" /* yacc.c:1646  */
    { (yyval.lex).string = (yyvsp[0].lex).string;}
#line 5803 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 308:
#line 2436 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = 0;}
#line 5809 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 2437 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[0].lex).string, 0); }
#line 5815 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 2438 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( "", *(yyvsp[0].lex).string, 0); }
#line 5821 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 2439 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( "", (yyvsp[0].interm.ann)); }
#line 5827 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 2440 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[-1].lex).string, (yyvsp[0].interm.ann)); }
#line 5833 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 2441 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[-1].lex).string, *(yyvsp[0].lex).string, 0); }
#line 5839 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 2442 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( "", *(yyvsp[-1].lex).string, (yyvsp[0].interm.ann)); }
#line 5845 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 2443 "hlslang.y" /* yacc.c:1646  */
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[-2].lex).string, *(yyvsp[-1].lex).string, (yyvsp[0].interm.ann)); }
#line 5851 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 2447 "hlslang.y" /* yacc.c:1646  */
    {
		TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst, 1), (yyvsp[-3].lex).line);
		constant->setValue(0.f);
		(yyval.interm.intermTypedNode) = constant;
	}
#line 5861 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 2452 "hlslang.y" /* yacc.c:1646  */
    {
	}
#line 5868 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 2457 "hlslang.y" /* yacc.c:1646  */
    { }
#line 5874 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 2458 "hlslang.y" /* yacc.c:1646  */
    { }
#line 5880 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 2462 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5886 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 2463 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5892 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 2464 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5898 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 2465 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5904 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 2466 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5910 "hlslang.tab.c" /* yacc.c:1646  */
    break;

  case 325:
#line 2467 "hlslang.y" /* yacc.c:1646  */
    {}
#line 5916 "hlslang.tab.c" /* yacc.c:1646  */
    break;


#line 5920 "hlslang.tab.c" /* yacc.c:1646  */
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
      yyerror (parseContext, YY_("syntax error"));
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
        yyerror (parseContext, yymsgp);
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
                      yytoken, &yylval, parseContext);
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
                  yystos[yystate], yyvsp, parseContext);
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
  yyerror (parseContext, YY_("memory exhausted"));
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
                  yytoken, &yylval, parseContext);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, parseContext);
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
#line 2470 "hlslang.y" /* yacc.c:1906  */

