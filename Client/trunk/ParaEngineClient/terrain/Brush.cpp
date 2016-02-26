//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2009.11
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TextureEntity.h"
#include "Brush.h"
#include "memdebug.h"
using namespace ParaTerrain;

Brush::Brush(int width)
{
	m_Width = width;
	m_Intensity = 1.0f;
	m_MaxIntensity = 1.0f;
	m_bErase = false;
	m_pBuffer = 0;
	BuildBuffer();
}

Brush::~Brush()
{
	delete[]m_pBuffer;
}

void Brush::Paint(Terrain * pTerrain, int detailTextureIndex, float x, float y)
{
	int textureCellX, textureCellY;
	float texU, texV;
	pTerrain->GetTextureCoordinates(x, y, textureCellX, textureCellY, texU, texV);
	{
		int imageWidth = TextureCell::GetDetailMaskImageWidth(detailTextureIndex);
		int imageHeight = imageWidth;
		/**
		* LiXizhi: There is a very tricky work around here. 
		* textureCellX, textureCellY may be out of valid range. 
		* if x<0, textureCellX is always 0. however texU is negative, 
		*  this will make (pos(textureCellX)+texU * imageWidth) the correct position.
		* if x is larger than the terrain size, textureCellX is larger than (the number of cells -1). however texU is positive, 
		*  this will make (pos(textureCellX)+texU * imageWidth) the correct position on a cell that does not actually exists. 
		*  however, the textureCellX passed to Paint() function does not have to be valid. 
		* The same applies to Y axis.
		*/
		Paint(pTerrain, detailTextureIndex, textureCellX, textureCellY, (int)(texU * imageWidth), (int)(texV * imageHeight));
	}
}

