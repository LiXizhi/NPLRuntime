//-----------------------------------------------------------------------------
// Class:	CGlobalTerrain
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.3.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Terrain.h"

#include "Loader.h"
#include "TerrainLattice.h"
#include "DynamicTerrainLoader.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "GlobalTerrain.h"
#include "ParaWorldAsset.h"
#include "OceanManager.h"
#include "BlockEngine/BlockWorldClient.h"
#include "memdebug.h"

using namespace ParaTerrain;
using namespace ParaEngine;

/**@def the largest radius during terrain morphing. (RATIO*Terrain_Size) */
#define MAX_TERRAIN_MORPH_RADIUS_RATIO	0.5f

/**@def whether to validate the brush size when applying terrain heightfield modification. if true, the brush size must be smaller than MAX_TERRAIN_MORPH_RADIUS_RATIO times Terrain size */
// #define VALIDATE_BRUSHSIZE 

namespace ParaTerrain
{
	Matrix4		g_matTerrainModelview = Matrix4::IDENTITY;
	Matrix4		g_matTerrainProjection = Matrix4::IDENTITY;
	Matrix4		g_matTerrainViewProj;
	const float		CGlobalTerrain::g_fTerrainInvisibleHeight = FLOAT_POS_INFINITY;
}

CGlobalTerrain::CGlobalTerrain(void)
	:m_bRenderTerrain(true), m_nTerrainType(SINGLE_TERRAIN), m_pTerrainLoader(NULL), m_pTerrainSingle(NULL), m_pTerrainLattice(NULL), m_fDefaultHeight(0.f), m_bEnablePathEncoding(true), m_useGeoMipmap(false),
	m_enableSlopeCollision(false), m_isEditorMode(false), m_bEnableTerrainEngine(true)
{
}

CGlobalTerrain::~CGlobalTerrain(void)
{
	Cleanup();
}

void CGlobalTerrain::Cleanup()
{
	SAFE_DELETE(m_pTerrainSingle);
	SAFE_DELETE(m_pTerrainLattice);
	SAFE_DELETE(m_pTerrainLoader);
}
Settings* CGlobalTerrain::GetSettings()
{
	return Settings::GetInstance();
}
void CGlobalTerrain::CreateTerrain(const char * pConfigFileName)
{
	try
	{
		string sConfigFileName;
		CPathReplaceables::GetSingleton().DecodePath(sConfigFileName, pConfigFileName);
		Cleanup();
		CGlobals::GetOceanManager()->CleanupTerrainCache();

		CParaFile cFile;
		cFile.OpenAssetFile(sConfigFileName.c_str());
		if (cFile.isEof())
		{
			OUTPUT_LOG("error: invalid terrain config file %s.\n", sConfigFileName.c_str());
			return;
		}

		string filetype;
		if (cFile.GetNextAttribute("type", filetype) && (filetype == "lattice"))
		{
			/// create latticed terrain
			cFile.close();
			m_pTerrainLoader = new CDynamicTerrainLoader(sConfigFileName.c_str());
			if (IsTerrainEngineEnabled())
			{
				m_pTerrainLattice = new TerrainLattice(m_pTerrainLoader, m_useGeoMipmap);
			}
			else
			{
				OUTPUT_LOG("Global terrain engine disabled hence %s is ignored.\n", pConfigFileName);
			}
			m_nTerrainType = LATTICED_TERRAIN;
		}
		else
		{
			/// create single terrain
			cFile.close();
			m_pTerrainSingle = new Terrain(MAX_NUM_VISIBLE_TERRAIN_TRIANGLES, 0.0f, 0.0f);
			m_pTerrainSingle->LoadFromConfigFile(sConfigFileName.c_str());
			m_nTerrainType = SINGLE_TERRAIN;
			m_pTerrainSingle->OnLoad();
		}
	}
	catch (...)
	{
		Cleanup();
		OUTPUT_LOG("Terrain creation failed.\n");
	}
}
bool CGlobalTerrain::IsTerrainEngineEnabled() const
{
	return m_bEnableTerrainEngine;
}

void CGlobalTerrain::EnableTerrainEngine(bool val)
{
	m_bEnableTerrainEngine = val;
}

