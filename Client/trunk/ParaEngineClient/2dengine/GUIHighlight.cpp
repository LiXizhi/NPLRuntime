//----------------------------------------------------------------------
// Class:	CGUIHighlight
// Authors:	Liu Weili, LiXizhi
// Date:	2006.7.27
// Revised: 2006.7.27
//
// desc: 
// The highlight control provides different styles of highlighting a control. This control is different from common GUI control
// in that it does not has any events
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "GUIResource.h"
#include "ic/ICConfigManager.h"
#include "ObjectManager.h"
#include "GUIResource.h"
#include "PaintEngine/Painter.h"
#include "GUIHighlight.h"
#include "CSingleton.h"
#include "util/StringHelper.h"
#include <math.h>
#include "memdebug.h"

using namespace ParaEngine;

#define PosBlending(d,t,s) (((float)d)*powf((s),10*(t)))

void SetElementHelper(const char* szValue, CGUIResource *pResource,int index)
{
	string background=szValue,filename;
	RECT* prect=NULL;
	RECT rect;
	TextureEntity* pTexture=NULL;
	GUITextureElement tex_;
	GUITextureElement* pElement=&tex_;
	
	prect=StringHelper::GetImageAndRect(background,filename, &rect);
	pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
	if (pTexture){
		pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
		pResource->SetElement(pElement,index);
	}
}
//////////////////////////////////////////////////////////////////////////
// CGUI4OutsideArrow
//////////////////////////////////////////////////////////////////////////
CGUI4OutsideArrow::CGUI4OutsideArrow()
{
	m_bInit=false;
	m_objResource=NULL;
	m_fSpeed=0.9f;
	m_nRange=50;
	m_nPos=20;
	m_nImageSize = 32;
	m_eStyle=Highlight4outsideArrow;
}

CGUI4OutsideArrow::~CGUI4OutsideArrow()
{
	SAFE_RELEASE(m_objResource);
}
void CGUI4OutsideArrow::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUI4OutsideArrow* ph=(CGUI4OutsideArrow*)pobj;
	SAFE_RELEASE(ph->m_objResource);
	ph->m_objResource=(CGUIResource*)m_objResource->Clone();
	ph->m_objResource->SetCurrentState();
	ph->m_fSpeed=m_fSpeed;
	ph->m_nImageSize=m_nImageSize;
	ph->m_nRange=m_nRange;
};

IObject* CGUI4OutsideArrow::Clone()const
{
	CGUI4OutsideArrow* pObj=new CGUI4OutsideArrow();
	Clone(pObj);
	return pObj;
}
void CGUI4OutsideArrow::Init()
{
	TextureEntity* pTexture=NULL;
	GUITextureElement tex_;
	GUITextureElement* pElement=&tex_;
	m_objResource=new CGUIResource();
	m_objResource->SetActiveLayer();
	m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
	m_objResource->SetCurrentState(GUIResourceState_Normal);
	RECT *prect=NULL;
	RECT rect;

	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	double temp;
	if (cm->GetDoubleValue("GUI_highlight_4outsideArrow_speed",&temp)==S_OK) {
		m_fSpeed=(float)temp;
	}
	int tempint;
	if (cm->GetIntValue("GUI_highlight_4outsideArrow_range",&tempint)==S_OK) {
		m_nRange=tempint;
	}

	if (cm->GetIntValue("GUI_highlight_4outsideArrow_size",&tempint)==S_OK) {
		m_nImageSize=tempint;
	}

	string background,filename;
	bool bLoadDefault=true;
	//left
	if (cm->GetTextValue("GUI_highlight_4outsideArrow_left",background)==S_OK) {
		
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("Texture/kidui/common/highlight_4out_left.png", "Texture/kidui/common/highlight_4out_left.png",TextureEntity::StaticTexture);
		prect=NULL;
	}

	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	m_objResource->AddElement(pElement,"left");
	
	//top
	bLoadDefault=true;
	if (cm->GetTextValue("GUI_highlight_4outsideArrow_top",background)==S_OK) {
		
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("Texture/kidui/common/highlight_4out_top.png", "Texture/kidui/common/highlight_4out_top.png",TextureEntity::StaticTexture);
		prect=NULL;
	}

	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	m_objResource->AddElement(pElement,"top");
	
	//right
	bLoadDefault=true;
	if (cm->GetTextValue("GUI_highlight_4outsideArrow_right",background)==S_OK) {
		
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("Texture/kidui/common/highlight_4out_right.png", "Texture/kidui/common/highlight_4out_right.png",TextureEntity::StaticTexture);
		prect=NULL;
	}

	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	m_objResource->AddElement(pElement,"right");
	
	//bottom
	bLoadDefault=true;
	if (cm->GetTextValue("GUI_highlight_4outsideArrow_bottom",background)==S_OK) {
		
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("Texture/kidui/common/highlight_4out_bottom.png", "Texture/kidui/common/highlight_4out_bottom.png",TextureEntity::StaticTexture);
		prect=NULL;
	}

	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	m_objResource->AddElement(pElement,"top");

	m_bInit=true;
}

