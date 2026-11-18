//-----------------------------------------------------------------------------
// Class: FontRendererOpenGL
// Authors:	LiXizhi
// Emails:
// Date: 2014.8.28
// Desc: rendering a given font from TTF format. It emulates the d3dxfont class.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include <cmath>
#include <initializer_list>
#include "IO/FileUtils.h"

#ifndef USE_NULL_RENDERER
#include "OpenGLWrapper/GLFontAtlas.h"
#include "OpenGLWrapper/GLFontAtlasCache.h"
#include "GUIBase.h"
#include "TextureEntity.h"
#include "StringHelper.h"
#include "EffectManager.h"

//#ifdef PARAENGINE_MOBILE
//#include "2d/CCLabelTextFormatter.h"
//#endif

#include "SpriteRendererOpenGL.h"
#include "FontRendererOpenGL.h"

using namespace ParaEngine;

ParaEngine::CFontRendererOpenGL::CFontRendererOpenGL()
	:m_nFontSize(14), m_nRealFontSize(14), m_fFontScaling(1.f), m_bUseCustomShader(false), m_pScaledFontAtlas(nullptr), m_nScaledFontSize(0)
{
}

CFontRendererOpenGL* ParaEngine::CFontRendererOpenGL::create(const std::string& sFontName, int nFontSize, bool bBold)
{
	CFontRendererOpenGL* pFontRenderer = new CFontRendererOpenGL();
	auto pickExistingFile = [](std::initializer_list<std::string> candidates) -> std::string {
		for (const auto& candidate : candidates)
		{
			if (!candidate.empty() && CFileUtils::FileExist(candidate.c_str()))
				return candidate;
		}
		return std::string();
	};
	auto iequalsFont = [](const std::string& a, const char* b) -> bool {
		if (!b)
			return false;
		size_t i = 0;
		for (; i < a.size() && b[i] != '\0'; ++i)
		{
			unsigned char ca = static_cast<unsigned char>(a[i]);
			unsigned char cb = static_cast<unsigned char>(b[i]);
			if (ca >= 'A' && ca <= 'Z')
				ca = static_cast<unsigned char>(ca - 'A' + 'a');
			if (cb >= 'A' && cb <= 'Z')
				cb = static_cast<unsigned char>(cb - 'A' + 'a');
			if (ca != cb)
				return false;
		}
		return i == a.size() && b[i] == '\0';
	};

	std::string fontFile;
	std::string fallbackFontFile;
#ifdef WIN32
	char szWinPath[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(szWinPath, MAX_PATH);
	std::string winDir = szWinPath;
	fallbackFontFile = bBold
		? pickExistingFile({
			winDir + "/Fonts/msyhbd.ttc",
			winDir + "/Fonts/msyhbd.ttf",
			winDir + "/Fonts/msyh.ttc",
			winDir + "/Fonts/msyh.ttf",
			winDir + "/Fonts/simhei.ttf",
			winDir + "/Fonts/simsun.ttc",
			"fonts/DroidSansFallback.ttf"
		})
		: pickExistingFile({
			winDir + "/Fonts/msyh.ttc",
			winDir + "/Fonts/msyh.ttf",
			winDir + "/Fonts/simhei.ttf",
			winDir + "/Fonts/simsun.ttc",
			"fonts/DroidSansFallback.ttf"
		});
#else
	fallbackFontFile = pickExistingFile({
		"fonts/DroidSansFallback.ttf",
		"/System/Library/Fonts/PingFang.ttc",
		"/System/Library/Fonts/STHeiti Light.ttc",
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
		"/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"
	});
	auto pickTahomaOrSimilarSans = [&]() -> std::string {
		return bBold
			? pickExistingFile({
				"fonts/tahomabd.ttf",
				"fonts/TahomaBold.ttf",
				"fonts/tahoma.ttf",
				"fonts/Tahoma.ttf",
				"/System/Library/Fonts/Supplemental/Tahoma Bold.ttf",
				"/System/Library/Fonts/Supplemental/Tahoma.ttf",
				"fonts/OpenSans-Semibold.ttf",
				"fonts/OpenSans-Bold.ttf",
				"fonts/OpenSans-Regular.ttf",
				"fonts/verdanab.ttf",
				"fonts/verdana.ttf",
				"fonts/segoeuib.ttf",
				"fonts/segoeui.ttf",
				"/System/Library/Fonts/Supplemental/Verdana Bold.ttf",
				"/System/Library/Fonts/Supplemental/Verdana.ttf",
				"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
				"/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
				"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
				"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
			})
			: pickExistingFile({
				"fonts/tahoma.ttf",
				"fonts/Tahoma.ttf",
				"/System/Library/Fonts/Supplemental/Tahoma.ttf",
				"fonts/OpenSans-Regular.ttf",
				"fonts/verdana.ttf",
				"fonts/segoeui.ttf",
				"/System/Library/Fonts/Supplemental/Verdana.ttf",
				"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
				"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
			});
	};
#endif

	if (iequalsFont(sFontName, "System"))
	{
#ifdef WIN32
		fontFile = !fallbackFontFile.empty()
			? fallbackFontFile
			: pickExistingFile({
				winDir + (bBold ? "/Fonts/segoeuib.ttf" : "/Fonts/segoeui.ttf"),
				winDir + "/Fonts/segoeui.ttf",
				winDir + (bBold ? "/Fonts/arialbd.ttf" : "/Fonts/arial.ttf"),
				winDir + "/Fonts/arial.ttf",
				"fonts/DroidSansFallback.ttf"
			});
#else
		fontFile = pickTahomaOrSimilarSans();
		if (fontFile.empty())
			fontFile = !fallbackFontFile.empty() ? fallbackFontFile : "fonts/DroidSansFallback.ttf";
#endif
	}
	else
	{
		if (CFileUtils::FileExist(sFontName.c_str()))
		{
			fontFile = sFontName;
		}
		else
		{
			fontFile = pickExistingFile({
				"fonts/" + sFontName + ".ttf",
				"fonts/" + sFontName + ".ttc"
			});
		}

#ifdef WIN32
		if (fontFile.empty())
		{
			if (_stricmp(sFontName.c_str(), "Tahoma") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/tahomabd.ttf" : "/Fonts/tahoma.ttf"),
					winDir + "/Fonts/tahoma.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Arial") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/arialbd.ttf" : "/Fonts/arial.ttf"),
					winDir + "/Fonts/arial.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Segoe UI") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/segoeuib.ttf" : "/Fonts/segoeui.ttf"),
					winDir + "/Fonts/segoeui.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Consolas") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/consolab.ttf" : "/Fonts/consola.ttf"),
					winDir + "/Fonts/consola.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Courier New") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/courbd.ttf" : "/Fonts/cour.ttf"),
					winDir + "/Fonts/cour.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Verdana") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/verdanab.ttf" : "/Fonts/verdana.ttf"),
					winDir + "/Fonts/verdana.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Times New Roman") == 0)
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/timesbd.ttf" : "/Fonts/times.ttf"),
					winDir + "/Fonts/times.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "Microsoft YaHei") == 0 ||
				_stricmp(sFontName.c_str(), "Microsoft YaHei UI") == 0 ||
				sFontName == "\xE5\xBE\xAE\xE8\xBD\xAF\xE9\x9B\x85\xE9\xBB\x91")
			{
				fontFile = pickExistingFile({
					winDir + (bBold ? "/Fonts/msyhbd.ttc" : "/Fonts/msyh.ttc"),
					winDir + (bBold ? "/Fonts/msyhbd.ttf" : "/Fonts/msyh.ttf"),
					winDir + "/Fonts/msyh.ttc",
					winDir + "/Fonts/msyh.ttf"
				});
			}
			else if (_stricmp(sFontName.c_str(), "SimHei") == 0 ||
				sFontName == "\xE9\xBB\x91\xE4\xBD\x93")
			{
				fontFile = pickExistingFile({ winDir + "/Fonts/simhei.ttf" });
			}
			else if (_stricmp(sFontName.c_str(), "SimSun") == 0 ||
				_stricmp(sFontName.c_str(), "NSimSun") == 0 ||
				sFontName == "\xE5\xAE\x8B\xE4\xBD\x93")
			{
				fontFile = pickExistingFile({
					winDir + "/Fonts/simsun.ttc",
					winDir + "/Fonts/simsun.ttf"
				});
			}
		}

		if (fontFile.empty())
		{
			fontFile = pickExistingFile({
				winDir + "/Fonts/" + sFontName + ".ttf",
				winDir + "/Fonts/" + sFontName + ".ttc"
			});
		}

		if (fontFile.empty())
		{
			HKEY hKey = NULL;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				std::string boldPrefix = sFontName + " Bold";
				std::string regularMatch;
				std::string boldMatch;
				char szValName[512];
				char szValData[MAX_PATH];
				DWORD dwIndex = 0;
				while (true)
				{
					DWORD dwNameLen = sizeof(szValName);
					DWORD dwDataLen = sizeof(szValData);
					DWORD dwType = 0;
					if (RegEnumValueA(hKey, dwIndex++, szValName, &dwNameLen, NULL, &dwType, (LPBYTE)szValData, &dwDataLen) != ERROR_SUCCESS)
						break;
					if (dwType != REG_SZ || dwDataLen == 0)
						continue;

					if (_strnicmp(szValName, sFontName.c_str(), sFontName.length()) == 0)
					{
						char nextCh = szValName[sFontName.length()];
						if (nextCh == '\0' || nextCh == ' ' || nextCh == '(' || nextCh == '&')
						{
							std::string candidate = (strchr(szValData, ':') != nullptr)
								? std::string(szValData)
								: (winDir + "/Fonts/" + szValData);
							if (CFileUtils::FileExist(candidate.c_str()))
							{
								if (_strnicmp(szValName, boldPrefix.c_str(), boldPrefix.length()) == 0)
								{
									if (boldMatch.empty())
										boldMatch = candidate;
								}
								else if (regularMatch.empty())
								{
									regularMatch = candidate;
								}
							}
						}
					}
				}
				RegCloseKey(hKey);
				fontFile = (bBold && !boldMatch.empty()) ? boldMatch : (!regularMatch.empty() ? regularMatch : boldMatch);
			}
		}