bool CGlobalTerrain::TerrainRenderingEnabled()
{
	return m_bRenderTerrain;
}

void CGlobalTerrain::EnableTerrainRendering(bool bEnable)
{
	m_bRenderTerrain = bEnable;
}
void CGlobalTerrain::CreateSingleTerrain(const char* sHeightmapfile,
	float fTerrainSize, float fElevscale, int bSwapvertical,
	const char*  sMainTextureFile, const char* sCommonTextureFile,
	int nMaxBlockSize, float fDetailThreshold)
{
	try
	{
		Cleanup();
		CGlobals::GetOceanManager()->CleanupTerrainCache();
		m_pTerrainSingle = new Terrain(MAX_NUM_VISIBLE_TERRAIN_TRIANGLES, 0.0f, 0.0f);
		ParaTerrain::Loader::GetInstance()->LoadElevations(m_pTerrainSingle, sHeightmapfile, fTerrainSize*OBJ_UNIT, fElevscale*OBJ_UNIT, bSwapvertical > 0);
		m_pTerrainSingle->SetBaseTexture(sMainTextureFile);
		m_pTerrainSingle->SetCommonTexture(sCommonTextureFile);
		m_pTerrainSingle->SetMaximumVisibleBlockSize(nMaxBlockSize);
		m_pTerrainSingle->SetDetailThreshold(fDetailThreshold);
		//m_pTerrainSingle->LoadDetailedTextures(sMainTextureFile);

		m_nTerrainType = SINGLE_TERRAIN;
	}
	catch (...)
	{
		Cleanup();
	}
}

void CGlobalTerrain::SetCameraPosition(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			m_pTerrainLattice->SetCameraPosition(x, y, 0);
	}
}
void CGlobalTerrain::ModelViewMatrixChanged()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice){
			m_pTerrainLattice->ModelViewMatrixChanged();
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle){
			m_pTerrainSingle->ModelViewMatrixChanged();
		}
	}
}
void CGlobalTerrain::UpdateHoles(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice){
			m_pTerrainLattice->UpdateHoles(x, y);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle){
			m_pTerrainSingle->UpdateHoles();
		}
	}
}

bool CGlobalTerrain::IsHole(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice){
			return m_pTerrainLattice->IsHole(x, y);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle){
			return m_pTerrainSingle->IsHoleW(x, y);
		}
	}
	return false;
}

void CGlobalTerrain::SetHole(float x, float y, bool bIsHold)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice){
			return m_pTerrainLattice->SetHole(x, y, bIsHold);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle){
			return m_pTerrainSingle->SetHole(x, y, bIsHold);
		}
	}
}

void CGlobalTerrain::GetBoundingBox(CShapeBox * BBox)
{
	if (TerrainRenderingEnabled())
	{
		if (m_nTerrainType == LATTICED_TERRAIN)
		{
			if (m_pTerrainLattice){
				*BBox = m_pTerrainLattice->GetBoundingBox();
			}
		}
		else if (m_nTerrainType == SINGLE_TERRAIN)
		{
			if (m_pTerrainSingle){
				*BBox = m_pTerrainSingle->GetBoundingBoxW();
			}
		}
	}
}

void CGlobalTerrain::GetBoundingBoxes(vector<ParaEngine::CShapeAABB>& boxes, const Matrix4* modelView, const ParaEngine::CShapeFrustum& frustum, int nMaxBoxesNum/*=1024*/, int nSmallestBoxStride/*=4*/)
{
	if (TerrainRenderingEnabled())
	{
		if (m_nTerrainType == LATTICED_TERRAIN)
		{
			if (m_pTerrainLattice){
				m_pTerrainLattice->GetBoundingBoxes(boxes, modelView, frustum, nMaxBoxesNum, nSmallestBoxStride);
			}
		}
		else if (m_nTerrainType == SINGLE_TERRAIN)
		{
			if (m_pTerrainSingle){
				m_pTerrainSingle->GetBoundingBoxes(boxes, modelView, frustum, nMaxBoxesNum, nSmallestBoxStride);
			}
		}
	}
}

