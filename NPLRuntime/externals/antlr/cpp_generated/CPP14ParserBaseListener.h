#include "CPP14ParserBase.h"

// Generated from CPP14Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "CPP14ParserListener.h"


/**
 * This class provides an empty implementation of CPP14ParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  CPP14ParserBaseListener : public CPP14ParserListener {
public:

  virtual void enterTranslationUnit(CPP14Parser::TranslationUnitContext * /*ctx*/) override { }
  virtual void exitTranslationUnit(CPP14Parser::TranslationUnitContext * /*ctx*/) override { }

  virtual void enterPrimaryExpression(CPP14Parser::PrimaryExpressionContext * /*ctx*/) override { }
  virtual void exitPrimaryExpression(CPP14Parser::PrimaryExpressionContext * /*ctx*/) override { }

  virtual void enterIdExpression(CPP14Parser::IdExpressionContext * /*ctx*/) override { }
  virtual void exitIdExpression(CPP14Parser::IdExpressionContext * /*ctx*/) override { }

  virtual void enterUnqualifiedId(CPP14Parser::UnqualifiedIdContext * /*ctx*/) override { }
  virtual void exitUnqualifiedId(CPP14Parser::UnqualifiedIdContext * /*ctx*/) override { }

  virtual void enterQualifiedId(CPP14Parser::QualifiedIdContext * /*ctx*/) override { }
  virtual void exitQualifiedId(CPP14Parser::QualifiedIdContext * /*ctx*/) override { }

  virtual void enterNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext * /*ctx*/) override { }
  virtual void exitNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext * /*ctx*/) override { }

  virtual void enterLambdaExpression(CPP14Parser::LambdaExpressionContext * /*ctx*/) override { }
  virtual void exitLambdaExpression(CPP14Parser::LambdaExpressionContext * /*ctx*/) override { }

  virtual void enterLambdaIntroducer(CPP14Parser::LambdaIntroducerContext * /*ctx*/) override { }
  virtual void exitLambdaIntroducer(CPP14Parser::LambdaIntroducerContext * /*ctx*/) override { }

  virtual void enterLambdaCapture(CPP14Parser::LambdaCaptureContext * /*ctx*/) override { }
  virtual void exitLambdaCapture(CPP14Parser::LambdaCaptureContext * /*ctx*/) override { }

  virtual void enterCaptureDefault(CPP14Parser::CaptureDefaultContext * /*ctx*/) override { }
  virtual void exitCaptureDefault(CPP14Parser::CaptureDefaultContext * /*ctx*/) override { }

  virtual void enterCaptureList(CPP14Parser::CaptureListContext * /*ctx*/) override { }
  virtual void exitCaptureList(CPP14Parser::CaptureListContext * /*ctx*/) override { }

  virtual void enterCapture(CPP14Parser::CaptureContext * /*ctx*/) override { }
  virtual void exitCapture(CPP14Parser::CaptureContext * /*ctx*/) override { }

  virtual void enterSimpleCapture(CPP14Parser::SimpleCaptureContext * /*ctx*/) override { }
  virtual void exitSimpleCapture(CPP14Parser::SimpleCaptureContext * /*ctx*/) override { }

  virtual void enterInitcapture(CPP14Parser::InitcaptureContext * /*ctx*/) override { }
  virtual void exitInitcapture(CPP14Parser::InitcaptureContext * /*ctx*/) override { }

  virtual void enterLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext * /*ctx*/) override { }
  virtual void exitLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext * /*ctx*/) override { }

  virtual void enterPostfixExpression(CPP14Parser::PostfixExpressionContext * /*ctx*/) override { }
  virtual void exitPostfixExpression(CPP14Parser::PostfixExpressionContext * /*ctx*/) override { }

  virtual void enterTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext * /*ctx*/) override { }
  virtual void exitTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext * /*ctx*/) override { }

  virtual void enterExpressionList(CPP14Parser::ExpressionListContext * /*ctx*/) override { }
  virtual void exitExpressionList(CPP14Parser::ExpressionListContext * /*ctx*/) override { }

  virtual void enterPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext * /*ctx*/) override { }
  virtual void exitPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext * /*ctx*/) override { }

  virtual void enterUnaryExpression(CPP14Parser::UnaryExpressionContext * /*ctx*/) override { }
  virtual void exitUnaryExpression(CPP14Parser::UnaryExpressionContext * /*ctx*/) override { }

  virtual void enterUnaryOperator(CPP14Parser::UnaryOperatorContext * /*ctx*/) override { }
  virtual void exitUnaryOperator(CPP14Parser::UnaryOperatorContext * /*ctx*/) override { }

  virtual void enterNewExpression_(CPP14Parser::NewExpression_Context * /*ctx*/) override { }
  virtual void exitNewExpression_(CPP14Parser::NewExpression_Context * /*ctx*/) override { }

  virtual void enterNewPlacement(CPP14Parser::NewPlacementContext * /*ctx*/) override { }
  virtual void exitNewPlacement(CPP14Parser::NewPlacementContext * /*ctx*/) override { }

  virtual void enterNewTypeId(CPP14Parser::NewTypeIdContext * /*ctx*/) override { }
  virtual void exitNewTypeId(CPP14Parser::NewTypeIdContext * /*ctx*/) override { }

  virtual void enterNewDeclarator_(CPP14Parser::NewDeclarator_Context * /*ctx*/) override { }
  virtual void exitNewDeclarator_(CPP14Parser::NewDeclarator_Context * /*ctx*/) override { }

  virtual void enterNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext * /*ctx*/) override { }
  virtual void exitNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext * /*ctx*/) override { }

  virtual void enterNewInitializer_(CPP14Parser::NewInitializer_Context * /*ctx*/) override { }
  virtual void exitNewInitializer_(CPP14Parser::NewInitializer_Context * /*ctx*/) override { }

  virtual void enterDeleteExpression(CPP14Parser::DeleteExpressionContext * /*ctx*/) override { }
  virtual void exitDeleteExpression(CPP14Parser::DeleteExpressionContext * /*ctx*/) override { }

  virtual void enterNoExceptExpression(CPP14Parser::NoExceptExpressionContext * /*ctx*/) override { }
  virtual void exitNoExceptExpression(CPP14Parser::NoExceptExpressionContext * /*ctx*/) override { }

  virtual void enterCastExpression(CPP14Parser::CastExpressionContext * /*ctx*/) override { }
  virtual void exitCastExpression(CPP14Parser::CastExpressionContext * /*ctx*/) override { }

  virtual void enterPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext * /*ctx*/) override { }
  virtual void exitPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext * /*ctx*/) override { }

  virtual void enterMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext * /*ctx*/) override { }
  virtual void exitMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext * /*ctx*/) override { }

  virtual void enterAdditiveExpression(CPP14Parser::AdditiveExpressionContext * /*ctx*/) override { }
  virtual void exitAdditiveExpression(CPP14Parser::AdditiveExpressionContext * /*ctx*/) override { }

  virtual void enterShiftExpression(CPP14Parser::ShiftExpressionContext * /*ctx*/) override { }
  virtual void exitShiftExpression(CPP14Parser::ShiftExpressionContext * /*ctx*/) override { }

  virtual void enterShiftOperator(CPP14Parser::ShiftOperatorContext * /*ctx*/) override { }
  virtual void exitShiftOperator(CPP14Parser::ShiftOperatorContext * /*ctx*/) override { }

  virtual void enterRelationalExpression(CPP14Parser::RelationalExpressionContext * /*ctx*/) override { }
  virtual void exitRelationalExpression(CPP14Parser::RelationalExpressionContext * /*ctx*/) override { }

  virtual void enterEqualityExpression(CPP14Parser::EqualityExpressionContext * /*ctx*/) override { }
  virtual void exitEqualityExpression(CPP14Parser::EqualityExpressionContext * /*ctx*/) override { }

  virtual void enterAndExpression(CPP14Parser::AndExpressionContext * /*ctx*/) override { }
  virtual void exitAndExpression(CPP14Parser::AndExpressionContext * /*ctx*/) override { }

  virtual void enterExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext * /*ctx*/) override { }
  virtual void exitExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext * /*ctx*/) override { }

  virtual void enterInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext * /*ctx*/) override { }
  virtual void exitInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext * /*ctx*/) override { }

  virtual void enterLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext * /*ctx*/) override { }
  virtual void exitLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext * /*ctx*/) override { }

  virtual void enterLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext * /*ctx*/) override { }
  virtual void exitLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext * /*ctx*/) override { }

  virtual void enterConditionalExpression(CPP14Parser::ConditionalExpressionContext * /*ctx*/) override { }
  virtual void exitConditionalExpression(CPP14Parser::ConditionalExpressionContext * /*ctx*/) override { }

  virtual void enterAssignmentExpression(CPP14Parser::AssignmentExpressionContext * /*ctx*/) override { }
  virtual void exitAssignmentExpression(CPP14Parser::AssignmentExpressionContext * /*ctx*/) override { }

  virtual void enterAssignmentOperator(CPP14Parser::AssignmentOperatorContext * /*ctx*/) override { }
  virtual void exitAssignmentOperator(CPP14Parser::AssignmentOperatorContext * /*ctx*/) override { }

  virtual void enterExpression(CPP14Parser::ExpressionContext * /*ctx*/) override { }
  virtual void exitExpression(CPP14Parser::ExpressionContext * /*ctx*/) override { }

  virtual void enterConstantExpression(CPP14Parser::ConstantExpressionContext * /*ctx*/) override { }
  virtual void exitConstantExpression(CPP14Parser::ConstantExpressionContext * /*ctx*/) override { }

  virtual void enterStatement(CPP14Parser::StatementContext * /*ctx*/) override { }
  virtual void exitStatement(CPP14Parser::StatementContext * /*ctx*/) override { }

  virtual void enterLabeledStatement(CPP14Parser::LabeledStatementContext * /*ctx*/) override { }
  virtual void exitLabeledStatement(CPP14Parser::LabeledStatementContext * /*ctx*/) override { }

  virtual void enterExpressionStatement(CPP14Parser::ExpressionStatementContext * /*ctx*/) override { }
  virtual void exitExpressionStatement(CPP14Parser::ExpressionStatementContext * /*ctx*/) override { }

  virtual void enterCompoundStatement(CPP14Parser::CompoundStatementContext * /*ctx*/) override { }
  virtual void exitCompoundStatement(CPP14Parser::CompoundStatementContext * /*ctx*/) override { }

  virtual void enterStatementSeq(CPP14Parser::StatementSeqContext * /*ctx*/) override { }
  virtual void exitStatementSeq(CPP14Parser::StatementSeqContext * /*ctx*/) override { }

  virtual void enterSelectionStatement(CPP14Parser::SelectionStatementContext * /*ctx*/) override { }
  virtual void exitSelectionStatement(CPP14Parser::SelectionStatementContext * /*ctx*/) override { }

  virtual void enterCondition(CPP14Parser::ConditionContext * /*ctx*/) override { }
  virtual void exitCondition(CPP14Parser::ConditionContext * /*ctx*/) override { }

  virtual void enterIterationStatement(CPP14Parser::IterationStatementContext * /*ctx*/) override { }
  virtual void exitIterationStatement(CPP14Parser::IterationStatementContext * /*ctx*/) override { }

  virtual void enterForInitStatement(CPP14Parser::ForInitStatementContext * /*ctx*/) override { }
  virtual void exitForInitStatement(CPP14Parser::ForInitStatementContext * /*ctx*/) override { }

  virtual void enterForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext * /*ctx*/) override { }
  virtual void exitForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext * /*ctx*/) override { }

  virtual void enterForRangeInitializer(CPP14Parser::ForRangeInitializerContext * /*ctx*/) override { }
  virtual void exitForRangeInitializer(CPP14Parser::ForRangeInitializerContext * /*ctx*/) override { }

  virtual void enterJumpStatement(CPP14Parser::JumpStatementContext * /*ctx*/) override { }
  virtual void exitJumpStatement(CPP14Parser::JumpStatementContext * /*ctx*/) override { }

  virtual void enterDeclarationStatement(CPP14Parser::DeclarationStatementContext * /*ctx*/) override { }
  virtual void exitDeclarationStatement(CPP14Parser::DeclarationStatementContext * /*ctx*/) override { }

  virtual void enterDeclarationseq(CPP14Parser::DeclarationseqContext * /*ctx*/) override { }
  virtual void exitDeclarationseq(CPP14Parser::DeclarationseqContext * /*ctx*/) override { }

  virtual void enterDeclaration(CPP14Parser::DeclarationContext * /*ctx*/) override { }
  virtual void exitDeclaration(CPP14Parser::DeclarationContext * /*ctx*/) override { }

  virtual void enterBlockDeclaration(CPP14Parser::BlockDeclarationContext * /*ctx*/) override { }
  virtual void exitBlockDeclaration(CPP14Parser::BlockDeclarationContext * /*ctx*/) override { }

  virtual void enterAliasDeclaration(CPP14Parser::AliasDeclarationContext * /*ctx*/) override { }
  virtual void exitAliasDeclaration(CPP14Parser::AliasDeclarationContext * /*ctx*/) override { }

  virtual void enterSimpleDeclaration(CPP14Parser::SimpleDeclarationContext * /*ctx*/) override { }
  virtual void exitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext * /*ctx*/) override { }

  virtual void enterStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext * /*ctx*/) override { }
  virtual void exitStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext * /*ctx*/) override { }

  virtual void enterEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context * /*ctx*/) override { }
  virtual void exitEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context * /*ctx*/) override { }

  virtual void enterAttributeDeclaration(CPP14Parser::AttributeDeclarationContext * /*ctx*/) override { }
  virtual void exitAttributeDeclaration(CPP14Parser::AttributeDeclarationContext * /*ctx*/) override { }

  virtual void enterDeclSpecifier(CPP14Parser::DeclSpecifierContext * /*ctx*/) override { }
  virtual void exitDeclSpecifier(CPP14Parser::DeclSpecifierContext * /*ctx*/) override { }

  virtual void enterDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext * /*ctx*/) override { }
  virtual void exitDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext * /*ctx*/) override { }

  virtual void enterStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext * /*ctx*/) override { }
  virtual void exitStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext * /*ctx*/) override { }

  virtual void enterFunctionSpecifier(CPP14Parser::FunctionSpecifierContext * /*ctx*/) override { }
  virtual void exitFunctionSpecifier(CPP14Parser::FunctionSpecifierContext * /*ctx*/) override { }

  virtual void enterTypedefName(CPP14Parser::TypedefNameContext * /*ctx*/) override { }
  virtual void exitTypedefName(CPP14Parser::TypedefNameContext * /*ctx*/) override { }

  virtual void enterTypeSpecifier(CPP14Parser::TypeSpecifierContext * /*ctx*/) override { }
  virtual void exitTypeSpecifier(CPP14Parser::TypeSpecifierContext * /*ctx*/) override { }

  virtual void enterTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext * /*ctx*/) override { }
  virtual void exitTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext * /*ctx*/) override { }

  virtual void enterTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext * /*ctx*/) override { }
  virtual void exitTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext * /*ctx*/) override { }

  virtual void enterTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext * /*ctx*/) override { }
  virtual void exitTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext * /*ctx*/) override { }

  virtual void enterSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext * /*ctx*/) override { }
  virtual void exitSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext * /*ctx*/) override { }

  virtual void enterSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext * /*ctx*/) override { }
  virtual void exitSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext * /*ctx*/) override { }

  virtual void enterSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext * /*ctx*/) override { }
  virtual void exitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext * /*ctx*/) override { }

  virtual void enterTheTypeName(CPP14Parser::TheTypeNameContext * /*ctx*/) override { }
  virtual void exitTheTypeName(CPP14Parser::TheTypeNameContext * /*ctx*/) override { }

  virtual void enterDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext * /*ctx*/) override { }
  virtual void exitDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext * /*ctx*/) override { }

  virtual void enterElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext * /*ctx*/) override { }
  virtual void exitElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext * /*ctx*/) override { }

  virtual void enterEnumName(CPP14Parser::EnumNameContext * /*ctx*/) override { }
  virtual void exitEnumName(CPP14Parser::EnumNameContext * /*ctx*/) override { }

  virtual void enterEnumSpecifier(CPP14Parser::EnumSpecifierContext * /*ctx*/) override { }
  virtual void exitEnumSpecifier(CPP14Parser::EnumSpecifierContext * /*ctx*/) override { }

  virtual void enterEnumHead(CPP14Parser::EnumHeadContext * /*ctx*/) override { }
  virtual void exitEnumHead(CPP14Parser::EnumHeadContext * /*ctx*/) override { }

  virtual void enterOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext * /*ctx*/) override { }
  virtual void exitOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext * /*ctx*/) override { }

  virtual void enterEnumkey(CPP14Parser::EnumkeyContext * /*ctx*/) override { }
  virtual void exitEnumkey(CPP14Parser::EnumkeyContext * /*ctx*/) override { }

  virtual void enterEnumbase(CPP14Parser::EnumbaseContext * /*ctx*/) override { }
  virtual void exitEnumbase(CPP14Parser::EnumbaseContext * /*ctx*/) override { }

  virtual void enterEnumeratorList(CPP14Parser::EnumeratorListContext * /*ctx*/) override { }
  virtual void exitEnumeratorList(CPP14Parser::EnumeratorListContext * /*ctx*/) override { }

  virtual void enterEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext * /*ctx*/) override { }
  virtual void exitEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext * /*ctx*/) override { }

  virtual void enterEnumerator(CPP14Parser::EnumeratorContext * /*ctx*/) override { }
  virtual void exitEnumerator(CPP14Parser::EnumeratorContext * /*ctx*/) override { }

  virtual void enterNamespaceName(CPP14Parser::NamespaceNameContext * /*ctx*/) override { }
  virtual void exitNamespaceName(CPP14Parser::NamespaceNameContext * /*ctx*/) override { }

  virtual void enterOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext * /*ctx*/) override { }
  virtual void exitOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext * /*ctx*/) override { }

  virtual void enterNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext * /*ctx*/) override { }
  virtual void exitNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext * /*ctx*/) override { }

  virtual void enterNamespaceAlias(CPP14Parser::NamespaceAliasContext * /*ctx*/) override { }
  virtual void exitNamespaceAlias(CPP14Parser::NamespaceAliasContext * /*ctx*/) override { }

  virtual void enterNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext * /*ctx*/) override { }
  virtual void exitNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext * /*ctx*/) override { }

  virtual void enterQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext * /*ctx*/) override { }
  virtual void exitQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext * /*ctx*/) override { }

  virtual void enterUsingDeclaration(CPP14Parser::UsingDeclarationContext * /*ctx*/) override { }
  virtual void exitUsingDeclaration(CPP14Parser::UsingDeclarationContext * /*ctx*/) override { }

  virtual void enterUsingDirective(CPP14Parser::UsingDirectiveContext * /*ctx*/) override { }
  virtual void exitUsingDirective(CPP14Parser::UsingDirectiveContext * /*ctx*/) override { }

  virtual void enterAsmDefinition(CPP14Parser::AsmDefinitionContext * /*ctx*/) override { }
  virtual void exitAsmDefinition(CPP14Parser::AsmDefinitionContext * /*ctx*/) override { }

  virtual void enterLinkageSpecification(CPP14Parser::LinkageSpecificationContext * /*ctx*/) override { }
  virtual void exitLinkageSpecification(CPP14Parser::LinkageSpecificationContext * /*ctx*/) override { }

  virtual void enterAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext * /*ctx*/) override { }
  virtual void exitAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext * /*ctx*/) override { }

  virtual void enterAttributeSpecifier(CPP14Parser::AttributeSpecifierContext * /*ctx*/) override { }
  virtual void exitAttributeSpecifier(CPP14Parser::AttributeSpecifierContext * /*ctx*/) override { }

  virtual void enterAlignmentspecifier(CPP14Parser::AlignmentspecifierContext * /*ctx*/) override { }
  virtual void exitAlignmentspecifier(CPP14Parser::AlignmentspecifierContext * /*ctx*/) override { }

  virtual void enterAttributeList(CPP14Parser::AttributeListContext * /*ctx*/) override { }
  virtual void exitAttributeList(CPP14Parser::AttributeListContext * /*ctx*/) override { }

  virtual void enterAttribute(CPP14Parser::AttributeContext * /*ctx*/) override { }
  virtual void exitAttribute(CPP14Parser::AttributeContext * /*ctx*/) override { }

  virtual void enterAttributeNamespace(CPP14Parser::AttributeNamespaceContext * /*ctx*/) override { }
  virtual void exitAttributeNamespace(CPP14Parser::AttributeNamespaceContext * /*ctx*/) override { }

  virtual void enterAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext * /*ctx*/) override { }
  virtual void exitAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext * /*ctx*/) override { }

  virtual void enterBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext * /*ctx*/) override { }
  virtual void exitBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext * /*ctx*/) override { }

  virtual void enterBalancedtoken(CPP14Parser::BalancedtokenContext * /*ctx*/) override { }
  virtual void exitBalancedtoken(CPP14Parser::BalancedtokenContext * /*ctx*/) override { }

  virtual void enterInitDeclaratorList(CPP14Parser::InitDeclaratorListContext * /*ctx*/) override { }
  virtual void exitInitDeclaratorList(CPP14Parser::InitDeclaratorListContext * /*ctx*/) override { }

  virtual void enterInitDeclarator(CPP14Parser::InitDeclaratorContext * /*ctx*/) override { }
  virtual void exitInitDeclarator(CPP14Parser::InitDeclaratorContext * /*ctx*/) override { }

  virtual void enterDeclarator(CPP14Parser::DeclaratorContext * /*ctx*/) override { }
  virtual void exitDeclarator(CPP14Parser::DeclaratorContext * /*ctx*/) override { }

  virtual void enterPointerDeclarator(CPP14Parser::PointerDeclaratorContext * /*ctx*/) override { }
  virtual void exitPointerDeclarator(CPP14Parser::PointerDeclaratorContext * /*ctx*/) override { }

  virtual void enterNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext * /*ctx*/) override { }
  virtual void exitNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext * /*ctx*/) override { }

  virtual void enterParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext * /*ctx*/) override { }
  virtual void exitParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext * /*ctx*/) override { }

  virtual void enterTrailingReturnType(CPP14Parser::TrailingReturnTypeContext * /*ctx*/) override { }
  virtual void exitTrailingReturnType(CPP14Parser::TrailingReturnTypeContext * /*ctx*/) override { }

  virtual void enterPointerOperator(CPP14Parser::PointerOperatorContext * /*ctx*/) override { }
  virtual void exitPointerOperator(CPP14Parser::PointerOperatorContext * /*ctx*/) override { }

  virtual void enterCvqualifierseq(CPP14Parser::CvqualifierseqContext * /*ctx*/) override { }
  virtual void exitCvqualifierseq(CPP14Parser::CvqualifierseqContext * /*ctx*/) override { }

  virtual void enterCvQualifier(CPP14Parser::CvQualifierContext * /*ctx*/) override { }
  virtual void exitCvQualifier(CPP14Parser::CvQualifierContext * /*ctx*/) override { }

  virtual void enterRefqualifier(CPP14Parser::RefqualifierContext * /*ctx*/) override { }
  virtual void exitRefqualifier(CPP14Parser::RefqualifierContext * /*ctx*/) override { }

  virtual void enterDeclaratorid(CPP14Parser::DeclaratoridContext * /*ctx*/) override { }
  virtual void exitDeclaratorid(CPP14Parser::DeclaratoridContext * /*ctx*/) override { }

  virtual void enterTheTypeId(CPP14Parser::TheTypeIdContext * /*ctx*/) override { }
  virtual void exitTheTypeId(CPP14Parser::TheTypeIdContext * /*ctx*/) override { }

  virtual void enterAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext * /*ctx*/) override { }
  virtual void exitAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext * /*ctx*/) override { }

  virtual void enterPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext * /*ctx*/) override { }
  virtual void exitPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext * /*ctx*/) override { }

  virtual void enterNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext * /*ctx*/) override { }
  virtual void exitNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext * /*ctx*/) override { }

  virtual void enterAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext * /*ctx*/) override { }
  virtual void exitAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext * /*ctx*/) override { }

  virtual void enterNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext * /*ctx*/) override { }
  virtual void exitNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext * /*ctx*/) override { }

  virtual void enterParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext * /*ctx*/) override { }
  virtual void exitParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext * /*ctx*/) override { }

  virtual void enterParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext * /*ctx*/) override { }
  virtual void exitParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext * /*ctx*/) override { }

  virtual void enterParameterDeclaration(CPP14Parser::ParameterDeclarationContext * /*ctx*/) override { }
  virtual void exitParameterDeclaration(CPP14Parser::ParameterDeclarationContext * /*ctx*/) override { }

  virtual void enterFunctionDefinition(CPP14Parser::FunctionDefinitionContext * /*ctx*/) override { }
  virtual void exitFunctionDefinition(CPP14Parser::FunctionDefinitionContext * /*ctx*/) override { }

  virtual void enterFunctionBody(CPP14Parser::FunctionBodyContext * /*ctx*/) override { }
  virtual void exitFunctionBody(CPP14Parser::FunctionBodyContext * /*ctx*/) override { }

  virtual void enterInitializer(CPP14Parser::InitializerContext * /*ctx*/) override { }
  virtual void exitInitializer(CPP14Parser::InitializerContext * /*ctx*/) override { }

  virtual void enterBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext * /*ctx*/) override { }
  virtual void exitBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext * /*ctx*/) override { }

  virtual void enterInitializerClause(CPP14Parser::InitializerClauseContext * /*ctx*/) override { }
  virtual void exitInitializerClause(CPP14Parser::InitializerClauseContext * /*ctx*/) override { }

  virtual void enterInitializerList(CPP14Parser::InitializerListContext * /*ctx*/) override { }
  virtual void exitInitializerList(CPP14Parser::InitializerListContext * /*ctx*/) override { }

  virtual void enterBracedInitList(CPP14Parser::BracedInitListContext * /*ctx*/) override { }
  virtual void exitBracedInitList(CPP14Parser::BracedInitListContext * /*ctx*/) override { }

  virtual void enterClassName(CPP14Parser::ClassNameContext * /*ctx*/) override { }
  virtual void exitClassName(CPP14Parser::ClassNameContext * /*ctx*/) override { }

  virtual void enterClassSpecifier(CPP14Parser::ClassSpecifierContext * /*ctx*/) override { }
  virtual void exitClassSpecifier(CPP14Parser::ClassSpecifierContext * /*ctx*/) override { }

  virtual void enterClassHead(CPP14Parser::ClassHeadContext * /*ctx*/) override { }
  virtual void exitClassHead(CPP14Parser::ClassHeadContext * /*ctx*/) override { }

  virtual void enterClassHeadName(CPP14Parser::ClassHeadNameContext * /*ctx*/) override { }
  virtual void exitClassHeadName(CPP14Parser::ClassHeadNameContext * /*ctx*/) override { }

  virtual void enterClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext * /*ctx*/) override { }
  virtual void exitClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext * /*ctx*/) override { }

  virtual void enterClassKey(CPP14Parser::ClassKeyContext * /*ctx*/) override { }
  virtual void exitClassKey(CPP14Parser::ClassKeyContext * /*ctx*/) override { }

  virtual void enterMemberSpecification(CPP14Parser::MemberSpecificationContext * /*ctx*/) override { }
  virtual void exitMemberSpecification(CPP14Parser::MemberSpecificationContext * /*ctx*/) override { }

  virtual void enterMemberdeclaration(CPP14Parser::MemberdeclarationContext * /*ctx*/) override { }
  virtual void exitMemberdeclaration(CPP14Parser::MemberdeclarationContext * /*ctx*/) override { }

  virtual void enterMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext * /*ctx*/) override { }
  virtual void exitMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext * /*ctx*/) override { }

  virtual void enterMemberDeclarator(CPP14Parser::MemberDeclaratorContext * /*ctx*/) override { }
  virtual void exitMemberDeclarator(CPP14Parser::MemberDeclaratorContext * /*ctx*/) override { }

  virtual void enterVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext * /*ctx*/) override { }
  virtual void exitVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext * /*ctx*/) override { }

  virtual void enterVirtualSpecifier(CPP14Parser::VirtualSpecifierContext * /*ctx*/) override { }
  virtual void exitVirtualSpecifier(CPP14Parser::VirtualSpecifierContext * /*ctx*/) override { }

  virtual void enterPureSpecifier(CPP14Parser::PureSpecifierContext * /*ctx*/) override { }
  virtual void exitPureSpecifier(CPP14Parser::PureSpecifierContext * /*ctx*/) override { }

  virtual void enterBaseClause(CPP14Parser::BaseClauseContext * /*ctx*/) override { }
  virtual void exitBaseClause(CPP14Parser::BaseClauseContext * /*ctx*/) override { }

  virtual void enterBaseSpecifierList(CPP14Parser::BaseSpecifierListContext * /*ctx*/) override { }
  virtual void exitBaseSpecifierList(CPP14Parser::BaseSpecifierListContext * /*ctx*/) override { }

  virtual void enterBaseSpecifier(CPP14Parser::BaseSpecifierContext * /*ctx*/) override { }
  virtual void exitBaseSpecifier(CPP14Parser::BaseSpecifierContext * /*ctx*/) override { }

  virtual void enterClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext * /*ctx*/) override { }
  virtual void exitClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext * /*ctx*/) override { }

  virtual void enterBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext * /*ctx*/) override { }
  virtual void exitBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext * /*ctx*/) override { }

  virtual void enterAccessSpecifier(CPP14Parser::AccessSpecifierContext * /*ctx*/) override { }
  virtual void exitAccessSpecifier(CPP14Parser::AccessSpecifierContext * /*ctx*/) override { }

  virtual void enterConversionFunctionId(CPP14Parser::ConversionFunctionIdContext * /*ctx*/) override { }
  virtual void exitConversionFunctionId(CPP14Parser::ConversionFunctionIdContext * /*ctx*/) override { }

  virtual void enterConversionTypeId(CPP14Parser::ConversionTypeIdContext * /*ctx*/) override { }
  virtual void exitConversionTypeId(CPP14Parser::ConversionTypeIdContext * /*ctx*/) override { }

  virtual void enterConversionDeclarator(CPP14Parser::ConversionDeclaratorContext * /*ctx*/) override { }
  virtual void exitConversionDeclarator(CPP14Parser::ConversionDeclaratorContext * /*ctx*/) override { }

  virtual void enterConstructorInitializer(CPP14Parser::ConstructorInitializerContext * /*ctx*/) override { }
  virtual void exitConstructorInitializer(CPP14Parser::ConstructorInitializerContext * /*ctx*/) override { }

  virtual void enterMemInitializerList(CPP14Parser::MemInitializerListContext * /*ctx*/) override { }
  virtual void exitMemInitializerList(CPP14Parser::MemInitializerListContext * /*ctx*/) override { }

  virtual void enterMemInitializer(CPP14Parser::MemInitializerContext * /*ctx*/) override { }
  virtual void exitMemInitializer(CPP14Parser::MemInitializerContext * /*ctx*/) override { }

  virtual void enterMeminitializerid(CPP14Parser::MeminitializeridContext * /*ctx*/) override { }
  virtual void exitMeminitializerid(CPP14Parser::MeminitializeridContext * /*ctx*/) override { }

  virtual void enterOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext * /*ctx*/) override { }
  virtual void exitOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext * /*ctx*/) override { }

  virtual void enterLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext * /*ctx*/) override { }
  virtual void exitLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext * /*ctx*/) override { }

  virtual void enterTemplateDeclaration(CPP14Parser::TemplateDeclarationContext * /*ctx*/) override { }
  virtual void exitTemplateDeclaration(CPP14Parser::TemplateDeclarationContext * /*ctx*/) override { }

  virtual void enterTemplateparameterList(CPP14Parser::TemplateparameterListContext * /*ctx*/) override { }
  virtual void exitTemplateparameterList(CPP14Parser::TemplateparameterListContext * /*ctx*/) override { }

  virtual void enterTemplateParameter(CPP14Parser::TemplateParameterContext * /*ctx*/) override { }
  virtual void exitTemplateParameter(CPP14Parser::TemplateParameterContext * /*ctx*/) override { }

  virtual void enterTypeParameter(CPP14Parser::TypeParameterContext * /*ctx*/) override { }
  virtual void exitTypeParameter(CPP14Parser::TypeParameterContext * /*ctx*/) override { }

  virtual void enterSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext * /*ctx*/) override { }
  virtual void exitSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext * /*ctx*/) override { }

  virtual void enterTemplateId(CPP14Parser::TemplateIdContext * /*ctx*/) override { }
  virtual void exitTemplateId(CPP14Parser::TemplateIdContext * /*ctx*/) override { }

  virtual void enterTemplateName(CPP14Parser::TemplateNameContext * /*ctx*/) override { }
  virtual void exitTemplateName(CPP14Parser::TemplateNameContext * /*ctx*/) override { }

  virtual void enterTemplateArgumentList(CPP14Parser::TemplateArgumentListContext * /*ctx*/) override { }
  virtual void exitTemplateArgumentList(CPP14Parser::TemplateArgumentListContext * /*ctx*/) override { }

  virtual void enterTemplateArgument(CPP14Parser::TemplateArgumentContext * /*ctx*/) override { }
  virtual void exitTemplateArgument(CPP14Parser::TemplateArgumentContext * /*ctx*/) override { }

  virtual void enterTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext * /*ctx*/) override { }
  virtual void exitTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext * /*ctx*/) override { }

  virtual void enterExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext * /*ctx*/) override { }
  virtual void exitExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext * /*ctx*/) override { }

  virtual void enterExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext * /*ctx*/) override { }
  virtual void exitExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext * /*ctx*/) override { }

  virtual void enterTryBlock(CPP14Parser::TryBlockContext * /*ctx*/) override { }
  virtual void exitTryBlock(CPP14Parser::TryBlockContext * /*ctx*/) override { }

  virtual void enterFunctionTryBlock(CPP14Parser::FunctionTryBlockContext * /*ctx*/) override { }
  virtual void exitFunctionTryBlock(CPP14Parser::FunctionTryBlockContext * /*ctx*/) override { }

  virtual void enterHandlerSeq(CPP14Parser::HandlerSeqContext * /*ctx*/) override { }
  virtual void exitHandlerSeq(CPP14Parser::HandlerSeqContext * /*ctx*/) override { }

  virtual void enterHandler(CPP14Parser::HandlerContext * /*ctx*/) override { }
  virtual void exitHandler(CPP14Parser::HandlerContext * /*ctx*/) override { }

  virtual void enterExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext * /*ctx*/) override { }
  virtual void exitExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext * /*ctx*/) override { }

  virtual void enterThrowExpression(CPP14Parser::ThrowExpressionContext * /*ctx*/) override { }
  virtual void exitThrowExpression(CPP14Parser::ThrowExpressionContext * /*ctx*/) override { }

  virtual void enterExceptionSpecification(CPP14Parser::ExceptionSpecificationContext * /*ctx*/) override { }
  virtual void exitExceptionSpecification(CPP14Parser::ExceptionSpecificationContext * /*ctx*/) override { }

  virtual void enterDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext * /*ctx*/) override { }
  virtual void exitDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext * /*ctx*/) override { }

  virtual void enterTypeIdList(CPP14Parser::TypeIdListContext * /*ctx*/) override { }
  virtual void exitTypeIdList(CPP14Parser::TypeIdListContext * /*ctx*/) override { }

  virtual void enterNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext * /*ctx*/) override { }
  virtual void exitNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext * /*ctx*/) override { }

  virtual void enterTheOperator(CPP14Parser::TheOperatorContext * /*ctx*/) override { }
  virtual void exitTheOperator(CPP14Parser::TheOperatorContext * /*ctx*/) override { }

  virtual void enterLiteral(CPP14Parser::LiteralContext * /*ctx*/) override { }
  virtual void exitLiteral(CPP14Parser::LiteralContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

