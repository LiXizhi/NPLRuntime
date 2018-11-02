// $Id$

#include <iostream>
#include <fstream>

#include "driver.h"
#include "dxeffects.h"


#ifdef WIN32  
#include <direct.h>  
#include <io.h>  
#else
#include <unistd.h>  
#include <dirent.h>  
#endif  


std::vector<std::string> getFiles(std::string cate_dir)
{
	std::vector<std::string> files;//存放文件名  

#ifdef WIN32  
	_finddata_t file;
	long lf;
	//输入文件夹路径  
	auto finalPath = cate_dir + "*";
	if ((lf = _findfirst(finalPath.c_str(), &file)) == -1) {
		std::cout << finalPath << " not found!!!" << std::endl;
	}
	else {
		while (_findnext(lf, &file) == 0) {
			//输出文件名  
			//cout<<file.name<<endl;  
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
				continue;
			files.push_back(file.name);
		}
	}
	_findclose(lf);

#else
	DIR *dir;
	struct dirent *ptr;
	char base[1000];

	if ((dir = opendir(cate_dir.c_str())) == NULL)
	{
		perror("Open dir error...");
		exit(1);
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir  
			continue;
		else if (ptr->d_type == 8)    ///file  
									  //printf("d_name:%s/%s\n",basePath,ptr->d_name);  
			files.push_back(ptr->d_name);
		else if (ptr->d_type == 10)    ///link file  
									   //printf("d_name:%s/%s\n",basePath,ptr->d_name);  
			continue;
		else if (ptr->d_type == 4)    ///dir  
		{
			files.push_back(ptr->d_name);
			/*
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_nSame);
			readFileList(base);
			*/
		}
	}
	closedir(dir);

#endif  
	return files;
}




bool test_file(std::string filePath, bool p, bool s, bool v)
{
	DxEffectsTree tree;
	DxEffectsParser::Driver driver(tree);
	driver.trace_parsing = p;
	driver.trace_scanning = s;

	std::fstream infile(filePath);
	if (!infile.good())
	{
		std::cerr << "Could not open file: " << filePath << std::endl;
		return false;
	}

	bool result = driver.parse_stream(infile,filePath);
	if (result && v) {
		std::cout << "CodeBlock:" << std::endl;
		std::cout << tree.getCodeBlock() << std::endl;
		std::cout << "Techniques:" << std::endl;
		auto techiques = tree.getTechiques();
		for (unsigned int ei = 0; ei < techiques.size(); ++ei)
		{
			std::cout << "Technique " << techiques[ei]->getName() << std::endl;
			auto passes = techiques[ei]->getPasses();
			for (unsigned int pi = 0; pi < passes.size(); pi++)
			{
				std::cout << "\tPass " << (passes[pi]->getName()) << std::endl;
				auto states = passes[pi]->getStateAssignments();
				for (unsigned int si = 0; si < states.size(); si++)
				{
					auto state = states[si];
					int nameIndex = state->getNameIndex();
					if (nameIndex != -1) {
						std::cout << "\t\tStateAssignment " << state->getName() << " Index:" << nameIndex << std::endl;
					}
					else {
						std::cout << "\t\tStateAssignment " << state->getName() << std::endl;
					}
					// output value
					const StateAssignmentValue* value = state->getValue();
					const std::string valueStr = value->toString();
					std::cout << "\t\tValue:" << valueStr << std::endl;

				}
			}
		}
	}

	return result;
}

bool is_dir(std::string path)
{
	char end_char = path[path.length() - 1];
	if (end_char == '/' || end_char =='\\') {
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{
	bool p = false, v = false, s = false;
    for(int ai = 1; ai < argc; ++ai)
    {
		if (argv[ai] == std::string("-p")) {
			p = true;
		}
		else if (argv[ai] == std::string("-s")) {
			s = true;
		}
		else if (argv[ai] == std::string("-v")) {
			v = true;
		}
		else
		{
			std::vector<std::string> files;
			std::string path = argv[ai];
			if (is_dir(path)) {
				auto fileNames = getFiles(path);
				for (auto fileName : fileNames) {
					files.push_back(path + fileName);
				}
			}
			else {
				files.push_back(path);
			}
			int totalFiled = 0;
			for (auto filePath : files)
			{

				bool result = test_file(filePath, p, s, v);
				if (result) {
					std::cout << "Parse file:" << filePath << " successed!\n";
				}
				else {
					std::cout << "Parse file:" << filePath << " failed!\n";
					totalFiled++;
				}
			}

			std::cout << "Total " << files.size() << " Failed:" << totalFiled << std::endl;

		}
    }
	std::system("pause");
}
