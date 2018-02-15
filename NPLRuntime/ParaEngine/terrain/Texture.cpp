//-----------------------------------------------------------------------------
// Class: Texture class used in Terrain Engine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Notes: A texture (mask or TextureEntity) in Terrain Engine of ParaEngine
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TextureEntity.h"
#include "Texture.h"
#include "memdebug.h"

using namespace ParaTerrain;
using namespace std;

/** set this flag to debug terrain normal, where a white texture is used.*/
//#define DEBUG_TERRAIN_NORMAL

int Texture::m_DefaultTextureFormat;
int Texture::m_CompressedTextureFormat;
int Texture::m_RgbaTextureFormat;
int Texture::m_AlphaTextureFormat;

extern vector < void* > AllocatedTextures;

Texture::Texture(const uint8 * pBuffer, int width, int height, int stride, int borderSize, bool bClamp, bool useCompression, bool bAlpha)
:m_bIsParaEngineEntity(false), m_TextureFormat(0), m_BytesPerPixel(0)
{
	Init(pBuffer, width, height, stride, borderSize, bClamp, useCompression, bAlpha);
}

Texture::Texture(const uint8 * pBuffer, int width, int height, int stride, int borderSize, bool bClamp, bool useCompression, int bytesPerPixel, int textureFormat)
:m_bIsParaEngineEntity(false)
{
	m_BytesPerPixel = bytesPerPixel;
	m_pBuffer = new uint8[height * width * bytesPerPixel];
	int k = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width * bytesPerPixel; j++, k++)
		{
			m_pBuffer[k] = pBuffer[i * stride * bytesPerPixel + j];
		}
	}
	m_nWidth = width;
	m_nHeight = height;
	m_UseCompression = useCompression;
	m_BorderSize = borderSize;
	m_bClamp = bClamp;
	m_RowLength = stride;
	m_SharedIndex = -1;
	m_szFilename = NULL;
	m_TextureFormat = textureFormat;
	m_BufferPersistent = Settings::GetInstance()->IsEditor();
}

Texture::Texture(TextureEntity* pEntity)
:m_bIsParaEngineEntity(true),m_pBuffer(NULL),m_nWidth(0),m_nHeight(0),m_UseCompression(false),m_bClamp(false),
m_RowLength(0), m_BorderSize(0),m_SharedIndex(0),m_szFilename(NULL), m_TextureFormat(0), m_BytesPerPixel(0)
{
	SetTextureEntity(pEntity);
	m_BufferPersistent = true;
}
Texture::Texture()
:m_bIsParaEngineEntity(false),m_pBuffer(NULL),m_nWidth(0),m_nHeight(0),m_UseCompression(false),m_bClamp(false),
m_RowLength(0), m_BorderSize(0),m_SharedIndex(0),m_szFilename(NULL), m_TextureFormat(0), m_BytesPerPixel(0)
{
	m_BufferPersistent = Settings::GetInstance()->IsEditor();
}

Texture::~Texture()
{
	UnloadTexture();
	
	SAFE_DELETE_ARRAY(m_pBuffer);
	SAFE_DELETE_ARRAY(m_szFilename);
}

void Texture::Init(const uint8 * pBuffer, int width, int height, int rowLength, int borderSize, bool bClamp, bool useCompression, bool bAlpha)
{
	int bytesPerPixel = bAlpha ? 4 : 3;
	m_pBuffer = new uint8[height * width * bytesPerPixel];
	int k = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width * bytesPerPixel; j++, k++)
		{
			m_pBuffer[k] = pBuffer[i * rowLength * bytesPerPixel + j];
		}
	}
	m_BytesPerPixel = bytesPerPixel;
	m_nWidth = width;
	m_nHeight = height;
	m_UseCompression = useCompression;
	m_BorderSize = borderSize;
	m_bClamp = bClamp;
	m_RowLength = rowLength;
	m_SharedIndex = -1;
	m_szFilename = NULL;
	if (bAlpha)
		m_TextureFormat = m_RgbaTextureFormat;
	else
		m_TextureFormat = m_DefaultTextureFormat;
	m_BufferPersistent = Settings::GetInstance()->IsEditor();
}

void Texture::SetBufferPersistent(bool persist)
{
	m_BufferPersistent = persist;
}

int Texture::GetFormat()
{
	return m_TextureFormat;
}

DeviceTexturePtr_type Texture::UploadTexture()
{
	if (!m_TextureID && !m_bIsParaEngineEntity)
	{
		if(m_pBuffer == NULL && m_szFilename!= NULL)
		{
			/// create d3d managed texture
			m_TextureID = TextureEntity::CreateTexture(m_szFilename);
		}
		else if(m_pBuffer != NULL)
		{
			/// create texture using the normal way from memory buffer.
			m_TextureID = TextureEntity::CreateTexture(m_pBuffer, m_nWidth, m_nHeight, m_nWidth, m_BytesPerPixel);
			if (!m_TextureID)
				OUTPUT_LOG("warning: TERRAIN:  failed to create texture \n");

			if (!m_BufferPersistent )
			{
				SAFE_DELETE_ARRAY(m_pBuffer);
			}
		}
	}
	return GetDevicePointer();
}

