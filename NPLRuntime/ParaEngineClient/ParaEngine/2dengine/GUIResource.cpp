//----------------------------------------------------------------------
// Class:	CGUIResource
// Authors:	LiXizh, LiuWeili
// Date:	2005.8.1
// Revised: 2014.8 using cross platform API
// desc: 
// When we design our GUI, we wish to render it according to resources independ of a certain control. 
// For example, suppose we are design the GUI of World of Warcraft. We want to drag a item from the 
// bag (in a dialog) to a empty place in the active item list. It is not a good idea to (1)delete the item from the bag
// (2)delete the empty item in the active item list (3)create a new item in the active item list and set
// it to the new item (4)create an empty item in the bag(otherwise, there will be a hole in the bag!)
// Indeed, we need only to exchange the data of the empty item in the active list and the item in the
// bag.
//
// The CGUIResource class solves this problem. It encapsulates the necessary resource for rendering 
// a control, the necessary game data for describing the item and the scripts for interacting with 
// the game engine.
//
// At the time of the current version, CGUIResource class only encapsulates information for exchanging 
// the same type of control. For example, when we wish to exchange button A and button B, we can 
// simply exchange their CGUIResource objects. But we cannot exchange the CGUIResource objects if we
// wish to exchange button A and label B. We will design a more common CGUIResource class for exchanging 
// more types of controls in the future. 
//
// Script for controlling the behavior of the control is contained in this class. By dispatching a CGUIEvent 
// object to member 
// 
//-----------------------------------------------------------------------

#include "ParaEngine.h"
#include "GUIResource.h"
#include "GUIScript.h"
#include <list>
#include "GUIRoot.h"
#include "AssetEntity.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "util/StringHelper.h"
#include "memdebug.h"

using namespace ParaEngine;


static TextureEntity emptyTexture;
//////////////////////////////////////////////////////////////////////////
// GUITextureElement
//////////////////////////////////////////////////////////////////////////

void GUITextureElement::GetRect(RECT* pOut)
{
	if (rcTexture.right >= 0 && rcTexture.bottom >= 0)
	{
		*pOut = rcTexture;
	}
	else if (pTexture)
	{
		const TextureEntity::TextureInfo* pTextureInfo = pTexture->GetTextureInfo();
		if (pTextureInfo)
		{
			/** 2009.8.16, LiXizhi:
			* for unavailable texture, we used texture width (-1, -1) and offset by this negative width whenever a smaller inner region is specified(making the rcTexture.right and bottom more negative).
			* so in case, the texture width is suddenly available, we can offset (image_width+1, image_height+1) to obtain the inner region properly */
			if (rcTexture.right < 0 && (pTextureInfo->m_width + rcTexture.right + 1) >= 0)
			{
				rcTexture.right = (pTextureInfo->m_width + rcTexture.right + 1);
				if (rcTexture.left < 0)
				{
					rcTexture.left = (pTextureInfo->m_width + rcTexture.left + 1);
				}
			}
			if (rcTexture.bottom < 0 && (pTextureInfo->m_height + rcTexture.bottom + 1) >= 0)
			{
				rcTexture.bottom = (pTextureInfo->m_height + rcTexture.bottom + 1);
				if (rcTexture.top < 0)
				{
					rcTexture.top = (pTextureInfo->m_height + rcTexture.top + 1);
				}
			}
		}
		*pOut = rcTexture;
	}
}

void GUITextureElement::SetElement(TextureEntity* pTexture, const RECT* prcTexture, Color defaultTextureColor)
{
	this->pTexture = pTexture;
	TextureColor = defaultTextureColor;

	if (pTexture == NULL)
	{
		pTexture = &::emptyTexture;
		rcTexture.left = 0;
		rcTexture.top = 0;
		rcTexture.right = 64;
		rcTexture.bottom = 64;

	}
	else
	{
		if (prcTexture)
			rcTexture = *prcTexture;
		else
		{
			rcTexture.left = 0; rcTexture.top = 0;
			const TextureEntity::TextureInfo* pTexInfo = pTexture->GetTextureInfo();
			if (pTexInfo)
			{
				rcTexture.right = pTexInfo->m_width;
				rcTexture.bottom = pTexInfo->m_height;
			}
			else
			{
				rcTexture.right = 64;
				rcTexture.bottom = 64;
			}
		}
	}
}

