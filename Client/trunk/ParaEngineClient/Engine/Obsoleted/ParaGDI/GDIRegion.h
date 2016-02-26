#pragma once
#include <vector>
#include <list>
using namespace std;
namespace ParaEngine
{
	struct GDISimpleEdge {
	public:
		GDISimpleEdge(const Vector3 &p1,const Vector3 &p2);
		friend class CGDIRegion;
	protected:
		GDISimpleEdge();
		Vector3 point1,point2;
		float slope;//=tgA=(point2.y-point1.y)/(point2.x-point1.x)
		float intersect;//=point1.y-slope*point1.x
		float GetY(float x)const{return intersect+slope*x;}
		int vertical;//if ==0 the edge is not vertical, if >0 the edge is upwards vertical, if <0 the edge is downwards vertical
	};
	struct GDIClipNode {
		Vector3 point;
		//-1 for inside-outside , 0 for none, 1 for outside-inside point
		int nodetype;
		//link to other place
		list<GDIClipNode>::const_iterator link;
	};
	/**
	* We always shape the region points and edges in clockwise
	*/
	class CGDIRegion
	{
	public:
		/**
		* This default constructor constructs an infinite region
		*/
		CGDIRegion();
		/**
		* This constructor constructs a region with the given rectangle
		*/
		CGDIRegion(const RECT &rect);
		bool operator ==(const CGDIRegion &region);
		bool operator ==(const RECT &rect);
		operator RECT();
		/**
		* Move the whole region 
		*/
		void Translate(float dx,float dy,float dz=0);
		void Translate(const Vector3 &v);

		/**
		* Check if the region is empty
		*/
		bool IsEmpty()const;
		/**
		* Check if the region is infinite
		*/
		bool IsInfinite()const{return m_points.size()==0;}
		void MakeEmpty();
		void MakeInifinite();
		CGDIRegion *Clone();
		/**
		* Find the intersection of a line with the region
		* @param point1, point2: [in] the start point and end point of the input line
		* @param outpoint1, outpoint2: [out] the start point and end point of the line which intersects with the region. Their meaningless if the return is false;
		* @reture: true if the line is inside the region or intersects with the region. false if the line is outside the region
		*/
		bool LineIntersect(const Vector3 &point1,const Vector3 &point2, Vector3 &outpoint)const;
		bool LineIntersect(const GDISimpleEdge &edge,vector<Vector3> &outpoint,vector<int> &outedge)const;
		/**
		* Decide if the point is inside or outside the region
		* @return: true if inside, false if outside
		*/
		bool InsideTest(const Vector3 &point)const;

		/**
		* Clip the input region with the current region
		* the outregion contains all intersections, because clipping a polygon may result in more than one polygon
		* The input region is the polygon to be clipped and the current region is the clipping window
		*/
		int PolygonClip(const CGDIRegion &region,vector<CGDIRegion> &outregions);

		/**
		* Clip two rectangle regions
		*/
		int RectClip(const CGDIRegion &region,CGDIRegion &outregion);
		/**
		* Static function to decide and find the intersect point of two edges
		* @return: true if the two edges intersect, false if not
		*/
		static bool EdgeIntersect(const GDISimpleEdge &edge1,const GDISimpleEdge &edge2,Vector3 &point);

		Vector3 GetPoint(int index)const;
		int PointCount()const{return (int)m_points.size();}
		GDISimpleEdge GetEdge(int index)const;
	protected:
		/**
		* Decide if the input edge intersects with any of the edges in the current region, 
		* @return: -1 if edge is inside the region, 0 if edge intersects with the region, 1 if edge is outside the region
		*/

		/**
		* Compare two points against the source point and decide which point is closer to the source point
		* if relation is 0, all the points are supposed to be on the same line
		*/
		bool IsPoint1Closer(const Vector3 &point,const Vector3 &point1, const Vector3 &point2,int relation=0);
		bool CalculateClipX(const GDISimpleEdge &edge1,const GDISimpleEdge &edge2,float &pointx)const;
		void GenerateEdge();
		void Preprocessing();
		vector<Vector3> m_points;
		vector<GDISimpleEdge> m_edges;
		list<GDIClipNode> m_nodes;
		int m_shape;
	};
}