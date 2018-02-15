

#pragma once
#include "windows.h"

namespace ParaEngine
{
	//PEGestureInfo is the same as GESTUREINFO
	struct PEGestureInfo
	{
		UINT      cbSize;
		DWORD     dwFlags;
		DWORD     dwID;
		HWND      hwndTarget;
		POINTS    ptsLocation;
		DWORD     dwInstanceID;
		DWORD     dwSequenceID;
		ULONGLONG ullArguments;
		UINT      cbExtraArgs;
	};

	class ITouchInputTranslator
	{
	public:
		bool virtual DecodeGestureMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,PEGestureInfo& outGesInfo) = 0;
	};
}