#pragma once
#include "BaseObject.h"
#include "ParticleElement.h"

namespace ParaEngine
{
	class CScriptParticle
		:public CBaseObject
	{
	public:
		struct SParticleElement
			:public CParticleElement
		{
			Vector3 mPosition;
			Vector2 mSize;
			Vector2 mUVOffset;
			LinearColor mColour;
			asset_ptr<TextureEntity> mTexture;
			Vector3 m_vRenderOffset;
			void draw(SceneState * sceneState);
			// Í¨¹ý CParticleElement ¼Ì³Ð
			virtual int RenderParticle(SPRITEVERTEX ** pVertexBuffer,SceneState * pSceneState) override;
			virtual TextureEntity * GetTexture() override;
		protected:
			ParaEngine::Vector3 GetRenderOffset() const { return m_vRenderOffset; }
			void SetRenderOffset(ParaEngine::Vector3 val) { m_vRenderOffset=val; }
		};
	protected:
		std::vector<SParticleElement*> mActiveElements;
		std::vector<SParticleElement*> mFreeElements;
		SParticleElement * mCurrentParticleElement;
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
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourR_s,float)	{ cls->setParticleColour(p1,cls->getParticleColour().g,cls->getParticleColour().b,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourG_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,p1,cls->getParticleColour().b,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourB_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,cls->getParticleColour().g,p1,cls->getParticleColour().a); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleColourA_s,float)	{ cls->setParticleColour(cls->getParticleColour().r,cls->getParticleColour().g,cls->getParticleColour().b,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleWidth_s,float)	{ cls->setParticleSize(p1,cls->getParticleSize().y); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleHeight_s,float)	{ cls->setParticleSize(cls->getParticleSize().x,p1); return S_OK; }
		ATTRIBUTE_METHOD1(CScriptParticle,setParticleTexture_s,const char*)	{ cls->setParticleTexture(p1); return S_OK; }
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
		void setParticleColour(float r,float g,float b,float a);
		const LinearColor & getParticleColour()const;
		void setParticleSize(float width,float height);
		const Vector2 & getParticleSize()const;
		void setParticleTexture(const string & filename);

	protected:
		SParticleElement*_createOreRetrieve();
	};
}