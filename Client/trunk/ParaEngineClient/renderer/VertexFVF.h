#pragma once

namespace ParaEngine
{
	struct my_vertex{
		//FLOAT x, y, z;  
		Vector3	 v;
		//FLOAT nx,ny,nz;
		Vector3	 n;
		DWORD colour;
		FLOAT tu, tv;  
	};

	/** matrix vertex */
	struct MatrixVertex
	{
	public:
		MatrixVertex(Vector4& i1,Vector4& i2,Vector4&i3,Vector4&i4)
			: r1(i1),r2(i2),r3(i3){}
		Vector4 r1;     // row 1
		Vector4 r2;     // row 2
		Vector4 r3;		// row 3
		// No FVF for this type
		// static const DWORD FVF;
	};


	/** Custom vertex type for the under water effect*/
	struct UNDERWATER_VERTEX
	{
		Vector4 p;
		Color    color;
		static const DWORD FVF;
	};

	/** for general mesh without normal and color */
	struct mesh_vertex_plain
	{
		Vector3	 p;
		Vector2 uv;
		static const DWORD FVF;
	};

	/** for general mesh with normal */
	struct mesh_vertex_normal
	{
		Vector3	 p;
		Vector3	 n;
		Vector2 uv;
		static const DWORD FVF;
	};
	struct mesh_vertex_normal_color
	{
		Vector3	 p;
		Vector3	 n;
		DWORD color;
		Vector2 uv;
		static const DWORD FVF;
	};
	struct block_vertex
	{
		Vector3	 p;
		Vector3	 n;
		DWORD color;
		DWORD color2;
		Vector2 uv;
		static const DWORD FVF;
	};
	struct mesh_vertex_normal_tex2
	{
		Vector3	 p;
		Vector3	 n;
		Vector2 uv;
		Vector2 uv2;
		static const DWORD FVF;
	};
	struct bmax_vertex
	{
		Vector3	 p;
		Vector3	 n;
		DWORD color;
		static const DWORD FVF;
	};

	/** for terrain */
	struct terrain_vertex
	{
		Vector3	 v;
		FLOAT tu1, tv1;  
		FLOAT tu2, tv2;
		static const DWORD FVF;
	};

	/** for terrain with normal*/
	struct terrain_vertex_normal
	{
		Vector3	 v;
		Vector3  n;

		FLOAT tu1, tv1;  
		FLOAT tu2, tv2;
		static const DWORD FVF;
	};

	/** Custom vertex type for the shadows*/
	struct SHADOWVERTEX
	{
		Vector4 p;
		Color    color;

		static const DWORD FVF;
	};

	/** Custom vertex type for the bounding box*/
	struct LINEVERTEX
	{
		/// Vertex position
		Vector3 p;      
		/// Vertex color
		DWORD       color;

		static const DWORD FVF;
	};
	/** Custom vertex type for the occlusion object */
	struct OCCLUSION_VERTEX
	{
		/// Vertex position
		Vector3 p;
		static const DWORD FVF;
	};

	/** 2D screen with textures. */
	struct DXUT_SCREEN_VERTEX
	{
		float x, y, z, h;
		Color color;
		float tu, tv;
		inline void SetPosition(const Vector3& pos, float w = 1.0f){
			x = pos.x; y = pos.y; z = pos.z; h = w;
		}
		inline Vector3* GetPosPointer(){ return (Vector3*)(&x); }
		static const DWORD FVF;
	};

	/// Custom vertex type for the trees
	struct SPRITEVERTEX
	{
		/// Vertex position
		Vector3 p;      
		/// Vertex color
		DWORD       color;  
		/// Vertex texture coordinates
		FLOAT       tu, tv; 
		inline void SetPosition(const Vector3& pos){
			p = pos;
		}
		inline Vector3* GetPosPointer(){ return (Vector3*)(&p); }
		static const DWORD FVF;
	};
}