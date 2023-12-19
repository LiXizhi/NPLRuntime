//-----------------------------------------------------------------------------
// Class: PaintEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintengine.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#endif
#include "Painter.h"
#include "qvectorpath.h"
#include "ParaWorldAsset.h"
#include "ViewportManager.h"
#include "PaintEngineGPUState.h"
#include "PaintEngineGPU.h"

using namespace ParaEngine;
using namespace boost;

bool ParaEngine::CPaintEngineGPU::s_IsStencilSupported = false;
bool ParaEngine::CPaintEngineGPU::s_IsScissorRectSupported = false;

ParaEngine::CPaintEngineGPU::CPaintEngineGPU()
	:m_bInitialized(false), m_bUse3DTransform(false), m_matrixDirty(false), m_worldViewProjectMatDirty(true), m_worldViewMatDirty(true), m_compositionModeDirty(false)
{
#ifdef USE_DIRECTX_RENDERER
	m_pStateBlock = NULL;
#endif
	m_pSprite = CSpriteRenderer::Create();
	auto pAssetManaget = CGlobals::GetAssetManager();
	pAssetManaget->OnCleanup.connect(bind(&CPaintEngineGPU::Cleanup, this));
	pAssetManaget->OnInitDeviceObjects.connect(bind(&CPaintEngineGPU::InitDeviceObjects, this));
	pAssetManaget->OnRestoreDeviceObjects.connect(bind(&CPaintEngineGPU::RestoreDeviceObjects, this));
	pAssetManaget->OnInvalidateDeviceObjects.connect(bind(&CPaintEngineGPU::InvalidateDeviceObjects, this));
	pAssetManaget->OnDeleteDeviceObjects.connect(bind(&CPaintEngineGPU::DeleteDeviceObjects, this));
}

ParaEngine::CPaintEngineGPU::~CPaintEngineGPU()
{
	SAFE_DELETE(m_pSprite);
}

static CPaintEngineGPU* s_singleton = NULL;
CPaintEngineGPU* ParaEngine::CPaintEngineGPU::GetInstance()
{
	if (s_singleton)
		return s_singleton;
	else
	{
		if (!CGlobals::GetRenderDevice())
			return NULL;
		s_singleton = new CPaintEngineGPU();
		return s_singleton;
	}
}

void ParaEngine::CPaintEngineGPU::DestroyInstance()
{
	if (s_singleton)
	{
		SAFE_DELETE(s_singleton);
	}
}

bool ParaEngine::CPaintEngineGPU::IsScissorRectSupported() const
{
	return s_IsScissorRectSupported;
}

bool ParaEngine::CPaintEngineGPU::IsStencilSupported() const
{
	return s_IsStencilSupported;
}

void ParaEngine::CPaintEngineGPU::Cleanup()
{
	// TODO: here

	// destroy the singleton when application exit. 
	if (s_singleton == this)
		DestroyInstance();
}

void ParaEngine::CPaintEngineGPU::InitDeviceObjects()
{
#ifdef USE_DIRECTX_RENDERER
	//initialize the device capabilities
	CPaintEngineGPU::s_IsScissorRectSupported = (CGlobals::GetDirectXEngine().m_d3dCaps.RasterCaps&D3DPRASTERCAPS_SCISSORTEST) != 0;
	CPaintEngineGPU::s_IsStencilSupported = (CGlobals::GetDirectXEngine().m_d3dCaps.StencilCaps&(D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE)) == (D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE);
#elif defined(USE_OPENGL_RENDERER)
	CPaintEngineGPU::s_IsScissorRectSupported = true;
	CPaintEngineGPU::s_IsStencilSupported = RenderDevice::GetStencilBits() > 0;
#endif
	m_pSprite->InitDeviceObjects();
}

void ParaEngine::CPaintEngineGPU::RestoreDeviceObjects()
{
	m_pSprite->RestoreDeviceObjects();
	// create new state block
#ifdef USE_DIRECTX_RENDERER
	CGlobals::GetRenderDevice()->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);
#endif
	m_bInitialized = true;
}