#else
		if (fontFile.empty())
		{
			if (iequalsFont(sFontName, "Tahoma") ||
				iequalsFont(sFontName, "Verdana") ||
				iequalsFont(sFontName, "Segoe UI") ||
				iequalsFont(sFontName, "Arial") ||
				iequalsFont(sFontName, "Helvetica") ||
				iequalsFont(sFontName, "Open Sans"))
			{
				fontFile = pickTahomaOrSimilarSans();
			}
			else if (iequalsFont(sFontName, "Consolas") ||
				iequalsFont(sFontName, "Courier New"))
			{
				fontFile = pickExistingFile({
					bBold ? "fonts/consolab.ttf" : "fonts/consola.ttf",
					"fonts/consola.ttf",
					"/System/Library/Fonts/Menlo.ttc",
					bBold ? "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf" : "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
					"/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
				});
				if (fontFile.empty())
					fontFile = pickTahomaOrSimilarSans();
			}
			else if (iequalsFont(sFontName, "Times New Roman"))
			{
				fontFile = pickExistingFile({
					bBold ? "fonts/timesbd.ttf" : "fonts/times.ttf",
					"fonts/times.ttf",
					"/System/Library/Fonts/Times.ttc",
					bBold ? "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf" : "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
					"/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf"
				});
				if (fontFile.empty())
					fontFile = pickTahomaOrSimilarSans();
			}
			else if (iequalsFont(sFontName, "Microsoft YaHei") ||
				iequalsFont(sFontName, "Microsoft YaHei UI") ||
				sFontName == "\xE5\xBE\xAE\xE8\xBD\xAF\xE9\x9B\x85\xE9\xBB\x91" ||
				iequalsFont(sFontName, "SimHei") ||
				sFontName == "\xE9\xBB\x91\xE4\xBD\x93" ||
				iequalsFont(sFontName, "SimSun") ||
				iequalsFont(sFontName, "NSimSun") ||
				sFontName == "\xE5\xAE\x8B\xE4\xBD\x93")
			{
				fontFile = pickExistingFile({
					bBold ? "fonts/msyhbd.ttc" : "fonts/msyh.ttc",
					"fonts/msyh.ttc",
					fallbackFontFile
				});
			}

			if (fontFile.empty())
			{
				fontFile = pickTahomaOrSimilarSans();
				if (fontFile.empty() && !fallbackFontFile.empty())
					fontFile = fallbackFontFile;
			}
		}
