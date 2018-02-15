#pragma once
#include "IAttributeFields.h"
#include "ParaXMaterial.h"
#include "ParaXRefObject.h"
#include "ParaXModel/MeshHeader.h"

namespace ParaEngine
{
	class CEffectFile;
	class CFaceGroup;
	class XFileParser;

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



	using namespace std;


	class CParaXStaticBase : public IAttributeFields
	{
	public:
		CParaXStaticBase(const char* strName = nullptr);
		virtual ~CParaXStaticBase();

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CParaXStaticBase, DumpTextureUsage_s, const char**) { *p1 = cls->DumpTextureUsage(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXStaticBase, GetPolyCount_s, int*) { *p1 = cls->GetPolyCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXStaticBase, GetPhysicsCount_s, int*) { *p1 = cls->GetPhysicsCount(); return S_OK; }

		/** get polycount of this mesh object */
		virtual int GetPolyCount()				= 0;
		/** get physics polycount of this mesh object */
		virtual int GetPhysicsCount()			= 0;
		/** get texture usage such as the number of textures and their sizes. */
		virtual const char* DumpTextureUsage()	= 0;
		
		
		/**
		* get the default replaceable texture by its ID. The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetDefaultReplaceableTexture(int ReplaceableTextureID) = 0;

		/**
		* get the current replaceable texture by its ID.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		TextureEntity* GetReplaceableTexture(int ReplaceableTextureID);

		/**
		* set the replaceable texture at the given index with a new texture. This function is called frequently before an instace of the mesh object draws itself.
		* because default instance of the mesh may use different replaceable texture set.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one.
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity);


		/** return the physics group id that is closest to nPhysicsGroup. or -1 if there is none. */
		virtual int GetNextPhysicsGroupID(int nPhysicsGroup = -1) = 0;


		/** build the texture entity. It will return NULL if the texture is not valid.
		* if the texture contains no directory information. the mesh directory will be used to locate the texture.
		* it will search for the current directory of the mesh file(m_strName)
		* @param sFilePath: texture file path.
		* @param TextureType: see TextureEntity::_SurfaceType
		*/
		virtual TextureEntity*	LoadTextureByPath(const string& sFilePath, DWORD TextureType);


		// Creation/destruction
		/**
		* @param bCheckSecondUV: whether to check the second UV set. if there exist a second UV set, it will be created.
		* 2008.1.25 by LXZ: this is a tricky workaround of D3DXLoadMeshFromXOf can only load a simple mesh from data node. If the mesh contains frames or second UV sets, the function will not work. So in such cases, I fall back to use the original D3DXLoadMeshFromXInMemory, which can handle frames and second UV sets.
		*/
		HRESULT Create(RenderDevicePtr pRenderDevice, const char* strFilename, bool bCheckSecondUV = false);

		virtual HRESULT Create(RenderDevicePtr pRenderDevice, void* buffer, DWORD nFileSize, bool bCheckSecondUV = false)			= 0;
#ifdef USE_DIRECTX_RENDERER
		virtual HRESULT Create(RenderDevicePtr pRenderDevice, LPD3DXFILEDATA pFileData)											= 0;
#endif
		virtual HRESULT Create(RenderDevicePtr pRenderDevice, XFile::Scene* pFileData)												= 0;

		virtual HRESULT Destroy();


		// Initializing
		virtual HRESULT InitDeviceObjects()																						= 0;
		virtual HRESULT DeleteDeviceObjects()																					= 0;

		// -- Get Bounding sphere
		virtual FLOAT ComputeBoundingSphere(Vector3* vObjectCenter, FLOAT* fObjectRadius)										= 0;
		virtual HRESULT ComputeBoundingBox(Vector3 *pMin, Vector3 *pMax)														= 0;

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
		virtual HRESULT ClonePhysicsMesh(DWORD* pNumVertices, Vector3 ** ppVerts, DWORD* pNumTriangles, WORD** ppIndices, int* pnMeshPhysicsGroup = nullptr, int* pnTotalMeshGroupCount = nullptr) = 0;

		/** get the mesh header. and compute if not valid.  one needs to check MeshHeader.m_bIsValid before using it.
		because if async loading is used, the mesh itself may not be ready when this function is called. */
#ifdef USE_DIRECTX_RENDERER
		virtual MeshHeader& GetMeshHeader(LPD3DXFILE pFileParser = nullptr) = 0;
#endif
		virtual MeshHeader& GetMeshHeader(XFileParser* pFileParser = nullptr) = 0;

#ifdef USE_DIRECTX_RENDERER
		/** Get mesh header from file. */
		virtual bool GetMeshHeaderFromFile(CParaFile& myFile, LPD3DXFILE pFileParser) = 0;
#endif
		virtual bool GetMeshHeaderFromFile(CParaFile& myFile, XFileParser* pFileParser) = 0;

		// Rendering
		virtual HRESULT Render(SceneState * pSceneState, RenderDevicePtr pRenderDevice,
			bool bDrawOpaqueSubsets = true,
			bool bDrawAlphaSubsets = true, float fAlphaFactor = 1.0f) = 0;
		virtual HRESULT Render(SceneState * pSceneState, CEffectFile *pEffect,
			bool bDrawOpaqueSubsets = true,
			bool bDrawAlphaSubsets = true, float fAlphaFactor = 1.0f, CParameterBlock* materialParams = nullptr) = 0;


		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID.
		* but it does not mean that all ID contains valid replaceable textures.
		* This function can be used to quickly decide whether the model contains replaceable textures.
		* Generally we allow 32 replaceable textures per model.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		int GetNumReplaceableTextures();

		/** set the mesh file data. if this is set we will load the mesh from the memory data instead of open the mesh file.
		* once the mesh has finished with the data, it will delete them using delete [] operator.
		*/
		void SetMeshFileData(void * pData, int nSize);
		void DeleteMeshFileData();

		/** get the XRef Object in this model. */
		int GetXRefObjectsCount() { return (int)m_XRefObjects.size(); }
		/** Get XRef Objects By Index. */
		const ParaXRefObject* GetXRefObjectsByIndex(int nIndex);

		// Rendering options
		void    UseMeshMaterials(bool bFlag) { m_bUseMaterials = bFlag; }


		static XModelFileType GetFileTypeByFilename(const std::string& sFilename);
	protected:
		/** clear all face groups. */
		void ClearFaceGroups();
	protected:
		string					m_strName;
		MeshHeader				m_header;

		/// the file data from which to load the mesh
		char*					m_pFileData;
		/// the file data size in bytes from which to load the mesh
		int						m_pFileDataSize;
		/** get a list of XRef objects*/
		vector<ParaXRefObject>	m_XRefObjects;
		/** a list of face group */
		vector<CFaceGroup*>		m_faceGroups;

		DWORD                   m_dwNumMaterials; // Materials for the mesh

		bool                    m_bUseMaterials;

		vector< ref_ptr<TextureEntity> >	m_ReplaceableTextures;
	};

} // end namespace