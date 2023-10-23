#include "Python3ParserBase.h"

// Generated from Python3Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "Python3Parser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by Python3Parser.
 */
class  Python3ParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by Python3Parser.
   */
    virtual std::any visitSingle_input(Python3Parser::Single_inputContext *context) = 0;

    virtual std::any visitFile_input(Python3Parser::File_inputContext *context) = 0;

    virtual std::any visitEval_input(Python3Parser::Eval_inputContext *context) = 0;

    virtual std::any visitDecorator(Python3Parser::DecoratorContext *context) = 0;

    virtual std::any visitDecorators(Python3Parser::DecoratorsContext *context) = 0;

    virtual std::any visitDecorated(Python3Parser::DecoratedContext *context) = 0;

    virtual std::any visitAsync_funcdef(Python3Parser::Async_funcdefContext *context) = 0;

    virtual std::any visitFuncdef(Python3Parser::FuncdefContext *context) = 0;

    virtual std::any visitParameters(Python3Parser::ParametersContext *context) = 0;

    virtual std::any visitTypedargslist(Python3Parser::TypedargslistContext *context) = 0;

    virtual std::any visitTfpdef(Python3Parser::TfpdefContext *context) = 0;

    virtual std::any visitVarargslist(Python3Parser::VarargslistContext *context) = 0;

    virtual std::any visitVfpdef(Python3Parser::VfpdefContext *context) = 0;

    virtual std::any visitStmt(Python3Parser::StmtContext *context) = 0;

    virtual std::any visitSimple_stmts(Python3Parser::Simple_stmtsContext *context) = 0;

    virtual std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *context) = 0;

    virtual std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *context) = 0;

    virtual std::any visitAnnassign(Python3Parser::AnnassignContext *context) = 0;

    virtual std::any visitTestlist_star_expr(Python3Parser::Testlist_star_exprContext *context) = 0;

    virtual std::any visitAugassign(Python3Parser::AugassignContext *context) = 0;

    virtual std::any visitDel_stmt(Python3Parser::Del_stmtContext *context) = 0;

    virtual std::any visitPass_stmt(Python3Parser::Pass_stmtContext *context) = 0;

    virtual std::any visitFlow_stmt(Python3Parser::Flow_stmtContext *context) = 0;

    virtual std::any visitBreak_stmt(Python3Parser::Break_stmtContext *context) = 0;

    virtual std::any visitContinue_stmt(Python3Parser::Continue_stmtContext *context) = 0;

    virtual std::any visitReturn_stmt(Python3Parser::Return_stmtContext *context) = 0;

    virtual std::any visitYield_stmt(Python3Parser::Yield_stmtContext *context) = 0;

    virtual std::any visitRaise_stmt(Python3Parser::Raise_stmtContext *context) = 0;

    virtual std::any visitImport_stmt(Python3Parser::Import_stmtContext *context) = 0;

    virtual std::any visitImport_name(Python3Parser::Import_nameContext *context) = 0;

    virtual std::any visitImport_from(Python3Parser::Import_fromContext *context) = 0;

    virtual std::any visitImport_as_name(Python3Parser::Import_as_nameContext *context) = 0;

    virtual std::any visitDotted_as_name(Python3Parser::Dotted_as_nameContext *context) = 0;

    virtual std::any visitImport_as_names(Python3Parser::Import_as_namesContext *context) = 0;

    virtual std::any visitDotted_as_names(Python3Parser::Dotted_as_namesContext *context) = 0;

    virtual std::any visitDotted_name(Python3Parser::Dotted_nameContext *context) = 0;

    virtual std::any visitGlobal_stmt(Python3Parser::Global_stmtContext *context) = 0;

    virtual std::any visitNonlocal_stmt(Python3Parser::Nonlocal_stmtContext *context) = 0;

    virtual std::any visitAssert_stmt(Python3Parser::Assert_stmtContext *context) = 0;

    virtual std::any visitCompound_stmt(Python3Parser::Compound_stmtContext *context) = 0;

    virtual std::any visitAsync_stmt(Python3Parser::Async_stmtContext *context) = 0;

    virtual std::any visitIf_stmt(Python3Parser::If_stmtContext *context) = 0;

    virtual std::any visitWhile_stmt(Python3Parser::While_stmtContext *context) = 0;

    virtual std::any visitFor_stmt(Python3Parser::For_stmtContext *context) = 0;

    virtual std::any visitTry_stmt(Python3Parser::Try_stmtContext *context) = 0;

    virtual std::any visitWith_stmt(Python3Parser::With_stmtContext *context) = 0;

    virtual std::any visitWith_item(Python3Parser::With_itemContext *context) = 0;

    virtual std::any visitExcept_clause(Python3Parser::Except_clauseContext *context) = 0;

    virtual std::any visitBlock(Python3Parser::BlockContext *context) = 0;

    virtual std::any visitMatch_stmt(Python3Parser::Match_stmtContext *context) = 0;

    virtual std::any visitSubject_expr(Python3Parser::Subject_exprContext *context) = 0;

    virtual std::any visitStar_named_expressions(Python3Parser::Star_named_expressionsContext *context) = 0;

    virtual std::any visitStar_named_expression(Python3Parser::Star_named_expressionContext *context) = 0;

    virtual std::any visitCase_block(Python3Parser::Case_blockContext *context) = 0;

    virtual std::any visitGuard(Python3Parser::GuardContext *context) = 0;

    virtual std::any visitPatterns(Python3Parser::PatternsContext *context) = 0;

    virtual std::any visitPattern(Python3Parser::PatternContext *context) = 0;

    virtual std::any visitAs_pattern(Python3Parser::As_patternContext *context) = 0;

    virtual std::any visitOr_pattern(Python3Parser::Or_patternContext *context) = 0;

    virtual std::any visitClosed_pattern(Python3Parser::Closed_patternContext *context) = 0;

    virtual std::any visitLiteral_pattern(Python3Parser::Literal_patternContext *context) = 0;

    virtual std::any visitLiteral_expr(Python3Parser::Literal_exprContext *context) = 0;

    virtual std::any visitComplex_number(Python3Parser::Complex_numberContext *context) = 0;

    virtual std::any visitSigned_number(Python3Parser::Signed_numberContext *context) = 0;

    virtual std::any visitSigned_real_number(Python3Parser::Signed_real_numberContext *context) = 0;

    virtual std::any visitReal_number(Python3Parser::Real_numberContext *context) = 0;

    virtual std::any visitImaginary_number(Python3Parser::Imaginary_numberContext *context) = 0;

    virtual std::any visitCapture_pattern(Python3Parser::Capture_patternContext *context) = 0;

    virtual std::any visitPattern_capture_target(Python3Parser::Pattern_capture_targetContext *context) = 0;

    virtual std::any visitWildcard_pattern(Python3Parser::Wildcard_patternContext *context) = 0;

    virtual std::any visitValue_pattern(Python3Parser::Value_patternContext *context) = 0;

    virtual std::any visitAttr(Python3Parser::AttrContext *context) = 0;

    virtual std::any visitName_or_attr(Python3Parser::Name_or_attrContext *context) = 0;

    virtual std::any visitGroup_pattern(Python3Parser::Group_patternContext *context) = 0;

    virtual std::any visitSequence_pattern(Python3Parser::Sequence_patternContext *context) = 0;

    virtual std::any visitOpen_sequence_pattern(Python3Parser::Open_sequence_patternContext *context) = 0;

    virtual std::any visitMaybe_sequence_pattern(Python3Parser::Maybe_sequence_patternContext *context) = 0;

    virtual std::any visitMaybe_star_pattern(Python3Parser::Maybe_star_patternContext *context) = 0;

    virtual std::any visitStar_pattern(Python3Parser::Star_patternContext *context) = 0;

    virtual std::any visitMapping_pattern(Python3Parser::Mapping_patternContext *context) = 0;

    virtual std::any visitItems_pattern(Python3Parser::Items_patternContext *context) = 0;

    virtual std::any visitKey_value_pattern(Python3Parser::Key_value_patternContext *context) = 0;

    virtual std::any visitDouble_star_pattern(Python3Parser::Double_star_patternContext *context) = 0;

    virtual std::any visitClass_pattern(Python3Parser::Class_patternContext *context) = 0;

    virtual std::any visitPositional_patterns(Python3Parser::Positional_patternsContext *context) = 0;

    virtual std::any visitKeyword_patterns(Python3Parser::Keyword_patternsContext *context) = 0;

    virtual std::any visitKeyword_pattern(Python3Parser::Keyword_patternContext *context) = 0;

    virtual std::any visitTest(Python3Parser::TestContext *context) = 0;

    virtual std::any visitTest_nocond(Python3Parser::Test_nocondContext *context) = 0;

    virtual std::any visitLambdef(Python3Parser::LambdefContext *context) = 0;

    virtual std::any visitLambdef_nocond(Python3Parser::Lambdef_nocondContext *context) = 0;

    virtual std::any visitOr_test(Python3Parser::Or_testContext *context) = 0;

    virtual std::any visitAnd_test(Python3Parser::And_testContext *context) = 0;

    virtual std::any visitNot_test(Python3Parser::Not_testContext *context) = 0;

    virtual std::any visitComparison(Python3Parser::ComparisonContext *context) = 0;

    virtual std::any visitComp_op(Python3Parser::Comp_opContext *context) = 0;

    virtual std::any visitStar_expr(Python3Parser::Star_exprContext *context) = 0;

    virtual std::any visitExpr(Python3Parser::ExprContext *context) = 0;

    virtual std::any visitAtom_expr(Python3Parser::Atom_exprContext *context) = 0;

    virtual std::any visitAtom(Python3Parser::AtomContext *context) = 0;

    virtual std::any visitName(Python3Parser::NameContext *context) = 0;

    virtual std::any visitTestlist_comp(Python3Parser::Testlist_compContext *context) = 0;

    virtual std::any visitTrailer(Python3Parser::TrailerContext *context) = 0;

    virtual std::any visitSubscriptlist(Python3Parser::SubscriptlistContext *context) = 0;

    virtual std::any visitSubscript_(Python3Parser::Subscript_Context *context) = 0;

    virtual std::any visitSliceop(Python3Parser::SliceopContext *context) = 0;

    virtual std::any visitExprlist(Python3Parser::ExprlistContext *context) = 0;

    virtual std::any visitTestlist(Python3Parser::TestlistContext *context) = 0;

    virtual std::any visitDictorsetmaker(Python3Parser::DictorsetmakerContext *context) = 0;

    virtual std::any visitClassdef(Python3Parser::ClassdefContext *context) = 0;

    virtual std::any visitArglist(Python3Parser::ArglistContext *context) = 0;

    virtual std::any visitArgument(Python3Parser::ArgumentContext *context) = 0;

    virtual std::any visitComp_iter(Python3Parser::Comp_iterContext *context) = 0;

    virtual std::any visitComp_for(Python3Parser::Comp_forContext *context) = 0;

    virtual std::any visitComp_if(Python3Parser::Comp_ifContext *context) = 0;

    virtual std::any visitEncoding_decl(Python3Parser::Encoding_declContext *context) = 0;

    virtual std::any visitYield_expr(Python3Parser::Yield_exprContext *context) = 0;

    virtual std::any visitYield_arg(Python3Parser::Yield_argContext *context) = 0;

    virtual std::any visitStrings(Python3Parser::StringsContext *context) = 0;


};

