//----------------------------------------------------------------------
// Class:	CSingleton
// Authors:	LiXizhi, LiuWeili
// company: paraengine.com
// Date:	2006.3.17
//-----------------------------------------------------------------------
#pragma once

#include "IParaEngineApp.h"

namespace ParaEngine
{
	template <class T >
	class CSingleton
	{
	public:
		static T & Instance()
		{
			static T g_instance;
			return g_instance;
		}
	};

	/** 
	* app singleton must inherit from CRefCounted. Object is released when global app exit.
	*/
	template <class T >
	class CAppSingleton
	{
	public:
		static T* GetInstance()
		{
			static T* g_pSington = 0;
			if (g_pSington == 0) {
				g_pSington = new T();
				CGlobals::GetApp()->AddToSingletonReleasePool(g_pSington);
			}
			return g_pSington;
		}
	};
}