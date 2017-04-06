#pragma once

#if defined(WIN32) && (defined(USE_DIRECTX_RENDERER)  || defined(USE_OPENGL_RENDERER))
#ifndef SUPPORT_FBX_MODEL_FILE
#define SUPPORT_FBX_MODEL_FILE
#endif

#include "XFileParser.h"
#include "ParaXModel.h"
#include "modelheaders.h"
#include "FBXModelInfo.h"
#include "FBXMaterial.h"

// assimp declare
struct aiScene;
struct aiMesh;
struct aiNode;
struct aiBone;
struct aiAnimation;

namespace ParaEngine
{
	struct FBXMaterial;

	/** we used our modified source of assimp to read FBX file.  
	assimp code is modified so that embedded textures can be read and used. 
	this is actually a universal parser for any assimp supported file format. 

	Multi-animation is supported in following ways:
		1. Use an external xml file with the same name as the import file in the same directory, see AnimationSplitter. 
		2. Assume [0-10s] is animation 0,   [40s-50s] is animation 4. Since most animation is less than 10 seconds, 
		   we will use start_time/10s as the animation id, and the last key time in that 10 seconds range as the animation range. 
	*/
	class FBXParser
	{
	public:
		FBXParser();
		FBXParser(const std::string& filename);
		~FBXParser();

		const std::string& GetFilename() const;
		void SetFilename(const std::string& val);
		
		/* Note the caller is responsible for deletion of the returned object.
		* parse as animated model
		*/
		CParaXModel* ParseParaXModel(const char* buffer, int nSize);
		/** use current filename if any. */
		CParaXModel* ParseParaXModel();

		
		/* Note the caller is responsible for deletion of the returned object.
		* parse as static model
		*/
		XFile::Scene* ParseFBXFile(const char* buffer, int nSize);
		XFile::Scene* ParseFBXFile();

		void Reset();
		void ResetAABB();

	protected:
		LinearColor GetRGBA(int colorTag);
		Vector2 GetTextureCoords();
		void FillMaterial(XFile::Material *pMaterial);
		void FillMaterialList(aiMesh *pFbxMesh, XFile::Mesh *pMesh);
		void FillTextureCoords(aiMesh *pFbxMesh, XFile::Mesh *pMesh);
		void ProcessStaticFBXMesh(aiMesh *pFbxMesh, XFile::Mesh *pMesh);

		void ProcessStaticFBXMaterial(const aiScene* pFbxScene, unsigned int iIndex);


		void CalculateMinMax(const Vector3& v);
		void ConvertFBXBone(ParaEngine::Bone& bone, const aiBone *pfbxBone);
		// the parentBoneIndex value is "-1" until find the node is a bone;
		void ProcessFBXBoneNodes(const aiScene* pFbxScene, aiNode* pFbxNode, int parentBoneIndex, CParaXModel *pMesh);
		void ProcessFBXMesh(const aiScene* pFbxScene, aiMesh *pFbxMesh, aiNode* pFbxNode, CParaXModel *pMesh);
		void ProcessFBXMaterial(const aiScene* pFbxScene, unsigned int iIndex, CParaXModel *pMesh);
		void ProcessFBXAnimation(const aiScene* pFbxScene, unsigned int iIndex, CParaXModel *pMesh);

		void AddDefaultColors(CParaXModel *pMesh);
		void AddDefaultTransparency(CParaXModel *pMesh);
		void FillParaXModelData(CParaXModel *pMesh, const aiScene *pFbxScene);
		void PostProcessParaXModelData(CParaXModel *pMesh);

		void PrintDebug(const aiScene* pFbxScene);
		/* make axis Y UP, by default FBX uses Z up as in 3dsmax. But we use Y up axis in ParaEngine */
		void MakeAxisY_UP();

		bool HasAnimations();
	
		int CreateGetBoneIndex(const char* pNodeName);
		std::string GetTexturePath(string textpath);
		ModelAnimation CreateModelAnimation(aiAnimation* pFbxAnim, ParaEngine::AnimInfo* pAnimInfo, int AnimIndex, bool beEndAnim);
		void SetAnimSplitterFilename();

		/** parse material by name*/
		void ParseMaterialByName(const std::string& sMatName, FBXMaterial* out);
	private:
		std::string m_sFilename;

		XFile::Scene* m_pScene;
		unsigned int m_nMaterialIndex;
		FBXModelInfo m_modelInfo;
		vector<ModelVertex> m_vertices;
		vector<ParaEngine::Bone> m_bones;
		vector<uint16> m_indices;
		vector<FBXMaterial> m_textures;
		/** embedded texture filename and its binary data */
		map<string, string> m_textureContentMapping;
		vector<ModelAnimation> m_anims;
		map<string, int> m_boneMapping;
		asset_ptr<TextureEntity> *textures;
		Vector3 m_maxExtent, m_minExtent;
		int m_nRootNodeIndex;
		int m_unique_id;
		bool m_bUsedVertexColor;
		bool m_bHasSkinnedMesh;
		std::string m_sAnimSplitterFilename;
	};
}

#endif