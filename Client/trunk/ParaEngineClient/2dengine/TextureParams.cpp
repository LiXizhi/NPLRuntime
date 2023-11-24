//----------------------------------------------------------------------
// Title: TextureParams
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2015.4.30
// Desc: 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "PaintEngine/Painter.h"
#include "TextureParams.h"

using namespace ParaEngine;

ParaEngine::TextureParams::TextureParams(const char * str)
	:m_left(0), m_top(0), m_right(0), m_bottom(0), m_toLeft(0), m_toTop(0), m_toRight(0), m_toBottom(0), m_pTexture(NULL)
{
	Init(str);
}

void ParaEngine::TextureParams::Init(const char * str)
{
	if (!str)
		return;
	int nInnerRectIndex = 0;
	char c = 0;
	std::string filename;
	for (int i = 0; (c=str[i]) != '\0'; ++i)
	{
		if (c == ';' || c == '#')
		{
			if (sscanf(&(str[i + 1]), "%d %d %d %d", &m_left, &m_top, &m_right, &m_bottom) == 4)
			{
				m_right = m_left + m_right;
				m_bottom = m_top + m_bottom;
				if (filename.empty())
					filename.append(&(str[0]), i);
			}
			else 
			{
				m_left = 0; m_top = 0; m_right = 0; m_bottom = 0;
			}
		}
		else if (c == ':' && i > 2) // this fixed a bug for win32 absolute path like "c:/"
		{
			if (sscanf(&(str[i + 1]), "%d %d %d %d", &m_toLeft, &m_toTop, &m_toRight, &m_toBottom) == 4)
			{
				if (filename.empty())
					filename.append(&(str[0]), i);
				break;
			}
			else
			{
				m_toLeft = 0; m_toTop = 0; m_toRight = 0; m_toBottom = 0;
			}
		}
	}
	if (filename.empty())
		filename = str;
	m_pTexture = CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture);
}

int ParaEngine::TextureParams::GetImageRects(RECT* rcTextures)
{
	if (m_pTexture)
	{
		if (m_right == 0)
		{
			// touch the texture 
			m_pTexture->GetTexture();
			const TextureEntity::TextureInfo* pTextureInfo = m_pTexture->GetTextureInfo();
			if (pTextureInfo)
			{
				// please note, right and bottom may be (-1,-1) if texture is not available. the GUIElement will automatically recover to correct value when texture is available. 
				m_right = pTextureInfo->m_width;
				m_bottom = pTextureInfo->m_height;
			}
		}
		// image is not loaded yet or invalid. 
		if (m_right <= 0)
			return 0;

		if (IsNineTileTexture())
		{
			RECT rcInner;
			rcInner.left = m_left + m_toLeft;
			rcInner.top = m_top + m_toTop;
			rcInner.right = m_right - m_toRight;
			rcInner.bottom = m_bottom - m_toBottom;

			RECT rect = { m_left, m_top, m_right, m_bottom };

			// assign all 9 textures
			rcTextures[0] = rcInner;
			rcTextures[1] = { rect.left, rect.top, rcInner.left, rcInner.top };
			rcTextures[2] = { rcInner.left, rect.top, rcInner.right, rcInner.top };
			rcTextures[3] = { rcInner.right, rect.top, rect.right, rcInner.top };
			rcTextures[4] = { rect.left, rcInner.top, rcInner.left, rcInner.bottom };
			rcTextures[5] = { rcInner.right, rcInner.top, rect.right, rcInner.bottom };
			rcTextures[6] = { rect.left, rcInner.bottom, rcInner.left, rect.bottom };
			rcTextures[7] = { rcInner.left, rcInner.bottom, rcInner.right, rect.bottom };
			rcTextures[8] = { rcInner.right, rcInner.bottom, rect.right, rect.bottom };
			return 9;
		}
		else
		{
			rcTextures[0] = { m_left, m_top, m_right, m_bottom };
			return 1;
		}
	}
	return 0;
}

void ParaEngine::TextureParams::DrawSingleTexture(CPainter* painter, int x, int y, int w, int h, const RECT &rc)
{
	painter->drawTexture(x, y, w, h, GetTexture(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

void ParaEngine::TextureParams::DrawNineTileTexture(CPainter* painter, int x, int y, int w, int h, const RECT* rcTextures)
{
	RECT rcInner;
	rcInner.left = x + m_toLeft;
	rcInner.top = y + m_toTop;
	rcInner.right = x + w - m_toRight;
	rcInner.bottom = y + h - m_toBottom;
	RECT rect = { x, y, x + w, y + h };

	const RECT* rc = &rcTextures[0];
	painter->drawTexture(rcInner.left, rcInner.top, rcInner.right - rcInner.left, rcInner.bottom - rcInner.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[1];
	painter->drawTexture(rect.left, rect.top, rcInner.left - rect.left, rcInner.top - rect.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[2];
	painter->drawTexture(rcInner.left, rect.top, rcInner.right - rcInner.left, rcInner.top - rect.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[3];
	painter->drawTexture(rcInner.right, rect.top, rect.right - rcInner.right, rcInner.top - rect.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[4];
	painter->drawTexture(rect.left, rcInner.top, rcInner.left - rect.left, rcInner.bottom - rcInner.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[5];
	painter->drawTexture(rcInner.right, rcInner.top, rect.right - rcInner.right, rcInner.bottom - rcInner.top, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[6];
	painter->drawTexture(rect.left, rcInner.bottom, rcInner.left - rect.left, rect.bottom - rcInner.bottom, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[7];
	painter->drawTexture(rcInner.left, rcInner.bottom, rcInner.right - rcInner.left, rect.bottom - rcInner.bottom, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
	rc = &rcTextures[8];
	painter->drawTexture(rcInner.right, rcInner.bottom, rect.right - rcInner.right, rect.bottom - rcInner.bottom, GetTexture(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);	
}


void ParaEngine::TextureParams::drawTexture(CPainter* painter, int x, int y, int w, int h, const RECT* rcTextures /*= NULL*/, int nCount /*= -1*/)
{
	if (painter)
	{
		RECT rcTextures_[9];
		if (rcTextures == 0)
		{
			nCount = GetImageRects(rcTextures_);
			rcTextures = rcTextures_;
		}
		if (nCount == 1)
		{
			DrawSingleTexture(painter, x, y, w, h, rcTextures[0]);
		}
		else if (nCount == 9)
		{
			DrawNineTileTexture(painter, x, y, w, h, rcTextures);
		}
	}
}
