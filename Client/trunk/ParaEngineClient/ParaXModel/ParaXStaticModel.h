#pragma once
#ifdef USE_DIRECTX_RENDERER
#include "StaticMesh.h"
#endif

#include "IAttributeFields.h"
#include "ParaXMaterial.h"
#include "ParaXRefObject.h"
#include "MeshHeader.h"
#include "ParaXStaticModelRenderPass.h"

namespace ParaEngine
{
	class CEffectFile;
	class CFaceGroup;
	class XFileParser;
	using namespace std;
	namespace XFile
	{
		struct Node;
		struct Mesh;
		struct Scene;
		struct Material;
	}

	enum XModelFileType
	{
		FileType_ParaX,
		FileType_FBX,
	};

	//-----------------------------------------------------------------------------
	// Name: class CParaXStaticModel
	/// Desc: Class for loading and rendering file-based meshes
	//-----------------------------------------------------------------------------
	class CParaXStaticModel : public IAttributeFields
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CParaXStaticModel; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CParaXStaticModel"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CParaXStaticModel, DumpTextureUsage_s, const char**)	{ *p1 = cls->DumpTextureUsage(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXStaticModel, GetPolyCount_s, int*)	{ *p1 = cls->GetPolyCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXStaticModel, GetPhysicsCount_s, int*)	{ *p1 = cls->GetPhysicsCount(); return S_OK; }

		/** get polycount of this mesh object */
		int GetPolyCount();
		/** get physics polycount of this mesh object */
		int GetPhysicsCount();
		/** get texture usage such as the number of textures and their sizes. */
		const char* DumpTextureUsage();

		/** set the mesh file data. if this is set we will load the mesh from the memory data instead of open the mesh file.
		* once the mesh has finished with the data, it will delete them using delete [] operator.
		*/
		void SetMeshFileData(const char * pData, int nSize);
		void DeleteMeshFileData();
	public:
		std::string					m_strName;

		MeshHeader				m_header;
		
		/// the file data from which to load the mesh
		char* m_pFileData;
		/// the file data size in bytes from which to load the mesh
		int m_pFileDataSize;

		/** get a list of XRef objects*/
		vector<ParaXRefObject> m_XRefObjects;

		/** a list of face group */
		vector<CFaceGroup*>  m_faceGroups;

		DWORD                   m_dwNumMaterials; // Materials for the mesh
		
		std::vector<ParaXStaticModelRenderPass> m_passes;
		// system buffers
		std::vector<mesh_vertex_normal> m_vertices;
		std::vector<int16> m_indices;
		// device buffers
		ParaVertexBuffer m_vertexBuffer;
		ParaIndexBuffer m_indexBuffer;

		/** get the XRef Object in this model. */
		int GetXRefObjectsCount(){ return (int)m_XRefObjects.size(); }

		/** Get XRef Objects By Index. */
		const ParaXRefObject* GetXRefObjectsByIndex(int nIndex);


		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID.
		* but it does not mean that all ID contains valid replaceable textures.
		* This function can be used to quickly decide whether the model contains replaceable textures.
		* Generally we allow 32 replaceable textures per model.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		virtual int GetNumReplaceableTextures();

		/**
		* get the default replaceable texture by its ID. The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetDefaultReplaceableTexture(int ReplaceableTextureID);

		/**
		* get the current replaceable texture by its ID.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID);

		/**
		* set the replaceable texture at the given index with a new texture. This function is called frequently before an instace of the mesh object draws itself.
		* because default instance of the mesh may use different replaceable texture set.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one.
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity);

		vector< ref_ptr<TextureEntity> >	m_ReplaceableTextures;

		/** return the physics group id that is closest to nPhysicsGroup. or -1 if there is none. */
		int GetNextPhysicsGroupID(int nPhysicsGroup = -1);

		bool                    m_bUseMaterials;

