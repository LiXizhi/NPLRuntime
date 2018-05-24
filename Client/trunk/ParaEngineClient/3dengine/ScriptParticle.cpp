#include "ParaEngine.h"
#include "SceneState.h"
#include "TextureEntity.h"
#include "AssetManager.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "IBatchedElementDraw.h"
#include "BaseCamera.h"
#include "SceneObject.h"
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
	pClass->AddField("mParticleWidth",FieldType_Float,(void*)setParticleWidth_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleHeight",FieldType_Float,(void*)setParticleHeight_s,NULL,NULL,NULL,bOverride);
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
	{
		element->m_vRenderOffset=m_vPos;
		element->draw(sceneState);
	}
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
	mCurrentParticleElement->mTexture=new asset_ptr<TextureEntity>(CGlobals::GetAssetManager()->LoadTexture("",filename,TextureEntity::StaticTexture));
}

CScriptParticle::SParticleElement * ParaEngine::CScriptParticle::_createOreRetrieve()
{
	if(mFreeElements.empty())
		mFreeElements.push_back(new SParticleElement);
	auto ret=mFreeElements.back();
	mFreeElements.pop_back();
	return ret;
}

ParaEngine::CScriptParticle::SParticleElement::SParticleElement()
	:mTexture(nullptr)
	,mPosition(0,0,0)
	,mSize(0,0)
	,mUVOffset(0,0)
	,mColour(1,1,1,1)
	,m_vRenderOffset(0,0,0)
{
	addref();
}

ParaEngine::CScriptParticle::SParticleElement::~SParticleElement()
{
	delete mTexture;
}

void ParaEngine::CScriptParticle::SParticleElement::draw(SceneState * sceneState)
{
	sceneState->GetBatchedElementDrawer()->AddParticle(this);
}

int ParaEngine::CScriptParticle::SParticleElement::RenderParticle(SPRITEVERTEX ** ppVertexBuffer,SceneState * pSceneState)
{
	auto billboard=pSceneState->mxView;
	billboard.setTrans(Vector3::ZERO);
	billboard=billboard.transpose();
	DWORD color=Color(mColour);
	SPRITEVERTEX* pVertexBuffer=*ppVertexBuffer;
	BillBoardViewInfo& bbInfo=pSceneState->BillBoardInfo();

	const Vector3 world_pos(mPosition+m_vRenderOffset-CGlobals::GetScene()->GetRenderOrigin());

	Vector3 v[]={Vector3(-mSize.x/2,-mSize.y/2,0),Vector3(-mSize.x/2,mSize.y/2,0),Vector3(mSize.x/2,mSize.y/2,0),Vector3(mSize.x/2,-mSize.y/2,0)};
	const Vector2 uv[]={Vector2(mUVOffset.x,mUVOffset.y+1.0f),Vector2(mUVOffset.x,mUVOffset.y),Vector2(mUVOffset.x+1.0f,mUVOffset.y),Vector2(mUVOffset.x+1.0f,mUVOffset.y+1.0f)};
	for(int i=0;i<4;++i)
	{
		Vector3 v_t;
		ParaVec3TransformCoord(&v_t,&v[i],&billboard);
		auto wpos=world_pos+v_t;
		SetParticleVertex(pVertexBuffer[i],wpos.x,wpos.y,wpos.z,uv[i].x,uv[i].y,color);
	}

	pVertexBuffer[4]=pVertexBuffer[0];
	pVertexBuffer[5]=pVertexBuffer[2];
	(*ppVertexBuffer)+=6;
	return 1;
}

TextureEntity * ParaEngine::CScriptParticle::SParticleElement::GetTexture()
{
	if(mTexture)
		return mTexture->get();
	else
		return nullptr;
}
