//----------------------------------------------------------------------
// Class:	CGDIGraphicsPath
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: 2005.12.20
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "../GUIResource.h"
#include "GDIGraphicsPath.h"
#include "GDIMisc.h"
#include "memdebug.h"

using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// GDIPathElement
//////////////////////////////////////////////////////////////////////////
//GDIPathElement::~GDIPathElement()
//{
//	SAFE_DELETE(pen);
//}
//void GDIPathElement::SetPen(const CGDIPen *pPen)
//{
//	SAFE_DELETE(pen);
//	if (pPen) {
//		pen=pPen->Clone();
//	}
//}

//////////////////////////////////////////////////////////////////////////
// CGDIGraphicsPath
//////////////////////////////////////////////////////////////////////////
CGDIGraphicsPath::CGDIGraphicsPath():m_bNeedUpdate(false)
{
	m_pVB=NULL;
}
CGDIGraphicsPath::CGDIGraphicsPath(const Vector3* points,const BYTE* types,int count):m_bNeedUpdate(true)
{
	m_pVB=NULL;
	int a=0;
	GDIPathElement tempelement;
	for (;a<count;a++)
	{
		tempelement.point= points[a];
		tempelement.pointtype=types[a];
		m_elements.push_back(tempelement);
	}
}
CGDIGraphicsPath *CGDIGraphicsPath::Clone()const 
{
	CGDIGraphicsPath *newpath=new CGDIGraphicsPath();
	newpath->m_elements=m_elements;
	return newpath;
}

void CGDIGraphicsPath::Reset()
{
	m_elements.clear();
	m_bNeedUpdate=true;
}

void CGDIGraphicsPath::Undo(int nStep)
{
	// TODO: make redo operation possible by shifting a pointer instead of removing entirely.
	if(nStep>(int)m_elements.size())
		nStep = (int)m_elements.size();
	
	if(nStep>0)
	{
		for (int i=0;i<nStep;++i)
		{
			m_elements.pop_back();
		}
		m_bNeedUpdate=true;
	}
}

void CGDIGraphicsPath::Redo(int nStep)
{
	// TODO
	// m_bNeedUpdate=true;
}

void CGDIGraphicsPath::AddLine(const Vector3 &point1,const Vector3& point2,const CGDIPen &pen)
{
	GDIPathElement tempelement;
	tempelement.point=point1;
	tempelement.pointtype=PathPointTypeStart;
	tempelement.pen=pen;
	m_elements.push_back(tempelement);
	tempelement.point=point2;
	tempelement.pointtype=PathPointTypeLine;
	tempelement.pen=CGDIPen::GetDefaultPen();
	m_elements.push_back(tempelement);
	m_bNeedUpdate=true;
}

void CGDIGraphicsPath::AddLines(const Vector3 *points,int nNumPoints,const CGDIPen &pen)
{
	GDIPathElement tempelement;
	for (int a=0;a<nNumPoints;a++,points++) {
		if (a==0) {
			tempelement.pointtype=PathPointTypeStart;
			tempelement.pen=pen;
		}else{
			tempelement.pointtype=PathPointTypeLine;
			tempelement.pen=CGDIPen::GetDefaultPen();
		}
		tempelement.point=*points;
		m_elements.push_back(tempelement);
		m_bNeedUpdate=true;		
	}
}

void CGDIGraphicsPath::AddPoint(const Vector3 &point,const CGDIPen &pen)
{
	GDIPathElement tempelement;
	tempelement.point=point;
	tempelement.pointtype=PathPointTypeStart;
	tempelement.pen=pen;
	m_elements.push_back(tempelement);
	m_bNeedUpdate=true;
}

void CGDIGraphicsPath::AddRectangle(const Vector3 *points,const CGDIPen &pen)
{
	//we assume there are 4 points in the input;
	GDIPathElement tempelement;
	tempelement.pointtype=PathPointTypeRectangle;
	tempelement.pen=pen;
	for (int a=0;a<4;a++) {
		tempelement.point=points[a];
		m_elements.push_back(tempelement);
	}
	m_bNeedUpdate=true;
}
void CGDIGraphicsPath::AddClear(const Color &color)
{
	GDIPathElement tempelement;
	tempelement.pointtype=PathPointTypeOperationClear;
	tempelement.color = color;
	m_elements.push_back(tempelement);
	m_bNeedUpdate=true;
}

GDIPathElement &CGDIGraphicsPath::operator [](int index)
{
	return m_elements[index];
}

void CGDIGraphicsPath::Optimize()
{
	if (!m_bNeedUpdate) {
		return;
	}
	vector<GDIPathElement>::iterator iter;
	GDIPathElement *pElement;
	//optimize for the clear operation
	for (iter=m_elements.begin();iter!=m_elements.end();iter++) {
		pElement=&(*iter);
		if (pElement->pointtype==PathPointTypeOperationClear&&iter!=m_elements.begin()) {
	//		iter--;
			iter=m_elements.erase(m_elements.begin(),iter);
		}
	}
}

