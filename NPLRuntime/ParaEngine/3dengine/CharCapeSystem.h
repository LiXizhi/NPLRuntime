#pragma once
#include "ParaEngine.h"
#include "CharCapeCore.h"
#include "TextureEntity.h"

namespace ParaEngine
{
	class CParaXModel;
	class ParaXModelCanvas;
	class SceneState;
	class CParameterBlock;
	struct PlayerReflectionMesh;
	class IAttributeFields;

	struct CapeConfig
	{
		CapeMode mode = CapeMode::None;
		CapeStyleParams style;
		CapeSimTuning tuning;

		bool autoScale = true;
		bool replaceBackGeoset = true;
		bool doubleSided = true;
		bool drawStarEmblem = true;

		DWORD outerColor = 0xffe85d3f;
		DWORD innerColor = 0xffffd166;
		DWORD gradientColor = 0x00000000;
		DWORD trimColor = 0xfffff3b0;
		float trimWidth = 0.045f;
		DWORD emissiveColor = 0xfffff3b0;
		float emissiveIntensity = 0.35f;
		float emblemScale = 0.22f;

		std::string outerTexture;
		std::string innerTexture;
		std::string emblemTexture;

		// GeosetTakeover options
		int targetGeosetId = 0;        // Explicit geoset ID (e.g. 1002, 1501), or 0 to use targetGeosetCategory
		int targetGeosetCategory = -1; // e.g. 10 (CSET_WINGS) or 15 (CSET_CAPE)
		int targetAttachmentId = -1;   // Optional attachment ID (e.g. 15 ATT_ID_BACK_ADDON)
		std::string bonePrefix;
		std::vector<int> boneIndices;
		bool preferBoneDrive = true;

		bool ParseFromJson(const std::string& jsonStr);
		std::string ToJson() const;
	};

	/**
	 * Per-character cape controller owned by CharModelInstance.
	 * Supports both Procedural Sky-style DIY capes and existing Geoset/Bone takeover,
	 * automatically switching to zero-physics static rendering when distant or over budget.
	 */
	class CharCapeInstance
	{
	public:
		CharCapeInstance();
		~CharCapeInstance();

		bool SetConfigJson(const std::string& jsonStr);
		std::string GetConfigJson() const;

		/** Used by automatic Haqi / CCS cape detection when no explicit script cape is active */
		void SetAutoGeosetTakeover(int geosetCategory, int geosetItemValue);
		void ClearAutoGeosetTakeover();
		void Clear();

		bool IsActive() const { return m_config.mode != CapeMode::None; }
		bool IsAutoManaged() const { return m_bAutoManaged; }
		bool IsSimulating() const { return IsActive() && m_bTargetActive && m_fSimBlend > 0.001f; }
		float GetSimBlend() const { return m_fSimBlend; }
		const CapeConfig& GetConfig() const { return m_config; }

		/** Whether CharModelInstance::UpdateGeosetsToModel should hide a base model geoset id (e.g. 1002 or 1501) */
		bool ShouldSuppressModelGeoset(int geosetId) const;

		/** Called at the end of CharModelInstance::AnimateModel after base skeleton matrices are computed */
		void UpdateSimulation(
			SceneState* sceneState,
			CParaXModel* pBaseModel,
			ParaXModelCanvas* pCanvas,
			const int* charGeosets,
			IAttributeFields* pAnimInstance);

		/** Called at the end of CharModelInstance::Draw */
		void Draw(
			SceneState* sceneState,
			CParaXModel* pBaseModel,
			ParaXModelCanvas* pCanvas,
			CParameterBlock* materialParams,
			PlayerReflectionMesh* reflection);

	private:
		struct BoneChainLink
		{
			int boneIndex = -1;
			int parentLink = -1;
			int particleRow = 0;
			int particleCol = 0;
			CapeVec3 restDirFromParent = { 0.0f, -1.0f, 0.0f };
			float restLen = 0.15f;
		};