ParaEngine::GUITextureElement::GUITextureElement()
{
	TextureColor = 0xffffffff;
	SetRect(&rcTexture, 0, 0, 0, 0);
	name = "";
}

GUITextureElement::~GUITextureElement()
{
	Cleanup();
}

void GUITextureElement::Cleanup()
{
	pTexture.reset();
}

string GUITextureElement::ToScript(int index, int option)
{
	string script = "";
	//add resource:SetTexture("background","0 0 0",255,0);"like script
	script += "__res1:SetTexture(\"";
	script += pTexture->GetKey();
	script += "\",\"";
	char temp[30];
	DWORD color = TextureColor;
	ParaEngine::StringHelper::fast_itoa(color & 0x00ff0000, temp, 30);
	script += temp;
	script += " ";
	ParaEngine::StringHelper::fast_itoa(color & 0x0000ff00, temp, 30);
	script += temp;
	script += " ";
	ParaEngine::StringHelper::fast_itoa(color & 0x000000ff, temp, 30);
	script += temp;
	script += "\",";
	ParaEngine::StringHelper::fast_itoa(color & 0xff000000, temp, 30);
	script += temp;
	script += ",";
	ParaEngine::StringHelper::fast_itoa(index, temp, 10);
	script += temp;
	script += ");\n";
	return script;

}
//////////////////////////////////////////////////////////////////////////
// GUIFontElement
//////////////////////////////////////////////////////////////////////////
void GUIFontElement::SetElement(SpriteFontEntity* pFont, Color defaultFontColor, DWORD dwTextFormat)
{
	this->pFont = pFont;
	this->dwTextFormat = dwTextFormat;

	FontColor = defaultFontColor;
}

GUIFontElement::~GUIFontElement()
{
	Cleanup();
}

void GUIFontElement::Cleanup()
{
	pFont.reset();
}

string GUIFontElement::ToScript(int index, int option)
{
	string script = "";
	//add resource:SetFont("background","0 0 0",255,0,0);"like script
	script += "__res1:SetFont(\"";
	script += pFont->GetKey();
	script += "\",\"";
	char temp[30];
	DWORD color = FontColor;
	ParaEngine::StringHelper::fast_itoa(color & 0x00ff0000, temp, 30);
	script += temp;
	script += " ";
	ParaEngine::StringHelper::fast_itoa(color & 0x0000ff00, temp, 30);
	script += temp;
	script += " ";
	ParaEngine::StringHelper::fast_itoa(color & 0x000000ff, temp, 30);
	script += temp;
	script += "\",";
	ParaEngine::StringHelper::fast_itoa(color & 0xff000000, temp, 30);
	script += temp;
	script += ",";
	ParaEngine::StringHelper::fast_itoa(dwTextFormat, temp, 30);
	script += temp;
	script += ",";
	ParaEngine::StringHelper::fast_itoa(index, temp, 30);
	script += temp;
	script += ");\n";
	return script;

}

//////////////////////////////////////////////////////////////////////////
// GUILayer
//////////////////////////////////////////////////////////////////////////
string GUILAYER::ToScript(int option)
{
	DWORD a;
	string script = "";
	script += "__res1:SetCurrentState(\"normal\");\n";
	for (a = 0; a < eNormal.TextureSize(); a++) {
		script += eNormal.GetTextureElement(a)->ToScript(a, option);
	}
	for (a = 0; a < eNormal.FontSize(); a++) {
		script += eNormal.GetFontElement(a)->ToScript(a, option);
	}
	script += "__res1:SetCurrentState(\"highlight\");\n";
	for (a = 0; a < eHighlight.TextureSize(); a++) {
		script += eHighlight.GetTextureElement(a)->ToScript(a, option);
	}
	for (a = 0; a < eHighlight.FontSize(); a++) {
		script += eHighlight.GetFontElement(a)->ToScript(a, option);
	}
	script += "__res1:SetCurrentState(\"pressed\");\n";
	for (a = 0; a < ePressed.TextureSize(); a++) {
		script += ePressed.GetTextureElement(a)->ToScript(a, option);
	}
	for (a = 0; a < ePressed.FontSize(); a++) {
		script += ePressed.GetFontElement(a)->ToScript(a, option);
	}
	script += "__res1:SetCurrentState(\"disabled\");\n";
	for (a = 0; a < eDisabled.TextureSize(); a++) {
		script += eDisabled.GetTextureElement(a)->ToScript(a, option);
	}
	for (a = 0; a < eDisabled.FontSize(); a++) {
		script += eDisabled.GetFontElement(a)->ToScript(a, option);
	}
	return script;
}


