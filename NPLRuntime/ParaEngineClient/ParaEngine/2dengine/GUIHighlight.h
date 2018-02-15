#pragma once
#include <map>
#include <string>
using namespace std;
namespace ParaEngine
{
	enum HighlightStyle{
		HighlightNone,
		Highlight4outsideArrow,
		HighlightNstageAnimation,
		HighlightCustom
	};
	class CGUIResource;
	struct GUIState;
	/**
	 * An abstract class
	 * "4outsideArrow", 4 arrows pointing from outside to the edge of the control
	 * "4insideArrow, 4 arrows pointing from the edge of the control to the center of the control
	 **/
	class CGUIHighlight:public IObject
	{
	public:
		/**
		 * Render the Highlight effect.
		 * If the effect is not properly initialize, it will call Init() to initialize it
		 * @param pGUIState: the same meaning as other GUI object
		 * @param rcScreen: the edge rectangle of the control we want to highlight
		 * @param fElapsedTime: the same meaning as other GUI object.
		 **/
		virtual HRESULT Render(GUIState* pGUIState,RECT& rcScreen, float fElapsedTime )=0;
		/**
		 * Set effect parameter. 
		 * the value will be properly translated to correct type.
		 * @param szName: name of the parameter
		 * @param szValue: new value of the parameter
		 * @return: false if the given parameter does not exist or the new value is not acceptable.
		 **/
		virtual bool SetParameter(const char* szName, const char* szValue)=0;

		virtual const char* GetHighlightName()const=0;
		virtual int GetHighlightStyle() const{return m_eStyle;};
		friend class CGUIHighlightManager;
	protected:
		virtual ~CGUIHighlight(){};
		virtual void Init()=0;
		bool			m_bInit;
		CGUIResource*	m_objResource;
		int				m_eStyle;
	};

	/**
	* element 0: left arrow
	* element 1: top arrow
	* element 2: right arrow
	* element 3: bottom. arrow
	* all elements should be 32*32
	 **/
	class CGUI4OutsideArrow:public CGUIHighlight
	{
	public:
		CGUI4OutsideArrow();

		virtual bool SetParameter(const char* szName, const char* szValue);

		virtual HRESULT Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime );

		virtual const char* GetHighlightName()const{return "4outsideArrow";}

		virtual void Clone(IObject* obj)const;
		virtual IObject* Clone()const;
	protected:
		virtual ~CGUI4OutsideArrow();
		virtual	void		Init();

		float				m_fSpeed;//animation speed factor, range [0,1]
		int					m_nRange;//animation range, 
		int					m_nPos;//animation position, range [0,m_nRange]	
		int					m_nImageSize;// the arrow image size
	};

	/**
	 * N stage animation          
	 * element 0---(n-1): the mask of the stages
	 **/
	class CGUINStageAnimation:public CGUIHighlight
	{
	public:
		CGUINStageAnimation();

		virtual bool SetParameter(const char* szName, const char* szValue);

		virtual HRESULT Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime );

		virtual const char* GetHighlightName()const{return "NstageAnimation";}

		virtual void Clone(IObject* obj)const;
		virtual IObject* Clone()const;
	protected:
		virtual ~CGUINStageAnimation();
		virtual	void		Init();
		int					m_nNumStage;
		int					m_nCurStage;//from 1 to n
		float				m_fSpeed;//in seconds
		float				m_fTime;
	};
	/**
	 * Holds the CGUIHighlight controls.
	 * Is a singleton, use CSingleton to hold it.
	 **/
	class CGUIHighlightManager
	{
	public:
		CGUIHighlightManager();
		~CGUIHighlightManager();
		/**
		 * Get a highlight effect. 
		 * @return: Null if not exists.
		 **/
		CGUIHighlight* GetHighlight(int style);
		CGUIHighlight* GetHighlight(const char* szName);

		/** add a new highlighting style */
		void AddHighlight(int style, CGUIHighlight* pHighlight);

		/** clear all instances in the post render list. This function is called at the beginning of each frame.*/
		void ClearAllInstance();
		/** add a new instance into the post render list. This function is called at the end of the rendering pipeline of each control.*/
		void AddNewInstance(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime, int style);

		/**
		* render all highlighting markers in the post rendering list
		* @param pGUIState 
		* @param fElapsedTime 
		* @param bAutoCleanInstance if true ClearAllInstance() will be called before this function returns
		* @return 
		*/
		HRESULT Render(GUIState* pGUIState, float fElapsedTime,bool bAutoCleanInstance = true);

		/** this function is not used, unless you do not want to post render an instance.
		a short-cut for getting a highlight style and render the style*/
		static HRESULT Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime, int style);

		void Finalize();

		/**
		 * Create a custom highlight effect from an existing highlight effect
		 * The new effect can has different parameter of the parent effect. 
		 **/
		CGUIHighlight* CreateCustomHighlight(const char* szName, const char* szParentName);
	protected:
		map<int,CGUIHighlight*> m_items;
		map<string, CGUIHighlight*> m_customs;

		/** marker instance */
		struct MarkerInstance
		{
			RECT rcScreen;
			float fElapsedTime;
			int style;
			MarkerInstance(RECT& rcScreen_,float fElapsedTime_, int style_):rcScreen(rcScreen_), fElapsedTime(fElapsedTime_), style(style_){};
		};
		/** post render instances */
		list<MarkerInstance> m_instaces;
	};
}