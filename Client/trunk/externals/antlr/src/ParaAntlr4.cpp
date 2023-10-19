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

using namespace antlr4;

std::string ParaPythonToLua(std::string python_code)
{
    ANTLRInputStream input(python_code.data(), python_code.size());
    Python3Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    Python3Parser parser(&tokens);
    Python3ToLuaVisitor visitor;
    std::any result = visitor.visitFile_input(parser.file_input());
    return std::any_cast<std::string>(result);
}