//////////////////////////////////////////////////////////////////////////
// GUIStateElement
//////////////////////////////////////////////////////////////////////////
void GUIStateElement::Clone(GUIStateElement* output)
{
	output->m_fonts = m_fonts;
	output->m_textures = m_textures;
	output->fontmap = fontmap;
	output->texturemap = texturemap;
	/*vector <GUIFontElement>::iterator iter;
	for (iter=m_fonts.begin();iter!=m_fonts.end();iter++) {
	output->m_fonts.push_back(*iter);
	}
	vector <GUITextureElement>::iterator iter1;
	for (iter1=m_textures.begin();iter1!=m_textures.end();iter1++) {
	output->m_textures.push_back(*iter1);
	}
	map<string,DWORD>::iterator  iter2;
	for (iter2=fontmap.begin();iter2!=fontmap.end();iter2++) {
	output->fontmap[iter2->first]=iter2->second;
	}
	map<string,DWORD>::iterator  iter3;
	for (iter3=texturemap.begin();iter3!=texturemap.end();iter3++) {
	output->texturemap[iter3->first]=iter3->second;
	}*/
}



//////////////////////////////////////////////////////////////////////////
// CGUIResource
//////////////////////////////////////////////////////////////////////////

CGUIResource::CGUIResource()
{
	m_objArtwork = NULL;
	m_objBackground = NULL;
	m_objOverlay = NULL;
	m_objType = NONE;
	m_pActiveLayer = NULL;

	m_CurrentState = GUIResourceState_Normal;
}

CGUIResource::~CGUIResource()
{

	SAFE_DELETE(m_objOverlay);
	SAFE_DELETE(m_objBackground);
	SAFE_DELETE(m_objArtwork);
};

int CGUIResource::Release()
{
	delete this;
	return 0;
}

void CGUIResource::Clear()
{
	if (m_objArtwork) {
		m_objArtwork->init();
	}
	if (m_objOverlay) {
		m_objOverlay->init();
	}
	if (m_objBackground) {
		m_objBackground->init();
	}
	SetActiveLayer();
	SetCurrentState();
}
bool CGUIResource::Equals(const IObject *obj)const
{
	return obj == this;
}

void CGUIResource::Clone(IObject *pobj)const
{
	PE_ASSERT(pobj != NULL);
	if (pobj == NULL) {
		return;
	}
	CGUIResource* output = (CGUIResource*)pobj;
	output->m_CurrentState = m_CurrentState;
	output->m_objType = m_objType;
	//	for (DWORD a=0;a<m_objScripts.size();a++) {
	//		output->m_objScripts.AddScript(&m_objScripts[a]);
	//	}
	if (m_objArtwork) {
		output->m_objArtwork = new GUILAYER();
		m_objArtwork->Clone(output->m_objArtwork);
	}
	if (m_objOverlay) {
		output->m_objOverlay = new GUILAYER();
		m_objOverlay->Clone(output->m_objOverlay);
	}
	if (m_objBackground) {
		output->m_objBackground = new GUILAYER();
		m_objBackground->Clone(output->m_objBackground);
	}
	if (m_pActiveLayer == m_objArtwork){
		output->m_pActiveLayer = output->m_objArtwork;
	}
	else if (m_pActiveLayer == m_objBackground){
		output->m_pActiveLayer = output->m_objBackground;
	}
	else if (m_pActiveLayer == m_objOverlay){
		output->m_pActiveLayer = output->m_objOverlay;
	}

}

