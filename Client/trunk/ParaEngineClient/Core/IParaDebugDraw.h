#pragma once
#include "PEtypes.h"
namespace ParaEngine
{

	/** IParaDebugDraw draw interface draws line based primitives for debugging purposes.
	* One can call any of its functions at any place at any time. The implementation of this class usually cache
	* the primitives and draw them all in one batch after the main 3d scene is rendered. 
	*/
	class IParaDebugDraw
	{
	public:
		enum DebugDrawModes
		{
			DDM_NoDebug=0,
			DDM_DrawWireframe = 1,
			DDM_DrawAabb=2,
			DDM_DrawFeaturesText=4,
			DDM_DrawContactPoints=8,
			DDM_NoDeactivation=16,
			DDM_NoHelpText = 32,
			DDM_DrawText=64,
			DDM_ProfileTimings = 128,
			DDM_EnableSatComparison = 256,
			DDM_DisableBulletLCP = 512,
			DDM_EnableCCD = 1024,
			DDM_DrawConstraints = (1 << 11),
			DDM_DrawConstraintLimits = (1 << 12),
			DDM_FastWireframe = (1<<13),
			DDM_MAX_DEBUG_DRAW_MODE
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
	};
}