HRESULT CGUI4OutsideArrow::Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime )
{
	if (!m_bInit){
		Init();
	}
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	RECT rc;
	GUITextureElement* pElement;
	m_nPos=(int)PosBlending(m_nPos,fElapsedTime,m_fSpeed);

	//left
	rc.left=rcScreen.left-m_nImageSize-m_nPos;
	rc.right=rc.left+m_nImageSize;
	rc.top=((rcScreen.top+rcScreen.bottom-m_nImageSize)>>1);
	rc.bottom=rc.top+m_nImageSize;
	pElement=m_objResource->GetTextureElement(0);
	if(pElement)
		pGUIState->painter->DrawSprite(pElement, &rc, 0);

	//top
	rc.left=((rcScreen.left+rcScreen.right-m_nImageSize)>>1);
	rc.right=rc.left+m_nImageSize;
	rc.top=rcScreen.top-m_nImageSize-m_nPos;
	rc.bottom=rc.top+m_nImageSize;
	pElement=m_objResource->GetTextureElement(1);
	if(pElement)
		pGUIState->painter->DrawSprite(pElement, &rc, 0);

	//right
	rc.left=rcScreen.right+m_nPos;
	rc.right=rc.left+m_nImageSize;
	rc.top=((rcScreen.top+rcScreen.bottom-m_nImageSize)>>1);
	rc.bottom=rc.top+m_nImageSize;
	pElement=m_objResource->GetTextureElement(2);
	if(pElement)
		pGUIState->painter->DrawSprite(pElement, &rc, 0);

	//bottom
	rc.left=((rcScreen.left+rcScreen.right-m_nImageSize)>>1);
	rc.right=rc.left+m_nImageSize;
	rc.top=rcScreen.bottom+m_nPos;
	rc.bottom=rc.top+m_nImageSize;
	pElement=m_objResource->GetTextureElement(3);
	if(pElement)
		pGUIState->painter->DrawSprite(pElement, &rc, 0);

	if (m_nPos==0){
		m_nPos=m_nRange;
	}
	return S_OK;
}

bool CGUI4OutsideArrow::SetParameter(const char* szName, const char* szValue)
{
	if (szName==NULL||szValue==NULL){
		return false;
	}
	if (strcmp(szName,"speed")==0){
		float speed=(float)atof(szValue);
		if (speed<0){
			return false;
		}
		m_fSpeed=speed;
	}else if (strcmp(szName,"range")==0){
		m_nRange=atoi(szValue);
	}else if (strcmp(szName,"left")==0){
		SetElementHelper(szValue,m_objResource,0);
	}else if (strcmp(szName,"top")==0){
		SetElementHelper(szValue,m_objResource,1);
	}else if (strcmp(szName,"right")==0){
		SetElementHelper(szValue,m_objResource,2);
	}else if (strcmp(szName,"bottom")==0){
		SetElementHelper(szValue,m_objResource,3);
	}else
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// CGUINStageAnimation
//////////////////////////////////////////////////////////////////////////
CGUINStageAnimation::CGUINStageAnimation()
{
	m_bInit=false;
	m_objResource=NULL;
	m_fSpeed=0.3f;
	m_nCurStage=0;
	m_nNumStage=1;
	m_fTime=0.0f;
	m_eStyle=HighlightNstageAnimation;
}

CGUINStageAnimation::~CGUINStageAnimation()
{
	SAFE_RELEASE(m_objResource);
}
void CGUINStageAnimation::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUINStageAnimation* ph=(CGUINStageAnimation*)pobj;
	SAFE_RELEASE(ph->m_objResource);
	ph->m_objResource=(CGUIResource*)m_objResource->Clone();
	ph->m_objResource->SetCurrentState();
	ph->m_fSpeed=m_fSpeed;
	ph->m_nNumStage=m_nNumStage;
};