IObject* CGUIResource::Clone()const
{
	CGUIResource *pobj = new CGUIResource();
	Clone(pobj);
	return pobj;
}


CGUIResource::GUI_RESOURCE_LAYER_ENUM CGUIResource::GetLayerIDByName(const char* layer/*=NULL*/)
{
	if (layer == 0 || strlen(layer) == 0 || strcmp(layer, "artwork") == 0)
	{
		return CGUIResource::LAYER_ARTWORK;
	}
	else if (strcmp(layer, "background") == 0)
	{
		return CGUIResource::LAYER_BACKGROUND;
	}
	else if (strcmp(layer, "overlay") == 0)
	{
		return CGUIResource::LAYER_OVERLAY;
	}
	else
	{
		OUTPUT_LOG("warning: unknown layer name %s. LAYER_ARTWORK is used instead \r\n", layer);
		return CGUIResource::LAYER_ARTWORK;
	}
}

bool CGUIResource::HasLayer(const char *layer/*=NULL*/)
{
	return HasLayer(CGUIResource::GetLayerIDByName(layer));
}

bool CGUIResource::HasLayer(GUI_RESOURCE_LAYER_ENUM nLayerID /*= LAYER_ARTWORK*/)
{
	switch (nLayerID)
	{
	case CGUIResource::LAYER_BACKGROUND:
	{
		return (m_objBackground != 0);
	}
	case CGUIResource::LAYER_OVERLAY:
	{
		return (m_objOverlay != 0);
	}
	default: // CGUIResource::LAYER_ARTWORK
	{
		return (m_objArtwork != 0);
	}
	}
}

void CGUIResource::SetActiveLayer(const char *layer)
{
	SetActiveLayer(CGUIResource::GetLayerIDByName(layer));
}

void ParaEngine::CGUIResource::SetActiveLayer(GUI_RESOURCE_LAYER_ENUM nLayerID /*= LAYER_ARTWORK*/)
{
	switch (nLayerID)
	{
	case CGUIResource::LAYER_BACKGROUND:
	{
		if (!m_objBackground) {
			m_objBackground = new GUILAYER();
			m_objBackground->init();
		}
		m_pActiveLayer = m_objBackground;
		break;
	}
	case CGUIResource::LAYER_OVERLAY:
	{
		if (!m_objOverlay) {
			m_objOverlay = new GUILAYER();
			m_objOverlay->init();
		}
		m_pActiveLayer = m_objOverlay;
		break;
	}
	default: // CGUIResource::LAYER_ARTWORK
	{
		if (!m_objArtwork) {
			m_objArtwork = new GUILAYER();
			m_objArtwork->init();
		}
		m_pActiveLayer = m_objArtwork;
		break;
	}
	}
	SetCurrentState(m_CurrentState);
}

CGUIResource::GUI_RESOURCE_LAYER_ENUM  CGUIResource::GetActiveLayer()
{
	if (m_pActiveLayer == m_objArtwork)
		return CGUIResource::LAYER_ARTWORK;
	else if (m_pActiveLayer == m_objBackground)
		return CGUIResource::LAYER_BACKGROUND;
	else if (m_pActiveLayer == m_objOverlay)
		return CGUIResource::LAYER_OVERLAY;
	else
		return CGUIResource::LAYER_ARTWORK;
}


void CGUIResource::SetCurrentState(const char* statename)
{
	GUIResourceState state = GUIResourceState_Normal;
	if (statename == 0 || strcmp(statename, "normal") == 0 || strlen(statename) == 0) {
		state = GUIResourceState_Normal;
	}
	else if (strcmp(statename, "pressed") == 0) {
		state = GUIResourceState_Pressed;
	}
	else if (strcmp(statename, "highlight") == 0) {
		state = GUIResourceState_Highlight;
	}
	else if (strcmp(statename, "disabled") == 0) {
		state = GUIResourceState_Disabled;
	}
	else{
		OUTPUT_LOG("warning: nState %s, is not a valid state.\n", statename);
	}
	SetCurrentState(state);
}
void CGUIResource::SetCurrentState(GUIResourceState state)
{
	m_CurrentState = state;
	switch (m_CurrentState)
	{
	case GUIResourceState_Normal:
		m_pActiveLayer->eCurrent = &m_pActiveLayer->eNormal;
		break;
	case GUIResourceState_Pressed:
		m_pActiveLayer->eCurrent = &m_pActiveLayer->ePressed;
		break;
	case GUIResourceState_Highlight:
		m_pActiveLayer->eCurrent = &m_pActiveLayer->eHighlight;
		break;
	case GUIResourceState_Disabled:
		m_pActiveLayer->eCurrent = &m_pActiveLayer->eDisabled;
		break;
	default:
		break;
	}
}

