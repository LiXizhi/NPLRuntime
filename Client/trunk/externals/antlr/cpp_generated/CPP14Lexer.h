
// Generated from CPP14Lexer.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"




class  CPP14Lexer : public antlr4::Lexer {
public:
  enum {
    IntegerLiteral = 1, CharacterLiteral = 2, FloatingLiteral = 3, StringLiteral = 4, 
    BooleanLiteral = 5, PointerLiteral = 6, UserDefinedLiteral = 7, MultiLineMacro = 8, 
    Directive = 9, Alignas = 10, Alignof = 11, Asm = 12, Auto = 13, Bool = 14, 
    Break = 15, Case = 16, Catch = 17, Char = 18, Char16 = 19, Char32 = 20, 
    Class = 21, Const = 22, Constexpr = 23, Const_cast = 24, Continue = 25, 
    Decltype = 26, Default = 27, Delete = 28, Do = 29, Double = 30, Dynamic_cast = 31, 
    Else = 32, Enum = 33, Explicit = 34, Export = 35, Extern = 36, False_ = 37, 
    Final = 38, Float = 39, For = 40, Friend = 41, Goto = 42, If = 43, Inline = 44, 
    Int = 45, Long = 46, Mutable = 47, Namespace = 48, New = 49, Noexcept = 50, 
    Nullptr = 51, Operator = 52, Override = 53, Private = 54, Protected = 55, 
    Public = 56, Register = 57, Reinterpret_cast = 58, Return = 59, Short = 60, 
    Signed = 61, Sizeof = 62, Static = 63, Static_assert = 64, Static_cast = 65, 
    Struct = 66, Switch = 67, Template = 68, This = 69, Thread_local = 70, 
    Throw = 71, True_ = 72, Try = 73, Typedef = 74, Typeid_ = 75, Typename_ = 76, 
    Union = 77, Unsigned = 78, Using = 79, Virtual = 80, Void = 81, Volatile = 82, 
    Wchar = 83, While = 84, LeftParen = 85, RightParen = 86, LeftBracket = 87, 
    RightBracket = 88, LeftBrace = 89, RightBrace = 90, Plus = 91, Minus = 92, 
    Star = 93, Div = 94, Mod = 95, Caret = 96, And = 97, Or = 98, Tilde = 99, 
    Not = 100, Assign = 101, Less = 102, Greater = 103, PlusAssign = 104, 
    MinusAssign = 105, StarAssign = 106, DivAssign = 107, ModAssign = 108, 
    XorAssign = 109, AndAssign = 110, OrAssign = 111, LeftShiftAssign = 112, 
    RightShiftAssign = 113, Equal = 114, NotEqual = 115, LessEqual = 116, 
    GreaterEqual = 117, AndAnd = 118, OrOr = 119, PlusPlus = 120, MinusMinus = 121, 
    Comma = 122, ArrowStar = 123, Arrow = 124, Question = 125, Colon = 126, 
    Doublecolon = 127, Semi = 128, Dot = 129, DotStar = 130, Ellipsis = 131, 
    Identifier = 132, DecimalLiteral = 133, OctalLiteral = 134, HexadecimalLiteral = 135, 
    BinaryLiteral = 136, Integersuffix = 137, UserDefinedIntegerLiteral = 138, 
    UserDefinedFloatingLiteral = 139, UserDefinedStringLiteral = 140, UserDefinedCharacterLiteral = 141, 
    Whitespace = 142, Newline = 143, BlockComment = 144, LineComment = 145
  };

  explicit CPP14Lexer(antlr4::CharStream *input);

  ~CPP14Lexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

