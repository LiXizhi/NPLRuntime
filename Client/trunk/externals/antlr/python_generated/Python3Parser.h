#include "Python3ParserBase.h"

// Generated from Python3Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"




class  Python3Parser : public Python3ParserBase {
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

  enum {
    RuleSingle_input = 0, RuleFile_input = 1, RuleEval_input = 2, RuleDecorator = 3, 
    RuleDecorators = 4, RuleDecorated = 5, RuleAsync_funcdef = 6, RuleFuncdef = 7, 
    RuleParameters = 8, RuleTypedargslist = 9, RuleTfpdef = 10, RuleVarargslist = 11, 
    RuleVfpdef = 12, RuleStmt = 13, RuleSimple_stmts = 14, RuleSimple_stmt = 15, 
    RuleExpr_stmt = 16, RuleAnnassign = 17, RuleTestlist_star_expr = 18, 
    RuleAugassign = 19, RuleDel_stmt = 20, RulePass_stmt = 21, RuleFlow_stmt = 22, 
    RuleBreak_stmt = 23, RuleContinue_stmt = 24, RuleReturn_stmt = 25, RuleYield_stmt = 26, 
    RuleRaise_stmt = 27, RuleImport_stmt = 28, RuleImport_name = 29, RuleImport_from = 30, 
    RuleImport_as_name = 31, RuleDotted_as_name = 32, RuleImport_as_names = 33, 
    RuleDotted_as_names = 34, RuleDotted_name = 35, RuleGlobal_stmt = 36, 
    RuleNonlocal_stmt = 37, RuleAssert_stmt = 38, RuleCompound_stmt = 39, 
    RuleAsync_stmt = 40, RuleIf_stmt = 41, RuleWhile_stmt = 42, RuleFor_stmt = 43, 
    RuleTry_stmt = 44, RuleWith_stmt = 45, RuleWith_item = 46, RuleExcept_clause = 47, 
    RuleBlock = 48, RuleMatch_stmt = 49, RuleSubject_expr = 50, RuleStar_named_expressions = 51, 
    RuleStar_named_expression = 52, RuleCase_block = 53, RuleGuard = 54, 
    RulePatterns = 55, RulePattern = 56, RuleAs_pattern = 57, RuleOr_pattern = 58, 
    RuleClosed_pattern = 59, RuleLiteral_pattern = 60, RuleLiteral_expr = 61, 
    RuleComplex_number = 62, RuleSigned_number = 63, RuleSigned_real_number = 64, 
    RuleReal_number = 65, RuleImaginary_number = 66, RuleCapture_pattern = 67, 
    RulePattern_capture_target = 68, RuleWildcard_pattern = 69, RuleValue_pattern = 70, 
    RuleAttr = 71, RuleName_or_attr = 72, RuleGroup_pattern = 73, RuleSequence_pattern = 74, 
    RuleOpen_sequence_pattern = 75, RuleMaybe_sequence_pattern = 76, RuleMaybe_star_pattern = 77, 
    RuleStar_pattern = 78, RuleMapping_pattern = 79, RuleItems_pattern = 80, 
    RuleKey_value_pattern = 81, RuleDouble_star_pattern = 82, RuleClass_pattern = 83, 
    RulePositional_patterns = 84, RuleKeyword_patterns = 85, RuleKeyword_pattern = 86, 
    RuleTest = 87, RuleTest_nocond = 88, RuleLambdef = 89, RuleLambdef_nocond = 90, 
    RuleOr_test = 91, RuleAnd_test = 92, RuleNot_test = 93, RuleComparison = 94, 
    RuleComp_op = 95, RuleStar_expr = 96, RuleExpr = 97, RuleAtom_expr = 98, 
    RuleAtom = 99, RuleName = 100, RuleTestlist_comp = 101, RuleTrailer = 102, 
    RuleSubscriptlist = 103, RuleSubscript_ = 104, RuleSliceop = 105, RuleExprlist = 106, 
    RuleTestlist = 107, RuleDictorsetmaker = 108, RuleClassdef = 109, RuleArglist = 110, 
    RuleArgument = 111, RuleComp_iter = 112, RuleComp_for = 113, RuleComp_if = 114, 
    RuleEncoding_decl = 115, RuleYield_expr = 116, RuleYield_arg = 117, 
    RuleStrings = 118
  };

  explicit Python3Parser(antlr4::TokenStream *input);

