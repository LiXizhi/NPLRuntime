//-----------------------------------------------------------------------------
// Class: CVoxelMesh
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2008.1.16
//-----------------------------------------------------------------------------
/**
- Voxel mesh is implemented using marching cube algorithm (http://www.polytech.unice.fr/~lingrand/MarchingCubes/algo.html). Please see ./VoxelMesh/*.*. To create Voxel mesh use ParaScene.CreateVoxelMesh();
The architecture is briefly below: CVoxelMesh is a scene object that contains a MetaWorldFragment. MetaWorldFragment is a collection of metaobjects. MetaWorldFragment can be serialzed to/from a 32*32*32 grid file.
instead of using a grid file, we can construct the grid manually using prefined metaobjects, such as metaball and metaheightmap. When MetaWorldFragment is serialized to file, all metaobjects may be collapsed to a single cube 3d grid, very economic in size.
Each MetaWorldFragment is associated with one IsoSurface.An IsoSurface is the triangle soap built from a 32*32*32 3d grid using marching cube algorithm. DataGrid is a temporary and shared singleton data class used when building the triangles. 
IsoSurfaceBuilder is the actual class that performs the marching cube algorithms. IsoSurface is rendered using the IsoSurfaceRenderable object, which in turn uses DynamicRenderable class defined in ParaEngine base class. DynamicRenderable contains everything needed for rendering, so the isosurface is only rebuilt when a voxel mesh is first drawn or modified. 
- Voxel mesh is usually used for overhang or cave terrains with traditional height map based terrain. That is why the high level manager is called VoxelTerrainManager. 
Voxel mesh can also be used independently with global height map terrain. E.g.  Create arbitrary sized and editiable mesh (sculptures) or single terrain mesh in miniscenegraphs, etc. 
- VoxelTerrainManager keeps an instance of  IsoSurfaceBuilder and GridData, which are used (shared) by all MetaWorldFragments. IsoSurfaceBuilder and GridData is initialized with 32*32*32*N data, which consumes quite a bit memory, and there are a few 
parameters that can be specified when creating them, such as grid size, grid scaling, whether use normals, normal method, whether use texture coordinate or vertex colors, etc. Unlike DataGrid, The data chain: CVoxelMesh->MetaWorldFragment(1)->IsoSurface(1), MetaObjects(n), only contains necessary data to store and render the grid, so it is economic in size. 
- TODO: make alignment of voxel mesh with terrain and pick the base texture of the global terrain as the voxel mesh texture. Create holes in the global terrain to create caves in terrain automatically. 
- TODO: UV of voxel mesh is currently set to be the same as the vertex grid position x,z in object space. This will kind of project a texture to the surface of the voxel mesh in the horizonal xz plain. This can be used with the global heightmap terrain to automatically pick the color of terrain for the voxel mesh. 
- TODO: Editing and saving voxel mesh in the game: we can search for existing voxel mesh in a region, if found we will use meta objects to edit the voxel mesh. If not found, a voxel mesh can be created at the current location. 
- Many thanks to Martin Enge who wrote the original marching cube algorithm and posted to the OGRE forum, and I have ported and improved much of its code to ParaEngine.
*/
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "EffectManager.h"
#include "VoxelMesh/MetaWorldFragment.h"
#include "VoxelMesh/IsoSurfaceRenderable.h"
#include "VoxelMesh/MetaBall.h"
#include ".\voxelmesh.h"
#include "memdebug.h"
using namespace ParaEngine;

CVoxelMesh::CVoxelMesh(void)
{
	SetMyType(_VoxelMesh);
}

CVoxelMesh::~CVoxelMesh(void)
{
	
}

void ParaEngine::CVoxelMesh::Reset()
{

}

void ParaEngine::CVoxelMesh::SetScaling( float s )
{
	// TODO: shall we allow scaling? perhaps only allow it, if it is not linked to the global terrain, 
	// i.e. contains no meta terrain object. 
}

float ParaEngine::CVoxelMesh::GetScaling()
{
	return 1.f;
}

HRESULT ParaEngine::CVoxelMesh::InitObject( const char* sVoxelFile, const DVector3& vPos, TextureEntity* pTexture /*= NULL*/ )
{
	SetPosition(vPos);
	m_pBaseTexture = pTexture;
	if(pTexture == NULL)
	{
		// TODO: remove this line: just testing
		// m_pBaseTexture = CGlobals::GetAssetManager()->LoadTexture("", "terrain/data/LlanoTex.jpg", TextureEntity::StaticTexture);
	}

	// TODO: use a voxel shader
	SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL);

	// TODO: set bounding sphere, this is for view clipping object calculation and scene attachment. 
	SetRadius(1.f);
	return S_OK;
}

HRESULT ParaEngine::CVoxelMesh::Draw( SceneState * sceneState )
{
	// TODO: remove this line . just a test here.
	GetMetaWorldFragment();

	if(m_pMetaWorldFragment == 0)
		return S_OK;
	
	m_pMetaWorldFragment->update(CGlobals::GetAssetManager()->GetVoxelTerrainManager().GetIsoSurfaceBuilder());
	IsoSurfaceRenderable* pIsoSurface = m_pMetaWorldFragment->getIsoSurface();
	if(pIsoSurface)
	{
		sceneState->SetCurrentSceneObject(this);
		auto pd3dDevice = sceneState->m_pd3dDevice;

		// get world transform matrix
		Matrix4 mxWorld;
		Vector3 vPos = GetRenderOffset();
		mxWorld.makeTrans(vPos.x, vPos.y, vPos.z);
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();

		CGlobals::GetWorldMatrixStack().push(mxWorld);

		pIsoSurface->SetTexture(m_pBaseTexture.get());

		if ( pEffectFile == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline
			pIsoSurface->Render(sceneState, pd3dDevice);
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// draw using effect file

			// apply per mesh object effect parameters
			if(GetEffectParamBlock()!=0)
			{
				GetEffectParamBlock()->ApplyToEffect(pEffectFile);
			}

			pIsoSurface->Render(sceneState, pEffectFile);
		}
		CGlobals::GetWorldMatrixStack().pop();
	}
	return S_OK;
}

int ParaEngine::CVoxelMesh::GetNumReplaceableTextures()
{
	return 1;
}

TextureEntity* ParaEngine::CVoxelMesh::GetReplaceableTexture( int ReplaceableTextureID )
{
	return m_pBaseTexture.get();
}

bool ParaEngine::CVoxelMesh::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	m_pBaseTexture = pTextureEntity;
	return true;
}

MetaWorldFragment* CVoxelMesh::GetMetaWorldFragment()
{
	if(m_pMetaWorldFragment == 0)
	{
		m_pMetaWorldFragment = new MetaWorldFragment(Vector3(0,0,0), 0);

		// TODO: remove this line: test with a meta ball
		MetaObjectPtr  TestMetaBall(new MetaBall(m_pMetaWorldFragment.get(), Vector3(0,0,0), 3.f, false));
		m_pMetaWorldFragment->addMetaObject(TestMetaBall);
		TestMetaBall = new MetaBall(m_pMetaWorldFragment.get(), Vector3(2.5,0,0), 2.f, false);
		m_pMetaWorldFragment->addMetaObject(TestMetaBall);
		
	}
	return m_pMetaWorldFragment.get();
}