void CGUIResource::CloneState(const char * statename)
{
	if (!statename || strlen(statename) == 0) { return; }
	if (strcmp(statename, "normal") == 0) {
		m_pActiveLayer->eNormal.Clone(m_pActiveLayer->eCurrent);
	}
	else if (strcmp(statename, "pressed") == 0) {
		m_pActiveLayer->ePressed.Clone(m_pActiveLayer->eCurrent);
	}
	else if (strcmp(statename, "highlight") == 0) {
		m_pActiveLayer->eHighlight.Clone(m_pActiveLayer->eCurrent);
	}
	else if (strcmp(statename, "disabled") == 0) {
		m_pActiveLayer->eDisabled.Clone(m_pActiveLayer->eCurrent);
	}
	else{
		OUTPUT_LOG("warning: nState %s, is not a valid state.\n", statename);
	}

}
RECT CGUIResource::GetDrawingRects(int index/* =0 */)const
{
	if (index < 0 || index >= (int)m_pActiveLayer->DrawingRects.size()) {
		RECT re = { 0, 0, 0, 0 };
		return re;
	}
	else
		return m_pActiveLayer->DrawingRects[index];
}

void CGUIResource::SetDrawingRects(RECT *rcDest, int index/* =0 */)
{
	if (index < 0 || rcDest == NULL) {
		return;
	}
	if (index >= (int)m_pActiveLayer->DrawingRects.size()) {
		m_pActiveLayer->DrawingRects.resize(index + 1);
	}
	m_pActiveLayer->DrawingRects[index] = *rcDest;
}

void CGUIResource::SetFont(const char *fontname, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->FontSize() || !fontname || strlen(fontname) == 0) {
		return;
	}
	GUIFontElement *pElement = m_pActiveLayer->eCurrent->GetFontElement(index);
	pElement->SetElement(CGlobals::GetAssetManager()->GetFont(fontname), pElement->FontColor, pElement->dwTextFormat);
}

void CGUIResource::SetFontColor(DWORD color, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->FontSize()) {
		return;
	}
	color = color & 0x00ffffff;
	GUIFontElement *pElement = m_pActiveLayer->eCurrent->GetFontElement(index);
	pElement->FontColor = color | (pElement->FontColor & 0xff000000);
}

void CGUIResource::SetFontFormat(DWORD dwFormat, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->FontSize()) {
		return;
	}
	GUIFontElement *pElement = m_pActiveLayer->eCurrent->GetFontElement(index);
	pElement->dwTextFormat = dwFormat;
}

void CGUIResource::SetFontTransparency(DWORD transparency, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->FontSize()) {
		return;
	}
	GUIFontElement *pElement = m_pActiveLayer->eCurrent->GetFontElement(index);
	pElement->FontColor = (transparency << 24) | (pElement->FontColor & 0x00ffffff);
}
void CGUIResource::SetTexture(const char *texturename, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->TextureSize() || !texturename || strlen(texturename) == 0) {
		return;
	}
	GUITextureElement *pElement = m_pActiveLayer->eCurrent->GetTextureElement(index);
	pElement->SetElement(CGlobals::GetAssetManager()->GetTexture(texturename), NULL, pElement->TextureColor);
}

void CGUIResource::SetTextureColor(DWORD color, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->TextureSize()) {
		return;
	}
	color = color & 0x00ffffff;
	GUITextureElement *pElement = m_pActiveLayer->eCurrent->GetTextureElement(index);
	pElement->TextureColor = color | (((DWORD)pElement->TextureColor) & 0xff000000);
}

