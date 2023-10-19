#include "Python3ParserBase.h"

// Generated from Python3Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "Python3Parser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by Python3Parser.
 */
class  Python3ParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterSingle_input(Python3Parser::Single_inputContext *ctx) = 0;
  virtual void exitSingle_input(Python3Parser::Single_inputContext *ctx) = 0;

  virtual void enterFile_input(Python3Parser::File_inputContext *ctx) = 0;
  virtual void exitFile_input(Python3Parser::File_inputContext *ctx) = 0;

  virtual void enterEval_input(Python3Parser::Eval_inputContext *ctx) = 0;
  virtual void exitEval_input(Python3Parser::Eval_inputContext *ctx) = 0;

  virtual void enterDecorator(Python3Parser::DecoratorContext *ctx) = 0;
  virtual void exitDecorator(Python3Parser::DecoratorContext *ctx) = 0;

  virtual void enterDecorators(Python3Parser::DecoratorsContext *ctx) = 0;
  virtual void exitDecorators(Python3Parser::DecoratorsContext *ctx) = 0;

  virtual void enterDecorated(Python3Parser::DecoratedContext *ctx) = 0;
  virtual void exitDecorated(Python3Parser::DecoratedContext *ctx) = 0;

  virtual void enterAsync_funcdef(Python3Parser::Async_funcdefContext *ctx) = 0;
  virtual void exitAsync_funcdef(Python3Parser::Async_funcdefContext *ctx) = 0;

  virtual void enterFuncdef(Python3Parser::FuncdefContext *ctx) = 0;
  virtual void exitFuncdef(Python3Parser::FuncdefContext *ctx) = 0;

  virtual void enterParameters(Python3Parser::ParametersContext *ctx) = 0;
  virtual void exitParameters(Python3Parser::ParametersContext *ctx) = 0;

  virtual void enterTypedargslist(Python3Parser::TypedargslistContext *ctx) = 0;
  virtual void exitTypedargslist(Python3Parser::TypedargslistContext *ctx) = 0;

  virtual void enterTfpdef(Python3Parser::TfpdefContext *ctx) = 0;
  virtual void exitTfpdef(Python3Parser::TfpdefContext *ctx) = 0;

  virtual void enterVarargslist(Python3Parser::VarargslistContext *ctx) = 0;
  virtual void exitVarargslist(Python3Parser::VarargslistContext *ctx) = 0;

  virtual void enterVfpdef(Python3Parser::VfpdefContext *ctx) = 0;
  virtual void exitVfpdef(Python3Parser::VfpdefContext *ctx) = 0;

  virtual void enterStmt(Python3Parser::StmtContext *ctx) = 0;
  virtual void exitStmt(Python3Parser::StmtContext *ctx) = 0;

  virtual void enterSimple_stmts(Python3Parser::Simple_stmtsContext *ctx) = 0;
  virtual void exitSimple_stmts(Python3Parser::Simple_stmtsContext *ctx) = 0;

  virtual void enterSimple_stmt(Python3Parser::Simple_stmtContext *ctx) = 0;
  virtual void exitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) = 0;

  virtual void enterExpr_stmt(Python3Parser::Expr_stmtContext *ctx) = 0;
  virtual void exitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) = 0;

  virtual void enterAnnassign(Python3Parser::AnnassignContext *ctx) = 0;
  virtual void exitAnnassign(Python3Parser::AnnassignContext *ctx) = 0;

  virtual void enterTestlist_star_expr(Python3Parser::Testlist_star_exprContext *ctx) = 0;
  virtual void exitTestlist_star_expr(Python3Parser::Testlist_star_exprContext *ctx) = 0;

  virtual void enterAugassign(Python3Parser::AugassignContext *ctx) = 0;
  virtual void exitAugassign(Python3Parser::AugassignContext *ctx) = 0;

  virtual void enterDel_stmt(Python3Parser::Del_stmtContext *ctx) = 0;
  virtual void exitDel_stmt(Python3Parser::Del_stmtContext *ctx) = 0;

  virtual void enterPass_stmt(Python3Parser::Pass_stmtContext *ctx) = 0;
  virtual void exitPass_stmt(Python3Parser::Pass_stmtContext *ctx) = 0;

  virtual void enterFlow_stmt(Python3Parser::Flow_stmtContext *ctx) = 0;
  virtual void exitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) = 0;

  virtual void enterBreak_stmt(Python3Parser::Break_stmtContext *ctx) = 0;
  virtual void exitBreak_stmt(Python3Parser::Break_stmtContext *ctx) = 0;

  virtual void enterContinue_stmt(Python3Parser::Continue_stmtContext *ctx) = 0;
  virtual void exitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) = 0;

  virtual void enterReturn_stmt(Python3Parser::Return_stmtContext *ctx) = 0;
  virtual void exitReturn_stmt(Python3Parser::Return_stmtContext *ctx) = 0;

  virtual void enterYield_stmt(Python3Parser::Yield_stmtContext *ctx) = 0;
  virtual void exitYield_stmt(Python3Parser::Yield_stmtContext *ctx) = 0;

  virtual void enterRaise_stmt(Python3Parser::Raise_stmtContext *ctx) = 0;
  virtual void exitRaise_stmt(Python3Parser::Raise_stmtContext *ctx) = 0;

  virtual void enterImport_stmt(Python3Parser::Import_stmtContext *ctx) = 0;
  virtual void exitImport_stmt(Python3Parser::Import_stmtContext *ctx) = 0;

  virtual void enterImport_name(Python3Parser::Import_nameContext *ctx) = 0;
  virtual void exitImport_name(Python3Parser::Import_nameContext *ctx) = 0;

  virtual void enterImport_from(Python3Parser::Import_fromContext *ctx) = 0;
  virtual void exitImport_from(Python3Parser::Import_fromContext *ctx) = 0;

  virtual void enterImport_as_name(Python3Parser::Import_as_nameContext *ctx) = 0;
  virtual void exitImport_as_name(Python3Parser::Import_as_nameContext *ctx) = 0;

  virtual void enterDotted_as_name(Python3Parser::Dotted_as_nameContext *ctx) = 0;
  virtual void exitDotted_as_name(Python3Parser::Dotted_as_nameContext *ctx) = 0;

  virtual void enterImport_as_names(Python3Parser::Import_as_namesContext *ctx) = 0;
  virtual void exitImport_as_names(Python3Parser::Import_as_namesContext *ctx) = 0;

  virtual void enterDotted_as_names(Python3Parser::Dotted_as_namesContext *ctx) = 0;
  virtual void exitDotted_as_names(Python3Parser::Dotted_as_namesContext *ctx) = 0;

  virtual void enterDotted_name(Python3Parser::Dotted_nameContext *ctx) = 0;
  virtual void exitDotted_name(Python3Parser::Dotted_nameContext *ctx) = 0;

  virtual void enterGlobal_stmt(Python3Parser::Global_stmtContext *ctx) = 0;
  virtual void exitGlobal_stmt(Python3Parser::Global_stmtContext *ctx) = 0;

  virtual void enterNonlocal_stmt(Python3Parser::Nonlocal_stmtContext *ctx) = 0;
  virtual void exitNonlocal_stmt(Python3Parser::Nonlocal_stmtContext *ctx) = 0;

  virtual void enterAssert_stmt(Python3Parser::Assert_stmtContext *ctx) = 0;
  virtual void exitAssert_stmt(Python3Parser::Assert_stmtContext *ctx) = 0;

  virtual void enterCompound_stmt(Python3Parser::Compound_stmtContext *ctx) = 0;
  virtual void exitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) = 0;

  virtual void enterAsync_stmt(Python3Parser::Async_stmtContext *ctx) = 0;
  virtual void exitAsync_stmt(Python3Parser::Async_stmtContext *ctx) = 0;

  virtual void enterIf_stmt(Python3Parser::If_stmtContext *ctx) = 0;
  virtual void exitIf_stmt(Python3Parser::If_stmtContext *ctx) = 0;

  virtual void enterWhile_stmt(Python3Parser::While_stmtContext *ctx) = 0;
  virtual void exitWhile_stmt(Python3Parser::While_stmtContext *ctx) = 0;

  virtual void enterFor_stmt(Python3Parser::For_stmtContext *ctx) = 0;
  virtual void exitFor_stmt(Python3Parser::For_stmtContext *ctx) = 0;

  virtual void enterTry_stmt(Python3Parser::Try_stmtContext *ctx) = 0;
  virtual void exitTry_stmt(Python3Parser::Try_stmtContext *ctx) = 0;

  virtual void enterWith_stmt(Python3Parser::With_stmtContext *ctx) = 0;
  virtual void exitWith_stmt(Python3Parser::With_stmtContext *ctx) = 0;

  virtual void enterWith_item(Python3Parser::With_itemContext *ctx) = 0;
  virtual void exitWith_item(Python3Parser::With_itemContext *ctx) = 0;

  virtual void enterExcept_clause(Python3Parser::Except_clauseContext *ctx) = 0;
  virtual void exitExcept_clause(Python3Parser::Except_clauseContext *ctx) = 0;

  virtual void enterBlock(Python3Parser::BlockContext *ctx) = 0;
  virtual void exitBlock(Python3Parser::BlockContext *ctx) = 0;

  virtual void enterMatch_stmt(Python3Parser::Match_stmtContext *ctx) = 0;
  virtual void exitMatch_stmt(Python3Parser::Match_stmtContext *ctx) = 0;

  virtual void enterSubject_expr(Python3Parser::Subject_exprContext *ctx) = 0;
  virtual void exitSubject_expr(Python3Parser::Subject_exprContext *ctx) = 0;

  virtual void enterStar_named_expressions(Python3Parser::Star_named_expressionsContext *ctx) = 0;
  virtual void exitStar_named_expressions(Python3Parser::Star_named_expressionsContext *ctx) = 0;

  virtual void enterStar_named_expression(Python3Parser::Star_named_expressionContext *ctx) = 0;
  virtual void exitStar_named_expression(Python3Parser::Star_named_expressionContext *ctx) = 0;

  virtual void enterCase_block(Python3Parser::Case_blockContext *ctx) = 0;
  virtual void exitCase_block(Python3Parser::Case_blockContext *ctx) = 0;

  virtual void enterGuard(Python3Parser::GuardContext *ctx) = 0;
  virtual void exitGuard(Python3Parser::GuardContext *ctx) = 0;

  virtual void enterPatterns(Python3Parser::PatternsContext *ctx) = 0;
  virtual void exitPatterns(Python3Parser::PatternsContext *ctx) = 0;

  virtual void enterPattern(Python3Parser::PatternContext *ctx) = 0;
  virtual void exitPattern(Python3Parser::PatternContext *ctx) = 0;

  virtual void enterAs_pattern(Python3Parser::As_patternContext *ctx) = 0;
  virtual void exitAs_pattern(Python3Parser::As_patternContext *ctx) = 0;

  virtual void enterOr_pattern(Python3Parser::Or_patternContext *ctx) = 0;
  virtual void exitOr_pattern(Python3Parser::Or_patternContext *ctx) = 0;

  virtual void enterClosed_pattern(Python3Parser::Closed_patternContext *ctx) = 0;
  virtual void exitClosed_pattern(Python3Parser::Closed_patternContext *ctx) = 0;

  virtual void enterLiteral_pattern(Python3Parser::Literal_patternContext *ctx) = 0;
  virtual void exitLiteral_pattern(Python3Parser::Literal_patternContext *ctx) = 0;

  virtual void enterLiteral_expr(Python3Parser::Literal_exprContext *ctx) = 0;
  virtual void exitLiteral_expr(Python3Parser::Literal_exprContext *ctx) = 0;

  virtual void enterComplex_number(Python3Parser::Complex_numberContext *ctx) = 0;
  virtual void exitComplex_number(Python3Parser::Complex_numberContext *ctx) = 0;

  virtual void enterSigned_number(Python3Parser::Signed_numberContext *ctx) = 0;
  virtual void exitSigned_number(Python3Parser::Signed_numberContext *ctx) = 0;

  virtual void enterSigned_real_number(Python3Parser::Signed_real_numberContext *ctx) = 0;
  virtual void exitSigned_real_number(Python3Parser::Signed_real_numberContext *ctx) = 0;

  virtual void enterReal_number(Python3Parser::Real_numberContext *ctx) = 0;
  virtual void exitReal_number(Python3Parser::Real_numberContext *ctx) = 0;

  virtual void enterImaginary_number(Python3Parser::Imaginary_numberContext *ctx) = 0;
  virtual void exitImaginary_number(Python3Parser::Imaginary_numberContext *ctx) = 0;

  virtual void enterCapture_pattern(Python3Parser::Capture_patternContext *ctx) = 0;
  virtual void exitCapture_pattern(Python3Parser::Capture_patternContext *ctx) = 0;

  virtual void enterPattern_capture_target(Python3Parser::Pattern_capture_targetContext *ctx) = 0;
  virtual void exitPattern_capture_target(Python3Parser::Pattern_capture_targetContext *ctx) = 0;

  virtual void enterWildcard_pattern(Python3Parser::Wildcard_patternContext *ctx) = 0;
  virtual void exitWildcard_pattern(Python3Parser::Wildcard_patternContext *ctx) = 0;

  virtual void enterValue_pattern(Python3Parser::Value_patternContext *ctx) = 0;
  virtual void exitValue_pattern(Python3Parser::Value_patternContext *ctx) = 0;

  virtual void enterAttr(Python3Parser::AttrContext *ctx) = 0;
  virtual void exitAttr(Python3Parser::AttrContext *ctx) = 0;

  virtual void enterName_or_attr(Python3Parser::Name_or_attrContext *ctx) = 0;
  virtual void exitName_or_attr(Python3Parser::Name_or_attrContext *ctx) = 0;

  virtual void enterGroup_pattern(Python3Parser::Group_patternContext *ctx) = 0;
  virtual void exitGroup_pattern(Python3Parser::Group_patternContext *ctx) = 0;

  virtual void enterSequence_pattern(Python3Parser::Sequence_patternContext *ctx) = 0;
  virtual void exitSequence_pattern(Python3Parser::Sequence_patternContext *ctx) = 0;

  virtual void enterOpen_sequence_pattern(Python3Parser::Open_sequence_patternContext *ctx) = 0;
  virtual void exitOpen_sequence_pattern(Python3Parser::Open_sequence_patternContext *ctx) = 0;

  virtual void enterMaybe_sequence_pattern(Python3Parser::Maybe_sequence_patternContext *ctx) = 0;
  virtual void exitMaybe_sequence_pattern(Python3Parser::Maybe_sequence_patternContext *ctx) = 0;

  virtual void enterMaybe_star_pattern(Python3Parser::Maybe_star_patternContext *ctx) = 0;
  virtual void exitMaybe_star_pattern(Python3Parser::Maybe_star_patternContext *ctx) = 0;

  virtual void enterStar_pattern(Python3Parser::Star_patternContext *ctx) = 0;
  virtual void exitStar_pattern(Python3Parser::Star_patternContext *ctx) = 0;

  virtual void enterMapping_pattern(Python3Parser::Mapping_patternContext *ctx) = 0;
  virtual void exitMapping_pattern(Python3Parser::Mapping_patternContext *ctx) = 0;

  virtual void enterItems_pattern(Python3Parser::Items_patternContext *ctx) = 0;
  virtual void exitItems_pattern(Python3Parser::Items_patternContext *ctx) = 0;

  virtual void enterKey_value_pattern(Python3Parser::Key_value_patternContext *ctx) = 0;
  virtual void exitKey_value_pattern(Python3Parser::Key_value_patternContext *ctx) = 0;

  virtual void enterDouble_star_pattern(Python3Parser::Double_star_patternContext *ctx) = 0;
  virtual void exitDouble_star_pattern(Python3Parser::Double_star_patternContext *ctx) = 0;

  virtual void enterClass_pattern(Python3Parser::Class_patternContext *ctx) = 0;
  virtual void exitClass_pattern(Python3Parser::Class_patternContext *ctx) = 0;

  virtual void enterPositional_patterns(Python3Parser::Positional_patternsContext *ctx) = 0;
  virtual void exitPositional_patterns(Python3Parser::Positional_patternsContext *ctx) = 0;

  virtual void enterKeyword_patterns(Python3Parser::Keyword_patternsContext *ctx) = 0;
  virtual void exitKeyword_patterns(Python3Parser::Keyword_patternsContext *ctx) = 0;

  virtual void enterKeyword_pattern(Python3Parser::Keyword_patternContext *ctx) = 0;
  virtual void exitKeyword_pattern(Python3Parser::Keyword_patternContext *ctx) = 0;

  virtual void enterTest(Python3Parser::TestContext *ctx) = 0;
  virtual void exitTest(Python3Parser::TestContext *ctx) = 0;

  virtual void enterTest_nocond(Python3Parser::Test_nocondContext *ctx) = 0;
  virtual void exitTest_nocond(Python3Parser::Test_nocondContext *ctx) = 0;

  virtual void enterLambdef(Python3Parser::LambdefContext *ctx) = 0;
  virtual void exitLambdef(Python3Parser::LambdefContext *ctx) = 0;

  virtual void enterLambdef_nocond(Python3Parser::Lambdef_nocondContext *ctx) = 0;
  virtual void exitLambdef_nocond(Python3Parser::Lambdef_nocondContext *ctx) = 0;

  virtual void enterOr_test(Python3Parser::Or_testContext *ctx) = 0;
  virtual void exitOr_test(Python3Parser::Or_testContext *ctx) = 0;

  virtual void enterAnd_test(Python3Parser::And_testContext *ctx) = 0;
  virtual void exitAnd_test(Python3Parser::And_testContext *ctx) = 0;

  virtual void enterNot_test(Python3Parser::Not_testContext *ctx) = 0;
  virtual void exitNot_test(Python3Parser::Not_testContext *ctx) = 0;

  virtual void enterComparison(Python3Parser::ComparisonContext *ctx) = 0;
  virtual void exitComparison(Python3Parser::ComparisonContext *ctx) = 0;

  virtual void enterComp_op(Python3Parser::Comp_opContext *ctx) = 0;
  virtual void exitComp_op(Python3Parser::Comp_opContext *ctx) = 0;

  virtual void enterStar_expr(Python3Parser::Star_exprContext *ctx) = 0;
  virtual void exitStar_expr(Python3Parser::Star_exprContext *ctx) = 0;

  virtual void enterExpr(Python3Parser::ExprContext *ctx) = 0;
  virtual void exitExpr(Python3Parser::ExprContext *ctx) = 0;

  virtual void enterAtom_expr(Python3Parser::Atom_exprContext *ctx) = 0;
  virtual void exitAtom_expr(Python3Parser::Atom_exprContext *ctx) = 0;

  virtual void enterAtom(Python3Parser::AtomContext *ctx) = 0;
  virtual void exitAtom(Python3Parser::AtomContext *ctx) = 0;

  virtual void enterName(Python3Parser::NameContext *ctx) = 0;
  virtual void exitName(Python3Parser::NameContext *ctx) = 0;

  virtual void enterTestlist_comp(Python3Parser::Testlist_compContext *ctx) = 0;
  virtual void exitTestlist_comp(Python3Parser::Testlist_compContext *ctx) = 0;

  virtual void enterTrailer(Python3Parser::TrailerContext *ctx) = 0;
  virtual void exitTrailer(Python3Parser::TrailerContext *ctx) = 0;

  virtual void enterSubscriptlist(Python3Parser::SubscriptlistContext *ctx) = 0;
  virtual void exitSubscriptlist(Python3Parser::SubscriptlistContext *ctx) = 0;

  virtual void enterSubscript_(Python3Parser::Subscript_Context *ctx) = 0;
  virtual void exitSubscript_(Python3Parser::Subscript_Context *ctx) = 0;

  virtual void enterSliceop(Python3Parser::SliceopContext *ctx) = 0;
  virtual void exitSliceop(Python3Parser::SliceopContext *ctx) = 0;

  virtual void enterExprlist(Python3Parser::ExprlistContext *ctx) = 0;
  virtual void exitExprlist(Python3Parser::ExprlistContext *ctx) = 0;

  virtual void enterTestlist(Python3Parser::TestlistContext *ctx) = 0;
  virtual void exitTestlist(Python3Parser::TestlistContext *ctx) = 0;

  virtual void enterDictorsetmaker(Python3Parser::DictorsetmakerContext *ctx) = 0;
  virtual void exitDictorsetmaker(Python3Parser::DictorsetmakerContext *ctx) = 0;

  virtual void enterClassdef(Python3Parser::ClassdefContext *ctx) = 0;
  virtual void exitClassdef(Python3Parser::ClassdefContext *ctx) = 0;

  virtual void enterArglist(Python3Parser::ArglistContext *ctx) = 0;
  virtual void exitArglist(Python3Parser::ArglistContext *ctx) = 0;

  virtual void enterArgument(Python3Parser::ArgumentContext *ctx) = 0;
  virtual void exitArgument(Python3Parser::ArgumentContext *ctx) = 0;

  virtual void enterComp_iter(Python3Parser::Comp_iterContext *ctx) = 0;
  virtual void exitComp_iter(Python3Parser::Comp_iterContext *ctx) = 0;

  virtual void enterComp_for(Python3Parser::Comp_forContext *ctx) = 0;
  virtual void exitComp_for(Python3Parser::Comp_forContext *ctx) = 0;

  virtual void enterComp_if(Python3Parser::Comp_ifContext *ctx) = 0;
  virtual void exitComp_if(Python3Parser::Comp_ifContext *ctx) = 0;

  virtual void enterEncoding_decl(Python3Parser::Encoding_declContext *ctx) = 0;
  virtual void exitEncoding_decl(Python3Parser::Encoding_declContext *ctx) = 0;

  virtual void enterYield_expr(Python3Parser::Yield_exprContext *ctx) = 0;
  virtual void exitYield_expr(Python3Parser::Yield_exprContext *ctx) = 0;

  virtual void enterYield_arg(Python3Parser::Yield_argContext *ctx) = 0;
  virtual void exitYield_arg(Python3Parser::Yield_argContext *ctx) = 0;

  virtual void enterStrings(Python3Parser::StringsContext *ctx) = 0;
  virtual void exitStrings(Python3Parser::StringsContext *ctx) = 0;


};

