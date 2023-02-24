//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi, Liu Weili
// Company: ParaEngine
// Date:	2005.9.8
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "2dengine/GUIEvent.h"
#include "2dengine/GUIText.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIButton.h"
#include "2dengine/GUIEdit.h"
#include "2dengine/GUIIMEEditBox.h"
#include "2dengine/GUIScript.h"
#include "2dengine/GUIResource.h"
#include "2dengine/GUIListBox.h"
#include "2dengine/GUIToolTip.h"
#include "2dengine/GUISlider.h"
#include "2dengine/GUIScrollBar.h"
#if defined(USE_DIRECTX_RENDERER) && !defined(NPLRUNTIME)
#include "2dengine/GUIWebBrowser.h"
#endif
#include "2dengine/GUIHighlight.h"
#include "util/SimpleTranslator.h"
#include "SceneObject.h"
#include "CSingleton.h"
#include "ObjectManager.h"
#include "ParaScriptingGlobal.h"
#include "ParaScriptingScene.h"
#include "NPLHelper.h"
#include "util/StringHelper.h"
#include "ParaScriptingGUI.h"

extern "C"
{
#include <lua.h>
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "memdebug.h"


namespace ParaScripting
{
//--------------------------------------------------------------
// for ParaUIObject namespace table.
//--------------------------------------------------------------

ParaUIObject::ParaUIObject()
{
}

ParaUIObject::ParaUIObject(CGUIBase * obj)
	:m_pObj(obj)
{
}
void ParaUIObject::BringToFront()
{
	if (m_pObj){
		m_pObj->BringToFront();
	}
}
void ParaUIObject::BringToBack()
{
	if (m_pObj){
		m_pObj->SendToBack();
	}
}
void ParaUIObject::AddChild(ParaUIObject pChild)
{
	if (!IsValid()) return;
	if(pChild.IsValid()){
		CGlobals::GetGUI()->AttachGUIElement(m_pObj, pChild.m_pObj);
		//m_pObj->UpdateRects();
	}
}

void ParaUIObject::AttachToRoot()
{
	if (!IsValid()) return;
    
	CGlobals::GetGUI()->AttachGUIElement(NULL, m_pObj);
}

void ParaUIObject::AttachTo3D(const ParaObject obj)
{
	if (!IsValid() || !obj.IsValid()) 
		return;
	if (IsValid())
	{
		m_pObj->AttachTo3D(obj.m_pObj.get());
		if (m_pObj->GetParent() == NULL)
			CGlobals::GetGUI()->AttachGUIElement(NULL, m_pObj);
	}
}

void ParaUIObject::AttachTo3D_(const char* s3DObjectName)
{
	if (IsValid())
	{
		m_pObj->AttachTo3D(s3DObjectName);
		if(m_pObj->GetParent()==NULL)
			CGlobals::GetGUI()->AttachGUIElement(NULL, m_pObj);
	}
}

string ParaUIObject::GetName()const 
{
	if (!IsValid()) return "";
	return m_pObj->GetName();
}

void ParaUIObject::GetAbsPosition(float &x,float &y,float& width, float& height, float &z)const
{
	CGUIPosition temp;
	if (!IsValid()){ return; }
	m_pObj->GetAbsolutePosition(&temp,m_pObj->GetPosition());
	x=(float)temp.rect.left;
	y=(float)temp.rect.top;
	width=(float)temp.rect.right - x;
	height=(float)temp.rect.bottom - y;
	z=m_pObj->GetPosition()->GetDepth();
}
ParaUIObject ParaUIObject::GetParent()const
{
	return ParaUIObject( IsValid() ? m_pObj->GetParent() : nullptr);
}

void ParaUIObject::SetParent(ParaUIObject parent)
{
	if (!IsValid()) return;
	CGlobals::GetGUI()->DetachGUIElement(m_pObj);
	CGlobals::GetGUI()->AttachGUIElement(parent.m_pObj,m_pObj);
}
string ParaUIObject::GetType()const
{
	if (!IsValid()) return "";
	string type="";
	switch(m_pObj->GetType()->GetTypeValue()) {
	case Type_GUIContainer:
		type="container";
		break;
	case Type_GUIButton:
		type="button";
		break;
	case Type_GUIEditBox:
		type="editbox";
		break;
	case Type_GUIIMEEditBox:
		type="imeeditbox";
		break;
	case Type_GUIScrollBar:
		type="scrollbar";
		break;
	case Type_GUIText:
		type="text";
		break;
	case Type_GUIRoot:
		type="root";
		break;
	case Type_GUISlider:
		type="slider";
		break;
	case Type_GUIListBox:
		type="listbox";
		break;
	case Type_GUIToolTip:
		type="tooltip";
		break;
	case Type_GUIWebBrowser:
		type="webbrowser";
		break;
	}
	return type;
}
//ParaUIResource ParaUIObject::GetResource()
//{
//	if (!IsValid()) return ParaUIResource(NULL);
//	return ParaUIResource(m_pObj->m_objResource);
//}

void ParaUIObject::SetZDepth(float fDepth)
{
	if ( IsValid() ) 
	{
		m_pObj->SetZDepth(fDepth);
	}
}

float ParaUIObject::GetZDepth()
{
	if ( IsValid() ) 
	{
		return m_pObj->GetZDepth();
	}
	return 0.f;
}

void ParaUIObject::SetLifeTime(int nSeconds)
{
	if (!IsValid()) return;
	// internally, we use milliseconds. 
	if(nSeconds < 0)
		m_pObj->SetLifeTime(-1); // permanent
	else if(nSeconds >=0)
		m_pObj->SetLifeTime(nSeconds*1000); 
}

int ParaUIObject::GetLifeTime()const
{
	if (!IsValid()) return -1;
	int nSeconds = m_pObj->GetLifeTime();
	return (nSeconds>0) ? nSeconds/30:nSeconds;
}

void ParaUIObject::SetMultipleSelect(bool multiple)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)
	{
		((CGUIListBox*)m_pObj)->SetMultiSelect(multiple);
	}
}

bool ParaUIObject::GetMultipleSelect()const
{
	if (!IsValid()) return false;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)
	{
		return ((CGUIListBox*)m_pObj)->GetMultiSelect();
	}
	return false;
}

void ParaUIObject::SetBindingObj(ParaUIObject obj)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)
	{
		((CGUIScrollBar*)m_pObj)->SetLinkedObject(obj.m_pObj);
	}
}

void ParaUIObject::SetFixedThumb(bool bFixed)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)
	{
		((CGUIScrollBar*)m_pObj)->SetFixedThumb(bFixed);
	}
}

bool ParaUIObject::GetFixedThumb()const 
{
	if (!IsValid()) return false;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)
	{
		return ((CGUIScrollBar*)m_pObj)->GetFixedThumb();
	}
	return false;
}
int ParaUIObject::GetThumbSize()const
{
	if (!IsValid()) return 0;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)
	{
		return ((CGUIScrollBar*)m_pObj)->GetThumbSize();
	}
	return 0;
}

void ParaUIObject::SetThumbSize(int size)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)
	{
		((CGUIScrollBar*)m_pObj)->SetThumbSize(size);
	}
}
void ParaUIObject::SetTopLevel(bool value)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIContainer)
	{
		return ((CGUIContainer*)m_pObj)->SetTopLevel(value);
	}

}

void ParaUIObject::SetPasswordChar(string PasswordChar)
{
	if (!IsValid()) return;

	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIEditBox:
		((CGUIEditBox*)m_pObj)->SetPasswordChar(PasswordChar.empty()?'\0':PasswordChar[0]);
		break;
	default:
		break;
	}
}

string ParaUIObject::GetPasswordChar() const
{
	if (!IsValid()) return "";
	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIEditBox:
		{
			string ss="";
			char s = ((CGUIEditBox*)m_pObj)->GetPasswordChar();
			if(s!='\0')
			{
				ss="*";
				ss[0] = s;
			}
			return ss;
			break;
		}
	default:
		break;
	}
	return "";
}

void ParaUIObject::SetText3(const char * strText, const char * strColor, const char * strFontAssetName)
{
	if (!IsValid()) return;
	if(strText == 0)
	{
		strText = "";
	}
	LinearColor color(0.f,0.f,0.f,0.f);
	if(strColor)
	{
		if(sscanf(strColor, "%f %f %f", &color.r,&color.g,&color.b) < 3)
		{
			OUTPUT_LOG("warning: invalid SetText3 color field\r\n");
		}
	}
	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIButton:
		((CGUIButton*)m_pObj)->SetTextA(strText);
		
		if(strColor)
		{
			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Pressed);
			m_pObj->m_objResource->SetFontColor(color);
			
			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Highlight);
			m_pObj->m_objResource->SetFontColor(color);
			
			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
			m_pObj->m_objResource->SetFontColor(color);
		}
		break;
	case Type_GUIIMEEditBox:
	case Type_GUIEditBox:
		((CGUIEditBox*)m_pObj)->SetTextA(strText);
		if(strColor)
			m_pObj->m_objResource->SetFontColor(color);
		break;
	case Type_GUIWebBrowser:
		m_pObj->SetTextA(strText);
		break;
	case Type_GUIText:
		((CGUIText*)m_pObj)->SetTextA(strText);
		if(strColor)
			m_pObj->m_objResource->SetFontColor(color);
		break;
	default:
		OUTPUT_LOG("warning: %s does not support property text\r\n", GetType().c_str());
		break;
	}
}

void ParaUIObject::SetText(const char* strText)
{
	SetText3(strText,NULL,"");
}

void ParaUIObject::SetText1(const object& strText)
{
	SetText(NPL::NPLHelper::LuaObjectToString(strText));
}

void ParaUIObject::SetPosition(const char * pos)
{
	if (!IsValid()) return;
	int left,top,width,height;
	if(sscanf(pos, "%d %d %d %d", &left,&top,&width,&height)==4){
		m_pObj->SetPosition(left,top,width,height);
	}
}

