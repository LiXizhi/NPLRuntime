//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2006.8
// Revised: 
// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "ParaWorldAsset.h"
#include "TextureSet.h"
#include "GlobalTerrain.h"
#include "memdebug.h"
using namespace ParaTerrain;
using namespace std;

TextureSet::TextureSet()
{
}

TextureSet::~TextureSet()
{
	std::vector < Texture * >::const_iterator itCur, itEnd = m_Textures.end();
	for (itCur = m_Textures.begin(); itCur != itEnd; ++itCur)
	{
		delete (*itCur);
	}
	m_Textures.clear();
}

int TextureSet::AddTexture(Texture * pTex)
{
	m_Textures.push_back(pTex);
	pTex->SetSharedIndex((int)m_Textures.size() - 1);
	return (int)m_Textures.size() - 1;
}

int TextureSet::FindTexture(ParaEngine::TextureEntity* pTexture)
{
	std::vector < Texture * >::const_iterator itCur, itEnd = m_Textures.end();
	for (itCur = m_Textures.begin(); itCur != itEnd; ++itCur)
	{
		if((*itCur)->IsEqual(pTexture))
		{
			return (*itCur)->GetSharedIndex();
		}
	}
	return -1;
}

Texture *TextureSet::GetTexture(int index)
{
	return (index>=0 && index<(int)(m_Textures.size())) ? m_Textures[index] : NULL;
}

bool TextureSet::ReplaceTexture(int nIndex, ParaEngine::TextureEntity* TextureAsset)
{
	if(nIndex<GetNumTextures())
	{
		Texture * pTex = GetTexture(nIndex);
		if(pTex)
		{
			if(TextureAsset!=0)
			{
				int detailTextureIndex = FindTexture(TextureAsset);
				if(detailTextureIndex<0)
				{
					// only replace if there is the given texture is not added before. 
					return pTex->SetTextureEntity(TextureAsset);
				}
				else
					return false;
			}
		}
	}
	else
	{
		// we will add this texture if it nIndex is bigger than total number of textures. 
		int detailTextureIndex = FindTexture(TextureAsset);
		if(detailTextureIndex<0)
		{
			detailTextureIndex = AddTexture(new Texture(TextureAsset));
		}
	}
	return false;
}

int TextureSet::GetNumTextures()
{
	return (int)m_Textures.size();
}

int TextureSet::UnbindAllTextures()
{
	std::vector < Texture * >::const_iterator itCur, itEnd = m_Textures.end();
	for (itCur = m_Textures.begin(); itCur != itEnd; ++itCur)
	{
		(*itCur)->UnloadTexture();
	}
	return S_OK;
}

void TextureSet::WriteMask(CParaFile& file, Terrain * pTerrain)
{
	int size = (int)m_Textures.size();
	file.WriteDWORD((DWORD)size);
	string sTextureFile;
	for (int i = 0; i < size; ++i)
	{
		TextureEntity * pTex = m_Textures[i]->GetTextureEntity();
		if(pTex!=0)
		{
			sTextureFile = pTex->GetKey();
			if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
			{
				CPathReplaceables::GetSingleton().EncodePath(sTextureFile, sTextureFile);
			}
			int nStrSize = (int)sTextureFile.size();
			file.WriteDWORD(nStrSize);
			file.write(sTextureFile.c_str(), nStrSize);
		}
		else
			file.WriteDWORD(0);
	}
}
void TextureSet::ReadMask(CParaFile& file, Terrain * pTerrain)
{
	int size = file.ReadDWORD();
	
	char buf[MAX_LINE];
	string sTextureFile;
	for (int i = 0; i < size; ++i)
	{
		int nStrSize = file.ReadDWORD();

		PE_ASSERT(nStrSize<MAX_LINE);
		file.read(buf, nStrSize);
		buf[nStrSize] = '\0';
		CPathReplaceables::GetSingleton().DecodePath(sTextureFile, buf);
		TextureEntity* pTexEntity =  CGlobals::GetAssetManager()->LoadTexture("", sTextureFile.c_str(), TextureEntity::StaticTexture);
		int detailTextureIndex = FindTexture(pTexEntity);
		if(detailTextureIndex<0)
		{
			Texture *pTex = new Texture(pTexEntity);
			AddTexture(pTex);
		}
	}
}

