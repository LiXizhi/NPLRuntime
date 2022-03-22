//-----------------------------------------------------------------------------
// main.cpp
// Authors: LiXizhi, big
// CreateDate: 2018.3.29
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "AppDelegate.h"
#include "jni/JniHelper.h"
#include <android/log.h>

// fixed link error with luajit2.0
#ifdef NOT_HAVE_SWBUF
extern "C" int __swbuf(int c, FILE *stream)
{
	return putc(c, stream);
}
#endif

#ifdef NOT_HAVE_SRGET
extern "C" int __srget(FILE *stream)
{
	return getc(stream);
}
#endif