void CGUIResource::SetTextureTransparency(DWORD transparency, int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->TextureSize()) {
		return;
	}
	GUITextureElement *pElement = m_pActiveLayer->eCurrent->GetTextureElement(index);
	pElement->TextureColor = (transparency << 24) | (((DWORD)pElement->TextureColor) & 0x00ffffff);

}

GUIFontElement* CGUIResource::GetFontElement(int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->FontSize()) {
		return NULL;
	}
	return m_pActiveLayer->eCurrent->GetFontElement(index);
}

GUITextureElement* CGUIResource::GetTextureElement(int index/* =0 */)
{
	if (index < 0 || index >= (int)m_pActiveLayer->eCurrent->TextureSize()) {
		return NULL;
	}
	return m_pActiveLayer->eCurrent->GetTextureElement(index);
}

GUIFontElement* CGUIResource::GetFontElement(const char *fontname)
{
	if (fontname == NULL) {
		return NULL;
	}
	return m_pActiveLayer->eCurrent->GetFontElement(fontname);
}

GUITextureElement* CGUIResource::GetTextureElement(const char *texturename)
{
	if (texturename == NULL) {
		return NULL;
	}
	return m_pActiveLayer->eCurrent->GetTextureElement(texturename);
}

int CGUIResource::GetLayerType()
{
	return m_pActiveLayer->m_objType;
}

void CGUIResource::SetLayerType(int etype)
{
	m_pActiveLayer->m_objType = etype;
}

void CGUIResource::AddElement(const GUIFontElement* pElement, const char*  name)
{
	if (pElement) {
		m_pActiveLayer->AddElement(pElement, name);
	}
}

GUITextureElement * CGUIResource::AddElement(const GUITextureElement* pElement, const char*  name)
{
	if (pElement) {
		return m_pActiveLayer->AddElement(pElement, name);
	}
	else
		return NULL;
}

void CGUIResource::SetElement(GUITextureElement* pElement, int index/* =0 */)
{
	if (pElement) {
		m_pActiveLayer->SetElement(pElement, index);
	}
}

void CGUIResource::SetElement(GUIFontElement* pElement, int index/* =0 */)
{
	if (pElement) {
		m_pActiveLayer->SetElement(pElement, index);
	}
}

string CGUIResource::ToScript(int option)
{
	string script = "";
	if (m_objArtwork) {
		script += "__res1:SetActiveLayer(\"artwork\");\n";
		script += m_objArtwork->ToScript(option);
	}
	if (m_objOverlay) {
		script += "__res1:SetActiveLayer(\"overlay\");\n";
		script += m_objOverlay->ToScript(option);
	}
	if (m_objBackground) {
		script += "__res1:SetActiveLayer(\"background\");\n";
		script += m_objBackground->ToScript(option);
	}
	return script;
}


GUIFontElement* ParaEngine::GUIStateElement::AddElement(const GUIFontElement *pElement, const char* name/*=NULL*/)
{
	if (pElement == NULL) {
		return NULL;
	}
	if (name != NULL && name[0] != '\0') {
		map<string, DWORD>::iterator it = fontmap.find(name);
		if (it != fontmap.end())
		{
			// replace old one
			m_fonts[(*it).second].Cleanup();
			m_fonts[(*it).second] = *pElement;

			return &m_fonts[(*it).second];
		}
		else
		{
			// add a new one at the back
			fontmap[name] = (DWORD)m_fonts.size();
			m_fonts.push_back(*pElement);

			return &m_fonts.back();
		}
	}
	else
	{
		//OUTPUT_LOG("it should never be here\n");
		return NULL;
	}
}

GUITextureElement * ParaEngine::GUIStateElement::AddElement(const GUITextureElement *pElement, const char* name/*=NULL*/)
{
	if (pElement == NULL) {
		return NULL;
	}
	if (name != NULL && name[0] != '\0') {
		map<string, DWORD>::iterator it = texturemap.find(name);
		if (it != texturemap.end())
		{
			// replace old one
			m_textures[(*it).second].Cleanup();
			m_textures[(*it).second] = *pElement;

			return &m_textures[(*it).second];
		}
		else
		{
			// add a new one at the back
			texturemap[name] = (DWORD)m_textures.size();
			m_textures.push_back(*pElement);

			return &m_textures.back();
		}
	}
	else
	{
		//OUTPUT_LOG("it should never be here\n");
		return NULL;
	}
}

