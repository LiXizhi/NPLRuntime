#pragma once
#include "Painter.h"
#include "PaintEngine.h"
#include "qpen.h"
#include "qfont.h"
#include "qbrush.h"
#include "qpainterpath.h"

namespace ParaEngine
{
	/** currently only RectClip is supported, since we are using Scissor Rect on GPU to do the clipping. */
	class CPainterClipInfo
	{
	public:
		enum ClipType { RegionClip, PathClip, RectClip, RectFClip };
		CPainterClipInfo(const QRect &r, ClipOperation op, const QTransform &m) :
			clipType(RectClip), matrix(m), operation(op), rect(r) { }
	public:
		ClipType clipType;
		QTransform matrix;
		ClipOperation operation;
		QRect rect;
	};

	class CPainterState : public CPaintEngineState
	{
	public:
		CPainterState();
		CPainterState(const CPainterState *s);
		virtual ~CPainterState();
		void init(CPainter *p);

	public:
		/** set the global transform to sprite object. */
		virtual void SetSpriteTransform(const Matrix4 * pMatrix = NULL);
		/** set sprite use world matrix. */
		virtual void SetSpriteUseWorldMatrix(bool bEnable){};
		virtual bool IsSpriteUseWorldMatrix(){ return false; };
	public:
		QPen pen() const;
		QBrush brush() const;
		QPointF brushOrigin() const;
		QBrush backgroundBrush() const;
		QFont font() const;
		QMatrix matrix() const;
		QTransform transform() const;
		/** get the current pen color multiplied by current opacity. */
		Color color();

		ClipOperation clipOperation() const;
		QRegion clipRegion() const;
		QPainterPath clipPath() const;
		bool isClipEnabled() const;
		
		CPainter::CompositionMode compositionMode() const;
		float opacity() const;

		/** number of texture that is being loaded, but not available at the time of drawing.
		* we generally make the paint device dirty if there is painted with at least one pending asset.
		*/
		void AddPendingAsset(int nCount = 1);

		/** calculate device matrix based on given matrix
		* @param pIn: if NULL, it will be current transform.
		*/
		void CalculateDeviceMatrix(Matrix4* pOut, const Matrix4* pIn = NULL);
	public:
		QPointF m_brushOrigin;
		QFont m_font;
		QPen m_pen;
		QBrush m_brush;
		QBrush m_bgBrush;             // background brush
		QRegion m_clipRegion;
		QPainterPath m_clipPath;
		ClipOperation m_clipOperation;
		std::vector<CPainterClipInfo> m_clipInfo;

		QTransform worldMatrix;       // World transformation matrix, not window and viewport
		QTransform m_matrix;            // Complete transformation matrix,
		/** final complete transform */
		Matrix4 matComplete;

		// viewport rectangle
		int m_viewportX, m_viewportY, m_viewportWidth, m_viewportHeight;         
		float m_fViewportLeft;
		float m_fViewportTop;
		float m_fUIScalingX;
		float m_fUIScalingY;

		float m_opacity;

		bool WxF : 1;                 // World transformation
		bool VxF : 1;                 // View transformation
		bool m_clipEnabled : 1;
		/** number of texture that is being loaded, but not available at the time of drawing. 
		* we generally make the paint device dirty if there is painted with at least one pending asset.
		*/
		int m_nPendingAssetCount;
		CPainter *m_painter;
		CPainter::CompositionMode m_composition_mode;
		uint32 changeFlags;
	};

	struct CPainterDummyState
	{
		QFont font;
		QPen pen;
		QBrush brush;
		QTransform transform;
	};
}