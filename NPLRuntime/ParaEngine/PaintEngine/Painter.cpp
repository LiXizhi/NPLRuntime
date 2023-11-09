//-----------------------------------------------------------------------------
// Class: Painter
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.24
// Desc: I have referred to QT framework's qpaintdevice.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "PaintEngine.h"
#include "PaintDevice.h"
#include "PainterState.h"
#include "ParaScriptBindings/ParaScriptingPainter.h"
#include "2dengine/GUIBase.h"
#include "BipedObject.h"
#include "Painter.h"

using namespace ParaEngine;

#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
#endif

/** text shadow color threshold */
const float g_ColorThreshold = 0.5f;
/** text shadow alpha*/
const unsigned char g_ShadowAlpha = 160;

//--------------------------------------------------------------------------------------

/** auto compute d3d color
the shadow color is automatically decided according to text color. if text color rgb is above 128, it will use white, otherwise use black.
*/
Color ComputeTextShadowColor(const LinearColor& color)
{
	unsigned char shadowColor = ((color.r > g_ColorThreshold) || (color.g > g_ColorThreshold) || (color.b > g_ColorThreshold)) ? 0 : 255;
	return COLOR_ARGB(uint8(color.a * g_ShadowAlpha), shadowColor, shadowColor, shadowColor);
}

inline int RectWidth(const RECT &rc) { return ((rc).right - (rc).left); }
inline int RectHeight(const RECT &rc) { return ((rc).bottom - (rc).top); }

ParaEngine::CPainter::CPainter()
	:state(0), m_device(0), original_device(0), helper_device(0), engine(0), m_bUse3DTransform(false), m_nMatrixMode(0), m_bAutoLineWidth(true)
{
}

ParaEngine::CPainter::CPainter(CPaintDevice * pd)
	: CPainter()
{
	begin(pd);
}

ParaEngine::CPainter::~CPainter()
{
	if (isActive())
		end();
}

void ParaEngine::CPainter::initFrom(const CPaintDevice *pd)
{

}

bool ParaEngine::CPainter::begin(CPaintDevice * pd)
{
	if (pd->painters > 0) {
		OUTPUT_LOG("warning: CPainter::begin: A paint device can only be painted by one painter at a time.\n");
		return false;
	}

	if (engine) {
		OUTPUT_LOG("warning: CPainter::begin: Painter already active");
		return false;
	}
	helper_device = pd;
	original_device = pd;

	engine = pd->paintEngine();
	if (!engine) {
		OUTPUT_LOG("warning: CPainter::begin: Paint device returned engine == 0, type: %d", pd->devType());
		return false;
	}
	m_device = pd;

	// Setup new state...
	PE_ASSERT(!state);
	state = engine->createState(0);
	state->m_painter = this;
	states.push_back(state);

	state->m_brushOrigin = QPointF();
	state->m_nPendingAssetCount = 0;

	// Slip a painter state into the engine before we do any other operations
	engine->setState(state);

	engine->setPaintDevice(pd);
	// must be called before begin()
	engine->SetUse3DTransform(IsUse3DTransform());

	// this ensures that we are working on the current 3d world matrix
	if (IsUse3DTransform())
	{
		SetMatrixMode(0);
		LoadCurrentMatrix();
	}

	bool begun = engine->begin(pd);
	if (!begun) {
		OUTPUT_LOG("warning: CPainter::begin(): Returned false\n");
		if (engine->isActive()) {
			end();
		}
		else {
			cleanup_painter_state();
		}
		return false;
	}
	else {
		engine->setActive(begun);
	}

	QRect systemRect = engine->systemRect();
	if (!systemRect.isEmpty()) {
		state->m_viewportWidth = systemRect.width();
		state->m_viewportHeight = systemRect.height();
	}
	else {
		state->m_viewportWidth = pd->metric(CPaintDevice::PdmWidth);
		state->m_viewportHeight = pd->metric(CPaintDevice::PdmHeight);
	}
	setUIScaling(QPointF(pd->GetUIScalingX(), pd->GetUIScalingY()));
	setViewport(QRect((int)pd->GetViewportLeft(), (int)pd->GetViewportTop(), 0, 0));

	PE_ASSERT(engine->isActive());

	++m_device->painters;

	ParaScripting::ParaPainter::SetPainter(this);

	// this ensures that uniform transforms are set, to prevent unpredictable sprite transforms in the renderer. 
	engine->transformChanged();

	return true;
}

bool ParaEngine::CPainter::end()
{
	Flush();
	if (ParaScripting::ParaPainter::GetPainter() == this)
		ParaScripting::ParaPainter::SetPainter(NULL);
	if (!engine) {
		OUTPUT_LOG("warning:CPainter::end: Painter not active, aborted\n");
		cleanup_painter_state();
		return false;
	}
	bool ended = true;

	if (engine->isActive()) {
		ended = engine->end();
		updateState(0);

		--m_device->painters;
		if (m_device->painters == 0) {
			engine->setPaintDevice(0);
			engine->setActive(false);
		}
	}

	if (states.size() > 1) {
		OUTPUT_LOG("warning: CPainter::end: Painter ended with %d saved states\n", states.size());
	}

	SetUse3DTransform(false);
	engine->SetUse3DTransform(false);

	if (engine->autoDestruct()) {
		delete engine;
	}

	cleanup_painter_state();

	return ended;
}

bool ParaEngine::CPainter::isActive() const
{
	return engine != NULL;
}

CPaintDevice * ParaEngine::CPainter::device() const
{
	if (isActive() && engine->currentClipDevice)
		return engine->currentClipDevice;
	return original_device;
}

CPaintEngine * ParaEngine::CPainter::paintEngine() const
{
	return engine;
}

void ParaEngine::CPainter::beginNativePainting()
{

}

void ParaEngine::CPainter::endNativePainting()
{

}

void ParaEngine::CPainter::cleanup_painter_state()
{
	states.clear();
	SAFE_DELETE(state);
	engine = 0;
	m_device = 0;
}

void ParaEngine::CPainter::updateStateImpl(CPainterState *newState)
{
	// ### we might have to call QPainter::begin() here...
	if (!engine->state) {
		engine->state = newState;
		engine->setDirty(CPaintEngine::AllDirty);
	}

	if (engine->state->painter() != newState->m_painter)
		// ### this could break with clip regions vs paths.
		engine->setDirty(CPaintEngine::AllDirty);

	// Upon restore, revert all changes since last save
	else if (engine->state != newState)
		newState->dirtyFlags |= CPaintEngine::DirtyFlags(static_cast<CPainterState *>(engine->state)->changeFlags);

	// We need to store all changes made so that restore can deal with them
	else
		newState->changeFlags |= newState->dirtyFlags;

	// Unset potential dirty background mode
	newState->dirtyFlags &= ~(CPaintEngine::DirtyBackgroundMode | CPaintEngine::DirtyBackground);

	engine->state = newState;
	engine->updateState(*newState);
	engine->clearDirty(CPaintEngine::AllDirty);
}

