#include "Python3ParserBaseVisitor.h"

#include <string>
#include <unordered_set>
#include <numeric>

class Python3ToLuaVisitor : public Python3ParserBaseVisitor
{
    class Scope
    {
    public:
        Scope(std::shared_ptr<Scope> parent = nullptr) : m_parent(parent) {}
        bool AddName(std::string name)
        {
            auto it = m_names.find(name);
            if (it == m_names.end())
            {
                m_names.insert(name);
                return true;
            }
            else
            {
                return false;
            }
        }

        bool IsExistName(std::string name)
        {
            auto it = m_names.find(name);
            if (it == m_names.end())
            {
                return m_parent == nullptr ? false : m_parent->IsExistName(name);
            }
            return true;
        }

        std::shared_ptr<Scope> GetParent() { return m_parent; }

    public:
        std::shared_ptr<Scope> m_parent;
        std::unordered_set<std::string> m_names;
    };

public:
    Python3ToLuaVisitor()
    {
        m_scope = std::make_shared<Scope>();
        m_scope_type = s_scope_type_global;
        m_contain_yield = false;
    }

protected:
    virtual std::any defaultResult()
    {
        return std::string("");
    }

    virtual std::any aggregateResult(std::any aggregate, std::any nextResult)
    {
        return std::any_cast<std::string>(aggregate) + std::any_cast<std::string>(nextResult);
    }

public:
    virtual std::any visitTerminal(antlr4::tree::TerminalNode *node) override
    {
        auto symbol = node->getSymbol();
        if (symbol->getType() == Python3Parser::EOF)
        {
            return std::string("");
        }
        else if (symbol->getType() == Python3Parser::NOT)
        {
            return std::string("not ");
        }
        else if (symbol->getType() == Python3Parser::RETURN)
        {
            return std::string("return ");
        }
        else if (symbol->getType() == Python3Parser::STRING)
        {
            std::string str = node->getText();
            if ((str.substr(0, 3) == "\"\"\"" && str.substr(str.size() - 3) == "\"\"\"") || (str.substr(0, 3) == "'''" && str.substr(str.size() - 3) == "'''"))
            {
                return "--[[" + str.substr(3, str.size() - 6) + "]]";
            }
            else
            {
                return node->getText();
            }
        }
        else
        {
            return node->getText();
        }
    }

    virtual std::any visitName(Python3Parser::NameContext *ctx) override
    {
        return GetScopePrefix() + ctx->getText();
    }

    virtual std::any visitArgument(Python3Parser::ArgumentContext *ctx) override
    {
        if (ctx->STAR())
        {
            return "unpack(" + GetText(ctx->test(0)) + ")";
        }
        else
        {
            return Python3ParserBaseVisitor::visitArgument(ctx);
        }
    }

    virtual std::any visitSubscript_(Python3Parser::Subscript_Context *ctx) override
    {
        if (ctx->COLON())
        {
            auto tests = ctx->test();
            auto tests_size = tests.size();
            auto sliceop = ctx->sliceop();
            std::string sliceop_str = (sliceop == nullptr || sliceop->test() == nullptr) ? std::string("nil") : GetText(sliceop->test());
            if (tests_size == 0)
            {
                return std::string("slice(nil, nil, nil)");
            }
            else if (tests_size == 2)
            {
                return "slice(" + GetText(tests[0]) + ", " + GetText(tests[1]) + ", " + sliceop_str + ")";
            }
            else
            {
                auto test = tests[0];
                auto colon = ctx->COLON();
                if (test->start->getTokenIndex() < colon->getSymbol()->getTokenIndex())
                {
                    return "slice(" + GetText(test) + ", nil, " + sliceop_str + ")";
                }
                else
                {
                    return "slice(nil, " + GetText(test) + ", " + sliceop_str + ")";
                }
            }
        }
        else
        {
            return GetText(ctx->test(0));
        }
    }

