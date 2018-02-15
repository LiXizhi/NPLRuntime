/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
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

// Take the name prefix into account.
#define yylex   DxEffectsParserlex

/* First part of user declarations.  */
/* Line 280 of lalr1.cc  */
#line 4 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
 /*** C/C++ Declarations ***/

#include <cstdio>
#include <string>
#include <vector>

#include "dxeffects.h"

class TechniqueNode;



/* Line 280 of lalr1.cc  */
#line 53 "parser.cc"


#include "parser.h"

/* User implementation prologue.  */
/* Line 286 of lalr1.cc  */
#line 112 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"


#include "driver.h"
#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex


/* Line 286 of lalr1.cc  */
#line 74 "parser.cc"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location) YYUSE(Type)
# define YY_REDUCE_PRINT(Rule)        static_cast<void>(0)
# define YY_STACK_PRINT()             static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace DxEffectsParser {
/* Line 354 of lalr1.cc  */
#line 169 "parser.cc"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
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
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Parser::Parser (class Driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  Parser::~Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 10: /* IDENTIFIER */
/* Line 456 of lalr1.cc  */
#line 106 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
        { delete ((*yyvaluep).stringVal); };
/* Line 456 of lalr1.cc  */
#line 276 "parser.cc"
        break;
      case 34: /* stmt_pass */
/* Line 456 of lalr1.cc  */
#line 108 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
        { delete ((*yyvaluep).passValue); };
/* Line 456 of lalr1.cc  */
#line 283 "parser.cc"
        break;
      case 36: /* stmt_tec */
/* Line 456 of lalr1.cc  */
#line 107 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
        { delete ((*yyvaluep).techValue); };
/* Line 456 of lalr1.cc  */
#line 290 "parser.cc"
        break;

	default:
	  break;
      }
  }

  void
  Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    // State.
    int yyn;
    int yylen = 0;
    int yystate = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    static semantic_type yyval_default;
    semantic_type yylval = yyval_default;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


/* User initialization code.  */
/* Line 546 of lalr1.cc  */
#line 44 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
{
    // initialize the initial location object
    yylloc.begin.filename = yylloc.end.filename = &driver.streamname;
}
/* Line 546 of lalr1.cc  */
#line 391 "parser.cc"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex (&yylval, &yylloc);
      }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    // Compute the default @$.
    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
          case 2:
/* Line 671 of lalr1.cc  */
#line 129 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.floatVal) = (yysemantic_stack_[(1) - (1)].floatVal);}
    break;

  case 3:
/* Line 671 of lalr1.cc  */
#line 130 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.floatVal) = (yysemantic_stack_[(1) - (1)].integerVal);}
    break;

  case 4:
/* Line 671 of lalr1.cc  */
#line 132 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);}
    break;

  case 5:
/* Line 671 of lalr1.cc  */
#line 133 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);}
    break;

  case 6:
/* Line 671 of lalr1.cc  */
#line 134 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);}
    break;

  case 7:
/* Line 671 of lalr1.cc  */
#line 137 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateIntegerValue((yysemantic_stack_[(1) - (1)].integerVal));}
    break;

  case 8:
/* Line 671 of lalr1.cc  */
#line 138 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateFloatValue((yysemantic_stack_[(1) - (1)].floatVal));}
    break;

  case 9:
/* Line 671 of lalr1.cc  */
#line 139 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateBooleanValue((yysemantic_stack_[(1) - (1)].boolVal));}
    break;

  case 10:
/* Line 671 of lalr1.cc  */
#line 140 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateStringValue(*(yysemantic_stack_[(1) - (1)].stringVal));delete (yysemantic_stack_[(1) - (1)].stringVal);}
    break;

  case 11:
/* Line 671 of lalr1.cc  */
#line 141 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateCompileValue(*(yysemantic_stack_[(5) - (2)].stringVal),*(yysemantic_stack_[(5) - (3)].stringVal));delete (yysemantic_stack_[(5) - (2)].stringVal);delete (yysemantic_stack_[(5) - (3)].stringVal);}
    break;

  case 12:
