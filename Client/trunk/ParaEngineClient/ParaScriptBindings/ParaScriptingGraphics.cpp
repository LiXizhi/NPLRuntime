//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2005.9.8
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "NPLHelper.h"
#include "2dengine/GUIBase.h"
#include "2dengine/GUIResource.h"
#include "util/StringHelper.h"
#include "ParaScriptingGraphics.h"

using namespace ParaScripting;

//--------------------------------------------------------------
// for ParaUITexture class table.
//--------------------------------------------------------------
DWORD ParaUITexture::GetTransparency()const
{
	if (m_pObj) {
		return ((uint32)m_pObj->TextureColor) >> 24;
	}
	return 0;
}

void ParaUITexture::SetTransparency(DWORD transparency)
{
	if (m_pObj) {
		m_pObj->TextureColor = (((uint32)m_pObj->TextureColor) & 0x00ffffff) | (transparency << 24);
	}
}

std::string ParaUITexture::GetColor()const
{
	char temp[30];
	std::string color = "";
	if (m_pObj) {
		DWORD tcolor = ((uint32)m_pObj->TextureColor) & 0x00ffffff;
		ParaEngine::StringHelper::fast_itoa((tcolor & 0x00ff0000) >> 16, temp, 30);
		color += temp; color += " ";
		ParaEngine::StringHelper::fast_itoa((tcolor & 0x0000ff00) >> 8, temp, 30);
		color += temp; color += " ";
		ParaEngine::StringHelper::fast_itoa(tcolor & 0x000000ff, temp, 30);
		color += temp;
	}
	return color;
}

void ParaUITexture::SetColor(const char *strColor)
{
	if (m_pObj) {
		int r = 255, g = 255, b = 255, a = 255;
		int nCount = sscanf(strColor, "%d %d %d %d", &r, &g, &b, &a);
		m_pObj->TextureColor = COLOR_ARGB(a, r, g, b);
	}
}

std::string ParaUITexture::GetTexture()const
{
	if (m_pObj) {
		return m_pObj->GetTexture()->GetKey();
	}
	return "";
}

void ParaUITexture::SetTexture(const char *texturename)
{
	if (texturename == NULL) { return; }
	if (m_pObj)
	{
		std::string temp = texturename;

		int nInnerRectIndex = 0;
		for (int i = 0; texturename[i] != '\0'; ++i)
		{
			if (texturename[i] == ':')
			{
				nInnerRectIndex = i;
				break;
			}
		}
		if (nInnerRectIndex > 0)
		{
			OUTPUT_LOG("warning: ParaUITexture.SetTexture() with 9 tile \":\" is not supported yet. \n");
			return;
		}
		// 2008.12.27. now support texture with ; rect in file name
		RECT rect;
		RECT* prect = NULL;
		std::string filename;
		prect = ParaEngine::StringHelper::GetImageAndRect(temp, filename, &rect);


		TextureEntity* pTextureEntity = NULL;
		size_t pos;
		if ((pos = filename.find("(n)")) == std::string::npos) {
			pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);

		}
		else{
			filename.erase(pos, 3);
			pTextureEntity = CGlobals::GetAssetManager()->GetTexture(filename.c_str());
		}
		if (pTextureEntity == NULL)
		{
			OUTPUT_LOG("error:Texture % does not exist\n", filename.c_str());
		}
		m_pObj->SetElement(pTextureEntity, prect, m_pObj->TextureColor);
	}
}

std::string ParaUITexture::GetTextureRect()const
{
	std::string rect = "";
	if (m_pObj) {
		char temp[30];
		ParaEngine::StringHelper::fast_itoa(m_pObj->rcTexture.left, temp, 30);
		rect += temp;
		rect += " ";
		ParaEngine::StringHelper::fast_itoa(m_pObj->rcTexture.top, temp, 30);
		rect += temp;
		rect += " ";
		ParaEngine::StringHelper::fast_itoa(m_pObj->rcTexture.right - m_pObj->rcTexture.left, temp, 30);
		rect += temp;
		rect += " ";
		ParaEngine::StringHelper::fast_itoa(m_pObj->rcTexture.bottom - m_pObj->rcTexture.top, temp, 30);
		rect += temp;
	}
	return rect;
}

void ParaUITexture::SetTextureRect(const char *strrect)
{
	if (m_pObj) {
		RECT rect;
		int width, height;
		if (sscanf(strrect, "%d %d %d %d", &rect.left, &rect.top, &width, &height) == 4){
			rect.right = rect.left + width;
			rect.bottom = rect.top + height;
			m_pObj->rcTexture = rect;
		}
	}
}

//--------------------------------------------------------------
// for ParaUIFont class table.
//--------------------------------------------------------------
DWORD ParaUIFont::GetTransparency()const
{
	if (m_pObj) {
		return m_pObj->FontColor >> 24;
	}
	return 0;
}

void ParaUIFont::SetTransparency(DWORD transparency)
{
	if (m_pObj) {
		m_pObj->FontColor = (m_pObj->FontColor & 0x00ffffff) | (transparency << 24);
	}
}

std::string ParaUIFont::GetColor()const
{
	char temp[30];
	std::string color = "";
	if (m_pObj) {
		DWORD tcolor = m_pObj->FontColor & 0x00ffffff;
		ParaEngine::StringHelper::fast_itoa((tcolor & 0x00ff0000) >> 16, temp, 30);
		color += temp; color += " ";
		ParaEngine::StringHelper::fast_itoa((tcolor & 0x0000ff00) >> 8, temp, 30);
		color += temp; color += " ";
		ParaEngine::StringHelper::fast_itoa(tcolor & 0x000000ff, temp, 30);
		color += temp;
	}
	return color;
}

void ParaUIFont::SetColor(const char *strColor)
{
	if (m_pObj)
	{
		LinearColor color;
		int r = 255, g = 255, b = 255, a = 255;
		int nCount = sscanf(strColor, "%d %d %d %d", &r, &g, &b, &a);
		color = COLOR_ARGB(a, r, g, b);
		if (nCount == 3)
		{
			m_pObj->FontColor = (((DWORD)m_pObj->FontColor) & 0xff000000) | (((DWORD)color) & 0x00ffffff);
		}
		else
		{
			m_pObj->FontColor = color;
		}
	}
}

DWORD ParaUIFont::GetFormat()const
{
	if (m_pObj) {
		return m_pObj->dwTextFormat;
	}
	return 0;
}

void ParaUIFont::SetFormat(DWORD format)
{
	if (m_pObj) {
		m_pObj->dwTextFormat = format;
	}
}

std::string ParaUIFont::GetFont()const
{
	if (m_pObj) {
		return m_pObj->GetFont()->GetKey();
	}
	return "";
}

void ParaUIFont::SetFont(const char *fontname)
{
	if (m_pObj) {
		m_pObj->SetElement(CGlobals::GetAssetManager()->GetFont(fontname), m_pObj->FontColor, m_pObj->dwTextFormat);
		if (m_pObj->GetFont() == NULL){
			if (fontname != 0)
			{
				OUTPUT_LOG("error: Font %s does not exist\n", fontname);
			}
		}
	}

}