void ParaEngine::CPaintEngineGPU::InvalidateDeviceObjects()
{
	m_pSprite->InvalidateDeviceObjects();
#ifdef USE_DIRECTX_RENDERER
	SAFE_RELEASE(m_pStateBlock);
#endif

	m_bInitialized = false;
}

void ParaEngine::CPaintEngineGPU::DeleteDeviceObjects()
{
	m_pSprite->DeleteDeviceObjects();
}

bool ParaEngine::CPaintEngineGPU::begin(CPaintDevice *pdev)
{
	Init();
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

#ifdef USE_OPENGL_RENDERER
	CGlobals::GetEffectManager()->EndEffect();
	if (SUCCEEDED(GetSprite()->Begin(IsUse3DTransform() ? D3DXSPRITE_OBJECTSPACE : 0)))
	{
		return true;
	}
#elif defined(USE_DIRECTX_RENDERER)
	CGlobals::GetEffectManager()->EndEffect();
	m_pStateBlock->Capture();

	if (!IsUse3DTransform())
	{
		if (CPaintEngineGPU::s_IsStencilSupported && !CPaintEngineGPU::s_IsScissorRectSupported){
			pd3dDevice->Clear(0L, NULL, D3DCLEAR_STENCIL, 0, 0, 0L);
			pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
		}
	}
	
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pd3dDevice->SetVertexShader(NULL);
	pd3dDevice->SetPixelShader(NULL);

	// D3DXSPRITE_SORT_DEPTH_BACKTOFRONT | D3DXSPRITE_OBJECTSPACE
	if (SUCCEEDED(GetSprite()->Begin(D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE | (IsUse3DTransform() ? D3DXSPRITE_OBJECTSPACE : 0))))
	{
		return true;
	}
#endif
	return false;
}

bool ParaEngine::CPaintEngineGPU::end()
{
	GetSprite()->End();
#ifdef USE_DIRECTX_RENDERER
	/* restore state*/
	m_pStateBlock->Apply();
#endif
	return true;
}

void ParaEngine::CPaintEngineGPU::updateState(const CPaintEngineState &state)
{

}

void ParaEngine::CPaintEngineGPU::drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr)
{

}

CSpriteRenderer* ParaEngine::CPaintEngineGPU::GetSprite()
{
	Init();
	return m_pSprite;
}

CPainterState * ParaEngine::CPaintEngineGPU::createState(CPainterState *orig) const
{
	CPaintEngineGPUState *s;
	if (!orig){
		s = new CPaintEngineGPUState();
	}
	else
		s = new CPaintEngineGPUState(*static_cast<CPaintEngineGPUState *>(orig));

	s->matrixChanged = false;
	s->compositionModeChanged = false;
	s->opacityChanged = false;
	s->renderHintsChanged = false;
	s->clipChanged = false;
	return s;
}

void ParaEngine::CPaintEngineGPU::Init()
{
	if (!m_bInitialized){
		InitDeviceObjects();
		RestoreDeviceObjects();
	}
}

void ParaEngine::CPaintEngineGPU::Flush()
{
	m_pSprite->Flush();
}

void ParaEngine::CPaintEngineGPU::ApplyStateChanges()
{
	CPaintEngineGPUState* state = GetState();
	
	if (!state)
		return;

	if (state->compositionModeChanged)
	{
		state->compositionModeChanged = false;
		ApplyCompositionMode(state->m_composition_mode);
	}
	
	if (m_matrixDirty)
	{
		m_matrixDirty = false;
		Matrix4 mat;
		state->CalculateDeviceMatrix(&mat);
		GetSprite()->SetTransform(mat);
	}
}