		/** build the texture entity. It will return NULL if the texture is not valid.
		* if the texture contains no directory information. the mesh directory will be used to locate the texture.
		* it will search for the current directory of the mesh file(m_strName)
		* @param sFilePath: texture file path.
		* @param TextureType: see TextureEntity::_SurfaceType
		*/
		TextureEntity*	LoadTextureByPath(const std::string& sFilePath, DWORD TextureType);

	private:
		/** clear all face groups. */
		void ClearFaceGroups();
	public:
		// Rendering
		HRESULT Render(SceneState * pSceneState, CEffectFile *pEffect,
			bool bDrawOpaqueSubsets = true,
			bool bDrawAlphaSubsets = true, float fAlphaFactor = 1.0f, CParameterBlock* materialParams = NULL);

		void DrawRenderPass(int i);

		// Rendering options
		void    UseMeshMaterials(bool bFlag) { m_bUseMaterials = bFlag; }
		
		// Initializing
		HRESULT InitDeviceObjects();
		HRESULT DeleteDeviceObjects();
		void RendererRecreated();
		void BuildRenderBuffer();

		// Creation/destruction
		/**
		* @param bCheckSecondUV: whether to check the second UV set. if there exist a second UV set, it will be created.
		* 2008.1.25 by LXZ: this is a tricky workaround of D3DXLoadMeshFromXOf can only load a simple mesh from data node. If the mesh contains frames or second UV sets, the function will not work. So in such cases, I fall back to use the original D3DXLoadMeshFromXInMemory, which can handle frames and second UV sets.
		*/
		HRESULT Create(RenderDevicePtr pd3dDevice, const char* strFilename, bool bCheckSecondUV = false);
		HRESULT Create(RenderDevicePtr pd3dDevice, char* buffer, DWORD nFileSize, bool bCheckSecondUV = false);
		HRESULT Create(RenderDevicePtr pd3dDevice, XFile::Scene* pFileData);

		static XModelFileType GetFileTypeByFilename(const::string& sFilename);

		HRESULT LoadToSystemBuffer(XFile::Scene* pFileData);
		HRESULT Destroy();

		// -- Get Bounding sphere
		FLOAT ComputeBoundingSphere(Vector3* vObjectCenter, FLOAT* fObjectRadius);
		HRESULT ComputeBoundingBox(Vector3 *pMin, Vector3 *pMax);

		
		/** get the mesh header. and compute if not valid.  one needs to check MeshHeader.m_bIsValid before using it.
		because if async loading is used, the mesh itself may not be ready when this function is called. */
		MeshHeader& GetMeshHeader(XFileParser* pFileParser = NULL);

		/** Get mesh header from file. */
		bool GetMeshHeaderFromFile(CParaFile& myFile, XFileParser* pFileParser);

		/**
		* Get the physics mesh in terms of vertices and indices.
		* @param pNumVertices [out] number of vertices
		* @param ppVerts [out] buffer contains all vertices. The caller needs to release the buffer using delete [] buffer.
		* @param pNumTriangles [out] number of triangles, each triangle has three indices.
		*	Please note that if the mesh contains no physics faces, the pNumTriangles is 0. However pNumVertices might be positive.
		* @param ppIndices [out] buffer contains all indices. The caller needs to release the buffer using delete [] buffer.
		* @param nMeshPhysicsGroup [in|out]: the mesh physics group to get. On return it will be assigned with the next mesh group.
		* @return S_OK, if succeed.
		*/
		HRESULT ClonePhysicsMesh(DWORD* pNumVertices, Vector3 ** ppVerts, DWORD* pNumTriangles, WORD** ppIndices, int* pnMeshPhysicsGroup = NULL, int* pnTotalMeshGroupCount = NULL);

		/**
		* @param strName: the file name
		* @param pDXFileParser: the x file template parser to use. if NULL, the default one is used.
		*/
		CParaXStaticModel(const char* strName = NULL, XFileParser* pXFileParser = NULL);
		virtual ~CParaXStaticModel();
	};

#ifndef USE_DIRECTX_RENDERER
	typedef ref_ptr<CParaXStaticModel> CParaXStaticModelPtr;
	typedef CParaXStaticModel* CParaXStaticModelRawPtr;
#endif
}
