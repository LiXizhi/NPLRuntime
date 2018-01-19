//----------------------------------------------------------------------
// Class:	CGUIPosition
// Authors:	LiXizhi
// Date:	2005.8.1
// desc: moved to new class file 2014.8.9
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIPosition.h"
#include "IScene.h"
#include "BaseObject.h"

using namespace ParaEngine;

ParaEngine::CGUIPosition::CGUIPosition() :m_fDepth(0), m_nPositionType(relative_to_parent), m_n3DObjectID(0)
{
	memset(&rect, 0, sizeof(RECT));
	memset(&Relative, 0, sizeof(_relativity));
}

ParaEngine::CGUIPosition::CGUIPosition(const RECT& rect_) :rect(rect_), m_fDepth(0), m_nPositionType(relative_to_parent), m_n3DObjectID(0)
{
	memset(&Relative, 0, sizeof(_relativity));
}

ParaEngine::CGUIPosition::CGUIPosition(int x, int y, int width, int height)
	: m_nPositionType(relative_to_parent)
{
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	m_fDepth = 0;
	
	memset(&Relative, 0, sizeof(_relativity));
}

void ParaEngine::CGUIPosition::SetDepth(float Depth)
{
	if (m_nPositionType == relative_to_3Dobject) {
		Relative.To3D.fOffSet = Depth;
	}
	else{
		m_fDepth = Depth;
	}
}

void ParaEngine::CGUIPosition::Update3DDepth(float Depth)
{
	m_fDepth = Depth;
}

int ParaEngine::CGUIPosition::GetWidth() const
{
	if (IsNormalWidthHeight())
		return rect.right - rect.left;
	else
		return rect.right;
}

int ParaEngine::CGUIPosition::GetHeight() const
{
	if (IsNormalWidthHeight())
		return rect.bottom - rect.top;
	else
		return rect.bottom;
}

float ParaEngine::CGUIPosition::GetDepth() const
{
	if (m_nPositionType == relative_to_3Dobject) {
		return Relative.To3D.fOffSet + m_fDepth;
	}
	else
		return m_fDepth;
}

void ParaEngine::CGUIPosition::SetLeft(int X)
{
	rect.left = X; 
}

void ParaEngine::CGUIPosition::SetTop(int Y)
{
	rect.top = Y;
}

void ParaEngine::CGUIPosition::SetXY(int X, int Y)
{
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	rect.left = X; rect.top = Y;
	rect.bottom = rect.top + height;
	rect.right = rect.left + width;
}

void ParaEngine::CGUIPosition::SetSize(int width, int height)
{
	SetWidth(width);
	SetHeight(height);
}

int CGUIPosition::SetAlignment(const char* alignment)
{
	/* destination position of the object in the screen coordinates */
	m_nPositionType = CGUIPosition::relative_to_parent;
	Relative.To2D.alignType = CGUIPosition::upper_left;
	if (alignment[1] != 'm')
	{
		if (alignment[1] == 'c')
		{
			if (::strcmp(alignment, "_ct") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::center;
			}
			else if (::strcmp(alignment, "_ctt") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::center_top;
			}
			else if (::strcmp(alignment, "_ctb") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::center_bottom;
			}
			else if (::strcmp(alignment, "_ctl") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::center_left;
			}
			else if (::strcmp(alignment, "_ctr") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::center_right;
			}
		}
		else
		{
			if (::strcmp(alignment, "_lt") == 0)
			{
			}
			else if (::strcmp(alignment, "_lb") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::bottom_left;
			}
			else if (::strcmp(alignment, "_rb") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::bottom_right;
			}
			else if (::strcmp(alignment, "_rt") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::upper_right;
			}
			else if (::strcmp(alignment, "_fi") == 0)
			{
				Relative.To2D.alignType = CGUIPosition::fill;
				return 1;
			}
		}
	}
	else
	{
		//for middle alignments, right is the position from the right, bottom is the position from the bottom
		if (::strcmp(alignment, "_mt") == 0)
		{
			Relative.To2D.alignType = CGUIPosition::middle_top;
		}
		else if (::strcmp(alignment, "_ml") == 0)
		{
			Relative.To2D.alignType = CGUIPosition::middle_left;
		}
		else if (::strcmp(alignment, "_mr") == 0)
		{
			Relative.To2D.alignType = CGUIPosition::middle_right;
		}
		else if (::strcmp(alignment, "_mb") == 0)
		{
			Relative.To2D.alignType = CGUIPosition::middle_bottom;
		}
		return 1;
	}
	return 0;
}

void ParaEngine::CGUIPosition::Reposition(const char* alignment, int left, int top, int width, int height)
{
	rect.left = left;
	rect.top = top;
	rect.right = left + width;
	rect.bottom = top + height;
	m_fDepth = 0;
	memset(&Relative, 0, sizeof(_relativity));

	/* destination position of the object in the screen coordinates */
	m_nPositionType = CGUIPosition::relative_to_parent;
	Relative.To2D.alignType = CGUIPosition::upper_left;
	if (SetAlignment(alignment) == 1) // !IsNormalWidthHeight()
	{
		// for middle or fill alignments, right is the position from the right, bottom is the position from the bottom
		rect.right = width;
		rect.bottom = height;
	}
}