IObject* CGUINStageAnimation::Clone()const
{
	CGUINStageAnimation* pObj=new CGUINStageAnimation();
	Clone(pObj);
	return pObj;
}
void CGUINStageAnimation::Init()
{
	TextureEntity* pTexture=NULL;
	GUITextureElement tex_;
	GUITextureElement* pElement=&tex_;
	m_objResource=new CGUIResource();
	m_objResource->SetActiveLayer();
	m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
	m_objResource->SetCurrentState(GUIResourceState_Normal);
	RECT *prect=NULL;
	RECT rect;

	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	double temp;
	if (cm->GetDoubleValue("GUI_highlight_NstageAnimation_speed",&temp)==S_OK) {
		m_fSpeed=(float)temp;
	}
	int tempint;
	if (cm->GetIntValue("GUI_highlight_NstageAnimation_size",&tempint)==S_OK) {
		m_nNumStage=tempint;
	}


	string background,filename;
	char valuename[100];
	char buff[20];
	for (int a=0;a<m_nNumStage;a++){
		snprintf(valuename,100,"GUI_highlight_NstageAnimation_stage%d",a);
		if (cm->GetTextValue(valuename,background)==S_OK) {
			
			prect=StringHelper::GetImageAndRect(background,filename, &rect);
			pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
			if (pTexture!=NULL) {
				snprintf(buff,20,"%d",a);
				pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
				m_objResource->AddElement(pElement,buff);
				continue;
			}
		}
		//code reach here means something wrong
		OUTPUT_LOG("Can't load animation stage %d\r\n",a);
		
	}
	m_bInit=true;
}

HRESULT CGUINStageAnimation::Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime )
{
	if (!m_bInit){
		Init();
	}
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	GUITextureElement* pElement;
	m_fTime+=fElapsedTime;
	m_nCurStage+=((int)(m_fTime/m_fSpeed));
	m_nCurStage%=(m_nNumStage+1);
	m_fTime=fmodf(m_fTime,m_fSpeed);
	if (m_nCurStage>0){
		pElement=m_objResource->GetTextureElement(m_nCurStage-1);
		pGUIState->painter->DrawSprite(pElement, &rcScreen, 0);
	}

	return S_OK;
}