void Texture::UnloadTexture()
{
	m_TextureID.reset();
}

bool Texture::IsBound()
{
	return (m_TextureID);
}

void Texture::FlipHorizontal()
{
	if(m_pBuffer)
	{
		uint8 *pBufferNew = new uint8[m_nWidth * m_nHeight * m_BytesPerPixel];
		for (int y = 0; y < m_nHeight; y++)
		{
			for (int x = 0; x < m_nWidth; x++)
			{
				int sourceIndex = y * m_nWidth * m_BytesPerPixel + x * m_BytesPerPixel;
				int destIndex = y * m_nWidth * m_BytesPerPixel + ((m_nWidth - x) - 1) * m_BytesPerPixel;
				for (int i = 0; i < m_BytesPerPixel; i++)
					pBufferNew[destIndex + i] = m_pBuffer[sourceIndex + i];
			}
		}
		SAFE_DELETE_ARRAY(m_pBuffer);
		m_pBuffer = pBufferNew;
	}
}

void Texture::FlipVertical()
{
	if(m_pBuffer)
	{
		uint8 *pBufferNew = new uint8[m_nWidth * m_nHeight * m_BytesPerPixel];
		for (int y = 0; y < m_nHeight; y++)
		{
			for (int x = 0; x < m_nWidth; x++)
			{
				int sourceIndex = y * m_nWidth * m_BytesPerPixel + x * m_BytesPerPixel;
				int destIndex = ((m_nHeight - y) - 1) * m_nWidth * m_BytesPerPixel + x * m_BytesPerPixel;
				for (int i = 0; i < m_BytesPerPixel; i++)
					pBufferNew[destIndex + i] = m_pBuffer[sourceIndex + i];
			}
		}
		SAFE_DELETE_ARRAY(m_pBuffer);
		m_pBuffer = pBufferNew;
	}
}

int Texture::GetBitsPerPixel()
{
	return m_BytesPerPixel * 8;
}
int Texture::GetBytesPerPixel()
{
	return m_BytesPerPixel;
}
void Texture::SetSharedIndex(int index)
{
	m_SharedIndex = index;
}

int Texture::GetSharedIndex()
{
	return m_SharedIndex;
}

uint8 *Texture::GetBuffer()
{
	return m_pBuffer;
}

int Texture::GetWidth()
{
	return m_nWidth;
}

void Texture::SetWidth( int nWidth )
{
	if(m_nWidth != nWidth && nWidth > 0)
	{
		if(m_pBuffer)
		{
			// a very basic resizing algorithm. 
			float fScale = (float)m_nWidth / (float)nWidth;
			uint8 *pBufferNew = new uint8[nWidth * nWidth * m_BytesPerPixel];
			for (int y = 0; y < nWidth; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					int sourceIndex = ((int)(y * fScale)) * m_nWidth * m_BytesPerPixel + ((int)(x * fScale)) * m_BytesPerPixel;
					int destIndex = y * nWidth * m_BytesPerPixel + x * m_BytesPerPixel;
					for (int i = 0; i < m_BytesPerPixel; i++)
						pBufferNew[destIndex + i] = m_pBuffer[sourceIndex + i];
				}
			}
			SAFE_DELETE_ARRAY(m_pBuffer);
			m_pBuffer = pBufferNew;
		}

		m_nWidth = nWidth;
		m_nHeight = m_nWidth;
	}
}

int Texture::GetHeight()
{
	return m_nHeight;
}

void Texture::SetFilename(const char *szFilename)
{
	SAFE_DELETE_ARRAY(m_szFilename);
	m_szFilename = new char[strlen(szFilename) + 1];
	strcpy(m_szFilename, szFilename);
}

bool Texture::IsClamped()
{
	return m_bClamp;
}

bool Texture::UseCompression()
{
	return m_UseCompression;
}

int Texture::GetBorderSize()
{
	return m_BorderSize;
}

bool Texture::IsEqual(ParaEngine::TextureEntity* pEntity)
{
	return (m_bIsParaEngineEntity && m_TextureID.get() == pEntity);
}

const char *Texture::GetFilename()
{
	return m_szFilename;
}

void Texture::SetDefaultTextureFormat(int format)
{
	m_DefaultTextureFormat = format;
}

void Texture::SetAlphaTextureFormat(int format)
{
	m_AlphaTextureFormat = format;
}

void Texture::SetRgbaTextureFormat(int format)
{
	m_RgbaTextureFormat = format;
}

void Texture::SetCompressedTextureFormat(int format)
{
	m_CompressedTextureFormat = format;
}
TextureEntity* Texture::GetTextureEntity()
{
	if(m_bIsParaEngineEntity)
		return m_TextureID.get();
	else
		return NULL;
}
bool Texture::SetTextureEntity(TextureEntity* texture)
{
	if(m_bIsParaEngineEntity)
	{
		m_TextureID = texture;
		return true;
	}
	return false;
}

DeviceTexturePtr_type ParaTerrain::Texture::GetDevicePointer()
{
	return (m_TextureID) ? m_TextureID->GetTexture() : 0;
}
