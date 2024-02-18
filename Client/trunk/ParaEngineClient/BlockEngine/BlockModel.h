
#pragma once

#include "ShapeAABB.h"

namespace ParaEngine
{
	class IBlockModelProvider;
	class BlockModelManager;
	class BlockTemplate;

	class BlockVertexCompressed
	{
	public:
		float position[3];  //4byte
		float normal[3];  //4byte
		
		/** for fixed function: ao_shadow, max_light, max_light, max_light 
		for shader:  ao_shadow, sun_light, block_light, block_id
		*/
		DWORD color;	  //4byte;
		/** color of the block, default to white. Only used in color block.*/
		DWORD color2;	  //4byte;
		float texcoord[2];  //8byte
		
	public:
		BlockVertexCompressed()
			:color(0xffffffff), color2(0xffffffff)
		{
		}

		inline void SetPosition(float x,float y,float z)
		{
			position[0] = x;
			position[1] = y;
			position[2] = z;
		}

		inline void SetPosition(const Vector3& vPos)
		{
			position[0] = vPos.x;
			position[1] = vPos.y;
			position[2] = vPos.z;
		}

		inline void SetPosition(const BlockVertexCompressed& r)
		{
			position[0] = r.position[0];
			position[1] = r.position[1];
			position[2] = r.position[2];
		}

		void GetPosition(Vector3& vPos)
		{
			vPos.x = position[0];
			vPos.y = position[1];
			vPos.z = position[2];
		}

		Vector3 GetPosition()
		{
			return Vector3(position[0], position[1], position[2]);
		}

		inline void SetNormal(float x,float y,float z)
		{
			normal[0] = x;
			normal[1] = y;
			normal[2] = z;
		}

		inline void SetNormal(const Vector3& vNormal)
		{
			normal[0] = vNormal.x;
			normal[1] = vNormal.y;
			normal[2] = vNormal.z;
		}

		void GetNormal(Vector3& vPos)
		{
			vPos.x = normal[0];
			vPos.y = normal[1];
			vPos.z = normal[2];
		}

		Vector3 GetNormal()
		{
			return Vector3(normal[0], normal[1], normal[2]);
		}

		/** get the face id according to normal.-1 for none. 0-5 for the 6 faces. */
		int GetCubeFaceId() const;

		/** if normal is zero, this is a hidden face. */
		inline bool BlockVertexCompressed::IsEmptyFace() const
		{
			return normal[0] == 0 && normal[1] == 0 && normal[2] == 0;
		}

		inline void OffsetPosition(float dx,float dy,float dz)
		{
			position[0] += dx;
			position[1] += dy;
			position[2] += dz;
		}

		inline void OffsetPosition(const Vector3& v)
		{
			position[0] += v.x;
			position[1] += v.y;
			position[2] += v.z;
		}

		inline void SetHeightScale(float scale)
		{
			position[1] *= scale;
		}

		inline void SetTexcoord(float u,float v)
		{
			texcoord[0] = u;
			texcoord[1] = v;
		}

		inline void SetTexcoord(const Vector2& v)
		{
			texcoord[0] = v.x;
			texcoord[1] = v.y;
		}


		inline void GetTexcoord(float& u,float& v)
		{
			u = texcoord[0];
			v = texcoord[1];
		}

		inline Vector2 GetTexcoord()
		{
			return Vector2(texcoord[0], texcoord[1]);
		}
		
		inline void SetLightColor(DWORD color_)
		{
			color = color_;
		}

		/** only used for fixed function rendering */
		inline void SetLightIntensity(float amount)
		{
			DWORD light = (DWORD)(amount * 255);
			color = (color&0xff000000) | (light<<16 | light<<8 | light);
		}

		/** used for shader rendering */
		inline void SetVertexLight(DWORD nBlockLight, DWORD nSunLight)
		{
			color = (color&0xff0000ff) | (nSunLight<<16 | nBlockLight<<8);
		}

		/** used for shader rendering */
		inline void SetCategoryID(DWORD nBlockID)
		{
			color = (color&0xffffff00) | nBlockID;
		}

		/** make it darker, each time this function is called. 
		*/
		inline void SetShadow()
		{
			// color = (180<<24) | (color&0x00ffffff);
			color = (((byte)(color>>24)-45)<<24) | (color&0x00ffffff);
		}

		/** similar to SetShadow, except that it uses a user provided value [0,255] */
		inline void SetColorStrength(byte strength)
		{
			color = (strength<<24) | (color&0x00ffffff);
		}

		
		inline void SetBlockColor(DWORD color)
		{
			color2 = color;
		}
	};

