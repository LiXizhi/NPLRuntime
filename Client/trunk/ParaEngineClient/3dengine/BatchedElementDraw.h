#pragma once
#include "IBatchedElementDraw.h"
#include "VertexFVF.h"
#include "unordered_ref_array.h"
#include <vector>

namespace ParaEngine
{
	struct TextureEntity;

	
	/** for drawing special element object(like particles, lines, etc) in the scene. 
	* this is usually a singleton associated with a scene object. And is always rendered after 3d scene is rendered but before GUI object. 
	*/
	class CBatchedElementDraw : public IBatchedElementDraw
	{
	public:
		CBatchedElementDraw();
		~CBatchedElementDraw();

		/** deprecated: interface */
		IParaDebugDraw* GetDebugDrawInterface();
	public:
		/** draw a line with a given color. */
		virtual void DrawLine(const PARAVECTOR3& from,const PARAVECTOR3& to,const PARAVECTOR3& color);

		/** draw multiple lines with a given color using the predefined index order. 
		* it can be used to render a quad(4 lines) or box(12 lines).
		0,1,1,2,2,3,3,0, // bottom
		4,5,5,6,6,7,7,4, // top
		0,4,1,5,2,6,3,7, // sides
		* @param lines: vertex arrays
		* @param nLineCount: it can be 4 or 12. the index order is given above. 
		* @param color: color to draw the line. 
		*/
		virtual void DrawPredefinedLines(const PARAVECTOR3* lines, int nLineCount, const PARAVECTOR3& color);

		/** draw a contact point with a given color. */
		virtual void DrawContactPoint( const PARAVECTOR3& PointOnB,const PARAVECTOR3& normalOnB,float distance,int lifeTime,const PARAVECTOR3& color );

		/** draw a contact point with a given color. */
		virtual void ReportErrorWarning( const char* warningString );

		/** draw a text with a given color. */
		virtual void Draw3dText( const PARAVECTOR3& location,const char* textString );

		/** set a render offset. */
		virtual void SetRenderOffset(const PARAVECTOR3& vOffset);

		virtual PARAVECTOR3 GetRenderOffset(){return (PARAVECTOR3&)m_vOffset;};

		/** bitwise fields of IParaDebugDraw::DebugDrawModes */
		virtual void SetDebugDrawMode(int debugMode);

		/** bitwise fields of PhysicsDebugDrawModes */
		virtual int	GetDebugDrawMode();

		/** get the current draw color. some object may reply on this value when drawing the primitives. */
		virtual void SetDefaultColor(const PARAVECTOR3& color) {m_vDefaultColor = color;};

		/** set the current draw color. some object may reply on this value when drawing the primitives. */
		virtual PARAVECTOR3 GetDefaultColor() {return m_vDefaultColor;};
	
		/** draw line with thickness */
		virtual void AddThickLine(const PARAVECTOR3& from, const PARAVECTOR3& to, const PARAVECTOR4& color, float fThickness = 0.f, float DepthBias = 0.f);
		/** nPointCount-1 lines segment is drawn*/
		virtual void AddThickLines(const PARAVECTOR3* points, int nPointCount, const PARAVECTOR4& color, float fThickness = 0.f, float DepthBias = 0.f);

		/** add a particle. although we will sort by texture in a special way, for better performance, 
		* it is recommended that particles with same textures are added in sequence or close to one another.
		* @param particle: the pointer must be valid until the end of the frame.
		*/
		virtual void AddParticle(CParticleElement* particle);

		void ClearParticles();

		/** call this before scene is reset. */
		void ClearAll();
	public:
		/** clear all cached drawing. */
		void ClearLines();

		void DrawAll(bool bClear = true);

		/** draw all cached line primitives since last Clear() call. 
		* @param bClear: if true(default), it will clear the render list after draw. 
		*/
		void DrawBatchedLines(bool bClear=true);
	
		void DrawBatchedThickLines(bool bClear = true);

		void DrawBatchedParticles(bool bClear = true);

		/** get the total number of line primitives in the cached render list. */
		int GetLineCount();

		bool IsEmpty();
	protected:
		/** sort particle into render passes according to texture. */
		void SortParticles();
	protected:
		Vector3 m_vOffset;
		int m_nDrawMode;
		PARAVECTOR3 m_vDefaultColor;

		std::vector<LINEVERTEX> m_lines_vertex_array;
		std::vector<int16> m_lines_index_array;
		int m_nLineCount;

		struct BatchedThickLines
		{
			PARAVECTOR3 vStart;
			PARAVECTOR3 vEnd;
			DWORD Color;
			// Negative thickness means that thickness is calculated in screen space, positive thickness should be used for world space thickness.
			float Thickness;
			float DepthBias;
			bool IsScreenSpace() const { return Thickness < 0.f; };
		};

		std::vector<BatchedThickLines> m_listThickLines;

		unordered_ref_array<CParticleElement*> m_listParticles;
		std::vector<ParticleRenderPass> m_ParticleRenderPasses;


		/** tessellation vertex buffer data */
		static std::vector<char> s_VertexData;
	};
}