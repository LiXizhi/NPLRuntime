//-----------------------------------------------------------------------------
// android_compat.cpp - Android libc compatibility stubs for HarmonyOS
// Provides stub implementations for Android-specific symbols
//-----------------------------------------------------------------------------

#include <errno.h>
#include <stdio.h>

// Android's __errno is a function that returns errno address
// HarmonyOS uses standard errno
extern "C" {

int* __errno(void) {
    return &errno;
}

// Android's __sF array for stdin/stdout/stderr
// HarmonyOS uses standard FILE* pointers
FILE __sF[3] = {
    *stdin,
    *stdout, 
    *stderr
};

} // extern "C"
