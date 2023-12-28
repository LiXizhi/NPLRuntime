#ifndef __CPP14_TO_LUA_VISITOR_H__
#define __CPP14_TO_LUA_VISITOR_H__

#include "CPP14Lexer.h"
#include "CPP14Parser.h"
#include "CPP14ParserBaseVisitor.h"
#include "antlr4-runtime.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_set>

#define DEBUG

class CPP14ToLuaVisitor : public CPP14ParserBaseVisitor
{
public:
    CPP14ToLuaVisitor() {}
    ~CPP14ToLuaVisitor() {}

    std::string NullString() { return ""; }

protected:
    virtual std::any defaultResult()
    {
        return NullString();
    }

    virtual std::any aggregateResult(std::any aggregate, std::any nextResult)
    {
        return std::any_cast<std::string>(aggregate) + std::any_cast<std::string>(nextResult);
    }

public:
    virtual std::any visitTerminal(antlr4::tree::TerminalNode *node) override
    {
        auto symbol = node->getSymbol();
        auto text   = symbol->getText();
        auto type   = symbol->getType();
        if (type == CPP14Parser::EOF)
        {
            return std::string("");
        }
        else if (symbol->getType() == CPP14Parser::Not)
        {
            return std::string("not ");
        }
        else if (symbol->getType() == CPP14Parser::AndAnd)
        {
            return std::string(" and ");
        }
        else if (symbol->getType() == CPP14Parser::OrOr)
        {
            return std::string(" or ");
        }
        else if (symbol->getType() == CPP14Parser::NotEqual)
        {
            return std::string(" ~= ");
        }
        else if (type == CPP14Parser::Return)
        {
            return text + " ";
        }
        else
        {
            return text;
        }
    }