void CGlobalTerrain::SnapPointToVertexGrid(float x, float y, float* vertex_x, float* vertex_y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			m_pTerrainLattice->SnapPointToVertexGrid(x, y);
			*vertex_x = x;
			*vertex_y = y;
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		*vertex_x = x;
		*vertex_y = y;
	}
}
float CGlobalTerrain::GetVertexSpacing() const
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice){
			return m_pTerrainLattice->GetVertexSpacing();
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle){
			return m_pTerrainSingle->GetVertexSpacing();
		}
	}
	// return default value.
	return 533.3333f / 128.f;
}

Terrain * CGlobalTerrain::GetTerrainAtPoint(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetTerrainAtPoint(x, y);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		return m_pTerrainSingle;
	}
	return NULL;
}

void CGlobalTerrain::GetTexturesInCell(float x, float y, std::vector<int>& indices)
{
	Terrain * pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain)
	{
		TextureCell * pCell = pTerrain->GetTextureCellW(x, y);
		if (pCell)
		{
			indices.resize(pCell->GetNumberOfDetails(), 0);
			for (int i = 0; i < pCell->GetNumberOfDetails(); i++)
			{
				indices[i] = pCell->GetDetail(i)->GetTexture()->GetSharedIndex();
			}
		}
	}
}

bool CGlobalTerrain::RemoveTextureInCell(float x, float y, int nIndex)
{
	Terrain * pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain)
	{
		TextureCell * pCell = pTerrain->GetTextureCellW(x, y);
		if (pCell)
		{
			for (int i = 0; i < pCell->GetNumberOfDetails(); i++)
			{
				if (pCell->GetDetail(i)->GetTexture()->GetSharedIndex() == nIndex)
				{
					// remove this texture
					pCell->RemoveDetail(i);
					pCell->NormalizeMask(-1);
				}
			}
		}
	}
	return true;
}

int CGlobalTerrain::GetTextureCount(float x, float y)
{
	Terrain * pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain)
	{
		if (pTerrain->GetTextureSet())
		{
			return pTerrain->GetTextureSet()->GetNumTextures();
		}
	}
	return 0;
}

TextureEntity* CGlobalTerrain::GetTexture(float x, float y, int nIndex)
{
	Terrain * pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain)
	{
		if (nIndex >= 0)
		{
			if (pTerrain->GetTextureSet())
			{
				Texture * pTex = pTerrain->GetTextureSet()->GetTexture(nIndex);
				if (pTex)
				{
					return pTex->GetTextureEntity();
				}
			}
		}
		else if (nIndex == -1)
		{
			// -1: common texture, which is repeated several times over each terrain tile surface.
			TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", pTerrain->GetTerrainCommonTextureFile(), TextureEntity::StaticTexture);
			return pTex;
		}
		else if (nIndex == -2)
		{
			// -2: main texture, which is chopped and mapped to the entire terrain surface.
			TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", pTerrain->GetTerrainBaseTextureFile(), TextureEntity::StaticTexture);
			return pTex;
		}
	}
	return NULL;
}

bool CGlobalTerrain::ReplaceTexture(float x, float y, int nIndex, TextureEntity* TextureAsset)
{
	Terrain * pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain && (nIndex >= 0 || TextureAsset != 0))
	{
		if (nIndex >= 0)
		{
			if (pTerrain->GetTextureSet())
			{
				if (TextureAsset)
				{
					if (pTerrain->GetTextureSet()->ReplaceTexture(nIndex, TextureAsset))
					{
						pTerrain->SetModified(true, MODIFIED_TEXTURE);
						return true;
					}
				}
				else
				{
					// True to normalize mask of the undeleted layers. 
					bool bNormalizeMask = true;
					pTerrain->GetTextureSet()->RemoveTexture(nIndex, pTerrain, bNormalizeMask);
				}
			}
		}
		else if (nIndex == -1)
		{
			// -1: common texture, which is repeated several times over each terrain tile surface.
			pTerrain->SetTerrainCommonTextureFile(TextureAsset->GetKey());
			return true;
		}
		else if (nIndex == -2)
		{
			// -2: main texture, which is chopped and mapped to the entire terrain surface.
			pTerrain->SetTerrainBaseTextureFile(TextureAsset->GetKey());
			return true;
		}
	}
	return false;
}

