//-----------------------------------------------------------------------------
// Class: Python to NPL(lua) translator
// Authors:	wuxiangan
// Emails:
// Date: 2023.10.19
// Desc:
//----------------------------------------------------------------------------
#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "Python3Lexer.h"
#include "Python3Parser.h"
#include "Python3ToLuaVisitor.h"
#include "CPP14ToLuaVisitor.h"

using namespace antlr4;

std::string ParaPythonToLua(std::string python_code)
{
    try
    {
        ANTLRInputStream input(python_code.data(), python_code.size());
        Python3Lexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        Python3Parser parser(&tokens);
        Python3ToLuaVisitor visitor;
        std::any result = visitor.visitFile_input(parser.file_input());
        return std::any_cast<std::string>(result);
    }
    catch (const std::exception &e)
    {
        std::cout << "ParaPythonToLua: " << e.what() << std::endl;
        return "";
    }
}

std::string ParaCppToLua(std::string cpp_code)
{
    try
    {
        ANTLRInputStream input(cpp_code.data(), cpp_code.size());
        CPP14Lexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        CPP14Parser parser(&tokens);
        CPP14ToLuaVisitor visitor;
        std::any result = visitor.visitTranslationUnit(parser.translationUnit());
        return std::any_cast<std::string>(result);
    }
    catch (std::exception &e)
    {
        std::cout << "ParaCppToLua: " << e.what() << std::endl;
        return "";
    }
}