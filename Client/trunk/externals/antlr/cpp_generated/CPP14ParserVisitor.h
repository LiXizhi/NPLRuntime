#include "CPP14ParserBase.h"

// Generated from CPP14Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "CPP14Parser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by CPP14Parser.
 */
class  CPP14ParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by CPP14Parser.
   */
    virtual std::any visitTranslationUnit(CPP14Parser::TranslationUnitContext *context) = 0;

    virtual std::any visitPrimaryExpression(CPP14Parser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitIdExpression(CPP14Parser::IdExpressionContext *context) = 0;

    virtual std::any visitUnqualifiedId(CPP14Parser::UnqualifiedIdContext *context) = 0;

    virtual std::any visitQualifiedId(CPP14Parser::QualifiedIdContext *context) = 0;

    virtual std::any visitNestedNameSpecifier(CPP14Parser::NestedNameSpecifierContext *context) = 0;

    virtual std::any visitLambdaExpression(CPP14Parser::LambdaExpressionContext *context) = 0;

    virtual std::any visitLambdaIntroducer(CPP14Parser::LambdaIntroducerContext *context) = 0;

    virtual std::any visitLambdaCapture(CPP14Parser::LambdaCaptureContext *context) = 0;

    virtual std::any visitCaptureDefault(CPP14Parser::CaptureDefaultContext *context) = 0;

    virtual std::any visitCaptureList(CPP14Parser::CaptureListContext *context) = 0;

    virtual std::any visitCapture(CPP14Parser::CaptureContext *context) = 0;

    virtual std::any visitSimpleCapture(CPP14Parser::SimpleCaptureContext *context) = 0;

    virtual std::any visitInitcapture(CPP14Parser::InitcaptureContext *context) = 0;

    virtual std::any visitLambdaDeclarator(CPP14Parser::LambdaDeclaratorContext *context) = 0;

    virtual std::any visitPostfixExpression(CPP14Parser::PostfixExpressionContext *context) = 0;

    virtual std::any visitTypeIdOfTheTypeId(CPP14Parser::TypeIdOfTheTypeIdContext *context) = 0;

    virtual std::any visitExpressionList(CPP14Parser::ExpressionListContext *context) = 0;

    virtual std::any visitPseudoDestructorName(CPP14Parser::PseudoDestructorNameContext *context) = 0;

    virtual std::any visitUnaryExpression(CPP14Parser::UnaryExpressionContext *context) = 0;

    virtual std::any visitUnaryOperator(CPP14Parser::UnaryOperatorContext *context) = 0;

    virtual std::any visitNewExpression_(CPP14Parser::NewExpression_Context *context) = 0;

    virtual std::any visitNewPlacement(CPP14Parser::NewPlacementContext *context) = 0;

    virtual std::any visitNewTypeId(CPP14Parser::NewTypeIdContext *context) = 0;

    virtual std::any visitNewDeclarator_(CPP14Parser::NewDeclarator_Context *context) = 0;

    virtual std::any visitNoPointerNewDeclarator(CPP14Parser::NoPointerNewDeclaratorContext *context) = 0;

    virtual std::any visitNewInitializer_(CPP14Parser::NewInitializer_Context *context) = 0;

    virtual std::any visitDeleteExpression(CPP14Parser::DeleteExpressionContext *context) = 0;

    virtual std::any visitNoExceptExpression(CPP14Parser::NoExceptExpressionContext *context) = 0;

    virtual std::any visitCastExpression(CPP14Parser::CastExpressionContext *context) = 0;

    virtual std::any visitPointerMemberExpression(CPP14Parser::PointerMemberExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(CPP14Parser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(CPP14Parser::ShiftExpressionContext *context) = 0;

    virtual std::any visitShiftOperator(CPP14Parser::ShiftOperatorContext *context) = 0;

    virtual std::any visitRelationalExpression(CPP14Parser::RelationalExpressionContext *context) = 0;

    virtual std::any visitEqualityExpression(CPP14Parser::EqualityExpressionContext *context) = 0;

    virtual std::any visitAndExpression(CPP14Parser::AndExpressionContext *context) = 0;

    virtual std::any visitExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext *context) = 0;

    virtual std::any visitInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext *context) = 0;

    virtual std::any visitLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext *context) = 0;

    virtual std::any visitLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext *context) = 0;

    virtual std::any visitConditionalExpression(CPP14Parser::ConditionalExpressionContext *context) = 0;

    virtual std::any visitAssignmentExpression(CPP14Parser::AssignmentExpressionContext *context) = 0;

    virtual std::any visitAssignmentOperator(CPP14Parser::AssignmentOperatorContext *context) = 0;

    virtual std::any visitExpression(CPP14Parser::ExpressionContext *context) = 0;

    virtual std::any visitConstantExpression(CPP14Parser::ConstantExpressionContext *context) = 0;

    virtual std::any visitStatement(CPP14Parser::StatementContext *context) = 0;

    virtual std::any visitLabeledStatement(CPP14Parser::LabeledStatementContext *context) = 0;

    virtual std::any visitExpressionStatement(CPP14Parser::ExpressionStatementContext *context) = 0;

    virtual std::any visitCompoundStatement(CPP14Parser::CompoundStatementContext *context) = 0;

    virtual std::any visitStatementSeq(CPP14Parser::StatementSeqContext *context) = 0;

    virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *context) = 0;

    virtual std::any visitCondition(CPP14Parser::ConditionContext *context) = 0;

    virtual std::any visitIterationStatement(CPP14Parser::IterationStatementContext *context) = 0;

    virtual std::any visitForInitStatement(CPP14Parser::ForInitStatementContext *context) = 0;

    virtual std::any visitForRangeDeclaration(CPP14Parser::ForRangeDeclarationContext *context) = 0;

    virtual std::any visitForRangeInitializer(CPP14Parser::ForRangeInitializerContext *context) = 0;

    virtual std::any visitJumpStatement(CPP14Parser::JumpStatementContext *context) = 0;

    virtual std::any visitDeclarationStatement(CPP14Parser::DeclarationStatementContext *context) = 0;

    virtual std::any visitDeclarationseq(CPP14Parser::DeclarationseqContext *context) = 0;

    virtual std::any visitDeclaration(CPP14Parser::DeclarationContext *context) = 0;

    virtual std::any visitBlockDeclaration(CPP14Parser::BlockDeclarationContext *context) = 0;

    virtual std::any visitAliasDeclaration(CPP14Parser::AliasDeclarationContext *context) = 0;

    virtual std::any visitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *context) = 0;

    virtual std::any visitStaticAssertDeclaration(CPP14Parser::StaticAssertDeclarationContext *context) = 0;

    virtual std::any visitEmptyDeclaration_(CPP14Parser::EmptyDeclaration_Context *context) = 0;

    virtual std::any visitAttributeDeclaration(CPP14Parser::AttributeDeclarationContext *context) = 0;

    virtual std::any visitDeclSpecifier(CPP14Parser::DeclSpecifierContext *context) = 0;

    virtual std::any visitDeclSpecifierSeq(CPP14Parser::DeclSpecifierSeqContext *context) = 0;

    virtual std::any visitStorageClassSpecifier(CPP14Parser::StorageClassSpecifierContext *context) = 0;

    virtual std::any visitFunctionSpecifier(CPP14Parser::FunctionSpecifierContext *context) = 0;

    virtual std::any visitTypedefName(CPP14Parser::TypedefNameContext *context) = 0;

    virtual std::any visitTypeSpecifier(CPP14Parser::TypeSpecifierContext *context) = 0;

    virtual std::any visitTrailingTypeSpecifier(CPP14Parser::TrailingTypeSpecifierContext *context) = 0;

    virtual std::any visitTypeSpecifierSeq(CPP14Parser::TypeSpecifierSeqContext *context) = 0;

    virtual std::any visitTrailingTypeSpecifierSeq(CPP14Parser::TrailingTypeSpecifierSeqContext *context) = 0;

    virtual std::any visitSimpleTypeLengthModifier(CPP14Parser::SimpleTypeLengthModifierContext *context) = 0;

    virtual std::any visitSimpleTypeSignednessModifier(CPP14Parser::SimpleTypeSignednessModifierContext *context) = 0;

    virtual std::any visitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *context) = 0;

    virtual std::any visitTheTypeName(CPP14Parser::TheTypeNameContext *context) = 0;

    virtual std::any visitDecltypeSpecifier(CPP14Parser::DecltypeSpecifierContext *context) = 0;

    virtual std::any visitElaboratedTypeSpecifier(CPP14Parser::ElaboratedTypeSpecifierContext *context) = 0;

    virtual std::any visitEnumName(CPP14Parser::EnumNameContext *context) = 0;

    virtual std::any visitEnumSpecifier(CPP14Parser::EnumSpecifierContext *context) = 0;

    virtual std::any visitEnumHead(CPP14Parser::EnumHeadContext *context) = 0;

    virtual std::any visitOpaqueEnumDeclaration(CPP14Parser::OpaqueEnumDeclarationContext *context) = 0;

    virtual std::any visitEnumkey(CPP14Parser::EnumkeyContext *context) = 0;

    virtual std::any visitEnumbase(CPP14Parser::EnumbaseContext *context) = 0;

    virtual std::any visitEnumeratorList(CPP14Parser::EnumeratorListContext *context) = 0;

    virtual std::any visitEnumeratorDefinition(CPP14Parser::EnumeratorDefinitionContext *context) = 0;

    virtual std::any visitEnumerator(CPP14Parser::EnumeratorContext *context) = 0;

    virtual std::any visitNamespaceName(CPP14Parser::NamespaceNameContext *context) = 0;

    virtual std::any visitOriginalNamespaceName(CPP14Parser::OriginalNamespaceNameContext *context) = 0;

    virtual std::any visitNamespaceDefinition(CPP14Parser::NamespaceDefinitionContext *context) = 0;

    virtual std::any visitNamespaceAlias(CPP14Parser::NamespaceAliasContext *context) = 0;

    virtual std::any visitNamespaceAliasDefinition(CPP14Parser::NamespaceAliasDefinitionContext *context) = 0;

    virtual std::any visitQualifiednamespacespecifier(CPP14Parser::QualifiednamespacespecifierContext *context) = 0;

    virtual std::any visitUsingDeclaration(CPP14Parser::UsingDeclarationContext *context) = 0;

    virtual std::any visitUsingDirective(CPP14Parser::UsingDirectiveContext *context) = 0;

    virtual std::any visitAsmDefinition(CPP14Parser::AsmDefinitionContext *context) = 0;

    virtual std::any visitLinkageSpecification(CPP14Parser::LinkageSpecificationContext *context) = 0;

    virtual std::any visitAttributeSpecifierSeq(CPP14Parser::AttributeSpecifierSeqContext *context) = 0;

    virtual std::any visitAttributeSpecifier(CPP14Parser::AttributeSpecifierContext *context) = 0;

    virtual std::any visitAlignmentspecifier(CPP14Parser::AlignmentspecifierContext *context) = 0;

    virtual std::any visitAttributeList(CPP14Parser::AttributeListContext *context) = 0;

    virtual std::any visitAttribute(CPP14Parser::AttributeContext *context) = 0;

    virtual std::any visitAttributeNamespace(CPP14Parser::AttributeNamespaceContext *context) = 0;

    virtual std::any visitAttributeArgumentClause(CPP14Parser::AttributeArgumentClauseContext *context) = 0;

    virtual std::any visitBalancedTokenSeq(CPP14Parser::BalancedTokenSeqContext *context) = 0;

    virtual std::any visitBalancedtoken(CPP14Parser::BalancedtokenContext *context) = 0;

    virtual std::any visitInitDeclaratorList(CPP14Parser::InitDeclaratorListContext *context) = 0;

    virtual std::any visitInitDeclarator(CPP14Parser::InitDeclaratorContext *context) = 0;

    virtual std::any visitDeclarator(CPP14Parser::DeclaratorContext *context) = 0;

    virtual std::any visitPointerDeclarator(CPP14Parser::PointerDeclaratorContext *context) = 0;

    virtual std::any visitNoPointerDeclarator(CPP14Parser::NoPointerDeclaratorContext *context) = 0;

    virtual std::any visitParametersAndQualifiers(CPP14Parser::ParametersAndQualifiersContext *context) = 0;

    virtual std::any visitTrailingReturnType(CPP14Parser::TrailingReturnTypeContext *context) = 0;

    virtual std::any visitPointerOperator(CPP14Parser::PointerOperatorContext *context) = 0;

    virtual std::any visitCvqualifierseq(CPP14Parser::CvqualifierseqContext *context) = 0;

    virtual std::any visitCvQualifier(CPP14Parser::CvQualifierContext *context) = 0;

    virtual std::any visitRefqualifier(CPP14Parser::RefqualifierContext *context) = 0;

    virtual std::any visitDeclaratorid(CPP14Parser::DeclaratoridContext *context) = 0;

    virtual std::any visitTheTypeId(CPP14Parser::TheTypeIdContext *context) = 0;

    virtual std::any visitAbstractDeclarator(CPP14Parser::AbstractDeclaratorContext *context) = 0;

    virtual std::any visitPointerAbstractDeclarator(CPP14Parser::PointerAbstractDeclaratorContext *context) = 0;

    virtual std::any visitNoPointerAbstractDeclarator(CPP14Parser::NoPointerAbstractDeclaratorContext *context) = 0;

    virtual std::any visitAbstractPackDeclarator(CPP14Parser::AbstractPackDeclaratorContext *context) = 0;

    virtual std::any visitNoPointerAbstractPackDeclarator(CPP14Parser::NoPointerAbstractPackDeclaratorContext *context) = 0;

    virtual std::any visitParameterDeclarationClause(CPP14Parser::ParameterDeclarationClauseContext *context) = 0;

    virtual std::any visitParameterDeclarationList(CPP14Parser::ParameterDeclarationListContext *context) = 0;

    virtual std::any visitParameterDeclaration(CPP14Parser::ParameterDeclarationContext *context) = 0;

    virtual std::any visitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *context) = 0;

    virtual std::any visitFunctionBody(CPP14Parser::FunctionBodyContext *context) = 0;

    virtual std::any visitInitializer(CPP14Parser::InitializerContext *context) = 0;

    virtual std::any visitBraceOrEqualInitializer(CPP14Parser::BraceOrEqualInitializerContext *context) = 0;

    virtual std::any visitInitializerClause(CPP14Parser::InitializerClauseContext *context) = 0;

    virtual std::any visitInitializerList(CPP14Parser::InitializerListContext *context) = 0;

    virtual std::any visitBracedInitList(CPP14Parser::BracedInitListContext *context) = 0;

    virtual std::any visitClassName(CPP14Parser::ClassNameContext *context) = 0;

    virtual std::any visitClassSpecifier(CPP14Parser::ClassSpecifierContext *context) = 0;

    virtual std::any visitClassHead(CPP14Parser::ClassHeadContext *context) = 0;

    virtual std::any visitClassHeadName(CPP14Parser::ClassHeadNameContext *context) = 0;

    virtual std::any visitClassVirtSpecifier(CPP14Parser::ClassVirtSpecifierContext *context) = 0;

    virtual std::any visitClassKey(CPP14Parser::ClassKeyContext *context) = 0;

    virtual std::any visitMemberSpecification(CPP14Parser::MemberSpecificationContext *context) = 0;

    virtual std::any visitMemberdeclaration(CPP14Parser::MemberdeclarationContext *context) = 0;

    virtual std::any visitMemberDeclaratorList(CPP14Parser::MemberDeclaratorListContext *context) = 0;

    virtual std::any visitMemberDeclarator(CPP14Parser::MemberDeclaratorContext *context) = 0;

    virtual std::any visitVirtualSpecifierSeq(CPP14Parser::VirtualSpecifierSeqContext *context) = 0;

    virtual std::any visitVirtualSpecifier(CPP14Parser::VirtualSpecifierContext *context) = 0;

    virtual std::any visitPureSpecifier(CPP14Parser::PureSpecifierContext *context) = 0;

    virtual std::any visitBaseClause(CPP14Parser::BaseClauseContext *context) = 0;

    virtual std::any visitBaseSpecifierList(CPP14Parser::BaseSpecifierListContext *context) = 0;

    virtual std::any visitBaseSpecifier(CPP14Parser::BaseSpecifierContext *context) = 0;

    virtual std::any visitClassOrDeclType(CPP14Parser::ClassOrDeclTypeContext *context) = 0;

    virtual std::any visitBaseTypeSpecifier(CPP14Parser::BaseTypeSpecifierContext *context) = 0;

    virtual std::any visitAccessSpecifier(CPP14Parser::AccessSpecifierContext *context) = 0;

    virtual std::any visitConversionFunctionId(CPP14Parser::ConversionFunctionIdContext *context) = 0;

    virtual std::any visitConversionTypeId(CPP14Parser::ConversionTypeIdContext *context) = 0;

    virtual std::any visitConversionDeclarator(CPP14Parser::ConversionDeclaratorContext *context) = 0;

    virtual std::any visitConstructorInitializer(CPP14Parser::ConstructorInitializerContext *context) = 0;

    virtual std::any visitMemInitializerList(CPP14Parser::MemInitializerListContext *context) = 0;

    virtual std::any visitMemInitializer(CPP14Parser::MemInitializerContext *context) = 0;

    virtual std::any visitMeminitializerid(CPP14Parser::MeminitializeridContext *context) = 0;

    virtual std::any visitOperatorFunctionId(CPP14Parser::OperatorFunctionIdContext *context) = 0;

    virtual std::any visitLiteralOperatorId(CPP14Parser::LiteralOperatorIdContext *context) = 0;

    virtual std::any visitTemplateDeclaration(CPP14Parser::TemplateDeclarationContext *context) = 0;

    virtual std::any visitTemplateparameterList(CPP14Parser::TemplateparameterListContext *context) = 0;

    virtual std::any visitTemplateParameter(CPP14Parser::TemplateParameterContext *context) = 0;

    virtual std::any visitTypeParameter(CPP14Parser::TypeParameterContext *context) = 0;

    virtual std::any visitSimpleTemplateId(CPP14Parser::SimpleTemplateIdContext *context) = 0;

    virtual std::any visitTemplateId(CPP14Parser::TemplateIdContext *context) = 0;

    virtual std::any visitTemplateName(CPP14Parser::TemplateNameContext *context) = 0;

    virtual std::any visitTemplateArgumentList(CPP14Parser::TemplateArgumentListContext *context) = 0;

    virtual std::any visitTemplateArgument(CPP14Parser::TemplateArgumentContext *context) = 0;

    virtual std::any visitTypeNameSpecifier(CPP14Parser::TypeNameSpecifierContext *context) = 0;

    virtual std::any visitExplicitInstantiation(CPP14Parser::ExplicitInstantiationContext *context) = 0;

    virtual std::any visitExplicitSpecialization(CPP14Parser::ExplicitSpecializationContext *context) = 0;

    virtual std::any visitTryBlock(CPP14Parser::TryBlockContext *context) = 0;

    virtual std::any visitFunctionTryBlock(CPP14Parser::FunctionTryBlockContext *context) = 0;

    virtual std::any visitHandlerSeq(CPP14Parser::HandlerSeqContext *context) = 0;

    virtual std::any visitHandler(CPP14Parser::HandlerContext *context) = 0;

    virtual std::any visitExceptionDeclaration(CPP14Parser::ExceptionDeclarationContext *context) = 0;

    virtual std::any visitThrowExpression(CPP14Parser::ThrowExpressionContext *context) = 0;

    virtual std::any visitExceptionSpecification(CPP14Parser::ExceptionSpecificationContext *context) = 0;

    virtual std::any visitDynamicExceptionSpecification(CPP14Parser::DynamicExceptionSpecificationContext *context) = 0;

    virtual std::any visitTypeIdList(CPP14Parser::TypeIdListContext *context) = 0;

    virtual std::any visitNoeExceptSpecification(CPP14Parser::NoeExceptSpecificationContext *context) = 0;

    virtual std::any visitTheOperator(CPP14Parser::TheOperatorContext *context) = 0;

    virtual std::any visitLiteral(CPP14Parser::LiteralContext *context) = 0;


};

