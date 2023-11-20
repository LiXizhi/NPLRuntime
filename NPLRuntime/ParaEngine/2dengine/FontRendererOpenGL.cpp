//-----------------------------------------------------------------------------
// Class: FontRendererOpenGL
// Authors:	LiXizhi
// Emails:
// Date: 2014.8.28
// Desc: rendering a given font from TTF format. It emulates the d3dxfont class.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "IO/FileUtils.h"

#ifdef USE_OPENGL_RENDERER
#include "RenderDeviceOpenGL.h"
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
	:m_nFontSize(14), m_nRealFontSize(14), m_fFontScaling(1.f), m_bUseCustomShader(false)
{
}

CFontRendererOpenGL* ParaEngine::CFontRendererOpenGL::create(const std::string& sFontName, int nFontSize)
{
	CFontRendererOpenGL* pFontRenderer = new CFontRendererOpenGL();
	std::string fontFile;
	if (sFontName == "System" || sFontName == "system")
	{
		// secretly replace system font with one of our own predefined font.
		//fontFile = "fonts/simhei.ttf"; // 9MB
		//fontFile = "fonts/FZYTK.TTF"; // 3MB
		//fontFile = "fonts/SIMYOU.TTF"; // 6MB
#ifdef WIN32
		char szPath[MAX_PATH];
		GetWindowsDirectoryA(szPath, MAX_PATH);
		fontFile = szPath;
		//fontFile = "fonts/DroidSansFallback.ttf"; // 3.6MB
		fontFile = fontFile + "/fonts/msyh.ttc";
#else
		fontFile = "fonts/DroidSansFallback.ttf"; // 3.6MB
#endif
		
	}
	else
	{
		fontFile = sFontName;
	}

	// TODO: force size to be 50. font-rendering crashes on android with big font.
	pFontRenderer->m_nFontSize = nFontSize;
#if defined(EMSCRIPTEN) || defined(WIN32) || defined(PLATFORM_MAC)
	if (pFontRenderer->m_nFontSize <= 12)
	{
		pFontRenderer->m_nRealFontSize = 12;
	}
	else if (pFontRenderer->m_nFontSize <= 14)
	{
		pFontRenderer->m_nRealFontSize = 14;
	}
	else if (pFontRenderer->m_nFontSize <= 20)
	{
		pFontRenderer->m_nRealFontSize = 20;
	}
	else if (pFontRenderer->m_nFontSize <= 28)
	{
		pFontRenderer->m_nRealFontSize = 28;
	}
	else if (pFontRenderer->m_nFontSize <= 50)
	{
		pFontRenderer->m_nRealFontSize = 50;
	}
	else
	{
		pFontRenderer->m_nRealFontSize = 50;
	}
#else
	if (pFontRenderer->m_nFontSize <= 14)
	{
		pFontRenderer->m_nRealFontSize = 20;
	}
	else if (pFontRenderer->m_nFontSize <= 28)
	{
		pFontRenderer->m_nRealFontSize = 28;
	}
	else if (pFontRenderer->m_nFontSize <= 50)
	{
		pFontRenderer->m_nRealFontSize = 50;
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
		if (pFontRenderer->setTTFConfig(ttfConfig))
		{
			pFontRenderer->setFontScale(pFontRenderer->m_fFontScaling);
			if (pFontRenderer->getFontAtlas())
			{
				// TODO: why disable aliasing? This should be enabled LiXizhi
				// pFontRenderer->getFontAtlas()->setAliasTexParameters();
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
		return CFontRendererOpenGL::create("System", nFontSize);
	}
	return NULL;
}

ParaEngine::CFontRendererOpenGL::~CFontRendererOpenGL()
{
	
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
	// unless nScaledHeight is 0, which means that we are calculating rect
	if (_commonLineHeight > nScaledHeight && nScaledHeight > 0)
	{
		if (vAlignment_ == TextVAlignment::CENTER)
		{
			setPosition(rect.left, rect.top - (_commonLineHeight - nScaledHeight) / 2);
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
			int nContentHeight = _contentSize.height;
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
	if (!(dwTextFormat & DT_SINGLELINE) &&  _maxLineWidth > 0 && _contentSize.width > _maxLineWidth && LabelTextFormatter::multilineText(this))
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
	FontAtlasCache::purgeCachedData();
}

void ParaEngine::CFontRendererOpenGL::DoRender(CSpriteRenderer* pSprite, DWORD color)
{
	auto textures = _fontAtlas->getTextures();
	
	if (GetFontScaling() == 1.f)
	{
		for (int ctr = 0; ctr < _limitShowCount; ++ctr)
		{
			const FontLetterDefinition &letterDef = _lettersInfo[ctr].def;

			if (letterDef.validDefinition)
			{
				TextureEntityOpenGL tex(textures[letterDef.textureID]);
				RECT rect;
				rect.left = letterDef.U;
				rect.top = letterDef.V;
				rect.right = rect.left + letterDef.width;
				rect.bottom = rect.top + letterDef.height;
				// directX and opengGL screen space y differs by 1-y, so we need to set to getHeight() - y.
				Vector3 vPos(_lettersInfo[ctr].position.x + getPositionX(), (getHeight() - _lettersInfo[ctr].position.y) + getPositionY(), 0);
				pSprite->DrawQuad(&tex, &rect, NULL, &vPos, color);
			}
		}
	}
	else
	{
		Matrix4 matOld, matTransform;
		pSprite->GetTransform(matOld);
		Vector2 vScaling(GetFontScaling(), GetFontScaling());
		Vector2 vTranslation(getPositionX(), getPositionY());
		ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScaling, NULL, 0.f, &vTranslation);
		matTransform = matTransform * matOld;
		pSprite->SetTransform(matTransform);

		for (int ctr = 0; ctr < _limitShowCount; ++ctr)
		{
			const FontLetterDefinition &letterDef = _lettersInfo[ctr].def;

			if (letterDef.validDefinition)
			{
				TextureEntityOpenGL tex(textures[letterDef.textureID]);
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
