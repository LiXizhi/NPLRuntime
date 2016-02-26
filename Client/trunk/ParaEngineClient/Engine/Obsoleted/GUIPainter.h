#pragma once
#include "GUIBase.h"

namespace ParaEngine
{
	class CGUIContainer;
	//struct GUIPainterElement {
	//	list<POINT3D> m_points;
	//	float m_fSize;
	//	int m_nBrushType;
	//	int m_nType;
	//	GUITextureElement* m_texture;
	//};
	//struct GUIPainterElementContainer{
	//public:
	//	void BeginElement(int nType,int nBrushType,float fSize,GUITextureElement *texture);
	//	void StepElement(POINT3D point);
	//	void EndElement(bool bCancel);
	//	list<GUIPainterElement>::iterator begin(){return m_items.begin();};
	//	list<GUIPainterElement>::iterator end(){return m_items.end();}
	//protected:
	//	list<GUIPainterElement> m_items;
	//	GUIPainterElement* m_pCurrent;
	//};
	class CGUIPainter:public CGUIContainer
	{
	public:
		enum DrawType{POINTS,FREELINE,LINE};
		enum DrawState{NONE,BEGIN,STEP};
		CGUIPainter();
		~CGUIPainter(){};
	//	void Show();
	//	void Hide();
	//	void SaveBMP(const char* filename);
	//	int GetState();

	//	/**
	//	* these three functions are for a drawing process
	//	*/
	//	//used by script interface
	//	void BeginDraw();
	//	void EndDraw(bool bCancel=false);
	//	void Step(POINT3D point);

	//	//the following function are for drawing different elements
	//	void DrawLine(POINT3D point1,POINT3D point2);
	//	void DrawPoint(POINT3D point);

	//	void SetDrawType(int drawtype){m_nDrawType=drawtype;EndDraw(false);}
	//	int GetDrawType(){return m_nDrawType;}
	//	void SetBrush(CGUIBrush *brush);
	//	const CGUIBrush* GetBrush(){return &m_brush;};
	//	void ClearScreen(GUIState* pGUIState);
	//	HRESULT Redraw(GUIState* pGUIState);

		//virtual void UpdateRects(){};
		//virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		//virtual bool MsgProc(CGUIEvent *event/* =NULL */);
		virtual HRESULT Render(GUIState* pGUIState ,float fElapsedTime);
		
		virtual HRESULT InitDeviceObjects(){return S_OK;};
		virtual HRESULT RestoreDeviceObjects(); // device dependent
		virtual HRESULT InvalidateDeviceObjects(){return S_OK;};
		virtual HRESULT DeleteDeviceObjects(){return S_OK;};
		virtual const IType* GetType()const{return m_type;}
	
	protected:
		static const IType* m_type;
		//this function is just for debug
		HRESULT TestGDI();
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DTEXTURE9 m_pTexture;
#endif
	};
}