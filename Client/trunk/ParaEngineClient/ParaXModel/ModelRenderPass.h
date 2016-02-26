#pragma once

namespace ParaEngine
{
	class CParaXModel;
	class CParameterBlock;
	struct SceneState;

	struct ModelRenderPass {
		/** Fix LiXizhi 2010.1.14. we may run out of 65535 vertices. so if indexStart is 0xffff, then we will use m_nIndexStart instead */
		uint16 indexStart, indexCount;
		union {
			struct {
				uint16 vertexStart, vertexEnd;
			};
			/** if indexStart is 0xffff, then m_nIndexStart stores the index offset in 32 bits. */
			int32 m_nIndexStart;
		};

		//TextureID texture, texture2;
		int tex;
		float p;
		int16 texanim, color, opacity, blendmode;
		int order;
		int geoset;

		// TODO: bit align to DWORD
		bool usetex2 : 1, useenvmap : 1, cull : 1, trans : 1, unlit : 1, nozwrite : 1, swrap : 1, twrap : 1, force_local_tranparency : 1, skinningAni : 1, is_rigid_body : 1;
	DWORD: 0;// Force alignment to next boundary.

		/// for fixed function pipeline
		bool init(CParaXModel *m, SceneState* pSceneState);
		void deinit();

		/// for programmable pipeline
		bool init_FX(CParaXModel *m, SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void deinit_FX(SceneState* pSceneState);

		/// for programmable pipeline
		bool init_bmax_FX(CParaXModel *m, SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void deinit_bmax_FX(SceneState* pSceneState);

		/** for deferred shading, we will render alpha blended blocks after final effect. */
		bool IsAlphaBlended();
		bool operator< (const ModelRenderPass &m) const
		{
			//return !trans;
			if (order<m.order) return true;
			else if (order>m.order) return false;
			else return blendmode == m.blendmode ? (p < m.p) : blendmode < m.blendmode;
		}

		inline bool operator == (const ModelRenderPass &m) const
		{
			return tex == m.tex && blendmode == m.blendmode && color == m.color;
		}

		void SetStartIndex(int32 nIndex);

		int32 GetStartIndex();

		int32 GetVertexStart(CParaXModel *m);
	};
}