void ParaEngine::CPaintEngineGPU::ApplyCompositionMode(CPainter::CompositionMode mode)
{
	auto pd3dDevice = CGlobals::GetRenderDevice();
	switch (mode)
	{
	case CPainter::CompositionMode_SourceBlend:
	{
		// default mode
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case CPainter::CompositionMode_SourceOver:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case CPainter::CompositionMode_DestinationOver:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	}
	case CPainter::CompositionMode_Clear:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		break;
	}
	case CPainter::CompositionMode_Source:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		break;
	}
	case CPainter::CompositionMode_Destination:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	}
	case CPainter::CompositionMode_SourceIn:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		break;
	}
	case CPainter::CompositionMode_DestinationIn:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
		break;
	}
	case CPainter::CompositionMode_SourceOut:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		break;
	}
	case CPainter::CompositionMode_DestinationOut:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case CPainter::CompositionMode_SourceAtop:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case CPainter::CompositionMode_DestinationAtop:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
		break;
	}
	case CPainter::CompositionMode_Xor:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case CPainter::CompositionMode_Plus:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	}
	case CPainter::CompositionMode_PlusSourceBlend:
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	}
	default:
		OUTPUT_LOG("warning: Unsupported composition mode\n");
		break;
	}
}

CPaintEngineGPUState * ParaEngine::CPaintEngineGPU::GetState()
{
	return static_cast<CPaintEngineGPUState *>(CPaintEngine::GetState());
}

const CPaintEngineGPUState * ParaEngine::CPaintEngineGPU::GetState() const
{
	return static_cast<const CPaintEngineGPUState *>(CPaintEngine::GetState());
}

HRESULT ParaEngine::CPaintEngineGPU::DrawRect(const RECT* rect, Color color, float depth)
{
	ApplyStateChanges();
	return GetSprite()->DrawRect(rect, color, depth);
}

HRESULT ParaEngine::CPaintEngineGPU::DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c)
{
	ApplyStateChanges();
	return GetSprite()->DrawQuad(pTexture, pSrcRect, vCenter, pPosition, c);
}

HRESULT ParaEngine::CPaintEngineGPU::drawText(SpriteFontEntity* pFont, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor)
{
	if (pFont){
		ApplyStateChanges();
		return pFont->DrawTextA(GetSprite(), strText, nCount, rect, dwTextFormat, textColor);
	}
	return S_OK;
}

HRESULT ParaEngine::CPaintEngineGPU::drawText(SpriteFontEntity* pFont, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor)
{
	if (pFont){
		ApplyStateChanges();
		return pFont->DrawTextW(GetSprite(), strText, nCount, rect, dwTextFormat, textColor);
	}
	return S_OK;
}

void ParaEngine::CPaintEngineGPU::SetUse3DTransform(bool bUse3DSpaceTransform)
{
	m_bUse3DTransform = bUse3DSpaceTransform;
}

bool ParaEngine::CPaintEngineGPU::IsUse3DTransform()
{
	return m_bUse3DTransform;
}

void ParaEngine::CPaintEngineGPU::transformChanged()
{
	CPaintEngineGPUState* state = GetState();
	if (!state)
		return;
	SetMatrixDirty();
}

void ParaEngine::CPaintEngineGPU::SetMatrixDirty()
{
	m_matrixDirty = true;
	m_worldViewProjectMatDirty = true;
	m_worldViewMatDirty = true;
}

void ParaEngine::CPaintEngineGPU::drawTriangles(const Vector3* vertices, int nTriangleCount)
{
	ApplyStateChanges();
	auto state = GetState();
	if (!state)
		return;
	Color color = state->color();
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	GetSprite()->DrawTriangleList(pWhiteTexture, NULL, vertices, nTriangleCount, color);
}


void ParaEngine::CPaintEngineGPU::drawLines(const QLine *lines, int lineCount)
{
	ApplyStateChanges();
	auto state = GetState();
	if (!state)
		return;
	float fLineThickness = state->m_pen.widthF();

	Color color = state->color();
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	for (int i = 0; i < lineCount;++i)
	{
		const QLine& line = lines[i];
		GetSprite()->DrawLine(pWhiteTexture, NULL, Vector3((float)line.x1(), (float)line.y1(), 0.f), Vector3((float)line.x2(), (float)line.y2(), 0.f), fLineThickness, color);
	}
}

void ParaEngine::CPaintEngineGPU::drawLines(const QLineF *lines, int lineCount)
{
	ApplyStateChanges();
	auto state = GetState();
	if (!state)
		return;
	float fLineThickness = state->m_pen.widthF();
	Color color = state->color();
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	for (int i = 0; i < lineCount; ++i)
	{
		const QLineF& line = lines[i];
		GetSprite()->DrawLine(pWhiteTexture, NULL, Vector3(line.x1(), line.y1(), 0.f), Vector3(line.x2(), line.y2(), 0.f), fLineThickness, color);
	}
}