string ParaUIObject::GetPosition()const
{
	if (!IsValid()) return "";
	char temp[30];
	string strkey="";
	ParaEngine::StringHelper::fast_itoa(GetX(),temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(GetY(),temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(Width(),temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(Height(),temp,30);
	strkey+=temp;
	
	return strkey;
}

void ParaUIObject::SetEnabled(bool bEnabled)
{
	if (!IsValid()) return;
	m_pObj->SetEnabled(bEnabled);
}

bool ParaUIObject::GetEnabled() const
{
	if (!IsValid()) return false;
	return m_pObj->GetEnabled();
}

//bool ParaUIObject::IsDummy() const
//{
//	if (IsValid()) 
//		return m_pObj->IsDummy();
//	else 
//		return false;
//}
//
//void ParaUIObject::SetDummy(bool bDummy)
//{
//	if (IsValid()) 
//		m_pObj->SetDummy(bDummy);
//}

void ParaUIObject::SetVisible(bool bVisible)
{
	if (!IsValid()) return;
	m_pObj->SetVisible(bVisible);
}

bool ParaUIObject::GetVisible() const
{
	if (!IsValid()) return false;
	return m_pObj->GetVisible();
}

void ParaUIObject::SetAutoSize(bool bAutosize)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIText){
		((CGUIText*)m_pObj)->SetAutoSize(bAutosize);
	}else{
		OUTPUT_LOG("error: %s of type %s does not support property autosize.\n", m_pObj->GetName().c_str(), this->GetType().c_str());
	}
}

bool ParaUIObject::GetAutoSize() const
{
	if (!IsValid()) return false;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIText)
	{
		return ((CGUIText*)m_pObj)->IsAutoSize();
	}else{
		OUTPUT_LOG("error: %s of type %s does not support property autosize.\n", m_pObj->GetName().c_str(), this->GetType().c_str());
		return false;
	}
}

void ParaUIObject::SetBGImage1(const object& Background)
{
	const char* szBackground = NPL::NPLHelper::LuaObjectToString(Background);
	SetBGImageStr(szBackground);
}
/**
szBackground can be filename[; left top width height][:left top toright to bottom], such as
"texture/whitedot.png", "texture/whitedot.png;0 0 64 64", "texture/whitedot.png:10 10 10 10", "texture/whitedot.png;0 0 64 64;10 10 10 10"
[:left top toright to bottom] is used to specify the inner rect of the center tile in a nine texture element UI object
*/
void ParaUIObject::SetBGImageStr(const char * szBackground)
{
	if (szBackground==NULL||!IsValid()) 
		return;
	string temp=szBackground,filename;

	int nInnerRectIndex = 0;
	for (int i=0;szBackground[i]!='\0'; ++i)
	{
		// this fixed a bug for win32 absolute path like "c:/"
		if(szBackground[i] == ':' && i>2)
		{
			nInnerRectIndex = i;
			break;
		}
	}
	if(nInnerRectIndex>0)
	{
		int left, top, toright, tobottom;
		if(sscanf(&(szBackground[nInnerRectIndex+1]), "%d %d %d %d", &left, &top, &toright, &tobottom)==4)
		{
			temp.resize(nInnerRectIndex);
			// LiXizhi: 2007.9.27 this is very tricky. since we will create different state for the artwork layer, using an empty texture, 
			// and then create the nine element background only on the normal state. 
			if(m_pObj->m_objResource->GetActiveLayer() != CGUIResource::LAYER_ARTWORK)
				SetBGImageStr("");
			SetNineElementBG(temp.c_str(), left, top, toright, tobottom);
			return;
		}
	}

	
	RECT *prect=NULL;
	RECT rect;
	if(!temp.empty())
		prect=ParaEngine::StringHelper::GetImageAndRect(temp,filename, &rect);
	else
	{
		rect.left = 0;
		rect.right = 0;
		rect.top = 0;
		rect.bottom = 0;
		prect=&rect;
	}
	SetBGImage2(filename.c_str(),prect);
}

void ParaUIObject::SetBGImage(ParaAssetObject entity)
{
	if (entity.m_pAsset) 
	{
		if(entity.GetType() == AssetEntity::texture)
		{
			SetBGImage_((TextureEntity*)entity.m_pAsset, NULL);
		}
	}
}

void ParaUIObject::SetBGImageAndRect(ParaAssetObject entity,int left,int top,int width,int height)
{
	if (entity.IsValid()) 
	{
		if(entity.GetType() == AssetEntity::texture)
		{
			RECT rect;
			rect.left = left;
			rect.right = left+width;
			rect.top = top;
			rect.bottom = top+height;
			SetBGImage_((TextureEntity*)entity.m_pAsset, &rect);
		}
	}
}

void ParaUIObject::SetBGImage2(const char * TextFilename,RECT *rect)
{
	if (!IsValid()) return;
	TextureEntity* pTextureEntity=NULL;
	if ((pTextureEntity=CGlobals::GetAssetManager()->GetTexture(TextFilename))==NULL) 
	{
		pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(TextFilename,TextFilename,TextureEntity::StaticTexture);
	}
	SetBGImage_(pTextureEntity, rect);
}
void ParaUIObject::SetBGImage5(const char * TextFilename,int left,int top,int width,int height)
{
	RECT rect;
	SetRect(&rect,left,top,left+width,top+height);
	SetBGImage2(TextFilename,&rect);
}

void ParaUIObject::SetBGImage_(TextureEntity* pTextureEntity,RECT *rect)
{
	if (pTextureEntity==NULL) {
		return;
	}
	m_pObj->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
	m_pObj->SetDirty(true);
	GUITextureElement* pElement = NULL;
	//m_pObj->m_objResource->SetActiveLayer();
	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIButton:
	case Type_GUISlider:
	case Type_GUIListBox:
	case Type_GUIContainer:
		{
			Color color = 0xffffffff;
			GUITextureElement TexElement;
			GUITextureElement* pElement = NULL;

			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
			pElement = m_pObj->m_objResource->GetTextureElement(0);
			if(pElement)
			{
				color = pElement->TextureColor;
				pElement->SetElement(pTextureEntity,rect,color);
			}
			else
			{
				TexElement.SetElement(pTextureEntity,rect,color);
				m_pObj->m_objResource->SetElement(&TexElement, 0);
			}
			
			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Highlight);
			pElement = m_pObj->m_objResource->GetTextureElement(0);
			if(pElement)
			{
				color = pElement->TextureColor;
				pElement->SetElement(pTextureEntity,rect,color);
			}
			else
			{
				TexElement.SetElement(pTextureEntity,rect,color);
				m_pObj->m_objResource->SetElement(&TexElement, 0);
			}

			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Pressed);
			pElement = m_pObj->m_objResource->GetTextureElement(0);
			if(pElement)
			{
				color = pElement->TextureColor;
				pElement->SetElement(pTextureEntity,rect,color);
			}
			else
			{
				TexElement.SetElement(pTextureEntity,rect,color);
				m_pObj->m_objResource->SetElement(&TexElement, 0);
			}

			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Disabled);
			pElement = m_pObj->m_objResource->GetTextureElement(0);
			if(pElement)
			{
				color = pElement->TextureColor;
				pElement->SetElement(pTextureEntity,rect,color);
			}
			else
			{
				TexElement.SetElement(pTextureEntity,rect,color);
				m_pObj->m_objResource->SetElement(&TexElement, 0);
			}

			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
			break;
		}
	case Type_GUIEditBox:
	case Type_GUIIMEEditBox:
	case Type_GUIText:
	case Type_GUICanvas:
		{
			Color color = 0xffffffff;
			GUITextureElement TexElement;
			GUITextureElement* pElement = NULL;

			m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
			pElement = m_pObj->m_objResource->GetTextureElement(0);
			if(pElement)
			{
				color = pElement->TextureColor;
				pElement->SetElement(pTextureEntity,rect,color);
			}
			else
			{
				TexElement.SetElement(pTextureEntity,rect,color);
				m_pObj->m_objResource->SetElement(&TexElement, 0);
			}
			break;
		}
	default:
		OUTPUT_LOG("warning: %s of type %s does not support background image\n", m_pObj->GetName().c_str(), this->GetType().c_str());
		break;
	}
	//m_pObj->m_objResource->SetCurrentState();
	//m_pObj->m_objResource->SetActiveLayer();
}

void ParaUIObject::SetNineElementBG(const char * TextFilename, int left, int top, int toRight, int toBottom)
{
	if (TextFilename==NULL||!IsValid()) {
		return;
	}
	if(left ==0 && top==0 && toRight==0 && toBottom ==0)
	{
		m_pObj->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
		return SetBGImageStr(TextFilename);
	}
	string filename;
	RECT rect;
	memset(&rect, 0, sizeof(RECT));
	ParaEngine::StringHelper::GetImageAndRect(TextFilename,filename, &rect);
	//if(m_pObj->m_objResource->getac)
	if(!filename.empty())
	{
		//m_pObj->m_objResource->SetActiveLayer();
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		m_pObj->SetDirty(true);
		switch(m_pObj->GetType()->GetTypeValue())
		{
		case Type_GUIButton:
		case Type_GUIListBox:
		case Type_GUIText:
		case Type_GUICanvas:
		case Type_GUISlider:
		case Type_GUIScrollBar:
		case Type_GUIIMEEditBox:
		case Type_GUIEditBox:
		case Type_GUIToolTip:
		case Type_GUIContainer:
			{
				int textureOffset = 0;
				int nObjType = m_pObj->GetType()->GetTypeValue();
				if(nObjType == Type_GUIListBox)
					textureOffset = 2;
				else if(nObjType == Type_GUISlider)
					textureOffset = 2;
				else if(nObjType == Type_GUIScrollBar)
					textureOffset = 4; // the "thumb" texture
				TextureEntity* pTexture = CGlobals::GetAssetManager()->LoadTexture("",filename,TextureEntity::StaticTexture);
				if(rect.right == 0)
				{
					const TextureEntity::TextureInfo* pTextureInfo = pTexture->GetTextureInfo();
					if(pTextureInfo)
					{
						// please note, right and bottom may be (-1,-1) if texture is not available. the GUIElement will automatically recover to correct value when texture is available. 
						rect.right = pTextureInfo->m_width;
						rect.bottom = pTextureInfo->m_height; 
					}
				}

				RECT rcInner;
				rcInner.left = rect.left +left;
				rcInner.top = rect.top +top;
				rcInner.right = rect.right - toRight;
				rcInner.bottom = rect.bottom - toBottom;
				GUITextureElement TexElement;
				m_pObj->m_objResource->SetLayerType(GUILAYER::NINE_ELEMENT);

				// use original (default) color
				Color color = 0xffffffff;
				GUITextureElement* pElement = m_pObj->m_objResource->GetTextureElement(0);
				if(pElement)
					color = pElement->TextureColor;
				
				RECT rcTexture = rcInner;
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+0);

				SetRect( &rcTexture, rect.left, rect.top, rcInner.left, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+1);

				SetRect( &rcTexture, rcInner.left, rect.top, rcInner.right, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+2);

				SetRect( &rcTexture, rcInner.right, rect.top, rect.right, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+3);

				SetRect( &rcTexture, rect.left, rcInner.top, rcInner.left, rcInner.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+4);

				SetRect( &rcTexture,rcInner.right, rcInner.top, rect.right, rcInner.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+5);

				SetRect( &rcTexture, rect.left, rcInner.bottom, rcInner.left, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+6);

				SetRect( &rcTexture, rcInner.left, rcInner.bottom, rcInner.right, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+7);

				SetRect( &rcTexture, rcInner.right, rcInner.bottom, rect.right, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_pObj->m_objResource->SetElement(&TexElement, textureOffset+8);

				m_pObj->SetNeedUpdate();
				break;
			}
		default:
			{
				OUTPUT_LOG("warning: SetNineElementBG currently not supported for this type of UI control.\n");
				break;
			}
		}
	}
}
void ParaUIObject::SetFontString(const char* font)
{
	if (font==NULL||!IsValid()){
		return;
	}
	string temp=font;
	string fontname,fontsize,isbold;
	ParaEngine::StringHelper::DevideString(temp,fontname,fontsize);
	temp = fontsize;
	ParaEngine::StringHelper::DevideString(temp,fontsize,isbold);
	DWORD fsize;
	if(sscanf(fontsize.c_str(), "%d", &fsize)!=1){
		if (isbold.find("bold")!=string::npos) {
			SetFontString3(fontname.c_str(),11,true);
		}else
			SetFontString3(fontname.c_str(),11,false);

	}else{
		if (isbold.find("bold")!=string::npos) {
			SetFontString3(fontname.c_str(),fsize,true);
		}else
			SetFontString3(fontname.c_str(),fsize,false);
	}
}