GUITextureElement * ParaEngine::GUIStateElement::SetElement(const GUITextureElement *pElement, int nIndex)
{
	if (nIndex < 0 || pElement == NULL)
		return NULL;

	if (nIndex >= (int)m_textures.size())
	{
		// add a new one at the given index
		m_textures.resize(nIndex);
		m_textures.push_back(*pElement);
		return &m_textures.back();
	}
	else
	{
		// replace old one
		m_textures[nIndex].Cleanup();
		m_textures[nIndex] = *pElement;
		return &m_textures[nIndex];
	}
}

GUIFontElement * ParaEngine::GUIStateElement::SetElement(const GUIFontElement *pElement, int nIndex)
{
	if (nIndex < 0 || pElement == NULL)
		return NULL;

	if (nIndex >= (int)m_fonts.size())
	{
		// add a new one at the given index
		m_fonts.resize(nIndex);
		m_fonts.push_back(*pElement);
		return &m_fonts.back();
	}
	else
	{
		// replace old one
		m_fonts[nIndex].Cleanup();
		m_fonts[nIndex] = *pElement;
		return &m_fonts[nIndex];
	}
}

GUIFontElement* ParaEngine::GUIStateElement::GetFontElement(const string &name)
{
	map<string, DWORD>::const_iterator iter = fontmap.find(name);
	if (iter == fontmap.end())
		return NULL;
	return GetFontElement(iter->second);
}

GUIFontElement* ParaEngine::GUIStateElement::GetFontElement(int index)
{
	if (index < 0 || index >= (int)m_fonts.size()) {
		return NULL;
	}
	return &m_fonts[index];
}

GUITextureElement* ParaEngine::GUIStateElement::GetTextureElement(const string &name)
{
	map<string, DWORD>::const_iterator iter = texturemap.find(name);
	if (iter == texturemap.end())
		return NULL;
	return GetTextureElement(iter->second);
}

GUITextureElement* ParaEngine::GUIStateElement::GetTextureElement(int index)
{
	if (index < 0 || index >= (int)m_textures.size()) {
		return NULL;
	}
	return &m_textures[index];
}

void ParaEngine::GUIStateElement::init()
{
	m_textures.clear(); m_fonts.clear(); fontmap.clear(); texturemap.clear();
}

GUITextureElement * ParaEngine::GUILAYER::AddElement(const GUITextureElement* pElement, const char* name/*=NULL*/)
{
	if (pElement == NULL) {
		return NULL;
	}
	return eCurrent->AddElement(pElement, name);
	//if (eCurrent->TextureSize()>DrawingRects.size()) {
	//	DrawingRects.resize(eCurrent->TextureSize());
	//}
}

void ParaEngine::GUILAYER::AddElement(const GUIFontElement* pElement, const char* name/*=NULL*/)
{
	if (pElement == NULL) {
		return;
	}
	eCurrent->AddElement(pElement, name);
}

void ParaEngine::GUILAYER::SetElement(GUITextureElement* pElement, int index/*=0*/)
{
	if (pElement == NULL) {
		return;
	}
	eCurrent->SetElement(pElement, index);
}

void ParaEngine::GUILAYER::SetElement(GUIFontElement* pElement, int index/*=0*/)
{
	if (pElement == NULL) {
		return;
	}
	eCurrent->SetElement(pElement, index);
}

void ParaEngine::GUILAYER::Clone(GUILAYER* output)
{
	eNormal.Clone(&output->eNormal);
	eHighlight.Clone(&output->eHighlight);
	eDisabled.Clone(&output->eDisabled);
	ePressed.Clone(&output->ePressed);
	output->DrawingRects = DrawingRects;
	/*for (DWORD a=0;a<DrawingRects.size();a++) {
		output->DrawingRects.push_back(DrawingRects[a]);
		}*/
	output->m_objType = m_objType;
}