string CGlobalTerrain::GetTerrainElevFile(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			Terrain * pTerrain = m_pTerrainLattice->GetTerrainAtPoint(x, y);
			if (pTerrain)
			{
				return pTerrain->GetTerrainElevFile();
			}
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetTerrainElevFile();
	}
	return "";
}
string CGlobalTerrain::GetOnloadScript(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			Terrain * pTerrain = m_pTerrainLattice->GetTerrainAtPoint(x, y);
			if (pTerrain)
			{
				return pTerrain->GetOnloadScript();
			}
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetOnloadScript();
	}
	return "";
}

void CGlobalTerrain::Update(bool bForceUpdate)
{
	if ((CGlobals::GetSceneState()->m_nRenderCount > 0 && CGlobals::GetSceneState()->m_bCameraMoved) || bForceUpdate)
	{
		PERF1("terrain_tessellation");
		g_matTerrainProjection = CGlobals::GetProjectionMatrixStack().SafeGetTop();
		g_matTerrainModelview = CGlobals::GetViewMatrixStack().SafeGetTop();
		g_matTerrainViewProj = g_matTerrainModelview*g_matTerrainProjection;
		m_vTerrainRenderOffset = CGlobals::GetScene()->GetRenderOrigin();

		Vector3 vEye;
		Matrix4 InverseViewMatrix = g_matTerrainModelview.inverse();
		vEye = Vector3(0, 0, 0)*InverseViewMatrix;

		m_matTerrainEye = vEye + m_vTerrainRenderOffset;
#ifdef _DEBUG
		// check vEye
		Vector3 vEyeCamera = CGlobals::GetSceneState()->vEye;
#endif
		SetCameraPosition(m_matTerrainEye.x, m_matTerrainEye.z);

		ModelViewMatrixChanged();
	}
}

void CGlobalTerrain::Render()
{
	// push transform
	Matrix4 matSavedWorld;

	// push Identity
	CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, false, false);

	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			m_pTerrainLattice->Render();
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			m_pTerrainSingle->Render();
	}

	// pop
	CGlobals::GetWorldMatrixStack().pop();
}

DWORD CGlobalTerrain::GetRegionValue(const string& sLayerName, float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetRegionValue(sLayerName, x, y);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetRegionValue(sLayerName, x, y);
	}
	return 0;
}

float CGlobalTerrain::GetElevation(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetElevation(x, y);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetElevationW(x, y);
	}

	return m_fDefaultHeight;
}

// TODO: untested
void CGlobalTerrain::SetVertexElevation(float x, float y, float fHeight)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			m_pTerrainLattice->SetVertexElevation(x, y, fHeight);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			m_pTerrainSingle->SetVertexElevation(x, y, fHeight);
	}
}

// TODO: untested
float CGlobalTerrain::GetVertexElevation(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetVertexElevation(x, y);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetVertexElevation(m_pTerrainSingle->GetVertexW(x, y));
	}
	return 0.f;
}

float CGlobalTerrain::IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance)
{

	if ((dirX == 0) && (dirZ == 0))
	{
		// if it is vertical direction ray, we will use elevation to compute it fast.
		float fDist;
		float fHeight = GetElevation(startX, startZ);
		intersectX = startX;
		intersectY = fHeight;
		intersectZ = startZ;
		if (dirY <= 0)
		{
			fDist = startY - fHeight;
		}
		else
		{
			fDist = fHeight - startY;
		}
		return fDist;
	}

	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->IntersectRay(startX, startZ, startY, dirX, dirZ, dirY, intersectX, intersectZ, intersectY, fMaxDistance);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->IntersectRayW(startX, startZ, startY, dirX, dirZ, dirY, intersectX, intersectZ, intersectY, fMaxDistance);
	}
	/// if terrain object is created, just return the intersection with the y=0 plane
	if (fabs(dirY) < 0.0001f)
	{
		return -1.f;
	}
	float fDist = (m_fDefaultHeight - startY) / dirY;
	intersectX = startX + fDist*dirX;
	intersectY = m_fDefaultHeight;
	intersectZ = startZ + fDist*dirZ;
	return fDist;
}

void CGlobalTerrain::InitDeviceObjects()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->InitDeviceObjects();
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->InitDeviceObjects();
	}
}