void ParaEngine::CPainter::updateState(CPainterState *newState)
{
	if (!newState) {
		engine->state = newState;
	}
	else if (newState->state() || engine->state != newState) {
		updateStateImpl(newState);
	}
}

bool ParaEngine::CPainter::IsScissorRectSupported() const
{
	return engine->IsScissorRectSupported();
}

bool ParaEngine::CPainter::IsStencilSupported() const
{
	return engine->IsStencilSupported();
}

void ParaEngine::CPainter::Flush()
{
	if (engine)
		engine->Flush();
}

void ParaEngine::CPainter::DrawSceneObject(CBaseObject * pObj, int nOption)
{
	if (pObj)
	{
		static std::vector<Vector3> output;
		output.clear();
		int nTriangleCount = pObj->GetMeshTriangleList(output, nOption);
		if (nTriangleCount > 0)
		{
			drawTriangles(&(output[0]), nTriangleCount);
		}
	}
}

void ParaEngine::CPainter::SetSpriteTransform(const Matrix4 * pMatrix /*= NULL*/)
{
	if (state && engine) {
		state->SetSpriteTransform(pMatrix);
		engine->transformChanged();
	}
}

ParaEngine::QPointF ParaEngine::CPainter::UIScaling() const
{
	return QPointF(state->m_fUIScalingX, state->m_fUIScalingY);
}

void ParaEngine::CPainter::setUIScaling(const QPointF &uiScaling)
{
	state->m_fUIScalingX = uiScaling.x();
	state->m_fUIScalingY = uiScaling.y();
	setViewTransformEnabled(!(state->m_viewportX == 0 && state->m_viewportY == 0 && state->m_fUIScalingX == 1.f && state->m_fUIScalingY == 1.f));
}


float ParaEngine::CPainter::FromUnitSpaceX(float fUnitspaceX)
{
	return state->m_viewportWidth * fUnitspaceX / state->m_fUIScalingX;
}

float ParaEngine::CPainter::FromUnitSpaceY(float fUnitspaceY)
{
	return state->m_viewportHeight * fUnitspaceY / state->m_fUIScalingY;
}

void ParaEngine::CPainter::setViewport(const QRect &viewport)
{
	state->m_viewportX = viewport.x();
	state->m_viewportY = viewport.y();
	state->m_fViewportLeft = (float)(state->m_viewportX);
	state->m_fViewportTop = (float)(state->m_viewportY);
	if (viewport.width() != 0)
	{
		state->m_viewportWidth = viewport.width();
		state->m_viewportHeight = viewport.height();
	}
	setViewTransformEnabled(!(state->m_viewportX == 0 && state->m_viewportY == 0 && state->m_fUIScalingX == 1.f && state->m_fUIScalingY == 1.f));
}

void ParaEngine::CPainter::SetHelperDevice(CPaintDevice * val)
{
	helper_device = val;
}

void ParaEngine::CPainter::SetUse3DTransform(bool bUse3DSpaceTransform)
{
	if (m_bUse3DTransform != bUse3DSpaceTransform)
	{
		m_bUse3DTransform = bUse3DSpaceTransform;

		if (engine)
		{
			engine->transformModeChanged();
		}
	}
}

void ParaEngine::CPainter::SetDepth2D(float fDepth)
{
	if (state)
		state->m_fDepth2D = fDepth;
}

bool ParaEngine::CPainter::IsUse3DTransform()
{
	return m_bUse3DTransform;
}

void ParaEngine::CPainter::SetSpriteUseWorldMatrix(bool bEnable)
{
	if (state)
		state->SetSpriteUseWorldMatrix(bEnable);
}

bool ParaEngine::CPainter::IsSpriteUseWorldMatrix()
{
	return state && state->IsSpriteUseWorldMatrix();
}

HRESULT CPainter::DrawRect(RECT* pRect, Color color, float depth)
{
	if (pRect && pRect->left != pRect->right) {
		Matrix4 matTransform(Matrix4::IDENTITY);
		matTransform.setScale(Vector3(1.f, 1.f, 0.f));
		matTransform._43 = depth;
		SetSpriteTransform(&matTransform);
		return engine->DrawRect(pRect, color, depth);
	}
	else
		return E_FAIL;
}


HRESULT CPainter::DrawSprite(GUITextureElement* pElement, RECT *prcDest, float depth)
{
	// no need to draw complete transparent element
	if (pElement == NULL || pElement->TextureColor.a == 0)
		return S_OK;
	HRESULT hr = S_OK;

	TextureEntity* pTexture = pElement->GetTexture();
	if (pTexture == NULL)
		return E_FAIL;

	RECT rcTexture;
	pElement->GetRect(&rcTexture);

	RECT rcScreen = *prcDest;

	if (pTexture->SurfaceType == TextureEntity::SysMemoryTexture)
	{
		//#ifdef USE_DIRECTX_RENDERER
		//		POINT pt = { rcScreen.left, rcScreen.top };
		//		hr = CGlobals::GetRenderDevice()->UpdateSurface(((TextureEntityDirectX*)pTexture)->GetSurface(), &rcTexture, guiroot->GetGUIState().pBackSurface, &pt);
		//#endif
	}
	else
	{
		if (pTexture->GetTexture() != 0)
		{
			if (RectWidth(rcTexture) != 0 || RectHeight(rcTexture) != 0)
			{
				float fScaleX = Math::Abs((float)RectWidth(rcScreen) / (RectWidth(rcTexture)));
				float fScaleY = Math::Abs((float)RectHeight(rcScreen) / (RectHeight(rcTexture)));

				Matrix4 matTransform(Matrix4::IDENTITY);
				matTransform.setScale(Vector3(fScaleX, fScaleY, 0.f));
				matTransform._43 = depth;
				SetSpriteTransform(&matTransform);

				Vector3 vPos((float)rcScreen.left, (float)rcScreen.top, 0.f);

				vPos.x /= fScaleX;
				vPos.y /= fScaleY;

				hr = engine->DrawQuad(pTexture, &rcTexture, NULL, &vPos, pElement->TextureColor);
			}
			else
				hr = E_FAIL;
		}
		else
		{
			if (pTexture->IsPending() && state)
				state->AddPendingAsset(1);
		}
	}

	return hr;
}

