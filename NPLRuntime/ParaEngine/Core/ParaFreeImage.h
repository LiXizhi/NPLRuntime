#pragma once

// FreeImage's C API uses a 32-bit BOOL. Keep its declarations independent
// of the mobile engine's BOOL.
#if defined(__ANDROID__) || defined(PLATFORM_IPHONE)
#define BOOL FreeImageBOOL
#endif
#include <FreeImage.h>
#if defined(__ANDROID__) || defined(PLATFORM_IPHONE)
#undef BOOL
#endif
