#pragma once
#include <FreeImage.h>

//////////////////////////////////////////////////////////////////////////
//
// Memory File interface for FreeImage lib
//
//////////////////////////////////////////////////////////////////////////

class MemIO : public FreeImageIO 
{
public :
	MemIO( BYTE *data ) : _start(data), _cp(data) {
		read_proc  = _ReadProc;
		write_proc = _WriteProc;
		tell_proc  = _TellProc;
		seek_proc  = _SeekProc;
	}

	void Reset() {
		_cp = _start;
	}

	static unsigned STDCALL _ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static unsigned STDCALL _WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static int STDCALL _SeekProc(fi_handle handle, long offset, int origin);
	static long STDCALL _TellProc(fi_handle handle);

private:
	BYTE * const _start;
	BYTE *_cp;
};
