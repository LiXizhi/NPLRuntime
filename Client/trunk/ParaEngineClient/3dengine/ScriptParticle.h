#pragma once
#include "TileObject.h"
#include "ParticleElement.h"

namespace ParaEngine
{
	class CScriptParticle
		:public CTileObject
	{
	public:
		enum BillboardRotationType
		{
			/// Rotate the billboard's vertices around their facing direction
			BBR_VERTEX,
			/// Rotate the billboard's texture coordinates
			BBR_TEXCOORD
		};
		enum BillboardType
		{
			/// Standard point billboard (default), always faces the camera completely and is always upright
			BBT_POINT,
			/// Billboards are oriented around a shared direction vector (used as Y axis) and only rotate around this to face the camera
			BBT_ORIENTED_COMMON,
			/// Billboards are oriented around their own direction vector (their own Y axis) and only rotate around this to face the camera
			BBT_ORIENTED_SELF,
			/// Billboards are perpendicular to a shared direction vector (used as Z axis, the facing direction) and X, Y axis are determined by a shared up-vertor
			BBT_PERPENDICULAR_COMMON,
			/// Billboards are perpendicular to their own direction vector (their own Z axis, the facing direction) and X, Y axis are determined by a shared up-vertor
			BBT_PERPENDICULAR_SELF
		};
		struct SParticleElement
			:public CParticleElement
		{
			CScriptParticle * const mContainer;
			Vector3 mPosition;
			Vector2 mSize;
			Vector2 mUVOffset;
			Vector2 mUVScale;
			LinearColor mColour;
			Vector3 m_vRenderOffset;
			float mRotation;
			SParticleElement(CScriptParticle * container);
			~SParticleElement();
			void draw(SceneState * sceneState);
			// Í¨¹ý CParticleElement ¼Ì³Ð
			virtual int RenderParticle(SPRITEVERTEX ** pVertexBuffer,SceneState * pSceneState) override;
			virtual TextureEntity * GetTexture() override;
		protected:
			ParaEngine::Vector3 GetRenderOffset() const { return m_vRenderOffset; }
			void SetRenderOffset(ParaEngine::Vector3 val) { m_vRenderOffset=val; }

			void genVertOffsets(float inleft,float inright,float intop,float inbottom,
				float width,float height,const Vector3& x,const Vector3& y,Vector3* pDestVec);
			void genBillboardAxes(Vector3* pX,Vector3 *pY,SceneState * pSceneState);
			void genVertices(const Vector3* const offsets,SPRITEVERTEX * vertex);
		};
	protected:
		std::vector<SParticleElement*> mActiveElements;
		std::vector<SParticleElement*> mFreeElements;
		SParticleElement * mCurrentParticleElement;

		asset_ptr<TextureEntity> mTexture;
		RECT mTextureRect;
		BillboardType mBillboardType;
		Vector3 mCommonDirection;
		Vector3 mCommonUpVector;
		bool mPointRendering;
		bool mAccurateFacing;
		BillboardRotationType mRotationType;
	public:
		ATTRIBUTE_DEFINE_CLASS(CScriptParticle);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CScriptParticle);
		virtual int InstallFields(CAttributeClass* pClass,bool bOverride) override;
		ATTRIBUTE_METHOD(CScriptParticle,clear_s)	{ cls->clear(); return S_OK; }
		ATTRIBUTE_METHOD(CScriptParticle,beginParticle_s)	{ cls->beginParticle(); return S_OK; }
		ATTRIBUTE_METHOD(CScriptParticle,endParticle_s)	{ cls->endParticle(); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticlePositionX_s,float)	{ cls->setParticlePosition(p1,cls->getParticlePosition().y,cls->getParticlePosition().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticlePositionY_s,float)	{ cls->setParticlePosition(cls->getParticlePosition().x,p1,cls->getParticlePosition().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticlePositionZ_s,float)	{ cls->setParticlePosition(cls->getParticlePosition().x,cls->getParticlePosition().y,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleUVOffsetX_s,float)	{ cls->setParticleUVOffset(p1,cls->getParticleUVOffset().y); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleUVOffsetY_s,float)	{ cls->setParticleUVOffset(cls->getParticleUVOffset().x,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleUVScaleX_s,float)	{ cls->setParticleUVScale(p1,cls->getParticleUVScale().y); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleUVScaleY_s,float)	{ cls->setParticleUVScale(cls->getParticleUVScale().x,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourR_s,float)	{ cls->setParticleColour(p1,cls->getParticleColour().g,cls->getParticleColour().b,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourG_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,p1,cls->getParticleColour().b,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourB_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,cls->getParticleColour().g,p1,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourA_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,cls->getParticleColour().g,cls->getParticleColour().b,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleWidth_s,float)	{ cls->setParticleSize(p1,cls->getParticleSize().y); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleHeight_s,float)	{ cls->setParticleSize(cls->getParticleSize().x,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleRotation_s,float)	{ cls->setParticleRotation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CScriptParticle,setTexture_s,const char*)	{ cls->setTexture(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setBillboardType_s,const char*)	{ cls->setBillboardType(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonDirectionX_s,float)	{ cls->setCommonDirection(p1,cls->getCommonDirection().y,cls->getCommonDirection().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonDirectionY_s,float)	{ cls->setCommonDirection(cls->getCommonDirection().x,p1,cls->getCommonDirection().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonDirectionZ_s,float)	{ cls->setCommonDirection(cls->getCommonDirection().x,cls->getCommonDirection().y,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonUpX_s,float)	{ cls->setCommonUp(p1,cls->getCommonUp().y,cls->getCommonUp().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonUpY_s,float)	{ cls->setCommonUp(cls->getCommonUp().x,p1,cls->getCommonUp().z); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setCommonUpZ_s,float)	{ cls->setCommonUp(cls->getCommonUp().x,cls->getCommonUp().y,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setPointRenderingEnable_s,bool)	{ cls->setPointRenderingEnable(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setAccurateFacing_s,bool)	{ cls->setAccurateFacing(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setRotationType_s,const char*)	{ cls->setRotationType(p1); return S_OK; }
		CScriptParticle();
		~CScriptParticle();

		virtual HRESULT Draw(SceneState * sceneState) override;

		void clear();
		void beginParticle();
		void endParticle();
		void setParticlePosition(float x,float y,float z);
		const Vector3 & getParticlePosition()const;
		void setParticleUVOffset(float u,float v);
		const Vector2 & getParticleUVOffset()const;
		void setParticleUVScale(float u,float v);
		const Vector2 & getParticleUVScale()const;
		void setParticleColour(float r,float g,float b,float a);
		const LinearColor & getParticleColour()const;
		void setParticleSize(float width,float height);
		const Vector2 & getParticleSize()const;
		void setParticleRotation(float radian);

		void setTexture(const string & filename);
		void setBillboardType(const std::string & type);
		void setCommonDirection(float x,float y,float z);
		const Vector3 & getCommonDirection()const{ return mCommonDirection; }
		void setCommonUp(float x,float y,float z);
		const Vector3 & getCommonUp()const{ return mCommonUpVector; }
		void setPointRenderingEnable(bool enable);
		void setAccurateFacing(bool facing);
		void setRotationType(const std::string & type);

	protected:
		SParticleElement*_createOreRetrieve();
	};
}