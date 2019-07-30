#include "ParaEngine.h"
#include "SceneState.h"
#include "TextureEntity.h"
#include "AssetManager.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "IBatchedElementDraw.h"
#include "BaseCamera.h"
#include "SceneObject.h"
#include "util/StringHelper.h"
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
	pClass->AddField("mParticleUVScaleX",FieldType_Float,(void*)setParticleUVScaleX_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleUVScaleY",FieldType_Float,(void*)setParticleUVScaleY_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourR",FieldType_Float,(void*)setParticleColourR_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourG",FieldType_Float,(void*)setParticleColourG_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourB",FieldType_Float,(void*)setParticleColourB_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleColourA",FieldType_Float,(void*)setParticleColourA_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleWidth",FieldType_Float,(void*)setParticleWidth_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleHeight",FieldType_Float,(void*)setParticleHeight_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mParticleRotation",FieldType_Float,(void*)setParticleRotation_s,NULL,NULL,NULL,bOverride);

	pClass->AddField("mTexture",FieldType_String,(void*)setTexture_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mBillboardType",FieldType_String,(void*)setBillboardType_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonDirectionX",FieldType_Float,(void*)setCommonDirectionX_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonDirectionY",FieldType_Float,(void*)setCommonDirectionY_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonDirectionZ",FieldType_Float,(void*)setCommonDirectionZ_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonUpVectorX",FieldType_Float,(void*)setCommonUpX_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonUpVectorY",FieldType_Float,(void*)setCommonUpY_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mCommonUpVectorZ",FieldType_Float,(void*)setCommonUpZ_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mPointRendering",FieldType_Bool,(void*)setPointRenderingEnable_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mRotationType",FieldType_String,(void*)setRotationType_s,NULL,NULL,NULL,bOverride);
	pClass->AddField("mAccurateFacing",FieldType_Bool,(void*)setAccurateFacing_s,NULL,NULL,NULL,bOverride);
	return S_OK;
}

ParaEngine::CScriptParticle::CScriptParticle()
	:mCurrentParticleElement(nullptr)
	,mBillboardType(BBT_POINT)
	,mCommonDirection(Vector3::UNIT_Z)
	,mCommonUpVector(Vector3::UNIT_Y)
	,mPointRendering(false)
	,mAccurateFacing(false)
	,mRotationType(BBR_TEXCOORD)
{
	memset(&mTextureRect,0,sizeof(mTextureRect));
}

ParaEngine::CScriptParticle::~CScriptParticle()
{
	clear();
	for(auto element:mFreeElements)
		delete element;
}