HRESULT CPainter::DrawSprite(GUITextureElement* pElement, RECT *prcDest, const Vector2& vRotOrigin, float fRadian, const Vector2* vScaling, const Vector2* vTranslation, const DWORD* dwColorMask, float depth)
{
	// no need to draw complete transparent element
	if (pElement == NULL || pElement->TextureColor.a == 0)
		return S_OK;
	HRESULT hr = S_OK;

	TextureEntity* pTexture = pElement->GetTexture();
	if (pTexture == NULL)
		return E_FAIL;

	RECT rcTexture;
	pElement->GetRect(&rcTexture);

	RECT rcScreen = *prcDest;

	if (pTexture->SurfaceType == TextureEntity::SysMemoryTexture)
	{
		//#ifdef USE_DIRECTX_RENDERER
		//		POINT pt = { rcScreen.left, rcScreen.top };
		//		hr = guiroot->m_stateGUI.pRenderDevice->UpdateSurface(((TextureEntityDirectX*)pTexture)->GetSurface(), &rcTexture, guiroot->GetGUIState().pBackSurface, &pt);
		//#endif
	}
	else
	{
		if (pTexture->GetTexture() != 0)
		{
			if (RectWidth(rcTexture) != 0 || RectHeight(rcTexture) != 0)
			{
				float fScaleX = Math::Abs((float)RectWidth(rcScreen) / RectWidth(rcTexture));
				float fScaleY = Math::Abs((float)RectHeight(rcScreen) / RectHeight(rcTexture));
				if (vScaling)
				{
					fScaleX *= vScaling->x;
					fScaleY *= vScaling->y;
				}

				//////////////////////////////////////////
				// the following code can do rotation: LiXizhi 2007.9.28
				//////////////////////////////////////////
				Matrix4 matTransform;
				Vector2 vPos((float)rcScreen.left, (float)rcScreen.top);
				if (vScaling || vTranslation || fRadian != 0.f)
				{
					vPos -= vRotOrigin;
					ParaMatrixTransformation2D(&matTransform, NULL, 0.0, vScaling, NULL, fRadian, vTranslation);
					vPos = vPos * matTransform;
					vPos += vRotOrigin;
				}

				// out, scaling center, scaling rotation, scaling, rotation center, rotation, translation
				Vector2 vScale(fScaleX, fScaleY);
				ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScale, NULL, fRadian, &vPos);
				matTransform._43 = depth;

				SetSpriteTransform(&matTransform);
				Color dwColor;
				if (dwColorMask == 0)
					dwColor = pElement->TextureColor;
				else
				{
					LinearColor colorMask(*dwColorMask);
					colorMask.a *= pElement->TextureColor.a;
					colorMask.r *= pElement->TextureColor.r;
					colorMask.g *= pElement->TextureColor.g;
					colorMask.b *= pElement->TextureColor.b;
					dwColor = colorMask;
				}

				hr = engine->DrawQuad(pTexture, &rcTexture, NULL, NULL, dwColor);
			}
			else
				hr = E_FAIL;
		}
		else
		{
			if (pTexture->IsPending() && state)
				state->AddPendingAsset(1);
		}
	}
	return hr;
}


HRESULT CPainter::DrawSprite(GUITextureElement* pElement, RECT *prcDest, Matrix4 matTransform, float depth)
{
	// no need to draw complete transparent element
	if (pElement == NULL || pElement->TextureColor.a == 0)
		return S_OK;

	TextureEntity* pTexture = pElement->GetTexture();
	if (pTexture == NULL)
		return E_FAIL;

	RECT rcTexture;
	pElement->GetRect(&rcTexture);

	RECT rcScreen = *prcDest;

	if (pTexture->SurfaceType == TextureEntity::SysMemoryTexture)
	{
		return DrawSprite(pElement, prcDest, depth);
	}

	if (pTexture->GetTexture() != 0)
	{
		if (RectWidth(rcTexture) != 0 || RectHeight(rcTexture) != 0)
		{
			float fScaleX = Math::Abs((float)RectWidth(rcScreen) / (RectWidth(rcTexture)));
			float fScaleY = Math::Abs((float)RectHeight(rcScreen) / (RectHeight(rcTexture)));

			matTransform._11 *= fScaleX;
			matTransform._22 *= fScaleY;
			matTransform._43 = depth;

			SetSpriteTransform(&matTransform);

			Vector3 vPos((float)rcScreen.left, (float)rcScreen.top, 0.f);

			vPos.x /= fScaleX;
			vPos.y /= fScaleY;
			return engine->DrawQuad(pTexture, &rcTexture, NULL, &vPos, pElement->TextureColor);
		}
		else
			return E_FAIL;
	}
	else
	{
		if (pTexture->IsPending() && state)
			state->AddPendingAsset(1);
	}
	return S_OK;
}

HRESULT CPainter::DrawSprite(GUITextureElement* pElement, RECT *prcDest, int n, float depth)
{
	// no need to draw complete transparent element
	if (pElement == NULL || pElement->TextureColor.a == 0)
		return S_OK;
	n = n % 4;

	TextureEntity* pTexture = pElement->GetTexture();
	if (pTexture == NULL)
		return E_FAIL;

	RECT rcTexture;
	pElement->GetRect(&rcTexture);

	RECT rcScreen = *prcDest;

	if (pTexture->SurfaceType == TextureEntity::SysMemoryTexture)
	{
		return DrawSprite(pElement, prcDest, depth);
	}

	if (pTexture->GetTexture() != 0)
	{
		if (RectWidth(rcTexture) != 0 || RectHeight(rcTexture) != 0)
		{
			float fScaleX;
			float fScaleY;
			Vector3 vCenter((float)RectWidth(rcTexture) / 2, (float)RectHeight(rcTexture) / 2, 0.f);
			Matrix4 matTransform;
			switch (n) {
			case 0:
				fScaleX = (float)RectWidth(rcScreen) / RectWidth(rcTexture);
				fScaleY = (float)RectHeight(rcScreen) / RectHeight(rcTexture);
				matTransform = Matrix4::IDENTITY;
				break;
			case 1:
			case -3:
				fScaleX = (float)RectWidth(rcScreen) / RectHeight(rcTexture);
				fScaleY = (float)RectHeight(rcScreen) / RectWidth(rcTexture);
				ParaMatrixRotationZ(&matTransform, (float)3.1415926536 / 2);
				break;
			case 2:
			case -2:
				fScaleX = (float)RectWidth(rcScreen) / RectWidth(rcTexture);
				fScaleY = (float)RectHeight(rcScreen) / RectHeight(rcTexture);
				ParaMatrixRotationZ(&matTransform, (float)3.1415926536 / 2);
				break;
			case 3:
			case -1:
				fScaleX = (float)RectWidth(rcScreen) / RectHeight(rcTexture);
				fScaleY = (float)RectHeight(rcScreen) / RectWidth(rcTexture);
				ParaMatrixRotationZ(&matTransform, (float)-3.1415926536 / 2);
				break;
			}
			fScaleX = Math::Abs(fScaleX);
			fScaleY = Math::Abs(fScaleY);
			matTransform._41 = (float)rcScreen.left + (float)RectWidth(rcScreen) / 2; matTransform._42 = (float)rcScreen.top + (float)RectHeight(rcScreen) / 2;
			matTransform._11 *= fScaleX;
			matTransform._21 *= fScaleX;
			matTransform._22 *= fScaleY;
			matTransform._12 *= fScaleY;
			matTransform._43 = depth;

			SetSpriteTransform(&matTransform);

			return engine->DrawQuad(pTexture, &rcTexture, &vCenter, NULL, pElement->TextureColor);
		}
		else
			return E_FAIL;
	}
	else
	{
		if (pTexture->IsPending() && state)
			state->AddPendingAsset(1);
	}
	return S_OK;
}

