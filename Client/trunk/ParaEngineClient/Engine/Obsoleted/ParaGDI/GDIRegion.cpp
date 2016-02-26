//----------------------------------------------------------------------
// Class:	CGDIRegion
// Authors:	Liu Weili
// Date:	2006.1.10
// Revised: 2006.1.10
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GDIRegion.h"
#include "memdebug.h"
#include "GDIMisc.h"

using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// GDISimpleEdge
//////////////////////////////////////////////////////////////////////////
GDISimpleEdge::GDISimpleEdge(const Vector3 &p1,const Vector3 &p2)
{
	point1=p1;
	point2=p2;
	if (point2.x!=point1.x) {
		slope=(point2.y-point1.y)/(point2.x-point1.x);
		vertical=0;
	}else{
		slope=3.402823e38f;
		if (point1.y>point2.y) {
			vertical=-1;
		}else
			vertical=1;
	}
	intersect=point1.y-slope*point1.x;
}
GDISimpleEdge::GDISimpleEdge()
{
}
//////////////////////////////////////////////////////////////////////////
// CGDIRegion
//////////////////////////////////////////////////////////////////////////
CGDIRegion::CGDIRegion()
{
	m_shape=RegionShapeNone;
}
CGDIRegion::CGDIRegion(const RECT &rect)
{
	Vector3 temppoint;
	temppoint.z=0;
	temppoint.x=(float)rect.left-0.5f;temppoint.y=(float)rect.top-0.5f;
	m_points.push_back(temppoint);
	temppoint.x=(float)rect.right-0.5f;temppoint.y=(float)rect.top-0.5f;
	m_points.push_back(temppoint);
	temppoint.x=(float)rect.right-0.5f;temppoint.y=(float)rect.bottom-0.5f;
	m_points.push_back(temppoint);
	temppoint.x=(float)rect.left-0.5f;temppoint.y=(float)rect.bottom-0.5f;
	m_points.push_back(temppoint);
	temppoint.x=(float)rect.left-0.5f;temppoint.y=(float)rect.top-0.5f;
	m_points.push_back(temppoint);
	GenerateEdge();
	m_shape=RegionShapeRectangle;
}
//the sequence of the points must be the same, it will return false even if the points in two region are the same but in difference sequence
bool CGDIRegion::operator ==(const CGDIRegion &region)
{
	if (region.m_points.size()!=m_points.size()) {
		return false;
	}
	for (DWORD a=0;a<m_points.size();a++) {
		if (m_points[a]!=region.m_points[a]) {
			return false;
		}
	}
	return true;
}

//the sequence of the points must be the same, it will return false even if the points in two region are the same but in difference sequence
bool CGDIRegion::operator ==(const RECT &rect)
{
	if (m_shape!=RegionShapeRectangle) {
		return false;
	}
	if (m_points[0].x!=rect.left||m_points[0].y!=rect.top||m_points[1].x!=rect.right||m_points[1].y!=rect.top||
		m_points[2].x!=rect.right||m_points[2].y!=rect.bottom||m_points[3].x!=rect.left||m_points[3].y!=rect.bottom) {
		return false;
	}
	return true;
}

CGDIRegion::operator RECT()
{
	RECT rect;
	if (m_shape==RegionShapeRectangle) {
		SetRect(&rect,(int)(m_points[0].x),(int)(m_points[0].y),(int)(m_points[1].x),(int)(m_points[2].y));
	}else 
		SetRect(&rect,0,0,0,0);
	return rect;
}
void CGDIRegion::Translate(float dx,float dy,float dz/*=0*/)
{
	if (dx==0&&dy==0&&dz==0) {
		return;
	}
	Vector3 tempvector(dx,dy,dz);
	Translate(tempvector);
}

void CGDIRegion::Translate(const Vector3 &v)
{
	for (DWORD a=0;a<m_points.size();a++) {
		m_points[a]+=v;
	}
}