	class SelectBlockVertex
	{
	public:
		Vector3 position;
		Vector2 texcoord;
	};



	class BlockModel
	{
	public:
		friend class BlockModelManager;
		enum EdgeVertexFlag
		{
			evf_none = 0,
			//edge
			evf_topFront	= 0x001,
			evf_topLeft		= 0x002,
			evf_topRight	= 0x004,
			evf_topBack		= 0x008,
			evf_LeftFront	= 0x010,
			evf_leftBack	= 0x020,
			evf_rightFont	= 0x040,
			evf_rightBack	= 0x080,
			evf_bottomFront	= 0x100,
			evf_bottomLeft	= 0x200,
			evf_bottomRight	= 0x400,
			evf_bottomBack	= 0x800,

			//vertex
			evf_xyz		= 0x01000,
			evf_xyNz	= 0x02000,
			evf_xNyz	= 0x04000,
			evf_xNyNz	= 0x08000,

			evf_Nxyz	= 0x10000,
			evf_NxyNz	= 0x20000,
			evf_NxNyz	= 0x40000,
			evf_NxNyNz  = 0x80000,
		};

		BlockModel(int32_t texFaceNum=0);

		/** clone the given block model. */
		void CloneRenderData(const BlockModel& from_block);
		void Clone(const BlockModel& from_block);

		/** clear all vertices */
		void ClearVertices();
		/** add a new vertex
		* @return the added vertex index. 
		*/
		int AddVertex(const BlockVertexCompressed& vertex);
		/** add vertex from another block. 
		* @return the added vertex index.
		*/
		int AddVertex(const BlockModel& from_block, int32 nVertexIndex);
		void CloneVertices(const BlockModel& from_block);
		
		/** vertices */
		std::vector<BlockVertexCompressed>& Vertices();

		void SetAOMask(uint32_t edges);

		void SetVerticalScale(EdgeVertexFlag vertexId,float scale);

		/** offset the vertices */
		void TranslateVertices(float dx, float dy, float dz);

		/** only used for fixed function pipeline. 
		*/
		inline void SetLightIntensity(uint32_t vertexId,float value)
		{
			m_Vertices[vertexId].SetLightIntensity(value);
		}

		/** 
		@param nBlockLight: 0-255
		@param nSunLight: 0-255
		*/
		void SetVertexLight(uint32_t vertexId, uint8_t nBlockLight, uint8_t nSunLight)
		{
			m_Vertices[vertexId].SetVertexLight(nBlockLight, nSunLight);
		}

		/** set category id. */
		void SetCategoryID(DWORD nCategoryID);


		/** get bounding box vertices */
		void GetBoundingBoxVertices(Vector3 * pVertices, int* pNumber);
		static void GetBoundingBoxVertices(CShapeAABB& aabb, Vector3 * pVertices, int* pNumber);

		/** whether AABB is a cube */
		bool IsCubeAABB();
		void SetIsCubeAABB(bool bIsCube);

		/** get axis aligned bounding box for the model.*/
		void GetAABB(CShapeAABB* pOut) const;
		const CShapeAABB& GetAABB() const;
		void SetAABB(const Vector3& vMin, const Vector3& vMax);
		
		/** whether use ambient occlusion. */
		bool IsUseAmbientOcclusion(){return m_bUseAO;};
		void SetUseAmbientOcclusion(bool bValue){m_bUseAO = bValue;};

		/** get face count. */
		int GetFaceCount(){return m_nFaceCount;}
		void SetFaceCount(int nFaceCount);
		int IncrementFaceCount(int nDelta = 1);
		/** get number of vertices */
		int GetVerticesCount();
		void ReserveVertices(int nReservedSize = 24);

		/** Is disable face culling */
		bool IsDisableFaceCulling(){return m_bDisableFaceCulling;};
		void SetDisableFaceCulling(bool bDisabled){ m_bDisableFaceCulling = bDisabled; };

		/** whether to disable block and sun lighting when rendering this block. */
		bool IsUsingSelfLighting() {return m_bUseSelfLighting;}
		void SetUsingSelfLighting(bool bEnable) {m_bUseSelfLighting = bEnable;}

		/** whether all vertices have same lighting value. */
		bool IsUniformLighting() const { return m_bUniformLighting; }
		void SetUniformLighting(bool val) { m_bUniformLighting = val; }

		/** transform each vertex of the model by this mat. */
		void Transform(const Matrix4& mat);
		/** transform by scale and translate */
		void Transform(const Vector3& vOffset, float fScaling);

		/** remove a given face by making infinite small triangle. */
		void RemoveFace(int nFirstIndex);

