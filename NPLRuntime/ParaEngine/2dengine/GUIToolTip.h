#pragma once
#include "GUIBase.h"

namespace ParaEngine
{
	struct GUIToolTipItem {
		enum TipState{BEFORESHOW,SHOWING,AFTERSHOW,BLINKING};
		CGUIBase* m_obj; //where the tooltip attached to
        u16string m_text; //what text it displays
		int m_behavior; //how this tooltip behaves 
		int m_timecount; //for effects
		int m_autopopdelay; //time length of displaying
		int m_initialdelay; //time before it shows
		bool m_show; //whether to show this item
		bool m_rendered; //whether this item needs to show in this frame
		RECT m_rect;//the size of the rectangle needed to draw text
		int m_state;
	};
	/*
	Elements:
	0. Text client "text"
	1. Left-Top corner "lefttop"
	2. Top border "top"
	3. Right-Top corner "righttop"
	4. Left border	"left"
	5. Right border "right"
	6. Left-bottom corner "leftbottom"
	7. Bottom border "bottom"
	8. Right-Bottom corner "rightbottom"
	9. Left Dialog indicator "leftindicator"
	10. Right Dialog indicator "rightindicator"
	11. Left Filling area "leftfill"
	12. Right Filling area "rightfill"
	*/
	/**
	m_objResource->m_objArtwork->DrawingRects[0]: text rectangle;
	m_objResource->m_objArtwork->DrawingRects[1]: left-top corner rectangle;
	m_objResource->m_objArtwork->DrawingRects[2]: top border rectangle;
	m_objResource->m_objArtwork->DrawingRects[3]: right-top corner rectangle;
	m_objResource->m_objArtwork->DrawingRects[4]: left border rectangle;
	m_objResource->m_objArtwork->DrawingRects[5]: right border rectangle;
	m_objResource->m_objArtwork->DrawingRects[6]: left-bottom corner rectangle;
	m_objResource->m_objArtwork->DrawingRects[7]: bottom border rectangle;
	m_objResource->m_objArtwork->DrawingRects[8]: right-bottom corner rectangle;
	m_objResource->m_objArtwork->DrawingRects[9]: indicator rectangle;
	m_objResource->m_objArtwork->DrawingRects[10]: indicator-extend line rectangle;
	m_objResource->m_objArtwork->DrawingRects[11]: left indicator filling rectangle;
	m_objResource->m_objArtwork->DrawingRects[12]: right indicator filling rectangle;
	*/
	class CGUIToolTip:public CGUIBase
	{
	public:
		/**
		* NONE: default behavior
		* FLASHING: flashing tip
		* POINTING: pointing to an object, drawing user attention to the object, text will not display.
		* STATIC: the tip will not automatically disappear.
		* HIGHTLIGHT: the tip will only disappear after the mouse has moved onto the object.
		*/
		enum ToolTip_Behavior{
			NONE=0,FLASHING=1,POINTING=2,STATIC=4,HIGHTLIGHT=8
		};
		CGUIToolTip();
		virtual ~CGUIToolTip();
		void Initialize();
		int GetBehavior(){return m_nBehavior;}
		void SetBehavior(int behavior){m_nBehavior=behavior;}
		int GetAutomaticDelay(){return m_nAutomaticDelay;}
		void SetAutomaticDelay(int automaticdelay){m_nAutoPopDelay=automaticdelay*20;m_nInitialDelay=automaticdelay;}
		int GetAutoPopDelay(){return m_nAutoPopDelay;}
		void SetAutoPopDelay(int autopopdelay){m_nAutoPopDelay=autopopdelay;}
		int GetInitialDelay(){return m_nInitialDelay;}
		void SetInitialDelay(int initialdelay){m_nInitialDelay=initialdelay;}
		
		/**
		* Set tool tip for an object
		* In addition to specifying the ToolTip text to display for a control, you can also use this method to modify the ToolTip 
		* text for a control. Calling the SetToolTip method more than once for a given control does not specify multiple ToolTip 
		* text to display for a control but instead changes the current ToolTip text for the control. To determine the ToolTip text 
		* that is associated with a control at run time, you can use the GetToolTip method.
		*/
		void SetToolTip(CGUIBase* obj,const char16_t* text);
		void SetToolTipA(CGUIBase* obj,const char* szText);
		const char16_t* GetToolTip(CGUIBase* obj);
		int GetToolTipA(CGUIBase* obj,std::string & out);
		GUIToolTipItem* GetActiveTip();
		void RemoveTip(CGUIBase* obj);
		void RemoveAll();

		/**
		* Activate the tip related to an object, if the tip exists.
		* If the there is already and activate tip, this function will deactivate it and activate a new tip.
		*/
		void ActivateTip(CGUIBase* obj);
		/**
		* Deactivate the a tip related to an object, if the tip is activated.
		* @param obj: the object whose tip is being deactivated. If obj is NULL, the current activated tip will be deactivated.
		* @remark: if you want to deactivate a tip whose behavior is HIGHLIGHT, you cannot use NULL to deactivated it. Provide the 
		*	exact object where the HIGHLIGHT tip is related to.
		*/
		void DeactivateTip(CGUIBase* obj);
		void SetIndicator(int indicator){m_nIndicator=indicator>0?indicator:0;}
		int GetIndicator(){return m_nIndicator;}
		virtual int Release();
		virtual void			UpdateRects();
		virtual void			InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		virtual HRESULT			Render(GUIState* pGUIState ,float fElapsedTime);
		virtual bool			MsgProc(CGUIEvent *event/* =NULL */){return false;}
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		map<CGUIBase*,GUIToolTipItem> m_items;
		int m_nBehavior;
		int m_nAutomaticDelay;
		int m_nAutoPopDelay;
		int m_nInitialDelay;
		int m_nIndicator;
		GUIToolTipItem *m_ActiveTip;
		/** time of second elapsed.*/
		void CheckTipState(float fTimeElapsed);
	};
}