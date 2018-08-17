
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 8 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"


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




/* Line 189 of yacc.c  */
#line 124 "hlslang.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 57 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"

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



/* Line 214 of yacc.c  */
#line 334 "hlslang.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 91 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"

    extern int yylex(YYSTYPE*, TParseContext&);


/* Line 264 of yacc.c  */
#line 351 "hlslang.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

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
/* YYNRULES -- Number of states.  */
#define YYNSTATES  497

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   393

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    17,    19,
      24,    26,    30,    33,    36,    38,    40,    42,    46,    49,
      52,    55,    57,    60,    64,    67,    69,    71,    73,    75,
      78,    81,    84,    89,    91,    93,    95,    97,    99,   103,
     107,   111,   113,   117,   121,   123,   127,   131,   133,   137,
     141,   145,   149,   151,   155,   159,   161,   165,   167,   171,
     173,   177,   179,   183,   185,   189,   191,   195,   197,   203,
     205,   209,   211,   213,   215,   217,   219,   221,   223,   225,
     227,   229,   231,   233,   237,   239,   242,   245,   248,   253,
     255,   257,   260,   264,   268,   271,   276,   280,   285,   291,
     299,   303,   306,   310,   313,   314,   316,   318,   320,   322,
     324,   329,   336,   344,   353,   363,   370,   372,   376,   382,
     389,   397,   406,   412,   414,   417,   419,   421,   424,   427,
     429,   431,   436,   438,   440,   442,   444,   446,   448,   455,
     462,   469,   471,   473,   475,   477,   479,   481,   483,   485,
     487,   489,   491,   493,   495,   497,   499,   501,   503,   505,
     507,   509,   511,   513,   515,   517,   519,   521,   523,   525,
     527,   529,   531,   533,   535,   537,   539,   541,   543,   545,
     547,   549,   551,   553,   555,   557,   559,   561,   563,   565,
     567,   569,   571,   573,   575,   577,   579,   581,   583,   585,
     587,   593,   598,   600,   603,   607,   609,   613,   615,   619,
     624,   631,   633,   635,   637,   639,   641,   643,   645,   647,
     649,   651,   653,   656,   657,   658,   664,   666,   668,   671,
     675,   677,   680,   682,   685,   691,   695,   697,   699,   704,
     705,   712,   713,   722,   723,   731,   733,   735,   737,   738,
     741,   745,   748,   751,   754,   758,   761,   763,   766,   768,
     770,   772,   773,   777,   781,   786,   788,   790,   794,   798,
     801,   805,   807,   810,   816,   818,   820,   822,   824,   826,
     828,   830,   832,   834,   836,   838,   840,   842,   844,   846,
     848,   850,   852,   854,   856,   858,   860,   862,   864,   866,
     868,   870,   872,   877,   879,   883,   887,   893,   896,   897,
     899,   901,   903,   906,   909,   912,   916,   921,   925,   927,
     930,   935,   942,   949,   954,   961
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    87,    -1,   140,    -1,    90,    -1,    89,
      -1,    91,    -1,   115,   167,   116,    -1,   141,    -1,   142,
     117,   143,   118,    -1,   144,    -1,   142,   121,    93,    -1,
     142,    96,    -1,   142,    97,    -1,   167,    -1,   145,    -1,
     146,    -1,   142,   121,   146,    -1,   148,   116,    -1,   147,
     116,    -1,   149,    66,    -1,   149,    -1,   149,   165,    -1,
     148,   122,   165,    -1,   150,   115,    -1,   182,    -1,    87,
      -1,    93,    -1,   142,    -1,    96,   151,    -1,    97,   151,
      -1,   152,   151,    -1,   115,   183,   116,   151,    -1,   129,
      -1,   127,    -1,   126,    -1,   128,    -1,   151,    -1,   153,
     130,   151,    -1,   153,   131,   151,    -1,   153,   132,   151,
      -1,   153,    -1,   154,   129,   153,    -1,   154,   127,   153,
      -1,   154,    -1,   155,    94,   154,    -1,   155,    95,   154,
      -1,   155,    -1,   156,   133,   155,    -1,   156,   134,   155,
      -1,   156,    98,   155,    -1,   156,    99,   155,    -1,   156,
      -1,   157,   100,   156,    -1,   157,   101,   156,    -1,   157,
      -1,   158,   137,   157,    -1,   158,    -1,   159,   136,   158,
      -1,   159,    -1,   160,   135,   159,    -1,   160,    -1,   161,
     102,   160,    -1,   161,    -1,   162,   104,   161,    -1,   162,
      -1,   163,   103,   162,    -1,   163,    -1,   163,   138,   167,
     123,   165,    -1,   164,    -1,   151,   166,   165,    -1,   124,
      -1,   105,    -1,   106,    -1,   108,    -1,   107,    -1,   114,
      -1,   109,    -1,   110,    -1,   111,    -1,   112,    -1,   113,
      -1,   165,    -1,   167,   122,   165,    -1,   164,    -1,   170,
     125,    -1,   178,   125,    -1,   171,   116,    -1,   171,   116,
     123,    87,    -1,   173,    -1,   172,    -1,   173,   175,    -1,
     172,   122,   175,    -1,   180,    87,   115,    -1,   182,    87,
      -1,   182,    87,   124,   189,    -1,   182,    87,   226,    -1,
     182,    87,   123,    87,    -1,   182,    87,   117,   168,   118,
      -1,   182,    87,   117,   168,   118,   123,    87,    -1,   181,
     176,   174,    -1,   176,   174,    -1,   181,   176,   177,    -1,
     176,   177,    -1,    -1,    61,    -1,    62,    -1,    63,    -1,
     182,    -1,   179,    -1,   178,   122,    87,   228,    -1,   178,
     122,    87,   117,   118,   228,    -1,   178,   122,    87,   117,
     168,   118,   228,    -1,   178,   122,    87,   117,   118,   228,
     124,   189,    -1,   178,   122,    87,   117,   168,   118,   228,
     124,   189,    -1,   178,   122,    87,   228,   124,   189,    -1,
     180,    -1,   180,    87,   228,    -1,   180,    87,   117,   118,
     228,    -1,   180,    87,   117,   168,   118,   228,    -1,   180,
      87,   117,   118,   228,   124,   189,    -1,   180,    87,   117,
     168,   118,   228,   124,   189,    -1,   180,    87,   228,   124,
     189,    -1,   182,    -1,   181,   182,    -1,     3,    -1,     4,
      -1,     4,     3,    -1,     3,     4,    -1,    64,    -1,   183,
      -1,   183,   117,   168,   118,    -1,    66,    -1,     6,    -1,
      10,    -1,     9,    -1,     7,    -1,     5,    -1,    82,   133,
       6,   122,    90,   134,    -1,    82,   133,     7,   122,    90,
     134,    -1,    82,   133,     5,   122,    90,   134,    -1,    25,
      -1,    26,    -1,    27,    -1,    28,    -1,    29,    -1,    30,
      -1,    31,    -1,    32,    -1,    33,    -1,    19,    -1,    20,
      -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,    34,
      -1,    35,    -1,    36,    -1,    37,    -1,    38,    -1,    39,
      -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,
      -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,    49,
      -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,    54,
      -1,    55,    -1,    56,    -1,    57,    -1,    58,    -1,    59,
      -1,    60,    -1,    85,    -1,    81,    -1,    68,    -1,    69,
      -1,    77,    -1,    78,    -1,    70,    -1,    71,    -1,    79,
      -1,    80,    -1,    76,    -1,    75,    -1,    72,    -1,    73,
      -1,    74,    -1,   184,    -1,    88,    -1,    65,    87,   119,
     185,   120,    -1,    65,   119,   185,   120,    -1,   186,    -1,
     185,   186,    -1,   182,   187,   125,    -1,   188,    -1,   187,
     122,   188,    -1,    87,    -1,    87,   123,    87,    -1,    87,
     117,   168,   118,    -1,    87,   117,   168,   118,   123,    87,
      -1,   165,    -1,   215,    -1,   229,    -1,   169,    -1,   193,
      -1,   192,    -1,   190,    -1,   199,    -1,   200,    -1,   203,
      -1,   210,    -1,   119,   120,    -1,    -1,    -1,   119,   194,
     198,   195,   120,    -1,   197,    -1,   192,    -1,   119,   120,
      -1,   119,   198,   120,    -1,   191,    -1,   198,   191,    -1,
     125,    -1,   167,   125,    -1,    16,   115,   167,   116,   201,
      -1,   191,    14,   191,    -1,   191,    -1,   167,    -1,   180,
      87,   124,   189,    -1,    -1,    67,   115,   204,   202,   116,
     196,    -1,    -1,    13,   205,   191,    67,   115,   167,   116,
     125,    -1,    -1,    15,   115,   206,   207,   209,   116,   196,
      -1,   199,    -1,   190,    -1,   202,    -1,    -1,   208,   125,
      -1,   208,   125,   167,    -1,    12,   125,    -1,    11,   125,
      -1,    18,   125,    -1,    18,   167,   125,    -1,    17,   125,
      -1,   212,    -1,   211,   212,    -1,   213,    -1,   169,    -1,
     125,    -1,    -1,   170,   214,   197,    -1,   119,   216,   120,
      -1,   119,   216,   122,   120,    -1,   165,    -1,   215,    -1,
     216,   122,   165,    -1,   216,   122,   215,    -1,   133,   134,
      -1,   133,   218,   134,    -1,   219,    -1,   218,   219,    -1,
     220,    87,   124,   221,   125,    -1,     6,    -1,    10,    -1,
       9,    -1,     7,    -1,     5,    -1,     8,    -1,    19,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,
      25,    -1,    26,    -1,    27,    -1,    28,    -1,    29,    -1,
      30,    -1,    31,    -1,    32,    -1,    33,    -1,   222,    -1,
      92,    -1,   223,    -1,   225,    -1,    90,    -1,    91,    -1,
      89,    -1,   220,   115,   224,   116,    -1,   222,    -1,   224,
     122,   222,    -1,   119,   224,   120,    -1,   123,    84,   115,
      87,   116,    -1,   123,    87,    -1,    -1,   227,    -1,   226,
      -1,   217,    -1,   227,   217,    -1,   227,   226,    -1,   226,
     217,    -1,   227,   226,   217,    -1,    86,   119,   230,   120,
      -1,    86,   119,   120,    -1,   231,    -1,   230,   231,    -1,
      87,   124,    87,   125,    -1,    87,   124,   133,    87,   134,
     125,    -1,    87,   124,   115,    87,   116,   125,    -1,    85,
     124,    87,   125,    -1,    85,   124,   133,    87,   134,   125,
      -1,    85,   124,   115,    87,   116,   125,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   176,   176,   214,   217,   222,   227,   232,   238,   241,
     305,   308,   395,   405,   418,   432,   548,   551,   569,   573,
     580,   584,   591,   600,   612,   620,   647,   659,   669,   672,
     682,   692,   710,   745,   746,   747,   748,   754,   755,   756,
     757,   761,   762,   763,   767,   768,   769,   773,   774,   775,
     776,   777,   781,   782,   783,   787,   788,   792,   793,   797,
     798,   802,   803,   807,   808,   812,   813,   817,   818,   833,
     834,   848,   849,   850,   851,   852,   853,   854,   855,   856,
     857,   858,   862,   865,   876,   884,   885,   889,   922,   959,
     962,   969,   977,   998,  1030,  1041,  1054,  1066,  1078,  1096,
    1125,  1130,  1140,  1145,  1155,  1158,  1161,  1164,  1170,  1177,
    1180,  1198,  1222,  1250,  1277,  1310,  1344,  1347,  1365,  1388,
    1414,  1440,  1470,  1532,  1535,  1552,  1555,  1558,  1561,  1564,
    1572,  1575,  1590,  1593,  1596,  1599,  1602,  1605,  1608,  1619,
    1630,  1641,  1645,  1649,  1653,  1657,  1661,  1665,  1669,  1673,
    1677,  1681,  1685,  1689,  1693,  1697,  1701,  1705,  1710,  1715,
    1720,  1724,  1729,  1734,  1739,  1743,  1747,  1752,  1757,  1762,
    1766,  1771,  1776,  1781,  1785,  1789,  1794,  1799,  1804,  1808,
    1813,  1818,  1823,  1827,  1830,  1833,  1836,  1839,  1842,  1845,
    1848,  1851,  1854,  1857,  1860,  1863,  1866,  1869,  1872,  1876,
    1888,  1898,  1906,  1909,  1924,  1957,  1961,  1967,  1972,  1978,
    1988,  2004,  2005,  2006,  2010,  2014,  2015,  2021,  2022,  2023,
    2024,  2025,  2029,  2030,  2030,  2030,  2038,  2039,  2044,  2047,
    2055,  2058,  2064,  2065,  2069,  2077,  2081,  2091,  2096,  2113,
    2113,  2118,  2118,  2125,  2125,  2138,  2141,  2147,  2150,  2156,
    2160,  2167,  2174,  2181,  2188,  2208,  2219,  2223,  2230,  2233,
    2236,  2240,  2240,  2329,  2332,  2339,  2343,  2347,  2351,  2358,
    2362,  2368,  2372,  2379,  2385,  2386,  2387,  2388,  2389,  2390,
    2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2400,
    2401,  2402,  2403,  2404,  2405,  2409,  2410,  2411,  2412,  2416,
    2419,  2422,  2428,  2432,  2433,  2437,  2441,  2447,  2451,  2452,
    2453,  2454,  2455,  2456,  2457,  2458,  2462,  2465,  2470,  2475,
    2482,  2485,  2488,  2491,  2494,  2497
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "sampler_init_item", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
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

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -387
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

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -90
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

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (parseContext, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, parseContext)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, parseContext); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, TParseContext& parseContext)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parseContext)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    TParseContext& parseContext;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (parseContext);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, TParseContext& parseContext)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parseContext)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    TParseContext& parseContext;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parseContext);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, TParseContext& parseContext)
#else
static void
yy_reduce_print (yyvsp, yyrule, parseContext)
    YYSTYPE *yyvsp;
    int yyrule;
    TParseContext& parseContext;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , parseContext);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parseContext); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, TParseContext& parseContext)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parseContext)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    TParseContext& parseContext;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parseContext);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (TParseContext& parseContext);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (TParseContext& parseContext)