HRESULT ParaEngine::CScriptParticle::Draw(SceneState * sceneState)
{
	Vector2 uv_offset(0,0);
	Vector2 uv_scale(0,0);
	if(mTexture.get()&&(mTextureRect.right!=0))
	{
		uv_offset.x=static_cast<float>(mTextureRect.left)/mTexture->GetTextureInfo()->GetWidth();
		uv_offset.y=static_cast<float>(mTextureRect.top)/mTexture->GetTextureInfo()->GetHeight();
		uv_scale.x=static_cast<float>(mTextureRect.right-mTextureRect.left)/mTexture->GetTextureInfo()->GetWidth();
		uv_scale.y=static_cast<float>(mTextureRect.bottom-mTextureRect.top)/mTexture->GetTextureInfo()->GetHeight();
	}
	auto need_update_uv=(uv_offset!=Vector2::ZERO)||(uv_scale!=Vector2::ZERO);
	for(auto element:mActiveElements)
	{
		if(need_update_uv)
		{
			element->mUVOffset=uv_offset;
			element->mUVScale=uv_scale;
		}
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

void ParaEngine::CScriptParticle::setParticleUVScale(float u,float v)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mUVScale=Vector2(u,v);
}

const Vector2 & ParaEngine::CScriptParticle::getParticleUVScale() const
{
	// TODO: 在此处插入 return 语句
	assert(mCurrentParticleElement);
	return mCurrentParticleElement->mUVScale;
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

void ParaEngine::CScriptParticle::setParticleRotation(float radian)
{
	assert(mCurrentParticleElement);
	mCurrentParticleElement->mRotation=radian;
}

void ParaEngine::CScriptParticle::setTexture(const string & filename)
{
	string real_file_name;
	if(!filename.empty())
	{
		memset(&mTextureRect,0,sizeof(mTextureRect));
		ParaEngine::StringHelper::GetImageAndRect(filename,real_file_name,&mTextureRect);
	}

	auto pTex = CGlobals::GetAssetManager()->GetTexture(real_file_name);
	 
	if(pTex == nullptr)
	{
		mTexture=CGlobals::GetAssetManager()->LoadTexture(real_file_name,real_file_name,TextureEntity::StaticTexture);
	}
	else
	{
		mTexture = pTex;
	}
}

void ParaEngine::CScriptParticle::setBillboardType(const std::string & val)
{
	if(val=="point")
	{
		mBillboardType=BBT_POINT;
	}
	else if(val=="oriented_common")
	{
		mBillboardType=BBT_ORIENTED_COMMON;
	}
	else if(val=="oriented_self")
	{
		mBillboardType=BBT_ORIENTED_SELF;
	}
	else if(val=="perpendicular_common")
	{
		mBillboardType=BBT_PERPENDICULAR_COMMON;
	}
	else if(val=="perpendicular_self")
	{
		mBillboardType=BBT_PERPENDICULAR_SELF;
	}
	else
	{
		mBillboardType=BBT_POINT;
	}
}

void ParaEngine::CScriptParticle::setCommonDirection(float x,float y,float z)
{
	mCommonDirection=Vector3(x,y,z);
}

void ParaEngine::CScriptParticle::setCommonUp(float x,float y,float z)
{
	mCommonUpVector=Vector3(x,y,z);
}

void ParaEngine::CScriptParticle::setPointRenderingEnable(bool enable)
{
	mPointRendering=enable;
}

void ParaEngine::CScriptParticle::setAccurateFacing(bool facing)
{
	mAccurateFacing=facing;
}

void ParaEngine::CScriptParticle::setRotationType(const std::string & val)
{
	if(val=="vertex")
		mRotationType=BBR_VERTEX;
	else if(val=="texcoord")
		mRotationType=BBR_TEXCOORD;
	else
		mRotationType=BBR_TEXCOORD;
}

CScriptParticle::SParticleElement * ParaEngine::CScriptParticle::_createOreRetrieve()
{
	if(mFreeElements.empty())
		mFreeElements.push_back(new SParticleElement(this));
	auto ret=mFreeElements.back();
	mFreeElements.pop_back();
	return ret;
}

ParaEngine::CScriptParticle::SParticleElement::SParticleElement(CScriptParticle * container)
	:mContainer(container)
	,mPosition(0,0,0)
	,mSize(0,0)
	,mUVOffset(0,0)
	,mUVScale(1,1)
	,mColour(1,1,1,1)
	,m_vRenderOffset(0,0,0)
	,mRotation(0)
{
	addref();
}

ParaEngine::CScriptParticle::SParticleElement::~SParticleElement()
{
}

void ParaEngine::CScriptParticle::SParticleElement::draw(SceneState * sceneState)
{
	sceneState->GetBatchedElementDrawer()->AddParticle(this);
}

int ParaEngine::CScriptParticle::SParticleElement::RenderParticle(SPRITEVERTEX ** ppVertexBuffer,SceneState * pSceneState)
{
	//auto billboard=pSceneState->mxView;
	//billboard.setTrans(Vector3::ZERO);
	//billboard=billboard.transpose();
	//DWORD color=Color(mColour);
	SPRITEVERTEX* pVertexBuffer=*ppVertexBuffer;
	//BillBoardViewInfo& bbInfo=pSceneState->BillBoardInfo();

	//const Vector3 world_pos(mPosition+m_vRenderOffset-CGlobals::GetScene()->GetRenderOrigin());

	//Vector3 v[]={Vector3(-mSize.x/2,-mSize.y/2,0),Vector3(-mSize.x/2,mSize.y/2,0),Vector3(mSize.x/2,mSize.y/2,0),Vector3(mSize.x/2,-mSize.y/2,0)};
	//const Vector2 uv[]={Vector2(mUVOffset.x,mUVOffset.y+1.0f),Vector2(mUVOffset.x,mUVOffset.y),Vector2(mUVOffset.x+1.0f,mUVOffset.y),Vector2(mUVOffset.x+1.0f,mUVOffset.y+1.0f)};
	//for(int i=0;i<4;++i)
	//{
	//	Vector3 v_t;
	//	ParaVec3TransformCoord(&v_t,&v[i],&billboard);
	//	auto wpos=world_pos+v_t;
	//	SetParticleVertex(pVertexBuffer[i],wpos.x,wpos.y,wpos.z,uv[i].x,uv[i].y,color);
	//}

	float left=-0.5f;
	float right=0.5f;
	float top=0.5f;
	float bottom=-0.5f;
	Vector3 camere_x(Vector3::ZERO),camera_y(Vector3::ZERO);
	Vector3 v_offset[4];
	if(!mContainer->mPointRendering)
	{
		// Generate axes etc up-front if not oriented per-billboard
		if(mContainer->mBillboardType!=BBT_ORIENTED_SELF&&
			mContainer->mBillboardType!=BBT_PERPENDICULAR_SELF&&
			!(mContainer->mAccurateFacing && mContainer->mBillboardType!=BBT_PERPENDICULAR_COMMON))
		{
			genBillboardAxes(&camere_x,&camera_y,pSceneState);

			/* If all billboards are the same size we can precalculate the
			offsets and just use '+' instead of '*' for each billboard,
			and it should be faster.
			*/
			genVertOffsets(left,right,top,bottom,
				mSize.x,mSize.y,camere_x,camera_y,v_offset);

		}
	}
	// If they're all the same size or we're point rendering
	if(mContainer->mPointRendering)
	{
		/* No per-billboard checking, just blast through.
		Saves us an if clause every billboard which may
		make a difference.
		*/

		genVertices(v_offset,pVertexBuffer);
	}
	else // not all default size and not point rendering
	{
		Vector3 vOwnOffset[4];
		// If it has own dimensions, or self-oriented, gen offsets
		if(mContainer->mBillboardType==BBT_ORIENTED_SELF||
			mContainer->mBillboardType==BBT_PERPENDICULAR_SELF||
			true/*bb.mOwnDimensions*/||
			(mContainer->mAccurateFacing && mContainer->mBillboardType!=BBT_PERPENDICULAR_COMMON))
		{
			// Generate using own dimensions
			genVertOffsets(left,right,top,bottom,
				mSize.x,mSize.y,camere_x,camera_y,vOwnOffset);
			// Create vertex data
			genVertices(vOwnOffset,pVertexBuffer);
		}
		else // Use default dimension, already computed before the loop, for faster creation
		{
			genVertices(v_offset,pVertexBuffer);
		}
	}

	pVertexBuffer[4]=pVertexBuffer[0];
	pVertexBuffer[5]=pVertexBuffer[2];
	(*ppVertexBuffer)+=6;
	return 1;
}

TextureEntity * ParaEngine::CScriptParticle::SParticleElement::GetTexture()
{
	return mContainer->mTexture.get();
}

void ParaEngine::CScriptParticle::SParticleElement::genVertOffsets(float inleft,float inright,float intop,float inbottom,float width,float height,const Vector3 & x,const Vector3 & y,Vector3 * pDestVec)
{
	Vector3 vLeftOff,vRightOff,vTopOff,vBottomOff;
	/* Calculate default offsets. Scale the axes by
	parametric offset and dimensions, ready to be added to
	positions.
	*/

	vLeftOff=x*(inleft   * width);
	vRightOff=x*(inright  * width);
	vTopOff=y*(intop   * height);
	vBottomOff=y*(inbottom * height);

	// Make final offsets to vertex positions
	pDestVec[1]=vLeftOff+vTopOff;
	pDestVec[2]=vRightOff+vTopOff;
	pDestVec[0]=vLeftOff+vBottomOff;
	pDestVec[3]=vRightOff+vBottomOff;
}

void ParaEngine::CScriptParticle::SParticleElement::genBillboardAxes(Vector3 * pX,Vector3 * pY,SceneState * pSceneState)
{
	const Vector3 world_pos(mPosition+m_vRenderOffset);
	Matrix3 rot_view;
	pSceneState->mxView.extract3x3Matrix(rot_view);
	Vector3 mCamDir=rot_view.GetColumn(2);
	// If we're using accurate facing, recalculate camera direction per BB
	if(mContainer->mAccurateFacing&&
		(mContainer->mBillboardType==BBT_POINT||
			mContainer->mBillboardType==BBT_ORIENTED_COMMON||
			mContainer->mBillboardType==BBT_ORIENTED_SELF))
	{
		// cam -> bb direction
		mCamDir=world_pos-pSceneState->vEye;
		mCamDir.normalise();
	}
	Vector3 vUp=rot_view.GetColumn(1);
	Vector3 vRight=vUp.crossProduct(mCamDir);
	vUp=mCamDir.crossProduct(vRight);
	Quaternion mCamQ=Quaternion(vRight,vUp,mCamDir).Inverse();


	switch(mContainer->mBillboardType)
	{
	case BBT_POINT:
		if(mContainer->mAccurateFacing)
		{
			// Point billboards will have 'up' based on but not equal to cameras
			// Use pY temporarily to avoid allocation
			*pY=mCamQ*Vector3::UNIT_Y;
			*pX=mCamDir.crossProduct(*pY);
			pX->normalise();
			*pY=pX->crossProduct(mCamDir); // both normalised already
		}
		else
		{
			// Get camera axes for X and Y (depth is irrelevant)
			*pX=mCamQ*Vector3::UNIT_X;
			*pY=mCamQ*Vector3::UNIT_Y;
		}
		break;

	case BBT_ORIENTED_COMMON:
		// Y-axis is common direction
		// X-axis is cross with camera direction
		*pY=mContainer->mCommonDirection;
		*pX=mCamDir.crossProduct(*pY);
		pX->normalise();
		break;

	case BBT_ORIENTED_SELF:
		// Y-axis is direction
		// X-axis is cross with camera direction
		// Scale direction first
		*pY=Vector3::UNIT_Z;
		*pX=mCamDir.crossProduct(*pY);
		pX->normalise();
		break;

	case BBT_PERPENDICULAR_COMMON:
		// X-axis is up-vector cross common direction
		// Y-axis is common direction cross X-axis
		*pX=mContainer->mCommonUpVector.crossProduct(mContainer->mCommonDirection);
		*pY=mContainer->mCommonDirection.crossProduct(*pX);
		break;

	case BBT_PERPENDICULAR_SELF:
		// X-axis is up-vector cross own direction
		// Y-axis is own direction cross X-axis
		*pX=mContainer->mCommonUpVector.crossProduct(Vector3::UNIT_Z);
		pX->normalise();
		*pY=Vector3::UNIT_Z.crossProduct(*pX); // both should be normalised
		break;
	}
}

void ParaEngine::CScriptParticle::SParticleElement::genVertices(const Vector3 * const offsets,SPRITEVERTEX * vertex)
{
	DWORD color=Color(mColour);
	Vector3 v[]={offsets[0],offsets[1],offsets[2],offsets[3]};
	Vector2 uv[]={Vector2(0,1.0f),Vector2(0,0),Vector2(1.0f,0),Vector2(1.0f,1.0f)};
	const Vector3 world_pos(mPosition+m_vRenderOffset-CGlobals::GetScene()->GetRenderOrigin());

	if(mRotation==0)
	{
		for(int i=0;i<4;++i)
		{
			uv[i]*=mUVScale;
			uv[i]+=mUVOffset;
			// Left-top
			// Positions
			vertex[i].p.x=v[i].x+world_pos.x;
			vertex[i].p.y=v[i].y+world_pos.y;
			vertex[i].p.z=v[i].z+world_pos.z;
			vertex[i].color=color;
			vertex[i].tu=uv[i].x;
			vertex[i].tv=uv[i].y;
		}
	}
	else if(mContainer->mRotationType==BBR_VERTEX)
	{
		// TODO: Cache axis when billboard type is BBT_POINT or BBT_PERPENDICULAR_COMMON
		Vector3 axis=(offsets[3]-offsets[1]).crossProduct(offsets[2]-offsets[0]).normalisedCopy();

		Quaternion rotation;
		rotation.FromAngleAxis(Radian(mRotation),axis);

		for(int i=0;i<4;++i)
		{
			uv[i]*=mUVScale;
			uv[i]+=mUVOffset;
			v[i]=rotation*v[i];
			// Left-top
			// Positions
			vertex[i].p.x=v[i].x+world_pos.x;
			vertex[i].p.y=v[i].y+world_pos.y;
			vertex[i].p.z=v[i].z+world_pos.z;
			vertex[i].color=color;
			vertex[i].tu=uv[i].x;
			vertex[i].tv=uv[i].y;
		}
	}
	else
	{
		const float      cos_rot(Math::Cos(mRotation));
		const float      sin_rot(Math::Sin(mRotation));

		float width=0.5f;
		float height=0.5f;
		float mid_u=0.5f;
		float mid_v=0.5f;

		float cos_rot_w=cos_rot*width;
		float cos_rot_h=cos_rot*height;
		float sin_rot_w=sin_rot*width;
		float sin_rot_h=sin_rot*height;

		// Texture coords
		uv[0].x=mid_u-cos_rot_w+sin_rot_h;
		uv[0].y=mid_v-sin_rot_w-cos_rot_h;

		// Texture coords
		uv[1].x=mid_u+cos_rot_w+sin_rot_h;
		uv[1].y=mid_v+sin_rot_w-cos_rot_h;

		// Texture coords
		uv[2].x=mid_u-cos_rot_w-sin_rot_h;
		uv[2].y=mid_v-sin_rot_w+cos_rot_h;

		// Texture coords
		uv[3].x=mid_u+cos_rot_w-sin_rot_h;
		uv[3].y=mid_v+sin_rot_w+cos_rot_h;

		for(int i=0;i<4;++i)
		{
			uv[i]*=mUVScale;
			uv[i]+=mUVOffset;
			// Left-top
			// Positions
			vertex[i].p.x=v[i].x+world_pos.x;
			vertex[i].p.y=v[i].y+world_pos.y;
			vertex[i].p.z=v[i].z+world_pos.z;
			vertex[i].color=color;
			vertex[i].tu=uv[i].x;
			vertex[i].tv=uv[i].y;
		}
	}
}