void ParaUIObject::SetFontString1(const object& objfont)
{
	const char* font = NPL::NPLHelper::LuaObjectToString(objfont);
	SetFontString(font);
}

void ParaUIObject::SetFontString3(const char *fontname,DWORD fontsize,bool isbold)
{
	if (!IsValid()) return;
	char temp[30];
	ParaEngine::StringHelper::fast_itoa(fontsize,temp,30);
	string stemp=fontname;
	stemp+=";";
	stemp+=temp;
	stemp+=";";
	if (isbold) {
		stemp+="bold";
	}else
		stemp+="norm";
	GUIFontElement* pElement;
	SpriteFontEntity *pFontEntity=NULL;
	if ((pFontEntity=CGlobals::GetAssetManager()->GetFont(stemp.c_str()))==NULL) {
		pFontEntity=CGlobals::GetAssetManager()->LoadGDIFont(stemp,fontname,fontsize,isbold);
	}
	if (pFontEntity==NULL) {
		return;
	}
	m_pObj->m_objResource->SetActiveLayer("artwork");
	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIButton:
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Highlight);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Pressed);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Disabled);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState();
		break;
	case Type_GUIEditBox:
	case Type_GUIIMEEditBox:
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		pElement=m_pObj->m_objResource->GetFontElement(1);
		pElement->SetElement(pFontEntity, pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState();
		break;
	case Type_GUIText:
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState();
		break;
	case Type_GUIToolTip:
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState();
		break;
	case Type_GUIListBox:
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetFontElement(0);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		pElement=m_pObj->m_objResource->GetFontElement(1);
		pElement->SetElement(pFontEntity,pElement->FontColor,pElement->dwTextFormat);
		m_pObj->m_objResource->SetCurrentState();
		break;
	
	case Type_GUIContainer:
		break;
	default:
		OUTPUT_LOG("error: %s of type %s does not support property font.\n", m_pObj->GetName().c_str(), GetType().c_str());
	    break;
	}

	m_pObj->m_objResource->SetCurrentState();
	m_pObj->m_objResource->SetActiveLayer();
}
string ParaUIObject::GetBtnImage()const
{
	if (!IsValid()) return "";
	string strkey="";
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIButton)	{
		strkey=GenGetImage(0);
	}else if (m_pObj->GetType()->GetTypeValue()==Type_GUISlider) {
		strkey=GenGetImage(1);
	}else{
		OUTPUT_LOG("error: %s of type %s does not support button image.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
	return strkey;
}

void ParaUIObject::SetBtnImage1(const object& objButton)
{
	const char* szButton = NPL::NPLHelper::LuaObjectToString(objButton);
	if (szButton==NULL||!IsValid()) {
		return;
	}
	string temp=szButton;
	string filename,srect;
	ParaEngine::StringHelper::DevideString(temp,filename,srect);
	RECT rect;
	int width,height;
	if(sscanf(srect.c_str(), "%d %d %d %d", &rect.left,&rect.top,&width,&height)<4){
		SetBtnImage2(filename.c_str(),NULL);
	}else{
		rect.right=rect.left+width;
		rect.bottom=rect.top+height;
		SetBtnImage2(filename.c_str(),&rect);
	}
}

void ParaUIObject::SetBtnImage2(const char * TextFilename, RECT *rect)
{	
	TextureEntity* pTextureEntity=NULL;
	if ((pTextureEntity=CGlobals::GetAssetManager()->GetTexture(TextFilename))==NULL) {
		pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(TextFilename,TextFilename,TextureEntity::StaticTexture);
	}
	if (pTextureEntity==NULL) {
		return;
	}
	GUITextureElement* pElement;
	m_pObj->m_objResource->SetActiveLayer("artwork");
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIButton)	{
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetTextureElement(0);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Highlight);
		pElement=m_pObj->m_objResource->GetTextureElement(0);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Pressed);
		pElement=m_pObj->m_objResource->GetTextureElement(0);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Disabled);
		pElement=m_pObj->m_objResource->GetTextureElement(0);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState();
	}else if (m_pObj->GetType()->GetTypeValue()==Type_GUISlider) {
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Normal);
		pElement=m_pObj->m_objResource->GetTextureElement(1);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Highlight);
		pElement=m_pObj->m_objResource->GetTextureElement(1);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Pressed);
		pElement=m_pObj->m_objResource->GetTextureElement(1);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState(GUIResourceState_Disabled);
		pElement=m_pObj->m_objResource->GetTextureElement(1);
		pElement->SetElement(pTextureEntity,rect,pElement->TextureColor);
		m_pObj->m_objResource->SetCurrentState();
	}else{
		OUTPUT_LOG("error: %s of type %s does not support button image.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}

	m_pObj->m_objResource->SetCurrentState();
	m_pObj->m_objResource->SetActiveLayer();
}
string ParaUIObject::GetFontString()const
{
	if (!IsValid()) return "";
	string strkey="";
	int nObjType = m_pObj->GetType()->GetTypeValue();
	switch (nObjType)
	{
	case Type_GUIButton:
	case Type_GUIEditBox:
	case Type_GUIText:
	case Type_GUIListBox:
	case Type_GUIToolTip:
		strkey=GenGetFont(0);
		break;
	default:
		OUTPUT_LOG("error: %s of type %s does not support property font.\n", m_pObj->GetName().c_str(), GetType().c_str());
		break;
	}
	return strkey;
}


//only for internal use
string ParaUIObject::GenGetFont(int index)const
{
	GUIFontElement* element;
	string strkey="";
	element= m_pObj->m_objResource->GetFontElement(index);
	SpriteFontEntity* pFontEntity = element->GetFont();
	return pFontEntity->GetKey();
}

string ParaUIObject::GenGetImage(int index)const
{
	GUITextureElement* element;
	char temp[30];
	string strkey="";
	element= m_pObj->m_objResource->GetTextureElement(index);
	TextureEntity* pTextureEntity = element->GetTexture();
	strkey= ParaEngine::StringHelper::StrReplace(pTextureEntity->GetKey().c_str(),'\\','/');
	strkey+=";";
	ParaEngine::StringHelper::fast_itoa(element->rcTexture.left,temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(element->rcTexture.top,temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(element->rcTexture.right-element->rcTexture.left,temp,30);
	strkey+=temp;
	strkey+=" ";
	ParaEngine::StringHelper::fast_itoa(element->rcTexture.bottom-element->rcTexture.top,temp,30);
	strkey+=temp;
	return strkey;
}
string ParaUIObject::GetBGImage()const
{
	if (!IsValid()) return "";
	string strkey="";
	switch(m_pObj->GetType()->GetTypeValue()){
	case Type_GUIButton:
	case Type_GUIEditBox:
	case Type_GUIIMEEditBox:
	case Type_GUIText:
	case Type_GUISlider:
	case Type_GUIListBox:
	case Type_GUIContainer:
	case Type_GUIToolTip:
		strkey=GenGetImage(0);
		break;
	default:
		OUTPUT_LOG("warning: %s of type %s does not support property background image.\n", m_pObj->GetName().c_str(), GetType().c_str());
	    break;
	}
	return strkey;
}
bool ParaUIObject::GetReadOnly()const
{
	if (!IsValid()) return false;
	if (m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox||(m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) {
		return ((CGUIEditBox*)m_pObj)->GetReadOnly();
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property readonly.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
	return false;
}

void ParaUIObject::SetReadOnly(bool readonly)
{
	if (!IsValid()) return;
	if (m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox||(m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) {
		((CGUIEditBox*)m_pObj)->SetReadOnly(readonly);
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property readonly.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}

}

bool ParaUIObject::IsModified()const
{
	if (!IsValid()) return false;
	if (m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox||(m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) {
		return ((CGUIEditBox*)m_pObj)->IsModified();
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property Ismodified.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
	return false;
}

void ParaUIObject::SetScrollable(bool bScrollable)
{
	if (!IsValid()) return;
	if(((CGUIType*)m_pObj->GetType())->IsContainer())
	{
		((CGUIContainer*)m_pObj)->SetScrollable(bScrollable);
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property scrollable.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
}

bool ParaUIObject::GetScrollable() const
{
	if (!IsValid()) return false;
	if(((CGUIType*)m_pObj->GetType())->IsContainer())
	{
		return ((CGUIContainer*)m_pObj)->IsScrollable();
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property scrollable.\n", m_pObj->GetName().c_str(), GetType().c_str());
		return false;
	}

}


void ParaUIObject::SetPageSize(int pagesize)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar){
		((CGUIScrollBar*)m_pObj)->SetPageSize(pagesize);
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property SetPageSize.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
}

void ParaUIObject::SetTrackRange(int nBegin,int nEnd)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar){
		((CGUIScrollBar*)m_pObj)->SetTrackRange(nBegin,nEnd);
	}else if(m_pObj->GetType()->GetTypeValue()==Type_GUISlider){
			((CGUISlider*)m_pObj)->SetRange(nBegin,nEnd);
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property SetTrackRange.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
}


void ParaUIObject::SetStep( int nDelta )
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar){
		((CGUIScrollBar*)m_pObj)->SetStep(nDelta);
	}
}

int ParaUIObject::GetStep()
{
	if (!IsValid()) return 0;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar){
		return ((CGUIScrollBar*)m_pObj)->GetStep();
	}
	return 0;
}

void ParaUIObject::GetTrackRange(int &nBegin, int &nEnd)
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar){
		nBegin=((CGUIScrollBar*)m_pObj)->GetTrackStart();
		nEnd=((CGUIScrollBar*)m_pObj)->GetTrackEnd();
	}else if(m_pObj->GetType()->GetTypeValue()==Type_GUISlider){
		((CGUISlider*)m_pObj)->GetRange(nBegin,nEnd);
	}else{
		OUTPUT_LOG("warning: %s of type %s does not support property GetTrackRange.\n", m_pObj->GetName().c_str(), GetType().c_str());
	}
}
void ParaUIObject::OnClick(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_CLICK, strScriptName);
}

void ParaUIObject::OnChange(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_CHANGE, strScriptName);
}