#else
int
yyparse (parseContext)
    TParseContext& parseContext;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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
  *++yyvsp = yylval;

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
     `$$ = $1'.

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

/* Line 1455 of yacc.c  */
#line 176 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // The symbol table search was done in the lexical phase
        const TSymbol* symbol = (yyvsp[(1) - (1)].lex).symbol;
        const TVariable* variable;
        if (symbol == 0) {
            parseContext.error((yyvsp[(1) - (1)].lex).line, "undeclared identifier", (yyvsp[(1) - (1)].lex).string->c_str(), "");
            parseContext.recover();
            TType type(EbtFloat, EbpUndefined);
            TVariable* fakeVariable = new TVariable((yyvsp[(1) - (1)].lex).string, type);
            parseContext.symbolTable.insert(*fakeVariable);
            variable = fakeVariable;
        } else {
            // This identifier can only be a variable type symbol 
            if (! symbol->isVariable()) {
                parseContext.error((yyvsp[(1) - (1)].lex).line, "variable expected", (yyvsp[(1) - (1)].lex).string->c_str(), "");
                parseContext.recover();
            }
            variable = static_cast<const TVariable*>(symbol);
        }

        // don't delete $1.string, it's used by error recovery, and the pool
        // pop will reclaim the memory
		
		if (variable->getType().getQualifier() == EvqConst && variable->constValue)
		{
			TIntermConstant* c = ir_add_constant(variable->getType(), (yyvsp[(1) - (1)].lex).line);
			c->copyValuesFrom(*variable->constValue);
			(yyval.interm.intermTypedNode) = c;
		}
		else
		{
			TIntermSymbol* sym = ir_add_symbol(variable, (yyvsp[(1) - (1)].lex).line);
			(yyval.interm.intermTypedNode) = sym;
		}
    ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 214 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 217 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[(1) - (1)].lex).line);
		constant->setValue((yyvsp[(1) - (1)].lex).i);
		(yyval.interm.intermTypedNode) = constant;
    ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 222 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[(1) - (1)].lex).line);
		constant->setValue((yyvsp[(1) - (1)].lex).f);
		(yyval.interm.intermTypedNode) = constant;
    ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 227 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TIntermConstant* constant = ir_add_constant(TType(EbtBool, EbpUndefined, EvqConst), (yyvsp[(1) - (1)].lex).line);
		constant->setValue((yyvsp[(1) - (1)].lex).b);
		(yyval.interm.intermTypedNode) = constant;
    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 232 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(2) - (3)].interm.intermTypedNode);
    ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 238 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { 
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 241 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (!(yyvsp[(1) - (4)].interm.intermTypedNode)) {
            parseContext.error((yyvsp[(2) - (4)].lex).line, " left of '[' is null ", "expression", "");
            YYERROR;
        }
        if (!(yyvsp[(1) - (4)].interm.intermTypedNode)->isArray() && !(yyvsp[(1) - (4)].interm.intermTypedNode)->isMatrix() && !(yyvsp[(1) - (4)].interm.intermTypedNode)->isVector()) {
            if ((yyvsp[(1) - (4)].interm.intermTypedNode)->getAsSymbolNode())
                parseContext.error((yyvsp[(2) - (4)].lex).line, " left of '[' is not of type array, matrix, or vector ", (yyvsp[(1) - (4)].interm.intermTypedNode)->getAsSymbolNode()->getSymbol().c_str(), "");
            else
                parseContext.error((yyvsp[(2) - (4)].lex).line, " left of '[' is not of type array, matrix, or vector ", "expression", "");
            parseContext.recover();
        }
		if ((yyvsp[(3) - (4)].interm.intermTypedNode)->getQualifier() == EvqConst) {
			if (((yyvsp[(1) - (4)].interm.intermTypedNode)->isVector() || (yyvsp[(1) - (4)].interm.intermTypedNode)->isMatrix()) && (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getRowsCount() <= (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt() && !(yyvsp[(1) - (4)].interm.intermTypedNode)->isArray() ) {
				parseContext.error((yyvsp[(2) - (4)].lex).line, "", "[", "field selection out of range '%d'", (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt());
				parseContext.recover();
			} else {
				if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isArray()) {
					if ((yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getArraySize() == 0) {
						if ((yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getMaxArraySize() <= (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt()) {
							if (parseContext.arraySetMaxSize((yyvsp[(1) - (4)].interm.intermTypedNode)->getAsSymbolNode(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getTypePointer(), (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt(), true, (yyvsp[(2) - (4)].lex).line))
								parseContext.recover(); 
						} else {
							if (parseContext.arraySetMaxSize((yyvsp[(1) - (4)].interm.intermTypedNode)->getAsSymbolNode(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getTypePointer(), 0, false, (yyvsp[(2) - (4)].lex).line))
								parseContext.recover(); 
						}
					} else if ( (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt() >= (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getArraySize()) {
						parseContext.error((yyvsp[(2) - (4)].lex).line, "", "[", "array index out of range '%d'", (yyvsp[(3) - (4)].interm.intermTypedNode)->getAsConstant()->toInt());
						parseContext.recover();
					}
				}
				(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexDirect, (yyvsp[(1) - (4)].interm.intermTypedNode), (yyvsp[(3) - (4)].interm.intermTypedNode), (yyvsp[(2) - (4)].lex).line);
			}
		} else {
			if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isArray() && (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getArraySize() == 0) {
				parseContext.error((yyvsp[(2) - (4)].lex).line, "", "[", "array must be redeclared with a size before being indexed with a variable");
				parseContext.recover();
			}
			
			(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexIndirect, (yyvsp[(1) - (4)].interm.intermTypedNode), (yyvsp[(3) - (4)].interm.intermTypedNode), (yyvsp[(2) - (4)].lex).line);
		}
        if ((yyval.interm.intermTypedNode) == 0) {
            TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[(2) - (4)].lex).line);
			constant->setValue(0.f);
			(yyval.interm.intermTypedNode) = constant;
        } else if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isArray()) {
            if ((yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getStruct())
                (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getStruct(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getTypeName(), EbpUndefined, (yyvsp[(1) - (4)].interm.intermTypedNode)->getLine()));
            else
                (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getPrecision(), EvqTemporary, (yyvsp[(1) - (4)].interm.intermTypedNode)->getColsCount(),(yyvsp[(1) - (4)].interm.intermTypedNode)->getRowsCount(),  (yyvsp[(1) - (4)].interm.intermTypedNode)->isMatrix()));
                
            if ((yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getQualifier() == EvqConst)
                (yyval.interm.intermTypedNode)->getTypePointer()->changeQualifier(EvqConst);
        } else if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isMatrix() && (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getQualifier() == EvqConst)         
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getPrecision(), EvqConst, 1, (yyvsp[(1) - (4)].interm.intermTypedNode)->getColsCount()));
        else if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isMatrix())            
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getPrecision(), EvqTemporary, 1, (yyvsp[(1) - (4)].interm.intermTypedNode)->getColsCount()));
        else if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isVector() && (yyvsp[(1) - (4)].interm.intermTypedNode)->getType().getQualifier() == EvqConst)          
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getPrecision(), EvqConst));
        else if ((yyvsp[(1) - (4)].interm.intermTypedNode)->isVector())       
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (4)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (4)].interm.intermTypedNode)->getPrecision(), EvqTemporary));
        else
            (yyval.interm.intermTypedNode)->setType((yyvsp[(1) - (4)].interm.intermTypedNode)->getType());
    ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 305 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 308 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (!(yyvsp[(1) - (3)].interm.intermTypedNode)) {
            parseContext.error((yyvsp[(3) - (3)].lex).line, "field selection on null object", ".", "");
            YYERROR;
        }
        if ((yyvsp[(1) - (3)].interm.intermTypedNode)->isArray()) {
            parseContext.error((yyvsp[(3) - (3)].lex).line, "cannot apply dot operator to an array", ".", "");
            parseContext.recover();
        }

        if ((yyvsp[(1) - (3)].interm.intermTypedNode)->isVector()) {
            TVectorFields fields;
            if (! parseContext.parseVectorFields(*(yyvsp[(3) - (3)].lex).string, (yyvsp[(1) - (3)].interm.intermTypedNode)->getRowsCount(), fields, (yyvsp[(3) - (3)].lex).line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

			(yyval.interm.intermTypedNode) = ir_add_vector_swizzle(fields, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, (yyvsp[(3) - (3)].lex).line);
        } else if ((yyvsp[(1) - (3)].interm.intermTypedNode)->isMatrix()) {
            TVectorFields fields;
            if (!parseContext.parseMatrixFields(*(yyvsp[(3) - (3)].lex).string, (yyvsp[(1) - (3)].interm.intermTypedNode)->getColsCount(), (yyvsp[(1) - (3)].interm.intermTypedNode)->getRowsCount(), fields, (yyvsp[(3) - (3)].lex).line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            TString vectorString = *(yyvsp[(3) - (3)].lex).string;
            TIntermTyped* index = ir_add_swizzle(fields, (yyvsp[(3) - (3)].lex).line);                
            (yyval.interm.intermTypedNode) = ir_add_index(EOpMatrixSwizzle, (yyvsp[(1) - (3)].interm.intermTypedNode), index, (yyvsp[(2) - (3)].lex).line);
            (yyval.interm.intermTypedNode)->setType(TType((yyvsp[(1) - (3)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (3)].interm.intermTypedNode)->getPrecision(), EvqTemporary, 1, fields.num));
                    
        } else if ((yyvsp[(1) - (3)].interm.intermTypedNode)->getBasicType() == EbtStruct) {
            bool fieldFound = false;
            TTypeList* fields = (yyvsp[(1) - (3)].interm.intermTypedNode)->getType().getStruct();
            if (fields == 0) {
                parseContext.error((yyvsp[(2) - (3)].lex).line, "structure has no fields", "Internal Error", "");
                parseContext.recover();
                (yyval.interm.intermTypedNode) = (yyvsp[(1) - (3)].interm.intermTypedNode);
            } else {
                unsigned int i;
                for (i = 0; i < fields->size(); ++i) {
                    if ((*fields)[i].type->getFieldName() == *(yyvsp[(3) - (3)].lex).string) {
                        fieldFound = true;
                        break;
                    }
                }
                if (fieldFound) {
					TIntermConstant* index = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[(3) - (3)].lex).line);
					index->setValue(i);
					(yyval.interm.intermTypedNode) = ir_add_index(EOpIndexDirectStruct, (yyvsp[(1) - (3)].interm.intermTypedNode), index, (yyvsp[(2) - (3)].lex).line);                
					(yyval.interm.intermTypedNode)->setType(*(*fields)[i].type);
                } else {
                    parseContext.error((yyvsp[(2) - (3)].lex).line, " no such field in structure", (yyvsp[(3) - (3)].lex).string->c_str(), "");
                    parseContext.recover();
                    (yyval.interm.intermTypedNode) = (yyvsp[(1) - (3)].interm.intermTypedNode);
                }
            }
        } else if ((yyvsp[(1) - (3)].interm.intermTypedNode)->isScalar()) {

            // HLSL allows ".xxxx" field selection on single component floats.  Handle that here.
            TVectorFields fields;

            // Check to make sure only the "x" component is accessed.
            if (! parseContext.parseVectorFields(*(yyvsp[(3) - (3)].lex).string, 1, fields, (yyvsp[(3) - (3)].lex).line))
			{
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
				(yyval.interm.intermTypedNode) = (yyvsp[(1) - (3)].interm.intermTypedNode);
            }
			else
			{
				// Create the appropriate constructor based on the number of ".x"'s there are in the selection field
				TString vectorString = *(yyvsp[(3) - (3)].lex).string;
				TQualifier qualifier = (yyvsp[(1) - (3)].interm.intermTypedNode)->getType().getQualifier() == EvqConst ? EvqConst : EvqTemporary;
				TType type((yyvsp[(1) - (3)].interm.intermTypedNode)->getBasicType(), (yyvsp[(1) - (3)].interm.intermTypedNode)->getPrecision(), qualifier, 1, (int) vectorString.size());
				(yyval.interm.intermTypedNode) = parseContext.constructBuiltIn(&type, parseContext.getConstructorOp(type),
												   (yyval.interm.intermTypedNode), (yyvsp[(1) - (3)].interm.intermTypedNode)->getLine(), false);
			}
        } else {
            parseContext.error((yyvsp[(2) - (3)].lex).line, " field selection requires structure, vector, or matrix on left hand side", (yyvsp[(3) - (3)].lex).string->c_str(), "");
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(1) - (3)].interm.intermTypedNode);
        }
        // don't delete $3.string, it's from the pool
    ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 395 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.lValueErrorCheck((yyvsp[(2) - (2)].lex).line, "++", (yyvsp[(1) - (2)].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPostIncrement, (yyvsp[(1) - (2)].interm.intermTypedNode), (yyvsp[(2) - (2)].lex).line, parseContext);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.unaryOpError((yyvsp[(2) - (2)].lex).line, "++", (yyvsp[(1) - (2)].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(1) - (2)].interm.intermTypedNode);
        }
    ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 405 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.lValueErrorCheck((yyvsp[(2) - (2)].lex).line, "--", (yyvsp[(1) - (2)].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPostDecrement, (yyvsp[(1) - (2)].interm.intermTypedNode), (yyvsp[(2) - (2)].lex).line, parseContext);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.unaryOpError((yyvsp[(2) - (2)].lex).line, "--", (yyvsp[(1) - (2)].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(1) - (2)].interm.intermTypedNode);
        }
    ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 418 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.scalarErrorCheck((yyvsp[(1) - (1)].interm.intermTypedNode), "[]"))
            parseContext.recover();
        TType type(EbtInt, EbpUndefined);
        (yyval.interm.intermTypedNode) = parseContext.constructBuiltIn(&type, EOpConstructInt, (yyvsp[(1) - (1)].interm.intermTypedNode), (yyvsp[(1) - (1)].interm.intermTypedNode)->getLine(), true);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.error((yyvsp[(1) - (1)].interm.intermTypedNode)->getLine(), "cannot convert to index", "[]", "");
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
        }
    ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 432 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TFunction* fnCall = (yyvsp[(1) - (1)].interm).function;
        TOperator op = fnCall->getBuiltInOp();

        if (op == EOpArrayLength) {
            if ((yyvsp[(1) - (1)].interm).intermNode->getAsTyped() == 0 || (yyvsp[(1) - (1)].interm).intermNode->getAsTyped()->getType().getArraySize() == 0) {
                parseContext.error((yyvsp[(1) - (1)].interm).line, "", fnCall->getName().c_str(), "array must be declared with a size before using this method");
                parseContext.recover();
            }

			TIntermConstant* constant = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), (yyvsp[(1) - (1)].interm).line);
			constant->setValue((yyvsp[(1) - (1)].interm).intermNode->getAsTyped()->getType().getArraySize());
            (yyval.interm.intermTypedNode) = constant;
        } else if (op != EOpNull) {
            //
            // Then this should be a constructor.
            // Don't go through the symbol table for constructors.
            // Their parameters will be verified algorithmically.
            //
            TType type(EbtVoid, EbpUndefined);  // use this to get the type back
            if (parseContext.constructorErrorCheck((yyvsp[(1) - (1)].interm).line, (yyvsp[(1) - (1)].interm).intermNode, *fnCall, op, &type)) {
                (yyval.interm.intermTypedNode) = 0;
            } else {
                //
                // It's a constructor, of type 'type'.
                //
                (yyval.interm.intermTypedNode) = parseContext.addConstructor((yyvsp[(1) - (1)].interm).intermNode, &type, op, fnCall, (yyvsp[(1) - (1)].interm).line);
            }

            if ((yyval.interm.intermTypedNode) == 0) {
                parseContext.recover();
                (yyval.interm.intermTypedNode) = ir_set_aggregate_op(0, op, (yyvsp[(1) - (1)].interm).line);
				(yyval.interm.intermTypedNode)->setType(type);
            }
        } else {
            //
            // Not a constructor.  Find it in the symbol table.
            //
            const TFunction* fnCandidate;
            bool builtIn;
            fnCandidate = parseContext.findFunction((yyvsp[(1) - (1)].interm).line, fnCall, &builtIn);

            if ( fnCandidate && fnCandidate->getMangledName() != fnCall->getMangledName()) {
                //add constructors to arguments to ensure that they have proper types
                TIntermNode *temp = parseContext.promoteFunctionArguments( (yyvsp[(1) - (1)].interm).intermNode,
                                      fnCandidate);
                if (temp)
                    (yyvsp[(1) - (1)].interm).intermNode = temp;
                else {
                    parseContext.error( (yyvsp[(1) - (1)].interm).intermNode->getLine(), " unable to suitably promote arguments to function",
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
                        (yyval.interm.intermTypedNode) = ir_add_unary_math(op, (yyvsp[(1) - (1)].interm).intermNode, gNullSourceLoc, parseContext);
                        if ((yyval.interm.intermTypedNode) == 0)  {
                            parseContext.error((yyvsp[(1) - (1)].interm).intermNode->getLine(), " wrong operand type", "Internal Error",
                                "built in unary operator function.  Type: %s",
                                static_cast<TIntermTyped*>((yyvsp[(1) - (1)].interm).intermNode)->getCompleteString().c_str());
                            YYERROR;
                        }
                    } else {
                        (yyval.interm.intermTypedNode) = ir_set_aggregate_op((yyvsp[(1) - (1)].interm).intermAggregate, op, (yyvsp[(1) - (1)].interm).line);
						(yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());
                    }
                } else {
                    // This is a real function call
                    
                    (yyval.interm.intermTypedNode) = ir_set_aggregate_op((yyvsp[(1) - (1)].interm).intermAggregate, EOpFunctionCall, (yyvsp[(1) - (1)].interm).line);
                    (yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());                   
                    
                    (yyval.interm.intermTypedNode)->getAsAggregate()->setName(fnCandidate->getMangledName());
                    (yyval.interm.intermTypedNode)->getAsAggregate()->setPlainName(fnCandidate->getName());

                    TQualifier qual;
                    for (int i = 0; i < fnCandidate->getParamCount(); ++i) {
                        qual = (*fnCandidate)[i].type->getQualifier();
                        if (qual == EvqOut || qual == EvqInOut) {
                            if (parseContext.lValueErrorCheck((yyval.interm.intermTypedNode)->getLine(), "assign", (yyval.interm.intermTypedNode)->getAsAggregate()->getNodes()[i]->getAsTyped())) {
                                parseContext.error((yyvsp[(1) - (1)].interm).intermNode->getLine(), "Constant value cannot be passed for 'out' or 'inout' parameters.", "Error", "");
                                parseContext.recover();
                            }
                        }
                    }
                }
                (yyval.interm.intermTypedNode)->setType(fnCandidate->getReturnType());
            } else {
                // error message was put out by PaFindFunction()
                // Put on a dummy node for error recovery
                
				TIntermConstant* constant = ir_add_constant(TType(EbtFloat, EbpUndefined, EvqConst), (yyvsp[(1) - (1)].interm).line);
				constant->setValue(0.f);
				(yyval.interm.intermTypedNode) = constant;
                parseContext.recover();
            }
        }
        delete fnCall;
    ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 548 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(1) - (1)].interm);
    ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 551 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(1) - (3)].interm.intermTypedNode)->isArray() && (yyvsp[(3) - (3)].interm).function->getName() == "length") {
            //
            // implement array.length()
            //
            (yyval.interm) = (yyvsp[(3) - (3)].interm);
            (yyval.interm).intermNode = (yyvsp[(1) - (3)].interm.intermTypedNode);
            (yyval.interm).function->relateToOperator(EOpArrayLength);

        } else {
            parseContext.error((yyvsp[(3) - (3)].interm).line, "methods are not supported", "", "");
            parseContext.recover();
            (yyval.interm) = (yyvsp[(3) - (3)].interm);
        }
    ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 569 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(1) - (2)].interm);
        (yyval.interm).line = (yyvsp[(2) - (2)].lex).line;
    ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 573 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(1) - (2)].interm);
        (yyval.interm).line = (yyvsp[(2) - (2)].lex).line;
    ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 580 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm).function = (yyvsp[(1) - (2)].interm.function);
        (yyval.interm).intermNode = 0;
    ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 584 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm).function = (yyvsp[(1) - (1)].interm.function);
        (yyval.interm).intermNode = 0;
    ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 591 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (!(yyvsp[(2) - (2)].interm.intermTypedNode)) {
          YYERROR;
		}
		TParameter param = { 0, 0, new TType((yyvsp[(2) - (2)].interm.intermTypedNode)->getType()) };
        (yyvsp[(1) - (2)].interm.function)->addParameter(param);
        (yyval.interm).function = (yyvsp[(1) - (2)].interm.function);
        (yyval.interm).intermNode = (yyvsp[(2) - (2)].interm.intermTypedNode);
    ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 600 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (!(yyvsp[(3) - (3)].interm.intermTypedNode)) {
          YYERROR;
		}
        TParameter param = { 0, 0, new TType((yyvsp[(3) - (3)].interm.intermTypedNode)->getType()) };
        (yyvsp[(1) - (3)].interm).function->addParameter(param);
        (yyval.interm).function = (yyvsp[(1) - (3)].interm).function;
        (yyval.interm).intermNode = ir_grow_aggregate((yyvsp[(1) - (3)].interm).intermNode, (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line);
    ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 612 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.function) = (yyvsp[(1) - (2)].interm.function);
    ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 620 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //
        // Constructor
        //
        if ((yyvsp[(1) - (1)].interm.type).array) {
            //TODO : figure out how to deal with array constructors
        }

        if ((yyvsp[(1) - (1)].interm.type).userDef) {
            TString tempString = "";
            TType type((yyvsp[(1) - (1)].interm.type));
            TFunction *function = new TFunction(&tempString, type, EOpConstructStruct);
            (yyval.interm.function) = function;
        } else {
            TOperator op = ir_get_constructor_op((yyvsp[(1) - (1)].interm.type), parseContext, false);
            if (op == EOpNull) {
                parseContext.error((yyvsp[(1) - (1)].interm.type).line, "cannot construct this type", TType::getBasicString((yyvsp[(1) - (1)].interm.type).type), "");
                parseContext.recover();
                (yyvsp[(1) - (1)].interm.type).type = EbtFloat;
                op = EOpConstructFloat;
            }
            TString tempString = "";
            TType type((yyvsp[(1) - (1)].interm.type));
            TFunction *function = new TFunction(&tempString, type, op);
            (yyval.interm.function) = function;
        }
    ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 647 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (parseContext.reservedErrorCheck((yyvsp[(1) - (1)].lex).line, *(yyvsp[(1) - (1)].lex).string)) 
			parseContext.recover();
		TType type(EbtVoid, EbpUndefined);
		const TString *mangled;
		if ( *(yyvsp[(1) - (1)].lex).string == "main")
			mangled = NewPoolTString("xlat_main");
		else
			mangled = (yyvsp[(1) - (1)].lex).string;
		TFunction *function = new TFunction( mangled, type);
		(yyval.interm.function) = function;
	;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 659 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (parseContext.reservedErrorCheck((yyvsp[(1) - (1)].lex).line, *(yyvsp[(1) - (1)].lex).string)) 
			parseContext.recover();
		TType type(EbtVoid, EbpUndefined);
		TFunction *function = new TFunction((yyvsp[(1) - (1)].lex).string, type);
		(yyval.interm.function) = function;
    ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 669 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 672 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (parseContext.lValueErrorCheck((yyvsp[(1) - (2)].lex).line, "++", (yyvsp[(2) - (2)].interm.intermTypedNode)))
			parseContext.recover();
		(yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPreIncrement, (yyvsp[(2) - (2)].interm.intermTypedNode), (yyvsp[(1) - (2)].lex).line, parseContext);
		if ((yyval.interm.intermTypedNode) == 0) {
			parseContext.unaryOpError((yyvsp[(1) - (2)].lex).line, "++", (yyvsp[(2) - (2)].interm.intermTypedNode)->getCompleteString());
			parseContext.recover();
			(yyval.interm.intermTypedNode) = (yyvsp[(2) - (2)].interm.intermTypedNode);
		}
    ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 682 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.lValueErrorCheck((yyvsp[(1) - (2)].lex).line, "--", (yyvsp[(2) - (2)].interm.intermTypedNode)))
            parseContext.recover();
		(yyval.interm.intermTypedNode) = ir_add_unary_math(EOpPreDecrement, (yyvsp[(2) - (2)].interm.intermTypedNode), (yyvsp[(1) - (2)].lex).line, parseContext);
		if ((yyval.interm.intermTypedNode) == 0) {
			parseContext.unaryOpError((yyvsp[(1) - (2)].lex).line, "--", (yyvsp[(2) - (2)].interm.intermTypedNode)->getCompleteString());
			parseContext.recover();
			(yyval.interm.intermTypedNode) = (yyvsp[(2) - (2)].interm.intermTypedNode);
		}
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 692 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if ((yyvsp[(1) - (2)].interm).op != EOpNull) {
			(yyval.interm.intermTypedNode) = ir_add_unary_math((yyvsp[(1) - (2)].interm).op, (yyvsp[(2) - (2)].interm.intermTypedNode), (yyvsp[(1) - (2)].interm).line, parseContext);
			if ((yyval.interm.intermTypedNode) == 0) {
				const char* errorOp = "";
				switch((yyvsp[(1) - (2)].interm).op) {
					case EOpNegative:   errorOp = "-"; break;
					case EOpLogicalNot: errorOp = "!"; break;
					case EOpBitwiseNot: errorOp = "~"; break;
					default: break;
				}
				parseContext.unaryOpError((yyvsp[(1) - (2)].interm).line, errorOp, (yyvsp[(2) - (2)].interm.intermTypedNode)->getCompleteString());
				parseContext.recover();
				(yyval.interm.intermTypedNode) = (yyvsp[(2) - (2)].interm.intermTypedNode);
			}
		} else
			(yyval.interm.intermTypedNode) = (yyvsp[(2) - (2)].interm.intermTypedNode);
    ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 710 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // cast operator, insert constructor
        TOperator op = ir_get_constructor_op((yyvsp[(2) - (4)].interm.type), parseContext, true);
        if (op == EOpNull) {
            parseContext.error((yyvsp[(2) - (4)].interm.type).line, "cannot cast this type", TType::getBasicString((yyvsp[(2) - (4)].interm.type).type), "");
            parseContext.recover();
            (yyvsp[(2) - (4)].interm.type).type = EbtFloat;
            op = EOpConstructFloat;
        }
        TString tempString = "";
        TType type((yyvsp[(2) - (4)].interm.type));
        TFunction *function = new TFunction(&tempString, type, op);
        TParameter param = { 0, 0, new TType((yyvsp[(4) - (4)].interm.intermTypedNode)->getType()) };
        function->addParameter(param);
        TType type2(EbtVoid, EbpUndefined);  // use this to get the type back
        if (parseContext.constructorErrorCheck((yyvsp[(2) - (4)].interm.type).line, (yyvsp[(4) - (4)].interm.intermTypedNode), *function, op, &type2)) {
            (yyval.interm.intermTypedNode) = 0;
        } else {
            //
            // It's a constructor, of type 'type'.
            //
            (yyval.interm.intermTypedNode) = parseContext.addConstructor((yyvsp[(4) - (4)].interm.intermTypedNode), &type2, op, function, (yyvsp[(2) - (4)].interm.type).line);
        }

        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.recover();
            (yyval.interm.intermTypedNode) = ir_set_aggregate_op(0, op, (yyvsp[(2) - (4)].interm.type).line);
        } else {
			(yyval.interm.intermTypedNode)->setType(type2);
		}
	;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 745 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpNull; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 746 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpNegative; ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 747 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpLogicalNot; ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 748 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("~", (yyvsp[(1) - (1)].lex).line);
              (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpBitwiseNot; ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 754 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 755 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpMul, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "*", false); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 756 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpDiv, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "/", false); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 757 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpMod, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "%", false); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 761 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 762 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpAdd, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "+", false); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 763 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpSub, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "-", false); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 767 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 768 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLeftShift, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "<<", false); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 769 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpRightShift, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, ">>", false); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 773 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 774 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLessThan, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "<", true); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 775 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpGreaterThan, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, ">", true); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 776 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLessThanEqual, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "<=", true); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 777 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpGreaterThanEqual, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, ">=", true); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 781 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 782 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpEqual, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "==", true); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 783 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpNotEqual, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "!=", true); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 787 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 788 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpAnd, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "&", false); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 792 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 793 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpExclusiveOr, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "^", false); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 797 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 798 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpInclusiveOr, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "|", false); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 802 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 803 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalAnd, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "&&", true); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 807 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 808 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalXor, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "^^", true); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 812 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 813 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = parseContext.add_binary(EOpLogicalOr, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line, "||", true); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 817 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 818 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
       if (parseContext.boolOrVectorErrorCheck((yyvsp[(2) - (5)].lex).line, (yyvsp[(1) - (5)].interm.intermTypedNode)))
            parseContext.recover();
       
		(yyval.interm.intermTypedNode) = ir_add_selection((yyvsp[(1) - (5)].interm.intermTypedNode), (yyvsp[(3) - (5)].interm.intermTypedNode), (yyvsp[(5) - (5)].interm.intermTypedNode), (yyvsp[(2) - (5)].lex).line, parseContext.infoSink);
           
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[(2) - (5)].lex).line, ":", (yyvsp[(3) - (5)].interm.intermTypedNode)->getCompleteString(), (yyvsp[(5) - (5)].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(5) - (5)].interm.intermTypedNode);
        }
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 833 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 834 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {        
        if (parseContext.lValueErrorCheck((yyvsp[(2) - (3)].interm).line, "assign", (yyvsp[(1) - (3)].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = parseContext.addAssign((yyvsp[(2) - (3)].interm).op, (yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].interm).line);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.assignError((yyvsp[(2) - (3)].interm).line, "assign", (yyvsp[(1) - (3)].interm.intermTypedNode)->getCompleteString(), (yyvsp[(3) - (3)].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(1) - (3)].interm.intermTypedNode);
        } else if (((yyvsp[(1) - (3)].interm.intermTypedNode)->isArray() || (yyvsp[(3) - (3)].interm.intermTypedNode)->isArray()))
            parseContext.recover();
    ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 848 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpAssign; ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 849 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpMulAssign; ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 850 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpDivAssign; ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 851 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("%=", (yyvsp[(1) - (1)].lex).line);  (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpModAssign; ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 852 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpAddAssign; ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 853 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpSubAssign; ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 854 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("<<=", (yyvsp[(1) - (1)].lex).line); (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpLeftShiftAssign; ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 855 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("<<=", (yyvsp[(1) - (1)].lex).line); (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpRightShiftAssign; ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 856 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("&=",  (yyvsp[(1) - (1)].lex).line); (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpAndAssign; ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 857 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("^=",  (yyvsp[(1) - (1)].lex).line); (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpExclusiveOrAssign; ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 858 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { UNSUPPORTED_FEATURE("|=",  (yyvsp[(1) - (1)].lex).line); (yyval.interm).line = (yyvsp[(1) - (1)].lex).line; (yyval.interm).op = EOpInclusiveOrAssign; ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 862 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 865 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = ir_add_comma((yyvsp[(1) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(2) - (3)].lex).line);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[(2) - (3)].lex).line, ",", (yyvsp[(1) - (3)].interm.intermTypedNode)->getCompleteString(), (yyvsp[(3) - (3)].interm.intermTypedNode)->getCompleteString());
            parseContext.recover();
            (yyval.interm.intermTypedNode) = (yyvsp[(3) - (3)].interm.intermTypedNode);
        }
    ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 876 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.constErrorCheck((yyvsp[(1) - (1)].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 884 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermDeclaration) = 0; ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 885 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermDeclaration) = (yyvsp[(1) - (2)].interm.intermDeclaration); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 889 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find((yyvsp[(1) - (2)].interm.function)->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != (yyvsp[(1) - (2)].interm.function)->getReturnType()) {
                parseContext.error((yyvsp[(2) - (2)].lex).line, "overloaded functions must have the same return type", (yyvsp[(1) - (2)].interm.function)->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*(yyvsp[(1) - (2)].interm.function))[i].type->getQualifier()) {
                    parseContext.error((yyvsp[(2) - (2)].lex).line, "overloaded functions must have the same parameter qualifiers", (*(yyvsp[(1) - (2)].interm.function))[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }

        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        (yyval.interm).function = (yyvsp[(1) - (2)].interm.function);
        (yyval.interm).line = (yyvsp[(2) - (2)].lex).line;

        parseContext.symbolTable.insert(*(yyval.interm).function);
    ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 922 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find((yyvsp[(1) - (4)].interm.function)->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != (yyvsp[(1) - (4)].interm.function)->getReturnType()) {
                parseContext.error((yyvsp[(2) - (4)].lex).line, "overloaded functions must have the same return type", (yyvsp[(1) - (4)].interm.function)->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*(yyvsp[(1) - (4)].interm.function))[i].type->getQualifier()) {
                    parseContext.error((yyvsp[(2) - (4)].lex).line, "overloaded functions must have the same parameter qualifiers", (*(yyvsp[(1) - (4)].interm.function))[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }

        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        (yyval.interm).function = (yyvsp[(1) - (4)].interm.function);
        (yyval.interm).line = (yyvsp[(2) - (4)].lex).line;
        (yyval.interm).function->setInfo(new TTypeInfo(*(yyvsp[(4) - (4)].lex).string, 0));

        parseContext.symbolTable.insert(*(yyval.interm).function);
    ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 959 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.function) = (yyvsp[(1) - (1)].interm.function);
    ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 962 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.function) = (yyvsp[(1) - (1)].interm.function);
    ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 969 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // Add the parameter
        (yyval.interm.function) = (yyvsp[(1) - (2)].interm.function);
        if ((yyvsp[(2) - (2)].interm).param.type->getBasicType() != EbtVoid)
            (yyvsp[(1) - (2)].interm.function)->addParameter((yyvsp[(2) - (2)].interm).param);
        else
            delete (yyvsp[(2) - (2)].interm).param.type;
    ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 977 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //
        // Only first parameter of one-parameter functions can be void
        // The check for named parameters not being void is done in parameter_declarator
        //
        if ((yyvsp[(3) - (3)].interm).param.type->getBasicType() == EbtVoid) {
            //
            // This parameter > first is void
            //
            parseContext.error((yyvsp[(2) - (3)].lex).line, "cannot be an argument type except for '(void)'", "void", "");
            parseContext.recover();
            delete (yyvsp[(3) - (3)].interm).param.type;
        } else {
            // Add the parameter
            (yyval.interm.function) = (yyvsp[(1) - (3)].interm.function);
            (yyvsp[(1) - (3)].interm.function)->addParameter((yyvsp[(3) - (3)].interm).param);
        }
    ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 998 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(1) - (3)].interm.type).qualifier != EvqGlobal && (yyvsp[(1) - (3)].interm.type).qualifier != EvqTemporary) {
			if ((yyvsp[(1) - (3)].interm.type).qualifier == EvqConst || (yyvsp[(1) - (3)].interm.type).qualifier == EvqStatic)
			{
				(yyvsp[(1) - (3)].interm.type).qualifier = EvqTemporary;
			}
			else
			{
				parseContext.error((yyvsp[(2) - (3)].lex).line, "no qualifiers allowed for function return", getQualifierString((yyvsp[(1) - (3)].interm.type).qualifier), "");
				parseContext.recover();
			}
        }
        // make sure a sampler is not involved as well...
        if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (3)].lex).line, (yyvsp[(1) - (3)].interm.type)))
            parseContext.recover();

        // Add the function as a prototype after parsing it (we do not support recursion)
        TFunction *function;
        TType type((yyvsp[(1) - (3)].interm.type));
    const TString* mangled = 0;
    if ( *(yyvsp[(2) - (3)].lex).string == "main")
        mangled = NewPoolTString( "xlat_main");
    else
        mangled = (yyvsp[(2) - (3)].lex).string;

        function = new TFunction(mangled, type);
        (yyval.interm.function) = function;
    ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1030 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(1) - (2)].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[(2) - (2)].lex).line, "illegal use of type 'void'", (yyvsp[(2) - (2)].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[(2) - (2)].lex).line, *(yyvsp[(2) - (2)].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[(2) - (2)].lex).string, 0, new TType((yyvsp[(1) - (2)].interm.type))};
        (yyval.interm).line = (yyvsp[(2) - (2)].lex).line;
        (yyval.interm).param = param;
    ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1041 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(1) - (4)].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[(2) - (4)].lex).line, "illegal use of type 'void'", (yyvsp[(2) - (4)].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[(2) - (4)].lex).line, *(yyvsp[(2) - (4)].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[(2) - (4)].lex).string, 0, new TType((yyvsp[(1) - (4)].interm.type))};
        (yyval.interm).line = (yyvsp[(2) - (4)].lex).line;
        (yyval.interm).param = param;

        //TODO: add initializer support
    ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1054 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // Parameter with register
        if ((yyvsp[(1) - (3)].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[(2) - (3)].lex).line, "illegal use of type 'void'", (yyvsp[(2) - (3)].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[(2) - (3)].lex).line, *(yyvsp[(2) - (3)].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[(2) - (3)].lex).string, new TTypeInfo("", *(yyvsp[(3) - (3)].lex).string, 0), new TType((yyvsp[(1) - (3)].interm.type))};
        (yyval.interm).line = (yyvsp[(2) - (3)].lex).line;
        (yyval.interm).param = param; 
    ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1066 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //Parameter with semantic
        if ((yyvsp[(1) - (4)].interm.type).type == EbtVoid) {
            parseContext.error((yyvsp[(2) - (4)].lex).line, "illegal use of type 'void'", (yyvsp[(2) - (4)].lex).string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck((yyvsp[(2) - (4)].lex).line, *(yyvsp[(2) - (4)].lex).string))
            parseContext.recover();
        TParameter param = {(yyvsp[(2) - (4)].lex).string, new TTypeInfo(*(yyvsp[(4) - (4)].lex).string, 0), new TType((yyvsp[(1) - (4)].interm.type))};
        (yyval.interm).line = (yyvsp[(2) - (4)].lex).line;
        (yyval.interm).param = param;
    ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1078 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)))
            parseContext.recover();

        if (parseContext.reservedErrorCheck((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string))
            parseContext.recover();

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[(3) - (5)].lex).line, (yyvsp[(4) - (5)].interm.intermTypedNode), size))
            parseContext.recover();
        (yyvsp[(1) - (5)].interm.type).setArray(true, size);

        TType* type = new TType((yyvsp[(1) - (5)].interm.type));
        TParameter param = { (yyvsp[(2) - (5)].lex).string, 0, type };
        (yyval.interm).line = (yyvsp[(2) - (5)].lex).line;
        (yyval.interm).param = param;
    ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1096 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (7)].lex).line, (yyvsp[(1) - (7)].interm.type)))
            parseContext.recover();

        if (parseContext.reservedErrorCheck((yyvsp[(2) - (7)].lex).line, *(yyvsp[(2) - (7)].lex).string))
            parseContext.recover();

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[(3) - (7)].lex).line, (yyvsp[(4) - (7)].interm.intermTypedNode), size))
            parseContext.recover();
        (yyvsp[(1) - (7)].interm.type).setArray(true, size);

        TType* type = new TType((yyvsp[(1) - (7)].interm.type));
        TParameter param = { (yyvsp[(2) - (7)].lex).string, new TTypeInfo(*(yyvsp[(7) - (7)].lex).string, 0), type };
        (yyval.interm).line = (yyvsp[(2) - (7)].lex).line;
        (yyval.interm).param = param;
    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1125 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(3) - (3)].interm);
        if (parseContext.paramErrorCheck((yyvsp[(3) - (3)].interm).line, (yyvsp[(1) - (3)].interm.type).qualifier, (yyvsp[(2) - (3)].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1130 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(2) - (2)].interm);
        if (parseContext.parameterSamplerErrorCheck((yyvsp[(2) - (2)].interm).line, (yyvsp[(1) - (2)].interm.qualifier), *(yyvsp[(2) - (2)].interm).param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck((yyvsp[(2) - (2)].interm).line, EvqTemporary, (yyvsp[(1) - (2)].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1140 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(3) - (3)].interm);
        if (parseContext.paramErrorCheck((yyvsp[(3) - (3)].interm).line, (yyvsp[(1) - (3)].interm.type).qualifier, (yyvsp[(2) - (3)].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1145 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm) = (yyvsp[(2) - (2)].interm);
        if (parseContext.parameterSamplerErrorCheck((yyvsp[(2) - (2)].interm).line, (yyvsp[(1) - (2)].interm.qualifier), *(yyvsp[(2) - (2)].interm).param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck((yyvsp[(2) - (2)].interm).line, EvqTemporary, (yyvsp[(1) - (2)].interm.qualifier), (yyval.interm).param.type))
            parseContext.recover();
    ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1155 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.qualifier) = EvqIn;
    ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1158 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.qualifier) = EvqIn;
    ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1161 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.qualifier) = EvqOut;
    ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1164 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.qualifier) = EvqInOut;
    ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1170 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TParameter param = { 0, 0, new TType((yyvsp[(1) - (1)].interm.type)) };
        (yyval.interm).param = param;
    ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1177 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermDeclaration) = (yyvsp[(1) - (1)].interm.intermDeclaration);
    ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1180 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (4)].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (4)].lex).line, type))
            parseContext.recover();
        
        if (parseContext.nonInitConstErrorCheck((yyvsp[(3) - (4)].lex).line, *(yyvsp[(3) - (4)].lex).string, type))
            parseContext.recover();

        if (parseContext.nonInitErrorCheck((yyvsp[(3) - (4)].lex).line, *(yyvsp[(3) - (4)].lex).string, (yyvsp[(4) - (4)].interm.typeInfo), type))
            parseContext.recover();
		
		TSymbol* sym = parseContext.symbolTable.find(*(yyvsp[(3) - (4)].lex).string);
		if (!sym)
			(yyval.interm.intermDeclaration) = (yyvsp[(1) - (4)].interm.intermDeclaration);
		else
			(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (4)].interm.intermDeclaration), sym, NULL, parseContext);
    ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1198 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (6)].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (6)].lex).line, type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck((yyvsp[(3) - (6)].lex).line, *(yyvsp[(3) - (6)].lex).string, type))
            parseContext.recover();
        
        if (parseContext.arrayTypeErrorCheck((yyvsp[(4) - (6)].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[(4) - (6)].lex).line, type))
            parseContext.recover();
        else {
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[(4) - (6)].lex).line, *(yyvsp[(3) - (6)].lex).string, (yyvsp[(6) - (6)].interm.typeInfo), type, variable))
                parseContext.recover();
		
			if (!variable)
				(yyval.interm.intermDeclaration) = (yyvsp[(1) - (6)].interm.intermDeclaration);
			else {
				variable->getType().setArray(true);
				(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (6)].interm.intermDeclaration), variable, NULL, parseContext);
			}
        }
    ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1222 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (7)].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (7)].lex).line, type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck((yyvsp[(3) - (7)].lex).line, *(yyvsp[(3) - (7)].lex).string, type))
            parseContext.recover();

        if (parseContext.arrayTypeErrorCheck((yyvsp[(4) - (7)].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[(4) - (7)].lex).line, type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[(4) - (7)].lex).line, (yyvsp[(5) - (7)].interm.intermTypedNode), size))
                parseContext.recover();
            type.setArray(true, size);
			
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[(4) - (7)].lex).line, *(yyvsp[(3) - (7)].lex).string, (yyvsp[(7) - (7)].interm.typeInfo), type, variable))
                parseContext.recover();
			
			if (!variable)
				(yyval.interm.intermDeclaration) = (yyvsp[(1) - (7)].interm.intermDeclaration);
			else {
				(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (7)].interm.intermDeclaration), variable, NULL, parseContext);
			}
        }
    ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1250 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (8)].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (8)].lex).line, type))
            parseContext.recover();
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[(4) - (8)].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[(4) - (8)].lex).line, type))
            parseContext.recover();
        else if (parseContext.arrayErrorCheck((yyvsp[(4) - (8)].lex).line, *(yyvsp[(3) - (8)].lex).string, type, variable))
			parseContext.recover();
		
        {
            TIntermSymbol* symbol;
            type.setArray(true, (yyvsp[(8) - (8)].interm.intermTypedNode)->getType().getArraySize());
            if (!parseContext.executeInitializer((yyvsp[(3) - (8)].lex).line, *(yyvsp[(3) - (8)].lex).string, (yyvsp[(6) - (8)].interm.typeInfo), type, (yyvsp[(8) - (8)].interm.intermTypedNode), symbol, variable)) {
                if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[(1) - (8)].interm.intermDeclaration);
				else {
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (8)].interm.intermDeclaration), variable, (yyvsp[(8) - (8)].interm.intermTypedNode), parseContext);
				}
            } else {
                parseContext.recover();
                (yyval.interm.intermDeclaration) = 0;
            }
        }
    ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1277 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (9)].interm.intermDeclaration));
		int array_size;
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (9)].lex).line, type))
            parseContext.recover();
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[(4) - (9)].lex).line, type) || parseContext.arrayQualifierErrorCheck((yyvsp[(4) - (9)].lex).line, type))
            parseContext.recover();
        else {
            if (parseContext.arraySizeErrorCheck((yyvsp[(4) - (9)].lex).line, (yyvsp[(5) - (9)].interm.intermTypedNode), array_size))
                parseContext.recover();
			
            type.setArray(true, array_size);
            if (parseContext.arrayErrorCheck((yyvsp[(4) - (9)].lex).line, *(yyvsp[(3) - (9)].lex).string, (yyvsp[(7) - (9)].interm.typeInfo), type, variable))
                parseContext.recover();
        }

        {
            TIntermSymbol* symbol;
            if (!parseContext.executeInitializer((yyvsp[(3) - (9)].lex).line, *(yyvsp[(3) - (9)].lex).string, (yyvsp[(7) - (9)].interm.typeInfo), type, (yyvsp[(9) - (9)].interm.intermTypedNode), symbol, variable)) {
				if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[(1) - (9)].interm.intermDeclaration);
				else {
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (9)].interm.intermDeclaration), variable, (yyvsp[(9) - (9)].interm.intermTypedNode), parseContext);
				}
            } else {
                parseContext.recover();
                (yyval.interm.intermDeclaration) = 0;
            }
        }
    ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1310 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		TPublicType type = ir_get_decl_type_noarray((yyvsp[(1) - (6)].interm.intermDeclaration));
		
        if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (6)].lex).line, type))
            parseContext.recover();
			
        TIntermSymbol* symbol;
		if ( !IsSampler(type.type)) {
			if (!parseContext.executeInitializer((yyvsp[(3) - (6)].lex).line, *(yyvsp[(3) - (6)].lex).string, (yyvsp[(4) - (6)].interm.typeInfo), type, (yyvsp[(6) - (6)].interm.intermTypedNode), symbol)) {
				TSymbol* variable = parseContext.symbolTable.find(*(yyvsp[(3) - (6)].lex).string);
				if (!variable)
					(yyval.interm.intermDeclaration) = (yyvsp[(1) - (6)].interm.intermDeclaration);
				else 				
					(yyval.interm.intermDeclaration) = ir_grow_declaration((yyvsp[(1) - (6)].interm.intermDeclaration), variable, (yyvsp[(6) - (6)].interm.intermTypedNode), parseContext);
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		} else {
			//Special code to skip initializers for samplers
			(yyval.interm.intermDeclaration) = (yyvsp[(1) - (6)].interm.intermDeclaration);
			if (parseContext.structQualifierErrorCheck((yyvsp[(3) - (6)].lex).line, type))
				parseContext.recover();
			
			if (parseContext.nonInitConstErrorCheck((yyvsp[(3) - (6)].lex).line, *(yyvsp[(3) - (6)].lex).string, type))
				parseContext.recover();
			
			if (parseContext.nonInitErrorCheck((yyvsp[(3) - (6)].lex).line, *(yyvsp[(3) - (6)].lex).string, (yyvsp[(4) - (6)].interm.typeInfo), type))
				parseContext.recover();
		}
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1344 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermDeclaration) = 0;
    ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1347 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {				
		bool error = false;
        if (error &= parseContext.structQualifierErrorCheck((yyvsp[(2) - (3)].lex).line, (yyvsp[(1) - (3)].interm.type)))
            parseContext.recover();
        
        if (error &= parseContext.nonInitConstErrorCheck((yyvsp[(2) - (3)].lex).line, *(yyvsp[(2) - (3)].lex).string, (yyvsp[(1) - (3)].interm.type)))
            parseContext.recover();

        if (error &= parseContext.nonInitErrorCheck((yyvsp[(2) - (3)].lex).line, *(yyvsp[(2) - (3)].lex).string, (yyvsp[(3) - (3)].interm.typeInfo), (yyvsp[(1) - (3)].interm.type)))
            parseContext.recover();
		
		TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[(2) - (3)].lex).string);
		if (!error && symbol) {
			(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, NULL, (yyvsp[(2) - (3)].lex).line, parseContext);
		} else {
			(yyval.interm.intermDeclaration) = 0;
		}
    ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1365 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(1) - (5)].interm.type)))
            parseContext.recover();

        if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[(3) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)))
            parseContext.recover();
        else {
            (yyvsp[(1) - (5)].interm.type).setArray(true);
            TVariable* variable;
            if (parseContext.arrayErrorCheck((yyvsp[(3) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(5) - (5)].interm.typeInfo), (yyvsp[(1) - (5)].interm.type), variable))
                parseContext.recover();
        }
		
		TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[(2) - (5)].lex).string);
		if (symbol) {
			(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, NULL, (yyvsp[(2) - (5)].lex).line, parseContext);
		} else {
			(yyval.interm.intermDeclaration) = 0;
		}
    ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1388 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (6)].lex).line, (yyvsp[(1) - (6)].interm.type)))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck((yyvsp[(2) - (6)].lex).line, *(yyvsp[(2) - (6)].lex).string, (yyvsp[(1) - (6)].interm.type)))
			parseContext.recover();
		
		TVariable* variable;
        if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (6)].lex).line, (yyvsp[(1) - (6)].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[(3) - (6)].lex).line, (yyvsp[(1) - (6)].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[(3) - (6)].lex).line, (yyvsp[(4) - (6)].interm.intermTypedNode), size))
                parseContext.recover();

            (yyvsp[(1) - (6)].interm.type).setArray(true, size);
            if (parseContext.arrayErrorCheck((yyvsp[(3) - (6)].lex).line, *(yyvsp[(2) - (6)].lex).string, (yyvsp[(6) - (6)].interm.typeInfo), (yyvsp[(1) - (6)].interm.type), variable))
                parseContext.recover();
			
			if (variable) {
				(yyval.interm.intermDeclaration) = ir_add_declaration(variable, NULL, (yyvsp[(2) - (6)].lex).line, parseContext);
			} else {
				(yyval.interm.intermDeclaration) = 0;
			}
        }
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1414 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (7)].lex).line, (yyvsp[(1) - (7)].interm.type)))
			parseContext.recover();

		TVariable* variable = 0;
		if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (7)].lex).line, (yyvsp[(1) - (7)].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[(3) - (7)].lex).line, (yyvsp[(1) - (7)].interm.type)))
			parseContext.recover();
		else {
			(yyvsp[(1) - (7)].interm.type).setArray(true, (yyvsp[(7) - (7)].interm.intermTypedNode)->getType().getArraySize());
			if (parseContext.arrayErrorCheck((yyvsp[(3) - (7)].lex).line, *(yyvsp[(2) - (7)].lex).string, (yyvsp[(5) - (7)].interm.typeInfo), (yyvsp[(1) - (7)].interm.type), variable))
				parseContext.recover();
		}

		{        
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[(2) - (7)].lex).line, *(yyvsp[(2) - (7)].lex).string, (yyvsp[(5) - (7)].interm.typeInfo), (yyvsp[(1) - (7)].interm.type), (yyvsp[(7) - (7)].interm.intermTypedNode), symbol, variable)) {
				if (variable)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[(7) - (7)].interm.intermTypedNode), (yyvsp[(6) - (7)].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		}
    ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1440 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (8)].lex).line, (yyvsp[(1) - (8)].interm.type)))
            parseContext.recover();

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck((yyvsp[(3) - (8)].lex).line, (yyvsp[(1) - (8)].interm.type)) || parseContext.arrayQualifierErrorCheck((yyvsp[(3) - (8)].lex).line, (yyvsp[(1) - (8)].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[(3) - (8)].lex).line, (yyvsp[(4) - (8)].interm.intermTypedNode), size))
                parseContext.recover();

            (yyvsp[(1) - (8)].interm.type).setArray(true, size);
            if (parseContext.arrayErrorCheck((yyvsp[(3) - (8)].lex).line, *(yyvsp[(2) - (8)].lex).string, (yyvsp[(6) - (8)].interm.typeInfo), (yyvsp[(1) - (8)].interm.type), variable))
                parseContext.recover();
        }
        
		{        
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[(2) - (8)].lex).line, *(yyvsp[(2) - (8)].lex).string, (yyvsp[(6) - (8)].interm.typeInfo), (yyvsp[(1) - (8)].interm.type), (yyvsp[(8) - (8)].interm.intermTypedNode), symbol, variable)) {
				if (variable)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[(8) - (8)].interm.intermTypedNode), (yyvsp[(7) - (8)].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		}       
    ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1470 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)))
			parseContext.recover();
		
		if (!IsSampler((yyvsp[(1) - (5)].interm.type).type)) {
			TIntermSymbol* symbol;
			if (!parseContext.executeInitializer((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(3) - (5)].interm.typeInfo), (yyvsp[(1) - (5)].interm.type), (yyvsp[(5) - (5)].interm.intermTypedNode), symbol)) {
				if (symbol)
					(yyval.interm.intermDeclaration) = ir_add_declaration(symbol, (yyvsp[(5) - (5)].interm.intermTypedNode), (yyvsp[(4) - (5)].lex).line, parseContext);
				else
					(yyval.interm.intermDeclaration) = 0;
			} else {
				parseContext.recover();
				(yyval.interm.intermDeclaration) = 0;
			}
		} else {
			if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (5)].lex).line, (yyvsp[(1) - (5)].interm.type)))
				parseContext.recover();

			if (parseContext.nonInitConstErrorCheck((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(1) - (5)].interm.type)))
				parseContext.recover();

			if (parseContext.nonInitErrorCheck((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(3) - (5)].interm.typeInfo), (yyvsp[(1) - (5)].interm.type)))
				parseContext.recover();
				
			
			TSymbol* symbol = parseContext.symbolTable.find(*(yyvsp[(2) - (5)].lex).string);
			if (symbol)
			{
				TIntermSymbol* intermSymbol;
				TVariable* var = static_cast<TVariable*>(symbol);
				if (!parseContext.executeInitializer((yyvsp[(2) - (5)].lex).line, *(yyvsp[(2) - (5)].lex).string, (yyvsp[(3) - (5)].interm.typeInfo), (yyvsp[(1) - (5)].interm.type), (yyvsp[(5) - (5)].interm.intermTypedNode), intermSymbol,var)) {
					if (intermSymbol)
						(yyval.interm.intermDeclaration) = ir_add_declaration(intermSymbol,NULL, (yyvsp[(4) - (5)].lex).line, parseContext);
					else
						(yyval.interm.intermDeclaration) = 0;
				} else {
					parseContext.recover();
					(yyval.interm.intermDeclaration) = 0;
				}			
			}else{
				(yyval.interm.intermDeclaration) = 0;
			}

			
		}
    ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1532 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type) = (yyvsp[(1) - (1)].interm.type);
    ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1535 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(2) - (2)].interm.type).array && parseContext.arrayQualifierErrorCheck((yyvsp[(2) - (2)].interm.type).line, (yyvsp[(1) - (2)].interm.type))) {
            parseContext.recover();
            (yyvsp[(2) - (2)].interm.type).setArray(false);
        }

        if ((yyvsp[(1) - (2)].interm.type).qualifier == EvqAttribute &&
            ((yyvsp[(2) - (2)].interm.type).type == EbtBool || (yyvsp[(2) - (2)].interm.type).type == EbtInt)) {
            parseContext.error((yyvsp[(2) - (2)].interm.type).line, "cannot be bool or int", getQualifierString((yyvsp[(1) - (2)].interm.type).qualifier), "");
            parseContext.recover();
        }
        (yyval.interm.type) = (yyvsp[(2) - (2)].interm.type); 
        (yyval.interm.type).qualifier = (yyvsp[(1) - (2)].interm.type).qualifier;
    ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1552 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[(1) - (1)].lex).line);
    ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1555 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqStatic, (yyvsp[(1) - (1)].lex).line);
    ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1558 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[(1) - (2)].lex).line); // same as "const" really
    ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1561 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type).setBasic(EbtVoid, EvqConst, (yyvsp[(1) - (2)].lex).line); // same as "const" really
    ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1564 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.globalErrorCheck((yyvsp[(1) - (1)].lex).line, parseContext.symbolTable.atGlobalLevel(), "uniform"))
            parseContext.recover();
        (yyval.interm.type).setBasic(EbtVoid, EvqUniform, (yyvsp[(1) - (1)].lex).line);
    ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1572 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type) = (yyvsp[(1) - (1)].interm.type);
    ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1575 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type) = (yyvsp[(1) - (4)].interm.type);

        if (parseContext.arrayTypeErrorCheck((yyvsp[(2) - (4)].lex).line, (yyvsp[(1) - (4)].interm.type)))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck((yyvsp[(2) - (4)].lex).line, (yyvsp[(3) - (4)].interm.intermTypedNode), size))
                parseContext.recover();
            (yyval.interm.type).setArray(true, size);
        }
    ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1590 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtVoid,EbpUndefined);
    ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1593 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
    ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1596 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
    ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1599 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
    ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1602 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtInt,EbpHigh);
    ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1605 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtBool,EbpHigh);
    ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1608 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[(5) - (6)].lex).i > 4 || (yyvsp[(5) - (6)].lex).i < 1 ) {
            parseContext.error((yyvsp[(2) - (6)].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtFloat, qual, (yyvsp[(1) - (6)].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtFloat, qual, (yyvsp[(1) - (6)].lex).line);
            (yyval.interm.type).setVector((yyvsp[(5) - (6)].lex).i);
        }
    ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1619 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[(5) - (6)].lex).i > 4 || (yyvsp[(5) - (6)].lex).i < 1 ) {
            parseContext.error((yyvsp[(2) - (6)].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtInt, qual, (yyvsp[(1) - (6)].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtInt, qual, (yyvsp[(1) - (6)].lex).line);
            (yyval.interm.type).setVector((yyvsp[(5) - (6)].lex).i);
        }
    ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1630 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( (yyvsp[(5) - (6)].lex).i > 4 || (yyvsp[(5) - (6)].lex).i < 1 ) {
            parseContext.error((yyvsp[(2) - (6)].lex).line, "vector dimension out of range", "", "");
            parseContext.recover();
            (yyval.interm.type).setBasic(EbtBool, qual, (yyvsp[(1) - (6)].lex).line);
        } else {
            (yyval.interm.type).setBasic(EbtBool, qual, (yyvsp[(1) - (6)].lex).line);
            (yyval.interm.type).setVector((yyvsp[(5) - (6)].lex).i);
        }
    ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1641 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(2);
    ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1645 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(3);
    ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1649 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setVector(4);
    ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1653 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(2);
    ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1657 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(3);
    ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1661 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setVector(4);
    ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1665 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(2);
    ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1669 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(3);
    ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1673 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setVector(4);
    ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1677 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(2);
    ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1681 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(3);
    ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1685 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtBool,EbpHigh);
        (yyval.interm.type).setVector(4);
    ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1689 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(2);
    ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1693 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(3);
    ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1697 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtInt,EbpHigh);
        (yyval.interm.type).setVector(4);
    ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1701 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 2);
    ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1705 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float2x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 2);
    ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1710 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float2x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 2);
    ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1715 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float3x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 3);
    ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1720 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 3);
    ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1724 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float3x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 3);
    ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1729 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float4x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(2, 4);
    ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1734 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("float4x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(3, 4);
    ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1739 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpHigh);
        (yyval.interm.type).setMatrix(4, 4);
    ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1743 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 2);
    ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1747 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half2x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 2);
    ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1752 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half2x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 2);
    ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1757 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half3x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 3);
    ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1762 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 3);
    ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1766 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half3x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 3);
    ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1771 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half4x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(2, 4);
    ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1776 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("half4x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(3, 4);
    ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1781 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpMedium);
        (yyval.interm.type).setMatrix(4, 4);
    ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1785 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 2);
    ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1789 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed2x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 2);
    ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1794 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed2x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 2);
    ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1799 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed3x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 3);
    ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1804 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 3);
    ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1808 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed3x4", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 3);
    ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1813 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed4x2", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(2, 4);
    ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1818 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		NONSQUARE_MATRIX_CHECK("fixed4x3", (yyvsp[(1) - (1)].lex).line);
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(3, 4);
    ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1823 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtFloat,EbpLow);
        (yyval.interm.type).setMatrix(4, 4);
    ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1827 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtTexture,EbpUndefined);
    ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1830 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerGeneric,EbpUndefined);
    ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1833 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler1D,EbpUndefined);
    ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1836 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler2D,EbpUndefined);
    ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1839 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler2D,EbpMedium);
	;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1842 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler2D,EbpHigh);
	;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1845 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler3D,EbpLow);
    ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1848 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerCube,EbpUndefined);
    ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1851 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerCube,EbpMedium);
	;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1854 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerCube,EbpHigh);
	;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1857 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerRect,EbpUndefined);
    ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1860 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSamplerRectShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1863 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler1DShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1866 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler2DShadow,EbpLow); // ES3 doesn't have default precision for shadow samplers, so always emit lowp
    ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1869 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtSampler2DArray,EbpLow);
	;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1872 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.type) = (yyvsp[(1) - (1)].interm.type);
        (yyval.interm.type).qualifier = parseContext.getDefaultQualifier();
    ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1876 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //
        // This is for user defined type names.  The lexical phase looked up the
        // type.
        //
        TType& structure = static_cast<TVariable*>((yyvsp[(1) - (1)].lex).symbol)->getType();
        SET_BASIC_TYPE((yyval.interm.type),(yyvsp[(1) - (1)].lex),EbtStruct,EbpUndefined);
        (yyval.interm.type).userDef = &structure;
    ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1888 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TType* structure = new TType((yyvsp[(4) - (5)].interm.typeList), *(yyvsp[(2) - (5)].lex).string, EbpUndefined, (yyvsp[(2) - (5)].lex).line);
        TVariable* userTypeDef = new TVariable((yyvsp[(2) - (5)].lex).string, *structure, true);
        if (! parseContext.symbolTable.insert(*userTypeDef)) {
            parseContext.error((yyvsp[(2) - (5)].lex).line, "redefinition", (yyvsp[(2) - (5)].lex).string->c_str(), "struct");
            parseContext.recover();
        }
        (yyval.interm.type).setBasic(EbtStruct, EvqTemporary, (yyvsp[(1) - (5)].lex).line);
        (yyval.interm.type).userDef = structure;
    ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1898 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TType* structure = new TType((yyvsp[(3) - (4)].interm.typeList), TString(""), EbpUndefined, (yyvsp[(1) - (4)].lex).line);
        (yyval.interm.type).setBasic(EbtStruct, EvqTemporary, (yyvsp[(1) - (4)].lex).line);
        (yyval.interm.type).userDef = structure;
    ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1906 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeList) = (yyvsp[(1) - (1)].interm.typeList);
    ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1909 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeList) = (yyvsp[(1) - (2)].interm.typeList);
        for (unsigned int i = 0; i < (yyvsp[(2) - (2)].interm.typeList)->size(); ++i) {
            for (unsigned int j = 0; j < (yyval.interm.typeList)->size(); ++j) {
                if ((*(yyval.interm.typeList))[j].type->getFieldName() == (*(yyvsp[(2) - (2)].interm.typeList))[i].type->getFieldName()) {
                    parseContext.error((*(yyvsp[(2) - (2)].interm.typeList))[i].line, "duplicate field name in structure:", "struct", (*(yyvsp[(2) - (2)].interm.typeList))[i].type->getFieldName().c_str());
                    parseContext.recover();
                }
            }
            (yyval.interm.typeList)->push_back((*(yyvsp[(2) - (2)].interm.typeList))[i]);
        }
    ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1924 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeList) = (yyvsp[(2) - (3)].interm.typeList);

        if (parseContext.voidErrorCheck((yyvsp[(1) - (3)].interm.type).line, (*(yyvsp[(2) - (3)].interm.typeList))[0].type->getFieldName(), (yyvsp[(1) - (3)].interm.type))) {
            parseContext.recover();
        }
        for (unsigned int i = 0; i < (yyval.interm.typeList)->size(); ++i) {
            //
            // Careful not to replace already know aspects of type, like array-ness
            //
            TType* type = (*(yyval.interm.typeList))[i].type;
            type->setBasicType((yyvsp[(1) - (3)].interm.type).type);
            type->setPrecision((yyvsp[(1) - (3)].interm.type).precision);
            type->setColsCount((yyvsp[(1) - (3)].interm.type).matcols);
            type->setRowsCount((yyvsp[(1) - (3)].interm.type).matrows);
            type->setMatrix((yyvsp[(1) - (3)].interm.type).matrix);
            
            // don't allow arrays of arrays
            if (type->isArray()) {
                if (parseContext.arrayTypeErrorCheck((yyvsp[(1) - (3)].interm.type).line, (yyvsp[(1) - (3)].interm.type)))
                    parseContext.recover();
            }
            if ((yyvsp[(1) - (3)].interm.type).array)
                type->setArraySize((yyvsp[(1) - (3)].interm.type).arraySize);
            if ((yyvsp[(1) - (3)].interm.type).userDef) {
                type->setStruct((yyvsp[(1) - (3)].interm.type).userDef->getStruct());
                type->setTypeName((yyvsp[(1) - (3)].interm.type).userDef->getTypeName());
            }
        }
    ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1957 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeList) = NewPoolTTypeList();
        (yyval.interm.typeList)->push_back((yyvsp[(1) - (1)].interm.typeLine));
    ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1961 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeList)->push_back((yyvsp[(3) - (3)].interm.typeLine));
    ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1967 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[(1) - (1)].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[(1) - (1)].lex).string);
    ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1972 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[(1) - (3)].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[(1) - (3)].lex).string);
        (yyval.interm.typeLine).type->setSemantic(*(yyvsp[(3) - (3)].lex).string);
    ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1978 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[(1) - (4)].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[(1) - (4)].lex).string);

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[(2) - (4)].lex).line, (yyvsp[(3) - (4)].interm.intermTypedNode), size))
            parseContext.recover();
        (yyval.interm.typeLine).type->setArraySize(size);
    ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1988 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.typeLine).type = new TType(EbtVoid, EbpUndefined);
        (yyval.interm.typeLine).line = (yyvsp[(1) - (6)].lex).line;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[(1) - (6)].lex).string);

        int size;
        if (parseContext.arraySizeErrorCheck((yyvsp[(2) - (6)].lex).line, (yyvsp[(3) - (6)].interm.intermTypedNode), size))
            parseContext.recover();
        (yyval.interm.typeLine).type->setArraySize(size);
        (yyval.interm.typeLine).type->setSemantic(*(yyvsp[(6) - (6)].lex).string);
    ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 2004 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 2005 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 2006 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 2010 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermDeclaration); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 2014 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermAggregate); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 2015 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 2021 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 2022 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 2023 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 2024 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 2025 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 2029 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermAggregate) = 0; ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 2030 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { parseContext.symbolTable.push(); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 2030 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { parseContext.symbolTable.pop(); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 2030 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(3) - (5)].interm.intermAggregate) != 0)
            (yyvsp[(3) - (5)].interm.intermAggregate)->setOperator(EOpSequence);
        (yyval.interm.intermAggregate) = (yyvsp[(3) - (5)].interm.intermAggregate);
    ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 2038 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 2039 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 2044 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = 0;
    ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 2047 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if ((yyvsp[(2) - (3)].interm.intermAggregate))
            (yyvsp[(2) - (3)].interm.intermAggregate)->setOperator(EOpSequence);
        (yyval.interm.intermNode) = (yyvsp[(2) - (3)].interm.intermAggregate);
    ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 2055 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermAggregate) = ir_make_aggregate((yyvsp[(1) - (1)].interm.intermNode), gNullSourceLoc); 
    ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 2058 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { 
        (yyval.interm.intermAggregate) = ir_grow_aggregate((yyvsp[(1) - (2)].interm.intermAggregate), (yyvsp[(2) - (2)].interm.intermNode), gNullSourceLoc);
    ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 2064 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = 0; ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 2065 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = static_cast<TIntermNode*>((yyvsp[(1) - (2)].interm.intermTypedNode)); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 2069 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.boolErrorCheck((yyvsp[(1) - (5)].lex).line, (yyvsp[(3) - (5)].interm.intermTypedNode)))
            parseContext.recover();
        (yyval.interm.intermNode) = ir_add_selection((yyvsp[(3) - (5)].interm.intermTypedNode), (yyvsp[(5) - (5)].interm.nodePair), (yyvsp[(1) - (5)].lex).line, parseContext.infoSink);
    ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 2077 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.nodePair).node1 = (yyvsp[(1) - (3)].interm.intermNode);
        (yyval.interm.nodePair).node2 = (yyvsp[(3) - (3)].interm.intermNode);
    ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 2081 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.nodePair).node1 = (yyvsp[(1) - (1)].interm.intermNode);
        (yyval.interm.nodePair).node2 = 0;
    ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 2091 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
        if (parseContext.boolErrorCheck((yyvsp[(1) - (1)].interm.intermTypedNode)->getLine(), (yyvsp[(1) - (1)].interm.intermTypedNode)))
            parseContext.recover();
    ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 2096 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TIntermSymbol* symbol;
        if (parseContext.structQualifierErrorCheck((yyvsp[(2) - (4)].lex).line, (yyvsp[(1) - (4)].interm.type)))
            parseContext.recover();
        if (parseContext.boolErrorCheck((yyvsp[(2) - (4)].lex).line, (yyvsp[(1) - (4)].interm.type)))
            parseContext.recover();

        if (!parseContext.executeInitializer((yyvsp[(2) - (4)].lex).line, *(yyvsp[(2) - (4)].lex).string, (yyvsp[(1) - (4)].interm.type), (yyvsp[(4) - (4)].interm.intermTypedNode), symbol)) {
			(yyval.interm.intermTypedNode) = ir_add_declaration(symbol, (yyvsp[(4) - (4)].interm.intermTypedNode), (yyvsp[(2) - (4)].lex).line, parseContext);
        } else {
            parseContext.recover();
            (yyval.interm.intermTypedNode) = 0;
        }
    ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 2113 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 2113 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_add_loop(ELoopWhile, (yyvsp[(4) - (6)].interm.intermTypedNode), 0, (yyvsp[(6) - (6)].interm.intermNode), (yyvsp[(1) - (6)].lex).line);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 2118 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { ++parseContext.loopNestingLevel; ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 2118 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.boolErrorCheck((yyvsp[(8) - (8)].lex).line, (yyvsp[(6) - (8)].interm.intermTypedNode)))
            parseContext.recover();
                    
        (yyval.interm.intermNode) = ir_add_loop(ELoopDoWhile, (yyvsp[(6) - (8)].interm.intermTypedNode), 0, (yyvsp[(3) - (8)].interm.intermNode), (yyvsp[(4) - (8)].lex).line);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 2125 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 2125 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_make_aggregate((yyvsp[(4) - (7)].interm.intermNode), (yyvsp[(2) - (7)].lex).line);
        (yyval.interm.intermNode) = ir_grow_aggregate(
                (yyval.interm.intermNode),
                ir_add_loop(ELoopFor, reinterpret_cast<TIntermTyped*>((yyvsp[(5) - (7)].interm.nodePair).node1), reinterpret_cast<TIntermTyped*>((yyvsp[(5) - (7)].interm.nodePair).node2), (yyvsp[(7) - (7)].interm.intermNode), (yyvsp[(1) - (7)].lex).line),
                (yyvsp[(1) - (7)].lex).line);
        (yyval.interm.intermNode)->getAsAggregate()->setOperator(EOpSequence);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 2138 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode);
    ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 2141 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode);
    ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 2147 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = (yyvsp[(1) - (1)].interm.intermTypedNode);
    ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 2150 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermTypedNode) = 0;
    ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 2156 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.nodePair).node1 = (yyvsp[(1) - (2)].interm.intermTypedNode);
        (yyval.interm.nodePair).node2 = 0;
    ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 2160 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.nodePair).node1 = (yyvsp[(1) - (3)].interm.intermTypedNode);
        (yyval.interm.nodePair).node2 = (yyvsp[(3) - (3)].interm.intermTypedNode);
    ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 2167 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error((yyvsp[(1) - (2)].lex).line, "continue statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        (yyval.interm.intermNode) = ir_add_branch(EOpContinue, (yyvsp[(1) - (2)].lex).line);
    ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 2174 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error((yyvsp[(1) - (2)].lex).line, "break statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        (yyval.interm.intermNode) = ir_add_branch(EOpBreak, (yyvsp[(1) - (2)].lex).line);
    ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 2181 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = ir_add_branch(EOpReturn, (yyvsp[(1) - (2)].lex).line);
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid) {
            parseContext.error((yyvsp[(1) - (2)].lex).line, "non-void function must return a value", "return", "");
            parseContext.recover();
        }
    ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 2188 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TIntermTyped *temp = (yyvsp[(2) - (3)].interm.intermTypedNode);
        if (parseContext.currentFunctionType->getBasicType() == EbtVoid) {
            parseContext.error((yyvsp[(1) - (3)].lex).line, "void function cannot return a value", "return", "");
            parseContext.recover();
        } else if (*(parseContext.currentFunctionType) != (yyvsp[(2) - (3)].interm.intermTypedNode)->getType()) {
            TOperator op = parseContext.getConstructorOp(*(parseContext.currentFunctionType));
            if (op != EOpNull)
                temp = parseContext.constructBuiltIn((parseContext.currentFunctionType), op, (yyvsp[(2) - (3)].interm.intermTypedNode), (yyvsp[(1) - (3)].lex).line, false);
            else
                temp = 0;
            if (temp == 0) {
                parseContext.error((yyvsp[(1) - (3)].lex).line, "function return is not matching type:", "return", "");
                parseContext.recover();
                temp = (yyvsp[(2) - (3)].interm.intermTypedNode);
            }
        }
        (yyval.interm.intermNode) = ir_add_branch(EOpReturn, temp, (yyvsp[(1) - (3)].lex).line);
        parseContext.functionReturnsValue = true;
    ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 2208 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		// Jim: using discard when compiling vertex shaders should not be considered a syntactic error, instead,
		// we should issue a semantic error only if the code path is actually executed. (Not yet implemented)
        //FRAG_ONLY("discard", $1.line);
        (yyval.interm.intermNode) = ir_add_branch(EOpKill, (yyvsp[(1) - (2)].lex).line);
    ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 2219 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode);
        parseContext.treeRoot = (yyval.interm.intermNode);
    ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 2223 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = ir_grow_aggregate((yyvsp[(1) - (2)].interm.intermNode), (yyvsp[(2) - (2)].interm.intermNode), gNullSourceLoc);
        parseContext.treeRoot = (yyval.interm.intermNode);
    ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 2230 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermNode);
    ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 2233 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.intermNode) = (yyvsp[(1) - (1)].interm.intermDeclaration);
    ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 2236 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.intermNode) = 0; ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 2240 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        TFunction& function = *((yyvsp[(1) - (1)].interm).function);
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
            parseContext.error((yyvsp[(1) - (1)].interm).line, "function already has a body", function.getName().c_str(), "");
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
                    parseContext.error((yyvsp[(1) - (1)].interm).line, "redefinition", variable->getName().c_str(), "");
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
                                               ir_add_symbol(variable, (yyvsp[(1) - (1)].interm).line),
                                               (yyvsp[(1) - (1)].interm).line);
            } else {
                paramNodes = ir_grow_aggregate(paramNodes, ir_add_symbol_internal(0, "", param.info, *param.type, (yyvsp[(1) - (1)].interm).line), (yyvsp[(1) - (1)].interm).line);
            }
        }
        ir_set_aggregate_op(paramNodes, EOpParameters, (yyvsp[(1) - (1)].interm).line);
        (yyvsp[(1) - (1)].interm).intermAggregate = paramNodes;
        parseContext.loopNestingLevel = 0;
    ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 2309 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //?? Check that all paths return a value if return type != void ?
        //   May be best done as post process phase on intermediate code
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid && ! parseContext.functionReturnsValue) {
            parseContext.error((yyvsp[(1) - (3)].interm).line, "function does not return a value:", "", (yyvsp[(1) - (3)].interm).function->getName().c_str());
            parseContext.recover();
        }
        parseContext.symbolTable.pop();
        (yyval.interm.intermNode) = ir_grow_aggregate((yyvsp[(1) - (3)].interm).intermAggregate, (yyvsp[(3) - (3)].interm.intermNode), gNullSourceLoc);
        ir_set_aggregate_op((yyval.interm.intermNode), EOpFunction, (yyvsp[(1) - (3)].interm).line);
        (yyval.interm.intermNode)->getAsAggregate()->setName((yyvsp[(1) - (3)].interm).function->getMangledName().c_str());
        (yyval.interm.intermNode)->getAsAggregate()->setPlainName((yyvsp[(1) - (3)].interm).function->getName().c_str());
        (yyval.interm.intermNode)->getAsAggregate()->setType((yyvsp[(1) - (3)].interm).function->getReturnType());
        
	if ( (yyvsp[(1) - (3)].interm).function->getInfo())
	    (yyval.interm.intermNode)->getAsAggregate()->setSemantic((yyvsp[(1) - (3)].interm).function->getInfo()->getSemantic());
    ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 2329 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermTypedNode) = (yyvsp[(2) - (3)].interm.intermAggregate);
    ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 2332 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermTypedNode) = (yyvsp[(2) - (4)].interm.intermAggregate);
    ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 2339 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //create a new aggNode
       (yyval.interm.intermAggregate) = ir_make_aggregate( (yyvsp[(1) - (1)].interm.intermTypedNode), (yyvsp[(1) - (1)].interm.intermTypedNode)->getLine());       
    ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 2343 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
       //take the inherited aggNode and return it
       (yyval.interm.intermAggregate) = (yyvsp[(1) - (1)].interm.intermTypedNode)->getAsAggregate();
    ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 2347 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        // append to the aggNode
       (yyval.interm.intermAggregate) = ir_grow_aggregate( (yyvsp[(1) - (3)].interm.intermAggregate), (yyvsp[(3) - (3)].interm.intermTypedNode), (yyvsp[(3) - (3)].interm.intermTypedNode)->getLine());       
    ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 2351 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
       // append all children or $3 to $1
       (yyval.interm.intermAggregate) = parseContext.mergeAggregates( (yyvsp[(1) - (3)].interm.intermAggregate), (yyvsp[(3) - (3)].interm.intermTypedNode)->getAsAggregate());
    ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 2358 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        //empty annotation
      (yyval.interm.ann) = 0;
    ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 2362 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
      (yyval.interm.ann) = (yyvsp[(2) - (3)].interm.ann);
    ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 2368 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.interm.ann) = new TAnnotation;
		(yyval.interm.ann)->addKey( *(yyvsp[(1) - (1)].lex).string);
    ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 2372 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyvsp[(1) - (2)].interm.ann)->addKey( *(yyvsp[(2) - (2)].lex).string);
		(yyval.interm.ann) = (yyvsp[(1) - (2)].interm.ann);
    ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 2379 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.lex).string = (yyvsp[(2) - (5)].lex).string;
    ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 2385 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 2386 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 2387 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 2388 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 2389 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 2390 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 2391 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 2392 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 2393 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 2394 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 2395 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 2396 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 2397 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 2398 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 2399 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 2400 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 2401 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 2402 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 2403 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 2404 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 2405 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 2409 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 2410 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 2411 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 2412 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 2416 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.lex).f = (float)(yyvsp[(1) - (1)].lex).i;
	;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 2419 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.lex).f = ((yyvsp[(1) - (1)].lex).b) ? 1.0f : 0.0f;
	;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 2422 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.lex).f = (yyvsp[(1) - (1)].lex).f;
	;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 2428 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 2432 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 2433 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 2437 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 2441 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
        (yyval.lex) = (yyvsp[(4) - (5)].lex);
    ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 2447 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.lex).string = (yyvsp[(2) - (2)].lex).string;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 2451 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = 0;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 2452 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[(1) - (1)].lex).string, 0); ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 2453 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( "", *(yyvsp[(1) - (1)].lex).string, 0); ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 2454 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( "", (yyvsp[(1) - (1)].interm.ann)); ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 2455 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[(1) - (2)].lex).string, (yyvsp[(2) - (2)].interm.ann)); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 2456 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[(1) - (2)].lex).string, *(yyvsp[(2) - (2)].lex).string, 0); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 2457 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( "", *(yyvsp[(1) - (2)].lex).string, (yyvsp[(2) - (2)].interm.ann)); ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 2458 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { (yyval.interm.typeInfo) = new TTypeInfo( *(yyvsp[(1) - (3)].lex).string, *(yyvsp[(2) - (3)].lex).string, (yyvsp[(3) - (3)].interm.ann)); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 2462 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermTypedNode) = (yyvsp[(3) - (4)].interm.intermAggregate)
	;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 2465 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
	;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 2470 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { 
		TIntermAggregate* paramNodes = new TIntermAggregate;
		paramNodes->getNodes().push_back((yyvsp[(1) - (1)].interm.intermNode));
		(yyval.interm.intermAggregate) = paramNodes;
	;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 2475 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    { 
		(yyvsp[(1) - (2)].interm.intermAggregate)->getNodes().push_back((yyvsp[(2) - (2)].interm.intermNode));
		(yyval.interm.intermAggregate) = (yyvsp[(1) - (2)].interm.intermAggregate);
	;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 2482 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem(*(yyvsp[(1) - (4)].lex).string,*(yyvsp[(3) - (4)].lex).string);
	;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 2485 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem(*(yyvsp[(1) - (6)].lex).string,*(yyvsp[(4) - (6)].lex).string);
	;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 2488 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem(*(yyvsp[(1) - (6)].lex).string,*(yyvsp[(4) - (6)].lex).string);
	;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 2491 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem("texture",*(yyvsp[(3) - (4)].lex).string);
	;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 2494 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem("texture",*(yyvsp[(4) - (6)].lex).string);
	;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 2497 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"
    {
		(yyval.interm.intermNode) = new TIntermInitItem("texture",*(yyvsp[(4) - (6)].lex).string);
	;}
    break;



/* Line 1455 of yacc.c  */
#line 6519 "hlslang.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (parseContext, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (parseContext, yymsg);
	  }
	else
	  {
	    yyerror (parseContext, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, parseContext);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 2502 "F:/GitRepos/NPLRuntime/NPLRuntime/externals/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y"