void ParaEngine::CPaintEngineGPU::drawLines(const Vector3 *vertices, int lineCount)
{
	ApplyStateChanges();
	auto state = GetState();
	if (!state)
		return;
	float fLineThickness = state->m_pen.widthF();

	Color color = state->color();
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	// for performance reason will only calculate line width from the first vertex instead of every vertex.  
	float fLineWidth = AdjustLineWidth(fLineThickness, vertices[0]);
	for (int i = 0; i < lineCount; ++i)
	{
		// fLineWidth = Math::Max(AdjustLineWidth(fLineThickness, vertices[i * 2]), AdjustLineWidth(fLineThickness, vertices[i * 2 + 1]));
		GetSprite()->DrawLine(pWhiteTexture, NULL, vertices[i * 2], vertices[i * 2 + 1], fLineWidth, color);
	}
}

void ParaEngine::CPaintEngineGPU::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{

}

void ParaEngine::CPaintEngineGPU::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
	CPaintEngineGPUState *s = GetState();

	PE_ASSERT(pointCount >= 2);
	if (mode != PolylineMode && QVectorPath::isRect((int *)points, pointCount)) {
		QRect r(points[0].x(),
			points[0].y(),
			points[2].x() - points[0].x(),
			points[2].y() - points[0].y());
		drawRects(&r, 1);
		return;
	}
	/*
	ensurePen();

	// Do the fill
	if (mode != PolylineMode) {
		ensureBrush();
		if (s->brushData.blend) {
			// Compose polygon fill..,
			ensureOutlineMapper();
			d->outlineMapper->beginOutline(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
			d->outlineMapper->moveTo(*points);
			const QPoint *p = points;
			const QPoint *ep = points + pointCount - 1;
			do {
				d->outlineMapper->lineTo(*(++p));
			} while (p < ep);
			d->outlineMapper->endOutline();

			// scanconvert.
			ProcessSpans brushBlend = d->getBrushFunc(d->outlineMapper->controlPointRect,
				&s->brushData);
			d->rasterize(d->outlineMapper->outline(), brushBlend, &s->brushData, d->rasterBuffer.data());
		}
	}

	// Do the outline...
	if (s->penData.blend) {
		int count = pointCount * 2;
		QVarLengthArray<qreal> fpoints(count);
		for (int i = 0; i < count; ++i)
			fpoints[i] = ((int *)points)[i];
		QVectorPath vp((qreal *)fpoints.data(), pointCount, 0, QVectorPath::polygonFlags(mode));

		if (s->flags.fast_pen) {
			QCosmeticStroker stroker(s, d->deviceRect, d->deviceRectUnclipped);
			stroker.setLegacyRoundingEnabled(s->flags.legacy_rounding);
			stroker.drawPath(vp);
		}
		else {
			QPaintEngineEx::stroke(vp, s->lastPen);
		}
	}
	*/
}

void ParaEngine::CPaintEngineGPU::penChanged()
{

}

void ParaEngine::CPaintEngineGPU::brushChanged()
{

}

void ParaEngine::CPaintEngineGPU::brushOriginChanged()
{

}

void ParaEngine::CPaintEngineGPU::opacityChanged()
{

}

void ParaEngine::CPaintEngineGPU::clipEnabledChanged()
{
	GetState()->clipChanged = true;

	if (painter()->hasClipping())
		regenerateClip();
	else
		systemStateChanged();
}

void ParaEngine::CPaintEngineGPU::transformModeChanged()
{
	if (isActive())
	{
		GetSprite()->End();
		GetSprite()->Begin(D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE | (IsUse3DTransform() ? D3DXSPRITE_OBJECTSPACE : 0));
	}
}