    virtual std::any visitAtom(Python3Parser::AtomContext *ctx) override
    {
        if (ctx->TRUE())
        {
            return std::string("true");
        }
        else if (ctx->FALSE())
        {
            return std::string("false");
        }
        else if (ctx->NONE())
        {
            return std::string("nil");
        }
        else if (ctx->NUMBER())
        {
            auto text = ctx->getText();
            if (text.size() > 2 && text[0] == '0')
            {
                if (text[1] == 'b' || text[1] == 'B')
                {
                    return std::to_string(std::stoi(text.substr(2), nullptr, 2));
                }
                if (text[1] == 'o' || text[1] == 'O')
                {
                    return std::to_string(std::stoi(text.substr(2), nullptr, 8));
                }
                if (text[1] == 'x' || text[1] == 'X')
                {
                    return std::to_string(std::stoi(text.substr(2), nullptr, 16));
                }
            }
            return text;
        }
        else if (ctx->OPEN_PAREN() && ctx->CLOSE_PAREN())
        {
            auto testlist_comp = ctx->testlist_comp();
            std::string result = GetText(ctx->yield_expr());
            if (testlist_comp != nullptr)
            {
                if (testlist_comp->comp_for() == nullptr)
                {
                    std::string result = GetText(testlist_comp->test(0));
                    auto size = testlist_comp->test().size() - 1;
                    for (auto i = 1; i <= size; i++)
                    {
                        result += ", " + GetText(testlist_comp->test(i));
                    }
                    if (testlist_comp->COMMA(0) == nullptr)
                    {
                        return "(" + result + ")";
                    }
                    else
                    {
                        return "tuple({" + result + "})";
                    }
                }
                else
                {
                    std::string old_comp_for_stmts = m_comp_for_stmts;
                    m_comp_for_stmts = "coroutine.yield(" + GetText(testlist_comp->test(0)) + ")";
                    std::string result = GetText(testlist_comp->comp_for());
                    m_comp_for_stmts = old_comp_for_stmts;
                    return "coroutine_wrap(function()\n" + result + " end)";
                }
            }
            else
            {
                return result;
            }
        }
        else if (ctx->OPEN_BRACK() && ctx->CLOSE_BRACK())
        {
            auto testlist_comp = ctx->testlist_comp();
            if (testlist_comp == nullptr || testlist_comp->comp_for() == nullptr)
            {
                std::string result;
                if (testlist_comp != nullptr)
                {
                    result = GetText(testlist_comp->test(0));
                    auto size = testlist_comp->test().size() - 1;
                    for (auto i = 1; i <= size; i++)
                    {
                        result += ", " + GetText(testlist_comp->test(i));
                    }
                }
                return std::string("list({_to_null(") + result + ")})";
            }
            else
            {
                std::string old_comp_for_stmts = m_comp_for_stmts;
                std::string result = "(function()\n";
                result += "local __comp_for_list__ = list({})\n";
                m_comp_for_stmts = "__comp_for_list__.append(" + GetText(testlist_comp->test(0)) + ")";
                result += GetText(testlist_comp->comp_for());
                result += "return __comp_for_list__";
                result += "\nend)()";
                m_comp_for_stmts = old_comp_for_stmts;
                return result;
            }
        }
        else if (ctx->OPEN_BRACE() && ctx->CLOSE_BRACE())
        {
            auto dictorsetmaker = ctx->dictorsetmaker();
            auto dictorsetmaker_str = GetText(dictorsetmaker);
            bool is_dict = dictorsetmaker && dictorsetmaker->COLON().size() > 0;
            if (dictorsetmaker == nullptr || dictorsetmaker->comp_for() == nullptr)
            {
                if (dictorsetmaker && dictorsetmaker->COLON(0) == nullptr)
                {
                    return std::string("set({") + dictorsetmaker_str + "})";
                }
                else
                {
                    return std::string("dict({") + dictorsetmaker_str + "})";
                }
            }
            else
            {
                return dictorsetmaker_str;
            }
        }
        else
        {
            return Python3ParserBaseVisitor::visitAtom(ctx);
        }
    }