void CGlobalTerrain::RestoreDeviceObjects()
{

}

void CGlobalTerrain::DeleteDeviceObjects()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->DeleteDeviceObjects();
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->DeleteDeviceObjects();
	}
}
void CGlobalTerrain::InvalidateDeviceObjects()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->InvalidateDeviceObjects();
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->InvalidateDeviceObjects();
	}
}

/// \brief Returns the surface normal of the terrain at the specified point.
/// \param x The x location of the point on the Terrain's surface in world units.
/// \param y The y location of the point on the Terrain's surface in world units.
/// \param normalX Gets filled with the surface normal x component
/// \param normalY Gets filled with the surface normal y component
/// \param normalZ Gets filled with the surface normal z component
void CGlobalTerrain::GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ)
{
	// secretly swap y,z, since the terrain use z as the terrain height.
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetNormal(x, y, normalX, normalZ, normalY);
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetNormalW(x, y, normalX, normalZ, normalY);
	}
	normalX = 0;
	normalY = 1.f;
	normalZ = 0;
}

void CGlobalTerrain::Paint(TextureEntity* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
{
	if (brushRadius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			// TODO: replace 16 with the detail tile count*2.
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()/16;
			if(brushRadius>=fMaxRaduis)
				brushRadius = fMaxRaduis;
#endif
			m_pTerrainLattice->Paint(detailTexture, brushRadius, brushIntensity, maxIntensity, erase, x, y);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainSingle->GetWidth()/(m_pTerrainSingle->GetNumberOfTextureTilesWidth()*2);
			if(brushRadius>=fMaxRaduis)
				brushRadius = fMaxRaduis;
#endif
			m_pTerrainSingle->Paint(detailTexture, brushRadius, brushIntensity, maxIntensity, erase, x, y);
		}
	}
}

float CGlobalTerrain::GetLowestVisibleHeight()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
			return m_pTerrainLattice->GetLowestVisibleHeight();
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->GetLowestVisibleHeight();
	}
	return FLOAT_POS_INFINITY;
}


void CGlobalTerrain::Flatten(float x, float y, float radius, int flatten_op, float elevation, float factor)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif
			return m_pTerrainLattice->Flatten(x, y, radius, flatten_op, elevation, factor);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::Spherical(float x, float y, float radius, float offset)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif
			return m_pTerrainLattice->Spherical(x, y, radius, offset);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::DigCircleFlat(float x, float y, float radius, float fFlatPercentage, float factor)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif
			return m_pTerrainLattice->DigCircleFlat(x, y, radius, fFlatPercentage, factor);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}
void CGlobalTerrain::GaussianHill(float x, float y, float radius, float hscale, float standard_deviation, float smooth_factor)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif
			return m_pTerrainLattice->GaussianHill(x, y, radius, hscale, standard_deviation, smooth_factor);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}
void CGlobalTerrain::RadialScale(float x, float y, float scale_factor, float min_dist, float max_dist, float smooth_factor, int frequency)
{
	if (max_dist <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(max_dist>=fMaxRaduis)
			{
				max_dist = fMaxRaduis;
				if(min_dist>max_dist)
					min_dist = max_dist;
			}
#endif
			return m_pTerrainLattice->RadialScale(x, y, scale_factor, min_dist, max_dist, smooth_factor, frequency);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}
void CGlobalTerrain::Roughen_Smooth(float x, float y, float radius, bool roughen, bool big_grid, float factor)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif

			return m_pTerrainLattice->Roughen_Smooth(x, y, radius, roughen, big_grid, factor);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::Ramp(float x1, float y1, float x2, float y2, float radius, float borderpercentage, float factor)
{
	if (radius <= 0)
		return;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif

			return m_pTerrainLattice->Ramp(x1, y1, x2, y2, radius, borderpercentage, factor);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::AddHeightField(float x, float y, const char* filename, int nSmoothPixels)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			return m_pTerrainLattice->AddHeightField(x, y, filename, nSmoothPixels);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::MergeHeightField(float x, float y, const char* filename, int mergeOperation/*=0*/, float weight1/*=1.0*/, float weight2/*=1.0*/, int nSmoothPixels/*=7*/)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			return m_pTerrainLattice->MergeHeightField(x, y, filename, mergeOperation, weight1, weight2, nSmoothPixels);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		// TODO: 
	}
}

