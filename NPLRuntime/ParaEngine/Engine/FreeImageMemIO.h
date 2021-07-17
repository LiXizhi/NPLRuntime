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
	MemIO( BYTE *data, size_t size) : _start(data), _cp(data), _size(size) {
		read_proc  = _ReadProc;
		write_proc = _WriteProc;
		tell_proc  = _TellProc;
		seek_proc  = _SeekProc;
	}

	void Reset() {
		_cp = _start;
	}

	static unsigned DLL_CALLCONV _ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static unsigned DLL_CALLCONV _WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static int DLL_CALLCONV _SeekProc(fi_handle handle, long offset, int origin);
	static long DLL_CALLCONV _TellProc(fi_handle handle);

private:
	BYTE * const _start;
	BYTE *_cp;
	size_t _size;
};
