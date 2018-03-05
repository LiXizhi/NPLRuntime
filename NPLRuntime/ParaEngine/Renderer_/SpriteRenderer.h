#pragma once

namespace ParaEngine
{
	struct TextureEntity;

	struct sprite_vertex
	{
		sprite_vertex() : tex(0.f, 0.f), col(0) {}
		Vector3 pos;
		DWORD col;
		Vector2 tex;
	};

	struct SpriteQuad
	{
		DeviceTexturePtr_type texture;
		UINT texw, texh;
		RECT rect;
		Vector3 center;
		Vector3 pos;
		DWORD color;
		Matrix4 transform;
	};

	struct SpriteTriangle
	{
		DeviceTexturePtr_type texture;
		UINT texw, texh;
		RECT rect;
		Vector3 vertices[3];
		DWORD color;
		Matrix4 transform;
	};

	struct SpriteThickLine
	{
		DeviceTexturePtr_type texture;
		UINT texw, texh;
		RECT rect;
		Vector3 vStart;
		Vector3 vEnd;
		DWORD color;
		float Thickness;
		Matrix4 transform;
	public:
		Vector3 normalVector();
		/** return a 2d normal that is perpendicular to line (x1,y1)-(x2,y2). 
		* @param fLineWidth: half of the line thickness. default to 0.5f. 
		*/
		static void normalVector(float x1, float y1, float x2, float y2, float *nx, float *ny, float fLineWidth = 0.5f);
	};

	/** base class for sprite renderer */
	class CSpriteRenderer
	{
	public:
		/* create based on current implementation. */
		static CSpriteRenderer* Create();

		CSpriteRenderer();;
		virtual ~CSpriteRenderer(){};
		
	public:
		virtual HRESULT Begin(DWORD Flags);
		virtual void End();
		virtual void Flush();

		/** call BeginCustomShader and EndCustomShader. between Begin() and End(), if one wants to render 
		using custom shader during sprite rendering. This is usually the case for GUI text rendering. */
		virtual bool BeginCustomShader(){ return true; };
		virtual void EndCustomShader(){};

		virtual void UpdateShader(bool bForceUpdate = false) {};
		virtual void SetNeedUpdateShader(bool bNeedUpdate) {};

		virtual HRESULT DrawRect(const RECT* pRect, Color color, float depth);

		virtual HRESULT DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c);

		virtual HRESULT DrawLine(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3& vStart, const Vector3& vEnd, float thickness, Color c);

		/** draw triangle list 
		* @param pVertices: must have nTriangleCount*3 vertices
		*/
		virtual HRESULT DrawTriangleList(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* pVertices, int nTriangleCount, Color c);

		virtual HRESULT SetTransform(const Matrix4& mat);
		virtual void GetTransform(Matrix4& transform);
		virtual const Matrix4& GetTransform() const;
		virtual float GetFontScaling() const;

		virtual void InitDeviceObjects(){};
		virtual void RestoreDeviceObjects(){};
		virtual void InvalidateDeviceObjects(){};
		virtual void DeleteDeviceObjects(){};

		/** whether to set text mode.*/
		virtual void SetTextMode(bool bIsTextMode = true) {};
		virtual void PrepareDraw(){};
	protected:
		virtual void FlushThickLines();
		virtual void FlushQuads();
		virtual void FlushTriangles();
		virtual void DrawTriangles(const sprite_vertex* pVertices, int nTriangleCount);
		/*
		The WORLD, VIEW, and PROJECTION transforms are NOT modified.  The
		transforms currently set to the device are used to transform the sprites
		when the batch is drawn (at Flush or End).  If this is not specified,
		WORLD, VIEW, and PROJECTION transforms are modified so that sprites are
		drawn in screen space coordinates.
		*/
		bool IsUseObjectSpaceTransform();

	protected:
		DWORD m_flags;
		Matrix4 m_transform;
		// cached vertices
		std::vector<sprite_vertex> m_vertices;

		// all sprite thick lines
		std::vector<SpriteThickLine> m_thickLines;

		// all sprite triangles
		std::vector<SpriteTriangle> m_triangles;

		
		// number of thick lines to be drawn
		int m_thickline_count;

		// number of triangles to be drawn
		int m_triangles_count;
	};
}