#endif
		if (fontFile.empty())
		{
			fontFile = sFontName;
		}
	}

	// force size to be <= 50. font-rendering crashes on android with big font.
	pFontRenderer->m_nFontSize = nFontSize;
#if !defined(PARAENGINE_MOBILE)
	if (pFontRenderer->m_nFontSize <= 0)
	{
		pFontRenderer->m_nRealFontSize = 12;
	}
	else if (pFontRenderer->m_nFontSize <= 50)
	{
		pFontRenderer->m_nRealFontSize = pFontRenderer->m_nFontSize;
	}
	else
	{
		pFontRenderer->m_nRealFontSize = 50;
	}
#else
	// on mobile platform, we only support 28 and 50 font size, so that smaller fonts are still very clear.
	if (pFontRenderer->m_nFontSize <= 25)
	{
		pFontRenderer->m_nRealFontSize = 28;
	}
	else
	{
		pFontRenderer->m_nRealFontSize = 50;
	}
#endif

	pFontRenderer->m_fFontScaling = float(nFontSize) / pFontRenderer->m_nRealFontSize;

	if (pFontRenderer && CFileUtils::FileExist(fontFile.c_str()))
	{
		TTFConfig ttfConfig(fontFile.c_str(), pFontRenderer->m_nRealFontSize, GlyphCollection::DYNAMIC);
		ttfConfig.bold = bBold;
#ifdef WIN32
		if (!fallbackFontFile.empty() && _stricmp(fallbackFontFile.c_str(), fontFile.c_str()) != 0)
#else
		if (!fallbackFontFile.empty() && fallbackFontFile != fontFile)
#endif
		{
			ttfConfig.fallbackFontFilePath = fallbackFontFile;
		}
		if (pFontRenderer->setTTFConfig(ttfConfig))
		{
			pFontRenderer->setFontScale(pFontRenderer->m_fFontScaling);
			if (pFontRenderer->getFontAtlas())
			{
				pFontRenderer->getFontAtlas()->setAntiAliasTexParameters();
			}
			pFontRenderer->AddToAutoReleasePool();
			return pFontRenderer;
		}
	}
	SAFE_DELETE(pFontRenderer);
	OUTPUT_LOG("error: font file: %s does not exist or invalid\n", fontFile.c_str());
	if (sFontName != "System" && sFontName != "system")
	{
		OUTPUT_LOG("non-exist font %s is replaced with system font\n", sFontName.c_str());
		return CFontRendererOpenGL::create("System", nFontSize, bBold);
	}
	return NULL;
}

