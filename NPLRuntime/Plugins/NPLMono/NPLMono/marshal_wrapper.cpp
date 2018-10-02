//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: 
//-----------------------------------------------------------------------------
#include "NPLMono.h"
#include "marshal_wrapper.h"

#ifdef WIN32
#define MONO_API_V2
#endif
#ifdef MONO_API_V2
#include <glib.h>
#endif

mono_string_utf8::mono_string_utf8( MonoString* str )
:m_str(NULL), m_nLength(0)
{
	if(str!=0)
	{
		m_str = mono_string_to_utf8 (str);
	}
}

mono_string_utf8::~mono_string_utf8()
{
	if(m_str!=0)
		g_free(m_str);
}

int mono_string_utf8::size()
{
	if(m_nLength>0)
		return m_nLength;
	else
	{
		if(m_str!=0)
			m_nLength = strlen(m_str);
	}
	return m_nLength;
}