void ParaUIObject::OnKeyDown(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_KEYDOWN, strScriptName);
}
void ParaUIObject::OnKeyUp(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_KEYUP, strScriptName);
}

void ParaUIObject::OnDragBegin(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_DRAGBEGIN, strScriptName);
}

void ParaUIObject::OnDragEnd(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_DRAGEND, strScriptName);
}

void ParaUIObject::OnDragOver(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_DRAGOVER, strScriptName);
}
void ParaUIObject::OnMouseMove(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_MOVE, strScriptName);
}

void ParaUIObject::OnDoubleClick(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_DBCLICK, strScriptName);
}

void ParaUIObject::OnMouseDown(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_DOWN, strScriptName);
}
void ParaUIObject::OnMouseUp(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_UP, strScriptName);
}
void ParaUIObject::OnMouseWheel(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_WHEEL, strScriptName);
}
void ParaUIObject::OnMouseHover(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_HOVER, strScriptName);
}
void ParaUIObject::OnMouseEnter(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_ENTER, strScriptName);
}
void ParaUIObject::OnMouseLeave(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_MOUSE_LEAVE, strScriptName);
}

void ParaUIObject::OnSelect(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_SELECT, strScriptName);
}

void ParaUIObject::OnSize( const object& objScriptName )
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_WM_SIZE, strScriptName);
}

void ParaUIObject::OnDestroy( const object& objScriptName )
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_WM_DESTROY, strScriptName);
}

void ParaUIObject::OnActivate( const object& objScriptName )
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_WM_ACTIVATE, strScriptName);
}

void ParaUIObject::OnFrameMove(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_FRAMEMOVE, strScriptName);
}

void ParaUIObject::SetEventScript(DWORD type, const char * strScriptName)
{
	if (IsValid())
	{
		if (strScriptName==NULL || strScriptName[0]=='\0') 
		{
			m_pObj->SetEventScript(type,NULL);
		}
		else
		{
			string temp(strScriptName);
			SimpleScript script;
			ParaEngine::StringHelper::DevideString(temp,script.szFile,script.szCode);
			m_pObj->SetEventScript(type,&script);
		}
	}
}


string ParaUIObject::GetOnActivate() const
{
	string code;GetEventScript(EM_WM_ACTIVATE, code);return code;
}


std::string ParaUIObject::GetOnSize() const
{
	string code;GetEventScript(EM_WM_SIZE, code);return code;
}

std::string ParaUIObject::GetOnDestroy() const
{
	string code;GetEventScript(EM_WM_DESTROY, code);return code;
}

void ParaUIObject::OnModify(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_MODIFY, strScriptName);
}


string ParaUIObject::GetOnModify()const
{
	string code;GetEventScript(EM_CTRL_MODIFY, code);return code;
}
string ParaUIObject::GetOnSelect()const
{
	string code;GetEventScript(EM_CTRL_SELECT, code);return code;
}


void ParaUIObject::OnFocusIn(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_FOCUSIN, strScriptName);
}

std::string ParaUIObject::GetOnFocusIn() const
{
	string code; GetEventScript(EM_CTRL_FOCUSIN, code); return code;
}

void ParaUIObject::OnFocusOut(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_CTRL_FOCUSOUT, strScriptName);
}

std::string ParaUIObject::GetOnFocusOut() const
{
	string code; GetEventScript(EM_CTRL_FOCUSOUT, code); return code;
}


string ParaUIObject::GetOnClick()const
{
	string code;GetEventScript(EM_MOUSE_CLICK, code);return code;
}

string ParaUIObject::GetOnDoubleClick()const
{
	string code;GetEventScript(EM_MOUSE_DBCLICK, code);return code;
}

string ParaUIObject::GetOnKeyDown()const
{
	string code;GetEventScript(EM_CTRL_KEYDOWN, code);return code;
}

string ParaUIObject::GetOnKeyUp()const
{
	string code;GetEventScript(EM_CTRL_KEYUP, code);return code;
}

string ParaUIObject::GetOnDragBegin()const
{
	string code;GetEventScript(EM_MOUSE_DRAGBEGIN, code);return code;
}

string ParaUIObject::GetOnDragEnd()const
{
	string code;GetEventScript(EM_MOUSE_DRAGEND, code);return code;
}

string ParaUIObject::GetOnDragOver()const
{
	string code;GetEventScript(EM_MOUSE_DRAGOVER, code);return code;
}

string ParaUIObject::GetOnMouseMove()const
{
	string code;GetEventScript(EM_MOUSE_MOVE, code);return code;
}

string ParaUIObject::GetOnMouseHover()const
{
	string code;GetEventScript(EM_MOUSE_HOVER, code);return code;
}
string ParaUIObject::GetOnMouseEnter()const
{
	string code;GetEventScript(EM_MOUSE_ENTER, code);return code;
}
string ParaUIObject::GetOnMouseLeave()const
{
	string code;GetEventScript(EM_MOUSE_LEAVE, code);return code;
}
string ParaUIObject::GetOnMouseDown()const
{
	string code;GetEventScript(EM_MOUSE_DOWN, code);return code;
}
string ParaUIObject::GetOnMouseUp()const
{
	string code;GetEventScript(EM_MOUSE_UP, code);return code;
}
string ParaUIObject::GetOnMouseWheel()const
{
	string code;GetEventScript(EM_MOUSE_WHEEL, code);return code;
}
string ParaUIObject::GetOnFrameMove()const
{
	string code;GetEventScript(EM_CTRL_FRAMEMOVE, code);return code;
}

void ParaUIObject::OnTouch(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_TOUCH, strScriptName);
}

std::string ParaUIObject::GetOnTouch() const
{
	string code; GetEventScript(EM_TOUCH, code); return code;
}


void ParaUIObject::OnDraw(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_WM_OWNER_DRAW, strScriptName);
}

std::string ParaUIObject::GetOnDraw() const
{
	string code; GetEventScript(EM_WM_OWNER_DRAW, code); return code;
}


void ParaUIObject::OnInputMethod(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	SetEventScript(EM_WM_INPUT_METHOD, strScriptName);
}

std::string ParaUIObject::GetOnInputMethod() const
{
	string code; GetEventScript(EM_WM_INPUT_METHOD, code); return code;
}

void ParaUIObject::GetEventScript(DWORD type, string& code)const
{
	if (IsValid())
	{
		const SimpleScript* script=m_pObj->GetEventScript(type);
		if (script) {
			code+=script->szFile;
			if (script->szCode.size()>0) {
				code+=(";"+script->szCode);
			}
		}
	}
}

string ParaUIObject::GetOnChange()const
{
	string code;GetEventScript(EM_CTRL_CHANGE, code);return code;
}

void ParaUIObject::SetCanDrag(bool bCanDrag)
{
	//if ((this->m_pObj->GetType()&CGUIBase::COMMON_CONTROL_MASK)==CGUIBase::GUIButton) {
	//	((CGUIButton*)m_pObj)->m_bCanDrag=bCanDrag;
	//}
	//if ((this->m_pObj->GetType()&CGUIBase::GUIContainer)!=0) {
	//	((CGUIContainer*)m_pObj)->m_bCanDrag=bCanDrag;
	//}
	if (!IsValid()) return;
	m_pObj->SetCandrag(bCanDrag);
}

bool ParaUIObject::GetCanDrag() const
{
	if (!IsValid()) return false;
	return m_pObj->GetCandrag();
}
void ParaUIObject::SetName(const char *szName)
{
	if (!IsValid()) return;
	m_pObj->SetName(szName);
}

void ParaUIObject::SetToolTip(const object&  objtooltip)
{
	const char* tooltip = NPL::NPLHelper::LuaObjectToString(objtooltip);
	if (tooltip==NULL||!IsValid()) {
		return;
	}
	CGUIRoot::GetInstance()->m_tooltip->SetToolTipA(m_pObj,tooltip);
}
string ParaUIObject::GetToolTip()const
{
	std::string returnStr="";
	if (!IsValid()) {
		return returnStr;
	}
	CGUIRoot::GetInstance()->m_tooltip->GetToolTipA(m_pObj, returnStr);
	return returnStr;
}
string ParaUIObject::GetText() const
{
	string returnStr;
	if (IsValid())
	{
		switch(m_pObj->GetType()->GetTypeValue())
		{
		case Type_GUIButton:
		case Type_GUIEditBox:
		case Type_GUIIMEEditBox:
		case Type_GUIListBox:
		case Type_GUIWebBrowser:
		case Type_GUIText:
			{
				m_pObj->GetTextA(returnStr);
				break;
			}
		default:
			OUTPUT_LOG("warning: %s of type %s does not support property text.\n", m_pObj->GetName().c_str(), GetType().c_str());
			break;
		}
	}
	return returnStr;
}