void CGlobalTerrain::SaveTerrain(bool bHeightMap, bool bTextures)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			m_pTerrainLattice->SaveTerrain(bHeightMap, bTextures);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
		{
			m_pTerrainSingle->SaveToFile();
		}
	}
	int nCount = CGlobals::GetScene()->SaveLocalCharacters();
	if (nCount > 0)
	{
		OUTPUT_LOG("number of local characters saved : %d \r\n", nCount);
	}
}

bool CGlobalTerrain::IsModified()
{
	bool bModified = false;
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			bModified = m_pTerrainLattice->IsModified();
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
		{
			bModified = m_pTerrainSingle->IsModified();
		}
	}

	return bModified || CGlobals::GetScene()->IsModified();
}

void CGlobalTerrain::SetContentModified(float x, float y, bool bIsModified, DWORD dwModifiedBits)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			Terrain * pTerrain = m_pTerrainLattice->GetTerrainAtPoint(x, y);
			if (pTerrain)
			{
				return pTerrain->SetModified(bIsModified, dwModifiedBits);
			}
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->SetModified(bIsModified, dwModifiedBits);
	}
}

void CGlobalTerrain::SetAllLoadedModified(bool bIsModified, DWORD dwModifiedBits)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			m_pTerrainLattice->SetAllLoadedModified(bIsModified, dwModifiedBits);
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return m_pTerrainSingle->SetModified(bIsModified, dwModifiedBits);
	}
}

void CGlobalTerrain::EnableLighting(bool bEnable)
{
	SetUseNormals(bEnable);
}

IAttributeFields* CGlobalTerrain::GetAttributeObjectAt(float x, float y)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			Terrain * pTerrain = m_pTerrainLattice->GetTerrainAtPoint(x, y);
			if (pTerrain)
			{
				return (IAttributeFields*)pTerrain;
			}
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		if (m_pTerrainSingle)
			return (IAttributeFields*)m_pTerrainSingle;
	}
	return NULL;
}

bool CGlobalTerrain::GetHeightmapOf(float fCenterX, float fCenterY, float fSize, TTerrain* pOut)
{
	if (pOut == 0)
		return false;
	float fSpacing = fSize / (pOut->GetGridSize() - 1);
	int nSize = pOut->GetGridSize();
	pOut->SetVertexSpacing(fSpacing);
	float fLeft = fCenterX - fSize / 2;
	float fTop = fCenterY - fSize / 2;
	for (int i = 0; i < nSize; ++i)
	{
		for (int j = 0; j < nSize; ++j)
		{
			float x = fLeft + i*fSpacing;
			float y = fTop + j*fSpacing;
			float fHeight = GetElevation(x, y);
			pOut->SetHeight(i, j, fHeight);
		}
	}
	return true;
}

// TODO: untested
bool CGlobalTerrain::SetHeightmapOf(float fCenterX, float fCenterY, float fSize, TTerrain* pIn)
{
	if (pIn == 0)
		return false;
	float fSpacing = fSize / (pIn->GetGridSize() - 1);
	int nSize = pIn->GetGridSize();
	float fLeft = fCenterX - fSize / 2;
	float fTop = fCenterY - fSize / 2;
	for (int i = 0; i < nSize; ++i)
	{
		for (int j = 0; j < nSize; ++j)
		{
			float x = fLeft + i*fSpacing;
			float y = fTop + j*fSpacing;
			SetVertexElevation(x, y, pIn->GetHeight(i, j));
		}
	}
	return true;
}

int CGlobalTerrain::GetMaxTileCacheSize()
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			return m_pTerrainLattice->GetMaxTileCacheSize();
		}
	}
	else if (m_nTerrainType == SINGLE_TERRAIN)
	{
		return 1;
	}
	return 1;
}

void CGlobalTerrain::SetMaxTileCacheSize(int nNum)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			m_pTerrainLattice->SetMaxTileCacheSize(nNum);
		}
	}
}

#pragma region settings
bool CGlobalTerrain::IsVerbose()
{
	return Settings::GetInstance()->IsVerbose();
}

