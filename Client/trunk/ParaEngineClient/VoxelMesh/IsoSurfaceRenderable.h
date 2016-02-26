#pragma once
#include "AssetEntity.h"
#include "DynamicRenderable.h"
#include <string>
#include "ShapeAABB.h"

namespace ParaEngine
{
	class IsoSurfaceBuilder;
	class CEffectFile;

	/**
	* Renderable object for the dynamically generated IsoSurface or voxel mesh. 
	*/
	class IsoSurfaceRenderable : public CDynamicRenderable
	{
	public:
		IsoSurfaceRenderable();
		virtual ~IsoSurfaceRenderable() {};
		void createVertexDeclaration();
		void initialize(IsoSurfaceBuilder *builder);
		virtual void fillHardwareBuffers(IsoSurfaceBuilder *surf);
		virtual bool getNormaliseNormals(void) const {return true; }
		virtual const CShapeAABB &getBoundingBox(void) const {return mAABB;}

		/** set the material (texture) used by this isosurface
		* it can be NULL to use the current texture
		*/
		void SetTexture(TextureEntity* pTexture);

		void deleteGeometry();

		// Rendering with fixed function 
		HRESULT Render( SceneState * pSceneState, LPDIRECT3DDEVICE9 pd3dDevice);

		// Rendering with shader effect
		HRESULT Render( SceneState * pSceneState, CEffectFile *pEffect);

	protected:
		CShapeAABB mAABB;
		/// Flags from IsoSurfaceBuilder used at initialize().
		int mSurfaceFlags;

		asset_ptr <TextureEntity> m_texture;
	};
}
