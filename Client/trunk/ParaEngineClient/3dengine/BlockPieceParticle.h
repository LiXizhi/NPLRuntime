#pragma once
#include "BlockDynamicObject.h"
#include "ParticleElement.h"

namespace ParaEngine
{
	class CBlockPieceParticle;

	/** particle element*/
	class CBlockPieceParticleElement : public CParticleElement
	{
	public:
		CBlockPieceParticleElement(CBlockPieceParticle* pParent);
		virtual ~CBlockPieceParticleElement();

		/** generate particle sprite vertex into the vertex buffer based on current camera setting (bill boarding).
		* @return the number of particles generated (1 particle has 2 triangles with 6 vertices).
		*/
		virtual int RenderParticle(SPRITEVERTEX** pVertexBuffer, SceneState* pSceneState);
		virtual TextureEntity* GetTexture();
		virtual bool IsPointTexture() { return true; }
		virtual bool IsAlphaTested() { return false; }

	protected:
		CBlockPieceParticle* m_pParent;
	};

	/** a single particle object */
	class CBlockPieceParticle : public CBlockDynamicObject
	{
	public:
		CBlockPieceParticle();

		virtual ~CBlockPieceParticle();

		ATTRIBUTE_DEFINE_CLASS(CBlockPieceParticle);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CBlockPieceParticle);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		
		ATTRIBUTE_METHOD1(CBlockPieceParticle, SetTextureFilename_s, const char*)	{ cls->SetTextureFilename(p1); return S_OK; }
	public:
		virtual HRESULT Draw(SceneState * sceneState);
		virtual int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);

		void SetTextureFilename(const std::string& texture);
	public:
		TextureEntity* GetTexture();
		/** generate particle sprite vertex into the vertex buffer based on current camera setting (bill boarding).
		* @return the number of particles generated (1 particle has 2 triangles with 6 vertices).
		*/
		int RenderParticle(SPRITEVERTEX** pVertexBuffer, SceneState* pSceneState);
	protected:
		/** based on fade out time */
		float CalculateOpacity();
		/** random texture uv */
		void GenerateParticleUV();
		
	protected:
	
		ref_ptr<TextureEntity> m_texture;
		ParticleRectUV m_texUVs;
		ref_ptr<CBlockPieceParticleElement> m_particle_element;
	};
}

