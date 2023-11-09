// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "FileManager.h"
#include "TextureEntity.h"
#include "TextureCell.h"

#include "memdebug.h"

using namespace ParaTerrain;
using namespace ParaEngine;
using namespace std;

TextureCell::TextureCell()
{
	m_pBaseLayer = NULL;
	m_Index = -1;
}

TextureCell::TextureCell(int index)
{
	m_pBaseLayer = NULL;
	m_Index = index;
}

TextureCell::~TextureCell()
{
	SAFE_DELETE(m_pBaseLayer);
	
	RemoveAllDetail();
}

DetailTexture* TextureCell::GetDetailBase()
{
	return m_pBaseLayer;
}

Texture *TextureCell::GetTexture()
{
	// this function is obsoleted. 
	if(m_pBaseLayer)
		return m_pBaseLayer->GetTexture();
	else 
		return NULL;
}

int TextureCell::GetDetailTextureImageWidth(int detailIndex)
{
	DetailTexture* pTexture =  GetDetail(detailIndex);
	if(pTexture && pTexture->GetTexture())
		return pTexture->GetTexture()->GetWidth();
	else
		return 0;
}

int TextureCell::GetDetailTextureImageHeight(int detailIndex)
{
	DetailTexture* pTexture =  GetDetail(detailIndex);
	if(pTexture && pTexture->GetTexture())
		return pTexture->GetTexture()->GetHeight();
	else
		return 0;
}

int TextureCell::GetDetailTextureImageBitsPerPixel(int detailIndex)
{
	DetailTexture* pTexture =  GetDetail(detailIndex);
	if(pTexture && pTexture->GetTexture())
		return pTexture->GetTexture()->GetBitsPerPixel();
	else
		return 0;

}

int TextureCell::GetDetailMaskImageWidth(int detailIndex)
{
	return Settings::GetInstance()->GetTextureMaskWidth();
}

int TextureCell::GetDetailMaskImageHeight(int detailIndex)
{
	return Settings::GetInstance()->GetTextureMaskHeight();
}

int TextureCell::GetDetailMaskImageBitsPerPixel(int detailIndex)
{
	return Settings::TextureMaskBitsPerPixel;
}

int TextureCell::GetBaseTextureImageWidth()
{
	if(m_pBaseLayer && m_pBaseLayer->GetTexture())
		return m_pBaseLayer->GetTexture()->GetWidth();
	else 
		return 0;
}

int TextureCell::GetBaseTextureImageHeight()
{
	if(m_pBaseLayer && m_pBaseLayer->GetTexture())
		return m_pBaseLayer->GetTexture()->GetHeight();
	else 
		return 0;
}

int TextureCell::GetBaseTextureImageBitsPerPixel()
{
	if(m_pBaseLayer && m_pBaseLayer->GetTexture())
		return m_pBaseLayer->GetTexture()->GetBitsPerPixel();
	else 
		return 0;
}

void TextureCell::FlipHorizontal()
{
	if(m_pBaseLayer)
		m_pBaseLayer->FlipHorizontal();
	for (uint32 i = 0; i < m_DetailTextures.size(); i++)
		m_DetailTextures[i]->FlipHorizontal();
}

void TextureCell::FlipVertical()
{
	if(m_pBaseLayer)
		m_pBaseLayer->FlipVertical();
	for (uint32 i = 0; i < m_DetailTextures.size(); i++)
		m_DetailTextures[i]->FlipVertical();
}

void TextureCell::WriteMask(CParaFile& file, Terrain * pTerrain)
{
	bool bWriteBaseLayerMask = true;
	int numDetails = (int)m_DetailTextures.size();
	file.WriteDWORD((DWORD)(bWriteBaseLayerMask ? (numDetails+1) : numDetails));

	if(bWriteBaseLayerMask)
	{
		// Write mask for base layer 0;
		if(m_pBaseLayer)
			m_pBaseLayer->WriteMask(file, pTerrain);
	}
	
	for (int i = 0; i < numDetails; i++)
		m_DetailTextures[i]->WriteMask(file, pTerrain);
}

void TextureCell::ReadMask(CParaFile& file, Terrain * pTerrain)
{
	RemoveAllDetail();
	int numDetails = (int)file.ReadDWORD();
	if(numDetails>0)
	{
		// peek the texture Index.
		//DWORD dwHeader = (*(DWORD*)(file.getPointer()));
		DWORD dwHeader;
		memcpy(&dwHeader, file.getPointer(), sizeof(DWORD));

		if((dwHeader & 0xff) == 0xff)
		{
			// read mask for base layer 0, if there are further data: for backward compatibilities, we need to allow blank of this segment of data during reading. 
			if(m_pBaseLayer)
			{
				m_pBaseLayer->ReadMask(file, pTerrain);
			}
			else
			{
				m_pBaseLayer = new DetailTexture(file, pTerrain);
			}
		}
		else
		{
			// this is just a normal layer after the base layer. 
			DetailTexture *pDet = new DetailTexture(file, pTerrain);
			m_DetailTextures.push_back(pDet);
		}
	}
	
	for (int i=1; i < numDetails; i++)
	{
		DetailTexture *pDet = new DetailTexture(file, pTerrain);
		m_DetailTextures.push_back(pDet);
	}
	
}

