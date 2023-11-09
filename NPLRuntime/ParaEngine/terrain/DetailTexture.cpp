// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "FileManager.h"
#include "TextureEntity.h"
#include "DetailTexture.h"
#include "memdebug.h"
using namespace ParaTerrain;
using namespace ParaEngine;

DetailTexture::DetailTexture(Texture * pTexture, Texture * pMask)
{
	m_bOwnTexture = false;
	m_bOwnMask = true;
	m_pMask = NULL;
	m_pTexture = NULL;
	SetMask(pMask);
	SetTexture(pTexture);
}

DetailTexture::DetailTexture(Texture * pTexture,unsigned char DefaultMaskValue)
{
	m_bOwnTexture = false;
	m_bOwnMask = true;
	m_pMask = NULL;
	m_pTexture = NULL;
	if (pTexture)
	{
		RegenerateMask(DefaultMaskValue);
		SetTexture(pTexture);
	}
}

Texture* DetailTexture::RegenerateMask(unsigned char DefaultMaskValue)
{
	int textureMaskWidth = (m_pMask && m_pMask->GetWidth()>0) ? m_pMask->GetWidth() : Settings::GetInstance()->GetTextureMaskWidth();
	int textureMaskHeight = textureMaskWidth;
	uint8 *pBuffer = new uint8[textureMaskWidth * textureMaskHeight];
	memset(pBuffer, DefaultMaskValue, textureMaskWidth * textureMaskHeight);
	Texture *pMask = new Texture(pBuffer, textureMaskWidth, textureMaskHeight, textureMaskWidth, 0, true, false, 1, Texture::m_AlphaTextureFormat);
	delete[]pBuffer;
	SetMask(pMask);
	return GetMask();
}

DetailTexture::DetailTexture(CParaFile& file,Terrain * pTerrain)
{
	m_bOwnTexture = false;
	m_bOwnMask = true;
	m_pMask = NULL;
	m_pTexture = NULL;
	ReadMask(file, pTerrain);
}

DetailTexture::~DetailTexture()
{
	if(m_bOwnMask)
	{
		SAFE_DELETE(m_pMask);
	}

	if(m_bOwnTexture)
	{
		SAFE_DELETE(m_pTexture);
	}
}

void DetailTexture::FlipHorizontal()
{
	if(m_pMask)
		m_pMask->FlipHorizontal();
	if(m_pTexture)
		m_pTexture->FlipHorizontal();
}

void DetailTexture::FlipVertical()
{
	if(m_pMask)
		m_pMask->FlipVertical();
	if(m_pTexture)
		m_pTexture->FlipVertical();
}

void DetailTexture::SetMask(Texture * pTexture)
{
	if(m_pMask != pTexture)
	{
		if(m_bOwnMask)
		{
			SAFE_DELETE(m_pMask);
		}
		m_pMask = pTexture;
		m_pMask->SetBufferPersistent(Settings::GetInstance()->IsEditor());
	}
}

Texture *DetailTexture::GetMask()
{
	return m_pMask;
}

void DetailTexture::SetTexture(Texture * pTexture)
{
	if(m_pTexture != pTexture)
	{
		if(m_bOwnTexture)
		{
			SAFE_DELETE(m_pTexture);
		}
		m_pTexture = pTexture;
	}
}

Texture *DetailTexture::GetTexture()
{
	return m_pTexture;
}

DeviceTexturePtr_type DetailTexture::BindMask()
{
	if(m_pMask)
		return m_pMask->UploadTexture();
	else
		return 0;
}

DeviceTexturePtr_type DetailTexture::BindTexture()
{
	if(m_pTexture)
		return m_pTexture->UploadTexture();
	else
		return 0;
}

void DetailTexture::Unbind()
{
	if(m_pMask)
		m_pMask->UnloadTexture();
	if(m_pTexture)
		m_pTexture->UnloadTexture();
}

#define INVALID_DETAIL_TEXTURE	0xffffff00

#define MAX_BUFFER	65544
static unsigned char g_output[MAX_BUFFER];

/**
* @param input: if this is NULL, a white mask will be written. 
*/
void EncodeMaskBuffer(const unsigned char* input, int nInputSize, unsigned char** output, int* nOutbutSize)
{
	bool bCanEncode = true;
	unsigned char c = 0xff;
	if(input!=NULL)
	{
		c = input[0];
		for (int i=0; bCanEncode && i<nInputSize; ++i)
		{
			if(c != input[i])
				bCanEncode = false;
		}
	}

	if(bCanEncode)
	{
		//*(int*)(g_output) = 1;
		//*((int*)(g_output)+1) = nInputSize;
		int num = 1;
		memcpy(g_output, &num, sizeof(int));
		memcpy(g_output + sizeof(int), &nInputSize, sizeof(int));

		g_output[8] = c;

		*nOutbutSize = 1+8;
		*output = g_output;
	}
	else
	{
		if(nInputSize>MAX_BUFFER)
		{
			OUTPUT_LOG("EncodeMaskBuffer can not decode larger than 4096 bytes\r\n");
			*nOutbutSize = 0;
			return;
		}
		//*(int*)(g_output) = nInputSize;
		//*((int*)(g_output)+1) = nInputSize;
		memcpy(g_output, &nInputSize, sizeof(int));
		memcpy(g_output + sizeof(int), &nInputSize, sizeof(int));

		memcpy(&(g_output[8]), input, nInputSize);

		*nOutbutSize = nInputSize+8;
		*output = g_output;
	}
}