    virtual std::any visitYield_expr(Python3Parser::Yield_exprContext *ctx) override
    {
        auto yield_arg = ctx->yield_arg();
        m_contain_yield = true;
        return "coroutine.yield(" + (yield_arg->test() ? GetText(yield_arg->test()) : GetText(yield_arg->testlist())) + ")";
    }

    virtual std::any visitComp_for(Python3Parser::Comp_forContext *ctx) override
    {
        std::string result = "for _, " + GetText(ctx->exprlist()) + " in " + GetText(ctx->or_test()) + " do\n";
        result += ctx->comp_iter() ? GetText(ctx->comp_iter()) : m_comp_for_stmts;
        result += "\nend\n";
        return result;
    }

    virtual std::any visitComp_if(Python3Parser::Comp_ifContext *ctx) override
    {
        std::string result = "if (" + GetText(ctx->test_nocond()) + ") then\n";
        result += ctx->comp_iter() ? GetText(ctx->comp_iter()) : m_comp_for_stmts;
        result += "\nend\n";
        return result;
    }

    virtual std::any visitDictorsetmaker(Python3Parser::DictorsetmakerContext *ctx) override
    {
        auto comp_for = ctx->comp_for();
        if (comp_for == nullptr)
        {
            if (ctx->COLON(0) == nullptr)
            {
                std::string result = GetText(ctx->test(0));
                auto size = ctx->test().size() - 1;
                for (auto i = 1; i <= size; i++)
                {
                    result += ", " + GetText(ctx->test(i));
                }
                return "_to_null(" + result + ")";
            }
            else
            {
                auto tests = ctx->test();
                auto size = ctx->test().size() / 2;
                std::string result;
                for (auto i = 0; i < size; i++)
                {
                    result += "[_to_null(" + GetText(tests[2 * i]) + ")] = " + "_to_null(" + GetText(tests[2 * i + 1]) + ")" + (i == size - 1 ? "" : ", ");
                }
                return result;
            }
        }
        else
        {
            std::string result = "(function()\n";
            std::string old_comp_for_stmts = m_comp_for_stmts;
            if (ctx->COLON(0) == nullptr)
            {
                result += "local __comp_for_set__ = set({})\n";
                m_comp_for_stmts = "__comp_for_set__.add(" + GetText(ctx->test(0)) + ")";
                result += GetText(ctx->comp_for());
                result += "return __comp_for_set__";
            }
            else
            {
                result += "local __comp_for_dict__ = dict({})\n";
                m_comp_for_stmts = "__comp_for_dict__[" + GetText(ctx->test(0)) + "] = " + GetText(ctx->test(1));
                result += GetText(ctx->comp_for());
                result += "return __comp_for_dict__";
            }
            result += "\nend)()";
            m_comp_for_stmts = old_comp_for_stmts;
            return result;
        }
    }

    virtual std::any visitTrailer(Python3Parser::TrailerContext *ctx) override
    {
        if (ctx->subscriptlist())
        {
            return std::string("[_to_null(" + std::any_cast<std::string>(ctx->subscriptlist()->accept(this)) + ")]");
        }
        if (ctx->arglist())
        {
            auto arglist = ctx->arglist()->argument();
            auto arglist_size = arglist.size();
            std::string result = "";
            std::string dict = "";
            for (auto i = 0; i < arglist_size; i++)
            {
                auto arg = arglist[i];
                if (arg->ASSIGN())
                {
                    dict += std::string(dict.empty() ? "" : ", ") + GetText(arg->test(0)) + " = " + GetText(arg->test(1));
                }
                else
                {
                    result += std::string(result.empty() ? "" : ", ") + GetText(arg);
                }
            }
            if (!dict.empty())
            {
                result += std::string(result.empty() ? "" : ", ") + "{" + dict + "}";
            }
            result = "(" + result + ")";
            return result;
        }
        return Python3ParserBaseVisitor::visitTrailer(ctx);
    }