// font border offset. we will draw the texture multiple times to give it a black border.
/*
const int g_offsets[4][2] = {
{1,0},
{-1,0},
{0,1},
{0,-1},
//{1,1},
};
*/

//note:element order matters,careful when making any change!
const int g_offsets[8][2] = {
	{ 1, 0 },
{ -1, 0 },
{ 0, 1 },
{ 0, -1 },

{ 1, -1 },
{ 1, 1 },
{ -1, -1 },
{ -1, 1 },
};

HRESULT CPainter::DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, float depth,
	bool bShadow, int nCount, int shadowQuality, Color shadowColor_)
{
	HRESULT hr = S_OK;

	// No need to draw fully transparent layers
	if (pElement->FontColor.a == 0 || strText == NULL)
		return S_OK;
	RECT rcScreen = *prcDest;

	//debug
	//DrawRect( &rcScreen, COLOR_ARGB(100, 255, 0, 0) );

	Matrix4 matTransform(Matrix4::IDENTITY);
	matTransform._43 = depth;
	SetSpriteTransform(&matTransform);

	SpriteFontEntity* pFontNode = pElement->GetFont();

	if (pFontNode != NULL)
	{
#ifdef USE_OPENGL_RENDERER
		// CompositionModeStack blendModeStack(this, CPainter::CompositionMode_PlusSourceBlend);

		if (!(pElement->dwTextFormat & DT_NOCLIP))
		{
			if (state->m_clipOperation != ClipOperation::NoClip && !(state->m_clipInfo.empty()))
			{
				RECT parentRect = state->m_clipInfo[0].rect;
				RECT finalRect;
				if (!ParaEngine::CGUIBase::IntersectRect(&finalRect, state->m_clipInfo[0].rect, rcScreen))
				{
					return S_OK;
				}
				QRect clipRect(finalRect);
				engine->clip(clipRect, ClipOperation::IntersectClip);
			}
			else
			{
				QRect clipRect(rcScreen);
				engine->clip(clipRect, ClipOperation::ReplaceClip);
			}
		}
#endif


		do
		{
			if (bShadow)
			{
				DWORD shadowColor = shadowColor_;
				if (shadowColor == 0)
					shadowColor = ComputeTextShadowColor(pElement->FontColor);

				int alpha = (shadowColor & 0xff000000) % 0xff;
				alpha = (int)(alpha * pElement->FontColor.a);
				shadowColor &= 0xffffff;
				shadowColor += (alpha << 24);

				int startIdx = 5;
				int endIdx = 6;
				if (shadowQuality == 4)
				{
					startIdx = 0;
					endIdx = 4;
				}
				else if (shadowQuality == 8)
				{
					startIdx = 0;
					endIdx = 8;
				}

				for (int i = startIdx; i < endIdx; ++i)
				{
					RECT rcShadow = rcScreen;
					CGUIBase::OffsetRect(&rcShadow, g_offsets[i][0], g_offsets[i][1]);
					/*
					V_RETURN(engine->drawText(pFontNode, strText, \
					nCount, &rcShadow, pElement->dwTextFormat, shadowColor));
					*/

					hr = engine->drawText(pFontNode, strText, \
						nCount, &rcShadow, pElement->dwTextFormat, shadowColor);
					if (FAILED(hr))
						break;
				}

				if (FAILED(hr))
					break;
			}

			hr = engine->drawText(pFontNode, strText, nCount, &rcScreen, pElement->dwTextFormat, pElement->FontColor);


		} while (false);

#ifdef USE_OPENGL_RENDERER
		if (!(pElement->dwTextFormat & DT_NOCLIP))
		{
			if (state->m_clipOperation != ClipOperation::NoClip && !(state->m_clipInfo.empty()))
			{
				engine->clip(state->m_clipInfo[0].rect, state->m_clipInfo[0].operation);
			}
			else
			{
				QRect empty;
				engine->clip(empty, ClipOperation::NoClip);
			}
		}
#endif

		return hr;
	}
	else {
		// OUTPUT_LOG("fontnode==null\n");
	}

	return S_OK;
}