void ParaUIObject::RemoveAll()
{
	if (!IsValid()) return ;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) 
	{
		((CGUIListBox*)m_pObj)->RemoveAll();
	}
	else if (((CGUIType*)m_pObj->GetType())->IsContainer())
	{
		((CGUIContainer*)m_pObj)->DestroyChildren();
	}
}
void ParaUIObject::SetX(int x)
{
	if (!IsValid()) return;
	m_pObj->SetX(x);
}

void ParaUIObject::SetY(int y)
{
	if (!IsValid()) return;
	m_pObj->SetY(y);
}

int ParaUIObject::GetX()const
{
	if (!IsValid()) return 0;
	return m_pObj->GetX();
}

int ParaUIObject::GetY()const
{
	if (!IsValid()) return 0;
	return m_pObj->GetY();
}

float ParaUIObject::GetDepth()const
{
	if (!IsValid()) return 0;
	return m_pObj->GetPosition()->GetDepth();
}

void ParaUIObject::SetDepth(float depth)
{
	if (!IsValid()) return;
	m_pObj->GetPosition()->SetDepth(depth);
}
void  ParaUIObject::SetWidth(int width)
{
	if (!IsValid()) return;
	m_pObj->SetWidth(width);
}

void  ParaUIObject::SetHeight(int height)
{
	if (!IsValid()) return;
	m_pObj->SetHeight(height);
}

int  ParaUIObject::Width()const
{
	if (!IsValid()) return 0;
	return m_pObj->GetWidth();
}

int  ParaUIObject::Height()const
{
	if (!IsValid()) return 0;
	return m_pObj->GetHeight();
}

void ParaUIObject::SetScrollbarWidth(int width)
{
	if (!IsValid()) return ;
	if (width<1){
		OUTPUT_LOG("Scrollbar Width should >0.\r\n");
		return;
	}
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) {
		((CGUIContainer*)m_pObj)->SetScrollbarWidth(width);
	}else if ((m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)){
		((CGUIScrollBar*)m_pObj)->SetScrollbarWidth(width);
	}
}

int ParaUIObject::GetScrollbarWidth()const
{
	if (!IsValid()) return 0;

	if (((CGUIType*)m_pObj->GetType())->IsContainer()) {
		return ((CGUIContainer*)m_pObj)->GetScrollbarWidth();
	}else if ((m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)){
		return ((CGUIScrollBar*)m_pObj)->GetScrollbarWidth();
	}
	return 0;
}
string ParaUIObject::ToScript()const
{
	if (!IsValid()) return "";
	return m_pObj->ToScript();
}


ParaUIFont ParaUIObject::CreateFont(const char *name,const char *fontname)
{
	if (!IsValid()) return ParaUIFont(NULL);
	GUIFontElement font_;
	GUIFontElement *pFontElement=&font_;
	pFontElement->SetElement(CGlobals::GetAssetManager()->GetFont(fontname),0xff000000,DT_LEFT|DT_TOP);
	
	m_pObj->m_objResource->AddElement(pFontElement,name);

	return ParaUIFont(m_pObj->m_objResource->GetFontElement((int)m_pObj->m_objResource->FontSize()-1));

}

ParaUITexture ParaUIObject::CreateTexture(const char *name,const char *texturename)
{
 	if (!IsValid()) return ParaUITexture(NULL);
	GUITextureElement tex_;
	GUITextureElement *pElement=&tex_;
	
	string filename;
	RECT *prect=NULL;
	RECT rect;
	prect=ParaEngine::StringHelper::GetImageAndRect(texturename,filename, &rect);

	TextureEntity* pTextureEntity=NULL;
	if ((pTextureEntity=CGlobals::GetAssetManager()->GetTexture(filename))==NULL)
	{
		pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(filename,filename,TextureEntity::StaticTexture);
	}
	else
	{	
	}
	pElement->SetElement(pTextureEntity,prect,0xffffffff);
	
	return ParaUITexture(m_pObj->m_objResource->AddElement(pElement,name));
}

ParaUIFont ParaUIObject::GetFont(const char *name)
{
	if (!IsValid()) return ParaUIFont(NULL);
	return ParaUIFont(m_pObj->GetFontElement(name));
}

ParaUIFont ParaUIObject::GetFont_(int nIndex)
{
	if (!IsValid()) return ParaUIFont(NULL);
	return ParaUIFont(m_pObj->GetFontElement(nIndex));
}

ParaUITexture ParaUIObject::GetTexture(const object& oName)
{
	const char* name= NPL::NPLHelper::LuaObjectToString(oName);
	if (!IsValid()) return ParaUITexture(NULL);
	if(name != NULL)
		return ParaUITexture(m_pObj->GetTextureElement(name));
	else
		return ParaUITexture(m_pObj->m_objResource->GetTextureElement(0));
}

string ParaUIObject::GetColor()const
{
	GUITextureElement* pObj = m_pObj->m_objResource->GetTextureElement(0);
	if (pObj) {
		Color color(pObj->TextureColor);
		return color.ToRGBString();
	}
	return "";
}

string ParaUIObject::GetColorMask() const
{
	if (m_pObj) {
		Color color(m_pObj->GetColorMask());
		return color.ToRGBAString();
	}
	return "";
}
void ParaUIObject::SetColor(const char *strColor)
{
	if (m_pObj) {
		Color color = Color::FromString(strColor);
		
		int nSize = (int)m_pObj->m_objResource->TextureSize();
		for (int i=0;i<nSize;++i)
		{
			GUITextureElement* pObj = m_pObj->m_objResource->GetTextureElement(i);
			pObj->TextureColor = color;
		}
		m_pObj->SetDirty(true);
	}
}

void ParaUIObject::SetColorMask( const char *strColor )
{
	if (m_pObj) {
		Color color = Color::FromString(strColor);
		m_pObj->SetColorMask(color);
	}
}
void ParaUIObject::CloneState(const char* statename)
{
	if (!IsValid()) return;
	m_pObj->m_objResource->CloneState(statename);
}
void ParaUIObject::SetActiveLayer(const char *layer)
{
	if (!IsValid()) return;
	m_pObj->m_objResource->SetActiveLayer(layer);
}

bool ParaUIObject::HasLayer( const char *layer )
{
	if (!IsValid()) return false;
	return m_pObj->m_objResource->HasLayer(layer);
}

void ParaUIObject::SetCurrentState(const char *statename)
{
	if (!IsValid()) return;
	m_pObj->m_objResource->SetCurrentState(statename);
}
void ParaUIObject::SetReceiveDrag(bool bReceiveDrag)
{
	if (!IsValid()) return;
	m_pObj->SetReceiveDrag(bReceiveDrag);
}
bool ParaUIObject::GetReceiveDrag()const
{
	if (!IsValid()) return false;
	return m_pObj->GetReceiveDrag();
}

void ParaUIObject::SetValue(int value)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUISlider)) {
		((CGUISlider*)m_pObj)->SetValue(value);
	}
	else if ((m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)) 
	{
		((CGUIScrollBar*)m_pObj)->SetValue(value);
	}
	else if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) 
	{
		((CGUIListBox*)m_pObj)->SelectItem(value);
	}
}

int ParaUIObject::GetValue()const
{
	if (!IsValid()) return 0;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUISlider)) {
		return ((CGUISlider*)m_pObj)->GetValue();
	}
	else if ((m_pObj->GetType()->GetTypeValue()==Type_GUIScrollBar)) {
		return ((CGUIScrollBar*)m_pObj)->GetValue();
	}
	return 0;
}

const char* ParaUIObject::GetHighlightStyle()const
{
	if (!IsValid()) return "none";
	int style=((CGUIBase*)m_pObj)->GetHighlightStyle();
	CGUIHighlightManager* gm=&CSingleton<CGUIHighlightManager>::Instance();
	CGUIHighlight* ph=gm->GetHighlight(style);
	if (ph){
		return ph->GetHighlightName();
	}
	return "none";
}

void ParaUIObject::SetHighlightStyle(const char* style)
{
	if (!IsValid()) return ;
	if (strcmp(style,"none")==0){
		((CGUIBase*)m_pObj)->SetHighlightStyle(HighlightNone);
	}else {

		CGUIHighlightManager* gm=&CSingleton<CGUIHighlightManager>::Instance();
		CGUIHighlight* ph=gm->GetHighlight(style);
		if (ph){
			((CGUIBase*)m_pObj)->SetHighlightStyle(ph->GetHighlightStyle());
		}
	}
}

ParaUIObject ParaUIObject::GetChild(const char *name)
{
	if (!IsValid()||name==NULL) return ParaUIObject(NULL) ;

	CGUIBase *temp=NULL;
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) {
		GUIBase_List_Type::iterator iter,iterend;
		iterend=((CGUIContainer*)m_pObj)->GetChildren()->end();
		for( iter = ((CGUIContainer*)m_pObj)->GetChildren()->begin(); iter != iterend;iter++ ){
			if ((*iter)->GetName().compare(name)==0) {
				temp=*iter;
				break;
			}
			
		}
		if(temp == NULL)
		{
			if (((CGUIContainer*)m_pObj)->GetScrollBar(0)->GetName().compare(name)==0) {
				temp=((CGUIContainer*)m_pObj)->GetScrollBar(0);
			}
			else if (((CGUIContainer*)m_pObj)->GetScrollBar(1)->GetName().compare(name)==0) {
				temp=((CGUIContainer*)m_pObj)->GetScrollBar(1);
			}
		}
	}
	return ParaUIObject(temp);
}

ParaUIObject ParaUIObject::GetChildAt(int index)
{
	if (!IsValid()||index<0) return ParaUIObject(NULL) ;

	CGUIBase *temp=NULL;
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) 
	{
		CGUIContainer* pParent = (CGUIContainer*)m_pObj;
		if (index < (int)pParent->GetChildren()->size())
		{
			temp = (*(pParent->GetChildren()))[index];
		}
	}
	return ParaUIObject(temp);
}

int ParaUIObject::GetChildCount()
{
	if (!IsValid()) 
		return 0;
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) 
	{
		return (int)(((CGUIContainer*)m_pObj)->GetChildren()->size());
	}
	return 0;
}

