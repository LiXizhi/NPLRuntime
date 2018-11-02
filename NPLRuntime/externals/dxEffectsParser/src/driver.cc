// $Id$
/** \file driver.cc Implementation of the example::Driver class. */

#include <fstream>
#include <sstream>

#include "driver.h"
#include "scanner.h"

namespace DxEffectsParser {

Driver::Driver(class DxEffectsTree& _tree)
    : trace_scanning(false),
      trace_parsing(false),
	  tree(_tree)
{
}

bool Driver::parse_stream(std::istream& in, const std::string& sname)
{

	std::string code = InsertCodeBlockTag(in);
	std::istringstream iss(code);


    streamname = sname;
    Scanner scanner(&iss);
    scanner.set_debug(trace_scanning);
    this->lexer = &scanner;

    Parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}

bool Driver::parse_file(const std::string &filename)
{
    std::ifstream in(filename.c_str());
    if (!in.good()) return false;
    return parse_stream(in, filename);
}

bool Driver::parse_string(const std::string &input, const std::string& sname)
{

    std::istringstream iss(input);
    return parse_stream(iss, sname);
}

void Driver::error(const class location& l,
		   const std::string& m)
{
    std::cerr << l << ": " << m << std::endl;
}

void Driver::error(const std::string& m)
{
    std::cerr << m << std::endl;
}

std::string Driver::InsertCodeBlockTag(std::istream& in)
{
	// 自动插入代码块标识
	

	std::stringstream fx_code_block;
	std::stringstream fx_technique_block;
	bool find_code_begin = false;
	bool find_code_end = false;
	bool find_technique_block = false;
	std::string line;
	while (std::getline(in, line))
	{
		if (!find_code_begin && line == "//HLSL_CODE_BEGIN") find_code_begin = true;
		if (!find_code_end && line == "//HLSL_CODE_END") find_code_end = true;
		if (!find_technique_block) {
			size_t technique_begin = line.find("technique");
			if (technique_begin != std::string::npos) {
				size_t i = 0;
				for (; i < technique_begin; i++) {
					if (!(line[i] == ' ' || line[i] == '\t')) {
						break;
					}
				}
				if (i == technique_begin) {
					find_technique_block = true;
				}

			}
		}

		if (find_technique_block) {
			fx_technique_block << line << std::endl;
		}
		else {
			fx_code_block << line << std::endl;
		}
	}

	std::stringstream fx_code;

	if (!find_code_begin) {
		fx_code << "//HLSL_CODE_BEGIN" << std::endl;
	}
	fx_code << fx_code_block.str();
	if (!find_code_end) {
		fx_code << "//HLSL_CODE_END" << std::endl;
	}
	fx_code << fx_technique_block.str();
	return fx_code.str();
}

} // namespace example
