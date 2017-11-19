// LiXizhi modified from https://github.com/cyrilcode/embed-resource
#include <boost/filesystem.hpp>
#include <iostream>
#include <iterator>
#include <fstream>

using namespace std;
using namespace boost::filesystem;

int main(int argc, char** argv)
{
	if (argc < 3) {
		fprintf(stderr, "USAGE: %s {sym} {rsrc}\n\n"
			"  Creates {sym}.c from the contents of {rsrc}\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	path dst(argv[1]);
	path src(argv[2]);

	string sym = src.filename().string();
	replace(sym.begin(), sym.end(), '.', '_');
	replace(sym.begin(), sym.end(), '-', '_');

	create_directories(dst.parent_path());

	std::fstream ofs(dst.c_str(), std::fstream::out | std::fstream::binary);
	
	ofs << "#include <stdlib.h>" << endl;
	ofs << "namespace ParaEngine" << endl << "{"<<endl; 

	ofs << "extern const unsigned char _resource_" << sym << "[] = {" << endl;

	size_t lineCount = 0;
	int nSize = 0;

	typedef std::istreambuf_iterator<char> buf_iter;
	std::fstream file(src.c_str(), std::fstream::in | std::fstream::binary);
	for (buf_iter i(file), e; i != e; ++i)
	{
		nSize++;
		char c = *i;
		ofs << "0x" << hex << (((unsigned char)c) & 0xff) << ", ";
		if (++lineCount == 16) {
			ofs << endl;
			lineCount = 0;
		}
	}

	ofs << "};" << endl;
	ofs << "extern const size_t _resource_" << sym << "_len = sizeof(_resource_" << sym << "); // total file size " << dec << nSize;
    
	ofs <<endl<< "};";  // End of namespace ParaEngine
	return EXIT_SUCCESS;
}
