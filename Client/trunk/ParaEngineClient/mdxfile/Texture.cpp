//----------------------------------------------------------------------
// Class:	CTexture
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03

#include "dxStdAfx.h"
#include "ParaEngine.h"
#include "..\ParaWorldAsset.h"
#include "texture.h"
#include "imageUtils.h"

using namespace ParaEngine;

CTexture::~CTexture(void)
{
	Cleanup();
}
void CTexture::Cleanup()
{
	//release textures
	vector< MyD3DTexture* >::iterator itCurCP, itEndCP = m_pTextures.end();

	for( itCurCP = m_pTextures.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		delete (*itCurCP);
	}
	m_pTextures.clear();
}

//---------------------------------------------------------------------------
// Added by LiXizhi
/// some MDX file load texture by specifying replaceable ID. 
/// we will use a simple hash set to store <ID, textureFilePath>, there can be 50 pairs
/// For simplicity, I will use a fixed mapping through out the game
/// replaceable bitmap are shared bitmaps. However, I do not use shared bitmap in Para engine.
/// so in the ReplaceIDWithName(), The team color and glow are simply set to NULL texture.
/// return true if found
//---------------------------------------------------------------------------
bool ReplaceIDWithName(int rID, char* file)
{
	switch(rID)
	{
	case 1:
		//use ""since we will set it in the engine, lstrcpy(file, "ReplaceableTextures\\TeamColor\\TeamColor00.blp");
		lstrcpy(file, "");
		break;
	case 2:
		lstrcpy(file, "ReplaceableTextures\\TeamGlow\\TeamGlow00.blp");
		break;
	case 11:
		lstrcpy(file, "ReplaceableTextures\\Cliff\\Cliff0.blp");
		break;
	case 31:
		lstrcpy(file, "ReplaceableTextures\\LordaeronTree\\LordaeronSummerTree.blp");
		break;
	case 32:
		lstrcpy(file, "ReplaceableTextures\\AshenvaleTree\\AshenTree.blp");
		break;
	case 33:
		lstrcpy(file, "ReplaceableTextures\\BarrensTree\\BarrensTree.blp");
		break;
	case 34:
		lstrcpy(file, "ReplaceableTextures\\NorthrendTree\\NorthTree.blp");
		break;
	case 35:
		lstrcpy(file, "ReplaceableTextures\\Mushroom\\MushroomTree.blp");
		break;
	default:
		return false;
	}
	return true;
}

/// one time read, each model can only call this once
void CTexture::Read(TypePointer inP,int inSize)
{
	/// one time call, any previous call to Read(...)  will be overridden
	Cleanup(); 

	TypePointer p(inP);

	/// read the chunks
	int numBitmaps = inSize / sizeof(MyD3DTexture::TextureBitmap);
	for(int i=0; i<numBitmaps; i++)
	{
		CreateTexture(p.c+i*sizeof(MyD3DTexture::TextureBitmap), sizeof(MyD3DTexture::TextureBitmap));
	}
}

MyD3DTexture* CTexture::CreateTexture(const char * buffer, DWORD nBufferSize)
{
	MyD3DTexture* pTexture = new MyD3DTexture();
	memset(pTexture, 0, sizeof(MyD3DTexture));
	m_pTextures.push_back(pTexture);

	memcpy(&(pTexture->bitmap), buffer, nBufferSize);

	// replaceable ID is mapped to file path
	if(pTexture->bitmap.replaceableID != 0)
		ReplaceIDWithName(pTexture->bitmap.replaceableID, pTexture->bitmap.texturePath);

	if( ((lstrcmpi(pTexture->bitmap.texturePath, "")==0)||(lstrcmpi(pTexture->bitmap.texturePath, " ")==0)))
	{
		pTexture->pTextureEntity.reset();
		return pTexture;
	}
	pTexture->pTextureEntity = CGlobals::GetAssetManager()->LoadTexture("", pTexture->bitmap.texturePath, TextureEntity::BlpTexture);

	return pTexture;
}

MyD3DTexture* CTexture::CreateTexture(const char* filename)
{
	MyD3DTexture* pTexture = new MyD3DTexture();
	memset(pTexture, 0, sizeof(MyD3DTexture));
	m_pTextures.push_back(pTexture);

	strcpy(pTexture->bitmap.texturePath, filename);
	pTexture->pTextureEntity = CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture);

	return pTexture;
}

void CTexture::InitDeviceObjects()
{
	//for(int i=0; i<(int)m_pTextures.size() ; ++i)
	//{
	//	int nLen = 0;
	//	if(m_pTextures[i]->bitmap.texturePath)
	//		nLen = (int)strlen(m_pTextures[i]->bitmap.texturePath);

	//	if(nLen>2 && (strcmp(m_pTextures[i]->bitmap.texturePath+nLen-3, "blp")!=0))
	//	{
	//		D3DXCreateTextureFromFile(CGlobals::GetRenderDevice(), m_pTextures[i]->bitmap.texturePath, &(m_pTextures[i]->pD3dTextures));
	//	}
	//	else
	//	{
	//		LoadBLP(m_pTextures[i]->bitmap.texturePath, m_pTextures[i]->width,m_pTextures[i]->height,
	//			m_pTextures[i]->type,m_pTextures[i]->subtype, &(m_pTextures[i]->pD3dTextures),m_pTextures[i]->bitmap.replaceableID);
	//	}
	//}
}
void CTexture::DeleteDeviceObjects()
{
	/*for(int i=0; i<(int)m_pTextures.size(); ++i)
	{
		SAFE_RELEASE(m_pTextures[i]->pD3dTextures);
	}*/
}

LPDIRECT3DTEXTURE9 CTexture::GetBindTexture(int index)
{
	if((index <(int)m_pTextures.size())&&(index >= 0))
	{
		if(m_pTextures[index]->pTextureEntity)
			return m_pTextures[index]->pTextureEntity->GetTexture();
	}
	return NULL;
}