/* Line 671 of lalr1.cc  */
#line 142 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateValue) = new StateCompileValue(*(yysemantic_stack_[(5) - (2)].stringVal),*(yysemantic_stack_[(5) - (3)].stringVal));delete (yysemantic_stack_[(5) - (2)].stringVal);delete (yysemantic_stack_[(5) - (3)].stringVal);}
    break;

  case 13:
/* Line 671 of lalr1.cc  */
#line 143 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
					float temp[2] = {(yysemantic_stack_[(6) - (3)].floatVal),(yysemantic_stack_[(6) - (5)].floatVal)};
					(yyval.stateValue) = new StateFloat2Value(temp);
				}
    break;

  case 14:
/* Line 671 of lalr1.cc  */
#line 147 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
					float temp[3] = {(yysemantic_stack_[(8) - (3)].floatVal),(yysemantic_stack_[(8) - (5)].floatVal),(yysemantic_stack_[(8) - (7)].floatVal)};
					(yyval.stateValue) = new StateFloat3Value(temp);
				}
    break;

  case 15:
/* Line 671 of lalr1.cc  */
#line 151 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
					float temp[4] = {(yysemantic_stack_[(10) - (3)].floatVal),(yysemantic_stack_[(10) - (5)].floatVal),(yysemantic_stack_[(10) - (7)].floatVal),(yysemantic_stack_[(10) - (9)].floatVal)};
					(yyval.stateValue) = new StateFloat4Value(temp);
				}
    break;

  case 16:
/* Line 671 of lalr1.cc  */
#line 157 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.integerVal) = (yysemantic_stack_[(3) - (2)].integerVal);}
    break;

  case 17:
/* Line 671 of lalr1.cc  */
#line 159 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateAssignmentValue) = new StateAssignmentNode(*(yysemantic_stack_[(4) - (1)].stringVal),(yysemantic_stack_[(4) - (3)].stateValue),-1);delete (yysemantic_stack_[(4) - (1)].stringVal);}
    break;

  case 18:
/* Line 671 of lalr1.cc  */
#line 160 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateAssignmentValue) = new StateAssignmentNode(*(yysemantic_stack_[(5) - (1)].stringVal),(yysemantic_stack_[(5) - (4)].stateValue),(yysemantic_stack_[(5) - (2)].integerVal));delete (yysemantic_stack_[(5) - (1)].stringVal);}
    break;

  case 19:
/* Line 671 of lalr1.cc  */
#line 162 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.stateAssignmentValues) = new std::vector<StateAssignmentNode*>();}
    break;

  case 20:
/* Line 671 of lalr1.cc  */
#line 163 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
												(yyval.stateAssignmentValues) = new std::vector<StateAssignmentNode*>();
												(yyval.stateAssignmentValues)->push_back((yysemantic_stack_[(2) - (1)].stateAssignmentValue));
												(yyval.stateAssignmentValues)->insert((yyval.stateAssignmentValues)->end(),(yysemantic_stack_[(2) - (2)].stateAssignmentValues)->begin(),(yysemantic_stack_[(2) - (2)].stateAssignmentValues)->end());
												delete (yysemantic_stack_[(2) - (2)].stateAssignmentValues);
											  }
    break;

  case 21:
/* Line 671 of lalr1.cc  */
#line 170 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.passValue) = new PassNode(*(yysemantic_stack_[(5) - (2)].stringVal),*(yysemantic_stack_[(5) - (4)].stateAssignmentValues));delete (yysemantic_stack_[(5) - (2)].stringVal);}
    break;

  case 22:
/* Line 671 of lalr1.cc  */
#line 172 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {(yyval.passValues) = new std::vector<PassNode*>();(yyval.passValues)->push_back((yysemantic_stack_[(1) - (1)].passValue));}
    break;

  case 23:
/* Line 671 of lalr1.cc  */
#line 173 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
											(yyval.passValues) = new std::vector<PassNode*>();
											(yyval.passValues)->push_back((yysemantic_stack_[(2) - (1)].passValue));
											(yyval.passValues)->insert((yyval.passValues)->end(),(yysemantic_stack_[(2) - (2)].passValues)->begin(),(yysemantic_stack_[(2) - (2)].passValues)->end());
											delete (yysemantic_stack_[(2) - (2)].passValues);
										   }
    break;

  case 24:
