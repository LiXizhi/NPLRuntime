//-----------------------------------------------------------------------------
// Class: PainterState
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.27
// Desc: I have referred to QT framework's qpaintdevice.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PainterState.h"
using namespace ParaEngine;

ParaEngine::CPainterState::CPainterState()
	:m_brushOrigin(0, 0),
	m_viewportX(0), m_viewportY(0), m_viewportWidth(0), m_viewportHeight(0), 
	m_fViewportLeft(0), m_fViewportTop(0), m_fUIScalingX(1.f), m_fUIScalingY(1.f),
	m_opacity(1.f), WxF(false), VxF(false), m_clipEnabled(false),
	m_painter(0), m_nPendingAssetCount(0), matComplete(Matrix4::IDENTITY),
	m_composition_mode(CPainter::CompositionMode_SourceBlend), changeFlags(0)
{
	dirtyFlags = 0;
}

ParaEngine::CPainterState::CPainterState(const CPainterState *s)
	:m_brushOrigin(s->m_brushOrigin),
	m_viewportX(s->m_viewportX), m_viewportY(s->m_viewportY), m_viewportWidth(s->m_viewportWidth), m_viewportHeight(s->m_viewportHeight), 
	m_fViewportLeft(s->m_fViewportLeft), m_fViewportTop(s->m_fViewportTop), m_fUIScalingX(s->m_fUIScalingX), m_fUIScalingY(s->m_fUIScalingY),
	m_opacity(s->m_opacity), WxF(s->WxF), VxF(s->VxF),
	worldMatrix(s->worldMatrix), m_matrix(s->m_matrix), 
	m_clipEnabled(s->m_clipEnabled), m_painter(s->m_painter),
	m_composition_mode(s->m_composition_mode), m_nPendingAssetCount(s->m_nPendingAssetCount),
	m_clipInfo(s->m_clipInfo), matComplete(s->matComplete),
	changeFlags(0)
{
	dirtyFlags = s->dirtyFlags;
}

ParaEngine::CPainterState::~CPainterState()
{

}

void ParaEngine::CPainterState::init(CPainter *p)
{
}

void ParaEngine::CPainterState::SetSpriteTransform(const Matrix4 * pMatrix /*= NULL*/)
{
	matComplete = (pMatrix != NULL) ? *pMatrix : Matrix4::IDENTITY;
}

float ParaEngine::CPainterState::opacity() const
{
	return m_opacity;
}

CPainter::CompositionMode ParaEngine::CPainterState::compositionMode() const
{
	return m_composition_mode;
}

ParaEngine::QPen ParaEngine::CPainterState::pen() const
{
	return m_pen;
}

ParaEngine::QBrush ParaEngine::CPainterState::brush() const
{
	return m_brush;
}

ParaEngine::QPointF ParaEngine::CPainterState::brushOrigin() const
{
	return m_brushOrigin;
}

ParaEngine::QBrush ParaEngine::CPainterState::backgroundBrush() const
{
	return m_bgBrush;
}

ParaEngine::QFont ParaEngine::CPainterState::font() const
{
	return m_font;
}

ParaEngine::QMatrix ParaEngine::CPainterState::matrix() const
{
	return m_matrix.toAffine();
}

ParaEngine::QTransform ParaEngine::CPainterState::transform() const
{
	return m_matrix;
}

ParaEngine::ClipOperation ParaEngine::CPainterState::clipOperation() const
{
	return m_clipOperation;
}

ParaEngine::QRegion ParaEngine::CPainterState::clipRegion() const
{
	return m_clipRegion;
}

ParaEngine::QPainterPath ParaEngine::CPainterState::clipPath() const
{
	return m_clipPath;
}

bool ParaEngine::CPainterState::isClipEnabled() const
{
	return m_clipEnabled;
}

void ParaEngine::CPainterState::AddPendingAsset(int nCount /*= 1*/)
{
	m_nPendingAssetCount += nCount;
}

void ParaEngine::CPainterState::CalculateDeviceMatrix(Matrix4* pOut, const Matrix4* pIn)
{
	if (pOut)
	{
		Matrix4 mat = pIn ? *pIn : matComplete;

		if (!IsSpriteUseWorldMatrix())
		{
			if (m_fUIScalingX != 1.f)
			{
				mat._11 *= m_fUIScalingX;
				mat._21 *= m_fUIScalingX;
				mat._41 *= m_fUIScalingX;
			}
			if (m_fUIScalingY != 1.f)
			{
				mat._12 *= m_fUIScalingY;
				mat._22 *= m_fUIScalingY;
				mat._42 *= m_fUIScalingY;
			}
			mat._41 += m_fViewportLeft;
			mat._42 += m_fViewportTop;
		}
		else
		{
			// use current 3d world transform, such as rendering head on text or overlays
			// here we will invert Y value, since directX GUI rendering use Y downward coordinate. 
			mat._12 = -mat._12;mat._22 = -mat._22;mat._32 = -mat._32;mat._42 = -mat._42;
			mat = mat * m_painter->GetCurMatrix();
		}
		*pOut = mat;
	}
}

Color ParaEngine::CPainterState::color()
{
	if (opacity() == 1.0)
		return m_pen.color();
	else
		return m_pen.color().MutiplyOpacity(opacity());
}
