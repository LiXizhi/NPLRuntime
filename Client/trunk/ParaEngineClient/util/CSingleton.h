//----------------------------------------------------------------------
// Class:	CSingleton
// Authors:	LiXizhi, LiuWeili
// company: paraengine.com
// Date:	2006.3.17
//-----------------------------------------------------------------------
#pragma once
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
}