void TextureCell::SetTexture(Texture * pTexture)
{
	if(pTexture)
		pTexture->SetSharedIndex(-1);

	if(m_pBaseLayer == 0)
	{
		// TODO: use lazy loading of mask file in DetailTexture for base layer. 
		m_pBaseLayer = new DetailTexture(pTexture, (Texture*)NULL);
		m_pBaseLayer->SetOwnTexture(true);
	}
	else
	{
		m_pBaseLayer->SetTexture(pTexture);
	}
}

DetailTexture *TextureCell::GetDetail(int index)
{
	return (index>=0) ? m_DetailTextures[index] : m_pBaseLayer;
}

DetailTexture *TextureCell::GetDetail(Texture * pTexFind)
{
	if(pTexFind != 0)
	{
		DetailTexture *pDetailFound = NULL;
		int numDetails = GetNumberOfDetails();
		for (int i = 0; i < numDetails && pDetailFound == NULL; ++i)
		{
			if (m_DetailTextures[i]->GetTexture() == pTexFind)
				pDetailFound = m_DetailTextures[i];
		}
		return pDetailFound;
	}
	else
		return m_pBaseLayer;
}

int TextureCell::GetDetailIndex( Texture * pTexFind )
{
	if(pTexFind != 0)
	{
		int numDetails = GetNumberOfDetails();
		for (int i = 0; i < numDetails; ++i)
		{
			if (m_DetailTextures[i]->GetTexture() == pTexFind)
				return i;
		}
		return -2;
	}
	else
		return -1;
}

void TextureCell::AddDetail(DetailTexture * pDetail)
{
	m_DetailTextures.push_back(pDetail);
}

int TextureCell::GetNumberOfDetails()
{
	return (int)m_DetailTextures.size();
}
DeviceTexturePtr_type TextureCell::BindTexture()
{
	if(m_pBaseLayer)
		return m_pBaseLayer->BindTexture();
	else
		return 0;
}

DeviceTexturePtr_type TextureCell::BindMask()
{
	if(m_pBaseLayer)
		return m_pBaseLayer->BindMask();
	else
		return 0;
}


DeviceTexturePtr_type TextureCell::BindMask(int index)
{
	DetailTexture* pTexture =  GetDetail(index);
	if(pTexture)
		return pTexture->BindMask();
	else
		return 0;
}

DeviceTexturePtr_type TextureCell::BindDetail(int index)
{
	DetailTexture* pTexture =  GetDetail(index);
	if(pTexture)
		return pTexture->BindTexture();
	else
		return 0;
}

void TextureCell::UnbindAll()
{
	if(m_pBaseLayer)
		m_pBaseLayer->Unbind();
	int numDetails = GetNumberOfDetails();
	for (int i = 0; i < numDetails ; ++i)
		m_DetailTextures[i]->Unbind();
}

void TextureCell::OptimizeLayer()
{
	DetailTextureArray_type::iterator Iter;
	for ( Iter = m_DetailTextures.begin() ; Iter != m_DetailTextures.end() ; )
	{
		bool bEmptyLayer = false;
		Texture * pMask = (*Iter)->GetMask();
		if(pMask)
		{
			unsigned char* buf = pMask->GetBuffer();
			int nBytes = pMask->GetWidth()*pMask->GetHeight()*pMask->GetBytesPerPixel();
			int k;
			for(k=0; (k<nBytes) && (buf[k] == 0); ++k)
				;
			if (k == nBytes)
				bEmptyLayer = true;
		}
		else
			bEmptyLayer = true;
		if (bEmptyLayer)
		{
			delete (*Iter);
			Iter = m_DetailTextures.erase(Iter);
		}
		else
			 ++Iter;
	}
}

bool ParaTerrain::TextureCell::RemoveDetail( int nIndex )
{
	int i=0;
	DetailTextureArray_type::iterator Iter;
	for ( Iter = m_DetailTextures.begin() ; Iter != m_DetailTextures.end() ; ++i)
	{
		if (i == nIndex)
		{
			delete (*Iter);
			Iter = m_DetailTextures.erase(Iter);
			return true;
		}
		else
			++Iter;
	}
	return false;
}

bool ParaTerrain::TextureCell::RemoveAllDetail()
{
	DetailTextureArray_type::iterator itCur, itEnd = m_DetailTextures.end();
	for (itCur = m_DetailTextures.begin(); itCur!=itEnd; ++itCur)
	{
		DetailTexture *pDetailTexture = *itCur;
		SAFE_DELETE(pDetailTexture);
	}
	m_DetailTextures.clear();
	return true;
}