bool CGDIRegion::IsEmpty()const
{
	float maxx,maxy,minx,miny;
	if (m_edges.empty()) {
		return false;
	}
	maxx=minx=m_points[0].x;
	maxy=miny=m_points[0].y;
	for (DWORD a=1;a<m_points.size();a++) {
		if (m_points[a].x>maxx) {
			maxx=m_points[a].x;
		}
		if (m_points[a].x<minx) {
			minx=m_points[a].x;
		}
		if (m_points[a].y>maxy) {
			maxy=m_points[a].y;
		}
		if (m_points[a].y<miny) {
			miny=m_points[a].y;
		}
	}
	if (maxy==miny||maxx==minx) {
		return true;
	}else
		return false;
}
void CGDIRegion::MakeEmpty()
{
	m_points.clear();
	m_points.push_back(Vector3(0,0,0));
	m_shape=RegionShapeNone;
}
void CGDIRegion::MakeInifinite()
{
	m_points.clear();
	m_shape=RegionShapeNone;
}
CGDIRegion *CGDIRegion::Clone()
{
	CGDIRegion *newregion=new CGDIRegion();
	newregion->m_points=m_points;
	return newregion;
}

void CGDIRegion::GenerateEdge()
{
	m_edges.clear();
	//close a region first
	if (m_points.size()>0) {
		for (DWORD a=0;a<m_points.size()-1;a++) {
			m_edges.push_back(GDISimpleEdge(m_points[a],m_points[a+1]));
		}
	}
}

bool CGDIRegion::LineIntersect(const Vector3 &point1,const Vector3 &point2, Vector3 &outpoint)const
{
	GDISimpleEdge tempedge;
	tempedge.point1=point1;
	tempedge.point2=point2;
	if (tempedge.point2.x!=tempedge.point1.x) {
		tempedge.slope=(tempedge.point2.y-tempedge.point1.y)/(tempedge.point2.x-tempedge.point1.x);
	}else{
		tempedge.slope=3.402823e38f;
	}
	float x,x1=min(point1.x,point2.x),x2=max(point1.x,point2.x);
	bool bIsP1in=InsideTest(point1);
	float dx=point2.x-point1.x;
	bool bIntersect=false;
	for (DWORD a=0;a<m_edges.size();a++) {
		if (CalculateClipX(tempedge,m_edges[a],x)) {
			if (dx>0) {
				if (x-point1.x<0) {
					if (x>x1) {
					}
				}
			}else if (dx<0) {
			}else{
			}
		}
	}

	return true;
}

bool CGDIRegion::LineIntersect(const GDISimpleEdge &edge,vector<Vector3> &outpoint,vector<int> &outedge)const
{
	Vector3 temppoint(0,0,0);
	bool bIsIntersect=false;
	//O(N*(O(EdgeInterscet)))
	for (DWORD a=0;a<m_edges.size();a++) {
		if (EdgeIntersect(edge,m_edges[a],temppoint)) {
			bIsIntersect= true;
			outpoint.push_back(temppoint);
			outedge.push_back(a);
		}
	}
	return bIsIntersect;
}

