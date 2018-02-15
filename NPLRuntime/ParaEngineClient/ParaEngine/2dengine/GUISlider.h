#pragma once
using namespace std;
#include "GUIBase.h"
namespace ParaEngine
{
	/**
	* 
	*/
	class CGUISlider : public CGUIBase
	{
	public:
		CGUISlider();

		virtual BOOL ContainsPoint( const POINT &pt ); 
		virtual bool CanHaveFocus() { return (m_bIsVisible && m_bIsEnabled); }

		virtual void UpdateRects(); 
		virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);

		virtual HRESULT Render(GUIState* pGUIState,float fElapsedTime);
		virtual bool OnChange(const char* code=NULL);
		
		virtual IObject* Clone()const;
		virtual void Clone(IObject *pobj)const;
		static void StaticInit();

		//virtual bool MsgProc(CGUIEvent *event=NULL);
		virtual bool MsgProc(MSG* event);

		void SetValue( int nValue );
		int  GetValue() const { return m_nValue; };

		void GetRange( int &nMin, int &nMax ) const { nMin = m_nMin; nMax = m_nMax; }
		void SetRange( int nMin, int nMax );

		void SetLinkedObject(CGUIBase* obj){m_Linked=obj;}
		virtual const IType* GetType()const{return m_type;}
	
	protected:
		static const IType* m_type;
		void SetValueInternal( int nValue );
		int  ValueFromPos( int x ); 

		int m_nValue;

		int m_nMin;
		int m_nMax;

		int m_nDragX;      // Mouse position at start of drag
		int m_nDragOffset; // Drag offset from the center of the button
		int m_nButtonX;

		bool m_bPressed;
		CGUIBase* m_Linked;
	private:
		int m_nOldValue;
	};
}