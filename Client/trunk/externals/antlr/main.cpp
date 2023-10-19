#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "Python3Lexer.h"
#include "Python3Parser.h"
#include "Python3ToLuaVisitor.h"

using namespace antlr4;

void PythonToLua(std::string src_filepath, std::string dst_filepath)
{
    std::ifstream ifs;
    std::ofstream ofs;
    ifs.open(src_filepath);
    ofs.open(dst_filepath);
    if (!ifs || !ofs)
    {
        std::cout << src_filepath << " invalid filepath!!!" << std::endl;
        std::cout << dst_filepath << " invalid filepath!!!" << std::endl;
        return;
    }

    ANTLRInputStream input(ifs);
    Python3Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    Python3Parser parser(&tokens);
    Python3ToLuaVisitor visitor;
    std::any result = visitor.visitFile_input(parser.file_input());
    std::cout << std::any_cast<std::string>(result) << std::endl;
    ofs << std::any_cast<std::string>(result) << std::endl;
}

int main(int, const char **)
{
    // PythonToLua("D:\\workspace\\antlr4\\test.py", "D:\\workspace\\antlr4\\test.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\arithmetic.py", "D:\\workspace\\antlr4\\test\\arithmetic.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\assert.py", "D:\\workspace\\antlr4\\test\\assert.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\assign.py", "D:\\workspace\\antlr4\\test\\assign.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\attribute.py", "D:\\workspace\\antlr4\\test\\attribute.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\augassign.py", "D:\\workspace\\antlr4\\test\\augassign.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\bit.py", "D:\\workspace\\antlr4\\test\\bit.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\bool.py", "D:\\workspace\\antlr4\\test\\bool.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\break.py", "D:\\workspace\\antlr4\\test\\break.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\builtin.py", "D:\\workspace\\antlr4\\test\\builtin.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\class.py", "D:\\workspace\\antlr4\\test\\class.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\compare.py", "D:\\workspace\\antlr4\\test\\compare.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\constant.py", "D:\\workspace\\antlr4\\test\\constant.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\del.py", "D:\\workspace\\antlr4\\test\\del.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\dict-expr.py", "D:\\workspace\\antlr4\\test\\dict-expr.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\dict.py", "D:\\workspace\\antlr4\\test\\dict.lua");
    // PythonToLua("D:\\workspace\\antlr4\\test\\for.py", "D:\\workspace\\antlr4\\test\\for.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\frozenset.py", "D:\\workspace\\antlr4\\test\\frozenset.lua"); 
    PythonToLua("D:\\workspace\\antlr4\\test\\function.py", "D:\\workspace\\antlr4\\test\\function.lua");  
    // PythonToLua("D:\\workspace\\antlr4\\test\\generator.py", "D:\\workspace\\antlr4\\test\\generator.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\if-expr.py", "D:\\workspace\\antlr4\\test\\if-expr.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\if.py", "D:\\workspace\\antlr4\\test\\if.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\import.py", "D:\\workspace\\antlr4\\test\\import.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\lambda.py", "D:\\workspace\\antlr4\\test\\lambda.lua");  
    // PythonToLua("D:\\workspace\\antlr4\\test\\list-expr.py", "D:\\workspace\\antlr4\\test\\list-expr.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\list.py", "D:\\workspace\\antlr4\\test\\list.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\module.py", "D:\\workspace\\antlr4\\test\\module.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\pass.py", "D:\\workspace\\antlr4\\test\\pass.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\return.py", "D:\\workspace\\antlr4\\test\\return.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\set-expr.py", "D:\\workspace\\antlr4\\test\\set-expr.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\set.py", "D:\\workspace\\antlr4\\test\\set.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\string.py", "D:\\workspace\\antlr4\\test\\string.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\subscript.py", "D:\\workspace\\antlr4\\test\\subscript.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\test.py", "D:\\workspace\\antlr4\\test\\test.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\tmp.py", "D:\\workspace\\antlr4\\test\\tmp.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\tuple.py", "D:\\workspace\\antlr4\\test\\tuple.lua"); // todo (a, b, c)
    // PythonToLua("D:\\workspace\\antlr4\\test\\variable.py", "D:\\workspace\\antlr4\\test\\variable.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\while.py", "D:\\workspace\\antlr4\\test\\while.lua"); 
    // PythonToLua("D:\\workspace\\antlr4\\test\\yield.py", "D:\\workspace\\antlr4\\test\\yield.lua"); 
    return 0;
}

// 作用域 默认 local or global  
// 函数参数