//result rectangle point order
//1,5  2
//4    3
int CGDIRegion::RectClip(const CGDIRegion &region,CGDIRegion &outregion)
{
	if (region.m_shape!=RegionShapeRectangle||m_shape!=RegionShapeRectangle) {
		return IncorrectShape;
	}
	float fmin1,fmax1,fmin2,fmax2;
	fmin1=fmin2=1e38f;fmax1=fmax2=-1e38f;
	for (DWORD a=0;a<4;a++) {
		if (fmax1<m_points[a].x) {
			fmax1=m_points[a].x;
		}
		if (fmax2<m_points[a].y) {
			fmax2=m_points[a].y;
		}
		if (fmin1>m_points[a].x) {
			fmin1=m_points[a].x;
		}
		if (fmin2>m_points[a].y) {
			fmin2=m_points[a].y;
		}
	}

	float ffmin1,ffmax1,ffmin2,ffmax2;
	ffmin1=ffmin2=1e38f;ffmax1=ffmax2=-1e38f;
	for (DWORD a=0;a<4;a++) {
		if (ffmax1<region.m_points[a].x) {
			ffmax1=region.m_points[a].x;
		}
		if (ffmax2<region.m_points[a].y) {
			ffmax2=region.m_points[a].y;
		}
		if (ffmin1>region.m_points[a].x) {
			ffmin1=region.m_points[a].x;
		}
		if (ffmin2>region.m_points[a].y) {
			ffmin2=region.m_points[a].y;
		}
	}
	outregion.m_points.clear();
	Vector3 temppoint;
	temppoint.z=0;
	temppoint.x=fmin1>ffmin1?fmin1:ffmin1;
	temppoint.y=fmin2>ffmin2?fmin2:ffmin2;
	outregion.m_points.push_back(temppoint);
	temppoint.x=fmax1>ffmax1?ffmax1:fmax1;
	temppoint.y=fmin2>ffmin2?fmin2:ffmin2;
	outregion.m_points.push_back(temppoint);
	temppoint.x=fmax1>ffmax1?ffmax1:fmax1;
	temppoint.y=fmax2>ffmax2?ffmax2:fmax2;
	outregion.m_points.push_back(temppoint);
	temppoint.x=fmin1>ffmin1?fmin1:ffmin1;
	temppoint.y=fmax2>ffmax2?ffmax2:fmax2;
	outregion.m_points.push_back(temppoint);
	temppoint.x=fmin1>ffmin1?fmin1:ffmin1;
	temppoint.y=fmin2>ffmin2?fmin2:ffmin2;
	outregion.m_points.push_back(temppoint);

	outregion.GenerateEdge();
	outregion.m_shape=RegionShapeRectangle;
	return S_OK;
}

//we assume the points are at the same order as the edges, so point0 and point1 is the start and end point of edge0
//not finished
int CGDIRegion::PolygonClip(const CGDIRegion &region, vector<CGDIRegion> &outregion)
{
	if (region.IsEmpty()||IsInfinite()) {
		outregion.push_back(region);
		return S_OK;
	}
	if (IsEmpty()||region.IsInfinite()) {
		outregion.push_back(*this);
		return S_OK;
	}
	CGDIRegion *newregion=NULL;
	if (region.m_shape==RegionShapeRectangle&&m_shape==RegionShapeRectangle) {
		newregion=new CGDIRegion();
		if(RectClip(region,*newregion)==0)
		{
			outregion.push_back(*newregion);
		}
		delete newregion;
		return S_OK;
	}
	//do not process regions which has less than 3 edges
	if (region.m_edges.size()<3||m_edges.size()<3) {
		return IncorrectShape;
	}
	//here 1 represents this region, and 2 represents the input region
	list<GDIClipNode>::const_iterator iter1,iter2;
	//preperation 1: clean and copy
//	m_nodes.insert(m_points.begin(),m_points.end());
//	region.m_nodes.insert(region.m_points.begin(),region.m_points.end());

	//preperation 2: calculate intersection
	for (iter2=region.m_nodes.begin();iter2!=region.m_nodes.end();iter2++) {

	}

	return S_OK;
}
//this algorithm is suitable for all cases. But the region itself does not support polygon with holes.
bool CGDIRegion::InsideTest(const Vector3 &point)const
{
	//O(N*(2 mul,3 add 11 comp))
	//draw a extended line from point to positive x infinite
	//points on the edge of the region is consider inside
	int counter=0;
	for (DWORD a=0;a<m_edges.size();a++) {
		const GDISimpleEdge *tempedge=&m_edges[a];
		if ((((point.y>=tempedge->point1.y)&&(point.y<=tempedge->point2.y))||((point.y>=tempedge->point2.y)&&(point.y<=tempedge->point1.y)))) {
			if (tempedge->point1.x==tempedge->point2.x) {
				//decide if the point is on the edge
					if (tempedge->point1.x==point.x) {
						return true;
					}else if (tempedge->point1.x>point.x) {
						counter++;				
					}
			}else{
				if ((point.x<=tempedge->point1.x)&&(point.x<=tempedge->point2.x)) {
					counter++;
				}else if ((point.x<=tempedge->point1.x)||(point.x<=tempedge->point2.x)) {
					float tempy=point.x*tempedge->slope+tempedge->intersect;
					//decide if the point is on the edge
					if  (tempy==point.y){
						return true;
					}else if(tempy<point.y)
						counter++;
				}
			}
		}
	}
	if ((counter%2)==0) {
		return false;
	}else
		return true;
}

