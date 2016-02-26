#pragma once
#include "StringHelper.h"

namespace ParaEngine
{
	/** this is usually used for translating from simplified Chinese to traditional Chinese character. 
	* TODO: move this to a separate source file
	*/
	class CSimpleTranslater
	{
	public:
		typedef std::map<WCHAR, WCHAR>  CharacterWideCharMap_type;
		CSimpleTranslater(const char* sFileName)
			:m_nMinRange(0xffff), m_nMaxRange(0)
		{
			CParaFile file;
			file.OpenAssetFile(sFileName);

			if(!file.isEof())
			{
				char sLineText[256];
				while(file.GetNextLine(sLineText, 256)>0)
				{
					const WCHAR* swText = ParaEngine::StringHelper::MultiByteToWideChar(sLineText, DEFAULT_GUI_ENCODING);
					if(swText)
					{
						WCHAR wcFrom = *swText;
						if(wcFrom != L'\0')
						{
							WCHAR wcTo = *(++swText);
							bool bBreak = false;
							while(wcTo!=L'\0' && !bBreak)
							{
								if(wcTo!=L' '  && wcTo!=L'\t')
								{
									bBreak = true;
									m_char_map[wcFrom] = wcTo;
									if(wcFrom<m_nMinRange)
										m_nMinRange = wcFrom;
									if(wcFrom>m_nMaxRange)
										m_nMaxRange = wcFrom;
								}
								wcTo = *(++swText);
							}
						}
					}
				}
			}
			else
			{
				OUTPUT_LOG("error: unable to open locale file %s\n", sFileName);
			}
		}

		const WCHAR* TranslateW(const WCHAR* sText)
		{
			if(sText == NULL)
				return NULL;
			static vector<WCHAR> sOutput;
			int nSize = (int)(wcslen(sText));
			if(nSize >= ((int)sOutput.size()))
			{
				sOutput.resize(nSize+2);
				sOutput[nSize] = L'\0';
			}
			for(int i=0; i<nSize; ++i)
			{
				WCHAR c = sText[i];
				if(m_nMinRange<=c && c<=m_nMaxRange)
				{
					CharacterWideCharMap_type::iterator iter = m_char_map.find(c);
					if(iter!=m_char_map.end())
					{
						sOutput[i] = iter->second;
					}
					else
					{
						sOutput[i] = sText[i];
					}
				}
				else
				{
					sOutput[i] = sText[i];
				}
			}
			sOutput[nSize] = L'\0';
			return &(sOutput[0]);
		}
	private:
		CharacterWideCharMap_type  m_char_map;
		WCHAR m_nMinRange;
		WCHAR m_nMaxRange;
	};
}