void ParaEngine::CPaintEngineGPU::setState(CPainterState *new_state)
{
	CPaintEngineGPUState *s = static_cast<CPaintEngineGPUState*>(new_state);
	CPaintEngineGPUState *old_state = GetState();
	CPaintEngine::setState(s);

	if (s->isNew) {
		// Newly created state object.  The call to setState()
		// will either be followed by a call to begin(), or we are
		// setting the state as part of a save().
		s->isNew = false;
		return;
	}
	// Setting the state as part of a restore().

	if (old_state == s || old_state->matrixChanged){
		SetMatrixDirty();
	}
		

	if (old_state == s || old_state->compositionModeChanged)
		m_compositionModeDirty = true;

	if (old_state == s || old_state->clipChanged) {
		if (old_state && old_state != s && old_state->canRestoreClip) {
			updateClipScissorTest();
		}
		else {
			regenerateClip();
		}
	}
}

void ParaEngine::CPaintEngineGPU::compositionModeChanged()
{
	CPaintEngineGPUState* state = GetState();
	if (state)
	{
		state->compositionModeChanged = true;
	}
	
}

void CPaintEngineGPU::RenderMask(const RECT& rcWindow)
{
	RECT clipRect = rcWindow;
	auto state = GetState();
	if (state)
	{
		state->m_nStencilValue++;
		RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
#ifdef USE_DIRECTX_RENDERER
		// use alpha blending with src=zero and dest=one to prevent writes to color buffer
		pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);


		// set up stencil function to increment stencil of pixels that pass alpha testing
		pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		pd3dDevice->SetRenderState(D3DRS_STENCILREF, state->m_nStencilValue);
		pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);

		// draw mask
		DrawRect(&clipRect, Color::White, 0.f);
		Flush();

		// disable alpha blending and testing
		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		state->compositionModeChanged = true;

#elif defined(USE_OPENGL_RENDERER)
		pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		if (state->m_nStencilValue == 1)
		{
			// clear stencil buffer to 0 on first stencil layer
			pd3dDevice->Clear(0L, NULL, D3DCLEAR_STENCIL, 0, 0, 0L);
			pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);
		}
		// disable color write
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		// draw m_nStencilValue to the stencil buffer
		/*glStencilFunc(GL_NEVER, pGUIState->m_nStencilValue, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);*/
		pd3dDevice->SetRenderState(D3DRS_STENCILREF, state->m_nStencilValue);
		pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER);
		pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);

		// this will automatically apply all the scaling, rotation, etc.
		// draw mask
		DrawRect(&clipRect, Color::White, 0.f);
		Flush();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
#endif
	}
}

void ParaEngine::CPaintEngineGPU::clip(const QRect & rect, ClipOperation op /*= ReplaceClip*/)
{
	auto state = GetState();
	state->clipChanged = true;
	
	Flush();

	if (op == ReplaceClip)
	{
		ApplyStateChanges();
		Matrix4 mat;
		state->CalculateDeviceMatrix(&mat);
		RECT clipRect = rect;
		clipRect.left = (LONG)(clipRect.left * mat._11 + mat._41);
		clipRect.top = (LONG)(clipRect.top * mat._22 + mat._42);
		clipRect.right = (LONG)(clipRect.right * mat._11 + mat._41);
		clipRect.bottom = (LONG)(clipRect.bottom * mat._22 + mat._42);
		state->rectangleClip = QRect(clipRect);
		updateClipScissorTest();
	}
	else if (op == NoClip)
	{
		state->rectangleClip = QRect();
		updateClipScissorTest();
	}
}

void ParaEngine::CPaintEngineGPU::regenerateClip()
{
	systemStateChanged();
	// replayClipOperations();
}

void ParaEngine::CPaintEngineGPU::systemStateChanged()
{
	auto state = GetState();
	if (!state)
		return;
	if (state->m_clipInfo.empty())
	{
		state->rectangleClip = QRect();
	}
	else
	{
		Matrix4 mat;
		state->CalculateDeviceMatrix(&mat);
		RECT clipRect = state->m_clipInfo[0].rect;
		clipRect.left = (LONG)(clipRect.left * mat._11 + mat._41);
		clipRect.top = (LONG)(clipRect.top * mat._22 + mat._42);
		clipRect.right = (LONG)(clipRect.right * mat._11 + mat._41);
		clipRect.bottom = (LONG)(clipRect.bottom * mat._22 + mat._42);
		state->rectangleClip = QRect(clipRect);
	}
	updateClipScissorTest();
}

