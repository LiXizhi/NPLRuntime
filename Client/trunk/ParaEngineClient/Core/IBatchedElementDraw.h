#pragma once
#include "PEtypes.h"
#include "IParaDebugDraw.h"
#include "ParticleElement.h"

namespace ParaEngine
{
	/** IParaDebugDraw draw interface draws line based primitives for debugging purposes.
	* One can call any of its functions at any place at any time. The implementation of this class usually cache
	* the primitives and draw them all in one batch after the main 3d scene is rendered. 
	*/
	class IBatchedElementDraw : public IParaDebugDraw
	{
	public:
		enum BatchDrawModes
		{
			BDM_NoDebug=0,
			BDM_DrawWireframe = 1,
			BDM_DrawAabb=2,
			BDM_DrawFeaturesText=4,
			BDM_DrawContactPoints=8,
			BDM_NoDeactivation=16,
			BDM_NoHelpText = 32,
			BDM_DrawText=64,
			BDM_ProfileTimings = 128,
			BDM_EnableSatComparison = 256,
			BDM_DisableBulletLCP = 512,
			BDM_EnableCCD = 1024,
			BDM_DrawConstraints = (1 << 11),
			BDM_DrawConstraintLimits = (1 << 12),
			BDM_FastWireframe = (1<<13),
			BDM_MAX_DRAW_MODE
		};

		/** draw a line with a given color. */
		virtual void DrawLine(const PARAVECTOR3& from,const PARAVECTOR3& to,const PARAVECTOR3& color) = 0;

		/** draw multiple lines with a given color using the predefined index order. 
		* it can be used to render a quad(4 lines) or box(12 lines).
		0,1,1,2,2,3,3,0, // bottom
		4,5,5,6,6,7,7,4, // top
		0,4,1,5,2,6,3,7, // sides
		* @param lines: vertex arrays
		* @param nLineCount: it can be 4 or 12. the index order is given above. 
		* @param color: color to draw the line. 
		*/
		virtual void DrawPredefinedLines(const PARAVECTOR3* lines, int nLineCount, const PARAVECTOR3& color) = 0;

		/** draw a contact point with a given color. */
		virtual void DrawContactPoint( const PARAVECTOR3& PointOnB,const PARAVECTOR3& normalOnB,float distance,int lifeTime,const PARAVECTOR3& color ) = 0;
		
		/** draw a contact point with a given color. */
		virtual void ReportErrorWarning( const char* warningString ) = 0;

		/** draw a text with a given color. */
		virtual void Draw3dText( const PARAVECTOR3& location,const char* textString ) = 0;

		/** set current render offset. It only takes effect when calling one of the above Draw functions. */
		virtual void SetRenderOffset(const PARAVECTOR3& vOffset) = 0;

		/** get current render offset. It only takes effect when calling one of the above Draw functions. */
		virtual PARAVECTOR3 GetRenderOffset() = 0;

		/** bitwise of DebugDrawModes */
		virtual void SetDebugDrawMode(int debugMode) = 0;

		/** bitwise of PhysicsDebugDrawModes */
		virtual int	GetDebugDrawMode() = 0;

		/** get the current draw color. some object may reply on this value when drawing the primitives. */
		virtual void SetDefaultColor(const PARAVECTOR3& color) = 0;

		/** set the current draw color. some object may reply on this value when drawing the primitives. */
		virtual PARAVECTOR3 GetDefaultColor() = 0;

		/** draw line with thickness 
		* @param thickness: Negative thickness means that thickness is calculated in screen space, positive thickness should be used for world space thickness.
		*/
		virtual void AddThickLine(const PARAVECTOR3& from, const PARAVECTOR3& to, const PARAVECTOR4& color, float fThickness = 0.f, float DepthBias = 0.f) = 0;
		/** nPointCount-1 lines segment is drawn*/
		virtual void AddThickLines(const PARAVECTOR3* points, int nPointCount, const PARAVECTOR4& color, float fThickness = 0.f, float DepthBias = 0.f) = 0;

		/** add a particle 
		* @param particle: the pointer must be valid until the end of the frame. 
		*/
		virtual void AddParticle(CParticleElement* particle) = 0;
	};
}