    virtual std::any visitTest(Python3Parser::TestContext *ctx) override
    {
        if (ctx->IF())
        {
            return "(" + GetText(ctx->or_test(1)) + ") and (" + GetText(ctx->or_test(0)) + ") or (" + GetText(ctx->test()) + ")";
        }
        else if (ctx->lambdef())
        {
            return GetText(ctx->lambdef());
        }
        else
        {
            return GetText(ctx->or_test(0));
        }
    }

    virtual std::any visitOr_test(Python3Parser::Or_testContext *ctx) override
    {
        auto and_tests = ctx->and_test();
        auto ors = ctx->OR();
        std::string result = std::any_cast<std::string>(and_tests[0]->accept(this));
        for (size_t i = 0; i < ors.size(); i++)
        {
            result += " or " + std::any_cast<std::string>(and_tests[i + 1]->accept(this));
        }
        return result;
    }

    virtual std::any visitAnd_test(Python3Parser::And_testContext *ctx) override
    {
        auto not_tests = ctx->not_test();
        auto ands = ctx->AND();
        std::string result = std::any_cast<std::string>(not_tests[0]->accept(this));
        for (size_t i = 0; i < ands.size(); i++)
        {
            result += " and " + std::any_cast<std::string>(not_tests[i + 1]->accept(this));
        }
        return result;
    }

    virtual std::any visitNot_test(Python3Parser::Not_testContext *ctx) override
    {
        return Python3ParserBaseVisitor::visitNot_test(ctx);
    }

    virtual std::any visitComparison(Python3Parser::ComparisonContext *ctx) override
    {
        auto ops = ctx->comp_op();
        auto exprs = ctx->expr();
        auto size = ops.size();
        std::vector<std::string> strs;
        strs.push_back(GetText(exprs[0]));
        for (auto i = 0; i < size; i++)
        {
            std::string op = GetText(ops[i]);
            std::string expr = GetText(exprs[i + 1]);
            if (op == "!=")
            {
                op = " ~= " + expr;
            }
            else if (op == "is")
            {
                strs[strs.size() - 1] = "operator_is(" + strs[strs.size() - 1] + "," + expr + ")";
                continue;
            }
            else if (op == "isnot " || op == "is not")
            {
                strs[strs.size() - 1] = "not operator_is(" + strs[strs.size() - 1] + "," + expr + ")";
                continue;
            }
            else if (op == "in")
            {
                strs[strs.size() - 1] = "operator_in(" + strs[strs.size() - 1] + "," + expr + ")";
                continue;
            }
            else if (op == "not in")
            {
                strs[strs.size() - 1] = "not operator_in(" + strs[strs.size() - 1] + "," + expr + ")";
                continue;
            }
            else
            {
                op = " " + op + " " + expr;
            }
            strs.push_back(op);
        }
        return std::accumulate(strs.begin(), strs.end(), std::string(""));
    }