HRESULT CPainter::DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, const Matrix4& mat, const DWORD* dwColorMask, float depth, bool bShadow /*= false*/, int nCount /*= -1*/, int shadowQuality /*= 0*/, Color shadowColor_ /*= 0*/)
{
	HRESULT hr = S_OK;

	// No need to draw fully transparent layers
	if (pElement->FontColor.a == 0 || strText == NULL)
		return S_OK;

	RECT rcScreen = *prcDest;

	SetSpriteTransform(&mat);

	SpriteFontEntity* pFontNode = pElement->GetFont();

	if (pFontNode != NULL)
	{
#ifdef USE_OPENGL_RENDERER
		// CompositionModeStack blendModeStack(this, CPainter::CompositionMode_PlusSourceBlend);

		if (!(pElement->dwTextFormat & DT_NOCLIP))
		{
			if (state->m_clipOperation != ClipOperation::NoClip && !(state->m_clipInfo.empty()))
			{
				RECT parentRect = state->m_clipInfo[0].rect;
				RECT finalRect;
				if (!ParaEngine::CGUIBase::IntersectRect(&finalRect, state->m_clipInfo[0].rect, rcScreen))
				{
					return S_OK;
				}
				QRect clipRect(finalRect);
				engine->clip(clipRect, ClipOperation::IntersectClip);
			}
			else
			{
				QRect clipRect(rcScreen);
				engine->clip(clipRect, ClipOperation::ReplaceClip);
			}
		}
#endif


		LinearColor color = pElement->FontColor;
		if (dwColorMask)
		{
			LinearColor colorMask(*dwColorMask);
			color.a *= colorMask.a;
			color.r *= colorMask.r;
			color.g *= colorMask.g;
			color.b *= colorMask.b;
		}

		do
		{
			if (bShadow)
			{
				DWORD shadowColor = shadowColor_;
				if (shadowColor == 0)
					shadowColor = ComputeTextShadowColor(pElement->FontColor);

				int alpha = (shadowColor & 0xff000000) % 0xff;
				alpha = (int)(alpha*color.a);
				shadowColor &= 0xffffff;
				shadowColor += (alpha << 24);

				int startIdx = 5;
				int endIdx = 6;
				if (shadowQuality == 4)
				{
					startIdx = 0;
					endIdx = 4;
				}
				else if (shadowQuality == 8)
				{
					startIdx = 0;
					endIdx = 8;
				}

				for (int i = startIdx; i < endIdx; ++i)
				{
					RECT rcShadow = rcScreen;
					CGUIBase::OffsetRect(&rcShadow, g_offsets[i][0], g_offsets[i][1]);
					hr = (engine->drawText(pFontNode, strText, \
						nCount, &rcShadow, pElement->dwTextFormat, shadowColor));

					if (FAILED(hr))
						break;
				}

				if (FAILED(hr))
					break;
			}

			hr = (engine->drawText(pFontNode, strText, nCount, &rcScreen, pElement->dwTextFormat, color));

		} while (false);

#ifdef USE_OPENGL_RENDERER
		if (!(pElement->dwTextFormat & DT_NOCLIP))
		{
			if (state->m_clipOperation != ClipOperation::NoClip && !(state->m_clipInfo.empty()))
			{
				engine->clip(state->m_clipInfo[0].rect, state->m_clipInfo[0].operation);
			}
			else
			{
				QRect empty;
				engine->clip(empty, ClipOperation::NoClip);
			}
		}
#endif

		return hr;

	}
	else {
		// OUTPUT_LOG("fontnode==null\n");
	}

	return S_OK;
}

HRESULT CPainter::DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, const Vector2& vRotOrigin,
	float fRadian, const Vector2* vScaling, const Vector2* vTranslation, const DWORD* dwColorMask, float depth,
	bool bShadow, int nCount, int shadowQuality, Color shadowColor_)
{
	HRESULT hr = S_OK;
	// No need to draw fully transparent layers
	if (pElement->FontColor.a == 0 || strText == NULL)
		return S_OK;
	RECT rcScreen = *prcDest;

	//////////////////////////////////////////
	// the following code can do rotation: LiXizhi 2007.9.28
	//////////////////////////////////////////
	Matrix4 matTransform;
	Vector2 vPos((float)rcScreen.left, (float)rcScreen.top);
	if (vScaling || vTranslation || fRadian != 0.f)
	{
		vPos -= vRotOrigin;
		ParaMatrixTransformation2D(&matTransform, NULL, 0.0, vScaling, NULL, fRadian, vTranslation);
		vPos = vPos * matTransform;
		vPos += vRotOrigin;
	}
	rcScreen.right -= rcScreen.left;
	rcScreen.left = 0;
	rcScreen.bottom -= rcScreen.top;
	rcScreen.top = 0;
	// out, scaling center, scaling rotation, scaling, rotation center, rotation, translation
	ParaMatrixTransformation2D(&matTransform, NULL, 0.0, vScaling, NULL, fRadian, &vPos);
	matTransform._43 = depth;
	return DrawText(strText, pElement, &rcScreen, matTransform, dwColorMask, depth, bShadow, nCount, shadowQuality, shadowColor_);
}


HRESULT CPainter::CalcTextRect(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, float depth, int nCount)
{
	HRESULT hr = S_OK;
	SpriteFontEntity* pFontNode = pElement->GetFont();

	if (pFontNode == NULL || strText == NULL)
		return E_FAIL;

	DWORD dwTextFormat = pElement->dwTextFormat | DT_CALCRECT;
	// Since we are only computing the rectangle, we don't need a sprite.
	if (pFontNode != NULL) {
		auto pEngine = engine;
		if (!pEngine && helper_device) {
			pEngine = helper_device->paintEngine();
		}
		if (pEngine) {
			V_RETURN(pEngine->drawText(pFontNode, strText, nCount, prcDest, dwTextFormat, pElement->FontColor));
		}
	}
	return S_OK;
}


void ParaEngine::CPainter::drawPoints(const QPointF *points, int pointCount)
{
	if (!engine || !state)
		return;
	Color color = state->color();
	for (int i = 0; i < pointCount; ++i)
	{
		const QPointF& point = points[i];
		float fPointRadius = state->m_pen.widthF()*0.5f;
		int32 fromX = (int)(point.x() - fPointRadius);
		int32 fromY = (int)(point.y() - fPointRadius);
		int nPointSize = Math::Max(1, (int)(fPointRadius * 2));
		RECT rect = { fromX, fromY, fromX + nPointSize, fromY + nPointSize };
		engine->DrawRect(&rect, color, 0.f);
	}
}

void ParaEngine::CPainter::drawPoints(const QPoint *points, int pointCount)
{
	if (!engine || !state)
		return;
	Color color = state->color();
	for (int i = 0; i < pointCount; ++i)
	{
		const QPoint& point = points[i];
		float fPointRadius = state->m_pen.widthF()*0.5f;
		int32 fromX = (int)(point.x() - fPointRadius);
		int32 fromY = (int)(point.y() - fPointRadius);
		int nPointSize = Math::Max(1, (int)(fPointRadius * 2));
		RECT rect = { fromX, fromY, fromX + nPointSize, fromY + nPointSize };
		engine->DrawRect(&rect, color, 0.f);
	}
}


void ParaEngine::CPainter::drawTriangles(const Vector3* vertices, int nTriangleCount)
{
	engine->drawTriangles(vertices, nTriangleCount);
}

void ParaEngine::CPainter::drawLines(const QLineF *lines, int lineCount)
{
	engine->drawLines(lines, lineCount);
}

void ParaEngine::CPainter::drawLines(const QPointF *pointPairs, int lineCount)
{
	// TODO: 
}

void ParaEngine::CPainter::drawLines(const QLine *lines, int lineCount)
{
	engine->drawLines(lines, lineCount);
}

void ParaEngine::CPainter::drawLines(const QPoint *pointPairs, int lineCount)
{
	// TODO: 
}

void ParaEngine::CPainter::drawLines(const Vector3* vertices, int nLineCount)
{
	engine->drawLines(vertices, nLineCount);
}

