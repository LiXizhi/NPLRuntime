#pragma once
#include "ParaScriptingCommon.h"
#include "ParaScriptingGraphics.h"

namespace ParaEngine
{
	class CPainter;
	class CGUIBase;
}

namespace ParaScripting
{
	using namespace ParaEngine;
	using namespace luabind;
	class ParaAttributeObject;
	class ParaObject;
	
	/**
	* @ingroup ParaUI
	* the current painter object. call Begin() End() to switch paint device.
	* please note, for ownerdraw GUI callbacks, they are automatically called.
	* 
	* for performance reasons, all methods are static.  
	*/
	class ParaPainter
	{
	public:
		//
		// state functions
		//
		static bool Begin(const object & paintDevice);
		static void End();

		static void Flush();
		static void Save();
		static void Restore();

		static void SetCompositionMode(int mode);
		static int GetCompositionMode();

		static const ParaAttributeObject& GetAttributeObject();
		static void CallField(const char*  sFieldname);
		static void SetField(const char*  sFieldname, const object& input);
		static object GetField(const char*  sFieldname, const object& output);

		/** set current font 
		* @param font: {family="System", size=10, bold=true}
		* or it can be string "System;14;" or "System;14;bold"
		*/
		static void SetFont(const object& font);

		/** set current pen
		* @param pen: { width=1, brush = {color="#00000000", texture="filename or texture asset"}, }
		* or it can be {width=1, color="#000000", texture="filename or texture asset"}
		* or it can be pen color "#ff000000" or "255 255 255"
		*/
		static void SetPen(const object& pen);

		/** set current brush (texture and color)
		* @param brush: { color="#00000000", texture="filename or texture asset"} 
		* or it can be pen color "#ff000000" or "255 255 255"
		*/
		static void SetBrush(const object& brush);

		static void SetBrushOrigin(float x, float y);

		/** set current background brush
		* @param brush: { color="#00000000", texture="filename or texture asset"}
		*/
		static void SetBackground(const object& brush);

		/** between [0,1]
		*/
		static void SetOpacity(float fOpacity);

		static void SetClipRegion(int x, int y, int w, int h);
		static void SetClipping(bool enable);
		static bool HasClipping();

		
		//
		// transforms
		//
		/** Sets the world transformation matrix.
		* If combine is true, the specified matrix is combined with the current matrix; otherwise it replaces the current matrix.
		*/
		static void SetTransform(const object& trans, bool combine);
		static luabind::object GetTransform(const luabind::object& out);
		static void Scale(float sx, float sy);
		static void Shear(float sh, float sv);
		static void Rotate(float a);
		static void Translate(float dx, float dy);

		//
		// draw shapes
		//
		static void DrawPoint(float x, float y);
		static void DrawLine(float x1, float y1, float x2, float y2);
		static void DrawRect(float x1, float y1, float w, float h);

		/** draw 3d triangles
		* @param triangleList: array of triangle vertices {{x,y,z}, {x,y,z}, {x,y,z}, ...}, 
		* @param nTriangleCount: triangle count. 
		* @param nIndexOffset: start index offset. default to 0.
		*/
		static void DrawTriangleList(const object& triangleList, int nTriangleCount, int nIndexOffset);
		/** draw 3d lines
		* @param lineList: array of line vertices {{x,y,z}, {x,y,z}, ...},
		* @param nTriangleCount: triangle count
		* @param nIndexOffset: start index offset. default to 0.
		*/
		static void DrawLineList(const object& lineList, int nLineCount, int nIndexOffset);

		static void DrawTexture(int x, int y, int w, int h, const object& pTexture);
		static void DrawTexture2(int x, int y, int w, int h, const object& pTexture, int sx, int sy, int sw, int sh);
		static void DrawText(float x, float y, const std::string &s);
		static void DrawText2(float x, float y, float w, float h, const std::string &s, int textOption);

		/** draw a scene object 
		* @param obj: can be a character or model object. 
		* @param nOption: default to 0, which renders using the transform of the scene object. 
		*/
		static void DrawSceneObject(ParaObject& obj, int nOption);

	public:
		// do not call from scripting interface
		static CPainter* GetPainter() { return m_pPainter; }
		static void SetPainter(CPainter* val) { m_pPainter = val; }
	protected:
		// unsafe weak reference. 
		static CPainter* m_pPainter;
		
	};
}