/* Line 671 of lalr1.cc  */
#line 180 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
                                                                (yyval.techValue) = new TechniqueNode(*(yysemantic_stack_[(5) - (2)].stringVal),*(yysemantic_stack_[(5) - (4)].passValues));
																driver.tree.AddTechnique(*(yyval.techValue));
																delete (yysemantic_stack_[(5) - (2)].stringVal);
                                                                delete (yysemantic_stack_[(5) - (4)].passValues);
                                                            }
    break;

  case 25:
/* Line 671 of lalr1.cc  */
#line 187 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {}
    break;

  case 26:
/* Line 671 of lalr1.cc  */
#line 188 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {}
    break;

  case 27:
/* Line 671 of lalr1.cc  */
#line 190 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
    {
				driver.tree.AddCodeBlock(*(yysemantic_stack_[(2) - (1)].stringVal));delete (yysemantic_stack_[(2) - (1)].stringVal);
		}
    break;


/* Line 671 of lalr1.cc  */
#line 693 "parser.cc"
      default:
        break;
      }

    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */
        if (yychar <= yyeof_)
          {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_)
              YYABORT;
          }
        else
          {
            yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
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
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	  YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystate_stack_.height ())
      {
        yydestruct_ ("Cleanup: popping",
                     yystos_[yystate_stack_[0]],
                     &yysemantic_stack_[0],
                     &yylocation_stack_[0]);
        yypop_ ();
      }

    return yyresult;
    }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (yychar != yyempty_)
          {
            /* Make sure we have latest lookahead translation.  See
               comments at user semantic actions for why this is
               necessary.  */
            yytoken = yytranslate_ (yychar);
            yydestruct_ (YY_NULL, yytoken, &yylval, &yylloc);
          }

        while (1 < yystate_stack_.height ())
          {
            yydestruct_ (YY_NULL,
                         yystos_[yystate_stack_[0]],
                         &yysemantic_stack_[0],
                         &yylocation_stack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULL;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char Parser::yypact_ninf_ = -42;
  const signed char
  Parser::yypact_[] =
  {
        17,     4,    23,    20,     4,   -42,   -42,     6,   -42,    29,
      24,    29,     7,    10,   -42,   -42,    28,    -2,    28,    11,
      26,    -3,    16,   -42,   -42,    18,    25,    27,    30,   -42,
     -42,   -42,   -42,   -42,   -42,     8,   -42,    22,    -3,   -42,
      14,    14,    14,    12,   -42,    31,   -42,   -42,    32,    33,
      34,    38,    40,   -42,    14,    14,    14,    41,    42,    21,
      43,    44,   -42,   -42,   -42,    14,    14,    37,    45,   -42,
      14,    46,   -42
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         0,     0,     0,     0,    25,    27,     1,     0,    26,     0,
       0,    22,     0,     0,    23,    24,    19,     0,    19,     0,
       0,     0,     0,    20,    21,     0,     0,     0,     0,     5,
       4,     6,     7,     8,     9,     0,    10,     0,     0,    16,
       0,     0,     0,     0,    17,     0,     3,     2,     0,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    11,    13,     0,     0,     0,     0,    14,
       0,     0,    15
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -42,   -41,     9,    35,   -42,   -42,    47,   -42,    36,   -42,
      56,   -42
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,    48,    36,    37,    22,    18,    19,    11,    12,     4,
       5,     2
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char Parser::yytable_ninf_ = -1;
  const unsigned char
  Parser::yytable_[] =
  {
        49,    50,    26,    27,    28,    29,    30,    31,     3,    32,
      33,    34,    35,    59,    60,    61,    29,    30,    31,    20,
      51,    21,    52,     6,    67,    68,    46,    47,     1,    71,
       7,     9,    10,    15,    13,    16,    17,    24,    25,    38,
      39,    64,     0,    40,    43,    41,    44,    14,    42,     0,
       0,    54,    55,    56,    57,    53,    58,    69,    62,    63,
       8,     0,    65,    66,    70,    23,    72,     0,     0,     0,
       0,     0,     0,    45
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
        41,    42,     5,     6,     7,     8,     9,    10,     4,    12,
      13,    14,    15,    54,    55,    56,     8,     9,    10,    21,
       8,    23,    10,     0,    65,    66,    12,    13,    11,    70,
      10,    25,     3,    26,    10,    25,     8,    26,    12,    23,
      22,    20,    -1,    18,    35,    18,    24,    11,    18,    -1,
      -1,    19,    19,    19,    16,    24,    16,    20,    17,    17,
       4,    -1,    19,    19,    19,    18,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    38
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    11,    38,     4,    36,    37,     0,    10,    37,    25,
       3,    34,    35,    10,    35,    26,    25,     8,    32,    33,
      21,    23,    31,    33,    26,    12,     5,     6,     7,     8,
       9,    10,    12,    13,    14,    15,    29,    30,    23,    22,
      18,    18,    18,    29,    24,    30,    12,    13,    28,    28,
      28,     8,    10,    24,    19,    19,    19,    16,    16,    28,
      28,    28,    17,    17,    20,    19,    19,    28,    28,    20,
      19,    28,    20
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    40,    41,    60,    44,
      62,    91,    93,    61,    59,   123,   125
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    27,    28,    28,    29,    29,    29,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    31,    32,    32,    33,
      33,    34,    35,    35,    36,    37,    37,    38
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     5,     6,     8,    10,     3,     4,     5,     0,
       2,     5,     1,     2,     5,     1,     2,     2
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "END", "error", "$undefined", "PASS", "TECHNIQUE", "FLOAT2", "FLOAT3",
  "FLOAT4", "STATE_NAME", "STRING", "IDENTIFIER", "HLSL_CODE_BLOCK",
  "INTEGER", "FLOAT", "BOOLEAN", "COMPILE", "'('", "')'", "'<'", "','",
  "'>'", "'['", "']'", "'='", "';'", "'{'", "'}'", "$accept", "stmt_float",
  "stmt_string", "stmt_state_value", "stmt_state_name_index", "stmt_state",
  "stmt_state_list", "stmt_pass", "stmt_pass_list", "stmt_tec",
  "stmt_tec_list", "start", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        38,     0,    -1,    13,    -1,    12,    -1,     9,    -1,     8,
      -1,    10,    -1,    12,    -1,    13,    -1,    14,    -1,    29,
      -1,    15,    29,    10,    16,    17,    -1,    15,    29,     8,
      16,    17,    -1,     5,    18,    28,    19,    28,    20,    -1,
       6,    18,    28,    19,    28,    19,    28,    20,    -1,     7,
      18,    28,    19,    28,    19,    28,    19,    28,    20,    -1,
      21,    12,    22,    -1,     8,    23,    30,    24,    -1,     8,
      31,    23,    30,    24,    -1,    -1,    32,    33,    -1,     3,
      10,    25,    33,    26,    -1,    34,    -1,    34,    35,    -1,
       4,    10,    25,    35,    26,    -1,    36,    -1,    36,    37,
      -1,    11,    37,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    27,    33,    40,    49,    60,    64,    69,    75,
      76,    79,    85,    87,    90,    96,    98,   101
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned char
  Parser::yyrline_[] =
  {
         0,   129,   129,   130,   132,   133,   134,   137,   138,   139,
     140,   141,   142,   143,   147,   151,   157,   159,   160,   162,
     163,   170,   172,   173,   180,   187,   188,   190
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  Parser::token_number_type
  Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      16,    17,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    24,
      18,    23,    20,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    21,     2,    22,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    25,     2,    26,     2,     2,     2,     2,
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
      15
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 73;
  const int Parser::yynnts_ = 12;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 6;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 27;

  const unsigned int Parser::yyuser_token_number_max_ = 270;
  const Parser::token_number_type Parser::yyundef_token_ = 2;


} // DxEffectsParser
/* Line 1142 of lalr1.cc  */
#line 1254 "parser.cc"
/* Line 1143 of lalr1.cc  */
#line 196 "E:/Source/GitRepos/dxEffectsParser/src/parser.yy"
 /*** Additional Code ***/

void DxEffectsParser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
