#pragma once
#include "ParaRegion.h"
#include "qtransform.h"
#include "PaintEngine.h"

namespace ParaEngine
{
	class CPaintDevice;
	class CPaintEngineState;
	class CFontEngine;
	class CPainter;
	class CLineF;
	class CLine;
	class CPainterPath;
	class CPainterState;
	class CPolygonF;
	struct TextureEntity;
	class CTextItem;

	/** software renderer 
	*/
	class CPaintEngineRaster : public CPaintEngine
	{
	public:
		CPaintEngineRaster();
		virtual ~CPaintEngineRaster();

		/** this is actually thread local instance */
		static CPaintEngineRaster* GetInstance();
	public:
		virtual bool begin(CPaintDevice *pdev);
		virtual bool end();

		virtual void updateState(const CPaintEngineState &state);
		virtual void drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr);

	protected:
		friend class CPainter;
	};
}