  Python3Parser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~Python3Parser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class Single_inputContext;
  class File_inputContext;
  class Eval_inputContext;
  class DecoratorContext;
  class DecoratorsContext;
  class DecoratedContext;
  class Async_funcdefContext;
  class FuncdefContext;
  class ParametersContext;
  class TypedargslistContext;
  class TfpdefContext;
  class VarargslistContext;
  class VfpdefContext;
  class StmtContext;
  class Simple_stmtsContext;
  class Simple_stmtContext;
  class Expr_stmtContext;
  class AnnassignContext;
  class Testlist_star_exprContext;
  class AugassignContext;
  class Del_stmtContext;
  class Pass_stmtContext;
  class Flow_stmtContext;
  class Break_stmtContext;
  class Continue_stmtContext;
  class Return_stmtContext;
  class Yield_stmtContext;
  class Raise_stmtContext;
  class Import_stmtContext;
  class Import_nameContext;
  class Import_fromContext;
  class Import_as_nameContext;
  class Dotted_as_nameContext;
  class Import_as_namesContext;
  class Dotted_as_namesContext;
  class Dotted_nameContext;
  class Global_stmtContext;
  class Nonlocal_stmtContext;
  class Assert_stmtContext;
  class Compound_stmtContext;
  class Async_stmtContext;
  class If_stmtContext;
  class While_stmtContext;
  class For_stmtContext;
  class Try_stmtContext;
  class With_stmtContext;
  class With_itemContext;
  class Except_clauseContext;
  class BlockContext;
  class Match_stmtContext;
  class Subject_exprContext;
  class Star_named_expressionsContext;
  class Star_named_expressionContext;
  class Case_blockContext;
  class GuardContext;
  class PatternsContext;
  class PatternContext;
  class As_patternContext;
  class Or_patternContext;
  class Closed_patternContext;
  class Literal_patternContext;
  class Literal_exprContext;
  class Complex_numberContext;
  class Signed_numberContext;
  class Signed_real_numberContext;
  class Real_numberContext;
  class Imaginary_numberContext;
  class Capture_patternContext;
  class Pattern_capture_targetContext;
  class Wildcard_patternContext;
  class Value_patternContext;
  class AttrContext;
  class Name_or_attrContext;
  class Group_patternContext;
  class Sequence_patternContext;
  class Open_sequence_patternContext;
  class Maybe_sequence_patternContext;
  class Maybe_star_patternContext;
  class Star_patternContext;
  class Mapping_patternContext;
  class Items_patternContext;
  class Key_value_patternContext;
  class Double_star_patternContext;
  class Class_patternContext;
  class Positional_patternsContext;
  class Keyword_patternsContext;
  class Keyword_patternContext;
  class TestContext;
  class Test_nocondContext;
  class LambdefContext;
  class Lambdef_nocondContext;
  class Or_testContext;
  class And_testContext;
  class Not_testContext;
  class ComparisonContext;
  class Comp_opContext;
  class Star_exprContext;
  class ExprContext;
  class Atom_exprContext;
  class AtomContext;
  class NameContext;
  class Testlist_compContext;
  class TrailerContext;
  class SubscriptlistContext;
  class Subscript_Context;
  class SliceopContext;
  class ExprlistContext;
  class TestlistContext;
  class DictorsetmakerContext;
  class ClassdefContext;
  class ArglistContext;
  class ArgumentContext;
  class Comp_iterContext;
  class Comp_forContext;
  class Comp_ifContext;
  class Encoding_declContext;
  class Yield_exprContext;
  class Yield_argContext;
  class StringsContext; 

  class  Single_inputContext : public antlr4::ParserRuleContext {
  public:
    Single_inputContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    Simple_stmtsContext *simple_stmts();
    Compound_stmtContext *compound_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Single_inputContext* single_input();