void ParaUIObject::SetPopUp(int popup)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIContainer)) {
		((CGUIContainer*)m_pObj)->SetPopUp(popup);
	}
}
int ParaUIObject::GetPopUp()const
{
	if (!IsValid()) return false;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIContainer)) {
		return ((CGUIContainer*)m_pObj)->GetPopUp();
	}
	return false;
}

void ParaUIObject::Focus()
{
	if (m_pObj){
		m_pObj->Focus();
	}
}

void ParaUIObject::SetWordbreak(bool wordbreak)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		((CGUIListBox*)m_pObj)->SetWordBreak(wordbreak);
	}
}

bool ParaUIObject::GetWordbreak()const
{
	if (!IsValid()) return false;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		return ((CGUIListBox*)m_pObj)->GetWordBreak();
	}
	return false;
}

void ParaUIObject::SetItemHeight(int itemheight)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		((CGUIListBox*)m_pObj)->SetItemHeight(itemheight);
	}
}

int ParaUIObject::GetItemHeight()const
{
	if (!IsValid()) return -1;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		return ((CGUIListBox*)m_pObj)->GetItemHeight();
	}
	return -1;
}

void ParaUIObject::ActivateScript(const char *scripttype)
{
	if (scripttype==NULL||!IsValid()) return;
	if(strcmp(scripttype, "ondragbegin")==0)
	{
		m_pObj->ActivateScript("",EM_MOUSE_DRAGBEGIN);
	}else if(strcmp(scripttype, "ondragend")==0){
		m_pObj->ActivateScript("",EM_MOUSE_DRAGBEGIN);
	}else if(strcmp(scripttype, "onchange")==0){
		m_pObj->ActivateScript("",EM_CTRL_MODIFY);
	}else if(strcmp(scripttype, "onchar")==0){
		m_pObj->ActivateScript("",CGUIScript::ONCHAR);
	}else if(strcmp(scripttype, "onclick")==0){
		m_pObj->ActivateScript("",EM_MOUSE_CLICK);
	}else if(strcmp(scripttype, "ondoubleclick")==0){
		m_pObj->ActivateScript("",EM_MOUSE_DBCLICK);
	}else if(strcmp(scripttype, "ondragover")==0){
		m_pObj->ActivateScript("",EM_MOUSE_DRAGOVER);
	}else if(strcmp(scripttype, "onevent")==0){
		m_pObj->ActivateScript("",CGUIScript::ONEVENT);
	}else if(strcmp(scripttype, "onfocusin")==0){
		m_pObj->ActivateScript("",CGUIScript::ONFOCUSIN);
	}else if(strcmp(scripttype, "onfocusout")==0){
		m_pObj->ActivateScript("",CGUIScript::ONFOCUSOUT);
	}else if(strcmp(scripttype, "onkeydown")==0){
		m_pObj->ActivateScript("",EM_CTRL_KEYDOWN);
	}else if(strcmp(scripttype, "onkeyup")==0){
		m_pObj->ActivateScript("",EM_CTRL_KEYUP);
	}else if(strcmp(scripttype, "onload")==0){
		m_pObj->ActivateScript("",CGUIScript::ONLOAD);
	}else if(strcmp(scripttype, "onmousedown")==0){
		m_pObj->ActivateScript("",EM_MOUSE_DOWN);
	}else if(strcmp(scripttype, "onmouseenter")==0){
		m_pObj->ActivateScript("",EM_MOUSE_ENTER);
	}else if(strcmp(scripttype, "onmousehover")==0){
		m_pObj->ActivateScript("",EM_MOUSE_HOVER);
	}else if(strcmp(scripttype, "onmouseleave")==0){
		m_pObj->ActivateScript("",EM_MOUSE_LEAVE);
	}else if(strcmp(scripttype, "onmousemove")==0){
		m_pObj->ActivateScript("",EM_MOUSE_MOVE);
	}else if(strcmp(scripttype, "onmouseup")==0){
		m_pObj->ActivateScript("",EM_MOUSE_UP);
	}else if(strcmp(scripttype, "onmousewheel")==0){
		m_pObj->ActivateScript("",EM_MOUSE_WHEEL);
	}else if(strcmp(scripttype, "onstring")==0){
		m_pObj->ActivateScript("",EM_CTRL_CHANGE);
	}else{
		OUTPUT_LOG("warning: %s of %s of type %s is not a valid event type.\n", scripttype, m_pObj->GetName().c_str(), GetType().c_str());
	}
}

void ParaUIObject::AddTextItem(const char*text)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		((CGUIListBox*)m_pObj)->AddTextItemA(text);
	}
}

void ParaUIObject::LostFocus()
{
	if (IsValid())
		m_pObj->LostFocus();
}
void ParaUIObject::InvalidateRect()
{
	if (IsValid()) 
		m_pObj->InvalidateRect(NULL);
}

void ParaUIObject::UpdateRect()
{
	if (IsValid()) 
		m_pObj->UpdateRects();
}

void ParaUIObject::SetFastRender(bool fastrender)
{
	if (!IsValid()) return;
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) {
		((CGUIContainer*)m_pObj)->SetFastRender(fastrender);
	}
}

bool ParaUIObject::GetFastRender()const
{
	if (!IsValid()) return false;
	if (((CGUIType*)m_pObj->GetType())->IsContainer()) {
		return ((CGUIContainer*)m_pObj)->GetFastRender();
	}
	return false;
}

void ParaUIObject::RemoveItem(int index)
{
	if (!IsValid()) return;
	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIListBox)) {
		((CGUIListBox*)m_pObj)->DeleteItem(index);
	}
}

int ParaUIObject::GetAnimationStyle() const 
{
	if (IsValid()) 
		return m_pObj->GetAnimationStyle();
	else
		return 0;
}

void ParaUIObject::SetAnimationStyle( int nStyle ) 
{
	if (IsValid()) 
		m_pObj->SetAnimationStyle(nStyle);
}

void ParaUIObject::DoAutoSize()
{
	if (IsValid()) 
		m_pObj->DoAutoSize();
}

void ParaUIObject::GetTextLineSize(int* width, int* height)
{
	if (!IsValid()) 
		return;
	int nType = m_pObj->GetType()->GetTypeValue();
	if (nType == Type_GUIText)
	{
		((CGUIText*)m_pObj)->GetTextLineSize(width, height);
	}
	else if (nType == Type_GUIEditBox)
	{
		((CGUIEditBox*)m_pObj)->GetTextLineSize(width, height);
	}
	else
	{
		OUTPUT_LOG("warning: method GetTextLineSize() is only valid on text control.\r\n");
	}
}


int ParaUIObject::GetFirstVisibleCharIndex()
{
	if (!IsValid()) return -1;

	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
	{
		return ((CGUIEditBox*)m_pObj)->GetFirstVisibleCharIndex();
	}
	else
	{
		OUTPUT_LOG("warning: method GetFirstVisibleCharIndex() is only valid on edit box control.\r\n");
		return -1;
	}
}

int ParaUIObject::GetCaretPosition()
{
	if (!IsValid()) return -1;

	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
	{
		return ((CGUIEditBox*)m_pObj)->GetCaretPosition();
	}
	else
	{
		OUTPUT_LOG("warning: method GetCaretPosition() is only valid on edit box control.\r\n");
		return -1;
	}
}

void ParaUIObject::SetCaretPosition( int nCharacterPos )
{
	if (!IsValid()) return;

	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
	{
		((CGUIEditBox*)m_pObj)->SetCaretPosition(nCharacterPos);
	}
	else
	{
		OUTPUT_LOG("warning: method SetCaretPosition() is only valid on edit box control.\r\n");
	}
}

int ParaUIObject::GetTextSize()
{
	if (!IsValid()) return -1;

	if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
	{
		return ((CGUIEditBox*)m_pObj)->GetTextSize();
	}
	else
	{
		OUTPUT_LOG("warning: method GetTextSize() is only valid on edit box control.\r\n");
		return -1;
	}
}

void ParaUIObject::GetPriorWordPos( int nCP, int PriorIn, int &Prior )
{
	if (IsValid())
	{
		if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
		{
			((CGUIEditBox*)m_pObj)->GetPriorWordPos(nCP, &PriorIn);
		}
		else
		{
			OUTPUT_LOG("warning: method GetPriorWordPos() is only valid on edit box control.\r\n");
		}
	}
	Prior = PriorIn;
}

void ParaUIObject::GetNextWordPos( int nCP, int NextIn, int &Next )
{
	if (IsValid())
	{
		if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
		{
			((CGUIEditBox*)m_pObj)->GetNextWordPos(nCP, &NextIn);
		}
		else
		{
			OUTPUT_LOG("warning: method GetNextWordPos() is only valid on edit box control.\r\n");
		}
	}
	Next = NextIn;
}

void ParaUIObject::CPtoXY( int nCP, bool bTrail, int XIn, int YIn, int &X, int &Y )
{
	if (IsValid())
	{
		if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
		{
			((CGUIEditBox*)m_pObj)->CPtoXY(nCP, bTrail, &XIn, &YIn);
		}
		else
		{
			OUTPUT_LOG("warning: method CPtoXY() is only valid on edit box control.\r\n");
		}
	}
	X = XIn;
	Y = YIn;
}

void ParaUIObject::XYtoCP( int nX, int nY, int CPIn, int nTrailIn, int & CP,int &nTrail)
{
	if (IsValid())
	{
		if ((m_pObj->GetType()->GetTypeValue()==Type_GUIEditBox) || (m_pObj->GetType()->GetTypeValue()==Type_GUIIMEEditBox)) 
		{
			((CGUIEditBox*)m_pObj)->XYtoCP(nX, nY, &CPIn, &nTrailIn);
		}
		else
		{
			OUTPUT_LOG("warning: method XYtoCP() is only valid on edit box control.\r\n");
		}
	}
	CP = CPIn;
	nTrail = nTrailIn;
}

void ParaUIObject::SetRotation( float fRot )
{
	if (!IsValid()) return;
	m_pObj->SetRotation(fRot);
}

float ParaUIObject::GetRotation() const
{
	if (!IsValid()) return 0.f;
	return m_pObj->GetRotation();
}

void ParaUIObject::SetRotOriginOffset( float x, float y )
{
	if (!IsValid()) return;
	m_pObj->SetRotOriginOffset(Vector2(x,y));
}

void ParaUIObject::GetRotOriginOffset( float *x, float * y ) const
{
	if (!IsValid()) return;
	Vector2 vOffset(0,0);
	m_pObj->GetRotOriginOffset(&vOffset);
	if(x)
		*x = vOffset.x;
	if(y)
		*y = vOffset.y;
}