bool CGDIRegion::EdgeIntersect(const GDISimpleEdge &edge1,const GDISimpleEdge &edge2,Vector3 &point)
{
	//3 addition, 2 multiplication and at most 15 comparison
	//we think of the two line overlap each other not intersect;
	//we do not think the two line intersect at either of their start point is intersection
	if (edge1.slope==edge2.slope) {
		//parallel edges
		return false;
	}
	if (edge1.point1.x==edge1.point2.x) {
		point.x=edge1.point1.x;
		point.y=edge2.GetY(point.x);
		if ((((edge2.point1.x<point.x)&&(edge2.point2.x>=point.x))||((edge2.point1.x>point.x)&&(edge2.point2.x<=point.x)))
			&&(((edge1.point1.y<point.y)&&(edge1.point2.y>=point.y))||((edge1.point1.y>point.y)&&(edge1.point2.y<=point.y)))){
			return true;
		}
	}else if (edge2.point1.x==edge2.point2.x) {
		point.x=edge2.point1.x;
		point.y=edge1.GetY(point.x);
		if (((edge1.point1.x<point.x)&&(edge1.point2.x>=point.x))||((edge1.point1.x>point.x)&&(edge1.point2.x<=point.x))
			&&(((edge2.point1.y<point.y)&&(edge2.point2.y>=point.y))||((edge2.point1.y>point.y)&&(edge2.point2.y<=point.y)))){
			return true;
		}
	}else{
		point.x=(edge1.intersect-edge2.intersect)/(edge2.slope-edge1.slope);
		point.y=edge1.GetY(point.x);
		if ((((point.x>edge1.point1.x)&&(point.x<=edge1.point2.x))||((point.x<edge1.point1.x)&&(point.x>=edge1.point2.x)))&&
			(((point.x>edge2.point1.x)&&(point.x<=edge2.point2.x))||((point.x<edge2.point1.x)&&(point.x>=edge2.point2.x))))
		{
			return true;
		}
	}
	return false;
}

bool CGDIRegion::CalculateClipX(const GDISimpleEdge &edge1,const GDISimpleEdge &edge2,float &pointx)const
{
	//2 addition 1 division and up to 3 comaprison
	if (edge1.slope==edge2.slope) {
		//parallel edges
		return false;
	}
	if (edge1.point1.x==edge1.point2.x) {
		pointx=edge1.point1.x;
	}else if (edge2.point1.x==edge2.point2.x) {
		pointx=edge2.point1.x;
	}else
		pointx=(edge1.intersect-edge2.intersect)/(edge2.slope-edge1.slope);
	return true;
}

bool CGDIRegion::IsPoint1Closer(const Vector3 &point,const Vector3 &point1, const Vector3 &point2,int relation/* =0 */)
{
	//we assume point1 != point2 
	if (relation==0) {
		//we assume the three points are on the same line
		if (point1.x==point2.x) {
			//if on a vertical line
			return abs(point1.y-point.y)<abs(point2.y-point.y)?true:false;
		}else
			return abs(point1.x-point.x)<abs(point2.x-point.x)?true:false;
	}
	return true;
}

Vector3 CGDIRegion::GetPoint(int index)const
{
	return m_points[index];
}