//this method can render the entire texture or part of the texture, the rectangle of the Brush will be used.
//the texture will be warped.
void CGDIGraphicsPath::CalculateRectVertex(const Vector3 *points,int nNumPoints, vector<DXUT_SCREEN_VERTEX> &vertices, const CGDIPen &pPen)
{
	//this is an internal function, so we do not handle the incorrect parameters;We assume the input is a rectangle with the point order:
	//1 2
	//4 3
	assert(points!=NULL&&nNumPoints==4);
	DXUT_SCREEN_VERTEX tempvex;
	double fWidth=points[1].x-points[0].x,fHeight=points[3].y-points[0].y;
	float  tv1=0,tu1=0,tv2=0,tu2=0;
	const CGDIBrush *pBrush=pPen.GetBrush();
	TextureEntity *pTexture=pBrush->GetTexture();
	RECT rcScreen,rcIntersect;
	rcScreen.left=int(points[0].x+0.5f);
	rcScreen.right=int(points[1].x+0.5f);
	rcScreen.top=int(points[0].y+0.5f);
	rcScreen.bottom=int(points[2].y+0.5f);
	if (m_region.IsInfinite()||m_region.IsEmpty()) {
		rcIntersect=rcScreen;
	}else{
		rcIntersect=m_region;
		IntersectRect(&rcIntersect,&rcScreen,&rcIntersect);
	}

	RECT rcTexture;
	pBrush->GetRect(&rcTexture);
	if (pTexture) {
		switch(pPen.GetScaleType()) {
		case PenScaleFitRect:
			tu1=float((rcTexture.left+rcIntersect.left-rcScreen.left)/(double)pTexture->GetTextureInfo()->m_width);
			tv1=float((rcTexture.top+rcIntersect.top-rcScreen.top)/(double)pTexture->GetTextureInfo()->m_height);
			tu2=float((rcTexture.right+rcIntersect.right-rcScreen.right)/(double)pTexture->GetTextureInfo()->m_width);
			tv2=float((rcTexture.bottom+rcIntersect.bottom-rcScreen.bottom)/(double)pTexture->GetTextureInfo()->m_height);
			break;
		default:
			tu1=float((rcIntersect.left-rcScreen.left)/(double)pTexture->GetTextureInfo()->m_width);
			tv1=float((rcIntersect.top-rcScreen.top)/(double)pTexture->GetTextureInfo()->m_height);
			tu2=1.0f+float((rcIntersect.right-rcScreen.right)/(double)pTexture->GetTextureInfo()->m_width);
			tv2=1.0f+float((rcIntersect.bottom-rcScreen.bottom)/(double)pTexture->GetTextureInfo()->m_height);
		}
	}
	tempvex.color=pPen.GetColor();
	tempvex.h=1.0f;
	tempvex.z=0;
	
	tempvex.x=float(rcIntersect.left)-0.5f;
	tempvex.y=float(rcIntersect.top)-0.5f;
	tempvex.tu=tu1;
	tempvex.tv=tv1;
	vertices.push_back(tempvex);

	tempvex.x=float(rcIntersect.right)-0.5f;
	tempvex.y=float(rcIntersect.top)-0.5f;
	tempvex.tu=tu2;
	tempvex.tv=tv1;
	vertices.push_back(tempvex);

	tempvex.x=float(rcIntersect.right)-0.5f;
	tempvex.y=float(rcIntersect.bottom)-0.5f;
	tempvex.tu=tu2;
	tempvex.tv=tv2;
	vertices.push_back(tempvex);

	tempvex.x=float(rcIntersect.left)-0.5f;
	tempvex.y=float(rcIntersect.bottom)-0.5f;
	tempvex.tu=tu1;
	tempvex.tv=tv2;
	vertices.push_back(tempvex);
}
//this method will render the entire texture, the rectangle of the Brush will be ignored.
//the texture will be warped
void CGDIGraphicsPath::CalculateLineVertex(const Vector3 *points,int nNumPoints, vector<DXUT_SCREEN_VERTEX> &vertices, const CGDIPen &pPen)
{
	//this is an internal function, so we do not handle the incorrect parameters;
	assert(points!=NULL&&nNumPoints>0);
	Vector3 *normalVec=new Vector3[nNumPoints-1];
	DXUT_SCREEN_VERTEX tempvex;
	double fSin,fRatioX=0,fRatioY=0,fDistance;
	//TODO: Add Clipping support
	const CGDIBrush *pBrush=pPen.GetBrush();
	TextureEntity *pTexture=pBrush->GetTexture();
	if (pTexture) {
		fRatioX=1.0f/pTexture->GetTextureInfo()->m_width;
		fRatioY=1.0f/pTexture->GetTextureInfo()->m_height;
	}
	fDistance=pPen.GetWidth()/2;
	int a;
	Vector3 tempPoint;
	for (a=0;a<nNumPoints-1;a++) {
		tempPoint = points[a+1] - points[a];
		normalVec[a] = tempPoint.normalisedCopy();
	}
	tempvex.color=pPen.GetColor();
	tempvex.h=1.0f;
	tempvex.z=0;
	//calculate start cap
	if (nNumPoints==1) {
		switch(pPen.GetStartCap()) {
		case LineCapSquare:
			tempvex.x=float(points[0].x-fDistance);
			tempvex.y=float(points[0].y-fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempvex.x=float(points[0].x+fDistance);
			tempvex.y=float(points[0].y-fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempvex.x=float(points[0].x-fDistance);
			tempvex.y=float(points[0].y+fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempvex.x=float(points[0].x+fDistance);
			tempvex.y=float(points[0].y+fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);
			break;
		//TODO: Add other cap support
		}
	}else{
		Vector4 tempvector;
		switch(pPen.GetStartCap()) {
		case LineCapSquare:
			tempvector = normalVec[0] * Matrix4(-1.0f,1.0f,0,0,	-1.0f,-1,0,0,	0,0,1,0,	0,0,0,1);
			tempvex.x=float(points[0].x+tempvector.x*fDistance);
			tempvex.y=float(points[0].y+tempvector.y*fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempvector = normalVec[0] * Matrix4(-1.0f,-1.0f,0,0,	1.0f,-1,0,0,	0,0,1,0,	0,0,0,1);
			tempvex.x=float(points[0].x+tempvector.x*fDistance);
			tempvex.y=float(points[0].y+tempvector.y*fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);
			break;

		//TODO: Add other cap support
		}	
	}

	//calculate middle vertices
	if (nNumPoints>2) {
		for (a=0;a<nNumPoints-2;a++) {
			tempPoint = normalVec[a + 1].crossProduct(normalVec[a]);
			fSin=tempPoint.z;
			tempPoint = normalVec[a] - normalVec[a+1];
			tempPoint*=float(fDistance/fSin);
			tempPoint = points[a+1]+ tempPoint;
			tempvex.x=tempPoint.x;
			tempvex.y=tempPoint.y;
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempPoint = normalVec[a+1] - normalVec[a];
			tempPoint*=float(fDistance/fSin);
			tempPoint = points[a+1]+tempPoint;
			tempvex.x=tempPoint.x;
			tempvex.y=tempPoint.y;
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

		}
	}

	//calculate end cap
	if (nNumPoints>1) {
		Vector4 tempvector;
		switch(pPen.GetEndCap()) {
		case LineCapSquare:
			tempvector = normalVec[nNumPoints-2] * Matrix4(1.0f,1.0f,0,0,	-1.0f,1,0,0,	0,0,1,0,	0,0,0,1);
			tempvex.x=float(points[nNumPoints-1].x+tempvector.x*fDistance);
			tempvex.y=float(points[nNumPoints-1].y+tempvector.y*fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);

			tempvector = normalVec[nNumPoints-2]*Matrix4(1.0f,-1.0f,0,0,	1.0f,1,0,0,	0,0,1,0,	0,0,0,1);
			tempvex.x=float(points[nNumPoints-1].x+tempvector.x*fDistance);
			tempvex.y=float(points[nNumPoints-1].y+tempvector.y*fDistance);
			tempvex.tu=float(tempvex.x*fRatioX);
			tempvex.tv=float(tempvex.y*fRatioY);
			vertices.push_back(tempvex);
			break;
			//TODO: Add other cap support
		}	
	}
	SAFE_DELETE(normalVec);
}

void CGDIGraphicsPath::UpdateVertices(int nOption)
{
	if (!m_bNeedUpdate) {
		return;
	}
	
	vector<DXUT_SCREEN_VERTEX> vertexs;
	vector<GDIPathElement>::const_iterator iter=PathBegin();
	const GDIPathElement *tempelement,*startelement;//tempelement is the current element, startelement is the start point of the current figure
	const CGDIPen *pPen;//points to the current drawing pen
	vector<Vector3> pointbuffer;
	enum DrawPathState{NONE,START,LINE};
	int nState=NONE;
	Optimize();
	//we currently just clear all old figures, maybe we should do more intelligent clear or update operation
	ClearFigures();
	for (;iter!=PathEnd();iter++) {
		tempelement=&(*iter);
		switch(tempelement->pointtype) {
		case PathPointTypeStart:
			switch(nState) {
			case START:
			case LINE:
				//calculate point or line from the point buffer
				CalculateLineVertex(&pointbuffer.front(),(int)pointbuffer.size(),vertexs,*pPen);
				break;
			}
			//updates the vertices to a figure
			if (nState!=NONE) {
				UpdateFigures(startelement,D3DPT_TRIANGLELIST,vertexs,&Matrix4::IDENTITY,VerticesMergeAppend);
				vertexs.clear();
			}
			startelement=tempelement;

			pointbuffer.clear();
			pointbuffer.push_back(tempelement->point);
			pPen=&tempelement->pen;
			nState=START;
			
			break;
		case PathPointTypeLine:
			//skip the wrong path sequence
			if (nState!=START&&nState!=LINE) {
				break;
			}
			pointbuffer.push_back(tempelement->point);
			nState=LINE;
			break;
		case PathPointTypeRectangle:
			switch(nState) {
			case START:
			case LINE:
				//calculate point or line from the point buffer
				CalculateLineVertex(&pointbuffer.front(),(int)pointbuffer.size(),vertexs,*pPen);
				break;
			}
			//updates the vertices to a figure
			if (nState!=NONE) {
				UpdateFigures(startelement, D3DPT_TRIANGLELIST, vertexs, &Matrix4::IDENTITY, VerticesMergeAppend);
				vertexs.clear();
			}
			pointbuffer.clear();
			pointbuffer.push_back(tempelement->point);
			iter++;
			pointbuffer.push_back((*iter).point);
			iter++;
			pointbuffer.push_back((*iter).point);
			iter++;
			pointbuffer.push_back((*iter).point);
			pPen=&tempelement->pen;
			CalculateRectVertex(&pointbuffer.front(),(int)pointbuffer.size(),vertexs,*pPen);

			UpdateFigures(startelement, D3DPT_TRIANGLELIST, vertexs, &Matrix4::IDENTITY, VerticesMergeAppend);
			vertexs.clear();
			nState=NONE;
			break;
		case PathPointTypeOperationClear:
			AddOperation(tempelement,PathPointTypeOperationClear);
			vertexs.clear();
			nState=NONE;
			break;
		}

	}
	//finish up the figure
	switch(nState) {
	case START:
	case LINE:
		//calculate point or line from the point buffer
		CalculateLineVertex(&pointbuffer.front(),(int)pointbuffer.size(),vertexs,*pPen);
		break;
	}
	//updates the vertices to a figure
	if (nState!=NONE) {
		UpdateFigures(startelement, D3DPT_TRIANGLELIST, vertexs, &Matrix4::IDENTITY, VerticesMergeAppend);
	}
	m_bNeedUpdate=false;
}

void CGDIGraphicsPath::UpdateFigures(const GDIPathElement *pElement,D3DPRIMITIVETYPE PrimitiveType,const vector<DXUT_SCREEN_VERTEX> &vertices,const Matrix4 *matTransform,int nOption/* =0 */,int index/* =0 */)
{
	GDIFigure *pFigure;
	pFigure=new GDIFigure;

	pFigure->PrimitiveType=PrimitiveType;
	pFigure->operationtype=-1;//not an operation
	pFigure->pElement=pElement;
	pFigure->matTransform=*matTransform;
	bool bFlip=false;//for drawing triangle list
	const DXUT_SCREEN_VERTEX *past1,*past2=NULL;//for drawing triangle list
	switch(pFigure->PrimitiveType) {
	case D3DPT_TRIANGLESTRIP:
		pFigure->vertices.insert(pFigure->vertices.begin(),vertices.begin(),vertices.end());
		break;
	case D3DPT_TRIANGLELIST:
		//this process maintains the clockwise vertex sequence
		for (DWORD a=0;a<vertices.size();a++) {
			if (a>2) {
				if (bFlip) {
					pFigure->vertices.push_back(*past2);
					pFigure->vertices.push_back(*past1);
					bFlip=false;
				}else{
					pFigure->vertices.push_back(*past1);
					pFigure->vertices.push_back(*past2);
					bFlip=true;
				}
			}
			pFigure->vertices.push_back(vertices[a]);
			past1=past2;
			past2=&(vertices[a]);
		}
		break;
	}
	switch(nOption) {
	case VerticesMergeReplace:
		m_figures.clear();
		m_figures.push_back(*pFigure);
		break;
	case VerticesMergeAppend:
		m_figures.push_back(*pFigure);
		break;
	}
	delete pFigure;

}

void CGDIGraphicsPath::AddOperation(const GDIPathElement *pElement,byte operationtype)
{
	if (!m_bNeedUpdate) {
		return;
	}
	GDIFigure *pFigure;
	pFigure=new GDIFigure;
	pFigure->operationtype=operationtype;
	pFigure->pElement=pElement;
	m_figures.push_back(*pFigure);
	delete pFigure;

}

void CGDIGraphicsPath::SetClip(const CGDIRegion &region)
{
	m_region=region;
}