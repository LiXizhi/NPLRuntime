#pragma once

#include "GUIBase.h"
namespace ParaEngine
{
	class CGUIContainer;
	class C3DCanvas;
	class CGUICanvas:public CGUIBase
	{
	public:
		CGUICanvas();
		virtual ~CGUICanvas();

		virtual void Reset();
		virtual HRESULT Render(GUIState* pGUIState,float fElapsedTime );
		void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		void UpdateRects();
		virtual bool MsgProc(MSG *event);

		//zoom the 3d object 
		void Zoom(double zoom);

		//rotate the 3d object, in radius
		void Rotate(double xrotate,double yrotate);

		//move the 3d object
		void Pan(double x, double y);

		void SetCanvasIndex(int index);
		int GetCanvasIndex()const{return m_nCanvasIndex;}

		virtual IObject* Clone()const;
		virtual void Clone(IObject* pobj)const;
		virtual const IType* GetType()const{return m_type;}
		/**
		* Initialize the event mapping for the default CGUIButton class
		* All its inheritances should have a StaticInit() for each kind of class.
		*/
		static void StaticInit();

	protected:
		enum{
			CanvasNone,
			CanvasZoom,
			CanvasRotate,
			CanvasPan
		}CanvasState;
		static const IType*			m_type;
		int							m_eState;
		int							m_nCanvasIndex;
		double						m_fRotateSpeed;
		double						m_fPanSpeed;
	};
}
