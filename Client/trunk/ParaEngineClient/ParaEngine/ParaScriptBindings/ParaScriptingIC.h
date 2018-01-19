//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.3
// Description:	API for configuration
//-----------------------------------------------------------------------------
#pragma once

#include <string>
namespace ParaScripting
{
	using namespace std;
	/**
	* @ingroup Config
	*/
	class PE_CORE_DECL ParaConfig
	{
	public:
		static bool SetIntValue(const char* szName,int value);
		static bool SetDoubleValue(const char* szName,double value);
		static bool SetTextValue(const char* szName,const char* value);
		static bool GetIntValue(const char* szName,int *value) ;
		static bool GetDoubleValue(const char* szName,double *value);
		static string GetTextValue(const char* szName);
		static bool SetIntValueEx(const char* szName,int value,int index);
		static bool SetDoubleValueEx(const char* szName,double value,int index);
		static bool SetTextValueEx(const char* szName,const char* value,int index);
		static bool GetIntValueEx(const char* szName,int *value,int index) ;
		static bool GetDoubleValueEx(const char* szName,double *value,int index);
		static string GetTextValueEx(const char* szName,int index);
		static bool AppendIntValue(const char* szName,int value);
		static bool AppendDoubleValue(const char* szName,double value);
		static bool AppendTextValue(const char* szName,const char* value);
	};
}