void ParaUIObject::SetScaling( float x, float y )
{
	if (!IsValid()) return;
	m_pObj->SetScaling(Vector2(x,y));
}

void ParaUIObject::GetScaling( float *x, float * y ) const
{
	if (!IsValid()) return;
	Vector2 vOffset(0,0);
	m_pObj->GetScaling(&vOffset);
	if(x)
		*x = vOffset.x;
	if(y)
		*y = vOffset.y;
}

void ParaUIObject::SetScalingX( float x )
{
	Vector2 v;
	GetScaling(&v.x, &v.y);
	v.x = x;
	SetScaling(v.x, v.y);
}

float ParaUIObject::GetScalingX() const
{
	Vector2 v;
	GetScaling(&v.x, &v.y);
	return v.x;
}

void ParaUIObject::SetScalingY( float y )
{
	Vector2 v;
	GetScaling(&v.x, &v.y);
	v.y = y;
	SetScaling(v.x, v.y);
}

float ParaUIObject::GetScalingY() const
{
	Vector2 v;
	GetScaling(&v.x, &v.y);
	return v.y;
}

void ParaUIObject::SetTranslation( float x, float y )
{
	if (!IsValid()) return;
	m_pObj->SetTranslation(Vector2(x, y));
}

void ParaUIObject::GetTranslation( float *x, float * y ) const
{
	if (!IsValid()) return;
	Vector2 vOffset(0, 0);
	m_pObj->GetTranslation(&vOffset);
	if(x)
		*x = vOffset.x;
	if(y)
		*y = vOffset.y;
}

void ParaUIObject::SetTranslationX( float x )
{
	Vector2 v;
	GetTranslation(&v.x, &v.y);
	v.x = x;
	SetTranslation(v.x, v.y);
}

float ParaUIObject::GetTranslationX() const
{
	Vector2 v;
	GetTranslation(&v.x, &v.y);
	return v.x;
}

void ParaUIObject::SetTranslationY( float y )
{
	Vector2 v;
	GetTranslation(&v.x, &v.y);
	v.y = y;
	SetTranslation(v.x, v.y);
}

float ParaUIObject::GetTranslationY() const
{
	Vector2 v;
	GetTranslation(&v.x, &v.y);
	return v.y;
}

void ParaUIObject::SetSpacing( int nSpacing )
{
	if(m_pObj)
		m_pObj->SetSpacing(nSpacing);
}

int ParaUIObject::GetSpacing() const
{
	if(m_pObj)
		return m_pObj->GetSpacing();
	else
		return 0;
}

void ParaUIObject::SetSize( int width, int height )
{
	if(m_pObj)
		return m_pObj->SetSize(width, height);
}

void ParaUIObject::SetUseTextShadow( bool bUseTextShadow )
{
	if(m_pObj)
		m_pObj->SetUseTextShadow(bUseTextShadow);
}

bool ParaUIObject::GetUseTextShadow() const
{
	if(m_pObj)
		return m_pObj->GetUseTextShadow();
	else
		return false;
}

void ParaUIObject::SetTextScale(float fScale)
{
	if(m_pObj)
		m_pObj->SetTextScale(fScale);
}

float ParaUIObject::GetTextScale()
{
	if(m_pObj)
		return m_pObj->GetTextScale();
	else
		return 1.f;
}

int ParaUIObject::GetID() const
{
	return (m_pObj) ? m_pObj->GetID() : -1;
}

void ParaUIObject::SetID( int nID )
{
	if(m_pObj) 
		m_pObj->SetID(nID);
}

ParaScripting::ParaUIObject ParaUIObject::GetChildByID( int nChildID )
{
	return ParaUIObject((m_pObj) ? m_pObj->GetChildByID(nChildID) : NULL);
}

ParaScripting::ParaUIObject ParaUIObject::GetChildByName( const char* name )
{
	return ParaUIObject((m_pObj && name) ? m_pObj->GetChildByName(name) : NULL);
}

void ParaUIObject::SetDefault( bool bDefaultButton )
{
	if (!IsValid()) return;
	if(m_pObj->GetType()->GetTypeValue() == Type_GUIButton){
		((CGUIButton*)m_pObj)->SetDefaultButton(bDefaultButton);
	}
}

void ParaUIObject::Reposition( const char* alignment, int left,int top, int width, int height )
{
	if (m_pObj)
	{
		m_pObj->Reposition(alignment, left, top, width, height);
		m_pObj->UpdateRects();
		m_pObj->UpdateParentRect();
	}
}

int ParaUIObject::GetZOrder() const
{
	if (m_pObj)
	{
		return m_pObj->GetZOrder();
	}
	return 0;
}

void ParaUIObject::SetZOrder( int nOrder )
{
	if (m_pObj)
	{
		m_pObj->SetZOrder(nOrder);
	}
}

void ParaUIObject::SetCursor( const object&  objCursorFile )
{
	if (m_pObj)
	{
		const char* szCursorFile = NPL::NPLHelper::LuaObjectToString(objCursorFile);
		m_pObj->SetCursor(szCursorFile);
	}
}

void ParaUIObject::SetCursorEx( const char* szCursorFile, int nHotSpotX, int nHotSpotY )
{
	if (m_pObj)
	{
		m_pObj->SetCursor(szCursorFile,nHotSpotX,nHotSpotY);
	}
}

string ParaUIObject::GetCursor() const
{
	return (m_pObj) ? m_pObj->GetCursor() : "";
}

ParaAttributeObject ParaUIObject::GetAttributeObject()
{
	return ParaAttributeObject((IAttributeFields*)m_pObj);
}

void ParaUIObject::GetAttributeObject_(ParaAttributeObject& output)
{
	output = GetAttributeObject();
}

void ParaUIObject::ApplyAnim()
{
	if(m_pObj)
	{
		m_pObj->ApplyAnim();
	}
}

void ParaUIObject::SetTextAutoTranslate( const char* strText, const char* sConverter )
{
	if(!IsValid())
		return;
	switch(m_pObj->GetType()->GetTypeValue())
	{
	case Type_GUIButton:
	case Type_GUIText:
#ifndef PARAENGINE_MOBILE
		if(sConverter[0] == 'c' && sConverter[1] == '_')
		{
			// if the name is "c_XXX", we will simply look for a file under "locale/c_XXX.txt" for character to character map. 
			if(sConverter[2] == 'S' && sConverter[3] == '2' && sConverter[4] == 'T')
			{
				static CSimpleTranslater g_simpl_to_trad("locale/c_simpl_to_trad.txt");
				const WCHAR * swText = g_simpl_to_trad.TranslateW(ParaEngine::StringHelper::MultiByteToWideChar(strText, DEFAULT_GUI_ENCODING));
				if(swText == 0)
				{
					swText = L"";
				}
#ifdef WIN32
				m_pObj->SetText((const char16_t*)swText);
#endif
			}
		}
		else
#else
		{
			SetText(strText);
		}
#endif
		break;
	default:
		SetText(strText);
		break;
	}
}

bool ParaUIObject::IsValid() const
{
	return m_pObj;
}

luabind::object ParaUIObject::GetField(const char* sFieldname, const object& output)
{
	ParaAttributeObject att(m_pObj);
	return att.GetField(sFieldname, output);
}

void ParaUIObject::SetField(const char* sFieldname, const object& input)
{
	ParaAttributeObject att(m_pObj);
	att.SetField(sFieldname, input);
}

void ParaUIObject::CallField(const char* sFieldname)
{
	ParaAttributeObject att(m_pObj);
	att.CallField(sFieldname);
}


//--------------------------------------------------------------
// for ParaUI namespace table.
//--------------------------------------------------------------

void ParaUI::GetMousePosition(float &x,float &y)
{
	int nX=0,nY=0;
	CGlobals::GetGUI()->GetMousePosition(&nX, &nY);
	x = (float)nX;
	y = (float)nY;
}

void ParaUI::SetMousePosition( float x,float y )
{
	CGlobals::GetGUI()->SetMousePosition((int)x, (int)y);
}

void ParaUI::AddDragReceiver( const char* sName )
{
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	pdrag->AddReceiver(sName);
}

void ParaUI::PlaySound(const char * strSoundAssetName, bool bLoop)
{
	OUTPUT_LOG("ParaUI::PlaySound has been deprecated! Use ParaAudio::PlayWaveFile() instead\n");

	//// test version of Audio Engine
	//XACTINDEX index = CGlobals::GetAudioEngine()->GetCueIndexByName(strSoundAssetName);
	//if(index != XACTINDEX_INVALID)
	//	CGlobals::GetAudioEngine()->PlayAudioCue(index);
	//else
	//	OUTPUT_LOG("Sound didn't found %s\n", strSoundAssetName);
}
void ParaUI::StopSound(const char * strSoundAssetName)
{
	//SoundEntity* pSoundEntity = CGlobals::GetAssetManager()->GetSound(strSoundAssetName);
	//if(pSoundEntity && pSoundEntity->GetSound())
	//{
	//	DSUtil_StopSound(pSoundEntity->GetSound());
	//}

	OUTPUT_LOG("ParaUI::StopSound has been deprecated!Use ParaAudio::StopWaveFile() instead\n ");
}

void ParaUI::Destroy(const char * strObjectName)
{
	CGlobals::GetGUI()->DestroyGUIElement(strObjectName);	
}

void ParaUI::Destroy1( int nID )
{
	CGlobals::GetGUI()->DestroyGUIElement(CGlobals::GetGUI()->GetUIObject(nID));
}

void ParaUI::DestroyUIObject(ParaUIObject& obj)
{
	if(obj.IsValid())
		CGlobals::GetGUI()->DestroyGUIElement((CGUIBase*)obj.m_pObj);	
}

void ParaUI::PostDestroy(const char * strObjectName)
{
	CGUIBase *obj=CGlobals::GetGUI()->GetUIObject(strObjectName);
	if (obj!=NULL){
		CGlobals::GetGUI()->PostDeleteGUIObject(obj);
	}
}

