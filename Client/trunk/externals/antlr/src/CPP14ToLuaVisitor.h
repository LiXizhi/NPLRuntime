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
        auto var_name  = GetText(ctx->initDeclaratorList());
        if (type_name.empty()) // 类型为基本类型
        {
            // auto init_declarator_list = ctx->initDeclaratorList();
            // auto init_declarator       = init_declarator_list == nullptr ? nullptr : init_declarator_list->initDeclarator(0);
            // auto declarator = init_declarator == nullptr? nullptr : init_declarator->declarator();
            // auto pointer_declarator = declarator == nullptr? nullptr : declarator->pointerDeclarator();
            // auto no_pointer_declarator = pointer_declarator == nullptr? nullptr : pointer_declarator->noPointerDeclarator();
            // // 函数声明
            // if (declarator != nullptr && declarator->parametersAndQualifiers() != nullptr) return "";

            if (ctx->declSpecifierSeq() != nullptr && var_name.find("(") != std::string::npos && var_name.find("=") == std::string::npos) return NullString();

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
                return var_name + " = " + type_name + "()\n";
            }
        }
    }

    virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) override
    {
#ifdef DEBUG
        CPP14ParserBaseVisitor::visitSelectionStatement(ctx);
#endif
        // 不支持 switch case
        if (ctx->Switch() != nullptr) return NullString();

        auto statements      = ctx->statement();
        auto statements_size = statements.size();
        auto if_stmt         = statements_size > 0 ? statements[0] : nullptr;
        auto else_stmt       = statements_size > 1 ? statements[1] : nullptr;
        std::ostringstream oss;
        oss << "if (" << GetText(ctx->condition()) << ") then" << std::endl << GetText(if_stmt) << std::endl;
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
        return GetText(ctx->statementSeq());
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

//     virtual std::any visitAttributeSpecifier(CPP14Parser::AttributeSpecifierContext *ctx)
//     {
// #ifdef DEBUG
//         CPP14ParserBaseVisitor::visitAttributeSpecifier(ctx);
// #endif
//         return NullString();
//     }
private:
    std::string GetText(antlr4::ParserRuleContext *ctx)
    {
        return ctx == nullptr ? NullString() : std::any_cast<std::string>(ctx->accept(this));
    }
};


#endif