#include "Python3LexerBase.h"

// Generated from Python3Lexer.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"




class  Python3Lexer : public Python3LexerBase {
public:
  enum {
    INDENT = 1, DEDENT = 2, STRING = 3, NUMBER = 4, INTEGER = 5, AND = 6, 
    AS = 7, ASSERT = 8, ASYNC = 9, AWAIT = 10, BREAK = 11, CASE = 12, CLASS = 13, 
    CONTINUE = 14, DEF = 15, DEL = 16, ELIF = 17, ELSE = 18, EXCEPT = 19, 
    FALSE = 20, FINALLY = 21, FOR = 22, FROM = 23, GLOBAL = 24, IF = 25, 
    IMPORT = 26, IN = 27, IS = 28, LAMBDA = 29, MATCH = 30, NONE = 31, NONLOCAL = 32, 
    NOT = 33, OR = 34, PASS = 35, RAISE = 36, RETURN = 37, TRUE = 38, TRY = 39, 
    UNDERSCORE = 40, WHILE = 41, WITH = 42, YIELD = 43, NEWLINE = 44, NAME = 45, 
    STRING_LITERAL = 46, BYTES_LITERAL = 47, DECIMAL_INTEGER = 48, OCT_INTEGER = 49, 
    HEX_INTEGER = 50, BIN_INTEGER = 51, FLOAT_NUMBER = 52, IMAG_NUMBER = 53, 
    DOT = 54, ELLIPSIS = 55, STAR = 56, OPEN_PAREN = 57, CLOSE_PAREN = 58, 
    COMMA = 59, COLON = 60, SEMI_COLON = 61, POWER = 62, ASSIGN = 63, OPEN_BRACK = 64, 
    CLOSE_BRACK = 65, OR_OP = 66, XOR = 67, AND_OP = 68, LEFT_SHIFT = 69, 
    RIGHT_SHIFT = 70, ADD = 71, MINUS = 72, DIV = 73, MOD = 74, IDIV = 75, 
    NOT_OP = 76, OPEN_BRACE = 77, CLOSE_BRACE = 78, LESS_THAN = 79, GREATER_THAN = 80, 
    EQUALS = 81, GT_EQ = 82, LT_EQ = 83, NOT_EQ_1 = 84, NOT_EQ_2 = 85, AT = 86, 
    ARROW = 87, ADD_ASSIGN = 88, SUB_ASSIGN = 89, MULT_ASSIGN = 90, AT_ASSIGN = 91, 
    DIV_ASSIGN = 92, MOD_ASSIGN = 93, AND_ASSIGN = 94, OR_ASSIGN = 95, XOR_ASSIGN = 96, 
    LEFT_SHIFT_ASSIGN = 97, RIGHT_SHIFT_ASSIGN = 98, POWER_ASSIGN = 99, 
    IDIV_ASSIGN = 100, SKIP_ = 101, UNKNOWN_CHAR = 102
  };

  explicit Python3Lexer(antlr4::CharStream *input);

  ~Python3Lexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;

  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.
  void NEWLINEAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_PARENAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_PARENAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_BRACKAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_BRACKAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_BRACEAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_BRACEAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.
  bool NEWLINESempred(antlr4::RuleContext *_localctx, size_t predicateIndex);

};

