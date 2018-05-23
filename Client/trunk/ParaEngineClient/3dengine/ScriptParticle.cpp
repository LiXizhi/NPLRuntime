#include "ParaEngine.h"
#include "SceneState.h"
#include "TextureEntity.h"
#include "AssetManager.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "IBatchedElementDraw.h"
#include "ScriptParticle.h"

using namespace ParaEngine;

int ParaEngine::CScriptParticle::InstallFields(CAttributeClass * pClass,bool bOverride)
{
	CBaseObject::InstallFields(pClass,bOverride);
	pClass->AddField("clear",FieldType_void,(void*)clear_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("beginParticle",FieldType_void,(void*)beginParticle_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("endParticle",FieldType_void,(void*)endParticle_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticlePositionX",FieldType_Float,(void*)setParticlePositionX_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticlePositionY",FieldType_Float,(void*)setParticlePositionY_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticlePositionZ",FieldType_Float,(void*)setParticlePositionZ_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticlePositionZ",FieldType_Float,(void*)setParticlePositionZ_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleUVOffsetX",FieldType_Float,(void*)setParticleUVOffsetX_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleUVOffsetY",FieldType_Float,(void*)setParticleUVOffsetY_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourR",FieldType_Float,(void*)setParticleColourR_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourG",FieldType_Float,(void*)setParticleColourG_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourB",FieldType_Float,(void*)setParticleColourB_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourA",FieldType_Float,(void*)setParticleColourA_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mTexture",FieldType_String,(void*)setParticleTexture_s,NULL,NULL,NULL,bOverride);
	return S_OK;
}

ParaEngine::CScriptParticle::CScriptParticle()
	:mCurrentParticleElement(nullptr)
{
}

ParaEngine::CScriptParticle::~CScriptParticle()
{
	clear();
	for(auto element:mFreeElements)
		delete element;
}

HRESULT ParaEngine::CScriptParticle::Draw(SceneState * sceneState)
{
	for(auto element:mActiveElements)
		element->draw(sceneState);
	return S_OK;
}

void ParaEngine::CScriptParticle::clear()
{
	for(auto element:mActiveElements)
		mFreeElements.push_back(element);
	mActiveElements.clear();
}

void ParaEngine::CScriptParticle::beginParticle()
{
	assert(!mCurrentParticleElement);
	mCurrentParticleElement=_createOreRetrieve();
}

void ParaEngine::CScriptParticle::endParticle()
{
	assert(mCurrentParticleElement);
	mActiveElements.push_back(mCurrentParticleElement);
	mCurrentParticleElement=nullptr;
}

void ParaEngine::CScriptParticle::setParticlePosition(float x,float y,float z)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mPosition=Vector3(x,y,z);
}

const Vector3 & ParaEngine::CScriptParticle::getParticlePosition() const
{
	// TODO: 在此处插入 return 语句
	assert(mCurrentParticleElement);
	return mCurrentParticleElement->mPosition;
}

void ParaEngine::CScriptParticle::setParticleUVOffset(float u,float v)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mUVOffset=Vector2(u,v);
}

const Vector2 & ParaEngine::CScriptParticle::getParticleUVOffset() const
{
	// TODO: 在此处插入 return 语句
	assert(mCurrentParticleElement);
	return mCurrentParticleElement->mUVOffset;
}

void ParaEngine::CScriptParticle::setParticleColour(float r,float g,float b,float a)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mColour=LinearColor(r,g,b,a);
}

const LinearColor & ParaEngine::CScriptParticle::getParticleColour() const
{
	// TODO: 在此处插入 return 语句
	assert(mCurrentParticleElement);
	return mCurrentParticleElement->mColour;
}

void ParaEngine::CScriptParticle::setParticleSize(float width,float height)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mSize=Vector2(width,height);
}

const Vector2 & ParaEngine::CScriptParticle::getParticleSize() const
{
	// TODO: 在此处插入 return 语句
	assert(mCurrentParticleElement);
	return mCurrentParticleElement->mSize;
}

void ParaEngine::CScriptParticle::setParticleTexture(const string & filename)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mTexture=CGlobals::GetAssetManager()->LoadTexture("",filename,TextureEntity::StaticTexture);
}

CScriptParticle::SParticleElement * ParaEngine::CScriptParticle::_createOreRetrieve()
{
	if(mFreeElements.empty())
		mFreeElements.push_back(new SParticleElement);
	auto ret=mFreeElements.back();
	mFreeElements.pop_back();
	return ret;
}

void ParaEngine::CScriptParticle::SParticleElement::draw(SceneState * sceneState)
{
	sceneState->GetBatchedElementDrawer()->AddParticle(this);
}

int ParaEngine::CScriptParticle::SParticleElement::RenderParticle(SPRITEVERTEX ** ppVertexBuffer,SceneState * pSceneState)
{
	float vX=mPosition.x+m_vRenderOffset.x;
	float vY=mPosition.y+m_vRenderOffset.y;
	float vZ=mPosition.z+m_vRenderOffset.z;
	DWORD color=Color(mColour);

	SPRITEVERTEX* pVertexBuffer=*ppVertexBuffer;
	// 2 triangle list with 6 vertices
	Vector3 v;

	BillBoardViewInfo& bbInfo=pSceneState->BillBoardInfo();
	v.x=-mSize.x; v.y=-mSize.y; v.z=0;
	v=bbInfo.TransformVertexWithoutY(v);
	SetParticleVertex(pVertexBuffer[0],vX+v.x,vY+v.y,vZ+v.z,mUVOffset.x,mUVOffset.y+1.0f,color);
	v.x=-mSize.x; v.y=mSize.y; v.z=0;
	v=bbInfo.TransformVertexWithoutY(v);
	SetParticleVertex(pVertexBuffer[1],vX+v.x,vY+v.y,vZ+v.z,mUVOffset.x,mUVOffset.y,color);
	v.x=mSize.x; v.y=mSize.y; v.z=0;
	v=bbInfo.TransformVertexWithoutY(v);
	SetParticleVertex(pVertexBuffer[2],vX+v.x,vY+v.y,vZ+v.z,mUVOffset.x+1.0f,mUVOffset.y,color);
	v.x=mSize.x; v.y=-mSize.y; v.z=0;
	v=bbInfo.TransformVertexWithoutY(v);
	SetParticleVertex(pVertexBuffer[3],vX+v.x,vY+v.y,vZ+v.z,mUVOffset.x+1.0f,mUVOffset.y+1.0f,color);

	pVertexBuffer[4]=pVertexBuffer[0];
	pVertexBuffer[5]=pVertexBuffer[2];
	(*ppVertexBuffer)+=6;
	return 1;
}

TextureEntity * ParaEngine::CScriptParticle::SParticleElement::GetTexture()
{
	return mTexture.get();
}