void CGlobalTerrain::SetVerbose(bool bEnable)
{
	Settings::GetInstance()->SetVerbose(bEnable);
}

void CGlobalTerrain::SetEditor(bool isEditor)
{
	Settings::GetInstance()->SetEditor(isEditor);
}

bool CGlobalTerrain::IsEditor()
{
	return Settings::GetInstance()->IsEditor();
}

void CGlobalTerrain::SetTessellateMethod(int method)
{
	Settings::GetInstance()->SetTessellateMethod(method);
}

int CGlobalTerrain::GetTessellateMethod()
{
	return Settings::GetInstance()->GetTessellateMethod();
}

float CGlobalTerrain::GetDefaultHeight() const
{
	return m_fDefaultHeight;
}

void CGlobalTerrain::SetDefaultHeight(float val)
{
	m_fDefaultHeight = val;
}

void CGlobalTerrain::SetUseNormals(bool bUseNormals)
{
	if (CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals() != bUseNormals)
	{
		CGlobals::GetGlobalTerrain()->GetSettings()->SetUseNormals(bUseNormals);
		// this will delete terrain buffers.
		InvalidateDeviceObjects();
	}
}

bool CGlobalTerrain::UseNormals()
{
	return Settings::GetInstance()->UseNormals();
}

void CGlobalTerrain::SetTextureMaskWidth(int nWidth)
{
	if (Settings::GetInstance()->GetTextureMaskWidth() != nWidth)
	{
		Settings::GetInstance()->SetTextureMaskWidth(nWidth);
		// resize all existing mask file. 
		if (m_nTerrainType == LATTICED_TERRAIN)
		{
			if (m_pTerrainLattice)
			{
				m_pTerrainLattice->ResizeTextureMaskWidth(nWidth);
				m_pTerrainLattice->SetGlobalConfigModified(true);
			}
		}
		else if (m_nTerrainType == SINGLE_TERRAIN)
		{
			m_pTerrainSingle->ResizeTextureMaskWidth(nWidth);
		}
	}
}

int CGlobalTerrain::GetTextureMaskWidth()
{
	return Settings::GetInstance()->GetTextureMaskWidth();
}

#pragma endregion settings


void CGlobalTerrain::SetGeoMipmapTerrain(bool enable)
{
	if (m_useGeoMipmap == enable)
		return;
#ifdef USE_OPENGL_RENDERER
	if (enable)
	{
		m_useGeoMipmap = false;
		OUTPUT_LOG("warning: GeoMipmap rendering is disabled for opengl\n");
		return;
	}
#endif
	m_useGeoMipmap = enable;
	if (m_pTerrainLattice != NULL)
		m_pTerrainLattice->SwitchLodStyle(m_useGeoMipmap);
}

int CGlobalTerrain::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);


	pClass->AddField("IsModified", FieldType_Bool, NULL, (void*)IsModified_s, NULL, NULL, bOverride);
	pClass->AddField("RenderTerrain", FieldType_Bool, (void*)EnableTerrainRendering_s, (void*)TerrainRenderingEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("EnableTerrain", FieldType_Bool, (void*)EnableTerrainEngine_s, (void*)IsTerrainEngineEnabled_s, NULL, NULL, bOverride);

	pClass->AddField("Verbose", FieldType_Bool, (void*)SetVerbose_s, (void*)IsVerbose_s, NULL, NULL, bOverride);
	pClass->AddField("IsEditor", FieldType_Bool, (void*)SetEditor_s, (void*)IsEditor_s, NULL, NULL, bOverride);
	pClass->AddField("TessellateMethod", FieldType_Int, (void*)SetTessellateMethod_s, (void*)GetTessellateMethod_s, NULL, NULL, bOverride);
	pClass->AddField("UseNormals", FieldType_Bool, (void*)SetUseNormals_s, (void*)UseNormals_s, NULL, NULL, bOverride);
	pClass->AddField("TextureMaskWidth", FieldType_Int, (void*)SetTextureMaskWidth_s, (void*)GetTextureMaskWidth_s, NULL, NULL, bOverride);

	pClass->AddField("UseGeoMipmapLod", FieldType_Bool, (void*)SetGeoMipmapTerrain_s, (void*)IsGeoMipmapTerrain_s, NULL, NULL, bOverride);
	pClass->AddField("AllowSlopeCollision", FieldType_Bool, (void*)SetSlopeCollision_s, (void*)IsSlopeCollision_s, NULL, NULL, bOverride);
	pClass->AddField("IsEditorMode", FieldType_Bool, (void*)SetEditorMode_s, (void*)IsEditorMode_s, NULL, NULL, bOverride);

	pClass->AddField("BlockDamageTexture", FieldType_String, (void*)SetBlockDamageTexture_s, NULL, NULL, "", bOverride);
	pClass->AddField("BlockSelectionTexture", FieldType_String, (void*)SetBlockSelectionTexture_s, NULL, NULL, "", bOverride);

	pClass->AddField("DefaultHeight", FieldType_Float, (void*)SetDefaultHeight_s, (void*)GetDefaultHeight_s, NULL, NULL, bOverride);
	return S_OK;
}

