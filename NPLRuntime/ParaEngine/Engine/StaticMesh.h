#pragma once
#include "ParaXStaticBase.h"

namespace ParaEngine
{
	class CEffectFile;
	class CFaceGroup;
	using namespace std;

	//-----------------------------------------------------------------------------
	// Name: class CParaXStaticMesh
	/// Desc: Class for loading and rendering file-based meshes
	//-----------------------------------------------------------------------------
	class CParaXStaticMesh : public CParaXStaticBase
	{
	public:
		/**
		* @param strName: the file name
		* @param pDXFileParser: the x file template parser to use. if NULL, the default one is used.
		*/
		CParaXStaticMesh(const char* strName = nullptr, LPD3DXFILE	pDXFileParser = nullptr);
		virtual ~CParaXStaticMesh();


		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CDXUTMesh; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CDXUTMesh"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }

		/** get polycount of this mesh object */
		virtual int GetPolyCount();
		/** get physics polycount of this mesh object */
		virtual int GetPhysicsCount();
		/** get texture usage such as the number of textures and their sizes. */
		virtual const char* DumpTextureUsage();


		/** return the physics group id that is closest to nPhysicsGroup. or -1 if there is none. */
		virtual int GetNextPhysicsGroupID(int nPhysicsGroup = -1);

		/**
		* get the default replaceable texture by its ID. The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetDefaultReplaceableTexture(int ReplaceableTextureID);


		// Creation/destruction
		/**
		* @param bCheckSecondUV: whether to check the second UV set. if there exist a second UV set, it will be created.
		* 2008.1.25 by LXZ: this is a tricky workaround of D3DXLoadMeshFromXOf can only load a simple mesh from data node. If the mesh contains frames or second UV sets, the function will not work. So in such cases, I fall back to use the original D3DXLoadMeshFromXInMemory, which can handle frames and second UV sets.
		*/
		virtual HRESULT Create(RenderDevicePtr pRenderDevice, void* buffer, DWORD nFileSize, bool bCheckSecondUV = false);
#ifdef USE_DIRECTX_RENDERER
		virtual HRESULT Create(RenderDevicePtr pRenderDevice, LPD3DXFILEDATA pFileData);
#endif
		virtual HRESULT Create(RenderDevicePtr pRenderDevice, XFile::Scene* pFileData) { return S_FALSE;  };

		virtual HRESULT Destroy();

		// Initializing
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		// -- Get Bounding sphere
		virtual FLOAT ComputeBoundingSphere(Vector3* vObjectCenter, FLOAT* fObjectRadius);
		virtual HRESULT ComputeBoundingBox(Vector3 *pMin, Vector3 *pMax);

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
		virtual HRESULT ClonePhysicsMesh(DWORD* pNumVertices, Vector3 ** ppVerts, DWORD* pNumTriangles, WORD** ppIndices, int* pnMeshPhysicsGroup = nullptr, int* pnTotalMeshGroupCount = nullptr);


		/** get the mesh header. and compute if not valid.  one needs to check MeshHeader.m_bIsValid before using it.
		because if async loading is used, the mesh itself may not be ready when this function is called. */
#ifdef USE_DIRECTX_RENDERER
		virtual MeshHeader& GetMeshHeader(LPD3DXFILE pFileParser = nullptr);
#endif
		virtual MeshHeader& GetMeshHeader(XFileParser* pFileParser = nullptr);

#ifdef USE_DIRECTX_RENDERER
		/** Get mesh header from file. */
		virtual bool GetMeshHeaderFromFile(CParaFile& myFile, LPD3DXFILE pFileParser);
#endif
		virtual bool GetMeshHeaderFromFile(CParaFile& myFile, XFileParser* pFileParser) { return false;  };

		// Rendering
		virtual HRESULT Render(SceneState * pSceneState, RenderDevicePtr pRenderDevice,
			bool bDrawOpaqueSubsets = true,
			bool bDrawAlphaSubsets = true, float fAlphaFactor = 1.0f);
		virtual HRESULT Render(SceneState * pSceneState, CEffectFile *pEffect,
			bool bDrawOpaqueSubsets = true,
			bool bDrawAlphaSubsets = true, float fAlphaFactor = 1.0f, CParameterBlock* materialParams = nullptr);

		virtual HRESULT RendererRecreated() override;

	public:
		// Mesh access
		LPD3DXMESH GetSysMemMesh()	{ return m_pSysMemMesh; }
		LPD3DXMESH GetLocalMesh()	{ return m_pLocalMesh; }

		LPD3DXMESH& GetSysMemMeshRef() { return m_pSysMemMesh; }
		LPD3DXMESH& GetLocalMeshRef() { return m_pLocalMesh; }

		/** get the file parser */
		static HRESULT GetMeshHeader(LPCSTR strFilename, LPD3DXFILE pFileParser, Vector3& vMin, Vector3& vMax, bool& bHasNormal, bool& bHasTex2);
	protected:
		HRESULT CreateMaterials(const char* strPath, IRenderDevice *pRenderDevice, ID3DXBuffer *pAdjacencyBuffer, ID3DXBuffer *pMtrlBuffer);
		/** a wrapper for DrawSubset().*/
		HRESULT DrawSubSetEx(int nIndex, D3DXATTRIBUTERANGE* pAtts);
	protected:
		CParaXMaterial*         m_pMaterials;
		
		LPD3DXFILE				m_pDXFileParser;	// x file template parsing
		LPD3DXMESH              m_pSysMemMesh;    // SysMem mesh, lives through resize
		LPD3DXMESH              m_pLocalMesh;     // Local mesh, rebuilt on resize
	};


#ifdef USE_DIRECTX_RENDERER
	typedef asset_ptr<CParaXStaticMesh> CParaXStaticModelPtr;
	typedef CParaXStaticMesh* CParaXStaticModelRawPtr;
#endif
}