void ParaEngine::CGUIPosition::CalculateAbsPosition(RECT* pOut, const RECT* pParentRect) const 
{
	const RECT& parentPos = *pParentRect;
	const RECT& InPos = rect;

	if(Relative.To2D.alignType <=CGUIPosition::center)
	{
		int nWidth = InPos.right - InPos.left;
		int nHeight = InPos.bottom - InPos.top;

		switch(Relative.To2D.alignType)
		{
		case CGUIPosition::upper_left: 
			{
				pOut->left=InPos.left+parentPos.left;
				pOut->top=InPos.top+parentPos.top;
				break;
			}
		case CGUIPosition::center: 
			{
				pOut->left = (parentPos.left+parentPos.right)/2 + InPos.left;
				pOut->top = (parentPos.top+parentPos.bottom)/2 + InPos.top;
				break;
			}
		case CGUIPosition::center_top: 
			{
				pOut->left = (parentPos.left+parentPos.right)/2 - nWidth/2 + InPos.left;
				pOut->top = InPos.top + parentPos.top;
				break;
			}
		case CGUIPosition::center_bottom: 
			{
				pOut->left = (parentPos.left+parentPos.right)/2 - nWidth/2 + InPos.left;
				pOut->top = parentPos.bottom-nHeight + InPos.top;
				break;
			}
		case CGUIPosition::center_left: 
			{
				pOut->left =  InPos.left + parentPos.left;
				pOut->top = (parentPos.top+parentPos.bottom)/2 - nHeight/2+ InPos.top;
				break;
			}
		case CGUIPosition::center_right: 
			{
				pOut->left = parentPos.right - nWidth + InPos.left;
				pOut->top = (parentPos.top+parentPos.bottom)/2 - nHeight/2+ InPos.top;
				break;
			}
		case CGUIPosition::bottom_left: 
			{
				pOut->top = parentPos.bottom+InPos.top;
				pOut->left = InPos.left + parentPos.left;
				break;
			}
		case CGUIPosition::bottom_right: 
			{
				pOut->left = parentPos.right + InPos.left;
				pOut->top = parentPos.bottom + InPos.top;
				break;
			}
		case CGUIPosition::upper_right: 
			{
				pOut->left = parentPos.right + InPos.left;
				pOut->top = InPos.top + parentPos.top;
				break;
			}
		default:
			break;
		}
		pOut->right = pOut->left+nWidth;
		pOut->bottom = pOut->top+nHeight;
	}
	else
	{
		pOut->left=InPos.left+parentPos.left;
		pOut->top=InPos.top+parentPos.top;
		pOut->right = parentPos.right-InPos.right;
		pOut->bottom = parentPos.bottom-InPos.bottom;

		switch(Relative.To2D.alignType)
		{
		case CGUIPosition::middle_top: 
			{
				pOut->bottom =pOut->top+InPos.bottom;
				break;
			}
		case CGUIPosition::middle_bottom: 
			{
				pOut->bottom = parentPos.bottom-InPos.top;
				pOut->top=pOut->bottom-InPos.bottom;
				break;
			}
		case CGUIPosition::middle_left: 
			{
				pOut->right=pOut->left+InPos.right;
				break;
			}
		case CGUIPosition::middle_right: 
			{
				pOut->right = parentPos.right - InPos.left;
				pOut->left=pOut->right-InPos.right;
				break;
			}
		case CGUIPosition::fill: 
			{
				break;
			}
		default:
			break;
		}
	}
	if (pOut->right<pOut->left){
		pOut->left+=pOut->right;
		pOut->right=pOut->left-pOut->right;
		pOut->left-=pOut->right;
	}
	if (pOut->bottom<pOut->top){
		pOut->top+=pOut->bottom;
		pOut->bottom=pOut->top-pOut->bottom;
		pOut->top-=pOut->bottom;
	}
}

void ParaEngine::CGUIPosition::SetRelatveTo3DObject(const char* s3DObjectName, float fOffset /*= 0.f*/)
{
	SetPositionType(CGUIPosition::relative_to_3Dobject);
	Relative.To3D.fOffSet = fOffset;
	m_s3DObjectName = s3DObjectName;
	m_n3DObjectID = 0;
}

bool ParaEngine::CGUIPosition::Is3DObjectSpecified() const
{
	return IsRelativeTo3DObject() && (!m_s3DObjectName.empty() || (m_n3DObjectID != 0));
}

CBaseObject* ParaEngine::CGUIPosition::GetRelative3DObject(IScene * pScene) const
{
	if (IsRelativeTo3DObject())
	{
		if (!m_s3DObjectName.empty() && pScene)
		{
			return pScene->GetGlobalObject(m_s3DObjectName);
		}
		else if (m_n3DObjectID != 0)
		{
			return CBaseObject::GetObjectByID(m_n3DObjectID);
		}
	}
	return NULL;
}

void ParaEngine::CGUIPosition::SetRelatveTo3DObject(CBaseObject* p3DObject, float fOffset /*= 0.f*/)
{
	SetPositionType(CGUIPosition::relative_to_3Dobject);
	Relative.To3D.fOffSet = fOffset;
	m_s3DObjectName.clear();
	m_n3DObjectID = (p3DObject) ? p3DObject->GetID() : 0; 
}

int ParaEngine::CGUIPosition::GetLeft() const
{
	return rect.left;
}

int ParaEngine::CGUIPosition::GetTop() const
{
	return rect.top;
}

bool ParaEngine::CGUIPosition::IsNormalWidthHeight() const
{
	return ((m_nPositionType != relative_to_3Dobject) && Relative.To2D.alignType <= CGUIPosition::center && Relative.To2D.alignType != CGUIPosition::fill);
}

void CGUIPosition::GetAbsPosition(RECT* pOut, const RECT* pParentRect, const CGUIPosition* pIn)
{
	if (pIn)
		pIn->CalculateAbsPosition(pOut, pParentRect);
}

void CGUIPosition::SetWidth( int width )
{
	if (IsNormalWidthHeight())
	{
		rect.right=rect.left+width;
	}
	else
	{
		rect.right=width;
	}
}

void CGUIPosition::SetHeight( int height )
{
	if (IsNormalWidthHeight())
	{
		rect.bottom=rect.top+height;
	}else
	{
		rect.bottom=height;
	}
}