    virtual std::any visitExpr(Python3Parser::ExprContext *ctx) override
    {
        auto adds = ctx->ADD();
        auto minus = ctx->MINUS();
        auto nots = ctx->NOT_OP();
        auto exprs_size = ctx->expr().size();
        if (exprs_size > 1)
        {
            auto left_expr_str = std::any_cast<std::string>(ctx->expr(0)->accept(this));
            auto right_expr_str = std::any_cast<std::string>(ctx->expr(1)->accept(this));
            if (ctx->POWER())
            {
                return "math.pow(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->MOD())
            {
                return "mod_operator(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->IDIV())
            {
                return "math.floor(" + left_expr_str + " / " + right_expr_str + ")";
            }
            else if (ctx->LEFT_SHIFT())
            {
                return "bit.blshift(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->RIGHT_SHIFT())
            {
                return "bit.brshift(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->AND_OP())
            {
                return "bit.band(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->XOR())
            {
                return "bit.bxor(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else if (ctx->OR_OP())
            {
                return "bit.bor(" + left_expr_str + ", " + right_expr_str + ")";
            }
            else
            {
                return Python3ParserBaseVisitor::visitExpr(ctx);
            }
        }
        if (exprs_size > 0)
        {
            auto expr_str = std::any_cast<std::string>(ctx->expr(0)->accept(this));
            if (adds.size() > 0)
            {
                return expr_str;
            }
            else if (minus.size() > 0)
            {
                return minus.size() % 2 == 1 ? ("-" + expr_str) : expr_str;
            }
            else if (nots.size() > 0)
            {
                return nots.size() % 2 == 1 ? ("bit.bnot(" + expr_str + ")") : expr_str;
            }
            else
            {
                return Python3ParserBaseVisitor::visitExpr(ctx);
            }
        }
        else
        {
            return Python3ParserBaseVisitor::visitExpr(ctx);
        }
    }

    virtual std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override
    {
        std::string result;
        auto assigns = ctx->ASSIGN();
        auto assigns_size = assigns.size();
        auto augassign = ctx->augassign();
        if (assigns_size > 0)
        {
            auto testlist_star_exprs = ctx->testlist_star_expr();
            auto testlist_star_exprs_size = testlist_star_exprs.size();
            std::string right_operand;
            if (ctx->yield_expr(0) != nullptr)
            {
                right_operand = GetText(ctx->yield_expr(0));
            }
            else
            {
                right_operand = GetText(testlist_star_exprs[testlist_star_exprs_size - 1]);
                testlist_star_exprs_size--;
            }
            for (int i = static_cast<int>(testlist_star_exprs_size - 1); i >= 0; i--)
            {
                auto testlist_star_expr = testlist_star_exprs[i];
                bool is_exist_name = true;
                auto tests = testlist_star_expr->test();
                auto tests_size = tests.size();
                for (auto i = 0; i < tests_size; i++)
                {
                    auto name = GetText(tests[i]);
                    if (m_scope->AddName(name))
                    {
                        is_exist_name = false;
                    }
                }
                auto left_operand = GetText(testlist_star_expr);
                result += (is_exist_name ? "" : "local ") + left_operand + "=" + right_operand + "\n";
                right_operand = left_operand;
            }
        }
        else if (augassign)
        {
            auto left_testlist = ctx->testlist_star_expr(0);
            auto right_testlist = ctx->testlist();
            auto augassign_str = std::any_cast<std::string>(augassign->accept(this));
            int testlist_size = static_cast<int>(left_testlist->test().size());
            for (int i = testlist_size - 1; i >= 0; i--)
            {
                auto left_operand = std::any_cast<std::string>(left_testlist->test(i)->accept(this));
                auto right_operand = std::any_cast<std::string>(right_testlist->test(i)->accept(this));
                // ('+=' | '-=' | '*=' | '@=' | '/=' | '%=' | '&=' | '|=' | '^=' | '<<=' | '>>=' | '**=' | '//=')   @= 矩阵运算不支持
                if (augassign_str == "+=")
                {
                    result += left_operand + " = " + left_operand + " + " + right_operand + "\n";
                }
                else if (augassign_str == "-=")
                {
                    result += left_operand + " = " + left_operand + " - " + right_operand + "\n";
                }
                else if (augassign_str == "*=")
                {
                    result += left_operand + " = " + left_operand + " * " + right_operand + "\n";
                }
                else if (augassign_str == "/=")
                {
                    result += left_operand + " = " + left_operand + " / " + right_operand + "\n";
                }
                else if (augassign_str == "%=")
                {
                    result += left_operand + " = mod_operator(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == "//=")
                {
                    result += left_operand + " = math.floor(" + left_operand + " / " + right_operand + ")\n";
                }
                else if (augassign_str == "**=")
                {
                    result += left_operand + " = math.pow(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == "&=")
                {
                    result += left_operand + " = bit.band(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == "|=")
                {
                    result += left_operand + " = bit.bor(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == "^=")
                {
                    result += left_operand + " = bit.bxor(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == "<<=")
                {
                    result += left_operand + " = bit.blshift(" + left_operand + ", " + right_operand + ")\n";
                }
                else if (augassign_str == ">>=")
                {
                    result += left_operand + " = bit.brshift(" + left_operand + ", " + right_operand + ")\n";
                }
            }
        }
        else
        {
            return Python3ParserBaseVisitor::visitExpr_stmt(ctx);
        }
        return result;
    }

    virtual std::any visitDel_stmt(Python3Parser::Del_stmtContext *ctx) override
    {
        return std::any_cast<std::string>(ctx->exprlist()->accept(this)) + " = nil";
    }

    virtual std::any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override
    {
        std::string result = "if (" + std::any_cast<std::string>(ctx->test(0)->accept(this)) + ") then";
        result += std::any_cast<std::string>(ctx->block(0)->accept(this));

        size_t elif_index = 0;
        auto elif_node = ctx->ELIF(elif_index++);
        while (elif_node != nullptr)
        {
            result += "elseif (" + std::any_cast<std::string>(ctx->test(elif_index)->accept(this)) + ") then";
            result += std::any_cast<std::string>(ctx->block(elif_index)->accept(this));
            elif_node = ctx->ELIF(elif_index++);
        }

        auto else_node = ctx->ELSE();
        if (else_node != nullptr)
        {
            result += "else";
            result += std::any_cast<std::string>(ctx->block(elif_index)->accept(this));
        }
        result += "end\n";
        return result;
    }

    virtual std::any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override
    {
        std::string result = "while (" + std::any_cast<std::string>(ctx->test()->accept(this)) + ") do";
        result += std::any_cast<std::string>(ctx->block(0)->accept(this));

        if (ctx->ELSE())
        {
            result += "if (not (" + std::any_cast<std::string>(ctx->test()->accept(this)) + ")) then";
            result += std::any_cast<std::string>(ctx->block(1)->accept(this));
            result += "end\n";
        }
        result += "end\n";

        return result;
    }

    virtual std::any visitFor_stmt(Python3Parser::For_stmtContext *ctx) override
    {
        auto exprlist_str = std::any_cast<std::string>(ctx->exprlist()->accept(this));
        auto testlist_str = std::any_cast<std::string>(ctx->testlist()->accept(this));
        std::string result = "for _, " + exprlist_str + " in " + testlist_str + " do";
        result += std::any_cast<std::string>(ctx->block(0)->accept(this));
        result += "end\n";
        return result;
    }

    virtual std::any visitAssert_stmt(Python3Parser::Assert_stmtContext *ctx) override
    {
        std::string result = "assert(";
        auto tests = ctx->test();
        int size = static_cast<int>(tests.size()) - 1;
        if (size >= 0)
        {
            for (int i = 0; i < size; i++)
            {
                result += std::any_cast<std::string>(tests[i]->accept(this)) + ", ";
            }
            result += std::any_cast<std::string>(tests[size]->accept(this));
        }
        result += ")";
        return result;
    }

    virtual std::any visitPass_stmt(Python3Parser::Pass_stmtContext *ctx) override
    {
        return std::string("");
    }

    virtual std::any visitLambdef(Python3Parser::LambdefContext *ctx) override
    {
        m_scope = std::make_shared<Scope>(m_scope);
        auto old_func_args_init_stmts = m_func_args_init_stmts;
        std::string result = "function(" + std::any_cast<std::string>(ctx->varargslist()->accept(this)) + ")\n";
        result += m_func_args_init_stmts;
        result += " return " + std::any_cast<std::string>(ctx->test()->accept(this)) + "\nend";
        m_func_args_init_stmts = old_func_args_init_stmts;
        m_scope = m_scope->GetParent();
        return result;
    }

    virtual std::any visitLambdef_nocond(Python3Parser::Lambdef_nocondContext *ctx) override
    {
        m_scope = std::make_shared<Scope>(m_scope);
        auto old_func_args_init_stmts = m_func_args_init_stmts;
        std::string result = "function(" + std::any_cast<std::string>(ctx->varargslist()->accept(this)) + ") \n";
        result += m_func_args_init_stmts;
        result += " return " + std::any_cast<std::string>(ctx->test_nocond()->accept(this)) + "\nend";
        m_func_args_init_stmts = old_func_args_init_stmts;
        m_scope = m_scope->GetParent();
        return result;
    }

    virtual std::any visitVarargslist(Python3Parser::VarargslistContext *ctx) override
    {
        std::string result;
        std::vector<std::string> strs;
        size_t n = ctx->children.size();

        m_func_args_init_stmts = "";
        for (size_t i = 0; i < n; i++)
        {
            auto children = ctx->children[i];
            std::string str = std::any_cast<std::string>(children->accept(this));
            if (str == "=")
            {
                m_func_args_init_stmts += strs[strs.size() - 1] + " = " + strs[strs.size() - 1] + " or " + std::any_cast<std::string>(ctx->children[i + 1]->accept(this)) + "\n";
                i++;
                continue;
            }
            else if (str == "*")
            {
                strs.push_back("...");
                if (i < (n - 1) && ctx->children[i + 1]->getTreeType() == antlr4::tree::ParseTreeType::RULE)
                {
                    m_func_args_init_stmts += "local " + std::any_cast<std::string>(ctx->children[i + 1]->accept(this)) + " = list({...})\n";
                    i++;
                }
            }
            else
            {
                strs.push_back(str);
            }
        }
        return std::accumulate(strs.begin(), strs.end(), std::string(""));
    }

    virtual std::any visitClassdef(Python3Parser::ClassdefContext *ctx) override
    {
        std::string name = std::any_cast<std::string>(ctx->name()->accept(this));
        std::string result = "local " + name + " = class(function(" + name + ")";
        auto old_scope_name = m_scope_name;
        auto old_scope_type = m_scope_type;
        m_scope_name = name;
        m_scope_type = s_scope_type_class;
        auto class_block = ctx->block();
        result += std::any_cast<std::string>(ctx->block()->accept(this));
        m_scope_name = old_scope_name;
        m_scope_type = old_scope_type;
        result += "return " + name + "\n";
        result += "end, {" + GetText(ctx->arglist()) + "}, '" + name + "')\n";
        return result;
    }

    virtual std::any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override
    {
        std::string result;
        std::vector<std::string> strs;
        size_t n = ctx->children.size();

        m_func_args_init_stmts = "";
        for (size_t i = 0; i < n; i++)
        {
            auto children = ctx->children[i];
            std::string str = std::any_cast<std::string>(children->accept(this));
            if (str == "=")
            {
                m_func_args_init_stmts += strs[strs.size() - 1] + " = " + strs[strs.size() - 1] + " or " + std::any_cast<std::string>(ctx->children[i + 1]->accept(this)) + "\n";
                i++;
                continue;
            }
            else if (str == "*")
            {
                strs.push_back("...");
                if (i < (n - 1) && ctx->children[i + 1]->getTreeType() == antlr4::tree::ParseTreeType::RULE)
                {
                    m_func_args_init_stmts += "local " + std::any_cast<std::string>(ctx->children[i + 1]->accept(this)) + " = list({...})\n";
                    i++;
                }
            }
            else
            {
                strs.push_back(str);
            }
        }
        return std::accumulate(strs.begin(), strs.end(), std::string(""));
    }

    virtual std::any visitFuncdef(Python3Parser::FuncdefContext *ctx) override
    {
        m_scope = std::make_shared<Scope>(m_scope);
        std::string name = GetText(ctx->name());

        auto old_func_args_init_stmts = m_func_args_init_stmts;
        auto old_scope_name = m_scope_name;
        auto old_scope_type = m_scope_type;
        auto old_contain_yield = m_contain_yield;

        m_scope_name = name;
        m_scope_type = s_scope_type_function;
        m_contain_yield = false;

        std::string result = "function " + name;
        result += GetText(ctx->parameters()) + "\n";
        result += m_func_args_init_stmts;
        result += GetText(ctx->block());
        result += "end\n";

        if (m_contain_yield)
        {
            result += name + " = meta_generator(" + name + ")\n";
        }

        m_scope_name = old_scope_name;
        m_scope_type = old_scope_type;
        m_contain_yield = old_contain_yield;
        m_func_args_init_stmts = old_func_args_init_stmts;

        m_scope = m_scope->GetParent();
        return result;
    }

    virtual std::any visitDecorated(Python3Parser::DecoratedContext *ctx) override
    {
        std::string result;
        std::string name;
        if (ctx->classdef())
        {
            result = GetText(ctx->classdef());
            name = GetText(ctx->classdef()->name());
        }
        else if (ctx->funcdef())
        {
            result = GetText(ctx->funcdef());
            name = GetText(ctx->funcdef()->name());
        }
        else if (ctx->async_funcdef())
        {
            result = GetText(ctx->async_funcdef());
            name = GetText(ctx->async_funcdef()->funcdef()->name());
        }

        result += name + " = ";
        auto decorators = ctx->decorators()->decorator();
        auto decorators_size = decorators.size();
        for (auto i = 0; i < decorators_size; i++)
        {
            auto decorator = decorators[i];
            auto decorator_name = GetText(decorator->dotted_name());
            auto arglist = "(" + GetText(decorator->arglist()) + ")";
            result += "(" + decorator_name + arglist + ")";
        }
        result += "(" + name + ")\n";
        return result;
    }

    virtual std::any visitImport_name(Python3Parser::Import_nameContext *ctx) override
    {
        auto dotted_as_names = ctx->dotted_as_names()->dotted_as_name();
        auto dotted_as_names_size = dotted_as_names.size();

        std::string result;
        for (auto i = 0; i < dotted_as_names_size; i++)
        {
            auto dotted_as_name = dotted_as_names[i];
            auto name = GetText(dotted_as_name->dotted_name());
            auto as_name = dotted_as_name->AS() ? GetText(dotted_as_name->name()) : name;
            result += "local " + as_name + " = _require(\"" + name + "\")\n";
        }

        return result;
    }

    virtual std::any visitImport_from(Python3Parser::Import_fromContext *ctx) override
    {
        if (ctx->dotted_name() == nullptr || ctx->import_as_names() == 0)
            return std::string("");

        std::string module_name = GetText(ctx->dotted_name());
        auto import_as_names = ctx->import_as_names()->import_as_name();
        auto import_as_names_size = import_as_names.size();
        std::string result;
        for (auto i = 0; i < import_as_names_size; i++)
        {
            auto import_as_name = import_as_names[i];
            auto name = GetText(import_as_name->name(0));
            auto as_name = import_as_name->AS() ? GetText(import_as_name->name(1)) : name;
            result += "local " + as_name + " = _require(\"" + module_name + "\")." + name + "\n";
        }
        return result;
    }

protected:
    std::string GetText(antlr4::ParserRuleContext *ctx)
    {
        return ctx == nullptr ? "" : std::any_cast<std::string>(ctx->accept(this));
    }

    std::string GetScopePrefix()
    {
        if (m_scope_type == s_scope_type_class)
        {
            return m_scope_name + (m_scope_name.empty() ? std::string("") : std::string("."));
        }
        return "";
    }

    antlr4::ParserRuleContext *GetScopeContext(antlr4::tree::ParseTree *ctx)
    {
        while (ctx != nullptr)
        {
            ctx = ctx->parent;
            Python3Parser::FuncdefContext *func_ctx = dynamic_cast<Python3Parser::FuncdefContext *>(ctx);
            if (func_ctx)
            {
                return func_ctx;
            }
            Python3Parser::ClassdefContext *class_ctx = dynamic_cast<Python3Parser::ClassdefContext *>(ctx);
            if (class_ctx)
            {
                return class_ctx;
            }
        }
        return nullptr;
    }

protected:
    std::shared_ptr<Scope> m_scope;
    std::string m_text;
    std::string m_scope_name;
    std::string m_comp_for_stmts;
    std::string m_func_args_init_stmts;
    int m_scope_type;
    bool m_contain_yield;

    static const int s_scope_type_global = 0;
    static const int s_scope_type_local = 1;
    static const int s_scope_type_class = 2;
    static const int s_scope_type_function = 3;
};