ParaEngine::CFontRendererOpenGL::~CFontRendererOpenGL()
{
	if (m_pScaledFontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(m_pScaledFontAtlas);
		m_pScaledFontAtlas = nullptr;
		m_nScaledFontSize = 0;
	}
}


void ParaEngine::CFontRendererOpenGL::SetUTF16Text(const char16_t* strText)
{
	_currentUTF16String = strText;
}



bool ParaEngine::CFontRendererOpenGL::DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, RECT& rect, DWORD dwTextFormat, DWORD textColor)
{
	if (!strText || strText[0] == '\0')
		return false;
	SetUTF16Text(strText);
	// remove all '\r', since cocos font rendering only support '\n', not "\r\n"
	_currentUTF16String.erase(std::remove(_currentUTF16String.begin(), _currentUTF16String.end(), '\r'), _currentUTF16String.end());

	if (!pSprite) return false;
	int nWidth = CGUIBase::RectWidth(rect);
	int nHeight = CGUIBase::RectHeight(rect);
	int nMaxHeight = nHeight;
	TextHAlignment hAlignment_ =
		(dwTextFormat & DT_CENTER) ? TextHAlignment::CENTER :
		(dwTextFormat & DT_RIGHT) ? TextHAlignment::RIGHT :
		TextHAlignment::LEFT;

	TextVAlignment vAlignment_ =
		(dwTextFormat & DT_VCENTER) ? TextVAlignment::CENTER :
		(dwTextFormat & DT_BOTTOM) ? TextVAlignment::BOTTOM :
		TextVAlignment::TOP;

	// TODO: DT_NOCLIP | DT_SINGLELINE | DT_WORDBREAK | DT_CALCRECT
	if ((dwTextFormat & DT_NOCLIP) > 0)
	{
		// we will not render out of range text,  even if no clip is true. so we just add a very big height here.
		if (vAlignment_ == TextVAlignment::TOP && nHeight < 500)
			nHeight = 500;
	}
	setPosition(rect.left, rect.top);

	int nScaledWidth, nScaledHeight, nMaxScaledHeight;
	if (GetFontScaling() == 1.f)
	{
		nScaledWidth = nWidth;
		nScaledHeight = nHeight;
		nMaxScaledHeight = nMaxHeight;
	}
	else
	{
		nScaledWidth = (int)(nWidth / GetFontScaling() + 0.999f);
		nScaledHeight = (int)(nHeight / GetFontScaling() + 0.999f);
		nMaxScaledHeight = (int)(nMaxHeight / GetFontScaling() + 0.999f);
	}
	// we will make the height at least of a single line height to prevent starting a new line in the front when calculating layout.
	// unless nScaledHeight is 0 or DT_CALCRECT is set
	if (!(dwTextFormat & DT_CALCRECT) && _commonLineHeight > nScaledHeight && nScaledHeight > 0)
	{
		if (vAlignment_ == TextVAlignment::CENTER)
		{
			const int topOffset = (nScaledHeight - static_cast<int>(_commonLineHeight)) / 2;
			setPosition(rect.left, (GetFontScaling() == 1.f) ? (rect.top + topOffset) : std::floor(rect.top + topOffset * GetFontScaling() + 0.5f));
		}
		else if (vAlignment_ == TextVAlignment::BOTTOM)
		{
			const int topOffset = nScaledHeight - static_cast<int>(_commonLineHeight);
			setPosition(rect.left, (GetFontScaling() == 1.f) ? (rect.top + topOffset) : std::floor(rect.top + topOffset * GetFontScaling() + 0.5f));
		}
		nScaledHeight = _commonLineHeight;
	}

	setWidth(nScaledWidth);
	setHeight(nScaledHeight);

	setAlignment(hAlignment_, vAlignment_);

	if (UpdateLetterSprites(dwTextFormat))
	{
		if (dwTextFormat & DT_CALCRECT)
		{
			int nContentHeight = (!_clipEnabled && _currNumLines > 0)
				? static_cast<int>(_commonLineHeight * _currNumLines)
				: static_cast<int>(_contentSize.height);
			if (vAlignment_ == TextVAlignment::CENTER && nContentHeight > nMaxScaledHeight)
			{
				rect.top = rect.top - (int)((nContentHeight - nScaledHeight) / 2 * GetFontScaling());
				// again preventing vertical center aligned text to exceed the total height.
				// nContentHeight = nMaxScaledHeight;
			}
			if (GetFontScaling() == 1.f)
			{
				rect.right = rect.left + _contentSize.width;
				rect.bottom = rect.top + nContentHeight;
			}
			else
			{
				rect.right = rect.left + (int)(_contentSize.width * GetFontScaling() + 0.999f);
				rect.bottom = rect.top + (int)(nContentHeight * GetFontScaling() + 0.999f);
			}
			return true;
		}
		RenderLetterSprites(pSprite, textColor);
		return true;
	}
	return false;
}