		/** get vertice array */
		BlockVertexCompressed* GetVertices();
		const BlockVertexCompressed* GetVerticesConst() const;

		/** get texture index into the Block template's texture array. */
		int GetTextureIndex() const;
		void SetTextureIndex(int val);
		/** set the uniform block color. default to white*/
		void SetColor(DWORD color);

		/** load static model from parax file. */
		void LoadModel(BlockTemplate* pTemplate, const std::string& filename, const Matrix4& mat, int nTextureIndex = 0);
		void LoadModelByTexture(int32_t texFaceNum);
		void LoadCubeModel();
		/** load from known model name */
		void LoadModel(const std::string& sModelName);
		/** set the vertex in shadow as in ao calculation. 
		* @param nShadowLevel: [0,255]. 255 is completely dark. 0 is un-shadowed
		*/
		void SetVertexShadow(int nIndex, unsigned char nShadowLevel);
		void SetVertexShadowFromAOFlags(int nIndex, int nCubeIndex, uint32 aoFlags);
		void SetVertexColor(int nIndex, DWORD color);
		
		/** @return [0,255] the shadow level from current ambient occlusion flags.*/
		unsigned char CalculateCubeVertexAOShadowLevel(int nIndex, uint32 aoFlags);

		void SetVertexHeightScale(int nIndex, float scale);

		/** assume that 4 vertices(two triangles) are a rect face. There can be 2 overlapping vertices. */
		void RecalculateNormals();
		Vector3 RecalculateNormalsOfRectFace(int startIdxOfFace);

		/** 
		* @param nFaceIndex: [0,5]
		*/
		inline uint8 GetFaceShape(int nFaceIndex) const
		{
			return m_faceShape[nFaceIndex];
		}

		/* sort the first 6 faces +x, -x, +y,-y,+z, -z and calculate its face shape. */
		void RecalculateFaceShapeAndSortFaces();

		/** if m_faceShape is calculated and used in rendering. */
		inline bool HasFaceShape() const
		{
			return m_bHasFaceShape;
		}

		// for debugging only
		void DumpToLog();
		//
		//    LT  -----  RT
		//       |     |
		//       |     |
		//    LB  -----  RB   
		//vertex indices
		//top face
		static const int32_t g_topLB = 0;
		static const int32_t g_topLT = 1;
		static const int32_t g_topRT = 2;
		static const int32_t g_topRB = 3;

		//front face
		static const int32_t g_frtLB = 4;
		static const int32_t g_frtLT = 5;
		static const int32_t g_frtRT = 6;
		static const int32_t g_frtRB = 7;

		//bottom face
		static const int32_t g_btmLB = 8;
		static const int32_t g_btmLT = 9;
		static const int32_t g_btmRT = 10;
		static const int32_t g_btmRB = 11;

		//left face
		static const int32_t g_leftLB = 12;
		static const int32_t g_leftLT = 13;
		static const int32_t g_leftRT = 14;
		static const int32_t g_leftRB = 15;

		//right face
		static const int32_t g_rightLB = 16;
		static const int32_t g_rightLT = 17;
		static const int32_t g_rightRT = 18;
		static const int32_t g_rightRB = 19;

		//back face
		static const int32_t g_bkLB = 20;
		static const int32_t g_bkLT = 21;
		static const int32_t g_bkRT = 22;
		static const int32_t g_bkRB = 23;
		
	private:
		/** all vertices */
		std::vector<BlockVertexCompressed> m_Vertices;
		/** face shape. the lower 4 bits are 1 if one of the four corners has a vertex. for example, a cube is 0xf for each face. 
		* the higher 4 bits represents irregular shape if not 0.
		* we may remove the face during rendering if the neighbor block' is solid's face has the same face shape value.
		*/
		uint8 m_faceShape[6];
		/** if m_faceShape is calculated and used in rendering. */
		bool m_bHasFaceShape;

		/** since we use triangle list. this is 24 for cube model, and 12 for */
		bool m_bUseAO;
		/** disable culling */
		bool m_bDisableFaceCulling;

		/** disable block light and sun light when rendering this block. */
		bool m_bUseSelfLighting;
		/** all vertices have same lighting value. */
		bool m_bUniformLighting;
		
		/** whether AABB is a cube*/
		bool m_bIsCubeAABB;

		/** default to 6 faces, with 12 triangles and 24 vertices*/
		int m_nFaceCount;
		/** index to the parent block template's texture array */
		int m_nTextureIndex;
		
		/** the aabb bounding box */
		CShapeAABB m_shapeAABB;

		friend class IBlockModelProvider;
	};
}

