#pragma once
#include <map>
#include <string>
#include "ParaEngineAppBase.h"


namespace ParaEngine
{

	class ParacraftAndroidApp
	{
	public:
		ParacraftAndroidApp();
	
		~ParacraftAndroidApp();
	private:
		std::map<std::string,std::string> m_map;
	};

}