bool ParaEngine::CFontRendererOpenGL::DrawTextA(CSpriteRenderer* pSprite, const char* strText, RECT& rect, DWORD dwTextFormat, DWORD textColor)
{
	if (!strText || strText[0] == '\0')
		return false;
	std::string utf8Str = strText;
	std::u16string utf16Str;
	if (StringHelper::UTF8ToUTF16(utf8Str, utf16Str))
	{
		return DrawTextW(pSprite, utf16Str.c_str(), rect, dwTextFormat, textColor);
	}
	return false;
}

bool ParaEngine::CFontRendererOpenGL::UpdateLetterSprites(DWORD dwTextFormat)
{
	if (_fontAtlas == nullptr || _currentUTF16String.empty())
	{
		return false;
	}

	computeStringNumLines();
	if (_fontAtlas)
	{
		computeHorizontalKernings(_currentUTF16String);
	}

	_fontAtlas->prepareLetterDefinitions(_currentUTF16String);

	LabelTextFormatter::createStringSprites(this);
	if ((dwTextFormat & DT_WORDBREAK) && !(dwTextFormat & DT_SINGLELINE) && _maxLineWidth > 0 && _contentSize.width > _maxLineWidth && LabelTextFormatter::multilineText(this))
		LabelTextFormatter::createStringSprites(this);

	if (_labelWidth > 0 || (_currNumLines > 1 && _hAlignment != TextHAlignment::LEFT))
		LabelTextFormatter::alignText(this);

	return true;
}

