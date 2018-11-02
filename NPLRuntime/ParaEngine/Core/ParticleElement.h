#pragma once
#include "VertexFVF.h"

namespace ParaEngine
{
	struct TextureEntity;
	struct SPRITEVERTEX;
	struct SceneState;

	/** for pre-calculated UV
	*/
	struct ParticleRectUV
	{
		Vector2 tc[4];
	};

	/** base class for a rectangle particle. one can override this class for different kind of particles.
	* particles are sorted by texture, and one should at least implement RenderParticle() for particle tessellation.
	* one need to implement at least RenderParticle and GetTexture() function. 
	*/
	class CParticleElement : public CRefCounted
	{
	public:
		CParticleElement(){};
		virtual ~CParticleElement(){};

		/** generate particle sprite vertex into the vertex buffer based on current camera setting (bill boarding).
		* @return the number of particles generated (1 particle has 2 triangles with 6 vertices).
		*/
		virtual int RenderParticle(SPRITEVERTEX** pVertexBuffer, SceneState* pSceneState) = 0;
		virtual TextureEntity* GetTexture() = 0;
		virtual bool IsPointTexture() { return false; }
		virtual bool IsAlphaTested() { return false; }

		static inline void SetParticleVertex(SPRITEVERTEX& vert, float x, float y, float z, float u, float v, DWORD color)
		{
			vert.p.x = x; vert.p.y = y; vert.p.z = z;
			vert.tu = u; vert.tv = v; vert.color = color;
		}
	protected:
		TextureEntity* m_pTexture;
	};


	/** particle render pass */
	struct ParticleRenderPass
	{
		ParticleRenderPass() : texture(NULL), isPointTexture(false), isAlphaTested(false), nMinIndex(0xfffffff), nMaxIndex(0), nCount(0){};
		ParticleRenderPass(CParticleElement* baseParticle)
			: ParticleRenderPass()
		{
			if (baseParticle)
			{
				texture = baseParticle->GetTexture();
				isPointTexture = baseParticle->IsPointTexture();
				isAlphaTested = baseParticle->IsAlphaTested();
			}
		}
		TextureEntity* GetTexture() { return texture; }
		
		/** check if particle belongs to this render pass */
		bool CheckParticle(CParticleElement* particle)
		{
			return texture == particle->GetTexture();
		}

		void AddParticle(int nParticleIndex)
		{
			if (nMinIndex > nParticleIndex)
				nMinIndex = nParticleIndex;
			if (nMaxIndex < nParticleIndex)
				nMaxIndex = nParticleIndex;
			nCount++;
		}
		int32 GetCount() const { return nCount; }
		int32 GetMinIndex() const { return nMinIndex; }
		int32 GetMaxIndex() const { return nMaxIndex; }
		bool IsPointTexture() const { return isPointTexture; }
		bool IsAlphaTested() const { return isAlphaTested; }

		int32 nMinIndex;
		int32 nMaxIndex;
		int32 nCount;
		bool isPointTexture;
		bool isAlphaTested;
		TextureEntity* texture;
	};
}