void Brush::Paint(Terrain * pTerrain, int detailTextureIndex, int cellX, int cellY, int x, int y)
{
	int targetX, targetY;
	int endX, endY;
	int halfWidth;
	
	int targetWidth = TextureCell::GetDetailMaskImageWidth(detailTextureIndex);
	int targetHeight = targetWidth;
	if (2 * targetWidth - 1 <= m_Width)
		m_Width = 2 * targetWidth - 1;
	if (2 * targetHeight - 1 <= m_Width)
		m_Width = 2 * targetHeight - 1;

	if (m_Width > 1)
		halfWidth = m_Width / 2;
	else
		halfWidth = 0;

	targetX = x - halfWidth;
	targetY = y - halfWidth;
	endX = targetX + m_Width;
	endY = targetY + m_Width;
	int myX, myY;
	int maxValue = (int)(m_MaxIntensity * 255.0f);
	
	bool refreshLeft = false;
	bool refreshTop = false;
	bool refreshRight = false;
	bool refreshBottom = false;

	for (myX = 0, targetX = x - halfWidth; targetX < endX; targetX++, myX++)
	{
		for (myY = 0, targetY = y - halfWidth; targetY < endY; targetY++, myY++)
		{
			int adjustedTargetX = targetX;
			int adjustedTargetY = targetY;
			int targetCellX = cellX;
			int targetCellY = cellY;
			if (targetX < 0)
			{
				targetCellX--;
				refreshLeft = true;
				adjustedTargetX = targetWidth+targetX;
			}
			if (targetY < 0)
			{
				targetCellY--;
				refreshBottom = true;
				adjustedTargetY = targetHeight+targetY;
			}
			if (targetWidth <= targetX)
			{
				targetCellX++;
				refreshRight = true;
				adjustedTargetX = (targetX - targetWidth);
			}
			if (targetHeight <= targetY)
			{
				targetCellY++;
				refreshTop = true;
				adjustedTargetY = (targetY - targetHeight);
			}
			if (0 <= targetCellX && 0 <= targetCellY && targetCellX < pTerrain->GetNumberOfTextureTilesWidth() && targetCellY < pTerrain->GetNumberOfTextureTilesHeight())
			{
				// TODO 2008.12.17: change the mask bits of all other texture layer according to the current layer value. 
				unsigned char *pTargetBuffer = pTerrain->GetMaskBits(targetCellX, targetCellY, detailTextureIndex, targetWidth, targetHeight);
				int targetIndex = adjustedTargetY * targetWidth + adjustedTargetX;
				int myIndex = myY * m_Width + myX;
				int offset = (int)((float)m_pBuffer[myIndex] * (float)m_Intensity);
				if (m_bErase)
					offset *= -1;
				int origValue = pTargetBuffer[targetIndex];
				int newValue = origValue;
				if (m_bErase)
				{
					if (maxValue < origValue)
					{
						newValue = origValue + offset;
						if (newValue < maxValue)
							newValue = maxValue;
					}
				}
				else
				{
					if (origValue < maxValue)
					{
						newValue = origValue + offset;
						if (maxValue < newValue)
							newValue = maxValue;

					}
				}
				if (255 < newValue)
					newValue = 255;
				if (newValue < 0)
					newValue = 0;
				pTargetBuffer[targetIndex] = (unsigned char)newValue;
			}
		}
	}
	pTerrain->ReloadMask(cellX, cellY, detailTextureIndex);
	if (refreshLeft && refreshBottom)
		pTerrain->ReloadMask(cellX - 1, cellY - 1, detailTextureIndex);
	if (refreshBottom)
		pTerrain->ReloadMask(cellX, cellY - 1, detailTextureIndex);
	if (refreshRight && refreshBottom)
		pTerrain->ReloadMask(cellX + 1, cellY - 1, detailTextureIndex);
	if (refreshRight)
		pTerrain->ReloadMask(cellX + 1, cellY, detailTextureIndex);
	if (refreshRight && refreshTop)
		pTerrain->ReloadMask(cellX + 1, cellY + 1, detailTextureIndex);
	if (refreshTop)
		pTerrain->ReloadMask(cellX, cellY + 1, detailTextureIndex);
	if (refreshLeft && refreshTop)
		pTerrain->ReloadMask(cellX - 1, cellY + 1, detailTextureIndex);
	if (refreshLeft)
		pTerrain->ReloadMask(cellX - 1, cellY, detailTextureIndex);

	bool bNormalizeMask = true;
	if(bNormalizeMask)
	{
		pTerrain->NormalizeMask(cellX, cellY, detailTextureIndex);
		if (refreshLeft && refreshBottom)
			pTerrain->NormalizeMask(cellX - 1, cellY - 1, detailTextureIndex);
		if (refreshBottom)
			pTerrain->NormalizeMask(cellX, cellY - 1, detailTextureIndex);
		if (refreshRight && refreshBottom)
			pTerrain->NormalizeMask(cellX + 1, cellY - 1, detailTextureIndex);
		if (refreshRight)
			pTerrain->NormalizeMask(cellX + 1, cellY, detailTextureIndex);
		if (refreshRight && refreshTop)
			pTerrain->NormalizeMask(cellX + 1, cellY + 1, detailTextureIndex);
		if (refreshTop)
			pTerrain->NormalizeMask(cellX, cellY + 1, detailTextureIndex);
		if (refreshLeft && refreshTop)
			pTerrain->NormalizeMask(cellX - 1, cellY + 1, detailTextureIndex);
		if (refreshLeft)
			pTerrain->NormalizeMask(cellX - 1, cellY, detailTextureIndex);
	}
}

void Brush::SetWidth(int width)
{
	if(width<=0)
		width = 1;
	if (m_Width != width)
	{
		m_Width = width;
		BuildBuffer();
	}
}

void Brush::SetErase(bool bErase)
{
	m_bErase = bErase;
}

void Brush::SetIntensity(float intensity)
{
	m_Intensity = intensity;
}

void Brush::SetMaxIntensity(float maxIntensity)
{
	m_MaxIntensity = maxIntensity;
}

void Brush::BuildBuffer()
{
	if (m_pBuffer)
		delete[]m_pBuffer;
	float centerX = (float)m_Width / 2.0f;
	float centerY = (float)m_Width / 2.0f;
	m_pBuffer = new unsigned char[m_Width * m_Width];
	if(m_Width > 1)
	{
		for (int x = 0; x < m_Width; x++)
		{
			for (int y = 0; y < m_Width; y++)
			{
				float deltaX = centerX - (float)x;
				float deltaY = centerY - (float)y;
				float dist = (float)sqrt(deltaX * deltaX + deltaY * deltaY);
				float alpha = 1.0f - (dist / ((float)m_Width / 2.0f));
				if (1.0f < alpha)
					alpha = 1.0f;
				if (alpha < 0.0f)
					alpha = 0.0f;
				m_pBuffer[y * m_Width + x] = (unsigned char)(alpha * 255.0f);
			}
		}
	}
	else
	{
		m_pBuffer[0] = 0xff;
	}
}
