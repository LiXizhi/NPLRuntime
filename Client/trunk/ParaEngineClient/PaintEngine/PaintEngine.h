#pragma once
#include "ParaRegion.h"
#include "qtransform.h"

namespace ParaEngine
{
	class CPaintDevice;
	class CPaintEngineState;
	class CFontEngine;
	class CPainter;
	class QLineF;
	class QLine;
	class CPainterPath;
	class CPainterState;
	class CPolygonF;
	struct TextureEntity;
	struct SpriteFontEntity;
	class CTextItem;
	class ImageEntity;

	/** The CPaintEngine class provides an abstract definition of how
	CPainter draws to a given device on a given platform.

	If one wants to use CPainter to draw to a different backend,
	one must subclass CPaintEngine and reimplement all its virtual
	functions. The CPaintEngine implementation is then made available by
	subclassing QPaintDevice and reimplementing the virtual function
	CPaintDevice::paintEngine().

	QPaintEngine is created and owned by the QPaintDevice that created it.

	@note: CPaintEngine is hidden from end users. The end user only needs to use CPainter to draw on a paint device. 
	*/
	class CPaintEngine
	{
	public:
		enum DirtyFlags {
			DirtyPen = 0x0001,
			DirtyBrush = 0x0002,
			DirtyBrushOrigin = 0x0004,
			DirtyFont = 0x0008,
			DirtyBackground = 0x0010,
			DirtyBackgroundMode = 0x0020,
			DirtyTransform = 0x0040,
			DirtyClipRegion = 0x0080,
			DirtyClipPath = 0x0100,
			DirtyHints = 0x0200,
			DirtyCompositionMode = 0x0400,
			DirtyClipEnabled = 0x0800,
			DirtyOpacity = 0x1000,

			AllDirty = 0xffff
		};

		enum PolygonDrawMode {
			OddEvenMode,
			WindingMode,
			ConvexMode,
			PolylineMode
		};

	public:
		CPaintEngine();
		virtual ~CPaintEngine();
		
		virtual CPainterState *createState(CPainterState *orig) const;
		virtual void setState(CPainterState *s);
		CPainterState *GetState();
		const CPainterState *GetState() const;

		bool isActive() const { return active; }
		void setActive(bool newState) { active = newState; }

		virtual bool begin(CPaintDevice *pdev) = 0;
		virtual bool end() = 0;
		virtual void Flush();

		virtual void ApplyStateChanges();

		virtual void updateState(const CPaintEngineState &state) = 0;

		virtual void drawRects(const QRect *rects, int rectCount);
		virtual void drawRects(const QRectF *rects, int rectCount);

		virtual void drawLines(const QLine *lines, int lineCount);
		virtual void drawLines(const QLineF *lines, int lineCount);
		virtual void drawLines(const Vector3 *vertices, int lineCount);

		virtual void drawEllipse(const QRectF &r);
		virtual void drawEllipse(const QRect &r);

		virtual void drawTriangles(const Vector3* vertices, int nTriangleCount);

		virtual void drawPath(const CPainterPath &path);

		virtual void drawPoints(const QPointF *points, int pointCount);
		virtual void drawPoints(const QPoint *points, int pointCount);

		virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
		virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

		virtual void drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr) = 0;
		virtual void drawTextItem(const QPointF &p, const CTextItem &textItem);
		virtual void drawImage(const QRectF &r, const ImageEntity &pm, const QRectF &sr);

		void setPaintDevice(CPaintDevice *device);
		CPaintDevice *paintDevice() const;

		void setSystemClip(const QRegion &baseClip);
		QRegion systemClip() const;

		void setSystemRect(const QRect &rect);
		QRect systemRect() const;


		virtual QPoint coordinateOffset() const;


		inline bool testDirty(DirtyFlags df);
		inline void setDirty(DirtyFlags df);
		inline void clearDirty(DirtyFlags df);
		CPainter *painter() const;

	
		virtual bool IsScissorRectSupported() const { return false; };
		virtual bool IsStencilSupported() const { return false; };
		virtual void clip(const QRect & rect, ClipOperation op = ReplaceClip);


		/** by default, transforms are in 2d screen space. However, we can also specify using 3d transform,
		such as when rendering GUI onto 3D scenes
		*/
		virtual void SetUse3DTransform(bool bUse3DSpaceTransform){};

		/** return false by default. */
		virtual bool IsUse3DTransform(){ return false; };
	public:
		///////////////////////////////////////////
		// following is for backward compatibility with old GUIResource API. 
		///////////////////////////////////////////
		virtual HRESULT DrawRect(const RECT* rect, Color color, float depth){ return S_OK; };

		virtual HRESULT DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c){ return S_OK; };

		virtual HRESULT drawText(SpriteFontEntity* pFont, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor){ return S_OK; };
		virtual HRESULT drawText(SpriteFontEntity* pFont, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor){ return S_OK; };
	private:
		void setAutoDestruct(bool autoDestr) { selfDestruct = autoDestr; }
		bool autoDestruct() const { return selfDestruct; }

		virtual void transformChanged(){};
		virtual void penChanged(){};
		virtual void brushChanged(){};
		virtual void brushOriginChanged(){};
		virtual void opacityChanged(){};
		virtual void clipEnabledChanged(){};
		virtual void transformModeChanged(){};
		virtual void compositionModeChanged(){};
		
	protected:
		friend class CPainter;

		CPaintDevice *pdev;
		CPaintEngineState *state;
		bool active : 1;
		bool selfDestruct : 1;
		bool hasSystemTransform : 1;
		bool hasSystemViewport : 1;

		QRegion m_systemClip;
		QRect m_systemRect;
		QRegion systemViewport;
		QTransform systemTransform;
		CPaintDevice *currentClipDevice;
	};

	/** painter state, such as selected pen, brush, etc. */
	class CPaintEngineState
	{
	public:
		CPaintEngine::DirtyFlags state() const { return (CPaintEngine::DirtyFlags)dirtyFlags; }

		CPainter *painter() const;

	protected:
		friend class CPaintEngine;
		friend class CPainter;

		/** type of CPaintEngine::DirtyFlags */
		uint32 dirtyFlags;
	};


	inline bool CPaintEngine::testDirty(DirtyFlags df) {
		PE_ASSERT(state);
		return ((state->dirtyFlags & df) != 0);
	}

	inline void CPaintEngine::setDirty(DirtyFlags df) {
		PE_ASSERT(state);
		state->dirtyFlags |= df;
	}

	inline void CPaintEngine::clearDirty(DirtyFlags df)
	{
		PE_ASSERT(state);
		state->dirtyFlags &= ~static_cast<uint32>(df);
	}
}