		struct ProxyVertexMap
		{
			uint16_t origVertexIndex = 0;
			float u = 0.5f;
			float v = 0.0f;
			float normalOffset = 0.0f;
			CapeVec2 uv = { 0.0f, 0.0f };
		};

		struct ProxyPassSlice
		{
			int passIndex = -1;
			size_t firstTriVertex = 0;
			size_t triVertexCount = 0;
		};

		void InvalidateGeometry();
		bool EnsureBuilt(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas, const int* charGeosets);
		bool BuildProceduralCape(CParaXModel* pBaseModel);
		bool BuildGeosetTakeover(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas, int resolvedGeosetId);
		CParaXModel* ResolveTargetModel(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas) const;

		void ExtractRigAnchorsAndCapsules(
			CParaXModel* pModel,
			CapeVec3& outNeck,
			CapeVec3& outLeftShoulder,
			CapeVec3& outRightShoulder,
			CapeCapsule* outCapsules,
			size_t& outCapsuleCount) const;

		void BuildCanonicalBodyCollider(
			CParaXModel* pModel,
			const CapeVec3& neckAnchor,
			const CapeVec3& leftShoulder,
			const CapeVec3& rightShoulder,
			CapeBodyCollider& outBody) const;

		void ApplyBoneChainOverrides(CParaXModel* pTargetModel);
		void BuildProxyDeformedTriangles(CParaXModel* pBaseModel);

		TextureEntity* ResolveOuterTexture();
		TextureEntity* ResolveInnerTexture();
		TextureEntity* ResolveEmblemTexture();
		void DrawTriangleList(
			SceneState* sceneState,
			const std::vector<CapeSurfaceVertex>& tris,
			TextureEntity* pTexture,
			DWORD tintColorARGB,
			DWORD emissiveColorARGB,
			float emissiveIntensity,
			int blendMode,
			bool enableBackfaceCull,
			CParameterBlock* materialParams);

	private:
		CapeConfig m_config;
		std::string m_rawJson;
		bool m_bAutoManaged = false;
		bool m_bGeometryBuilt = false;

		const CParaXModel* m_pBuiltForModel = nullptr;
		int m_boundGeosetId = 0;
		bool m_bUseBoneChainDrive = false;

		float m_modelScale = 1.0f;
		CapeVec3 m_forwardDir = { 1.0f, 0.0f, 0.0f };
		CapeVec3 m_upDir = { 0.0f, 1.0f, 0.0f };
		CapeVec3 m_rightDir = { 0.0f, 0.0f, -1.0f };
		CapeVec3 m_initialNeckRef = { 0.0f, 1.2f, 0.0f };
		CapeVec3 m_currentNeckAnchor = { 0.0f, 1.2f, 0.0f };

		CapeClothMesh m_cloth;
		std::vector<CapeSurfaceVertex> m_cachedStaticOuterTris;
		std::vector<CapeSurfaceVertex> m_cachedStaticInnerTris;
		std::vector<CapeSurfaceVertex> m_dynamicOuterTris;
		std::vector<CapeSurfaceVertex> m_dynamicInnerTris;

		// GeosetTakeover states
		std::vector<BoneChainLink> m_boneLinks;
		std::vector<ProxyVertexMap> m_proxyTriVertices;
		std::vector<ProxyPassSlice> m_proxyPasses;

		// Budget & motion state
		bool m_bTargetActive = false;
		float m_fSimBlend = 0.0f;
		int m_nLastSimRenderCount = -1;
		bool m_bHasPrevMotion = false;
		DVector3 m_prevWorldPos = { 0.0, 0.0, 0.0 };
		float m_prevYaw = 0.0f;
		float m_accumTimeSec = 0.0f;

		// Synthesized or loaded textures
		bool m_bTexturesDirty = true;
		asset_ptr<TextureEntity> m_outerTex;
		asset_ptr<TextureEntity> m_innerTex;
		asset_ptr<TextureEntity> m_emblemTex;
	};
}
