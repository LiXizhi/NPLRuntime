#pragma once

#include "ParaXModel.h"
#include "ParaXModel/FBXModelInfo.h"

struct aiScene;
struct aiMaterial;
struct aiNode;
struct aiAnimation;
struct aiBone;

namespace ParaEngine
{
	struct FBXMaterial2
	{
	public:
		FBXMaterial2();
	
		string strDiffuseTexName;
		int nDiffuseTexIndex;

		bool isAlphaBlended() { return fAlphaTestingRef == 0.f; }

		/* material category id, may be used by some deferred shading shader. default to 0*/
		int32 GetCategoryID() const { return m_nCategoryID; }
		void SetCategoryID(int32 val) { m_nCategoryID = val; }

		bool bHasOpacityTex;
		// alpha testing threshold, if this is 0, alpha testing is disabled. if this is 0.5, alpha testing is enabled. 
		// although this value can be [0,1), the game engine currently only support 0 or 0.5 alpha testing. i.e. boolean value.
		float fAlphaTestingRef;
		// whether z buffer is disabled for the given material, default is false, unless you are working on special material such as particles. 
		bool bDisableZWrite;
		// if this is true, we will not sort transparent mesh(with bDisableZWrite) globally, instead it will only be sorted per mesh, hence greatly increased rendering speed. 
		bool bForceLocalTranparency;

		/* material category id, may be used by some deferred shading shader. default to 0*/
		int32 m_nCategoryID;
		int32 m_nOrder;

		// if true, physics of this material will be disabled.
		bool bDisablePhysics;
		// if true, we will force physics and ignore all physics from other materials 
		bool bForcePhysics;
		// if true, no lighting is applied to the surface. default to false
		bool bUnlit;
		// if true, additive alpha or one alpha blending is enabled. if bHasOpacityMap, it is BM_ADDITIVE_ALPHA, otherwise, it is BM_ADDITIVE
		bool bAddictive;
		// billboarded
		bool bBillboard;
		// axis aligned billboard, always up. 
		bool bAABillboard;
		// this will force this material to be unique, since the exporter will materials with the same parameter. 
		int nForceUnique;
		// -1 if it is not replaceable texture.
		int nIsReplaceableIndex;

		int opacity;

		//////////////////////////////////////////////////////////////////////////
		bool bIsUsed;
		vector<int> vecRefPassIndex;

	};

	/*struct FBXNode2
	{
	public:
		FBXNode2();

		int index;
		int parentIndex;
		string strNodeName;
		Matrix4 mLocalMatrix;
		Matrix4 mGlobalMatrix;

	};*/
	
	class FBXParser2
	{
	public:
		//FBXParser2();
		FBXParser2(const string& path);
		~FBXParser2();

		CParaXModel* Parse(const aiScene* pFBXScene, const aiNode* pStartNode);

	private:
		void ParseMaterial(int nMatIndex, const aiMaterial* pMat);
		void ParseNode(const aiNode* pNode, int parentIndex);
		void ParseMesh(unsigned int meshIndex, const Bone& bone);
		void ParseBone(const aiBone* pBone, const int& nVertexStart);
		void ParseAnimation(const aiAnimation* pAnim);
		ModelAnimation CreateAnimation(const aiAnimation* pAnim, const AnimInfo* pAnimInfo);
		void ClipAnimation(const uint32& timeStart, const uint32& timeEnd);
		bool ReachMaxMaterials() const;
		bool HasMesh() const;
		void FillData(CParaXModel* pModel);
		void ClearData();

		void ParseMaterialByName(const std::string& sMatName, FBXMaterial2* out);
		int GetDiffuseTexIndex(const string& strTexName) const;
		int CreateOrGetBoneIndex(const string& strBoneName);

		//////////////////////////////////////////////////////////////////////////
		string m_strFilePath;
		string m_strAnimConfig;
		const aiScene* m_pFBXScene;

		// assimp解析出来的数据
		vector<FBXMaterial2> m_vecMaterials;
		vector<TextureEntity*> m_vecTexEnity;
		vector<ModelTransparency> m_vecTransparencys;

		// 需保存到CParaXModel中的数据
		vector<Bone> m_vecBones;
		map<string, int> m_mapBones;
		vector<ModelVertex> m_vecVertices;
		vector<uint16> m_vecIndices;
		vector<ModelRenderPass> m_vecPasses;
		vector<ModelGeoset> m_vecGeoset;
		Vector3 minExtent;
		Vector3 maxExtent;
		// 动画配置
		FBXModelInfo m_cAnimConfig;
		vector<ModelAnimation> m_vecAnims;


	};
}