bool CGUINStageAnimation::SetParameter(const char* szName, const char* szValue)
{
	if (strcmp(szName,"speed")==0){
		float speed=(float)atof(szValue);
		if (speed<0){
			return false;
		}
		m_fSpeed=speed;
	}else if (strcmp(szName,"size")==0){
		int stages=atoi(szValue);
		if (stages<0){
			return false;
		}
		m_nNumStage=stages;
	}else if (strncmp(szName,"stage",5)==0){
		szName+=5;
		int stage=atoi(szName);
		if (stage>=0&&stage<m_nNumStage){
			SetElementHelper(szValue,m_objResource,stage);
		}else
			return false;
	}else
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//	CGUIHighlightManager
//////////////////////////////////////////////////////////////////////////
CGUIHighlightManager::CGUIHighlightManager()
{
	CGUIHighlight* pHighlight;
	pHighlight=new CGUI4OutsideArrow();
	AddHighlight(pHighlight->GetHighlightStyle(),pHighlight);
	pHighlight=new CGUINStageAnimation();
	AddHighlight(pHighlight->GetHighlightStyle(),pHighlight);
}

CGUIHighlightManager::~CGUIHighlightManager()
{
	Finalize();
}

void CGUIHighlightManager::Finalize()
{
	ClearAllInstance();

	map<int,CGUIHighlight*>::iterator iter,iterend=m_items.end();
	for (iter=m_items.begin();iter!=iterend;iter++){
		SAFE_RELEASE(iter->second);
	}
	m_items.clear();
	map<string,CGUIHighlight*>::iterator iter1,iterend1=m_customs.end();
	for (iter1=m_customs.begin();iter1!=iterend1;iter1++){
		SAFE_RELEASE(iter1->second);
	}
	m_customs.clear();
}
void CGUIHighlightManager::AddHighlight(int style, CGUIHighlight* pHighlight)
{
	map<int,CGUIHighlight*>::iterator iter=m_items.find(style);
	if (iter!=m_items.end()){
		SAFE_RELEASE(iter->second);
		m_items.erase(iter);
	}
	m_items[style]=pHighlight;
}

CGUIHighlight* CGUIHighlightManager::GetHighlight(int style)
{
	map<int,CGUIHighlight*>::iterator iter=m_items.find(style);
	if (iter!=m_items.end()){
		return iter->second;
	}
	return NULL;
}

CGUIHighlight* CGUIHighlightManager::GetHighlight(const char* szName)
{
	if (szName==NULL){
		return NULL;
	}
	map<int,CGUIHighlight*>::iterator iter,iterend=m_items.end();
	for (iter=m_items.begin();iter!=iterend;iter++){
		if(strcmp(iter->second->GetHighlightName(),szName)==0){
			return iter->second;
		}
	}
	map<string,CGUIHighlight*>::iterator iter1=m_customs.find(szName);
	if (iter1!=m_customs.end()){
		return iter1->second;
	}
	return NULL;
}

HRESULT CGUIHighlightManager::Render(GUIState* pGUIState,RECT& rcScreen,float fElapsedTime, int style)
{
	CGUIHighlightManager* gm=&CSingleton<CGUIHighlightManager>::Instance();
	CGUIHighlight* pHighlight=gm->GetHighlight(style);
	if (pHighlight){
		return pHighlight->Render(pGUIState,rcScreen,fElapsedTime);
	}
	return S_OK;
}

CGUIHighlight* CGUIHighlightManager::CreateCustomHighlight(const char* szName, const char* szParentName)
{
	CGUIHighlight* ph=GetHighlight(szParentName);
	if (ph){
		CGUIHighlight* pNew=(CGUIHighlight*)ph->Clone();
		pNew->m_eStyle=HighlightCustom;
		map<string,CGUIHighlight*>::iterator iter=m_customs.find(szName);
		if (iter!=m_customs.end()){
			SAFE_RELEASE(iter->second);
		}
		m_customs[szName]=pNew;
		return pNew;
	}
	return NULL;
}

void CGUIHighlightManager::ClearAllInstance()
{
	m_instaces.clear();
}

void CGUIHighlightManager::AddNewInstance( GUIState* pGUIState,RECT& rcScreen,float fElapsedTime, int style )
{
	m_instaces.push_back(MarkerInstance(rcScreen, fElapsedTime, style));
}

HRESULT CGUIHighlightManager::Render( GUIState* pGUIState, float fElapsedTime,bool bAutoCleanInstance )
{
	if(m_instaces.empty())
		return S_OK;

	list<MarkerInstance>::iterator itCur, itEnd = m_instaces.end();
	for (itCur = m_instaces.begin(); itCur!=itEnd; ++itCur)
	{
		MarkerInstance& marker = (*itCur);
		CGUIHighlight* pHighlight=GetHighlight(marker.style);
		if (pHighlight){
			/* TODO: if there are multiple instance of the same type, all fElapsedTime except the first one should be set to zero.
			Otherwise, the marker may beep faster.*/
			pHighlight->Render(pGUIState,marker.rcScreen,marker.fElapsedTime);
		}
	}

	if(bAutoCleanInstance)
		ClearAllInstance();
	return S_OK;
}