void ParaEngine::CPaintEngineGPU::updateClipScissorTest()
{
	auto state = GetState();
	if (!state)
		return;

	Flush();

	if (IsScissorRectSupported())
	{
		if (state->rectangleClip.isValid())
		{
			RECT clipRect = state->rectangleClip;

			if (CGlobals::GetApp()->IsRotateScreen())
			{
				RECT clipRect2 = clipRect;
				clipRect.left = clipRect2.top;
				ParaViewport viewport;
				CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
				auto viewHeight = viewport.Height;
				clipRect.top = viewHeight - clipRect2.right;
				clipRect.right = clipRect2.bottom;
				clipRect.bottom = viewHeight - clipRect2.left;
			}

			CGlobals::GetRenderDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			CGlobals::GetRenderDevice()->SetScissorRect(&clipRect);
		}
		else
		{
			CGlobals::GetRenderDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		}
	}
	else if (IsStencilSupported())
	{
		if (state->rectangleClip.isValid())
		{
			RECT clipRect = state->rectangleClip;
			RenderMask(clipRect);
			// here we mean to allow the non-zero pixels to draw on frame buffer if we set it to D3DCMP_EQUAL.
			CGlobals::GetRenderDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		}
		else
		{
			CGlobals::GetRenderDevice()->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
#ifdef USE_OPENGL_RENDERER
			CGlobals::GetRenderDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);
#endif
		}
	}
}

const Matrix4& ParaEngine::CPaintEngineGPU::GetWorldViewProjectMatrix()
{
	if (m_worldViewProjectMatDirty)
	{
		ApplyStateChanges();
		m_worldViewProjectMatDirty = false;
		Matrix4 mat;
		GetSprite()->GetTransform(mat);
		mat *= CGlobals::GetViewMatrixStack().SafeGetTop();
		m_matWorldViewProject = mat * CGlobals::GetProjectionMatrixStack().SafeGetTop();
	}
	return m_matWorldViewProject;
}

void ParaEngine::CPaintEngineGPU::UpdateWorldViewParams()
{
	if (m_worldViewMatDirty)
	{
		ApplyStateChanges();
		m_worldViewMatDirty = false;
		m_matWorldView = GetSprite()->GetTransform() * CGlobals::GetViewMatrixStack().SafeGetTop();
		m_vWorldScaling = GetSprite()->GetTransform().GetScaleVector();
	}
}

const Matrix4& ParaEngine::CPaintEngineGPU::GetWorldViewMatrix()
{
	UpdateWorldViewParams();
	return m_matWorldView;
}

const Vector3& ParaEngine::CPaintEngineGPU::GetWorldScaling()
{
	UpdateWorldViewParams();
	return m_vWorldScaling;
}

float ParaEngine::CPaintEngineGPU::AdjustLineWidth(float fLineThickness, const Vector3& vPos)
{
	if (painter()->IsAutoLineWidth())
	{
		fLineThickness = Math::Max(GetPixelWidthAtPos(vPos), fLineThickness);
	}
	return fLineThickness;
}

float ParaEngine::CPaintEngineGPU::GetPixelWidthAtPos(const Vector3& vPos)
{
	auto state = GetState();
	const Matrix4& mat = GetWorldViewMatrix();
	// since Y axis is downward
	Vector3 vPosCameraSpace = vPos.InvertYCopy() * mat;
	// assume field of view is 90 and ensure it is at least 2.f pixels on screen. 
	return vPosCameraSpace.z * 2.0f / CGlobals::GetViewportManager()->GetActiveViewPort()->GetHeight() / GetWorldScaling().y;
	// return vPosCameraSpace.z / CGlobals::GetProjectionMatrixStack().SafeGetTop()._22 * 2.f 
	//  	/ CGlobals::GetViewportManager()->GetActiveViewPort()->GetHeight() 
	//  	/ GetWorldScaling().y;
}