    virtual std::any visitPostfixExpression(CPP14Parser::PostfixExpressionContext *ctx)
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitPostfixExpression(ctx);
#endif
        // std::ostringstream oss;
        if (ctx->postfixExpression() != nullptr)
        {
            if (ctx->PlusPlus() != nullptr)
            {
                auto var = GetText(ctx->postfixExpression());
                return var + " = " + var + " + 1";
                // oss << "(function() local temp = " << var << "; i = i + 1; return temp; end)()";
                // return oss.str();
            }
            else if (ctx->MinusMinus() != nullptr)
            {
                auto var = GetText(ctx->postfixExpression());
                return var + " = " + var + " - 1";
                // oss << "(function() local temp = " << var << "; i = i - 1; return temp; end)()";
                // return oss.str();
            }
            else
            {
                return CPP14ParserBaseVisitor::visitPostfixExpression(ctx);
            }
        }
        return CPP14ParserBaseVisitor::visitPostfixExpression(ctx);
    }

    // virtual std::any virtualMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *ctx)
    // {

    // }

    virtual std::any visitAssignmentExpression(CPP14Parser::AssignmentExpressionContext *ctx)
    {
        auto logicalOrExpressionText = GetText(ctx->logicalOrExpression());
        auto assignmentOperatorText                  = GetText(ctx->assignmentOperator());
        auto initializerClauseText = GetText(ctx->initializerClause());
        auto assignmentOperator = ctx->assignmentOperator();
        if (assignmentOperator == nullptr) return CPP14ParserBaseVisitor::visitAssignmentExpression(ctx);

        if (assignmentOperator->PlusAssign() != nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " + " + initializerClauseText;
        }
        else if (assignmentOperator->MinusAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " - " + initializerClauseText;
        }
        else if (assignmentOperator->StarAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " * " + initializerClauseText;
        }
        else if (assignmentOperator->DivAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " / " + initializerClauseText;
        }
        else if (assignmentOperator->ModAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = mod_operator(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->AndAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = bit.band(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->XorAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = bit.bxor(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->OrAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = bit.bor(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->LeftShiftAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = bit.blshift(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->RightShiftAssign()!= nullptr)
        {
            return logicalOrExpressionText + " = bit.brshift(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else
        {
            return logicalOrExpressionText + logicalOrExpressionText + initializerClauseText;
        }
    }

    virtual std::any visitUnaryExpression(CPP14Parser::UnaryExpressionContext *ctx)
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitUnaryExpression(ctx);
#endif
        if (ctx->unaryExpression() != nullptr)
        {
            if (ctx->PlusPlus() != nullptr)
            {
                auto var = GetText(ctx->unaryExpression());
                return var + " = " + var + " + 1";
            }
            else if (ctx->MinusMinus() != nullptr)
            {
                auto var = GetText(ctx->unaryExpression());
                return var + " = " + var + " - 1";
            }
        }
        return CPP14ParserBaseVisitor::visitUnaryExpression(ctx);
    }

    // virtual std::any visitExpressionStatement
    virtual std::any visitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitSimpleTypeSpecifier(ctx);
#endif
        return GetText(ctx->theTypeName());
        // 根据条件 返回 local 或空
        // return NullString();
    }

    virtual std::any visitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitSimpleDeclaration(ctx);
#endif

        auto type_name = GetText(ctx->declSpecifierSeq());
        auto var_name  = GetText(ctx->initDeclaratorList());
        if (type_name.empty()) // 类型为基本类型
        {
            if (ctx->declSpecifierSeq() == nullptr)
            {
                // 函数调用
                return var_name + "\n";
            }
            else
            {
                // 类型声明
                // 移除函数声明
                if (var_name.find("(") != std::string::npos && var_name.find("=") == std::string::npos) return NullString();
                // 保留类型声明
                return "local " + var_name + "\n";
            }
        }
        else
        {
            auto pos = var_name.find("(");
            if (pos != std::string::npos)
            {
                return "local " + var_name.substr(0, pos) + " = " + type_name + var_name.substr(pos) + "\n";
            }
            else
            {
                return var_name + " = " + type_name + "()\n";
            }
        }
    }

    virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitSelectionStatement(ctx);
#endif
        // 不支持 switch case
        if (ctx->Switch() != nullptr) return NullString();

        auto statements      = ctx->statement();
        auto statements_size = statements.size();
        auto if_stmt         = statements_size > 0 ? statements[0] : nullptr;
        auto else_stmt       = statements_size > 1 ? statements[1] : nullptr;
        std::ostringstream oss;
        oss << "if (" << GetText(ctx->condition()) << ") then" << std::endl
            << GetText(if_stmt) << std::endl;
        if (ctx->Else() == nullptr || statements_size <= 1)
        {
            oss << "end" << std::endl;
        }
        else
        {
            if (else_stmt->selectionStatement() != nullptr)
            {
                oss << "else" << GetText(else_stmt);
            }
            else
            {
                oss << "else" << std::endl
                    << GetText(else_stmt) << std::endl
                    << "end" << std::endl;
            }
        }
        return oss.str();
    }

    virtual std::any visitExpression(CPP14Parser::ExpressionContext *ctx)
    {
        std::ostringstream oss;
        auto assignment_expression = ctx->assignmentExpression();
        for (int i = 0; i < assignment_expression.size(); i++)
        {
            oss << GetText(assignment_expression[i]) << std::endl;
        }
        return oss.str();
    }

    virtual std::any visitExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitExpressionStatement(ctx);
#endif
        auto text = GetText(ctx->expression()) + "\n";
        return text;
    }

    virtual std::any visitIterationStatement(CPP14Parser::IterationStatementContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitIterationStatement(ctx);
#endif

        std::ostringstream oss;

        if (ctx->While() != nullptr)
        {
            oss << "while (" << GetText(ctx->condition()) << ") do" << std::endl
                << GetText(ctx->statement()) << std::endl
                << "end";
        }
        else if (ctx->For() != nullptr)
        {
            auto for_ctr            = ctx->For();
            auto for_init_statement = ctx->forInitStatement();
            if (for_init_statement != nullptr)
            {
                oss << GetText(for_init_statement)
                    << "while (" << Trim(GetText(ctx->condition())) << ") do" << std::endl
                    << GetText(ctx->statement()) << std::endl
                    << GetText(ctx->expression()) << std::endl
                    << "end";
            }
        }

        return oss.str();
    }

    virtual std::any visitStatement(CPP14Parser::StatementContext *ctx)
    {
        auto text = std::any_cast<std::string>(CPP14ParserBaseVisitor::visitStatement(ctx));
        return text;
    }

    virtual std::any visitCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) override
    {
        auto text = GetText(ctx->statementSeq());
        return text;
    }

    virtual std::any visitFunctionBody(CPP14Parser::FunctionBodyContext *ctx) override
    {
        return CPP14ParserBaseVisitor::visitFunctionBody(ctx);
    }

    virtual std::any visitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitFunctionDefinition(ctx);
#endif
        std::ostringstream oss;
        oss << "function " << GetText(ctx->declarator()) << std::endl
            << GetText(ctx->functionBody()) << std::endl
            << "end" << std::endl;
        return oss.str();
    }

private:
    std::string GetText(antlr4::ParserRuleContext *ctx)
    {
        return ctx == nullptr ? NullString() : std::any_cast<std::string>(ctx->accept(this));
    }

    std::string Trim(std::string str)
    {
        // clang-format off
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
        return str;
        // clang-format on
    }
};

#endif