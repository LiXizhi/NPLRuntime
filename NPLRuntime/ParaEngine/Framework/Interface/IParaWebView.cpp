#include "ParaEngine.h"
#include "IParaWebView.h"


namespace ParaEngine {

	IAttributeFields* IParaWebView::GetAttributeObject()
	{
		return this;
	}


	int IParaWebView::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		// install parent fields if there are any. Please replace __super with your parent class name.
		IAttributeFields::InstallFields(pClass, bOverride);
		PE_ASSERT(pClass != nullptr);

		pClass->AddField("Url", FieldType_String, (void*)loadUrl_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("Alpha", FieldType_Float, (void*)setAlpha_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("Visible", FieldType_Bool, (void*)setVisible_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("HideViewWhenClickBack", FieldType_Bool, (void*)SetHideViewWhenClickBack_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("IgnoreCloseWhenClickBack", FieldType_Bool, (void*)SetIgnoreCloseWhenClickBack_s, (void*)nullptr, nullptr, nullptr, bOverride);
        pClass->AddField("CloseWhenClickBackground", FieldType_Bool, (void*)SetCloseWhenClickBackground_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("Refresh", FieldType_void, (void*)Refresh_s, nullptr, nullptr, "", bOverride);
        pClass->AddField("HideCloseButton", FieldType_Bool, (void*)hideCloseButton_s, (void*)nullptr, nullptr, nullptr, bOverride);
		pClass->AddField("bringToTop", FieldType_void, (void*)bringToTop_s, nullptr, nullptr, "", bOverride);
		pClass->AddField("move", FieldType_Float_Float, (void*)move_s, nullptr, nullptr, "", bOverride);
		pClass->AddField("resize", FieldType_Float_Float, (void*)resize_s, nullptr, nullptr, "", bOverride);
        pClass->AddField("Activate", FieldType_String, (void*)activate_s, (void*)nullptr, nullptr, nullptr, bOverride);

		return S_OK;
	}

} // end namespace
