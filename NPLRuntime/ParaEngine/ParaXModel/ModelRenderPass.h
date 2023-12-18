#pragma once

namespace ParaEngine
{
	class CParaXModel;
	class CParameterBlock;
	struct SceneState;

	struct ModelRenderPass
	{
	public:
		ModelRenderPass();
	public:
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
		int32 tex;
		union {
			float m_fStripLength;
			float m_fCategoryID;
			float m_fReserved0;
		};


		int16 texanim, color, opacity, blendmode;
		int32 order;
		int32 geoset;

		// TODO: bit align to DWORD
		bool usetex2 : 1, useenvmap : 1, cull : 1, trans : 1, unlit : 1, nozwrite : 1, swrap : 1, twrap : 1, force_local_tranparency : 1, skinningAni : 1, is_rigid_body : 1, disable_physics : 1, force_physics : 1, has_category_id : 1;
		DWORD : 0;// Force alignment to next boundary.

		/// for fixed function pipeline
		bool init(CParaXModel* m, SceneState* pSceneState);
		void deinit();

		/// for programmable pipeline
		bool init_FX(CParaXModel* m, SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void deinit_FX(SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);

		/// for programmable pipeline
		bool init_bmax_FX(CParaXModel* m, SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);
		void deinit_bmax_FX(SceneState* pSceneState, CParameterBlock* pMaterialParams = NULL);

		/** for deferred shading, we will render alpha blended blocks after final effect. */
		bool IsAlphaBlended();

		/** get the physics group. */
		int GetPhysicsGroup();

		bool hasPhysics();

		bool operator< (const ModelRenderPass& m) const
		{
			//return !trans;
			if (order < m.order) return true;
			else if (order > m.order) return false;
			else return blendmode == m.blendmode ? (m_fReserved0 < m.m_fReserved0) : blendmode < m.blendmode;
		}

		inline bool operator == (const ModelRenderPass& m) const
		{
			return tex == m.tex && blendmode == m.blendmode && color == m.color;
		}

		void SetStartIndex(int32 nIndex);

		int32 GetStartIndex();

		int32 GetVertexStart(CParaXModel* m);

		/** used in some shader parameter */
		int32 GetCategoryId();
		void SetCategoryId(int32 nCategoryID);

		void SetTexture1(int32_t texture)
		{
			tex = (texture & 0xffff) + (tex & 0xffff0000);
		}
		int32_t GetTexture1()
		{
			return tex & 0xffff;
		}
		void SetTexture2(int32_t texture)
		{
			tex = ((texture & 0xffff) << 16) + (tex & 0xffff);
			usetex2 = texture != 0;
		}
		int32_t GetTexture2()
		{
			return tex >> 16;
		}
	};
}