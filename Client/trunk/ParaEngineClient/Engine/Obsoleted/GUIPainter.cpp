//----------------------------------------------------------------------
// Class:	CGUIPainter
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: 2005.12.20
//
// desc: 
// The CGUIListBox control enables you to display a list of items to the user that the user can select by clicking. 
// A CGUIListBox control can provide single or multiple selections using the SetMultiSelect method. 
// In addition to display and selection functionality, the ListBox also provides features that enable you to 
// efficiently add items to the CGUIListBox and to find text within the items of the list. Use Size() to get the number of 
// list items in the CGUIListBox. When the control's GetMultiSelect()==true, use GetSelected to check if the item at given 
// index is selected. When the control's GetMultiSelect()==false, use GetSelected to get the selected item. 
// The GetItem method provide access to the list items that are used by the CGUIListBox. 
// This class provides fast back-end-adding and sequential and random access of the list items. 
// But deleting and randomly-inserting items are slower.
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "ParaWorldAsset.h"
#include "GUIResource.h"

#include "ParaGDI.h"
#include "GUIContainer.h"
#include "GUIPainter.h"
using namespace ParaEngine;
//for debug
const IType* CGUIPainter::m_type=NULL;


HRESULT CGUIPainter::TestGDI()
{
	return S_OK;
}

CGUIPainter::CGUIPainter()
{
	if (!m_type){
		m_type=IType::GetType("guipainter");
	}
}
HRESULT CGUIPainter::Render(GUIState* pGUIState,float fElapsedTime )
{
	//for test
	static bool btest=true;
	if (btest) {
		TestGDI();
	}
	return S_OK;
}

HRESULT CGUIPainter::RestoreDeviceObjects()
{
	return S_OK;
}
#endif