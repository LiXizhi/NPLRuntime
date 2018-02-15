//-----------------------------------------------------------------------------
// Class:	CMovieClipBase
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "MovieClip.h"

using namespace ParaEngine;

///////////////////////////////////////////////////////////////////////////
//
// movie clip base class
//
//////////////////////////////////////////////////////////////////////////

CMovieClipBase::CMovieClipBase(void)
:m_nFrameCount(0),m_nClipWidth(0),m_nClipHeight(0)
{
}

CMovieClipBase::~CMovieClipBase(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
// movie clip screen capture
//
//////////////////////////////////////////////////////////////////////////
CMovieClipScreenCapture::CMovieClipScreenCapture()
{

}

CMovieClipScreenCapture::~CMovieClipScreenCapture()
{

}