void TextureSet::GarbageCollect(Terrain * pTerrain)
{
	int nNumTextures = GetNumTextures();
	if(nNumTextures==0)
		return;
	int nNumOfCells = pTerrain->m_NumberOfTextureTiles;
	if(nNumOfCells == 0)
		return;

	vector<int> indices;
	const int nUnused_ = 0xffff;
	indices.resize(nNumTextures, nUnused_);

	// find unused textures
	int i;
	for (i = 0; i < nNumOfCells; ++i)
	{
		TextureCell *pCell = pTerrain->GetTextureCell(i);
		pCell->OptimizeLayer();
		int nNumOfDetails = pCell->GetNumberOfDetails();
		if(nNumOfDetails>0)
		{
			for (int j=0;j<nNumOfDetails;++j)
			{
				DetailTexture* pDetailTex = pCell->GetDetail(j);
				if(pDetailTex!=NULL)
				{
					int nIndex = pDetailTex->GetTexture()->GetSharedIndex();
					if(nIndex>=0 && nIndex <nNumTextures)
						indices[nIndex] = 0;
				}
			}
		}
	}
	// pre-build shared index 
	int nNewIndex = 0;
	for (i=0;i<nNumTextures;++i)
	{
		if(indices[i] != nUnused_){
			indices[i] = nNewIndex++;
		}
	}
	// remove all unused textures and update shared index in the texture.
	vector<Texture*>::iterator Iter;
	i=0;
	for ( Iter = m_Textures.begin( ) ; Iter != m_Textures.end( ) ; ++i)
	{
		if(indices[i] == nUnused_){
			delete (*Iter);
			Iter = m_Textures.erase(Iter);
		}
		else
		{
			(*Iter)->SetSharedIndex(indices[i]);
			++Iter;
		}
	}
}

void ParaTerrain::TextureSet::RemoveTexture( int nIndex, Terrain * pTerrain, bool bNormalizeOthers )
{
	if(pTerrain == 0)
		return;
	int nNumTextures = GetNumTextures();
	if(nNumTextures==0)
		return;
	int nNumOfCells = pTerrain->m_NumberOfTextureTiles;
	if(nNumOfCells == 0)
		return;

	if(bNormalizeOthers)
	{
		for (int i = 0; i < nNumOfCells; ++i)
		{
			TextureCell *pCell = pTerrain->GetTextureCell(i);

			int nNumOfDetails = pCell->GetNumberOfDetails();
			if(nNumOfDetails>0)
			{
				for (int j=0;j<nNumOfDetails;++j)
				{
					DetailTexture* pDetailTex = pCell->GetDetail(j);
					if(pDetailTex!=NULL)
					{
						int index_ = pDetailTex->GetTexture()->GetSharedIndex();
						if(index_ == nIndex)
						{
							// remove this one. 
							pCell->NormalizeMask(j, 0.f);
							break;
						}
					}
				}
			}
		}

	}

	// find unused textures
	int i;
	for (i = 0; i < nNumOfCells; ++i)
	{
		TextureCell *pCell = pTerrain->GetTextureCell(i);
		int nNumOfDetails = pCell->GetNumberOfDetails();
		if(nNumOfDetails>0)
		{
			for (int j=0;j<nNumOfDetails;++j)
			{
				DetailTexture* pDetailTex = pCell->GetDetail(j);
				if(pDetailTex!=NULL)
				{
					int index_ = pDetailTex->GetTexture()->GetSharedIndex();
					if(index_ == nIndex)
					{
						// remove this one. 
						pCell->RemoveDetail(j);
						break;
					}
				}
			}
		}
	}

	// remove all the texture and update shared index in the texture.
	vector<Texture*>::iterator Iter;
	i=0;
	int nNewIndex = 0;
	for ( Iter = m_Textures.begin( ) ; Iter != m_Textures.end( ); ++i)
	{
		if(i == nIndex)
		{
			delete (*Iter);
			Iter = m_Textures.erase(Iter);
		}
		else
		{
			(*Iter)->SetSharedIndex(nNewIndex++);
			++Iter;
		}
	}
}