void CGlobalTerrain::PaintTerrainData(float x, float y, float radius, unsigned int data, unsigned int dataMask, unsigned int bitOffset)
{
	if (radius <= 0)
		return;

	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
#ifdef VALIDATE_BRUSHSIZE
			float fMaxRaduis = m_pTerrainLoader->GetTerrainWidth()*MAX_TERRAIN_MORPH_RADIUS_RATIO;
			if(radius>=fMaxRaduis)
				radius = fMaxRaduis;
#endif
			m_pTerrainLattice->SetTerrainInfo(x, y, radius, data, dataMask, bitOffset);
		}
	}
}


uint32 CGlobalTerrain::GetTerrainData(float x, float y, uint32 dataMask, uint8 bitOffset)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
	{
		if (m_pTerrainLattice)
		{
			return m_pTerrainLattice->GetTerrainInfo(x, y, dataMask, bitOffset);
		}
	}
	return 0;
}

bool CGlobalTerrain::IsWalkable(float x, float y, Vector3& oNormal) const
{
	if (m_nTerrainType == LATTICED_TERRAIN && m_pTerrainLattice && m_enableSlopeCollision)
	{
		return m_pTerrainLattice->IsWalkable(x, y, oNormal);
	}
	else
	{
		oNormal.x = 0;
		oNormal.y = 1;
		oNormal.z = 0;
		return true;
	}
}

void CGlobalTerrain::SetEditorMode(bool enable)
{
	m_isEditorMode = enable;
	if (m_pTerrainLattice != NULL)
	{
		m_pTerrainLattice->SetEditorMode(m_isEditorMode);
	}
}

void CGlobalTerrain::SetVisibleDataMask(uint32 mask, uint8 bitOffset)
{
	if (m_pTerrainLattice != NULL)
	{
		m_pTerrainLattice->SetVisibleDataMask(mask, bitOffset);
	}
}
void CGlobalTerrain::SetBlockDamageTexture(const char* textureName)
{
	BlockWorldClient* mgr = BlockWorldClient::GetInstance();
	if (mgr && textureName != NULL)
		mgr->SetDamageTexture(textureName);
}

void CGlobalTerrain::SetBlockSelectionTexture(const char* textureName)
{
	BlockWorldClient* mgr = BlockWorldClient::GetInstance();
	if (mgr && textureName != NULL)
		mgr->SetSelectionTexture(textureName);
}

const Vector3& CGlobalTerrain::GetMatTerrainEye() const
{
	return m_matTerrainEye;
}

const Vector3& CGlobalTerrain::GetTerrainRenderOffset() const
{
	return m_vTerrainRenderOffset;
}

IAttributeFields* CGlobalTerrain::GetChildAttributeObject(const std::string& sName)
{
	if (m_nTerrainType == LATTICED_TERRAIN)
		return m_pTerrainLattice;
	else
		return m_pTerrainSingle;
}

IAttributeFields* CGlobalTerrain::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		if (nRowIndex == 0)
		{
			if (m_nTerrainType == LATTICED_TERRAIN)
				return m_pTerrainLattice;
			else
				return m_pTerrainSingle;
		}
	}
	return NULL;
}

int CGlobalTerrain::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return 1;
}

int CGlobalTerrain::GetChildAttributeColumnCount()
{
	return 1;
}