void ParaEngine::CPainter::drawRects(const QRectF *rects, int rectCount)
{
	Color color = state->color();
	for (int i = 0; i < rectCount; ++i)
	{
		RECT rect = rects[i];
		engine->DrawRect(&rect, color, 0.f);
	}
}

void ParaEngine::CPainter::drawRects(const QRect *rects, int rectCount)
{
	if (!engine || !state)
		return;
	Color color = state->color();
	for (int i = 0; i < rectCount; ++i)
	{
		RECT rect = rects[i];
		engine->DrawRect(&rect, color, 0.f);
	}
}

void ParaEngine::CPainter::drawTexture(const QRectF &targetRect, TextureEntity* pTexture, const QRectF &sourceRect, float fDepth)
{
	if (!engine || !state || !pTexture)
		return;
	if (pTexture->GetTexture())
	{
		QRect srcRect(sourceRect);

		Color color = state->color();

		if (srcRect.isEmpty()) {
			srcRect.setWidth(pTexture->GetWidth());
			srcRect.setHeight(pTexture->GetHeight());
		}
		if (!srcRect.isEmpty())
		{
			if ((int)targetRect.width() != srcRect.width() || (int)targetRect.height() != srcRect.height())
			{
				float fScaleX = Math::Abs(targetRect.width() / (srcRect.width()));
				float fScaleY = Math::Abs(targetRect.height() / (srcRect.height()));

				QTransform oldTranform = worldTransform();
				QTransform t = QTransform::fromScale(fScaleX, fScaleY) * oldTranform;
				setWorldTransform(t);
				Vector3 vPos(targetRect.x(), targetRect.y(), fDepth);
				vPos.x /= fScaleX;
				vPos.y /= fScaleY;
				RECT src_rect = srcRect;
				engine->DrawQuad(pTexture, &src_rect, NULL, &vPos, color);
				setWorldTransform(oldTranform);
			}
			else
			{
				Vector3 vPos(targetRect.x(), targetRect.y(), fDepth);
				RECT src_rect = srcRect;
				engine->DrawQuad(pTexture, &src_rect, NULL, &vPos, color);
			}
		}
	}
	else
	{
		if (pTexture->IsPending() && state)
			state->AddPendingAsset(1);
	}
}

void ParaEngine::CPainter::drawTexture(const QPointF &p, TextureEntity* pTexture, float fDepth)
{
	if (!engine || !state || !pTexture)
		return;
	if (pTexture->GetTexture())
	{
		Color color = state->color();
		QRect srcRect(0, 0, pTexture->GetWidth(), pTexture->GetHeight());
		if (!srcRect.isEmpty())
		{
			Vector3 vPos(p.x(), p.y(), fDepth);
			RECT src_rect = srcRect;
			engine->DrawQuad(pTexture, &src_rect, NULL, &vPos, color);
		}
	}
	else
	{
		if (pTexture->IsPending() && state)
			state->AddPendingAsset(1);
	}
}

void ParaEngine::CPainter::drawText(const QPointF &p, const std::string &text)
{
	if (!engine || !state)
		return;
	RECT rect = { (int)p.x(), (int)p.y(), std::max((int)p.x(), 0) + 5000, (int)p.y() + 100 };
	Color color = state->color();

	engine->drawText(state->m_font.GetSpriteFont(), text.c_str(), text.size(), &rect, QTextOption(), color);
}

void ParaEngine::CPainter::drawText(const QRectF &r, const std::string &text, const QTextOption &o /*= QTextOption()*/)
{
	if (!engine || !state)
		return;
	RECT rect = r;
	Color color = state->color();
	engine->drawText(state->m_font.GetSpriteFont(), text.c_str(), text.size(), &rect, o, color);
}

void ParaEngine::CPainter::setWorldTransform(const QTransform &matrix, bool combine /*= false*/)
{
	if (combine)
		state->worldMatrix = matrix * state->worldMatrix;
	else
		state->worldMatrix = matrix;

	state->WxF = true;
	updateMatrix();
}

const QTransform & ParaEngine::CPainter::worldTransform() const
{
	if (!engine) {
		OUTPUT_LOG("warning: CPainter::worldTransform: Painter not active\n");
		return fakeState()->transform;
	}
	return state->worldMatrix;
}

const QTransform & ParaEngine::CPainter::deviceTransform() const
{
	return state->m_matrix;
}

void ParaEngine::CPainter::resetTransform()
{
	state->m_viewportX = state->m_viewportY = 0;                        // default view origins
	state->m_viewportWidth = device()->metric(CPaintDevice::PdmWidth);
	state->m_viewportHeight = device()->metric(CPaintDevice::PdmHeight);
	state->m_fUIScalingX = state->m_fUIScalingY = 1.f;
	state->worldMatrix = QTransform();
	setWorldMatrixEnabled(false);
	setViewTransformEnabled(false);
}

ParaEngine::QTransform ParaEngine::CPainter::combinedTransform() const
{
	if (!engine) {
		OUTPUT_LOG("warning: CPainter::combinedTransform: Painter not active\n");
		return QTransform();
	}
	return state->worldMatrix * viewTransform();
}

CPainterDummyState * ParaEngine::CPainter::fakeState() const
{
	static CPainterDummyState s_dummyState;
	return &s_dummyState;
}

ParaEngine::QTransform ParaEngine::CPainter::viewTransform() const
{
	if (state->VxF) {
		return QTransform(state->m_fUIScalingX, 0, 0, state->m_fUIScalingY, state->m_fViewportLeft, state->m_fViewportTop);
	}
	return QTransform();
}

void ParaEngine::CPainter::setWorldMatrixEnabled(bool enabled)
{
	if (enabled == state->WxF)
		return;
	state->WxF = enabled;
	updateMatrix();
}

bool ParaEngine::CPainter::worldMatrixEnabled() const
{
	return state->WxF;
}

void ParaEngine::CPainter::setViewTransformEnabled(bool enable)
{
	if (enable == state->VxF)
		return;
	state->VxF = enable;
	updateMatrix();
}

void ParaEngine::CPainter::updateMatrix()
{
	state->m_matrix = state->WxF ? state->worldMatrix : QTransform();
	/*
	if (state->VxF)
	state->m_matrix *= viewTransform();*/

	const QMatrix& mat = state->worldMatrix.toAffine();
	Matrix4 mat4x4(mat);
	state->SetSpriteTransform(&mat4x4);

	engine->transformChanged();
}

bool ParaEngine::CPainter::viewTransformEnabled() const
{
	return state->VxF;
}

void ParaEngine::CPainter::scale(float sx, float sy)
{
	state->worldMatrix.scale(sx, sy);
	state->WxF = true;
	updateMatrix();
}