ParaUIObject ParaUI::GetUIObject_any(const object& NameOrID)
{
	using namespace luabind;
	int nType = type(NameOrID);
	if(nType == LUA_TSTRING)
	{
		return ParaUIObject(CGlobals::GetGUI()->GetUIObject(NPL::NPLHelper::LuaObjectToString(NameOrID)));
	}
	else if(nType == LUA_TNUMBER)
	{
		int nID = object_cast<int>(NameOrID);
		return ParaUIObject(CGlobals::GetGUI()->GetUIObject(nID));
	}
	return ParaUIObject();
}

ParaUIObject ParaUI::GetUIObject(const char* Name)
{
	return ParaUIObject(CGlobals::GetGUI()->GetUIObject(Name));
}

ParaScripting::ParaUIObject ParaUI::GetTopLevelControl()
{
	CGUIBase* pObj = CGlobals::GetGUI()->GetTopLevelControl();
	return ParaUIObject(pObj);
}

ParaUIObject ParaUI::GetUIObjectAtPoint(int x,int y)
{
	CGUIBase* pObj = CGlobals::GetGUI()->GetUIObject(x,y);
	return ParaUIObject(pObj);
}


ParaUIObject ParaUI::CreateUIObject(const char* strType, const char * strObjectName,const char * alignment, int x, int y, int width, int height)
{
	//TODO: here is possible memory leak because nobody knows when to delete the new object if the object is not attached to something later.
	CGUIBase* pNewObj=NULL; 

	if(strcmp(strType, "button")==0)
	{
		pNewObj= new CGUIButton();
		((CGUIButton*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
		
	}
	else if (strcmp(strType, "editbox")==0)
	{
		pNewObj = new CGUIEditBox();
		((CGUIEditBox*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "imeeditbox")==0)
	{
		pNewObj = new CGUIIMEEditBox();
		((CGUIIMEEditBox*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "scrollbar")==0)
	{
		pNewObj = new CGUIScrollBar();
		((CGUIScrollBar*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "container")==0)
	{
		pNewObj = new CGUIContainer();
		((CGUIContainer*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "text")==0)
	{
		pNewObj = new CGUIText();
		((CGUIText*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "listbox")==0)
	{
		pNewObj = new CGUIListBox();
		((CGUIListBox*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
	else if (strcmp(strType, "slider")==0)
	{
		pNewObj = new CGUISlider();
		((CGUISlider*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
#if defined(USE_DIRECTX_RENDERER ) && !defined(NPLRUNTIME)
#ifdef USE_GUI_DIRECTSHOW_VIDEO
	else if (strcmp(strType, "video")==0)
	{
		pNewObj = new CGUIVideo();
		((CGUIVideo*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
#endif
	else if (strcmp(strType, "webbrowser")==0)
	{
		pNewObj = new CGUIWebBrowser();
		((CGUIWebBrowser*)pNewObj)->InitObject(strObjectName, alignment, x, y, width, height);
	}
#endif
	else{
		OUTPUT_LOG("warning: type %s is not supported\n", strType);
	}
	return ParaUIObject(pNewObj);
}

ParaUIObject ParaUI::GetDefaultObject(const char *strType)
{
	return ParaUIObject(CGlobals::GetGUI()->GetDefaultObject(strType));
}

void ParaUI::SetCursorFont(const char *fontname,const char * strColor,DWORD transparency)
{
	LinearColor color;
	if(sscanf(strColor, "%f %f %f", &color.r,&color.g,&color.b)<3)
		color = LinearColor(1,1,1,1);
	color=(transparency<<24)|(color&0x00ffffff);
	if (CGlobals::GetAssetManager()->GetFont(fontname)==NULL) {
		if(fontname)
		{
			OUTPUT_LOG("warning: Font %s does not exist \n", fontname);
		}
	}
}

void ParaUI::SetUseSystemCursor( bool bUseSystem )
{
	CGlobals::GetGUI()->SetUseSystemCursor(bUseSystem);
}

bool ParaUI::GetUseSystemCursor()
{
	return CGlobals::GetGUI()->GetUseSystemCursor();
}

void ParaUI::SetCursorTexture(const char *texturename, const char * strColor,DWORD transparency )
{
	LinearColor color;
	if(sscanf(strColor, "%f %f %f", &color.r,&color.g,&color.b)<3)
		color = LinearColor(1,1,1,1);
	color=(transparency<<24)|(color&0x00ffffff);
	if (CGlobals::GetAssetManager()->GetTexture(texturename)==NULL) 
	{
		if(texturename)
		{
			OUTPUT_LOG("warning: ParaUI.SetCursorTexture, texture %s does not exist \n", texturename);
		}
	}
}


void ParaUI::SetCursorText(const char *strText)
{
	if (strText==NULL) {
		OUTPUT_LOG("\nSetCursorText: Input text is nil.");
	}
}

void ParaUI::SetCursorFromFile(const char *szCursor, int XHotSpot, int YHotSpot)
{
#ifdef USE_DIRECTX_RENDERER
	//CGlobals::GetGUI()->m_pMouse->SetCursorFromFile(szCursor, XHotSpot, YHotSpot);
	CGlobals::GetScene()->SetCursor(szCursor, XHotSpot, YHotSpot);
#endif
}

void ParaUI::SetCursorFromFile_(const char *szCursor)
{
#ifdef USE_DIRECTX_RENDERER
	//SetCursorFromFile(szCursor, 0,0);
	CGlobals::GetScene()->SetCursor(szCursor, 0,0);
#endif
}

const char* ParaUI::GetCursorFile()
{
	return CGlobals::GetString().c_str();
}

string ParaUI::ToScript()
{
	return CGlobals::GetGUI()->ToScript();
}

//obsolete
bool ParaUI::SaveLayout(const char *filename)
{
	FILE *file;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
	file = ::_wfopen(filename16, L"w+");
#else
	file = ::fopen(filename, "w+");
#endif
	if ((file)==NULL) {
		ParaGlobal::WriteToLogFile("Can't open output file for saving layout.");
		return false;
	}

	string script=ParaUI::ToScript();
	if (fwrite(script.data(),1,script.size(),file)!=script.size()) {
		ParaGlobal::WriteToLogFile("Writing current layout to file failed.");
		return false;
	};
	fclose(file);
	return true;
}

void ParaUI::SetDesignTime(bool bDesign)
{
	CGlobals::GetGUI()->m_bDesign=bDesign;
}

void ParaUI::ShowCursor(bool bShow)
{
	auto pMouse = CGlobals::GetGUI()->m_pMouse;
	if (pMouse)
	{
		pMouse->ShowCursor(bShow);
	}
}

void ParaUI::LockMouse(bool bLock)
{
	auto pMouse = CGlobals::GetGUI()->m_pMouse;
	if (pMouse)
	{
		CGlobals::GetGUI()->m_pMouse->SetLock(bLock);
	}
}

bool ParaUI::IsMouseLocked()
{
	auto pMouse = CGlobals::GetGUI()->m_pMouse;
	if (pMouse)
	{
		return CGlobals::GetGUI()->m_pMouse->IsLocked();
	}
	return false;	
}

void ParaUI::SetToolTipBehavior(const char* behavior)
{
	if (behavior==NULL) {
		return;
	}
	if(strcmp(behavior, "normal")==0)
	{
		CGUIRoot::GetInstance()->m_tooltip->SetBehavior(CGUIToolTip::NONE);
	}else if(strcmp(behavior, "flashing")==0)
	{
		CGUIRoot::GetInstance()->m_tooltip->SetBehavior(CGUIToolTip::FLASHING);
	}
}

void ParaUI::ResetUI()
{
	CGUIRoot::GetInstance()->Initialize();
}

void ParaUI::SetIMEOpenStatus(bool bOpen)
{
#ifdef USE_DIRECTX_RENDERER
	CGUIIMEEditBox::SetIMEOpenStatus(bOpen);
#endif
}

bool ParaUI::GetIMEOpenStatus()
{
#ifdef USE_DIRECTX_RENDERER
	return CGUIIMEEditBox::GetIMEOpenStatus();
#else
	return false;
#endif
}

void ParaUI::SetMinimumScreenSize(int nWidth, int nHeight, bool bAutoUIScaling)
{
	CGUIRoot::GetInstance()->SetMinimumScreenSize(nWidth, nHeight, bAutoUIScaling);
}

void ParaUI::SetUIScale(float fScalingX, float fScalingY)
{
	CGUIRoot::GetInstance()->SetUIScale(fScalingX, fScalingY);
}

void ParaUI::SetMaximumScreenSize(int nWidth, int nHeight, bool bAutoUIScaling)
{
	CGUIRoot::GetInstance()->SetMaximumScreenSize(nWidth, nHeight, bAutoUIScaling);
}

bool ParaUI::SetHighlightParam(const char* szEffectName, const char* szParamName, const char* szParamValue)
{
	CGUIHighlightManager* gm=&CSingleton<CGUIHighlightManager>::Instance();
	CGUIHighlight* ph=gm->GetHighlight(szEffectName);
	if (ph){
		return ph->SetParameter(szParamName,szParamValue);
	}
	return false;
}

bool ParaUI::IsKeyPressed(const EVirtualKey& nDikScanCode )
{
	CGUIKeyboardVirtual *pKeyboard=CGlobals::GetGUI()->m_pKeyboard;
	if(pKeyboard)
	{
		return pKeyboard->IsKeyPressed(nDikScanCode);
	}
	return false;
}

bool ParaUI::IsMousePressed(int nButton)
{
	CGUIMouseVirtual *pMouse=CGlobals::GetGUI()->m_pMouse;
	if(pMouse)
	{
		return pMouse->IsButtonDown((EMouseButton)nButton);
	}
	return false;
}

#if defined (USE_FLASH_MANAGER) && defined(USE_DIRECTX_RENDERER)
ParaScripting::ParaFlashPlayer ParaUI::GetFlashPlayer(const char* sFileName)
{
	return ParaFlashPlayer(CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayerIndex(sFileName));
}

ParaScripting::ParaFlashPlayer ParaUI::CreateFlashPlayer(const char* sFileName)
{
	CFlashPlayer* pFlashPlayer = CGlobals::GetAssetManager()->GetFlashManager().GetFlashPlayer(sFileName);
	if (pFlashPlayer != NULL)
		return ParaFlashPlayer(pFlashPlayer->GetIndex());
	else
		return ParaFlashPlayer(-1);
}

ParaScripting::ParaFlashPlayer ParaUI::GetFlashPlayer1(int nIndex)
{
	return ParaFlashPlayer(nIndex);
}
#endif
}//namespace ParaScripting