GDISimpleEdge CGDIRegion::GetEdge(int index)const
{
	return m_edges[index];
}
//old polygonclip
/*int CGDIRegion::PolygonClip(const CGDIRegion &region, vector<CGDIRegion> &outregion)
{
	if (region.IsEmpty()||IsInfinite()) {
		outregion.push_back(region);
		return S_OK;
	}
	if (IsEmpty()||region.IsInfinite()) {
		outregion.push_back(*this);
		return S_OK;
	}
	CGDIRegion *newregion=NULL;
	if (region.m_shape==RegionShapeRectangle&&m_shape==RegionShapeRectangle) {
		newregion=new CGDIRegion();
		if(RectClip(region,*newregion)==0)
		{
			outregion.push_back(*newregion);
		}
		delete newregion;
		return S_OK;
	}
	//do not process regions which has less than 3 edges
	if (region.m_edges.size()<3||m_edges.size()<3) {
		return IncorrectShape;
	}
	const CGDIRegion *tempregion1=this,*tempregion2=&region;
	//start edge and current edge of the tempregion1 and tempregion2
	//	GDISimpleEdge *startedge1,*startedge2,*curedge1,*curedge2;
	//start point of the tempregion1 and tempregion2
	//	Vector3 *startpoint1,*startpoint2;
	//	Vector3 *curpoint1,*curpoint2;
	//the previous point added to the result region
	const Vector3 *prepoint;
	//the saved point where it is switch to mode 2
	Vector3 savepoint(0,0,0);

	//the base of the point buffer, for avoid erasing frequently
	DWORD numpointbuffer=0;
	//a list to record all processed points
	vector<Vector3> pointlist;
	//store intersect points temporary
	vector<Vector3> pointbuffer;
	//store intersect edges temporary
	vector<int>edgebuffer;

	bool bStart=false;
	//curedge1=tempregion1->m_edges[0];
	//curedge2=tempregion2->m_edges[0];
	//preprocessing points and edges of the regions
	//here 1 represents current region and 2 represents input region
	bool *b1Inside2=new bool[m_points.size()]; 
	bool *b2Inside1=new bool[region.m_points.size()]; 
	//decide whether the previous point which is added to the result region is inside 1 or inside 2
	bool bpreInside1;
	//temporary pointers used in mode 2
	bool *pb1Inside2,*pb2Inside1;
	//counters for mode1 and mode2
	DWORD mode1,mode2;
	//temporary variables
	DWORD a,b,c,intersectEdge=0;
	//int inoutedge=-1,outinedge=-1;
	for (a=0;a<m_points.size();a++) {
		b1Inside2[a]=region.InsideTest(m_points[a]);
	}
	for (a=0;a<region.m_points.size();a++) {
		b2Inside1[a]=InsideTest(region.m_points[a]);
	}

	//tells if the mode is switched or has just create a new region 
	bool bSwitched=false,bNewRegion=false;
	bool bIntersect=false;
	if (b2Inside1[0]) {
		//only create a new region if the start point is inside the current region
		newregion=new CGDIRegion();
		bNewRegion=true;
	}
	mode1=0;
	Vector3 temppoint,*oldpoint=NULL;
	prepoint=&region.m_points[0];
	bpreInside1=b2Inside1[0];
	while (mode1<region.m_edges.size()) {
		numpointbuffer=(DWORD)pointbuffer.size();
		SAFE_DELETE(oldpoint);
		if (!bSwitched) {
			if ((bpreInside1&&b2Inside1[mode1+1])) {
				//if the region.m_edges[mode1] is completely inside the current region, then the edge is saved to the output region
				if (bNewRegion) {
					newregion->m_points.push_back(region.m_points[mode1]);
					pointlist.push_back(region.m_points[mode1]);
					bNewRegion=false;
				}
				for (c=0;c<pointlist.size();c++) {
					if (region.m_points[mode1+1]==pointlist[c]) {
						break;
					}
				}
				//if the point is not processed before
				if (c>=pointlist.size()) {
					newregion->m_points.push_back(region.m_points[mode1+1]);
					pointlist.push_back(region.m_points[mode1+1]);
				}
				prepoint=&(region.m_points[mode1+1]);
				bpreInside1=true;
			}else {
				//find intersection point only if the region.m_edges[mode1] DO intersect with the current region
				if (bNewRegion) {
					bIntersect=LineIntersect(region.m_edges[mode1],pointbuffer,edgebuffer);
				}else{
					bIntersect=LineIntersect(GDISimpleEdge(*prepoint,region.m_points[mode1+1]),pointbuffer,edgebuffer);
				}
				if (bIntersect){
					for (b=numpointbuffer;b<pointbuffer.size();b++) {
						//if the intersect point is already recorded, report and do something
						for (c=0;c<pointlist.size();c++) {
							if (pointbuffer[b]==pointlist[c]) {
								break;
							}
						}
						if (c<pointlist.size()) {
							continue;
						}
						//find the closest point to region.m_edges[mode1].point1;
						if (oldpoint==NULL) {
							oldpoint=new Vector3();
							*oldpoint=pointbuffer[b];
							intersectEdge=edgebuffer[b];
						}else{
							if (temppoint!=*oldpoint) {
								bool bTemp;
								bTemp=IsPoint1Closer(*prepoint,pointbuffer[b],*oldpoint);
								if (bTemp) {
									*oldpoint=pointbuffer[b];
									intersectEdge=edgebuffer[b];
								}
							}
						}

					}
					//if the line do intersect with the current region and the intersection has not been processed yet
					if (oldpoint!=NULL) {
						if (bpreInside1&&!b2Inside1[mode1+1]) {
							//for inside-outside line, we switch mode and save the inside part of the line
							if (bNewRegion) {
								newregion->m_points.push_back(region.m_points[mode1]);
								pointlist.push_back(region.m_points[mode1]);
								bNewRegion=false;
							}
							newregion->m_points.push_back(*oldpoint);
							pointlist.push_back(*oldpoint);
							prepoint=oldpoint;
							//point is on the edge so it is true
							bpreInside1=true;
							bSwitched=true;
							savepoint=*oldpoint;

							tempregion2=this;
							tempregion1=&region;
							pb1Inside2=b2Inside1;pb2Inside1=b1Inside2;
							mode2=intersectEdge;
						}else {
							//for outside-inside line or outside-outside line, we finish the old region (if any) and create mode1 new region and save the inside part of the line
							if (newregion) {
								//make sure the new region is not infinite
								if (newregion->m_points.size()==0) {
									newregion->m_points.push_back(Vector3(0,0,0));
								}
								newregion->m_points.push_back(newregion->m_points.front());
								newregion->GenerateEdge();
								outregion.push_back(*newregion);
							}
							SAFE_DELETE(newregion);
							newregion=new CGDIRegion();
							bNewRegion=true;
							newregion->m_points.push_back(*oldpoint);
							newregion->m_points.push_back(region.m_edges[mode1].point2);
							pointlist.push_back(*oldpoint);
							pointlist.push_back(region.m_edges[mode1].point2);
							prepoint=&(region.m_edges[mode1].point2);
							bpreInside1=true;
							bNewRegion=false;
						}//if (bpreInside1&&!b2Inside1[mode1+1])
					}else{
						//no new point added
						prepoint=&(region.m_points[mode1+1]);
						bpreInside1=b2Inside1[mode1+1];
					}//if (oldpoint!=NULL)
				}else{
					//not intersect with anything
					prepoint=&(region.m_points[mode1+1]);
					bpreInside1=b2Inside1[mode1+1];
				}//if (bIntersect)
			}	//	if ((bpreInside1&&b2Inside1[mode1+1]))
			//do not combine this if sentence with the previous if sentence
			if (!bSwitched) {
				mode1++;
			}
		}else{
			//mode 2 exits when it comes to a point which is already processed
			//deal with the mode2
			//there will not be newregion in mode 2
			if (pb2Inside1[mode2+1]) {
				//if the point is already recorded, skip
				for (c=0;c<pointlist.size();c++) {
					if (tempregion2->m_points[mode2+1]==pointlist[c]) {
						break;
					}
				}
				if (c<pointlist.size()) {
					//the line intersects but the intersect points are already processed, so switch back to mode 1
					bSwitched=false;
					prepoint=&savepoint;
					//we consider the resume point is out of the current region, although it is on the edge of the current region
					bpreInside1=b2Inside1[mode1+1];
				}else{
					//if the tempregion1->m_edges[mode2] is completely inside the input region, then the edge is saved to the output region
					newregion->m_points.push_back(tempregion2->m_points[mode2+1]);
					pointlist.push_back(tempregion2->m_points[mode2+1]);
					prepoint=&(tempregion2->m_points[mode2+1]);
				}
			}else{
				//find intersection point only if the tempregion1->m_edges[mode2] DO intersect with the current region
				bIntersect=tempregion1->LineIntersect(GDISimpleEdge(*prepoint,tempregion2->m_points[mode2+1]),pointbuffer,edgebuffer);
				if (bIntersect){
					for (b=numpointbuffer;b<pointbuffer.size();b++) {
						//if the intersect point is already recorded, skip
						for (c=0;c<pointlist.size();c++) {
							if (pointbuffer[b]==pointlist[c]) {
								break;
							}
						}
						if (c<pointlist.size()) {
							continue;
						}
						//find the closest point to region.m_edges[mode1].point1;
						if (oldpoint==NULL) {
							oldpoint=new Vector3();
							*oldpoint=pointbuffer[b];
							intersectEdge=edgebuffer[b];
						}else{
							if (temppoint!=*oldpoint) {
								bool bTemp;
								bTemp=IsPoint1Closer(*prepoint,pointbuffer[b],*oldpoint);
								if (bTemp) {
									*oldpoint=pointbuffer[b];
									intersectEdge=edgebuffer[b];
								}
							}
						}

					}
					if (oldpoint==NULL&&pointbuffer.size()>numpointbuffer) {
						//the line intersects but the intersect points are already processed, so switch back to mode 1
						bSwitched=false;
						prepoint=&savepoint;
						//we consider the resume point is out of the current region, although it is on the edge of the current region
						bpreInside1=b2Inside1[mode1+1];
						continue;
					}else if(oldpoint!=NULL){
						if (!b2Inside1[mode2+1]) {
							//for inside-outside line, we exchange the tempregions and other variables
							newregion->m_points.push_back(*oldpoint);
							pointlist.push_back(*oldpoint);
							prepoint=oldpoint;

							//exchange temp variables
							const CGDIRegion *exchange=tempregion1;
							tempregion1=tempregion2;
							tempregion2=exchange;
							bool *bexchange=pb1Inside2;
							pb1Inside2=pb2Inside1;
							pb2Inside1=bexchange;
							mode2=intersectEdge-1;

						}//no outside-inside or outside-outside shall happen
					}else
					{
						prepoint=&tempregion2->m_points[mode2+1];
					}//if (oldpoint==NULL&&pointbuffer.size()>numpointbuffer)


				}else
				{
					prepoint=&tempregion2->m_points[mode2+1];
				}//if (bIntersect)
			}//if (pb2Inside1[mode2+1]) 
			mode2++;
			if (mode2>=tempregion2->m_edges.size()) {
				mode2=0;
			}
		}//if (!bSwitched)
	}
	if (newregion) {
		//make sure the new region is not infinite
		if (newregion->m_points.size()==0) {
			newregion->m_points.push_back(Vector3(0,0,0));
		}
		newregion->m_points.push_back(newregion->m_points.front());
		newregion->GenerateEdge();
		outregion.push_back(*newregion);	
	}
	SAFE_DELETE(oldpoint);
	SAFE_DELETE(newregion);
	return S_OK;
}*/