void ParaEngine::CFontRendererOpenGL::RenderLetterSprites(CSpriteRenderer* pSprite, DWORD color)
{
	if (m_bUseCustomShader)
	{
		if (pSprite->BeginCustomShader())
		{
			CEffectFile * pEffectFile = NULL;
			CGlobals::GetEffectManager()->BeginEffect(TECH_GUI_TEXT, &pEffectFile);
			if (pEffectFile)
			{
				pEffectFile->begin();
				pEffectFile->BeginPass(0);
				pEffectFile->CommitChanges();
			}
			DoRender(pSprite, color);

			pSprite->Flush();
			if (pEffectFile)
			{
				pEffectFile->EndPass();
				pEffectFile->end();
			}
			pSprite->EndCustomShader();
		}
	}
	else
	{
		pSprite->SetTextMode(true);
		DoRender(pSprite, color);
		pSprite->SetTextMode(false);
	}
}

float ParaEngine::CFontRendererOpenGL::GetFontScaling() const
{
	return m_fFontScaling;
}

void ParaEngine::CFontRendererOpenGL::RendererRecreated()
{
	if (m_pScaledFontAtlas)
	{
		FontAtlasCache::releaseFontAtlas(m_pScaledFontAtlas);
		m_pScaledFontAtlas = nullptr;
		m_nScaledFontSize = 0;
	}
	FontAtlasCache::purgeCachedData();
}

int ParaEngine::CFontRendererOpenGL::SelectRenderFontSize(float screenFontSize)
{
#if !defined(PARAENGINE_MOBILE)
	if (screenFontSize >= 8.0f && screenFontSize <= 50.0f)
	{
		const int nearestSize = static_cast<int>(std::floor(screenFontSize + 0.5f));
		if (std::fabs(screenFontSize - static_cast<float>(nearestSize)) < 0.08f)
			return nearestSize;
	}
#endif
	if (std::fabs(screenFontSize - 12.0f) < 0.08f)
		return 12;
	if (std::fabs(screenFontSize - 14.0f) < 0.08f)
		return 14;
	if (std::fabs(screenFontSize - 20.0f) < 0.08f)
		return 20;
	if (std::fabs(screenFontSize - 28.0f) < 0.08f)
		return 28;

	// For scaled / fractional screen sizes, select from the shared {12, 14, 20, 28, 50}
	// atlas buckets so screenFontSize / atlasSize stays in [0.51, 0.91]:
	// - Never < 0.5x minification (where 2x2 linear sampling skips 1-texel strokes)
	// - ~1.4x-1.9x supersampled for small scaled fonts so thin CJK/Latin strokes stay crisp
	// - High-resolution atlas when upscaling small base fonts (e.g. HiDPI or zoomed UI)
	if (screenFontSize <= 8.0f)
		return 12;
	if (screenFontSize <= 10.5f)
		return 14;
	if (screenFontSize <= 16.5f)
		return 20;
	if (screenFontSize <= 25.5f)
		return 28;
	return 50;
}

GLFontAtlas* ParaEngine::CFontRendererOpenGL::GetScaledFontAtlas(int nRealFontSize, CSpriteRenderer* pSprite)
{
	if (nRealFontSize == m_nRealFontSize || nRealFontSize <= 0)
		return _fontAtlas;

	if (m_pScaledFontAtlas && m_nScaledFontSize == nRealFontSize)
		return m_pScaledFontAtlas;

	if (m_pScaledFontAtlas)
	{
		// Queued glyph quads borrow the atlas textures until the sprite batch is drawn.
		// Submit them before releasing an atlas when labels change scale.
		pSprite->Flush();
		FontAtlasCache::releaseFontAtlas(m_pScaledFontAtlas);
		m_pScaledFontAtlas = nullptr;
		m_nScaledFontSize = 0;
	}

	TTFConfig cfg(_fontConfig.fontFilePath.c_str(), nRealFontSize, GlyphCollection::DYNAMIC, nullptr, _fontConfig.distanceFieldEnabled, _fontConfig.outlineSize);
	cfg.bold = _fontConfig.bold;
	cfg.fallbackFontFilePath = _fontConfig.fallbackFontFilePath;
	m_pScaledFontAtlas = FontAtlasCache::getFontAtlasTTF(cfg);
	if (m_pScaledFontAtlas)
	{
		m_pScaledFontAtlas->setAntiAliasTexParameters();
		m_nScaledFontSize = nRealFontSize;
		return m_pScaledFontAtlas;
	}
	return _fontAtlas;
}

