#pragma once
#include "ParaRegion.h"
#include "qtransform.h"
#include "PaintEngine.h"
#include "SpriteRenderer.h"
#include "PaintEngineGPUState.h"

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
	class CTextItem;

	/** opengl/directX renderer
	*/
	class CPaintEngineGPU : public CPaintEngine, public boost::signals2::trackable
	{
	public:
		CPaintEngineGPU();
		virtual ~CPaintEngineGPU();

		static CPaintEngineGPU* GetInstance();
		static void DestroyInstance();

		void Cleanup();
		void InitDeviceObjects();	// device independent
		void RestoreDeviceObjects(); // device dependent
		void InvalidateDeviceObjects();
		void DeleteDeviceObjects();

		void Init();
	public:
		
		virtual bool begin(CPaintDevice *pdev);
		virtual bool end();

		/** flush sprite */
		virtual void Flush();

		virtual CPainterState *createState(CPainterState *orig) const;
		virtual void updateState(const CPaintEngineState &state);
		virtual void setState(CPainterState *s);
		/** apply transform and blending mode, etc before drawing. */
		virtual void ApplyStateChanges();

		CPaintEngineGPUState *GetState();
		const CPaintEngineGPUState *GetState() const;

		virtual void drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr);


		/** get the global sprite object used to draw texture and images */
		CSpriteRenderer* GetSprite();

		virtual bool IsScissorRectSupported() const;;
		virtual bool IsStencilSupported() const;;
		virtual void clip(const QRect & rect, ClipOperation op = ReplaceClip);

		/** by default, transforms are in 2d screen space. However, we can also specify using 3d transform,
		such as when rendering GUI onto 3D scenes
		*/
		virtual void SetUse3DTransform(bool bUse3DSpaceTransform);

		/** return false by default. */
		virtual bool IsUse3DTransform();

		virtual void drawLines(const QLine *lines, int lineCount);
		virtual void drawLines(const QLineF *lines, int lineCount);
		virtual void drawLines(const Vector3 *vertices, int lineCount);

		virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
		virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

		virtual void drawTriangles(const Vector3* vertices, int nTriangleCount);
	public:
		///////////////////////////////////////////
		// following is for backward compatibility with old GUIResource API. 
		///////////////////////////////////////////
		virtual HRESULT DrawRect(const RECT* rect, Color color, float depth);

		virtual HRESULT DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c);

		virtual HRESULT drawText(SpriteFontEntity* pFont, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);
		virtual HRESULT drawText(SpriteFontEntity* pFont, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);;

	private:
		virtual void transformChanged();

		void SetMatrixDirty();

		virtual void penChanged();
		virtual void brushChanged();
		virtual void brushOriginChanged();
		virtual void opacityChanged();
		virtual void clipEnabledChanged();
		virtual void transformModeChanged();
		virtual void compositionModeChanged();

		void RenderMask(const RECT& rcWindow);
		void ApplyCompositionMode(CPainter::CompositionMode mode);
		void regenerateClip();
		void systemStateChanged();
		void updateClipScissorTest();
		float AdjustLineWidth(float fLineThickness, const Vector3& vPos);
		/** get pixel width at given 3d position. */
		float GetPixelWidthAtPos(const Vector3& vPos);
		/** @Note: the matrix returned uses Y download coordinate system. so one needs to invert Y for 3d transforms. 
		*/
		const Matrix4& GetWorldViewProjectMatrix();
		/** @Note: the matrix returned uses Y download coordinate system. so one needs to invert Y for 3d transforms.
		*/
		const Matrix4& GetWorldViewMatrix();
		const Vector3& GetWorldScaling();
		void UpdateWorldViewParams();
	protected:
		friend class CPainter;

#ifdef USE_DIRECTX_RENDERER
		// shared state block
		IDirect3DStateBlock9*	m_pStateBlock;
#endif
		// shared drawing sprite, for drawing all gui elements
		CSpriteRenderer*			m_pSprite;
		bool m_bInitialized;
		static bool s_IsScissorRectSupported;
		static bool s_IsStencilSupported;
		bool m_bUse3DTransform;


		// Dirty flags
		bool m_compositionModeDirty;
		bool m_matrixDirty; 
		bool m_worldViewProjectMatDirty;
		bool m_worldViewMatDirty;
		Matrix4 m_matWorldViewProject;
		Matrix4 m_matWorldView;
		Vector3 m_vWorldScaling;
	};

	
}