void ParaTerrain::TextureCell::ResizeTextureMaskWidth( int nWidth )
{
	DetailTextureArray_type::iterator itCur, itEnd = m_DetailTextures.end();
	for (itCur = m_DetailTextures.begin(); itCur!=itEnd; ++itCur)
	{
		DetailTexture *pDetailTexture = *itCur;
		if(pDetailTexture && pDetailTexture->GetMask())
		{
			if(pDetailTexture->GetMask()->GetWidth() != nWidth)
			{
				pDetailTexture->GetMask()->SetWidth(nWidth);
				pDetailTexture->GetMask()->UnloadTexture();
			}
		}
	}
	if(m_pBaseLayer && m_pBaseLayer->GetMask() && m_pBaseLayer->GetMask()->GetWidth() != nWidth)
	{
		m_pBaseLayer->GetMask()->SetWidth(nWidth);
		m_pBaseLayer->GetMask()->UnloadTexture();
	}
}

int TextureCell::NormalizeMask( int index, float fScale)
{
	int nLayerAffected = 0;
	bool bNeedRefresh = false;
	DetailTexture* pRefLayer = GetDetail(index);
	if(pRefLayer == 0)
		return 0;
	Texture * pRefMask = pRefLayer->GetMask();
	if(pRefMask == 0 || pRefMask->GetBuffer() == NULL)
		return 0;
	unsigned char *pRefBuffer = pRefMask->GetBuffer();

	int nlayerCount = (int)(m_DetailTextures.size());

	int pixelCount = GetDetailMaskImageWidth(index)*GetDetailMaskImageHeight(index);
	for (int nPixelIndex=0; nPixelIndex<pixelCount; ++nPixelIndex)
	{
		int refAlpha = (int)(pRefBuffer[nPixelIndex] * fScale);

		int remainAlpha = 0;
		for (int i=-1;i<nlayerCount;++i)
		{
			if(i!=index)
			{
				DetailTexture* pTexLayer = GetDetail(i);
				if (pTexLayer)
				{
					Texture * pMask = pTexLayer->GetMask();
					if(pMask && pMask->GetBuffer())
					{
						remainAlpha += pMask->GetBuffer()[nPixelIndex];
					}
					else
					{
						remainAlpha += (i>0) ? 0 : 255;
					}
				}
			}
		}
		float sScaleFactor = 1.f;
		if(remainAlpha == 0 && refAlpha==0)
		{
			if(fScale>0.f)
			{
				// use reference layer
				pRefBuffer[nPixelIndex] = 255;
			}
			else
			{
				// need to select a most possible layer in the rest of the layers, 
				// we will use the first none empty layer before the index. 
				for (int i=index-1;i>=-1;--i)
				{
					DetailTexture* pTexLayer = GetDetail(i);
					if (pTexLayer)
					{
						Texture * pMask = pTexLayer->GetMask();
						if(pMask == 0)
						{
							if(i == -1)
							{
								pMask = pTexLayer->RegenerateMask(0xff);
								break;
							}
						}
						else
						{
							pMask->GetBuffer()[nPixelIndex] = 255;
							break;
						}
					}
				}
			}
		}
		else if(remainAlpha > 0)
		{
			// scale the other value
			sScaleFactor = (float)(255.f-refAlpha) / (float)remainAlpha;
		}
		else if(refAlpha < 255)
		{
			pRefBuffer[nPixelIndex] = 255;
		}
		if(sScaleFactor!=1.f)
		{
			bNeedRefresh = true;
			for (int i=-1;i<nlayerCount;++i)
			{
				if(i!=index)
				{
					DetailTexture* pTexLayer = GetDetail(i);
					if (pTexLayer)
					{
						Texture * pMask = pTexLayer->GetMask();
						if(pMask == 0)
						{
							pMask = pTexLayer->RegenerateMask(i>=0 ? 0 : 0xff);
						}
						if(pMask && pMask->GetBuffer())
						{
							int newValue = (int)(pMask->GetBuffer()[nPixelIndex] * sScaleFactor);
							if (255 < newValue)
								newValue = 255;
							if (newValue < 0)
								newValue = 0;
							pMask->GetBuffer()[nPixelIndex] = newValue;
						}
					}
				}
			}
		}
	}
	nLayerAffected = bNeedRefresh ? nlayerCount : 0;
	if(bNeedRefresh)
	{
		for (int i=-1;i<nlayerCount;++i)
		{
			DetailTexture* pTexLayer = GetDetail(i);
			if (pTexLayer)
			{
				Texture * pMask = pTexLayer->GetMask();
				if(pMask)
				{
					pMask->UnloadTexture();
				}
			}
		}
	}
	return nLayerAffected;
}