void ParaEngine::CPainter::shear(float sh, float sv)
{
	state->worldMatrix.shear(sh, sv);
	state->WxF = true;
	updateMatrix();
}

void ParaEngine::CPainter::rotate(float a)
{
	state->worldMatrix.rotate(a);
	state->WxF = true;
	updateMatrix();
}

void ParaEngine::CPainter::translate(const QPointF &offset)
{
	float dx = offset.x();
	float dy = offset.y();
	state->worldMatrix.translate(dx, dy);
	state->WxF = true;
	updateMatrix();
}

void ParaEngine::CPainter::drawPolyline(const QPointF *points, int pointCount)
{
	if (!engine || pointCount < 2)
		return;
	engine->drawPolygon(points, pointCount, CPaintEngine::PolylineMode);
}

void ParaEngine::CPainter::drawPolyline(const QPoint *points, int pointCount)
{
	if (!engine || pointCount < 2)
		return;
	engine->drawPolygon(points, pointCount, CPaintEngine::PolylineMode);
}

void ParaEngine::CPainter::drawPolygon(const QPointF *points, int pointCount, FillRule fillRule /*= OddEvenFill*/)
{
	if (!engine || pointCount < 2)
		return;
	engine->drawPolygon(points, pointCount, CPaintEngine::PolygonDrawMode(fillRule));
}

void ParaEngine::CPainter::drawPolygon(const QPoint *points, int pointCount, FillRule fillRule /*= OddEvenFill*/)
{
	if (!engine || pointCount < 2)
		return;
	engine->drawPolygon(points, pointCount, CPaintEngine::PolygonDrawMode(fillRule));
}

const QFont & ParaEngine::CPainter::font() const
{
	if (!engine || !state)
		return fakeState()->font;
	return state->m_font;
}

void ParaEngine::CPainter::setFont(const QFont &f)
{
	if (!engine || !state)
		return;
	state->m_font = f;
}

void ParaEngine::CPainter::setPen(const Color &color)
{
	if (!engine || !state)
		return;
	state->m_pen.setColor(color);
}

void ParaEngine::CPainter::setPen(const QPen &pen)
{
	if (!engine || !state)
		return;
	if (state->m_pen != pen)
	{
		state->m_pen = pen;
		engine->penChanged();
	}
}

void ParaEngine::CPainter::setPen(PenStyle style)
{
	if (!engine || !state)
		return;
	setPen(QPen(style));
}

const QPen & ParaEngine::CPainter::pen() const
{
	if (!engine || !state)
		return fakeState()->pen;
	return state->m_pen;
}

void ParaEngine::CPainter::setBrush(const QBrush &brush)
{
	if (!engine || !state)
		return;
	if (state->m_brush != brush) {
		state->m_brush = brush;
		engine->brushChanged();
	}
}

void ParaEngine::CPainter::setBrush(BrushStyle style)
{
	if (state->m_brush.style() == style &&
		(style == NoBrush || (style == SolidPattern && state->m_brush.color() == Color::Black)))
		return;
	state->m_brush = QBrush(Color::Black, style);
	engine->brushChanged();
}

const QBrush & ParaEngine::CPainter::brush() const
{
	if (!engine || !state)
		return fakeState()->brush;
	return state->m_brush;
}

ParaEngine::QPoint ParaEngine::CPainter::brushOrigin() const
{
	if (!engine || !state)
		return QPoint();
	return QPointF(state->m_brushOrigin).toPoint();
}

void ParaEngine::CPainter::setBrushOrigin(const QPointF & p)
{
	if (!engine || !state)
		return;
	state->m_brushOrigin = p;

	engine->brushOriginChanged();
}

void ParaEngine::CPainter::setBackground(const QBrush &bg)
{
	if (!engine || !state)
		return;
	state->m_bgBrush = bg;
}

const QBrush & ParaEngine::CPainter::background() const
{
	if (!engine || !state)
		return fakeState()->brush;
	return state->m_bgBrush;
}

float ParaEngine::CPainter::opacity() const
{
	if (!engine || !state)
		return 1.f;
	return state->m_opacity;
}

void ParaEngine::CPainter::setOpacity(float opacity)
{
	if (!engine || !state)
		return;
	opacity = Math::Min(1.f, Math::Max(0.f, opacity));
	if (state->m_opacity != opacity)
	{
		state->m_opacity = opacity;
		engine->opacityChanged();
	}
}

ParaEngine::QRegion ParaEngine::CPainter::clipRegion() const
{
	if (!engine)
		return QRegion();
	return QRegion();
}

void ParaEngine::CPainter::setClipRect(const QRectF &, ClipOperation op /*= ReplaceClip*/)
{

}

void ParaEngine::CPainter::setClipRect(const QRect & rect, ClipOperation op /*= ReplaceClip*/)
{
	if (!engine || !state)
		return;
	if ((!state->m_clipEnabled && op != NoClip))
		op = ReplaceClip;
	state->m_clipEnabled = true;
	engine->clip(rect, op);
	if (op == ReplaceClip || op == NoClip)
		state->m_clipInfo.clear();
	state->m_clipInfo.push_back(CPainterClipInfo(rect, op, state->m_matrix));
	state->m_clipOperation = op;
	return;
}

void ParaEngine::CPainter::setClipRegion(const QRegion & region, ClipOperation op /*= ReplaceClip*/)
{
	QRect rect = region.boundingRect();
	setClipRect(rect, op);
}

void ParaEngine::CPainter::setClipPath(const QPainterPath &path, ClipOperation op /*= ReplaceClip*/)
{

}

void ParaEngine::CPainter::setClipping(bool enable)
{
	if (!engine || !state)
		return;
	if (hasClipping() == enable)
		return;
	// we can't enable clipping if we don't have a clip
	if (enable
		&& (state->m_clipInfo.empty() || state->m_clipInfo.back().operation == NoClip))
		return;
	state->m_clipEnabled = enable;
	engine->clipEnabledChanged();
}

bool ParaEngine::CPainter::hasClipping() const
{
	if (!engine || !state)
		return false;
	return state->m_clipEnabled && state->m_clipOperation != NoClip;
}

ParaEngine::QRectF ParaEngine::CPainter::clipBoundingRect() const
{
	return QRectF();
}

void ParaEngine::CPainter::save()
{
	if (!engine || !state)
		return;

	state = engine->createState(states.back());
	engine->setState(state);
	states.push_back(state);
}

void ParaEngine::CPainter::restore()
{
	if (!engine || !state)
		return;
	CPainterState *tmp = state;
	states.pop_back();
	state = states.back();

	engine->setState(state);
	delete tmp;
}