void ParaEngine::CFontRendererOpenGL::DoRender(CSpriteRenderer* pSprite, DWORD color)
{
	if (!_fontAtlas || _limitShowCount <= 0)
		return;

	Matrix4 matOld;
	pSprite->GetTransform(matOld);

	int targetRealFontSize = m_nRealFontSize;
	if (!pSprite->IsUseObjectSpaceTransform() && !_useDistanceField && !_clipEnabled && _fontConfig.outlineSize == 0 && !_fontConfig.fontFilePath.empty())
	{
		const float scaleX = std::sqrt(matOld._11 * matOld._11 + matOld._12 * matOld._12);
		const float scaleY = std::sqrt(matOld._21 * matOld._21 + matOld._22 * matOld._22);
		const float spriteScale = (std::max)(scaleX, scaleY);
		if (spriteScale > 0.01f)
		{
			const float screenFontSize = static_cast<float>(m_nFontSize) * spriteScale;
			if (std::fabs(screenFontSize - static_cast<float>(m_nRealFontSize)) < 0.08f)
				targetRealFontSize = m_nRealFontSize;
			else
				targetRealFontSize = SelectRenderFontSize(screenFontSize);
		}
	}

	GLFontAtlas* renderAtlas = GetScaledFontAtlas(targetRealFontSize, pSprite);
	if (!renderAtlas)
		renderAtlas = _fontAtlas;

	if (renderAtlas != _fontAtlas)
	{
		renderAtlas->prepareLetterDefinitions(_currentUTF16String);
		const auto& textures = renderAtlas->getTextures();
		const float fRenderScaling = static_cast<float>(m_nFontSize) / static_cast<float>(targetRealFontSize);
		const float fLayoutToRender = static_cast<float>(targetRealFontSize) / static_cast<float>(m_nRealFontSize);

		Matrix4 matTransform;
		Vector2 vScaling(fRenderScaling, fRenderScaling);
		Vector2 vTranslation(getPositionX(), getPositionY());
		ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScaling, NULL, 0.f, &vTranslation);
		matTransform = matTransform * matOld;
		const bool snapScaled1To1 = !pSprite->IsUseObjectSpaceTransform()
			&& std::fabs(matTransform._12) < 1e-4f && std::fabs(matTransform._21) < 1e-4f
			&& std::fabs(matTransform._11 - 1.0f) < 0.02f && std::fabs(matTransform._22 - 1.0f) < 0.02f;
		if (snapScaled1To1)
		{
			matTransform._41 = std::floor(matTransform._41 + 0.5f);
			matTransform._42 = std::floor(matTransform._42 + 0.5f);
		}
		pSprite->SetTransform(matTransform);

		const int strLen = static_cast<int>(_currentUTF16String.size());
		FontLetterDefinition renderDef;
		for (int ctr = 0; ctr < _limitShowCount && ctr < strLen; ++ctr)
		{
			const FontLetterDefinition& layoutDef = _lettersInfo[ctr].def;
			if (!layoutDef.validDefinition)
				continue;

			const char16_t ch = _currentUTF16String[ctr];
			if (!renderAtlas->getLetterDefinitionForChar(ch, renderDef) || !renderDef.validDefinition || renderDef.width <= 0.f || renderDef.height <= 0.f)
				continue;

			auto texIt = textures.find(renderDef.textureID);
			if (texIt == textures.end() || !texIt->second)
				continue;

			const float penLayoutX = _lettersInfo[ctr].position.x - layoutDef.offsetX;
			const float lineTopLayoutY = (getHeight() - _lettersInfo[ctr].position.y) - layoutDef.offsetY;

			TextureEntityOpenGL tex(texIt->second);
			RECT rect;
			rect.left = static_cast<LONG>(renderDef.U);
			rect.top = static_cast<LONG>(renderDef.V);
			rect.right = rect.left + static_cast<LONG>(renderDef.width);
			rect.bottom = rect.top + static_cast<LONG>(renderDef.height);
			const float rx = snapScaled1To1 ? std::floor(penLayoutX * fLayoutToRender + 0.5f) : (penLayoutX * fLayoutToRender);
			const float ry = snapScaled1To1 ? std::floor(lineTopLayoutY * fLayoutToRender + 0.5f) : (lineTopLayoutY * fLayoutToRender);
			Vector3 vPos(rx + renderDef.offsetX, ry + renderDef.offsetY, 0.f);
			pSprite->DrawQuad(&tex, &rect, NULL, &vPos, color);
		}

		pSprite->SetTransform(matOld);
		return;
	}

	const auto& textures = _fontAtlas->getTextures();
	if (GetFontScaling() == 1.f)
	{
		const bool isPixelAligned1To1 = !pSprite->IsUseObjectSpaceTransform()
			&& std::fabs(matOld._12) < 1e-4f && std::fabs(matOld._21) < 1e-4f
			&& std::fabs(matOld._11 - 1.0f) < 1e-3f && std::fabs(matOld._22 - 1.0f) < 1e-3f;
		float baseX = getPositionX();
		float baseY = getPositionY();
		if (isPixelAligned1To1)
		{
			baseX = std::floor(baseX + matOld._41 + 0.5f) - matOld._41;
			baseY = std::floor(baseY + matOld._42 + 0.5f) - matOld._42;
		}

		for (int ctr = 0; ctr < _limitShowCount; ++ctr)
		{
			const FontLetterDefinition &letterDef = _lettersInfo[ctr].def;

			if (letterDef.validDefinition && letterDef.width > 0.f && letterDef.height > 0.f)
			{
				auto texIt = textures.find(letterDef.textureID);
				if (texIt == textures.end() || !texIt->second)
					continue;
				TextureEntityOpenGL tex(texIt->second);
				RECT rect;
				rect.left = letterDef.U;
				rect.top = letterDef.V;
				rect.right = rect.left + letterDef.width;
				rect.bottom = rect.top + letterDef.height;
				// directX and opengGL screen space y differs by 1-y, so we need to set to getHeight() - y.
				const float localX = isPixelAligned1To1 ? std::floor(_lettersInfo[ctr].position.x + 0.5f) : _lettersInfo[ctr].position.x;
				const float localY = isPixelAligned1To1 ? std::floor((getHeight() - _lettersInfo[ctr].position.y) + 0.5f) : (getHeight() - _lettersInfo[ctr].position.y);
				Vector3 vPos(localX + baseX, localY + baseY, 0);
				pSprite->DrawQuad(&tex, &rect, NULL, &vPos, color);
			}
		}
	}
	else
	{
		Matrix4 matTransform;
		Vector2 vScaling(GetFontScaling(), GetFontScaling());
		Vector2 vTranslation(getPositionX(), getPositionY());
		ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScaling, NULL, 0.f, &vTranslation);
		matTransform = matTransform * matOld;
		pSprite->SetTransform(matTransform);

		for (int ctr = 0; ctr < _limitShowCount; ++ctr)
		{
			const FontLetterDefinition &letterDef = _lettersInfo[ctr].def;

			if (letterDef.validDefinition && letterDef.width > 0.f && letterDef.height > 0.f)
			{
				auto texIt = textures.find(letterDef.textureID);
				if (texIt == textures.end() || !texIt->second)
					continue;
				TextureEntityOpenGL tex(texIt->second);
				RECT rect;
				rect.left = letterDef.U;
				rect.top = letterDef.V;
				rect.right = rect.left + letterDef.width;
				rect.bottom = rect.top + letterDef.height;
				// directX and opengGL screen space y differs by 1-y, so we need to set to getHeight() - y.
				Vector3 vPos(_lettersInfo[ctr].position.x, (getHeight() - _lettersInfo[ctr].position.y), 0);
				pSprite->DrawQuad(&tex, &rect, NULL, &vPos, color);
			}
		}
		pSprite->SetTransform(matOld);
	}
}

#endif
