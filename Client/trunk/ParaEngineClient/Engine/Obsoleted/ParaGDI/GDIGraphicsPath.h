#pragma once
#include <vector>
#include "GDIPen.h"
#include "GDIRegion.h"
namespace ParaEngine
{
	struct GUITextureElement;
	struct GDIPathElement{
	public:
		Vector3 point;
		Color color;
		//the element's drawing pen, this value is meaningful only when the element's pointtype is PathPointTypeStart
		CGDIPen pen;
		byte pointtype;
		CGDIRegion region;//the clip region for the figure, it is only useful when point type is PathPointTypeStart;
	};
	struct GDIFigure{
	public:
		vector<DXUT_SCREEN_VERTEX> vertices;
		D3DPRIMITIVETYPE PrimitiveType;
		const GDIPathElement *pElement;//pointer to the start element of this figure
		byte operationtype;
		Matrix4 matTransform;
	};
	class CGDIGraphicsPath
	{
	public:
		CGDIGraphicsPath();
		CGDIGraphicsPath(const Vector3* points,const BYTE* types,int count);
		~CGDIGraphicsPath(){};

		CGDIGraphicsPath* Clone()const ;

		/** undo a certain amount of steps*/
		void Undo(int nStep=1);
		/** redo a certain amount of steps*/
		void Redo(int nStep=1);

		/** reset to be reused */
		void Reset();

		/**
		* Add a line to the path, 
		* If point1=point2, the line will not draw, use AddPoint if you want to draw a point
		*/
		void AddLine(const Vector3 &point1,const Vector3& point2,const CGDIPen &pen);

		/**
		* Add a polyline to the path
		*/
		void AddLines(const Vector3 *points,int nNumPoints,const CGDIPen &pen);
		/**
		* Add a point to the path, 
		*/
		void AddPoint(const Vector3 &point,const CGDIPen &pPen);

		/**
		* Add a rectangle to the path, 
		*/
		void AddRectangle(const Vector3 *points,const CGDIPen &pen);
		/**
		* Get the number of points in the path
		*/
		int GetPointCount()const {return (int)m_elements.size();};

		/**
		* Add a clear opeartion of the render target to the path
		*/
		void AddClear(const Color &color);
		/**
		* Get the state of whether the path has changed. 
		* If the path has changed, we will need to update its vertices.
		*/
//		bool GetChanged()const{return m_bNeedUpdate;}
		/**
		* Set the state of whether the path has changed. 
		* If the path has changed, we will need to update its vertices.
		*/
//		void SetChanged(bool changed){m_bHasChanged=changed;}
		/**
		* This operator allows you to get a GDIPathElement at a given index
		* If the position specified is greater than the size of the container, the result is undefined
		*/
		GDIPathElement& operator [](int index);

		/**
		* Optimize the path to eliminate duplicate or unnecessary elements
		*/
		void Optimize()	;

		/**
		* Updates the vertices.
		*/
		void UpdateVertices(int nOption=0);
		//void UpdateVertices(const GDIPathElement *pElement,D3DPRIMITIVETYPE PrimitiveType,const DXUT_SCREEN_VERTEX *pVertices, int nNumVertices,int nOption=0);
		/**
		* Add an operation to the figures
		*/
		void AddOperation(const GDIPathElement *pElement,byte operationtype);

		/**
		* Set the current clipping rectangle for the path
		*/
		void SetClip(const CGDIRegion &region);
		const CGDIRegion* GetClip()const{return &m_region;}
		/**
		* The following two are for iteration of the GDIPathElement
		*/
		vector<GDIPathElement>::iterator PathBegin(){return m_elements.begin();}
		vector<GDIPathElement>::iterator PathEnd(){return m_elements.end();}
		/**
		* The following two are for iteration of the GDIFigure
		*/
		list<GDIFigure>::iterator FigureBegin(){return m_figures.begin();}
		list<GDIFigure>::iterator FigureEnd(){return m_figures.end();}
	protected:
		void ClearFigures(){m_figures.clear();m_bNeedUpdate=true;}
		void CalculateLineVertex(const Vector3 *points,int nNumPoints, vector<DXUT_SCREEN_VERTEX> &vertices, const CGDIPen &pPen);	
		void CalculateRectVertex(const Vector3 *points,int nNumPoints, vector<DXUT_SCREEN_VERTEX> &vertices, const CGDIPen &pPen);	
		void UpdateFigures(const GDIPathElement *pElement, D3DPRIMITIVETYPE PrimitiveType, const vector<DXUT_SCREEN_VERTEX> &vertices, const Matrix4 *matTransform, int nOption = 0, int index = 0);
		vector<GDIPathElement> m_elements;
		list<GDIFigure> m_figures;
		LPDIRECT3DVERTEXBUFFER9 m_pVB;
		bool m_bNeedUpdate;
		CGDIRegion m_region;
	};
}