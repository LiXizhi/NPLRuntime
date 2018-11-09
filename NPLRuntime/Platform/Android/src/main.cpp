//----------------------------------------------------------------------
// Class:	main native 
// Authors:	LiXizhi@yeah.net
// Company: tatfook
// Date: 2018.3
//-----------------------------------------------------------------------
#include "android_native_app_glue.h"
#include "AppDelegate.h"
#include "jni/JniHelper.h"
#include <android/log.h>


/*
	fixed link error with luajit2.0
*/
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




/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
	LOGI("android_main");
    // Make sure glue isn't stripped.
	app_dummy();

	using namespace ParaEngine;
	AppDelegate& app = AppDelegate::getInstance();

	/*
	if (state->savedState)
	{
		saved_state* savedState = (saved_state*)state->savedState;
		app = savedState->app;
		LOGI("android main state saved");
	}
	else {
		app = new AppDelegate();
		LOGI("android main new state");
	}	
	*/
	app.Run(state);
}

