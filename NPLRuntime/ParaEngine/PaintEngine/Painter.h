#pragma once
#include "qtransform.h"
#include "ParaRect.h"
#include "PaintEngine.h"
#include "IAttributeFields.h"
#include "2dengine/GUIResource.h"
#include "qfont.h"
#include "qpen.h"
#include "qbrush.h"
#include "qtextoption.h"
#include "qpolygon.h"

namespace ParaEngine
{
	class CPaintEngine;
	class CPaintDevice;
	class CPainterState;
	struct CPainterDummyState;
	class QFont;
	class QBrush;
	class QPen;
	class QPainterPath;
	class CBaseObject;

	/** base class for a surface to paint to. The CRenderTarget subclass this.
	*
	* ---++ On Coordinate System
	* The 2D API uses Y axis downward coordinate; where as the 3D API uses Y axis.
	* Both APIs can be used together. The final world matrix is calculated as thus:
	* 	finalMatrix = mat2DFinal * matInvertY * mat3DFinal;
	* 	mat2DFinal is the final matrix by all the 2d api like scale, translate, rotate
	* 	mat3DFinal is the final matrix by all the 3d api like PushMatrix, TranslateMatrix, etc.
	* Please note, mat3DFinal only takes effect when 3d mode is enabled, such as during rendering head on display or overlays.
	* When you are rendering 3D triangles mixed with 2D GUI, it is good practice to restore mat2DFinal to identity matrix after
	* drawing 2D GUI.
	*
	*/
	class CPainter : public IAttributeFields
	{
	public:
		CPainter();
		explicit CPainter(CPaintDevice* pd);
		virtual ~CPainter();
		ATTRIBUTE_DEFINE_CLASS(CPainter);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CPainter, GetCompositionMode_s, int*) { *p1 = cls->compositionMode(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, SetCompositionMode_s, int) { cls->setCompositionMode((CompositionMode)p1); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, GetOpacity_s, float*) { *p1 = cls->opacity(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, SetOpacity_s, float) { cls->setOpacity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, IsAutoLineWidth_s, bool*) { *p1 = cls->IsAutoLineWidth(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, EnableAutoLineWidth_s, bool) { cls->EnableAutoLineWidth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, IsSpriteUseWorldMatrix_s, bool*) { *p1 = cls->IsSpriteUseWorldMatrix(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, SetSpriteUseWorldMatrix_s, bool) { cls->SetSpriteUseWorldMatrix(p1); return S_OK; }

		ATTRIBUTE_METHOD(CPainter, Save_s) { cls->save(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, Restore_s) { cls->restore(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, Flush_s) { cls->Flush(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, BeginNativePainting_s) { cls->beginNativePainting(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, EndNativePainting_s) { cls->endNativePainting(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, ResetTransform_s) { cls->resetTransform(); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, GetMatrixMode_s, int*) { *p1 = cls->GetMatrixMode(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, SetMatrixMode_s, int) { cls->SetMatrixMode(p1); return S_OK; }

		ATTRIBUTE_METHOD(CPainter, PushMatrix_s) { cls->PushMatrix(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, PopMatrix_s) { cls->PopMatrix(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, LoadCurrentMatrix_s) { cls->LoadCurrentMatrix(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, LoadIdentityMatrix_s) { cls->LoadIdentityMatrix(); return S_OK; }
		ATTRIBUTE_METHOD(CPainter, LoadBillboardMatrix_s) { cls->LoadBillboardMatrix(); return S_OK; }

		ATTRIBUTE_METHOD3(CPainter, TranslateMatrix_s, float) { cls->TranslateMatrix(p1, p2, p3); return S_OK; }
		ATTRIBUTE_METHOD4(CPainter, RotateMatrix_s, float) { cls->RotateMatrix(p1, p2, p3, p4); return S_OK; }
		ATTRIBUTE_METHOD3(CPainter, ScaleMatrix_s, float) { cls->ScaleMatrix(p1, p2, p3); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, LoadMatrix_s, const Matrix4&) { cls->LoadMatrix(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, MultiplyMatrix_s, const Matrix4&) { cls->MultiplyMatrix(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CPainter, GetCurrentMatrix_s, Matrix4*) { *p1 = cls->GetCurMatrix(); return S_OK; }
		ATTRIBUTE_METHOD1(CPainter, GetScaling_s, Vector3*) { *p1 = cls->GetScaling(); return S_OK; }


	public:
		/*!
		Returns the paint device on which this painter is currently
		painting, or 0 if the painter is not active.
		\sa isActive()
		*/
		CPaintDevice* device() const;

		bool begin(CPaintDevice* pd);
		bool end();
		bool isActive() const;

		void initFrom(const CPaintDevice* device);

		enum CompositionMode {
			CompositionMode_SourceBlend,
			CompositionMode_SourceOver,
			CompositionMode_DestinationOver,
			CompositionMode_Clear,
			CompositionMode_Source,
			CompositionMode_Destination,
			CompositionMode_SourceIn,
			CompositionMode_DestinationIn,
			CompositionMode_SourceOut,
			CompositionMode_DestinationOut,
			CompositionMode_SourceAtop,
			CompositionMode_DestinationAtop,
			CompositionMode_Xor,
			CompositionMode_Plus,
			CompositionMode_PlusSourceBlend,
			// ROPs
			RasterOp_SourceOrDestination,
			RasterOp_SourceAndDestination,
			RasterOp_SourceXorDestination,
			RasterOp_NotSourceAndNotDestination,
			RasterOp_NotSourceOrNotDestination,
			RasterOp_NotSourceXorDestination,
			RasterOp_NotSource,
			RasterOp_NotSourceAndDestination,
			RasterOp_SourceAndNotDestination,
			RasterOp_NotSourceOrDestination,
			RasterOp_SourceOrNotDestination,
			RasterOp_ClearDestination,
			RasterOp_SetDestination,
			RasterOp_NotDestination
		};

		CPaintEngine* paintEngine() const;
		bool IsScissorRectSupported() const;
		bool IsStencilSupported() const;

		//
		// state functions
		//
		void setCompositionMode(CompositionMode mode);
		CompositionMode compositionMode() const;
		/** number of texture between begin() end() that is not available at the time of drawing.
		* we generally make the paint device dirty if there is painted with at least one pending asset.
		*/
		int GetPendingAssetCount();

		/** whether we will render line so that they are at least 1 pixel width
		* automatically adjust line width according to camera to world distance.
		*/
		bool IsAutoLineWidth() const;
		void EnableAutoLineWidth(bool val);

		/** get screen space coordinate from a unit space x parameter. It will take UI scaling into consideration
		* @param fUnitspaceX: [0,1], where 0 is left most, 1 is right most.
		*/
		float FromUnitSpaceX(float fUnitspaceX);
		float FromUnitSpaceY(float fUnitspaceY);

		const QFont& font() const;
		void setFont(const QFont& f);

		void setPen(const Color& color);
		void setPen(const QPen& pen);
		void setPen(PenStyle style);
		const QPen& pen() const;

		void setBrush(const QBrush& brush);
		void setBrush(BrushStyle style);
		const QBrush& brush() const;

		QPoint brushOrigin() const;
		inline void setBrushOrigin(int x, int y);
		inline void setBrushOrigin(const QPoint&);
		void setBrushOrigin(const QPointF&);

		void setBackground(const QBrush& bg);
		const QBrush& background() const;

		float opacity() const;
		void setOpacity(float opacity);

		// Clip functions
		QRegion clipRegion() const;
		QPainterPath clipPath() const;

		void setClipRect(const QRectF&, ClipOperation op = ReplaceClip);
		void setClipRect(const QRect&, ClipOperation op = ReplaceClip);
		inline void setClipRect(int x, int y, int w, int h, ClipOperation op = ReplaceClip);

		void setClipRegion(const QRegion&, ClipOperation op = ReplaceClip);

		void setClipPath(const QPainterPath& path, ClipOperation op = ReplaceClip);

		void setClipping(bool enable);
		bool hasClipping() const;

		QRectF clipBoundingRect() const;

		void save();
		void restore();

		/** native painting is supported by default. however, one need to call SetUse3DTransform(true) before begin().
		* this function does nothing here.
		*/
		void beginNativePainting();
		void endNativePainting();

		void Flush();

		//
		// transforms
		//
		/** Sets the world transformation matrix.
		* If combine is true, the specified matrix is combined with the current matrix; otherwise it replaces the current matrix.
		*/
		void setWorldTransform(const QTransform& matrix, bool combine = false);
		const QTransform& worldTransform() const;
		const QTransform& deviceTransform() const;
		/** Resets any transformations that were made using translate(),
		* scale(), shear(), rotate(), setWorldTransform(), setViewport()
		*/
		void resetTransform();
		/** Returns the transformation matrix combining the current window/viewport and world transformation.
		*/
		QTransform combinedTransform() const;

		void setWorldMatrixEnabled(bool enabled);
		bool worldMatrixEnabled() const;

		void scale(float sx, float sy);
		void shear(float sh, float sv);
		void rotate(float a);

		void translate(const QPointF& offset);
		inline void translate(const QPoint& offset);
		inline void translate(float dx, float dy);

		QRect viewport() const;
		void setViewport(const QRect& viewport);

		QPointF UIScaling() const;
		/** called automatically according to paint device */
		void setUIScaling(const QPointF& uiScaling);

		void setViewTransformEnabled(bool enable);
		bool viewTransformEnabled() const;


		//
		// draw shapes
		//
		void drawPoints(const QPointF* points, int pointCount);
		void drawPoints(const QPoint* points, int pointCount);
		void drawLines(const QLineF* lines, int lineCount);
		void drawLines(const QPointF* pointPairs, int lineCount);
		void drawLines(const QLine* lines, int lineCount);
		void drawLines(const QPoint* pointPairs, int lineCount);
		void drawLines(const Vector3* vertices, int nLineCount); // 3d lines
		void drawRects(const QRectF* rects, int rectCount);
		void drawRects(const QRect* rects, int rectCount);

		void drawTriangles(const Vector3* vertices, int nTriangleCount);


		inline void drawPoint(const QPointF& pt);
		inline void drawPoint(const QPoint& p);
		inline void drawPoint(int x, int y);

		inline void drawLine(const QLineF& line);
		inline void drawLine(const QLine& line);
		inline void drawLine(int x1, int y1, int x2, int y2);
		inline void drawLine(const QPoint& p1, const QPoint& p2);
		inline void drawLine(const QPointF& p1, const QPointF& p2);
		inline void drawLines(const std::vector<QLineF>& lines);
		inline void drawLines(const std::vector<QPointF>& pointPairs);
		inline void drawLines(const std::vector<QLine>& lines);
		inline void drawLines(const std::vector<QPoint>& pointPairs);

		inline void drawRect(const QRectF& rect);
		inline void drawRect(int x1, int y1, int w, int h);
		inline void drawRect(const QRect& rect);
		inline void drawRects(const std::vector<QRectF>& rectangles);
		inline void drawRects(const std::vector<QRect>& rectangles);

		void drawPolyline(const QPointF* points, int pointCount);
		inline void drawPolyline(const QPolygonF& polyline);
		void drawPolyline(const QPoint* points, int pointCount);
		inline void drawPolyline(const QPolygon& polygon);

		void drawPolygon(const QPointF* points, int pointCount, FillRule fillRule = OddEvenFill);
		inline void drawPolygon(const QPolygonF& polygon, FillRule fillRule = OddEvenFill);
		void drawPolygon(const QPoint* points, int pointCount, FillRule fillRule = OddEvenFill);
		inline void drawPolygon(const QPolygon& polygon, FillRule fillRule = OddEvenFill);

		//
		// draw images
		//
		void drawTexture(const QPointF& p, TextureEntity* pTexture, float fDepth = 0.f);
		void drawTexture(const QRectF& targetRect, TextureEntity* pTexture, const QRectF& sourceRect, float fDepth = 0.f);

		inline void drawTexture(const QRect& targetRect, TextureEntity* pTexture, const QRect& sourceRect, float fDepth = 0.f);
		inline void drawTexture(int x, int y, int w, int h, TextureEntity* pTexture, int sx, int sy, int sw, int sh, float fDepth = 0.f);
		inline void drawTexture(int x, int y, TextureEntity* pTexture, int sx, int sy, int sw, int sh, float fDepth = 0.f);
		inline void drawTexture(const QPointF& p, TextureEntity* pTexture, const QRectF& sr, float fDepth = 0.f);
		inline void drawTexture(const QPoint& p, TextureEntity* pTexture, const QRect& sr, float fDepth = 0.f);
		inline void drawTexture(const QPoint& p, TextureEntity* pTexture, float fDepth = 0.f);
		inline void drawTexture(int x, int y, TextureEntity* pTexture, float fDepth = 0.f);
		inline void drawTexture(const QRect& r, TextureEntity* pTexture, float fDepth = 0.f);
		inline void drawTexture(int x, int y, int w, int h, TextureEntity* pTexture, float fDepth = 0.f);

		//
		// draw text
		//
		void drawText(const QPointF& p, const std::string& s);
		void drawText(const QRectF& r, const std::string& text, const QTextOption& o = QTextOption());

		inline void drawText(const QPoint& p, const std::string& s);
		inline void drawText(int x, int y, const std::string& s);

		// 
		// scene object
		// 

		/** draw a scene object
		* @param obj: can be a character or model object.
		* @param nOption: default to 0, which renders using the transform of the scene object.
		*/
		void DrawSceneObject(CBaseObject* pObj, int nOption = 0);

	public:
		///////////////////////////////////////////
		// following is for backward compatibility with old GUIResource API, which is only used by system drawing of GUIBase controls.
		///////////////////////////////////////////

		/** set the global transform to sprite object. */
		void SetSpriteTransform(const Matrix4* pMatrix = NULL);

		/**
		* rotate sprite at a rect
		*/
		HRESULT DrawSprite(GUITextureElement* pElement, RECT* prcDest, float depth);
		HRESULT DrawSprite(GUITextureElement* pElement, RECT* prcDest, Matrix4 matTransform, float depth);
		/**
		* rotate prcDest with an origin.
		*/
		HRESULT DrawSprite(GUITextureElement* pElement, RECT* prcDest, const Vector2& vRotOrigin, float fRadian, const Vector2* vScaling, const Vector2* vTranslation, const DWORD* dwColorMask, float depth);

		/**
		we only support n*¦Ð/2 angle rotation.
		*/
		HRESULT DrawSprite(GUITextureElement* pElement, RECT* prcDest, int n, float depth);
		HRESULT DrawRect(RECT* pRect, Color color, float depth);
		HRESULT DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, float depth, bool bShadow = false, int nCount = -1, int shadowQuality = 0, Color shadowColor = 0);

		/**
		* rotate prcDest with an origin.
		*/
		HRESULT DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, const Vector2& vRotOrigin,
			float fRadian, const Vector2* vScaling, const Vector2* vTranslation, const DWORD* dwColorMask,
			float depth, bool bShadow = false, int nCount = -1, int shadowQuality = 0, Color shadowColor = 0);

		HRESULT DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, const Matrix4& mat, const DWORD* dwColorMask,
			float depth, bool bShadow = false, int nCount = -1, int shadowQuality = 0, Color shadowColor = 0);

		HRESULT CalcTextRect(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, float depth, int nCount = -1);

		/** by default, transforms are in 2d screen space. However, we can also specify using 3d transform,
		such as when rendering GUI onto 3D scenes. Call this before begin()
		*/
		void SetUse3DTransform(bool bUse3DSpaceTransform);

		/** default 0.f*/
		void SetDepth2D(float fDepth);

		/** return false by default. */
		bool IsUse3DTransform();

		/** set sprite use world matrix. */
		void SetSpriteUseWorldMatrix(bool bEnable);

		bool IsSpriteUseWorldMatrix();

		/** currently only used for text size calculation.*/
		void SetHelperDevice(CPaintDevice* val);

	public:
		/** similar to glMatrixMode() in opengl.
		* @param nMode:  0 is world, 1 is view, 2 is projection. default to 0.
		*/
		void SetMatrixMode(int nMode);
		int GetMatrixMode() const;

		/** similar to glPushMatrix() in opengl.*/
		void PushMatrix();
		/** similar to glPopMatrix() in opengl.*/
		void PopMatrix();
		/** retrieve the current matrix. */
		void LoadCurrentMatrix();

		/** load identity matrix */
		void LoadIdentityMatrix();

		/** load a matrix so that everything rendered faces the camera. it will only do so by a rotation. */
		void LoadBillboardMatrix();

		/** we use row-major matrix */
		void LoadMatrix(const Matrix4& mat);
		/** multiply the current matrix with the specified matrix. we use row-major matrix */
		void MultiplyMatrix(const Matrix4& mat);
		/** multiply the current matrix by a translation matrix */
		void TranslateMatrix(float x, float y, float z);
		/** multiply the current matrix by a rotation matrix */
		void RotateMatrix(float angle, float x, float y, float z);
		/** multiply the current matrix by a scaling matrix */
		void ScaleMatrix(float x, float y, float z);

		/** get current world matrix. */
		const ParaEngine::Matrix4& GetCurMatrix() const;
		/** get the current world matrix's scaling.  */
		Vector3 GetScaling();
	private:

		void updateStateImpl(CPainterState* state);
		void updateState(CPainterState* state);

		void cleanup_painter_state();

		CPainterDummyState* fakeState() const;

		/** view clipping and ui scaling. */
		QTransform viewTransform() const;
		void updateMatrix();

		TransformStack& GetMatrixStack();
	private:
		CPaintDevice* m_device;
		CPaintDevice* original_device;
		CPaintDevice* helper_device;

		CPaintEngine* engine;
		CPainterState* state;
		std::vector<CPainterState*> states;
		bool m_bUse3DTransform;
		/** 0 is world, 1 is view, 2 is projection. default to 0. */
		int m_nMatrixMode;
		/** current matrix. */
		Matrix4 m_curMatrix;
		/** whether we will render line so that they are at least 1 pixel width
		* automatically adjust line width according to camera to world distance.
		*/
		bool m_bAutoLineWidth;
	};

	//
	// inline functions
	//

	inline void CPainter::translate(float dx, float dy)
	{
		translate(QPointF(dx, dy));
	}

	inline void CPainter::translate(const QPoint& offset)
	{
		translate((float)offset.x(), (float)offset.y());
	}

	inline void CPainter::drawLine(const QLineF& l)
	{
		drawLines(&l, 1);
	}

	inline void CPainter::drawLine(const QLine& line)
	{
		drawLines(&line, 1);
	}

	inline void CPainter::drawLine(int x1, int y1, int x2, int y2)
	{
		QLine l(x1, y1, x2, y2);
		drawLines(&l, 1);
	}

	inline void CPainter::drawLine(const QPoint& p1, const QPoint& p2)
	{
		QLine l(p1, p2);
		drawLines(&l, 1);
	}

	inline void CPainter::drawLine(const QPointF& p1, const QPointF& p2)
	{
		drawLine(QLineF(p1, p2));
	}

	inline void CPainter::drawLines(const std::vector<QLineF>& lines)
	{
		drawLines(constVectorData(lines), (int)lines.size());
	}

	inline void CPainter::drawLines(const std::vector<QLine>& lines)
	{
		drawLines(constVectorData(lines), (int)lines.size());
	}

	inline void CPainter::drawLines(const std::vector<QPointF>& pointPairs)
	{
		drawLines(constVectorData(pointPairs), (int)pointPairs.size() / 2);
	}

	inline void CPainter::drawLines(const std::vector<QPoint>& pointPairs)
	{
		drawLines(constVectorData(pointPairs), (int)pointPairs.size() / 2);
	}

	inline void CPainter::drawRect(const QRectF& rect)
	{
		drawRects(&rect, 1);
	}

	inline void CPainter::drawRect(int x, int y, int w, int h)
	{
		QRect r(x, y, w, h);
		drawRects(&r, 1);
	}

	inline void CPainter::drawRect(const QRect& r)
	{
		drawRects(&r, 1);
	}

	inline void CPainter::drawRects(const std::vector<QRectF>& rects)
	{
		drawRects(constVectorData(rects), (int)rects.size());
	}

	inline void CPainter::drawRects(const std::vector<QRect>& rects)
	{
		drawRects(constVectorData(rects), (int)rects.size());
	}

	inline void CPainter::drawPoint(const QPointF& p)
	{
		drawPoints(&p, 1);
	}

	inline void CPainter::drawPoint(int x, int y)
	{
		QPoint p(x, y);
		drawPoints(&p, 1);
	}

	inline void CPainter::drawPoint(const QPoint& p)
	{
		drawPoints(&p, 1);
	}


	inline void CPainter::drawPolyline(const QPolygonF& polyline)
	{
		drawPolyline(polyline.constData(), (int)polyline.size());
	}

	inline void CPainter::drawPolyline(const QPolygon& polyline)
	{
		drawPolyline(polyline.constData(), (int)polyline.size());
	}

	inline void CPainter::drawPolygon(const QPolygonF& polygon, FillRule fillRule)
	{
		drawPolygon(polygon.constData(), (int)polygon.size(), fillRule);
	}

	inline void CPainter::drawPolygon(const QPolygon& polygon, FillRule fillRule)
	{
		drawPolygon(polygon.constData(), (int)polygon.size(), fillRule);
	}

	inline void CPainter::drawTexture(const QRect& targetRect, TextureEntity* pTexture, const QRect& sourceRect, float fDepth)
	{
		drawTexture(QRectF(targetRect), pTexture, QRectF(sourceRect), fDepth);
	}

	inline void CPainter::drawTexture(const QPoint& p, TextureEntity* pTexture, float fDepth)
	{
		drawTexture(QPointF(p), pTexture, fDepth);
	}

	inline void CPainter::drawTexture(const QRect& r, TextureEntity* pTexture, float fDepth)
	{
		drawTexture(QRectF(r), pTexture, QRectF(), fDepth);
	}

	inline void CPainter::drawTexture(int x, int y, TextureEntity* pTexture, float fDepth)
	{
		drawTexture(QPointF((float)x, (float)y), pTexture, fDepth);
	}

	inline void CPainter::drawTexture(int x, int y, int w, int h, TextureEntity* pTexture, float fDepth)
	{
		drawTexture(QRectF((float)x, (float)y, (float)w, (float)h), pTexture, QRectF(), fDepth);
	}

	inline void CPainter::drawTexture(int x, int y, int w, int h, TextureEntity* pTexture,
		int sx, int sy, int sw, int sh, float fDepth)
	{
		drawTexture(QRectF((float)x, (float)y, (float)w, (float)h), pTexture, QRectF((float)sx, (float)sy, (float)sw, (float)sh), fDepth);
	}

	inline void CPainter::drawTexture(int x, int y, TextureEntity* pTexture,
		int sx, int sy, int sw, int sh, float fDepth)
	{
		drawTexture(QRectF((float)x, (float)y, -1, -1), pTexture, QRectF((float)sx, (float)sy, (float)sw, (float)sh), fDepth);
	}

	inline void CPainter::drawTexture(const QPointF& p, TextureEntity* pTexture, const QRectF& sr, float fDepth)
	{
		drawTexture(QRectF(p.x(), p.y(), -1, -1), pTexture, sr, fDepth);
	}

	inline void CPainter::drawTexture(const QPoint& p, TextureEntity* pTexture, const QRect& sr, float fDepth)
	{
		drawTexture(QRectF((float)p.x(), (float)p.y(), -1.f, -1.f), pTexture, sr, fDepth);
	}

	inline void CPainter::drawText(const QPoint& p, const std::string& s)
	{
		drawText(QPointF(p), s);
	}

	inline void CPainter::drawText(int x, int y, const std::string& s)
	{
		drawText(QPointF((float)x, (float)y), s);
	}

	inline void CPainter::setBrushOrigin(int x, int y)
	{
		setBrushOrigin(QPoint(x, y));
	}

	inline void CPainter::setBrushOrigin(const QPoint& p)
	{
		setBrushOrigin(QPointF(p));
	}

	inline void CPainter::setClipRect(int x, int y, int w, int h, ClipOperation op)
	{
		setClipRect(QRect(x, y, w, h), op);
	}

	/** for ease of push/pop composition mode. */
	class CompositionModeStack
	{
	public:
		CompositionModeStack(CPainter* pPainter, CPainter::CompositionMode newMode, bool bAutoBegin = true)
			:m_painter(pPainter), m_newBlendMode(newMode), m_bIsBegin(false)
		{
			if (bAutoBegin)
				Begin();
		}
		~CompositionModeStack()
		{
			if (m_bIsBegin)
				End();
		}
		void Begin()
		{
			if (!m_bIsBegin)
			{
				m_lastBlendMode = m_painter->compositionMode();
				m_painter->setCompositionMode(m_newBlendMode);
				m_bIsBegin = true;
			}
		}
		void End()
		{
			if (m_bIsBegin)
			{
				m_painter->setCompositionMode(m_lastBlendMode);
				m_bIsBegin = false;
			}
		}
	public:
		CPainter::CompositionMode m_lastBlendMode;
		CPainter::CompositionMode m_newBlendMode;
		CPainter* m_painter;
		bool m_bIsBegin;
	};

}

