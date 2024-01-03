#include "Python3ParserBase.h"

// Generated from Python3Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "Python3ParserListener.h"


/**
 * This class provides an empty implementation of Python3ParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  Python3ParserBaseListener : public Python3ParserListener {
public:

  virtual void enterSingle_input(Python3Parser::Single_inputContext * /*ctx*/) override { }
  virtual void exitSingle_input(Python3Parser::Single_inputContext * /*ctx*/) override { }

  virtual void enterFile_input(Python3Parser::File_inputContext * /*ctx*/) override { }
  virtual void exitFile_input(Python3Parser::File_inputContext * /*ctx*/) override { }

  virtual void enterEval_input(Python3Parser::Eval_inputContext * /*ctx*/) override { }
  virtual void exitEval_input(Python3Parser::Eval_inputContext * /*ctx*/) override { }

  virtual void enterDecorator(Python3Parser::DecoratorContext * /*ctx*/) override { }
  virtual void exitDecorator(Python3Parser::DecoratorContext * /*ctx*/) override { }

  virtual void enterDecorators(Python3Parser::DecoratorsContext * /*ctx*/) override { }
  virtual void exitDecorators(Python3Parser::DecoratorsContext * /*ctx*/) override { }

  virtual void enterDecorated(Python3Parser::DecoratedContext * /*ctx*/) override { }
  virtual void exitDecorated(Python3Parser::DecoratedContext * /*ctx*/) override { }

  virtual void enterAsync_funcdef(Python3Parser::Async_funcdefContext * /*ctx*/) override { }
  virtual void exitAsync_funcdef(Python3Parser::Async_funcdefContext * /*ctx*/) override { }

  virtual void enterFuncdef(Python3Parser::FuncdefContext * /*ctx*/) override { }
  virtual void exitFuncdef(Python3Parser::FuncdefContext * /*ctx*/) override { }

  virtual void enterParameters(Python3Parser::ParametersContext * /*ctx*/) override { }
  virtual void exitParameters(Python3Parser::ParametersContext * /*ctx*/) override { }

  virtual void enterTypedargslist(Python3Parser::TypedargslistContext * /*ctx*/) override { }
  virtual void exitTypedargslist(Python3Parser::TypedargslistContext * /*ctx*/) override { }

  virtual void enterTfpdef(Python3Parser::TfpdefContext * /*ctx*/) override { }
  virtual void exitTfpdef(Python3Parser::TfpdefContext * /*ctx*/) override { }

  virtual void enterVarargslist(Python3Parser::VarargslistContext * /*ctx*/) override { }
  virtual void exitVarargslist(Python3Parser::VarargslistContext * /*ctx*/) override { }

  virtual void enterVfpdef(Python3Parser::VfpdefContext * /*ctx*/) override { }
  virtual void exitVfpdef(Python3Parser::VfpdefContext * /*ctx*/) override { }

  virtual void enterStmt(Python3Parser::StmtContext * /*ctx*/) override { }
  virtual void exitStmt(Python3Parser::StmtContext * /*ctx*/) override { }

  virtual void enterSimple_stmts(Python3Parser::Simple_stmtsContext * /*ctx*/) override { }
  virtual void exitSimple_stmts(Python3Parser::Simple_stmtsContext * /*ctx*/) override { }

  virtual void enterSimple_stmt(Python3Parser::Simple_stmtContext * /*ctx*/) override { }
  virtual void exitSimple_stmt(Python3Parser::Simple_stmtContext * /*ctx*/) override { }

  virtual void enterExpr_stmt(Python3Parser::Expr_stmtContext * /*ctx*/) override { }
  virtual void exitExpr_stmt(Python3Parser::Expr_stmtContext * /*ctx*/) override { }

  virtual void enterAnnassign(Python3Parser::AnnassignContext * /*ctx*/) override { }
  virtual void exitAnnassign(Python3Parser::AnnassignContext * /*ctx*/) override { }

  virtual void enterTestlist_star_expr(Python3Parser::Testlist_star_exprContext * /*ctx*/) override { }
  virtual void exitTestlist_star_expr(Python3Parser::Testlist_star_exprContext * /*ctx*/) override { }

  virtual void enterAugassign(Python3Parser::AugassignContext * /*ctx*/) override { }
  virtual void exitAugassign(Python3Parser::AugassignContext * /*ctx*/) override { }

  virtual void enterDel_stmt(Python3Parser::Del_stmtContext * /*ctx*/) override { }
  virtual void exitDel_stmt(Python3Parser::Del_stmtContext * /*ctx*/) override { }

  virtual void enterPass_stmt(Python3Parser::Pass_stmtContext * /*ctx*/) override { }
  virtual void exitPass_stmt(Python3Parser::Pass_stmtContext * /*ctx*/) override { }

  virtual void enterFlow_stmt(Python3Parser::Flow_stmtContext * /*ctx*/) override { }
  virtual void exitFlow_stmt(Python3Parser::Flow_stmtContext * /*ctx*/) override { }

  virtual void enterBreak_stmt(Python3Parser::Break_stmtContext * /*ctx*/) override { }
  virtual void exitBreak_stmt(Python3Parser::Break_stmtContext * /*ctx*/) override { }

  virtual void enterContinue_stmt(Python3Parser::Continue_stmtContext * /*ctx*/) override { }
  virtual void exitContinue_stmt(Python3Parser::Continue_stmtContext * /*ctx*/) override { }

  virtual void enterReturn_stmt(Python3Parser::Return_stmtContext * /*ctx*/) override { }
  virtual void exitReturn_stmt(Python3Parser::Return_stmtContext * /*ctx*/) override { }

  virtual void enterYield_stmt(Python3Parser::Yield_stmtContext * /*ctx*/) override { }
  virtual void exitYield_stmt(Python3Parser::Yield_stmtContext * /*ctx*/) override { }

  virtual void enterRaise_stmt(Python3Parser::Raise_stmtContext * /*ctx*/) override { }
  virtual void exitRaise_stmt(Python3Parser::Raise_stmtContext * /*ctx*/) override { }

  virtual void enterImport_stmt(Python3Parser::Import_stmtContext * /*ctx*/) override { }
  virtual void exitImport_stmt(Python3Parser::Import_stmtContext * /*ctx*/) override { }

  virtual void enterImport_name(Python3Parser::Import_nameContext * /*ctx*/) override { }
  virtual void exitImport_name(Python3Parser::Import_nameContext * /*ctx*/) override { }

  virtual void enterImport_from(Python3Parser::Import_fromContext * /*ctx*/) override { }
  virtual void exitImport_from(Python3Parser::Import_fromContext * /*ctx*/) override { }

  virtual void enterImport_as_name(Python3Parser::Import_as_nameContext * /*ctx*/) override { }
  virtual void exitImport_as_name(Python3Parser::Import_as_nameContext * /*ctx*/) override { }

  virtual void enterDotted_as_name(Python3Parser::Dotted_as_nameContext * /*ctx*/) override { }
  virtual void exitDotted_as_name(Python3Parser::Dotted_as_nameContext * /*ctx*/) override { }

  virtual void enterImport_as_names(Python3Parser::Import_as_namesContext * /*ctx*/) override { }
  virtual void exitImport_as_names(Python3Parser::Import_as_namesContext * /*ctx*/) override { }

  virtual void enterDotted_as_names(Python3Parser::Dotted_as_namesContext * /*ctx*/) override { }
  virtual void exitDotted_as_names(Python3Parser::Dotted_as_namesContext * /*ctx*/) override { }

  virtual void enterDotted_name(Python3Parser::Dotted_nameContext * /*ctx*/) override { }
  virtual void exitDotted_name(Python3Parser::Dotted_nameContext * /*ctx*/) override { }

  virtual void enterGlobal_stmt(Python3Parser::Global_stmtContext * /*ctx*/) override { }
  virtual void exitGlobal_stmt(Python3Parser::Global_stmtContext * /*ctx*/) override { }

  virtual void enterNonlocal_stmt(Python3Parser::Nonlocal_stmtContext * /*ctx*/) override { }
  virtual void exitNonlocal_stmt(Python3Parser::Nonlocal_stmtContext * /*ctx*/) override { }

  virtual void enterAssert_stmt(Python3Parser::Assert_stmtContext * /*ctx*/) override { }
  virtual void exitAssert_stmt(Python3Parser::Assert_stmtContext * /*ctx*/) override { }

  virtual void enterCompound_stmt(Python3Parser::Compound_stmtContext * /*ctx*/) override { }
  virtual void exitCompound_stmt(Python3Parser::Compound_stmtContext * /*ctx*/) override { }

  virtual void enterAsync_stmt(Python3Parser::Async_stmtContext * /*ctx*/) override { }
  virtual void exitAsync_stmt(Python3Parser::Async_stmtContext * /*ctx*/) override { }

  virtual void enterIf_stmt(Python3Parser::If_stmtContext * /*ctx*/) override { }
  virtual void exitIf_stmt(Python3Parser::If_stmtContext * /*ctx*/) override { }

  virtual void enterWhile_stmt(Python3Parser::While_stmtContext * /*ctx*/) override { }
  virtual void exitWhile_stmt(Python3Parser::While_stmtContext * /*ctx*/) override { }

  virtual void enterFor_stmt(Python3Parser::For_stmtContext * /*ctx*/) override { }
  virtual void exitFor_stmt(Python3Parser::For_stmtContext * /*ctx*/) override { }

  virtual void enterTry_stmt(Python3Parser::Try_stmtContext * /*ctx*/) override { }
  virtual void exitTry_stmt(Python3Parser::Try_stmtContext * /*ctx*/) override { }

  virtual void enterWith_stmt(Python3Parser::With_stmtContext * /*ctx*/) override { }
  virtual void exitWith_stmt(Python3Parser::With_stmtContext * /*ctx*/) override { }

  virtual void enterWith_item(Python3Parser::With_itemContext * /*ctx*/) override { }
  virtual void exitWith_item(Python3Parser::With_itemContext * /*ctx*/) override { }

  virtual void enterExcept_clause(Python3Parser::Except_clauseContext * /*ctx*/) override { }
  virtual void exitExcept_clause(Python3Parser::Except_clauseContext * /*ctx*/) override { }

  virtual void enterBlock(Python3Parser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(Python3Parser::BlockContext * /*ctx*/) override { }

  virtual void enterMatch_stmt(Python3Parser::Match_stmtContext * /*ctx*/) override { }
  virtual void exitMatch_stmt(Python3Parser::Match_stmtContext * /*ctx*/) override { }

  virtual void enterSubject_expr(Python3Parser::Subject_exprContext * /*ctx*/) override { }
  virtual void exitSubject_expr(Python3Parser::Subject_exprContext * /*ctx*/) override { }

  virtual void enterStar_named_expressions(Python3Parser::Star_named_expressionsContext * /*ctx*/) override { }
  virtual void exitStar_named_expressions(Python3Parser::Star_named_expressionsContext * /*ctx*/) override { }

  virtual void enterStar_named_expression(Python3Parser::Star_named_expressionContext * /*ctx*/) override { }
  virtual void exitStar_named_expression(Python3Parser::Star_named_expressionContext * /*ctx*/) override { }

  virtual void enterCase_block(Python3Parser::Case_blockContext * /*ctx*/) override { }
  virtual void exitCase_block(Python3Parser::Case_blockContext * /*ctx*/) override { }

  virtual void enterGuard(Python3Parser::GuardContext * /*ctx*/) override { }
  virtual void exitGuard(Python3Parser::GuardContext * /*ctx*/) override { }

  virtual void enterPatterns(Python3Parser::PatternsContext * /*ctx*/) override { }
  virtual void exitPatterns(Python3Parser::PatternsContext * /*ctx*/) override { }

  virtual void enterPattern(Python3Parser::PatternContext * /*ctx*/) override { }
  virtual void exitPattern(Python3Parser::PatternContext * /*ctx*/) override { }

  virtual void enterAs_pattern(Python3Parser::As_patternContext * /*ctx*/) override { }
  virtual void exitAs_pattern(Python3Parser::As_patternContext * /*ctx*/) override { }

  virtual void enterOr_pattern(Python3Parser::Or_patternContext * /*ctx*/) override { }
  virtual void exitOr_pattern(Python3Parser::Or_patternContext * /*ctx*/) override { }

  virtual void enterClosed_pattern(Python3Parser::Closed_patternContext * /*ctx*/) override { }
  virtual void exitClosed_pattern(Python3Parser::Closed_patternContext * /*ctx*/) override { }

  virtual void enterLiteral_pattern(Python3Parser::Literal_patternContext * /*ctx*/) override { }
  virtual void exitLiteral_pattern(Python3Parser::Literal_patternContext * /*ctx*/) override { }

  virtual void enterLiteral_expr(Python3Parser::Literal_exprContext * /*ctx*/) override { }
  virtual void exitLiteral_expr(Python3Parser::Literal_exprContext * /*ctx*/) override { }

  virtual void enterComplex_number(Python3Parser::Complex_numberContext * /*ctx*/) override { }
  virtual void exitComplex_number(Python3Parser::Complex_numberContext * /*ctx*/) override { }

  virtual void enterSigned_number(Python3Parser::Signed_numberContext * /*ctx*/) override { }
  virtual void exitSigned_number(Python3Parser::Signed_numberContext * /*ctx*/) override { }

  virtual void enterSigned_real_number(Python3Parser::Signed_real_numberContext * /*ctx*/) override { }
  virtual void exitSigned_real_number(Python3Parser::Signed_real_numberContext * /*ctx*/) override { }

  virtual void enterReal_number(Python3Parser::Real_numberContext * /*ctx*/) override { }
  virtual void exitReal_number(Python3Parser::Real_numberContext * /*ctx*/) override { }

  virtual void enterImaginary_number(Python3Parser::Imaginary_numberContext * /*ctx*/) override { }
  virtual void exitImaginary_number(Python3Parser::Imaginary_numberContext * /*ctx*/) override { }

  virtual void enterCapture_pattern(Python3Parser::Capture_patternContext * /*ctx*/) override { }
  virtual void exitCapture_pattern(Python3Parser::Capture_patternContext * /*ctx*/) override { }

  virtual void enterPattern_capture_target(Python3Parser::Pattern_capture_targetContext * /*ctx*/) override { }
  virtual void exitPattern_capture_target(Python3Parser::Pattern_capture_targetContext * /*ctx*/) override { }

  virtual void enterWildcard_pattern(Python3Parser::Wildcard_patternContext * /*ctx*/) override { }
  virtual void exitWildcard_pattern(Python3Parser::Wildcard_patternContext * /*ctx*/) override { }

  virtual void enterValue_pattern(Python3Parser::Value_patternContext * /*ctx*/) override { }
  virtual void exitValue_pattern(Python3Parser::Value_patternContext * /*ctx*/) override { }

  virtual void enterAttr(Python3Parser::AttrContext * /*ctx*/) override { }
  virtual void exitAttr(Python3Parser::AttrContext * /*ctx*/) override { }

  virtual void enterName_or_attr(Python3Parser::Name_or_attrContext * /*ctx*/) override { }
  virtual void exitName_or_attr(Python3Parser::Name_or_attrContext * /*ctx*/) override { }

  virtual void enterGroup_pattern(Python3Parser::Group_patternContext * /*ctx*/) override { }
  virtual void exitGroup_pattern(Python3Parser::Group_patternContext * /*ctx*/) override { }

  virtual void enterSequence_pattern(Python3Parser::Sequence_patternContext * /*ctx*/) override { }
  virtual void exitSequence_pattern(Python3Parser::Sequence_patternContext * /*ctx*/) override { }

  virtual void enterOpen_sequence_pattern(Python3Parser::Open_sequence_patternContext * /*ctx*/) override { }
  virtual void exitOpen_sequence_pattern(Python3Parser::Open_sequence_patternContext * /*ctx*/) override { }

  virtual void enterMaybe_sequence_pattern(Python3Parser::Maybe_sequence_patternContext * /*ctx*/) override { }
  virtual void exitMaybe_sequence_pattern(Python3Parser::Maybe_sequence_patternContext * /*ctx*/) override { }

  virtual void enterMaybe_star_pattern(Python3Parser::Maybe_star_patternContext * /*ctx*/) override { }
  virtual void exitMaybe_star_pattern(Python3Parser::Maybe_star_patternContext * /*ctx*/) override { }

  virtual void enterStar_pattern(Python3Parser::Star_patternContext * /*ctx*/) override { }
  virtual void exitStar_pattern(Python3Parser::Star_patternContext * /*ctx*/) override { }

  virtual void enterMapping_pattern(Python3Parser::Mapping_patternContext * /*ctx*/) override { }
  virtual void exitMapping_pattern(Python3Parser::Mapping_patternContext * /*ctx*/) override { }

  virtual void enterItems_pattern(Python3Parser::Items_patternContext * /*ctx*/) override { }
  virtual void exitItems_pattern(Python3Parser::Items_patternContext * /*ctx*/) override { }

  virtual void enterKey_value_pattern(Python3Parser::Key_value_patternContext * /*ctx*/) override { }
  virtual void exitKey_value_pattern(Python3Parser::Key_value_patternContext * /*ctx*/) override { }

  virtual void enterDouble_star_pattern(Python3Parser::Double_star_patternContext * /*ctx*/) override { }
  virtual void exitDouble_star_pattern(Python3Parser::Double_star_patternContext * /*ctx*/) override { }

  virtual void enterClass_pattern(Python3Parser::Class_patternContext * /*ctx*/) override { }
  virtual void exitClass_pattern(Python3Parser::Class_patternContext * /*ctx*/) override { }

  virtual void enterPositional_patterns(Python3Parser::Positional_patternsContext * /*ctx*/) override { }
  virtual void exitPositional_patterns(Python3Parser::Positional_patternsContext * /*ctx*/) override { }

  virtual void enterKeyword_patterns(Python3Parser::Keyword_patternsContext * /*ctx*/) override { }
  virtual void exitKeyword_patterns(Python3Parser::Keyword_patternsContext * /*ctx*/) override { }

  virtual void enterKeyword_pattern(Python3Parser::Keyword_patternContext * /*ctx*/) override { }
  virtual void exitKeyword_pattern(Python3Parser::Keyword_patternContext * /*ctx*/) override { }

  virtual void enterTest(Python3Parser::TestContext * /*ctx*/) override { }
  virtual void exitTest(Python3Parser::TestContext * /*ctx*/) override { }

  virtual void enterTest_nocond(Python3Parser::Test_nocondContext * /*ctx*/) override { }
  virtual void exitTest_nocond(Python3Parser::Test_nocondContext * /*ctx*/) override { }

  virtual void enterLambdef(Python3Parser::LambdefContext * /*ctx*/) override { }
  virtual void exitLambdef(Python3Parser::LambdefContext * /*ctx*/) override { }

  virtual void enterLambdef_nocond(Python3Parser::Lambdef_nocondContext * /*ctx*/) override { }
  virtual void exitLambdef_nocond(Python3Parser::Lambdef_nocondContext * /*ctx*/) override { }

  virtual void enterOr_test(Python3Parser::Or_testContext * /*ctx*/) override { }
  virtual void exitOr_test(Python3Parser::Or_testContext * /*ctx*/) override { }

  virtual void enterAnd_test(Python3Parser::And_testContext * /*ctx*/) override { }
  virtual void exitAnd_test(Python3Parser::And_testContext * /*ctx*/) override { }

  virtual void enterNot_test(Python3Parser::Not_testContext * /*ctx*/) override { }
  virtual void exitNot_test(Python3Parser::Not_testContext * /*ctx*/) override { }

  virtual void enterComparison(Python3Parser::ComparisonContext * /*ctx*/) override { }
  virtual void exitComparison(Python3Parser::ComparisonContext * /*ctx*/) override { }

  virtual void enterComp_op(Python3Parser::Comp_opContext * /*ctx*/) override { }
  virtual void exitComp_op(Python3Parser::Comp_opContext * /*ctx*/) override { }

  virtual void enterStar_expr(Python3Parser::Star_exprContext * /*ctx*/) override { }
  virtual void exitStar_expr(Python3Parser::Star_exprContext * /*ctx*/) override { }

  virtual void enterExpr(Python3Parser::ExprContext * /*ctx*/) override { }
  virtual void exitExpr(Python3Parser::ExprContext * /*ctx*/) override { }

  virtual void enterAtom_expr(Python3Parser::Atom_exprContext * /*ctx*/) override { }
  virtual void exitAtom_expr(Python3Parser::Atom_exprContext * /*ctx*/) override { }

  virtual void enterAtom(Python3Parser::AtomContext * /*ctx*/) override { }
  virtual void exitAtom(Python3Parser::AtomContext * /*ctx*/) override { }

  virtual void enterName(Python3Parser::NameContext * /*ctx*/) override { }
  virtual void exitName(Python3Parser::NameContext * /*ctx*/) override { }

  virtual void enterTestlist_comp(Python3Parser::Testlist_compContext * /*ctx*/) override { }
  virtual void exitTestlist_comp(Python3Parser::Testlist_compContext * /*ctx*/) override { }

  virtual void enterTrailer(Python3Parser::TrailerContext * /*ctx*/) override { }
  virtual void exitTrailer(Python3Parser::TrailerContext * /*ctx*/) override { }

  virtual void enterSubscriptlist(Python3Parser::SubscriptlistContext * /*ctx*/) override { }
  virtual void exitSubscriptlist(Python3Parser::SubscriptlistContext * /*ctx*/) override { }

  virtual void enterSubscript_(Python3Parser::Subscript_Context * /*ctx*/) override { }
  virtual void exitSubscript_(Python3Parser::Subscript_Context * /*ctx*/) override { }

  virtual void enterSliceop(Python3Parser::SliceopContext * /*ctx*/) override { }
  virtual void exitSliceop(Python3Parser::SliceopContext * /*ctx*/) override { }

  virtual void enterExprlist(Python3Parser::ExprlistContext * /*ctx*/) override { }
  virtual void exitExprlist(Python3Parser::ExprlistContext * /*ctx*/) override { }

  virtual void enterTestlist(Python3Parser::TestlistContext * /*ctx*/) override { }
  virtual void exitTestlist(Python3Parser::TestlistContext * /*ctx*/) override { }

  virtual void enterDictorsetmaker(Python3Parser::DictorsetmakerContext * /*ctx*/) override { }
  virtual void exitDictorsetmaker(Python3Parser::DictorsetmakerContext * /*ctx*/) override { }

  virtual void enterClassdef(Python3Parser::ClassdefContext * /*ctx*/) override { }
  virtual void exitClassdef(Python3Parser::ClassdefContext * /*ctx*/) override { }

  virtual void enterArglist(Python3Parser::ArglistContext * /*ctx*/) override { }
  virtual void exitArglist(Python3Parser::ArglistContext * /*ctx*/) override { }

  virtual void enterArgument(Python3Parser::ArgumentContext * /*ctx*/) override { }
  virtual void exitArgument(Python3Parser::ArgumentContext * /*ctx*/) override { }

  virtual void enterComp_iter(Python3Parser::Comp_iterContext * /*ctx*/) override { }
  virtual void exitComp_iter(Python3Parser::Comp_iterContext * /*ctx*/) override { }

  virtual void enterComp_for(Python3Parser::Comp_forContext * /*ctx*/) override { }
  virtual void exitComp_for(Python3Parser::Comp_forContext * /*ctx*/) override { }

  virtual void enterComp_if(Python3Parser::Comp_ifContext * /*ctx*/) override { }
  virtual void exitComp_if(Python3Parser::Comp_ifContext * /*ctx*/) override { }

  virtual void enterEncoding_decl(Python3Parser::Encoding_declContext * /*ctx*/) override { }
  virtual void exitEncoding_decl(Python3Parser::Encoding_declContext * /*ctx*/) override { }

  virtual void enterYield_expr(Python3Parser::Yield_exprContext * /*ctx*/) override { }
  virtual void exitYield_expr(Python3Parser::Yield_exprContext * /*ctx*/) override { }

  virtual void enterYield_arg(Python3Parser::Yield_argContext * /*ctx*/) override { }
  virtual void exitYield_arg(Python3Parser::Yield_argContext * /*ctx*/) override { }

  virtual void enterStrings(Python3Parser::StringsContext * /*ctx*/) override { }
  virtual void exitStrings(Python3Parser::StringsContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

