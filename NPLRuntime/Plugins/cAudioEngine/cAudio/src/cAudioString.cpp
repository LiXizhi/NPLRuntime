#include "cAudioString.h"
#include <memory>

namespace cAudio {
	const WCHAR* cMultiByteToWideChar(const char* name, unsigned int nCodePage, size_t* outLen)
	{
		static std::shared_ptr< std::vector<WCHAR> > wsName_;
		if (!wsName_.get()) {
			// first time called by this thread
			// construct test element to be used in all subsequent calls from this thread
			wsName_.reset(new std::vector<WCHAR>());
		}
		std::vector<WCHAR>& wsName = *wsName_;

#ifdef WIN32
		int nLength = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, NULL, 0);
		if (nLength > 0)
		{
			if (((int)wsName.size()) < nLength)
				wsName.resize(nLength);
			int nResult = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, &(wsName[0]), nLength);
			wsName[nLength - 1] = L'\0';

			if (outLen)
				*outLen = nLength;
		}
		else
		{
			if (((int)wsName.size()) < 1)
				wsName.resize(1);
			wsName[0] = L'\0';

			if (outLen)
				*outLen = 0;
		}

		return &(wsName[0]);
#else
		size_t nLength = mbstowcs(0, name, 0);
		if (nLength != (size_t)(-1))
		{
			if (wsName.size() <= nLength)
				wsName.resize(nLength + 1);
			size_t nResult = mbstowcs(&(wsName[0]), name, nLength + 1);

			if (outLen)
				*outLen = nResult;
		}
		else
		{
			if (((int)wsName.size()) < 1)
				wsName.resize(1);
			wsName[0] = L'\0';

			if (outLen)
				*outLen = 0;
		}
		return &(wsName[0]);
#endif

	}

	const char* cWideCharToMultiByte(const WCHAR* name, unsigned int nCodePage, size_t* outLen)
	{
		static std::shared_ptr< std::vector<char> > cName_;
		if (!cName_.get()) {
			// first time called by this thread
			// construct test element to be used in all subsequent calls from this thread
			cName_.reset(new std::vector<char>());
		}
		std::vector<char>& cName = *cName_;

#ifdef WIN32
		int nLength = ::WideCharToMultiByte(nCodePage, 0, name, -1, NULL, NULL, NULL, NULL);
		if (nLength > 0)
		{
			if (((int)cName.size()) < nLength)
				cName.resize(nLength);
			int nResult = ::WideCharToMultiByte(nCodePage, 0, name, -1, &(cName[0]), nLength, NULL, NULL);
			cName[nLength - 1] = '\0';

			if (outLen)
				*outLen = nLength;
		}
		else
		{
			if (((int)cName.size()) < 1)
				cName.resize(1);
			cName[0] = '\0';

			if (outLen)
				*outLen = 0;
		}
		return &(cName[0]);
#else
		size_t nLength = wcstombs(0, name, 0);
		if (nLength != (size_t)(-1))
		{
			if (cName.size() < nLength)
				cName.resize(nLength);
			wcstombs(&(cName[0]), name, nLength + 1);

			if (outLen)
				*outLen = nLength;
		}
		else
		{
			if (((int)cName.size()) < 1)
				cName.resize(1);
			cName[0] = '\0';

			if (outLen)
				*outLen = 0;
		}
		return &(cName[0]);
#endif
	}
}