void ParaEngine::CPainter::setCompositionMode(CompositionMode mode)
{
	if (!engine || !state)
		return;
	if (state->m_composition_mode == mode)
		return;
	// because composition has changed, we will render any cached quads with last composition mode. 
	Flush();
	state->m_composition_mode = mode;
	engine->compositionModeChanged();
}

ParaEngine::CPainter::CompositionMode ParaEngine::CPainter::compositionMode() const
{
	return state ? state->m_composition_mode : CompositionMode_SourceOver;
}

int ParaEngine::CPainter::GetPendingAssetCount()
{
	return state ? state->m_nPendingAssetCount : 0;
}

bool ParaEngine::CPainter::IsAutoLineWidth() const
{
	return m_bAutoLineWidth;
}

void ParaEngine::CPainter::EnableAutoLineWidth(bool val)
{
	m_bAutoLineWidth = val;
}

int ParaEngine::CPainter::GetMatrixMode() const
{
	return m_nMatrixMode;
}

void ParaEngine::CPainter::SetMatrixMode(int val)
{
	m_nMatrixMode = val;
}

TransformStack& ParaEngine::CPainter::GetMatrixStack()
{
	if (m_nMatrixMode == 0)
		return CGlobals::GetWorldMatrixStack();
	else if (m_nMatrixMode == 1)
		return CGlobals::GetViewMatrixStack();
	else if (m_nMatrixMode == 2)
		return CGlobals::GetProjectionMatrixStack();
	else
		return CGlobals::GetWorldMatrixStack();
}

void ParaEngine::CPainter::PushMatrix()
{
	GetMatrixStack().push(m_curMatrix);
	engine->transformChanged();
}

void ParaEngine::CPainter::PopMatrix()
{
	m_curMatrix = GetMatrixStack().SafeGetTop();
	GetMatrixStack().pop();
	engine->transformChanged();
}

void ParaEngine::CPainter::LoadCurrentMatrix()
{
	m_curMatrix = GetMatrixStack().SafeGetTop();
}

void ParaEngine::CPainter::LoadIdentityMatrix()
{
	m_curMatrix.identity();
	engine->transformChanged();
}

void ParaEngine::CPainter::LoadBillboardMatrix()
{
	Matrix4 mat;
	Math::CreateBillboardMatrix(&mat, NULL, NULL, true);

	// to rotate 180 degrees for ease of composing.
	Quaternion q;
	q.FromAngleAxis(Radian(Math::PI), Vector3::UNIT_Y);
	Matrix4 matRot;
	q.ToRotationMatrix(matRot, Vector3::ZERO);
	mat = matRot * mat;

	MultiplyMatrix(mat);
}

void ParaEngine::CPainter::LoadMatrix(const Matrix4& mat)
{
	m_curMatrix = mat;
	engine->transformChanged();
}

void ParaEngine::CPainter::MultiplyMatrix(const Matrix4& mat)
{
	m_curMatrix = mat.Multiply4x3(m_curMatrix);
	engine->transformChanged();
}

void ParaEngine::CPainter::TranslateMatrix(float x, float y, float z)
{
	Matrix4 mat;
	mat.makeTrans(x, y, z);
	m_curMatrix = mat.Multiply4x3(m_curMatrix);
	engine->transformChanged();
}

void ParaEngine::CPainter::RotateMatrix(float angle, float x, float y, float z)
{
	Matrix4 mat;
	mat.makeRot(Quaternion(Vector3(x, y, z), angle), Vector3::ZERO);
	m_curMatrix = mat.Multiply4x3(m_curMatrix);
	engine->transformChanged();
}

void ParaEngine::CPainter::ScaleMatrix(float x, float y, float z)
{
	Matrix4 mat;
	mat.makeScale(x, y, z);
	m_curMatrix = mat.Multiply4x3(m_curMatrix);
	engine->transformChanged();
}

const ParaEngine::Matrix4& ParaEngine::CPainter::GetCurMatrix() const
{
	return m_curMatrix;
}

Vector3 ParaEngine::CPainter::GetScaling()
{
	return m_curMatrix.GetScaleVector();
}

int ParaEngine::CPainter::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("CompositionMode", FieldType_Int, (void*)SetCompositionMode_s, (void*)GetCompositionMode_s, NULL, "", bOverride);
	pClass->AddField("Opacity", FieldType_Float, (void*)SetOpacity_s, (void*)GetOpacity_s, NULL, "", bOverride);
	pClass->AddField("Save", FieldType_void, (void*)Save_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("Restore", FieldType_void, (void*)Restore_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("Flush", FieldType_void, (void*)Flush_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("BeginNativePainting", FieldType_void, (void*)BeginNativePainting_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("EndNativePainting", FieldType_void, (void*)EndNativePainting_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("ResetTransform", FieldType_void, (void*)ResetTransform_s, (void*)0, NULL, "", bOverride);

	pClass->AddField("MatrixMode", FieldType_Int, (void*)SetMatrixMode_s, (void*)GetMatrixMode_s, NULL, "", bOverride);
	pClass->AddField("LoadIdentityMatrix", FieldType_void, (void*)LoadIdentityMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("LoadCurrentMatrix", FieldType_void, (void*)LoadCurrentMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("LoadBillboardMatrix", FieldType_void, (void*)LoadBillboardMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("PopMatrix", FieldType_void, (void*)PopMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("PushMatrix", FieldType_void, (void*)PushMatrix_s, (void*)0, NULL, "", bOverride);

	pClass->AddField("TranslateMatrix", FieldType_Float_Float_Float, (void*)TranslateMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("ScaleMatrix", FieldType_Float_Float_Float, (void*)ScaleMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("RotateMatrix", FieldType_Float_Float_Float_Float, (void*)RotateMatrix_s, (void*)0, NULL, "", bOverride);

	pClass->AddField("LoadMatrix", FieldType_Matrix4, (void*)LoadMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("MultiplyMatrix", FieldType_Matrix4, (void*)MultiplyMatrix_s, (void*)0, NULL, "", bOverride);

	pClass->AddField("CurrentMatrix", FieldType_Matrix4, (void*)0, (void*)GetCurrentMatrix_s, NULL, "", bOverride);
	pClass->AddField("Scaling", FieldType_Vector3, (void*)0, (void*)GetScaling_s, NULL, "", bOverride);
	pClass->AddField("AutoLineWidth", FieldType_Bool, (void*)EnableAutoLineWidth_s, (void*)IsAutoLineWidth_s, NULL, "", bOverride);
	pClass->AddField("UseWorldMatrix", FieldType_Bool, (void*)SetSpriteUseWorldMatrix_s, (void*)IsSpriteUseWorldMatrix_s, NULL, "", bOverride);
	return S_OK;
}

