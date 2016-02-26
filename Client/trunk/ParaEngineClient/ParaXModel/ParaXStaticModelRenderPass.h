#pragma once
#include "ParaXMaterial.h"

namespace ParaEngine
{
	class CParaXStaticModel;
	struct SceneState;
	class CParameterBlock;

	struct ParaXStaticModelRenderPass
	{
	public:
		ParaXStaticModelRenderPass();
		/** Fix LiXizhi 2010.1.14. we may run out of 65535 vertices. so if indexStart is 0xffff, then we will use m_nIndexStart instead */
		uint16 indexStart, indexCount;
		union {
			struct {
				uint16 vertexStart, vertexEnd;
			};
			/** if indexStart is 0xffff, then m_nIndexStart stores the index offset in 32 bits. */
			int32 m_nIndexStart;
		};
		CParaXMaterial m_material;
		
		int order;
		int geoset;
		float p;
		int16 blendmode;
		
		// for programmable pipeline
		bool init_FX(CParaXStaticModel *m_, SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void deinit_FX(SceneState* pSceneState);

		bool operator< (const ParaXStaticModelRenderPass &m) const
		{
			//return !trans;
			if (order < m.order) return true;
			else if (order > m.order) return false;
			else return blendmode == m.blendmode ? (p < m.p) : blendmode < m.blendmode;
		}

		inline bool operator == (const ParaXStaticModelRenderPass &m) const
		{
			return m_material.m_pTexture1 == m.m_material.m_pTexture1 && blendmode == m.blendmode;
		}
	};
}