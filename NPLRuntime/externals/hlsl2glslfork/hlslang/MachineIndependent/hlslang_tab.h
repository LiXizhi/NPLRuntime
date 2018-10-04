
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 226 "hlslang.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




