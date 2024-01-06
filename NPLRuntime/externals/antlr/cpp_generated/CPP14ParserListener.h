#include "CPP14ParserBase.h"

// Generated from CPP14Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "CPP14Parser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by CPP14Parser.
 */
class  CPP14ParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterTranslationUnit(CPP14Parser::TranslationUnitContext *ctx) = 0;
  virtual void exitTranslationUnit(CPP14Parser::TranslationUnitContext *ctx) = 0;

  virtual void enterPrimaryExpression(CPP14Parser::PrimaryExpressionContext *ctx) = 0;
  virtual void exitPrimaryExpression(CPP14Parser::PrimaryExpressionContext *ctx) = 0;

  virtual void enterIdExpression(CPP14Parser::IdExpressionContext *ctx) = 0;
  virtual void exitIdExpression(CPP14Parser::IdExpressionContext *ctx) = 0;

  virtual void enterUnqualifiedId(CPP14Parser::UnqualifiedIdContext *ctx) = 0;
  virtual void exitUnqualifiedId(CPP14Parser::UnqualifiedIdContext *ctx) = 0;

  virtual void enterQualifiedId(CPP14Parser::QualifiedIdContext *ctx) = 0;
  virtual void exitQualifiedId(CPP14Parser::QualifiedIdContext *ctx) = 0;

  virtual void enterNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext *ctx) = 0;
  virtual void exitNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext *ctx) = 0;

  virtual void enterLambdaExpression(CPP14Parser::LambdaExpressionContext *ctx) = 0;
  virtual void exitLambdaExpression(CPP14Parser::LambdaExpressionContext *ctx) = 0;

  virtual void enterLambdaIntroducer(CPP14Parser::LambdaIntroducerContext *ctx) = 0;
  virtual void exitLambdaIntroducer(CPP14Parser::LambdaIntroducerContext *ctx) = 0;

  virtual void enterLambdaCapture(CPP14Parser::LambdaCaptureContext *ctx) = 0;
  virtual void exitLambdaCapture(CPP14Parser::LambdaCaptureContext *ctx) = 0;

  virtual void enterCaptureDefault(CPP14Parser::CaptureDefaultContext *ctx) = 0;
  virtual void exitCaptureDefault(CPP14Parser::CaptureDefaultContext *ctx) = 0;

  virtual void enterCaptureList(CPP14Parser::CaptureListContext *ctx) = 0;
  virtual void exitCaptureList(CPP14Parser::CaptureListContext *ctx) = 0;

  virtual void enterCapture(CPP14Parser::CaptureContext *ctx) = 0;
  virtual void exitCapture(CPP14Parser::CaptureContext *ctx) = 0;

  virtual void enterSimpleCapture(CPP14Parser::SimpleCaptureContext *ctx) = 0;
  virtual void exitSimpleCapture(CPP14Parser::SimpleCaptureContext *ctx) = 0;

  virtual void enterInitcapture(CPP14Parser::InitcaptureContext *ctx) = 0;
  virtual void exitInitcapture(CPP14Parser::InitcaptureContext *ctx) = 0;

  virtual void enterLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext *ctx) = 0;
  virtual void exitLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext *ctx) = 0;

  virtual void enterPostfixExpression(CPP14Parser::PostfixExpressionContext *ctx) = 0;
  virtual void exitPostfixExpression(CPP14Parser::PostfixExpressionContext *ctx) = 0;

  virtual void enterTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext *ctx) = 0;
  virtual void exitTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext *ctx) = 0;

  virtual void enterExpressionList(CPP14Parser::ExpressionListContext *ctx) = 0;
  virtual void exitExpressionList(CPP14Parser::ExpressionListContext *ctx) = 0;

  virtual void enterPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext *ctx) = 0;
  virtual void exitPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext *ctx) = 0;

  virtual void enterUnaryExpression(CPP14Parser::UnaryExpressionContext *ctx) = 0;
  virtual void exitUnaryExpression(CPP14Parser::UnaryExpressionContext *ctx) = 0;

  virtual void enterUnaryOperator(CPP14Parser::UnaryOperatorContext *ctx) = 0;
  virtual void exitUnaryOperator(CPP14Parser::UnaryOperatorContext *ctx) = 0;

  virtual void enterNewExpression_(CPP14Parser::NewExpression_Context *ctx) = 0;
  virtual void exitNewExpression_(CPP14Parser::NewExpression_Context *ctx) = 0;

  virtual void enterNewPlacement(CPP14Parser::NewPlacementContext *ctx) = 0;
  virtual void exitNewPlacement(CPP14Parser::NewPlacementContext *ctx) = 0;

  virtual void enterNewTypeId(CPP14Parser::NewTypeIdContext *ctx) = 0;
  virtual void exitNewTypeId(CPP14Parser::NewTypeIdContext *ctx) = 0;

  virtual void enterNewDeclarator_(CPP14Parser::NewDeclarator_Context *ctx) = 0;
  virtual void exitNewDeclarator_(CPP14Parser::NewDeclarator_Context *ctx) = 0;

  virtual void enterNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext *ctx) = 0;
  virtual void exitNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext *ctx) = 0;

  virtual void enterNewInitializer_(CPP14Parser::NewInitializer_Context *ctx) = 0;
  virtual void exitNewInitializer_(CPP14Parser::NewInitializer_Context *ctx) = 0;

  virtual void enterDeleteExpression(CPP14Parser::DeleteExpressionContext *ctx) = 0;
  virtual void exitDeleteExpression(CPP14Parser::DeleteExpressionContext *ctx) = 0;

  virtual void enterNoExceptExpression(CPP14Parser::NoExceptExpressionContext *ctx) = 0;
  virtual void exitNoExceptExpression(CPP14Parser::NoExceptExpressionContext *ctx) = 0;

  virtual void enterCastExpression(CPP14Parser::CastExpressionContext *ctx) = 0;
  virtual void exitCastExpression(CPP14Parser::CastExpressionContext *ctx) = 0;

  virtual void enterPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext *ctx) = 0;
  virtual void exitPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext *ctx) = 0;

  virtual void enterMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *ctx) = 0;
  virtual void exitMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *ctx) = 0;

  virtual void enterAdditiveExpression(CPP14Parser::AdditiveExpressionContext *ctx) = 0;
  virtual void exitAdditiveExpression(CPP14Parser::AdditiveExpressionContext *ctx) = 0;

  virtual void enterShiftExpression(CPP14Parser::ShiftExpressionContext *ctx) = 0;
  virtual void exitShiftExpression(CPP14Parser::ShiftExpressionContext *ctx) = 0;

  virtual void enterShiftOperator(CPP14Parser::ShiftOperatorContext *ctx) = 0;
  virtual void exitShiftOperator(CPP14Parser::ShiftOperatorContext *ctx) = 0;

  virtual void enterRelationalExpression(CPP14Parser::RelationalExpressionContext *ctx) = 0;
  virtual void exitRelationalExpression(CPP14Parser::RelationalExpressionContext *ctx) = 0;

  virtual void enterEqualityExpression(CPP14Parser::EqualityExpressionContext *ctx) = 0;
  virtual void exitEqualityExpression(CPP14Parser::EqualityExpressionContext *ctx) = 0;

  virtual void enterAndExpression(CPP14Parser::AndExpressionContext *ctx) = 0;
  virtual void exitAndExpression(CPP14Parser::AndExpressionContext *ctx) = 0;

  virtual void enterExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext *ctx) = 0;
  virtual void exitExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext *ctx) = 0;

  virtual void enterInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext *ctx) = 0;
  virtual void exitInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext *ctx) = 0;

  virtual void enterLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext *ctx) = 0;
  virtual void exitLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext *ctx) = 0;

  virtual void enterLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext *ctx) = 0;
  virtual void exitLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext *ctx) = 0;

  virtual void enterConditionalExpression(CPP14Parser::ConditionalExpressionContext *ctx) = 0;
  virtual void exitConditionalExpression(CPP14Parser::ConditionalExpressionContext *ctx) = 0;

  virtual void enterAssignmentExpression(CPP14Parser::AssignmentExpressionContext *ctx) = 0;
  virtual void exitAssignmentExpression(CPP14Parser::AssignmentExpressionContext *ctx) = 0;

  virtual void enterAssignmentOperator(CPP14Parser::AssignmentOperatorContext *ctx) = 0;
  virtual void exitAssignmentOperator(CPP14Parser::AssignmentOperatorContext *ctx) = 0;

  virtual void enterExpression(CPP14Parser::ExpressionContext *ctx) = 0;
  virtual void exitExpression(CPP14Parser::ExpressionContext *ctx) = 0;

  virtual void enterConstantExpression(CPP14Parser::ConstantExpressionContext *ctx) = 0;
  virtual void exitConstantExpression(CPP14Parser::ConstantExpressionContext *ctx) = 0;

  virtual void enterStatement(CPP14Parser::StatementContext *ctx) = 0;
  virtual void exitStatement(CPP14Parser::StatementContext *ctx) = 0;

  virtual void enterLabeledStatement(CPP14Parser::LabeledStatementContext *ctx) = 0;
  virtual void exitLabeledStatement(CPP14Parser::LabeledStatementContext *ctx) = 0;

  virtual void enterExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) = 0;
  virtual void exitExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) = 0;

  virtual void enterCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) = 0;
  virtual void exitCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) = 0;

  virtual void enterStatementSeq(CPP14Parser::StatementSeqContext *ctx) = 0;
  virtual void exitStatementSeq(CPP14Parser::StatementSeqContext *ctx) = 0;

  virtual void enterSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) = 0;
  virtual void exitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) = 0;

  virtual void enterCondition(CPP14Parser::ConditionContext *ctx) = 0;
  virtual void exitCondition(CPP14Parser::ConditionContext *ctx) = 0;

  virtual void enterIterationStatement(CPP14Parser::IterationStatementContext *ctx) = 0;
  virtual void exitIterationStatement(CPP14Parser::IterationStatementContext *ctx) = 0;

  virtual void enterForInitStatement(CPP14Parser::ForInitStatementContext *ctx) = 0;
  virtual void exitForInitStatement(CPP14Parser::ForInitStatementContext *ctx) = 0;

  virtual void enterForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext *ctx) = 0;
  virtual void exitForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext *ctx) = 0;

  virtual void enterForRangeInitializer(CPP14Parser::ForRangeInitializerContext *ctx) = 0;
  virtual void exitForRangeInitializer(CPP14Parser::ForRangeInitializerContext *ctx) = 0;

  virtual void enterJumpStatement(CPP14Parser::JumpStatementContext *ctx) = 0;
  virtual void exitJumpStatement(CPP14Parser::JumpStatementContext *ctx) = 0;

  virtual void enterDeclarationStatement(CPP14Parser::DeclarationStatementContext *ctx) = 0;
  virtual void exitDeclarationStatement(CPP14Parser::DeclarationStatementContext *ctx) = 0;

  virtual void enterDeclarationseq(CPP14Parser::DeclarationseqContext *ctx) = 0;
  virtual void exitDeclarationseq(CPP14Parser::DeclarationseqContext *ctx) = 0;

  virtual void enterDeclaration(CPP14Parser::DeclarationContext *ctx) = 0;
  virtual void exitDeclaration(CPP14Parser::DeclarationContext *ctx) = 0;

  virtual void enterBlockDeclaration(CPP14Parser::BlockDeclarationContext *ctx) = 0;
  virtual void exitBlockDeclaration(CPP14Parser::BlockDeclarationContext *ctx) = 0;

  virtual void enterAliasDeclaration(CPP14Parser::AliasDeclarationContext *ctx) = 0;
  virtual void exitAliasDeclaration(CPP14Parser::AliasDeclarationContext *ctx) = 0;

  virtual void enterSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) = 0;
  virtual void exitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) = 0;

  virtual void enterStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext *ctx) = 0;
  virtual void exitStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext *ctx) = 0;

  virtual void enterEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context *ctx) = 0;
  virtual void exitEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context *ctx) = 0;

  virtual void enterAttributeDeclaration(CPP14Parser::AttributeDeclarationContext *ctx) = 0;
  virtual void exitAttributeDeclaration(CPP14Parser::AttributeDeclarationContext *ctx) = 0;

  virtual void enterDeclSpecifier(CPP14Parser::DeclSpecifierContext *ctx) = 0;
  virtual void exitDeclSpecifier(CPP14Parser::DeclSpecifierContext *ctx) = 0;

  virtual void enterDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext *ctx) = 0;
  virtual void exitDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext *ctx) = 0;

  virtual void enterStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext *ctx) = 0;
  virtual void exitStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext *ctx) = 0;

  virtual void enterFunctionSpecifier(CPP14Parser::FunctionSpecifierContext *ctx) = 0;
  virtual void exitFunctionSpecifier(CPP14Parser::FunctionSpecifierContext *ctx) = 0;

  virtual void enterTypedefName(CPP14Parser::TypedefNameContext *ctx) = 0;
  virtual void exitTypedefName(CPP14Parser::TypedefNameContext *ctx) = 0;

  virtual void enterTypeSpecifier(CPP14Parser::TypeSpecifierContext *ctx) = 0;
  virtual void exitTypeSpecifier(CPP14Parser::TypeSpecifierContext *ctx) = 0;

  virtual void enterTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext *ctx) = 0;
  virtual void exitTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext *ctx) = 0;

  virtual void enterTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext *ctx) = 0;
  virtual void exitTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext *ctx) = 0;

  virtual void enterTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext *ctx) = 0;
  virtual void exitTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext *ctx) = 0;

  virtual void enterSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext *ctx) = 0;
  virtual void exitSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext *ctx) = 0;

  virtual void enterSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext *ctx) = 0;
  virtual void exitSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext *ctx) = 0;

  virtual void enterSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) = 0;
  virtual void exitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) = 0;

  virtual void enterTheTypeName(CPP14Parser::TheTypeNameContext *ctx) = 0;
  virtual void exitTheTypeName(CPP14Parser::TheTypeNameContext *ctx) = 0;

  virtual void enterDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext *ctx) = 0;
  virtual void exitDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext *ctx) = 0;

  virtual void enterElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext *ctx) = 0;
  virtual void exitElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext *ctx) = 0;

  virtual void enterEnumName(CPP14Parser::EnumNameContext *ctx) = 0;
  virtual void exitEnumName(CPP14Parser::EnumNameContext *ctx) = 0;

  virtual void enterEnumSpecifier(CPP14Parser::EnumSpecifierContext *ctx) = 0;
  virtual void exitEnumSpecifier(CPP14Parser::EnumSpecifierContext *ctx) = 0;

  virtual void enterEnumHead(CPP14Parser::EnumHeadContext *ctx) = 0;
  virtual void exitEnumHead(CPP14Parser::EnumHeadContext *ctx) = 0;

  virtual void enterOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext *ctx) = 0;
  virtual void exitOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext *ctx) = 0;

  virtual void enterEnumkey(CPP14Parser::EnumkeyContext *ctx) = 0;
  virtual void exitEnumkey(CPP14Parser::EnumkeyContext *ctx) = 0;

  virtual void enterEnumbase(CPP14Parser::EnumbaseContext *ctx) = 0;
  virtual void exitEnumbase(CPP14Parser::EnumbaseContext *ctx) = 0;

  virtual void enterEnumeratorList(CPP14Parser::EnumeratorListContext *ctx) = 0;
  virtual void exitEnumeratorList(CPP14Parser::EnumeratorListContext *ctx) = 0;

  virtual void enterEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext *ctx) = 0;
  virtual void exitEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext *ctx) = 0;

  virtual void enterEnumerator(CPP14Parser::EnumeratorContext *ctx) = 0;
  virtual void exitEnumerator(CPP14Parser::EnumeratorContext *ctx) = 0;

  virtual void enterNamespaceName(CPP14Parser::NamespaceNameContext *ctx) = 0;
  virtual void exitNamespaceName(CPP14Parser::NamespaceNameContext *ctx) = 0;

  virtual void enterOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext *ctx) = 0;
  virtual void exitOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext *ctx) = 0;

  virtual void enterNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext *ctx) = 0;
  virtual void exitNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext *ctx) = 0;

  virtual void enterNamespaceAlias(CPP14Parser::NamespaceAliasContext *ctx) = 0;
  virtual void exitNamespaceAlias(CPP14Parser::NamespaceAliasContext *ctx) = 0;

  virtual void enterNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext *ctx) = 0;
  virtual void exitNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext *ctx) = 0;

  virtual void enterQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext *ctx) = 0;
  virtual void exitQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext *ctx) = 0;

  virtual void enterUsingDeclaration(CPP14Parser::UsingDeclarationContext *ctx) = 0;
  virtual void exitUsingDeclaration(CPP14Parser::UsingDeclarationContext *ctx) = 0;

  virtual void enterUsingDirective(CPP14Parser::UsingDirectiveContext *ctx) = 0;
  virtual void exitUsingDirective(CPP14Parser::UsingDirectiveContext *ctx) = 0;

  virtual void enterAsmDefinition(CPP14Parser::AsmDefinitionContext *ctx) = 0;
  virtual void exitAsmDefinition(CPP14Parser::AsmDefinitionContext *ctx) = 0;

  virtual void enterLinkageSpecification(CPP14Parser::LinkageSpecificationContext *ctx) = 0;
  virtual void exitLinkageSpecification(CPP14Parser::LinkageSpecificationContext *ctx) = 0;

  virtual void enterAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext *ctx) = 0;
  virtual void exitAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext *ctx) = 0;

  virtual void enterAttributeSpecifier(CPP14Parser::AttributeSpecifierContext *ctx) = 0;
  virtual void exitAttributeSpecifier(CPP14Parser::AttributeSpecifierContext *ctx) = 0;

  virtual void enterAlignmentspecifier(CPP14Parser::AlignmentspecifierContext *ctx) = 0;
  virtual void exitAlignmentspecifier(CPP14Parser::AlignmentspecifierContext *ctx) = 0;

  virtual void enterAttributeList(CPP14Parser::AttributeListContext *ctx) = 0;
  virtual void exitAttributeList(CPP14Parser::AttributeListContext *ctx) = 0;

  virtual void enterAttribute(CPP14Parser::AttributeContext *ctx) = 0;
  virtual void exitAttribute(CPP14Parser::AttributeContext *ctx) = 0;

  virtual void enterAttributeNamespace(CPP14Parser::AttributeNamespaceContext *ctx) = 0;
  virtual void exitAttributeNamespace(CPP14Parser::AttributeNamespaceContext *ctx) = 0;

  virtual void enterAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext *ctx) = 0;
  virtual void exitAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext *ctx) = 0;

  virtual void enterBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext *ctx) = 0;
  virtual void exitBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext *ctx) = 0;

  virtual void enterBalancedtoken(CPP14Parser::BalancedtokenContext *ctx) = 0;
  virtual void exitBalancedtoken(CPP14Parser::BalancedtokenContext *ctx) = 0;

  virtual void enterInitDeclaratorList(CPP14Parser::InitDeclaratorListContext *ctx) = 0;
  virtual void exitInitDeclaratorList(CPP14Parser::InitDeclaratorListContext *ctx) = 0;

  virtual void enterInitDeclarator(CPP14Parser::InitDeclaratorContext *ctx) = 0;
  virtual void exitInitDeclarator(CPP14Parser::InitDeclaratorContext *ctx) = 0;

  virtual void enterDeclarator(CPP14Parser::DeclaratorContext *ctx) = 0;
  virtual void exitDeclarator(CPP14Parser::DeclaratorContext *ctx) = 0;

  virtual void enterPointerDeclarator(CPP14Parser::PointerDeclaratorContext *ctx) = 0;
  virtual void exitPointerDeclarator(CPP14Parser::PointerDeclaratorContext *ctx) = 0;

  virtual void enterNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext *ctx) = 0;
  virtual void exitNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext *ctx) = 0;

  virtual void enterParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext *ctx) = 0;
  virtual void exitParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext *ctx) = 0;

  virtual void enterTrailingReturnType(CPP14Parser::TrailingReturnTypeContext *ctx) = 0;
  virtual void exitTrailingReturnType(CPP14Parser::TrailingReturnTypeContext *ctx) = 0;

  virtual void enterPointerOperator(CPP14Parser::PointerOperatorContext *ctx) = 0;
  virtual void exitPointerOperator(CPP14Parser::PointerOperatorContext *ctx) = 0;

  virtual void enterCvqualifierseq(CPP14Parser::CvqualifierseqContext *ctx) = 0;
  virtual void exitCvqualifierseq(CPP14Parser::CvqualifierseqContext *ctx) = 0;

  virtual void enterCvQualifier(CPP14Parser::CvQualifierContext *ctx) = 0;
  virtual void exitCvQualifier(CPP14Parser::CvQualifierContext *ctx) = 0;

  virtual void enterRefqualifier(CPP14Parser::RefqualifierContext *ctx) = 0;
  virtual void exitRefqualifier(CPP14Parser::RefqualifierContext *ctx) = 0;

  virtual void enterDeclaratorid(CPP14Parser::DeclaratoridContext *ctx) = 0;
  virtual void exitDeclaratorid(CPP14Parser::DeclaratoridContext *ctx) = 0;

  virtual void enterTheTypeId(CPP14Parser::TheTypeIdContext *ctx) = 0;
  virtual void exitTheTypeId(CPP14Parser::TheTypeIdContext *ctx) = 0;

  virtual void enterAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext *ctx) = 0;
  virtual void exitAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext *ctx) = 0;

  virtual void enterPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext *ctx) = 0;
  virtual void exitPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext *ctx) = 0;

  virtual void enterNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext *ctx) = 0;
  virtual void exitNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext *ctx) = 0;

  virtual void enterAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext *ctx) = 0;
  virtual void exitAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext *ctx) = 0;

  virtual void enterNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext *ctx) = 0;
  virtual void exitNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext *ctx) = 0;

  virtual void enterParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext *ctx) = 0;
  virtual void exitParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext *ctx) = 0;

  virtual void enterParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext *ctx) = 0;
  virtual void exitParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext *ctx) = 0;

  virtual void enterParameterDeclaration(CPP14Parser::ParameterDeclarationContext *ctx) = 0;
  virtual void exitParameterDeclaration(CPP14Parser::ParameterDeclarationContext *ctx) = 0;

  virtual void enterFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) = 0;
  virtual void exitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) = 0;

  virtual void enterFunctionBody(CPP14Parser::FunctionBodyContext *ctx) = 0;
  virtual void exitFunctionBody(CPP14Parser::FunctionBodyContext *ctx) = 0;

  virtual void enterInitializer(CPP14Parser::InitializerContext *ctx) = 0;
  virtual void exitInitializer(CPP14Parser::InitializerContext *ctx) = 0;

  virtual void enterBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext *ctx) = 0;
  virtual void exitBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext *ctx) = 0;

  virtual void enterInitializerClause(CPP14Parser::InitializerClauseContext *ctx) = 0;
  virtual void exitInitializerClause(CPP14Parser::InitializerClauseContext *ctx) = 0;

  virtual void enterInitializerList(CPP14Parser::InitializerListContext *ctx) = 0;
  virtual void exitInitializerList(CPP14Parser::InitializerListContext *ctx) = 0;

  virtual void enterBracedInitList(CPP14Parser::BracedInitListContext *ctx) = 0;
  virtual void exitBracedInitList(CPP14Parser::BracedInitListContext *ctx) = 0;

  virtual void enterClassName(CPP14Parser::ClassNameContext *ctx) = 0;
  virtual void exitClassName(CPP14Parser::ClassNameContext *ctx) = 0;

  virtual void enterClassSpecifier(CPP14Parser::ClassSpecifierContext *ctx) = 0;
  virtual void exitClassSpecifier(CPP14Parser::ClassSpecifierContext *ctx) = 0;

  virtual void enterClassHead(CPP14Parser::ClassHeadContext *ctx) = 0;
  virtual void exitClassHead(CPP14Parser::ClassHeadContext *ctx) = 0;

  virtual void enterClassHeadName(CPP14Parser::ClassHeadNameContext *ctx) = 0;
  virtual void exitClassHeadName(CPP14Parser::ClassHeadNameContext *ctx) = 0;

  virtual void enterClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext *ctx) = 0;
  virtual void exitClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext *ctx) = 0;

  virtual void enterClassKey(CPP14Parser::ClassKeyContext *ctx) = 0;
  virtual void exitClassKey(CPP14Parser::ClassKeyContext *ctx) = 0;

  virtual void enterMemberSpecification(CPP14Parser::MemberSpecificationContext *ctx) = 0;
  virtual void exitMemberSpecification(CPP14Parser::MemberSpecificationContext *ctx) = 0;

  virtual void enterMemberdeclaration(CPP14Parser::MemberdeclarationContext *ctx) = 0;
  virtual void exitMemberdeclaration(CPP14Parser::MemberdeclarationContext *ctx) = 0;

  virtual void enterMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext *ctx) = 0;
  virtual void exitMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext *ctx) = 0;

  virtual void enterMemberDeclarator(CPP14Parser::MemberDeclaratorContext *ctx) = 0;
  virtual void exitMemberDeclarator(CPP14Parser::MemberDeclaratorContext *ctx) = 0;

  virtual void enterVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext *ctx) = 0;
  virtual void exitVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext *ctx) = 0;

  virtual void enterVirtualSpecifier(CPP14Parser::VirtualSpecifierContext *ctx) = 0;
  virtual void exitVirtualSpecifier(CPP14Parser::VirtualSpecifierContext *ctx) = 0;

  virtual void enterPureSpecifier(CPP14Parser::PureSpecifierContext *ctx) = 0;
  virtual void exitPureSpecifier(CPP14Parser::PureSpecifierContext *ctx) = 0;

  virtual void enterBaseClause(CPP14Parser::BaseClauseContext *ctx) = 0;
  virtual void exitBaseClause(CPP14Parser::BaseClauseContext *ctx) = 0;

  virtual void enterBaseSpecifierList(CPP14Parser::BaseSpecifierListContext *ctx) = 0;
  virtual void exitBaseSpecifierList(CPP14Parser::BaseSpecifierListContext *ctx) = 0;

  virtual void enterBaseSpecifier(CPP14Parser::BaseSpecifierContext *ctx) = 0;
  virtual void exitBaseSpecifier(CPP14Parser::BaseSpecifierContext *ctx) = 0;

  virtual void enterClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext *ctx) = 0;
  virtual void exitClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext *ctx) = 0;

  virtual void enterBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext *ctx) = 0;
  virtual void exitBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext *ctx) = 0;

  virtual void enterAccessSpecifier(CPP14Parser::AccessSpecifierContext *ctx) = 0;
  virtual void exitAccessSpecifier(CPP14Parser::AccessSpecifierContext *ctx) = 0;

  virtual void enterConversionFunctionId(CPP14Parser::ConversionFunctionIdContext *ctx) = 0;
  virtual void exitConversionFunctionId(CPP14Parser::ConversionFunctionIdContext *ctx) = 0;

  virtual void enterConversionTypeId(CPP14Parser::ConversionTypeIdContext *ctx) = 0;
  virtual void exitConversionTypeId(CPP14Parser::ConversionTypeIdContext *ctx) = 0;

  virtual void enterConversionDeclarator(CPP14Parser::ConversionDeclaratorContext *ctx) = 0;
  virtual void exitConversionDeclarator(CPP14Parser::ConversionDeclaratorContext *ctx) = 0;

  virtual void enterConstructorInitializer(CPP14Parser::ConstructorInitializerContext *ctx) = 0;
  virtual void exitConstructorInitializer(CPP14Parser::ConstructorInitializerContext *ctx) = 0;

  virtual void enterMemInitializerList(CPP14Parser::MemInitializerListContext *ctx) = 0;
  virtual void exitMemInitializerList(CPP14Parser::MemInitializerListContext *ctx) = 0;

  virtual void enterMemInitializer(CPP14Parser::MemInitializerContext *ctx) = 0;
  virtual void exitMemInitializer(CPP14Parser::MemInitializerContext *ctx) = 0;

  virtual void enterMeminitializerid(CPP14Parser::MeminitializeridContext *ctx) = 0;
  virtual void exitMeminitializerid(CPP14Parser::MeminitializeridContext *ctx) = 0;

  virtual void enterOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext *ctx) = 0;
  virtual void exitOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext *ctx) = 0;

  virtual void enterLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext *ctx) = 0;
  virtual void exitLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext *ctx) = 0;

  virtual void enterTemplateDeclaration(CPP14Parser::TemplateDeclarationContext *ctx) = 0;
  virtual void exitTemplateDeclaration(CPP14Parser::TemplateDeclarationContext *ctx) = 0;

  virtual void enterTemplateparameterList(CPP14Parser::TemplateparameterListContext *ctx) = 0;
  virtual void exitTemplateparameterList(CPP14Parser::TemplateparameterListContext *ctx) = 0;

  virtual void enterTemplateParameter(CPP14Parser::TemplateParameterContext *ctx) = 0;
  virtual void exitTemplateParameter(CPP14Parser::TemplateParameterContext *ctx) = 0;

  virtual void enterTypeParameter(CPP14Parser::TypeParameterContext *ctx) = 0;
  virtual void exitTypeParameter(CPP14Parser::TypeParameterContext *ctx) = 0;

  virtual void enterSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext *ctx) = 0;
  virtual void exitSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext *ctx) = 0;

  virtual void enterTemplateId(CPP14Parser::TemplateIdContext *ctx) = 0;
  virtual void exitTemplateId(CPP14Parser::TemplateIdContext *ctx) = 0;

  virtual void enterTemplateName(CPP14Parser::TemplateNameContext *ctx) = 0;
  virtual void exitTemplateName(CPP14Parser::TemplateNameContext *ctx) = 0;

  virtual void enterTemplateArgumentList(CPP14Parser::TemplateArgumentListContext *ctx) = 0;
  virtual void exitTemplateArgumentList(CPP14Parser::TemplateArgumentListContext *ctx) = 0;

  virtual void enterTemplateArgument(CPP14Parser::TemplateArgumentContext *ctx) = 0;
  virtual void exitTemplateArgument(CPP14Parser::TemplateArgumentContext *ctx) = 0;

  virtual void enterTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext *ctx) = 0;
  virtual void exitTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext *ctx) = 0;

  virtual void enterExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext *ctx) = 0;
  virtual void exitExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext *ctx) = 0;

  virtual void enterExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext *ctx) = 0;
  virtual void exitExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext *ctx) = 0;

  virtual void enterTryBlock(CPP14Parser::TryBlockContext *ctx) = 0;
  virtual void exitTryBlock(CPP14Parser::TryBlockContext *ctx) = 0;

  virtual void enterFunctionTryBlock(CPP14Parser::FunctionTryBlockContext *ctx) = 0;
  virtual void exitFunctionTryBlock(CPP14Parser::FunctionTryBlockContext *ctx) = 0;

  virtual void enterHandlerSeq(CPP14Parser::HandlerSeqContext *ctx) = 0;
  virtual void exitHandlerSeq(CPP14Parser::HandlerSeqContext *ctx) = 0;

  virtual void enterHandler(CPP14Parser::HandlerContext *ctx) = 0;
  virtual void exitHandler(CPP14Parser::HandlerContext *ctx) = 0;

  virtual void enterExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext *ctx) = 0;
  virtual void exitExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext *ctx) = 0;

  virtual void enterThrowExpression(CPP14Parser::ThrowExpressionContext *ctx) = 0;
  virtual void exitThrowExpression(CPP14Parser::ThrowExpressionContext *ctx) = 0;

  virtual void enterExceptionSpecification(CPP14Parser::ExceptionSpecificationContext *ctx) = 0;
  virtual void exitExceptionSpecification(CPP14Parser::ExceptionSpecificationContext *ctx) = 0;

  virtual void enterDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext *ctx) = 0;
  virtual void exitDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext *ctx) = 0;

  virtual void enterTypeIdList(CPP14Parser::TypeIdListContext *ctx) = 0;
  virtual void exitTypeIdList(CPP14Parser::TypeIdListContext *ctx) = 0;

  virtual void enterNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext *ctx) = 0;
  virtual void exitNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext *ctx) = 0;

  virtual void enterTheOperator(CPP14Parser::TheOperatorContext *ctx) = 0;
  virtual void exitTheOperator(CPP14Parser::TheOperatorContext *ctx) = 0;

  virtual void enterLiteral(CPP14Parser::LiteralContext *ctx) = 0;
  virtual void exitLiteral(CPP14Parser::LiteralContext *ctx) = 0;


};

