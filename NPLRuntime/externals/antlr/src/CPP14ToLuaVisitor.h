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
    CPP14ToLuaVisitor()
    {
        m_in_switch      = false;
        m_in_switch_case = false;
    }

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

    virtual std::any visitUnaryExpression(CPP14Parser::UnaryExpressionContext *ctx)
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitUnaryExpression(ctx);
#endif
        // std::ostringstream oss;
        if (ctx->unaryExpression() != nullptr)
        {
            if (ctx->PlusPlus() != nullptr)
            {
                auto var = GetText(ctx->unaryExpression());
                return var + " = " + var + " + 1";
                // oss << "(function() local temp = " << var << "; i = i + 1; return temp; end)()";
                // return oss.str();
            }
            else if (ctx->MinusMinus() != nullptr)
            {
                auto var = GetText(ctx->unaryExpression());
                return var + " = " + var + " - 1";
                // oss << "(function() local temp = " << var << "; i = i - 1; return temp; end)()";
                // return oss.str();
            }
        }
        return CPP14ParserBaseVisitor::visitUnaryExpression(ctx);
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
            }
            else
            {
                return CPP14ParserBaseVisitor::visitPostfixExpression(ctx);
            }
        }
        return CPP14ParserBaseVisitor::visitPostfixExpression(ctx);
    }

    virtual std::any visitMultiplicativeExpression(CPP14Parser::MultiplicativeExpressionContext *ctx)
    {
        auto pointerMemberExpression     = ctx->pointerMemberExpression();
        auto pointerMemberExpressionSize = pointerMemberExpression.size();
        auto operand                     = GetText(pointerMemberExpression[0]);
        for (int i = 1; i < pointerMemberExpressionSize; i++)
        {
            auto oper        = ctx->children[i * 2 - 1]->getText();
            auto nextOperand = GetText(pointerMemberExpression[i]);
            if (oper == "*")
            {
                operand = operand + " * " + nextOperand;
            }
            else if (oper == "/")
            {
                operand = operand + " / " + nextOperand;
            }
            else
            {
                operand = "mod_operator(" + operand + ", " + nextOperand + ")";
            }
        }
        return operand;
    }

    virtual std::any visitShiftExpression(CPP14Parser::ShiftExpressionContext *ctx)
    {
        auto additiveExpression     = ctx->additiveExpression();
        auto additiveExpressionSize = additiveExpression.size();
        auto operand                = GetText(additiveExpression[0]);
        bool is_cin_cout            = operand == "cin" || operand == "cout" || operand == "std::cin" || operand == "std::cout";
        std::ostringstream oss;
        for (int i = 1; i < additiveExpressionSize; i++)
        {
            auto oper        = ctx->children[i * 2 - 1]->getText();
            auto nextOperand = GetText(additiveExpression[i]);
            if (oper == "<<")
            {
                if (is_cin_cout)
                {
                    oss << "cout(" << nextOperand << ")" << std::endl;
                }
                else
                {
                    operand = "bit.blshift(" + operand + ", " + nextOperand + ")";
                }
            }
            else
            {
                if (is_cin_cout)
                {
                    oss << nextOperand << " = cin()" << std::endl;
                }
                else
                {
                    operand = "bit.brshift(" + operand + ", " + nextOperand + ")";
                }
            }
        }
        return is_cin_cout ? oss.str() : operand;
    }

    virtual std::any visitAndExpression(CPP14Parser::AndExpressionContext *ctx) override
    {
        auto equalityExpression     = ctx->equalityExpression();
        auto equalityExpressionSize = equalityExpression.size();
        auto operand                = GetText(equalityExpression[0]);
        for (int i = 1; i < equalityExpressionSize; i++)
        {
            auto nextOperand = GetText(equalityExpression[i]);
            operand          = "bit.band(" + operand + ", " + nextOperand + ")";
        }
        return operand;
    }
    virtual std::any visitExclusiveOrExpression(CPP14Parser::ExclusiveOrExpressionContext *ctx) override
    {
        auto andExpression     = ctx->andExpression();
        auto andExpressionSize = andExpression.size();
        auto operand           = GetText(andExpression[0]);
        for (int i = 1; i < andExpressionSize; i++)
        {
            auto nextOperand = GetText(andExpression[i]);
            operand          = "bit.bxor(" + operand + ", " + nextOperand + ")";
        }
        return operand;
    }
    virtual std::any visitInclusiveOrExpression(CPP14Parser::InclusiveOrExpressionContext *ctx) override
    {
        auto exclusiveOrExpression     = ctx->exclusiveOrExpression();
        auto exclusiveOrExpressionSize = exclusiveOrExpression.size();
        auto operand                   = GetText(exclusiveOrExpression[0]);
        for (int i = 1; i < exclusiveOrExpressionSize; i++)
        {
            auto nextOperand = GetText(exclusiveOrExpression[i]);
            operand          = "bit.bor(" + operand + ", " + nextOperand + ")";
        }
        return operand;
    }

    virtual std::any visitConditionalExpression(CPP14Parser::ConditionalExpressionContext *ctx) override
    {
        auto logicalOrExpression = GetText(ctx->logicalOrExpression());

        if (ctx->Question() == nullptr)
        {
            return logicalOrExpression;
        }

        std::ostringstream oss;
        auto expression           = GetText(ctx->expression());
        auto assignmentExpression = GetText(ctx->assignmentExpression());
        oss << "if (" << logicalOrExpression << ") then " << expression << " else " << assignmentExpression << " end";
        return oss.str();
    }

    virtual std::any visitAssignmentExpression(CPP14Parser::AssignmentExpressionContext *ctx)
    {
        auto logicalOrExpressionText = GetText(ctx->logicalOrExpression());
        auto assignmentOperatorText  = GetText(ctx->assignmentOperator());
        auto initializerClauseText   = GetText(ctx->initializerClause());
        auto assignmentOperator      = ctx->assignmentOperator();
        if (assignmentOperator == nullptr) return CPP14ParserBaseVisitor::visitAssignmentExpression(ctx);

        if (assignmentOperator->Assign() != nullptr)
        {
            return logicalOrExpressionText + " = " + initializerClauseText;
        }
        else if (assignmentOperator->PlusAssign() != nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " + " + initializerClauseText;
        }
        else if (assignmentOperator->MinusAssign() != nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " - " + initializerClauseText;
        }
        else if (assignmentOperator->StarAssign() != nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " * " + initializerClauseText;
        }
        else if (assignmentOperator->DivAssign() != nullptr)
        {
            return logicalOrExpressionText + " = " + logicalOrExpressionText + " / " + initializerClauseText;
        }
        else if (assignmentOperator->ModAssign() != nullptr)
        {
            return logicalOrExpressionText + " = mod_operator(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->AndAssign() != nullptr)
        {
            return logicalOrExpressionText + " = bit.band(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->XorAssign() != nullptr)
        {
            return logicalOrExpressionText + " = bit.bxor(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->OrAssign() != nullptr)
        {
            return logicalOrExpressionText + " = bit.bor(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->LeftShiftAssign() != nullptr)
        {
            return logicalOrExpressionText + " = bit.blshift(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else if (assignmentOperator->RightShiftAssign() != nullptr)
        {
            return logicalOrExpressionText + " = bit.brshift(" + logicalOrExpressionText + ", " + initializerClauseText + ")";
        }
        else
        {
            return logicalOrExpressionText + assignmentOperatorText + initializerClauseText;
        }
    }

    virtual std::any visitExpression(CPP14Parser::ExpressionContext *ctx)
    {
        std::ostringstream oss;
        auto assignment_expression      = ctx->assignmentExpression();
        auto assignment_expression_size = assignment_expression.size();
        for (int i = 0; i < assignment_expression_size; i++)
        {
            oss << GetText(assignment_expression[i]);
            if (i < (assignment_expression_size - 1)) oss << std::endl;
        }
        return oss.str();
    }

    virtual std::any visitParameterDeclaration(CPP14Parser::ParameterDeclarationContext *ctx)
    {
        auto declarator = ctx->declarator();
        if (declarator != nullptr)
        {
            auto noPointerDeclarator = declarator->noPointerDeclarator() == nullptr ? declarator->pointerDeclarator()->noPointerDeclarator() : declarator->noPointerDeclarator();
            std::string declaratorid;
            while (noPointerDeclarator != nullptr && declaratorid.empty())
            {
                declaratorid = GetText(noPointerDeclarator->declaratorid());
                if (noPointerDeclarator->noPointerDeclarator() != nullptr) noPointerDeclarator = noPointerDeclarator->noPointerDeclarator();
                if (noPointerDeclarator->pointerDeclarator() != nullptr) noPointerDeclarator = noPointerDeclarator->pointerDeclarator()->noPointerDeclarator();
            }
            return declaratorid;
        }
        return CPP14ParserBaseVisitor::visitParameterDeclaration(ctx);
    }

    virtual std::any visitUsingDeclaration(CPP14Parser::UsingDeclarationContext *ctx) override
    {
        return NullString();
    }

    virtual std::any visitUsingDirective(CPP14Parser::UsingDirectiveContext *ctx) override
    {
        return NullString();
    }

    virtual std::any visitSimpleTypeSpecifier(CPP14Parser::SimpleTypeSpecifierContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitSimpleTypeSpecifier(ctx);
#endif
        return GetText(ctx->theTypeName());
    }

    virtual std::any visitSimpleDeclaration(CPP14Parser::SimpleDeclarationContext *ctx) override
    {
#ifdef DEBUG
        // CPP14ParserBaseVisitor::visitSimpleDeclaration(ctx);
#endif
        auto declSpecifierSeq   = ctx->declSpecifierSeq();
        auto initDeclaratorList = ctx->initDeclaratorList();
        auto initDeclarator     = initDeclaratorList->initDeclarator();
        auto initDeclaratorSize = initDeclarator.size();

        // 函数调用
        if (declSpecifierSeq == nullptr) return GetText(initDeclaratorList) + "\n";

        auto raw_type_name = declSpecifierSeq->stop->getText();
        auto type_name     = GetText(ctx->declSpecifierSeq());

        std::ostringstream oss;
        for (int i = 0; i < initDeclaratorSize; i++)
        {
            auto declarator = GetText(initDeclarator[i]);
            if (type_name.empty()) // 类型为基本类型
            {
                // 移除函数声明 无类型存在(不存在=被解析成函数声明
                if (declarator.find("(") != std::string::npos && declarator.find("=") == std::string::npos) continue;

                // 类型声明
                auto equal_pos    = declarator.find("=");
                std::string name  = "";
                std::string value = "";
                if (equal_pos == std::string::npos)
                {
                    name = declarator;
                }
                else
                {
                    name  = declarator.substr(0, equal_pos);
                    value = declarator.substr(equal_pos + 1);
                }

                // 解析指针符和数组符
                auto star_size  = std::count(name.begin(), name.end(), '*');
                auto array_size = std::count(name.begin(), name.end(), '[');

                // 移除指针符
                if (star_size > 0) name = name.substr(name.find_last_of("*") + 1);

                // 设置默认值 没有默认值且不是数组直接跳过
                if (value.empty())
                {
                    if (raw_type_name == "char")
                    {
                        if ((array_size == 0 && star_size > 0) || (array_size == 1 && star_size == 0))
                        {
                            if (array_size > 0) name = name.substr(0, name.find("["));
                            oss << "local " + name + " = \"\"" << std::endl;
                            continue;
                        }
                        if (star_size == 0 && array_size == 0)
                        {
                            oss << "local " + name + " = 0" << std::endl;
                            continue;
                        }
                    }
                    else if (array_size == 0 && star_size == 0)
                    {
                        oss << "local " + name + " = 0" << std::endl;
                        continue; 
                    }
                }

                // 是数组
                if (name.find("[") != std::string::npos && name.find("]") != std::string::npos)
                {
                    auto bracket_start = name.find("[");
                    auto bracket_end   = name.find("]");
                    auto subname       = name.substr(0, bracket_start);
                    auto arrsize       = name.substr(bracket_start + 1, bracket_end - bracket_start - 1);
                    oss << "local " << subname << " = NewMultiArray(" << arrsize;
                    name = name.substr(bracket_end + 1);
                    while (name.find("[") != std::string::npos && name.find("]") != std::string::npos)
                    {
                        bracket_start = name.find("[");
                        bracket_end   = name.find("]");
                        arrsize       = name.substr(bracket_start + 1, bracket_end - bracket_start - 1);
                        oss << "," << arrsize;
                        name = name.substr(bracket_end + 1);
                    }
                    oss << ")" << std::endl;
                    if (!value.empty())
                    {
                        oss << "InitMultiArray(" << subname << "," << value << ")" << std::endl;
                    }
                }
                else
                {
                    // 保留类型声明
                    oss << "local " << name << " = " << value << std::endl;
                }
            }
            else
            {
                auto pos = declarator.find("(");
                if (pos == std::string::npos)
                {
                    oss << "local " << declarator << " = " << type_name << "()" << std::endl;
                }
                else if (pos == 0)
                {
                    oss << type_name << declarator << std::endl;
                }
                else
                {
                    oss <<  "local " << declarator.substr(0, pos) << " = " << type_name << declarator.substr(pos) << std::endl;
                }
            }
        }
        return oss.str();
    }

    virtual std::any visitSelectionStatement(CPP14Parser::SelectionStatementContext *ctx) override
    {
        std::ostringstream oss;
        auto statements      = ctx->statement();
        auto statements_size = statements.size();
        if (ctx->If() != nullptr)
        {
            auto if_stmt   = statements_size > 0 ? statements[0] : nullptr;
            auto else_stmt = statements_size > 1 ? statements[1] : nullptr;
            oss << "if (" << GetText(ctx->condition()) << ") then" << std::endl
                << GetText(if_stmt);
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
                        << GetText(else_stmt) << "end" << std::endl;
                }
            }
        }
        else if (ctx->Switch() != nullptr)
        {
            auto old_in_switch      = m_in_switch;
            auto old_in_switch_case = m_in_switch_case;
            m_in_switch             = true;
            m_in_switch_case        = false;

            oss << "local __switch_case__ = false;" << std::endl
                << "local __switch_condition__ = " << Trim(GetText(ctx->condition())) << ";" << std::endl
                << "repeat" << std::endl
                << GetText(statements[0]);

            if (m_in_switch_case) oss << "end" << std::endl;
            oss << "until (true);" << std::endl;
            m_in_switch      = old_in_switch;
            m_in_switch_case = old_in_switch_case;
        }

        return oss.str();
    }

    virtual std::any visitLabeledStatement(CPP14Parser::LabeledStatementContext *ctx) override
    {
        std::ostringstream oss;

        if (ctx->Case() != nullptr)
        {
            if (m_in_switch_case) oss << "end" << std::endl;
            m_in_switch_case = true;
            oss << "if (__switch_case__ or (__switch_condition__ == " << GetText(ctx->constantExpression()) << ")) then" << std::endl
                << GetText(ctx->statement());
        }
        else if (ctx->Default() != nullptr)
        {
            if (m_in_switch_case) oss << "end" << std::endl;
            m_in_switch_case = false;
            oss << GetText(ctx->statement());
        }
        else
        {
            return CPP14ParserBaseVisitor::visitLabeledStatement(ctx);
        }
        return oss.str();
    }

    virtual std::any visitExpressionStatement(CPP14Parser::ExpressionStatementContext *ctx) override
    {
        return GetText(ctx->expression());
    }

    virtual std::any visitIterationStatement(CPP14Parser::IterationStatementContext *ctx) override
    {
        std::ostringstream oss;

        if (ctx->For() != nullptr)
        {
            auto for_ctr            = ctx->For();
            auto for_init_statement = ctx->forInitStatement();
            oss << GetText(for_init_statement) << std::endl
                << "while (" << Trim(GetText(ctx->condition())) << ") do" << std::endl
                << GetText(ctx->statement())
                << GetText(ctx->expression()) << std::endl
                << "end";
        }
        else if (ctx->Do() != nullptr)
        {
            oss << "repeat" << std::endl
                << GetText(ctx->statement())
                << "until (not (" << GetText(ctx->expression()) << "))";
        }
        else if (ctx->While() != nullptr)
        {
            oss << "while (" << GetText(ctx->condition()) << ") do" << std::endl
                << GetText(ctx->statement())
                << "end";
        }
        return oss.str();
    }

    virtual std::any visitJumpStatement(CPP14Parser::JumpStatementContext *ctx) override
    {
        return std::any_cast<std::string>(CPP14ParserBaseVisitor::visitJumpStatement(ctx));
    }

    virtual std::any visitStatement(CPP14Parser::StatementContext *ctx)
    {
        auto text = std::any_cast<std::string>(CPP14ParserBaseVisitor::visitStatement(ctx));
        return Trim(text) + "\n";
    }

    virtual std::any visitCompoundStatement(CPP14Parser::CompoundStatementContext *ctx) override
    {
        return GetText(ctx->statementSeq());
    }

    virtual std::any visitLambdaExpression(CPP14Parser::LambdaExpressionContext *ctx) override
    {
        auto lambdaDeclarator           = ctx->lambdaDeclarator();
        auto parameterDeclarationClause = GetText(lambdaDeclarator == nullptr ? nullptr : lambdaDeclarator->parameterDeclarationClause());
        auto compoundStatement          = GetText(ctx->compoundStatement());
        std::ostringstream oss;
        oss << "function(" << parameterDeclarationClause << ")" << std::endl
            << compoundStatement << "end" << std::endl;
        return oss.str();
    }

    virtual std::any visitFunctionBody(CPP14Parser::FunctionBodyContext *ctx) override
    {
        return CPP14ParserBaseVisitor::visitFunctionBody(ctx);
    }

    virtual std::any visitFunctionDefinition(CPP14Parser::FunctionDefinitionContext *ctx) override
    {
        std::ostringstream oss;
        oss << "function " << GetText(ctx->declarator()) << std::endl
            << GetText(ctx->functionBody()) << "end" << std::endl;
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

private:
    bool m_in_switch;
    bool m_in_switch_case;
};

#endif

// virtual std::any visitEqualityExpression(CPP14Parser::EqualityExpressionContext *ctx)
// {
//     auto relationalExpression     = ctx->relationalExpression();
//     auto relationalExpressionSize = relationalExpression.size();
//     auto operand                  = GetText(relationalExpression[0]);

//     for (int i = 1; i < relationalExpressionSize; i++)
//     {
//         auto oper        = ctx->children[i * 2 - 1]->getText();
//         auto nextOperand = GetText(relationalExpression[i]);
//         if (oper != "==")
//         {
//             operand = operand + " == " + nextOperand;
//         }
//         else
//         {
//             operand = operand + " ~= " + nextOperand;
//         }
//     }
//     return operand;
// }
// virtual std::any visitLogicalAndExpression(CPP14Parser::LogicalAndExpressionContext *ctx) override
// {
//     auto inclusiveOrExpression     = ctx->inclusiveOrExpression();
//     auto inclusiveOrExpressionSize = inclusiveOrExpression.size();
//     auto operand                   = GetText(inclusiveOrExpression[0]);
//     for (int i = 1; i < inclusiveOrExpressionSize; i++)
//     {
//         auto nextOperand = GetText(inclusiveOrExpression[i]);
//         operand          = operand + " and " + nextOperand;
//     }
//     return operand;
// }
// virtual std::any visitLogicalOrExpression(CPP14Parser::LogicalOrExpressionContext *ctx) override
// {
//     auto logicalAndExpression     = ctx->logicalAndExpression();
//     auto logicalAndExpressionSize = logicalAndExpression.size();
//     auto operand                  = GetText(logicalAndExpression[0]);
//     for (int i = 1; i < logicalAndExpressionSize; i++)
//     {
//         auto nextOperand = GetText(logicalAndExpression[i]);
//         operand          = operand + " or " + nextOperand;
//     }
//     return operand;
// }