/**
* @param input: 
* @param nInputRead: number of unsigned char read from input during decoding. 
* @param 
* @param 
*/
void DecodeMaskBuffer(const unsigned char* input, int* nInputRead, unsigned char** output, int* nOutbufSize)
{
	//int nByteCount = *(int*)(input);
	//int nOutSize = *((int*)(input)+1);

	int nByteCount;
	memcpy(&nByteCount, input, sizeof(int));
	int nOutSize;
	memcpy(&nOutSize, input + sizeof(int), sizeof(int));

	if(nByteCount==1)
	{
		if(input[8] == 0xff)
		{
			// this is a default mask with all pixel color 0xff, so do not copy data. 
			*nOutbufSize = 0;
			*output = g_output;
			*nInputRead = nByteCount +8;
		}
		else
		{
			memset(&(g_output[0]), input[8], nOutSize);
			*nOutbufSize = nOutSize;
			*output = g_output;
			*nInputRead = nByteCount +8;	
		}
	}
	else
	{
		memcpy(&(g_output[0]), &(input[8]), nOutSize);
		*nOutbufSize = nOutSize;
		*output = g_output;
		*nInputRead = nByteCount +8;
	}
}

void DetailTexture::WriteMask(CParaFile& file, Terrain * pTerrain)
{
	if(m_pTexture==0)
	{
		file.WriteDWORD(INVALID_DETAIL_TEXTURE);
		return;
	}
	int nIndex = m_pTexture->GetSharedIndex();
	DWORD dwHeader = nIndex;
	// make the high bits 01,so that we know that the version number is 01. 
	dwHeader &= 0x00ffffff;  dwHeader |= 0x01000000;  
	
	file.WriteDWORD(dwHeader);
	
	int textureMaskWidth = (m_pMask && m_pMask->GetWidth()>0) ? m_pMask->GetWidth() : Settings::GetInstance()->GetTextureMaskWidth();
	int textureMaskHeight = textureMaskWidth;
	
	int nSize = textureMaskWidth*textureMaskHeight;
	// write the texture width. 
	file.WriteDWORD(textureMaskWidth);

	if(nIndex <0 )
	{
		unsigned char* output = NULL;
		int nOutputSize = 0;
		if(m_pMask != 0)
		{
			EncodeMaskBuffer(m_pMask->GetBuffer(), nSize, &output, &nOutputSize);
		}
		else
		{
			// encode white mask.
			EncodeMaskBuffer(NULL, nSize, &output, &nOutputSize);
		}
		
		if(output)
		{
			file.write(output, nOutputSize);
		}
		else
		{
			OUTPUT_LOG("failed writing detail texture to file.\r\n");
		}
	}
	else
	{
		file.write(m_pMask->GetBuffer(), nSize);
	}
}
void DetailTexture::ReadMask(CParaFile& file, Terrain * pTerrain)
{
	DWORD dwHeader = file.ReadDWORD();
	
	if(dwHeader == INVALID_DETAIL_TEXTURE)
	{
		if(m_bOwnMask)
		{
			SAFE_DELETE(m_pMask);
		}
		else
			m_bOwnMask = false;
		
		if(m_bOwnTexture)
		{
			SAFE_DELETE(m_pTexture);
		}
		else
			m_bOwnTexture = false;

		return;
	}
	
	DWORD dwVersion = dwHeader >> 24;
	int nSharedIndex = INVALID_DETAIL_TEXTURE;
	int textureMaskWidth = 0;
	int textureMaskHeight = 0;
	if(dwVersion == 1)
	{
		// for version 1. the next DWORD is the mask width. 
		dwHeader &= 0x00ffffff;
		if(dwHeader > 256){
			dwHeader |= 0xff000000;
		}
		nSharedIndex = (int)dwHeader;
		textureMaskWidth = (int)(file.ReadDWORD());
		textureMaskHeight = textureMaskWidth;
	}
	else
	{
		// old version
		nSharedIndex = (int)dwHeader;
	}

	Texture* pTexture = NULL;
	if(nSharedIndex != -1)
	{
		// if this is not the base layer, we will set the texture as well. 
		pTexture = pTerrain->GetTextureSet()->GetTexture(nSharedIndex);
		SetTexture(pTexture);
	}

	if (nSharedIndex==-1 || pTexture!=NULL)
	{
		// read mask file from file. 
		unsigned char * MaskBuf = (unsigned char*)file.getPointer();
		
		if(textureMaskWidth == 0)
		{
			// this is only for back-compatible, should never be executed for latest mask file format.
			textureMaskWidth = 256; // Settings::GetInstance()->GetTextureMaskWidth()
			textureMaskHeight = textureMaskWidth;
		}

		int nCompressedSize = textureMaskWidth*textureMaskHeight;
		bool bCreateMask = true;
		if(nSharedIndex==-1)
		{
			int nSize=0;
			DecodeMaskBuffer((const unsigned char*)file.getPointer(), &nCompressedSize, &MaskBuf, &nSize);
			if(nSize == 0)
			{
				// For white mask file, do not create the mask.
				SetMask(NULL);
				bCreateMask = false;
			}
		}
		if(bCreateMask)
		{
			Texture *pMask = new Texture((const uint8*)MaskBuf, textureMaskWidth, textureMaskHeight, textureMaskWidth, 0, true, false, 1, Texture::m_AlphaTextureFormat);
			if(textureMaskWidth > Settings::GetInstance()->GetTextureMaskWidth())
			{
				// use smaller width. 
				pMask->SetWidth(Settings::GetInstance()->GetTextureMaskWidth());
			}
			SetMask(pMask);
		}
		file.seekRelative(nCompressedSize);
	}
}