  class  File_inputContext : public antlr4::ParserRuleContext {
  public:
    File_inputContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<antlr4::tree::TerminalNode *> NEWLINE();
    antlr4::tree::TerminalNode* NEWLINE(size_t i);
    std::vector<StmtContext *> stmt();
    StmtContext* stmt(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  File_inputContext* file_input();

  class  Eval_inputContext : public antlr4::ParserRuleContext {
  public:
    Eval_inputContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TestlistContext *testlist();
    antlr4::tree::TerminalNode *EOF();
    std::vector<antlr4::tree::TerminalNode *> NEWLINE();
    antlr4::tree::TerminalNode* NEWLINE(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Eval_inputContext* eval_input();

  class  DecoratorContext : public antlr4::ParserRuleContext {
  public:
    DecoratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    Dotted_nameContext *dotted_name();
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    ArglistContext *arglist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DecoratorContext* decorator();

  class  DecoratorsContext : public antlr4::ParserRuleContext {
  public:
    DecoratorsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<DecoratorContext *> decorator();
    DecoratorContext* decorator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DecoratorsContext* decorators();

  class  DecoratedContext : public antlr4::ParserRuleContext {
  public:
    DecoratedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DecoratorsContext *decorators();
    ClassdefContext *classdef();
    FuncdefContext *funcdef();
    Async_funcdefContext *async_funcdef();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DecoratedContext* decorated();

  class  Async_funcdefContext : public antlr4::ParserRuleContext {
  public:
    Async_funcdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASYNC();
    FuncdefContext *funcdef();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Async_funcdefContext* async_funcdef();

  class  FuncdefContext : public antlr4::ParserRuleContext {
  public:
    FuncdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DEF();
    NameContext *name();
    ParametersContext *parameters();
    antlr4::tree::TerminalNode *COLON();
    BlockContext *block();
    antlr4::tree::TerminalNode *ARROW();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FuncdefContext* funcdef();

  class  ParametersContext : public antlr4::ParserRuleContext {
  public:
    ParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    TypedargslistContext *typedargslist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParametersContext* parameters();

  class  TypedargslistContext : public antlr4::ParserRuleContext {
  public:
    TypedargslistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TfpdefContext *> tfpdef();
    TfpdefContext* tfpdef(size_t i);
    antlr4::tree::TerminalNode *STAR();
    antlr4::tree::TerminalNode *POWER();
    std::vector<antlr4::tree::TerminalNode *> ASSIGN();
    antlr4::tree::TerminalNode* ASSIGN(size_t i);
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypedargslistContext* typedargslist();

  class  TfpdefContext : public antlr4::ParserRuleContext {
  public:
    TfpdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    antlr4::tree::TerminalNode *COLON();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TfpdefContext* tfpdef();

  class  VarargslistContext : public antlr4::ParserRuleContext {
  public:
    VarargslistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VfpdefContext *> vfpdef();
    VfpdefContext* vfpdef(size_t i);
    antlr4::tree::TerminalNode *STAR();
    antlr4::tree::TerminalNode *POWER();
    std::vector<antlr4::tree::TerminalNode *> ASSIGN();
    antlr4::tree::TerminalNode* ASSIGN(size_t i);
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarargslistContext* varargslist();

  class  VfpdefContext : public antlr4::ParserRuleContext {
  public:
    VfpdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VfpdefContext* vfpdef();

  class  StmtContext : public antlr4::ParserRuleContext {
  public:
    StmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Simple_stmtsContext *simple_stmts();
    Compound_stmtContext *compound_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StmtContext* stmt();

  class  Simple_stmtsContext : public antlr4::ParserRuleContext {
  public:
    Simple_stmtsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Simple_stmtContext *> simple_stmt();
    Simple_stmtContext* simple_stmt(size_t i);
    antlr4::tree::TerminalNode *NEWLINE();
    std::vector<antlr4::tree::TerminalNode *> SEMI_COLON();
    antlr4::tree::TerminalNode* SEMI_COLON(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Simple_stmtsContext* simple_stmts();

  class  Simple_stmtContext : public antlr4::ParserRuleContext {
  public:
    Simple_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Expr_stmtContext *expr_stmt();
    Del_stmtContext *del_stmt();
    Pass_stmtContext *pass_stmt();
    Flow_stmtContext *flow_stmt();
    Import_stmtContext *import_stmt();
    Global_stmtContext *global_stmt();
    Nonlocal_stmtContext *nonlocal_stmt();
    Assert_stmtContext *assert_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Simple_stmtContext* simple_stmt();

  class  Expr_stmtContext : public antlr4::ParserRuleContext {
  public:
    Expr_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Testlist_star_exprContext *> testlist_star_expr();
    Testlist_star_exprContext* testlist_star_expr(size_t i);
    AnnassignContext *annassign();
    AugassignContext *augassign();
    std::vector<Yield_exprContext *> yield_expr();
    Yield_exprContext* yield_expr(size_t i);
    TestlistContext *testlist();
    std::vector<antlr4::tree::TerminalNode *> ASSIGN();
    antlr4::tree::TerminalNode* ASSIGN(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Expr_stmtContext* expr_stmt();

  class  AnnassignContext : public antlr4::ParserRuleContext {
  public:
    AnnassignContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    antlr4::tree::TerminalNode *ASSIGN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AnnassignContext* annassign();

  class  Testlist_star_exprContext : public antlr4::ParserRuleContext {
  public:
    Testlist_star_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<Star_exprContext *> star_expr();
    Star_exprContext* star_expr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Testlist_star_exprContext* testlist_star_expr();

  class  AugassignContext : public antlr4::ParserRuleContext {
  public:
    AugassignContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ADD_ASSIGN();
    antlr4::tree::TerminalNode *SUB_ASSIGN();
    antlr4::tree::TerminalNode *MULT_ASSIGN();
    antlr4::tree::TerminalNode *AT_ASSIGN();
    antlr4::tree::TerminalNode *DIV_ASSIGN();
    antlr4::tree::TerminalNode *MOD_ASSIGN();
    antlr4::tree::TerminalNode *AND_ASSIGN();
    antlr4::tree::TerminalNode *OR_ASSIGN();
    antlr4::tree::TerminalNode *XOR_ASSIGN();
    antlr4::tree::TerminalNode *LEFT_SHIFT_ASSIGN();
    antlr4::tree::TerminalNode *RIGHT_SHIFT_ASSIGN();
    antlr4::tree::TerminalNode *POWER_ASSIGN();
    antlr4::tree::TerminalNode *IDIV_ASSIGN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AugassignContext* augassign();

  class  Del_stmtContext : public antlr4::ParserRuleContext {
  public:
    Del_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DEL();
    ExprlistContext *exprlist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Del_stmtContext* del_stmt();

  class  Pass_stmtContext : public antlr4::ParserRuleContext {
  public:
    Pass_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PASS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pass_stmtContext* pass_stmt();

  class  Flow_stmtContext : public antlr4::ParserRuleContext {
  public:
    Flow_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Break_stmtContext *break_stmt();
    Continue_stmtContext *continue_stmt();
    Return_stmtContext *return_stmt();
    Raise_stmtContext *raise_stmt();
    Yield_stmtContext *yield_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Flow_stmtContext* flow_stmt();

  class  Break_stmtContext : public antlr4::ParserRuleContext {
  public:
    Break_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BREAK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Break_stmtContext* break_stmt();

  class  Continue_stmtContext : public antlr4::ParserRuleContext {
  public:
    Continue_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CONTINUE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Continue_stmtContext* continue_stmt();

  class  Return_stmtContext : public antlr4::ParserRuleContext {
  public:
    Return_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURN();
    TestlistContext *testlist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Return_stmtContext* return_stmt();

  class  Yield_stmtContext : public antlr4::ParserRuleContext {
  public:
    Yield_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Yield_exprContext *yield_expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Yield_stmtContext* yield_stmt();

  class  Raise_stmtContext : public antlr4::ParserRuleContext {
  public:
    Raise_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RAISE();
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    antlr4::tree::TerminalNode *FROM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Raise_stmtContext* raise_stmt();

  class  Import_stmtContext : public antlr4::ParserRuleContext {
  public:
    Import_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Import_nameContext *import_name();
    Import_fromContext *import_from();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Import_stmtContext* import_stmt();

  class  Import_nameContext : public antlr4::ParserRuleContext {
  public:
    Import_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IMPORT();
    Dotted_as_namesContext *dotted_as_names();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Import_nameContext* import_name();

  class  Import_fromContext : public antlr4::ParserRuleContext {
  public:
    Import_fromContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FROM();
    antlr4::tree::TerminalNode *IMPORT();
    Dotted_nameContext *dotted_name();
    antlr4::tree::TerminalNode *STAR();
    antlr4::tree::TerminalNode *OPEN_PAREN();
    Import_as_namesContext *import_as_names();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    std::vector<antlr4::tree::TerminalNode *> DOT();
    antlr4::tree::TerminalNode* DOT(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ELLIPSIS();
    antlr4::tree::TerminalNode* ELLIPSIS(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Import_fromContext* import_from();

  class  Import_as_nameContext : public antlr4::ParserRuleContext {
  public:
    Import_as_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    antlr4::tree::TerminalNode *AS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Import_as_nameContext* import_as_name();

  class  Dotted_as_nameContext : public antlr4::ParserRuleContext {
  public:
    Dotted_as_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Dotted_nameContext *dotted_name();
    antlr4::tree::TerminalNode *AS();
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Dotted_as_nameContext* dotted_as_name();

  class  Import_as_namesContext : public antlr4::ParserRuleContext {
  public:
    Import_as_namesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Import_as_nameContext *> import_as_name();
    Import_as_nameContext* import_as_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Import_as_namesContext* import_as_names();

  class  Dotted_as_namesContext : public antlr4::ParserRuleContext {
  public:
    Dotted_as_namesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Dotted_as_nameContext *> dotted_as_name();
    Dotted_as_nameContext* dotted_as_name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Dotted_as_namesContext* dotted_as_names();

  class  Dotted_nameContext : public antlr4::ParserRuleContext {
  public:
    Dotted_nameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DOT();
    antlr4::tree::TerminalNode* DOT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Dotted_nameContext* dotted_name();

  class  Global_stmtContext : public antlr4::ParserRuleContext {
  public:
    Global_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GLOBAL();
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Global_stmtContext* global_stmt();

  class  Nonlocal_stmtContext : public antlr4::ParserRuleContext {
  public:
    Nonlocal_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NONLOCAL();
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Nonlocal_stmtContext* nonlocal_stmt();

  class  Assert_stmtContext : public antlr4::ParserRuleContext {
  public:
    Assert_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSERT();
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Assert_stmtContext* assert_stmt();

  class  Compound_stmtContext : public antlr4::ParserRuleContext {
  public:
    Compound_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    If_stmtContext *if_stmt();
    While_stmtContext *while_stmt();
    For_stmtContext *for_stmt();
    Try_stmtContext *try_stmt();
    With_stmtContext *with_stmt();
    FuncdefContext *funcdef();
    ClassdefContext *classdef();
    DecoratedContext *decorated();
    Async_stmtContext *async_stmt();
    Match_stmtContext *match_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Compound_stmtContext* compound_stmt();

  class  Async_stmtContext : public antlr4::ParserRuleContext {
  public:
    Async_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASYNC();
    FuncdefContext *funcdef();
    With_stmtContext *with_stmt();
    For_stmtContext *for_stmt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Async_stmtContext* async_stmt();

  class  If_stmtContext : public antlr4::ParserRuleContext {
  public:
    If_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COLON();
    antlr4::tree::TerminalNode* COLON(size_t i);
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ELIF();
    antlr4::tree::TerminalNode* ELIF(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  If_stmtContext* if_stmt();

  class  While_stmtContext : public antlr4::ParserRuleContext {
  public:
    While_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WHILE();
    TestContext *test();
    std::vector<antlr4::tree::TerminalNode *> COLON();
    antlr4::tree::TerminalNode* COLON(size_t i);
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  While_stmtContext* while_stmt();

  class  For_stmtContext : public antlr4::ParserRuleContext {
  public:
    For_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOR();
    ExprlistContext *exprlist();
    antlr4::tree::TerminalNode *IN();
    TestlistContext *testlist();
    std::vector<antlr4::tree::TerminalNode *> COLON();
    antlr4::tree::TerminalNode* COLON(size_t i);
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  For_stmtContext* for_stmt();

  class  Try_stmtContext : public antlr4::ParserRuleContext {
  public:
    Try_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRY();
    std::vector<antlr4::tree::TerminalNode *> COLON();
    antlr4::tree::TerminalNode* COLON(size_t i);
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    antlr4::tree::TerminalNode *FINALLY();
    std::vector<Except_clauseContext *> except_clause();
    Except_clauseContext* except_clause(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Try_stmtContext* try_stmt();

  class  With_stmtContext : public antlr4::ParserRuleContext {
  public:
    With_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WITH();
    std::vector<With_itemContext *> with_item();
    With_itemContext* with_item(size_t i);
    antlr4::tree::TerminalNode *COLON();
    BlockContext *block();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  With_stmtContext* with_stmt();

  class  With_itemContext : public antlr4::ParserRuleContext {
  public:
    With_itemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TestContext *test();
    antlr4::tree::TerminalNode *AS();
    ExprContext *expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  With_itemContext* with_item();

  class  Except_clauseContext : public antlr4::ParserRuleContext {
  public:
    Except_clauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EXCEPT();
    TestContext *test();
    antlr4::tree::TerminalNode *AS();
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Except_clauseContext* except_clause();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Simple_stmtsContext *simple_stmts();
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<StmtContext *> stmt();
    StmtContext* stmt(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  Match_stmtContext : public antlr4::ParserRuleContext {
  public:
    Match_stmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MATCH();
    Subject_exprContext *subject_expr();
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<Case_blockContext *> case_block();
    Case_blockContext* case_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Match_stmtContext* match_stmt();

  class  Subject_exprContext : public antlr4::ParserRuleContext {
  public:
    Subject_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Star_named_expressionContext *star_named_expression();
    antlr4::tree::TerminalNode *COMMA();
    Star_named_expressionsContext *star_named_expressions();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Subject_exprContext* subject_expr();

  class  Star_named_expressionsContext : public antlr4::ParserRuleContext {
  public:
    Star_named_expressionsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    std::vector<Star_named_expressionContext *> star_named_expression();
    Star_named_expressionContext* star_named_expression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Star_named_expressionsContext* star_named_expressions();

  class  Star_named_expressionContext : public antlr4::ParserRuleContext {
  public:
    Star_named_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STAR();
    ExprContext *expr();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Star_named_expressionContext* star_named_expression();

  class  Case_blockContext : public antlr4::ParserRuleContext {
  public:
    Case_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CASE();
    PatternsContext *patterns();
    antlr4::tree::TerminalNode *COLON();
    BlockContext *block();
    GuardContext *guard();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Case_blockContext* case_block();

  class  GuardContext : public antlr4::ParserRuleContext {
  public:
    GuardContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GuardContext* guard();

  class  PatternsContext : public antlr4::ParserRuleContext {
  public:
    PatternsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Open_sequence_patternContext *open_sequence_pattern();
    PatternContext *pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternsContext* patterns();

  class  PatternContext : public antlr4::ParserRuleContext {
  public:
    PatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    As_patternContext *as_pattern();
    Or_patternContext *or_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternContext* pattern();

  class  As_patternContext : public antlr4::ParserRuleContext {
  public:
    As_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Or_patternContext *or_pattern();
    antlr4::tree::TerminalNode *AS();
    Pattern_capture_targetContext *pattern_capture_target();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  As_patternContext* as_pattern();

  class  Or_patternContext : public antlr4::ParserRuleContext {
  public:
    Or_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Closed_patternContext *> closed_pattern();
    Closed_patternContext* closed_pattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> OR_OP();
    antlr4::tree::TerminalNode* OR_OP(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Or_patternContext* or_pattern();

  class  Closed_patternContext : public antlr4::ParserRuleContext {
  public:
    Closed_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Literal_patternContext *literal_pattern();
    Capture_patternContext *capture_pattern();
    Wildcard_patternContext *wildcard_pattern();
    Value_patternContext *value_pattern();
    Group_patternContext *group_pattern();
    Sequence_patternContext *sequence_pattern();
    Mapping_patternContext *mapping_pattern();
    Class_patternContext *class_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Closed_patternContext* closed_pattern();

  class  Literal_patternContext : public antlr4::ParserRuleContext {
  public:
    Literal_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Signed_numberContext *signed_number();
    Complex_numberContext *complex_number();
    StringsContext *strings();
    antlr4::tree::TerminalNode *NONE();
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Literal_patternContext* literal_pattern();

  class  Literal_exprContext : public antlr4::ParserRuleContext {
  public:
    Literal_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Signed_numberContext *signed_number();
    Complex_numberContext *complex_number();
    StringsContext *strings();
    antlr4::tree::TerminalNode *NONE();
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Literal_exprContext* literal_expr();

  class  Complex_numberContext : public antlr4::ParserRuleContext {
  public:
    Complex_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Signed_real_numberContext *signed_real_number();
    antlr4::tree::TerminalNode *ADD();
    Imaginary_numberContext *imaginary_number();
    antlr4::tree::TerminalNode *MINUS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Complex_numberContext* complex_number();

  class  Signed_numberContext : public antlr4::ParserRuleContext {
  public:
    Signed_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *MINUS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Signed_numberContext* signed_number();

  class  Signed_real_numberContext : public antlr4::ParserRuleContext {
  public:
    Signed_real_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Real_numberContext *real_number();
    antlr4::tree::TerminalNode *MINUS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Signed_real_numberContext* signed_real_number();

  class  Real_numberContext : public antlr4::ParserRuleContext {
  public:
    Real_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Real_numberContext* real_number();

  class  Imaginary_numberContext : public antlr4::ParserRuleContext {
  public:
    Imaginary_numberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Imaginary_numberContext* imaginary_number();

  class  Capture_patternContext : public antlr4::ParserRuleContext {
  public:
    Capture_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Pattern_capture_targetContext *pattern_capture_target();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Capture_patternContext* capture_pattern();

  class  Pattern_capture_targetContext : public antlr4::ParserRuleContext {
  public:
    Pattern_capture_targetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pattern_capture_targetContext* pattern_capture_target();

  class  Wildcard_patternContext : public antlr4::ParserRuleContext {
  public:
    Wildcard_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNDERSCORE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Wildcard_patternContext* wildcard_pattern();

  class  Value_patternContext : public antlr4::ParserRuleContext {
  public:
    Value_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttrContext *attr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Value_patternContext* value_pattern();

  class  AttrContext : public antlr4::ParserRuleContext {
  public:
    AttrContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NameContext *> name();
    NameContext* name(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DOT();
    antlr4::tree::TerminalNode* DOT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttrContext* attr();

  class  Name_or_attrContext : public antlr4::ParserRuleContext {
  public:
    Name_or_attrContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttrContext *attr();
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Name_or_attrContext* name_or_attr();

  class  Group_patternContext : public antlr4::ParserRuleContext {
  public:
    Group_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAREN();
    PatternContext *pattern();
    antlr4::tree::TerminalNode *CLOSE_PAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Group_patternContext* group_pattern();

  class  Sequence_patternContext : public antlr4::ParserRuleContext {
  public:
    Sequence_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_BRACK();
    antlr4::tree::TerminalNode *CLOSE_BRACK();
    Maybe_sequence_patternContext *maybe_sequence_pattern();
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    Open_sequence_patternContext *open_sequence_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Sequence_patternContext* sequence_pattern();

  class  Open_sequence_patternContext : public antlr4::ParserRuleContext {
  public:
    Open_sequence_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Maybe_star_patternContext *maybe_star_pattern();
    antlr4::tree::TerminalNode *COMMA();
    Maybe_sequence_patternContext *maybe_sequence_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Open_sequence_patternContext* open_sequence_pattern();

  class  Maybe_sequence_patternContext : public antlr4::ParserRuleContext {
  public:
    Maybe_sequence_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Maybe_star_patternContext *> maybe_star_pattern();
    Maybe_star_patternContext* maybe_star_pattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Maybe_sequence_patternContext* maybe_sequence_pattern();

  class  Maybe_star_patternContext : public antlr4::ParserRuleContext {
  public:
    Maybe_star_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Star_patternContext *star_pattern();
    PatternContext *pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Maybe_star_patternContext* maybe_star_pattern();

  class  Star_patternContext : public antlr4::ParserRuleContext {
  public:
    Star_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STAR();
    Pattern_capture_targetContext *pattern_capture_target();
    Wildcard_patternContext *wildcard_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Star_patternContext* star_pattern();

  class  Mapping_patternContext : public antlr4::ParserRuleContext {
  public:
    Mapping_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_BRACE();
    antlr4::tree::TerminalNode *CLOSE_BRACE();
    Double_star_patternContext *double_star_pattern();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    Items_patternContext *items_pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Mapping_patternContext* mapping_pattern();

  class  Items_patternContext : public antlr4::ParserRuleContext {
  public:
    Items_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Key_value_patternContext *> key_value_pattern();
    Key_value_patternContext* key_value_pattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Items_patternContext* items_pattern();

  class  Key_value_patternContext : public antlr4::ParserRuleContext {
  public:
    Key_value_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    PatternContext *pattern();
    Literal_exprContext *literal_expr();
    AttrContext *attr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Key_value_patternContext* key_value_pattern();

  class  Double_star_patternContext : public antlr4::ParserRuleContext {
  public:
    Double_star_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *POWER();
    Pattern_capture_targetContext *pattern_capture_target();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Double_star_patternContext* double_star_pattern();

  class  Class_patternContext : public antlr4::ParserRuleContext {
  public:
    Class_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_or_attrContext *name_or_attr();
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    Positional_patternsContext *positional_patterns();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);
    Keyword_patternsContext *keyword_patterns();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Class_patternContext* class_pattern();

  class  Positional_patternsContext : public antlr4::ParserRuleContext {
  public:
    Positional_patternsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PatternContext *> pattern();
    PatternContext* pattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Positional_patternsContext* positional_patterns();

  class  Keyword_patternsContext : public antlr4::ParserRuleContext {
  public:
    Keyword_patternsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Keyword_patternContext *> keyword_pattern();
    Keyword_patternContext* keyword_pattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Keyword_patternsContext* keyword_patterns();

  class  Keyword_patternContext : public antlr4::ParserRuleContext {
  public:
    Keyword_patternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    antlr4::tree::TerminalNode *ASSIGN();
    PatternContext *pattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Keyword_patternContext* keyword_pattern();

  class  TestContext : public antlr4::ParserRuleContext {
  public:
    TestContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Or_testContext *> or_test();
    Or_testContext* or_test(size_t i);
    antlr4::tree::TerminalNode *IF();
    antlr4::tree::TerminalNode *ELSE();
    TestContext *test();
    LambdefContext *lambdef();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TestContext* test();

  class  Test_nocondContext : public antlr4::ParserRuleContext {
  public:
    Test_nocondContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Or_testContext *or_test();
    Lambdef_nocondContext *lambdef_nocond();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Test_nocondContext* test_nocond();

  class  LambdefContext : public antlr4::ParserRuleContext {
  public:
    LambdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LAMBDA();
    antlr4::tree::TerminalNode *COLON();
    TestContext *test();
    VarargslistContext *varargslist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LambdefContext* lambdef();

  class  Lambdef_nocondContext : public antlr4::ParserRuleContext {
  public:
    Lambdef_nocondContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LAMBDA();
    antlr4::tree::TerminalNode *COLON();
    Test_nocondContext *test_nocond();
    VarargslistContext *varargslist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Lambdef_nocondContext* lambdef_nocond();

  class  Or_testContext : public antlr4::ParserRuleContext {
  public:
    Or_testContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<And_testContext *> and_test();
    And_testContext* and_test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> OR();
    antlr4::tree::TerminalNode* OR(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Or_testContext* or_test();

  class  And_testContext : public antlr4::ParserRuleContext {
  public:
    And_testContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Not_testContext *> not_test();
    Not_testContext* not_test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> AND();
    antlr4::tree::TerminalNode* AND(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  And_testContext* and_test();

  class  Not_testContext : public antlr4::ParserRuleContext {
  public:
    Not_testContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NOT();
    Not_testContext *not_test();
    ComparisonContext *comparison();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Not_testContext* not_test();

  class  ComparisonContext : public antlr4::ParserRuleContext {
  public:
    ComparisonContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<Comp_opContext *> comp_op();
    Comp_opContext* comp_op(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComparisonContext* comparison();

  class  Comp_opContext : public antlr4::ParserRuleContext {
  public:
    Comp_opContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LESS_THAN();
    antlr4::tree::TerminalNode *GREATER_THAN();
    antlr4::tree::TerminalNode *EQUALS();
    antlr4::tree::TerminalNode *GT_EQ();
    antlr4::tree::TerminalNode *LT_EQ();
    antlr4::tree::TerminalNode *NOT_EQ_1();
    antlr4::tree::TerminalNode *NOT_EQ_2();
    antlr4::tree::TerminalNode *IN();
    antlr4::tree::TerminalNode *NOT();
    antlr4::tree::TerminalNode *IS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Comp_opContext* comp_op();

  class  Star_exprContext : public antlr4::ParserRuleContext {
  public:
    Star_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STAR();
    ExprContext *expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Star_exprContext* star_expr();

  class  ExprContext : public antlr4::ParserRuleContext {
  public:
    ExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Atom_exprContext *atom_expr();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ADD();
    antlr4::tree::TerminalNode* ADD(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MINUS();
    antlr4::tree::TerminalNode* MINUS(size_t i);
    std::vector<antlr4::tree::TerminalNode *> NOT_OP();
    antlr4::tree::TerminalNode* NOT_OP(size_t i);
    antlr4::tree::TerminalNode *POWER();
    antlr4::tree::TerminalNode *STAR();
    antlr4::tree::TerminalNode *AT();
    antlr4::tree::TerminalNode *DIV();
    antlr4::tree::TerminalNode *MOD();
    antlr4::tree::TerminalNode *IDIV();
    antlr4::tree::TerminalNode *LEFT_SHIFT();
    antlr4::tree::TerminalNode *RIGHT_SHIFT();
    antlr4::tree::TerminalNode *AND_OP();
    antlr4::tree::TerminalNode *XOR();
    antlr4::tree::TerminalNode *OR_OP();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExprContext* expr();
  ExprContext* expr(int precedence);
  class  Atom_exprContext : public antlr4::ParserRuleContext {
  public:
    Atom_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AtomContext *atom();
    antlr4::tree::TerminalNode *AWAIT();
    std::vector<TrailerContext *> trailer();
    TrailerContext* trailer(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Atom_exprContext* atom_expr();

  class  AtomContext : public antlr4::ParserRuleContext {
  public:
    AtomContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    Yield_exprContext *yield_expr();
    Testlist_compContext *testlist_comp();
    antlr4::tree::TerminalNode *OPEN_BRACK();
    antlr4::tree::TerminalNode *CLOSE_BRACK();
    antlr4::tree::TerminalNode *OPEN_BRACE();
    antlr4::tree::TerminalNode *CLOSE_BRACE();
    DictorsetmakerContext *dictorsetmaker();
    NameContext *name();
    antlr4::tree::TerminalNode *NUMBER();
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);
    antlr4::tree::TerminalNode *ELLIPSIS();
    antlr4::tree::TerminalNode *NONE();
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AtomContext* atom();

  class  NameContext : public antlr4::ParserRuleContext {
  public:
    NameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();
    antlr4::tree::TerminalNode *UNDERSCORE();
    antlr4::tree::TerminalNode *MATCH();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NameContext* name();

  class  Testlist_compContext : public antlr4::ParserRuleContext {
  public:
    Testlist_compContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<Star_exprContext *> star_expr();
    Star_exprContext* star_expr(size_t i);
    Comp_forContext *comp_for();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Testlist_compContext* testlist_comp();

  class  TrailerContext : public antlr4::ParserRuleContext {
  public:
    TrailerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    ArglistContext *arglist();
    antlr4::tree::TerminalNode *OPEN_BRACK();
    SubscriptlistContext *subscriptlist();
    antlr4::tree::TerminalNode *CLOSE_BRACK();
    antlr4::tree::TerminalNode *DOT();
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TrailerContext* trailer();

  class  SubscriptlistContext : public antlr4::ParserRuleContext {
  public:
    SubscriptlistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Subscript_Context *> subscript_();
    Subscript_Context* subscript_(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SubscriptlistContext* subscriptlist();

  class  Subscript_Context : public antlr4::ParserRuleContext {
  public:
    Subscript_Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    antlr4::tree::TerminalNode *COLON();
    SliceopContext *sliceop();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Subscript_Context* subscript_();

  class  SliceopContext : public antlr4::ParserRuleContext {
  public:
    SliceopContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    TestContext *test();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SliceopContext* sliceop();

  class  ExprlistContext : public antlr4::ParserRuleContext {
  public:
    ExprlistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    std::vector<Star_exprContext *> star_expr();
    Star_exprContext* star_expr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExprlistContext* exprlist();

  class  TestlistContext : public antlr4::ParserRuleContext {
  public:
    TestlistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TestlistContext* testlist();

  class  DictorsetmakerContext : public antlr4::ParserRuleContext {
  public:
    DictorsetmakerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COLON();
    antlr4::tree::TerminalNode* COLON(size_t i);
    std::vector<antlr4::tree::TerminalNode *> POWER();
    antlr4::tree::TerminalNode* POWER(size_t i);
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    Comp_forContext *comp_for();
    std::vector<Star_exprContext *> star_expr();
    Star_exprContext* star_expr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictorsetmakerContext* dictorsetmaker();

  class  ClassdefContext : public antlr4::ParserRuleContext {
  public:
    ClassdefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CLASS();
    NameContext *name();
    antlr4::tree::TerminalNode *COLON();
    BlockContext *block();
    antlr4::tree::TerminalNode *OPEN_PAREN();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    ArglistContext *arglist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassdefContext* classdef();

  class  ArglistContext : public antlr4::ParserRuleContext {
  public:
    ArglistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ArgumentContext *> argument();
    ArgumentContext* argument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArglistContext* arglist();

  class  ArgumentContext : public antlr4::ParserRuleContext {
  public:
    ArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TestContext *> test();
    TestContext* test(size_t i);
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *POWER();
    antlr4::tree::TerminalNode *STAR();
    Comp_forContext *comp_for();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgumentContext* argument();

  class  Comp_iterContext : public antlr4::ParserRuleContext {
  public:
    Comp_iterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Comp_forContext *comp_for();
    Comp_ifContext *comp_if();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Comp_iterContext* comp_iter();

  class  Comp_forContext : public antlr4::ParserRuleContext {
  public:
    Comp_forContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOR();
    ExprlistContext *exprlist();
    antlr4::tree::TerminalNode *IN();
    Or_testContext *or_test();
    antlr4::tree::TerminalNode *ASYNC();
    Comp_iterContext *comp_iter();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Comp_forContext* comp_for();

  class  Comp_ifContext : public antlr4::ParserRuleContext {
  public:
    Comp_ifContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    Test_nocondContext *test_nocond();
    Comp_iterContext *comp_iter();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Comp_ifContext* comp_if();

  class  Encoding_declContext : public antlr4::ParserRuleContext {
  public:
    Encoding_declContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Encoding_declContext* encoding_decl();

  class  Yield_exprContext : public antlr4::ParserRuleContext {
  public:
    Yield_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YIELD();
    Yield_argContext *yield_arg();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Yield_exprContext* yield_expr();

  class  Yield_argContext : public antlr4::ParserRuleContext {
  public:
    Yield_argContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FROM();
    TestContext *test();
    TestlistContext *testlist();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Yield_argContext* yield_arg();

  class  StringsContext : public antlr4::ParserRuleContext {
  public:
    StringsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StringsContext* strings();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool literal_patternSempred(Literal_patternContext *_localctx, size_t predicateIndex);
  bool literal_exprSempred(Literal_exprContext *_localctx, size_t predicateIndex);
  bool pattern_capture_targetSempred(Pattern_capture_targetContext *_localctx, size_t predicateIndex);
  bool value_patternSempred(Value_patternContext *_localctx, size_t predicateIndex);
  bool exprSempred(ExprContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

