#ifndef CPP14TOLUA_H
#define CPP14TOLUA_H

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
        auto text = symbol->getText();
        auto type = symbol->getType();
        if (type == CPP14Parser::EOF)
        {
            return std::string("");
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

    virtual std::any visitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitSimpleTypeSpecifier(ctx);
#endif
        return GetText(ctx->theTypeName());
        // 根据条件 返回 local 或空
        // return NullString();
    }

    virtual std::any visitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitSimpleDeclaration(ctx);
#endif

        auto type_name = GetText(ctx->declSpecifierSeq());
        auto var_name = GetText(ctx->initDeclaratorList());
        if (type_name.empty())
        {
            if (ctx->declSpecifierSeq() == nullptr)
            {
                return var_name + "\n";
            }
            else
            {
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
                return var_name + " " + type_name + "()\n";
            }
        }
    }

    virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitSelectionStatement(ctx);
#endif
        auto statements = ctx->statement();
        auto statements_size = statements.size();
        auto if_stmt = statements_size > 0 ? GetText(statements[0]) : NullString();
        std::ostringstream oss;
        oss << "if (" << GetText(ctx->condition()) << ") then" << std::endl
            << if_stmt << std::endl;
        if (ctx->Else() == nullptr || statements_size <= 1)
        {
            oss << "end" << std::endl;
        }
        else
        {
            if (statements.size() > 2)
            {
                oss << "else" << GetText(statements[1]);
            }
            else
            {
                oss << "else" << std::endl
                    << GetText(statements[1]) << std::endl
                    << "end" << std::endl;
            }
        }
        return oss.str();
    }

    virtual std::any visitExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitExpressionStatement(ctx);
#endif
        return GetText(ctx->expression()) + "\n";
    }

    virtual std::any visitIterationStatement(CPP14Parser::IterationStatementContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitIterationStatement(ctx);
#endif

        std::ostringstream oss;

        if (ctx->While())
        {
            oss << "while (" << GetText(ctx->condition()) << ") do" << std::endl
                << GetText(ctx->statement()) << std::endl
                << "end";
        }

        return oss.str();
    }

    virtual std::any visitCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitCompoundStatement(ctx);
#endif

        std::ostringstream oss;
        oss << "(function()" << std::endl
            << GetText(ctx->statementSeq()) << std::endl
            << "end)()";
        return oss.str();
    }

    virtual std::any visitFunctionBody(CPP14Parser::FunctionBodyContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitFunctionBody(ctx);
#endif
        std::ostringstream oss;
        oss << "return " << GetText(ctx->compoundStatement());
        return oss.str();
    }

    virtual std::any visitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitFunctionDefinition(ctx);
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
};

#endif