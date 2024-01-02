#include "CPP14ParserBase.h"

// Generated from CPP14Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "CPP14ParserVisitor.h"


/**
 * This class provides an empty implementation of CPP14ParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  CPP14ParserBaseVisitor : public CPP14ParserVisitor {
public:

  virtual std::any visitTranslationUnit(CPP14Parser::TranslationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpression(CPP14Parser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdExpression(CPP14Parser::IdExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnqualifiedId(CPP14Parser::UnqualifiedIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifiedId(CPP14Parser::QualifiedIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLambdaExpression(CPP14Parser::LambdaExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLambdaIntroducer(CPP14Parser::LambdaIntroducerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLambdaCapture(CPP14Parser::LambdaCaptureContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCaptureDefault(CPP14Parser::CaptureDefaultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCaptureList(CPP14Parser::CaptureListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCapture(CPP14Parser::CaptureContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleCapture(CPP14Parser::SimpleCaptureContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitcapture(CPP14Parser::InitcaptureContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPostfixExpression(CPP14Parser::PostfixExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionList(CPP14Parser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(CPP14Parser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOperator(CPP14Parser::UnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewExpression_(CPP14Parser::NewExpression_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewPlacement(CPP14Parser::NewPlacementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewTypeId(CPP14Parser::NewTypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewDeclarator_(CPP14Parser::NewDeclarator_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewInitializer_(CPP14Parser::NewInitializer_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeleteExpression(CPP14Parser::DeleteExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoExceptExpression(CPP14Parser::NoExceptExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpression(CPP14Parser::CastExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(CPP14Parser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(CPP14Parser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftOperator(CPP14Parser::ShiftOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelationalExpression(CPP14Parser::RelationalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpression(CPP14Parser::EqualityExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(CPP14Parser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalExpression(CPP14Parser::ConditionalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpression(CPP14Parser::AssignmentExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentOperator(CPP14Parser::AssignmentOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(CPP14Parser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstantExpression(CPP14Parser::ConstantExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(CPP14Parser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLabeledStatement(CPP14Parser::LabeledStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatementSeq(CPP14Parser::StatementSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondition(CPP14Parser::ConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIterationStatement(CPP14Parser::IterationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForInitStatement(CPP14Parser::ForInitStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForRangeInitializer(CPP14Parser::ForRangeInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJumpStatement(CPP14Parser::JumpStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclarationStatement(CPP14Parser::DeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclarationseq(CPP14Parser::DeclarationseqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclaration(CPP14Parser::DeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockDeclaration(CPP14Parser::BlockDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAliasDeclaration(CPP14Parser::AliasDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeDeclaration(CPP14Parser::AttributeDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclSpecifier(CPP14Parser::DeclSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionSpecifier(CPP14Parser::FunctionSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypedefName(CPP14Parser::TypedefNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpecifier(CPP14Parser::TypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTheTypeName(CPP14Parser::TheTypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumName(CPP14Parser::EnumNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumSpecifier(CPP14Parser::EnumSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumHead(CPP14Parser::EnumHeadContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumkey(CPP14Parser::EnumkeyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumbase(CPP14Parser::EnumbaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumeratorList(CPP14Parser::EnumeratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumerator(CPP14Parser::EnumeratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceName(CPP14Parser::NamespaceNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceAlias(CPP14Parser::NamespaceAliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUsingDeclaration(CPP14Parser::UsingDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUsingDirective(CPP14Parser::UsingDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsmDefinition(CPP14Parser::AsmDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLinkageSpecification(CPP14Parser::LinkageSpecificationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeSpecifier(CPP14Parser::AttributeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlignmentspecifier(CPP14Parser::AlignmentspecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeList(CPP14Parser::AttributeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttribute(CPP14Parser::AttributeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeNamespace(CPP14Parser::AttributeNamespaceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBalancedtoken(CPP14Parser::BalancedtokenContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitDeclaratorList(CPP14Parser::InitDeclaratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitDeclarator(CPP14Parser::InitDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclarator(CPP14Parser::DeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointerDeclarator(CPP14Parser::PointerDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTrailingReturnType(CPP14Parser::TrailingReturnTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointerOperator(CPP14Parser::PointerOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCvqualifierseq(CPP14Parser::CvqualifierseqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCvQualifier(CPP14Parser::CvQualifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRefqualifier(CPP14Parser::RefqualifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclaratorid(CPP14Parser::DeclaratoridContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTheTypeId(CPP14Parser::TheTypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterDeclaration(CPP14Parser::ParameterDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionBody(CPP14Parser::FunctionBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializer(CPP14Parser::InitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializerClause(CPP14Parser::InitializerClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializerList(CPP14Parser::InitializerListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBracedInitList(CPP14Parser::BracedInitListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassName(CPP14Parser::ClassNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassSpecifier(CPP14Parser::ClassSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassHead(CPP14Parser::ClassHeadContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassHeadName(CPP14Parser::ClassHeadNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassKey(CPP14Parser::ClassKeyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberSpecification(CPP14Parser::MemberSpecificationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberdeclaration(CPP14Parser::MemberdeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberDeclarator(CPP14Parser::MemberDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVirtualSpecifier(CPP14Parser::VirtualSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPureSpecifier(CPP14Parser::PureSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseClause(CPP14Parser::BaseClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseSpecifierList(CPP14Parser::BaseSpecifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseSpecifier(CPP14Parser::BaseSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAccessSpecifier(CPP14Parser::AccessSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConversionFunctionId(CPP14Parser::ConversionFunctionIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConversionTypeId(CPP14Parser::ConversionTypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConversionDeclarator(CPP14Parser::ConversionDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstructorInitializer(CPP14Parser::ConstructorInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemInitializerList(CPP14Parser::MemInitializerListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemInitializer(CPP14Parser::MemInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMeminitializerid(CPP14Parser::MeminitializeridContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateDeclaration(CPP14Parser::TemplateDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateparameterList(CPP14Parser::TemplateparameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateParameter(CPP14Parser::TemplateParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeParameter(CPP14Parser::TypeParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateId(CPP14Parser::TemplateIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateName(CPP14Parser::TemplateNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateArgumentList(CPP14Parser::TemplateArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplateArgument(CPP14Parser::TemplateArgumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTryBlock(CPP14Parser::TryBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionTryBlock(CPP14Parser::FunctionTryBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitHandlerSeq(CPP14Parser::HandlerSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitHandler(CPP14Parser::HandlerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitThrowExpression(CPP14Parser::ThrowExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExceptionSpecification(CPP14Parser::ExceptionSpecificationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeIdList(CPP14Parser::TypeIdListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTheOperator(CPP14Parser::TheOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(CPP14Parser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

