#ifndef ERRORSTREAM_H
#define ERRORSTREAM_H

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

class ErrorStream : public NxUserOutputStream
	{
	public:
	void reportError(NxErrorCode e, const char* message, const char* file, int line)
		{
		OUTPUT_LOG("%s (%d) :", file, line);
		switch (e)
			{
			case NXE_INVALID_PARAMETER:
				OUTPUT_LOG( "invalid parameter");
				break;
			case NXE_INVALID_OPERATION:
				OUTPUT_LOG( "invalid operation");
				break;
			case NXE_OUT_OF_MEMORY:
				OUTPUT_LOG( "out of memory");
				break;
			case NXE_DB_INFO:
				OUTPUT_LOG( "info");
				break;
			case NXE_DB_WARNING:
				OUTPUT_LOG( "warning");
				break;
			default:
				OUTPUT_LOG("unknown error");
			}

		OUTPUT_LOG(" : %s\n", message);
		}

	NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
		{
		OUTPUT_LOG("access violation : %s (%s line %d)\n", message, file, line);
#ifdef WIN32
		switch (MessageBox(0, message, "AssertViolation, see console for details.", MB_ABORTRETRYIGNORE))
			{
			case IDRETRY:
				return NX_AR_CONTINUE;
			case IDIGNORE:
				return NX_AR_IGNORE;
			case IDABORT:
			default:
				return NX_AR_BREAKPOINT;
			}
#elif LINUX
		assert(0);
#elif _XBOX
		return NX_AR_BREAKPOINT;
#endif
		}

	void print(const char* message)
		{
		OUTPUT_LOG("%s", message);
		}
	};



#endif
