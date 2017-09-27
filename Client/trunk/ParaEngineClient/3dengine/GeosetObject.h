#pragma once
#include "BaseObject.h"
#include "ParaXEntity.h"
#include "BipedObject.h"
#include "PERefPtr.h"
#include "PEtypes.h"
namespace ParaEngine
{
	class CGeosetObject: public CBaseObject
	{
	protected:
		asset_ptr<ParaXEntity> mEntity;
		CBipedObject * mParent;
		std::map<uint32,TextureEntity*> mReplaceTextures;
	public:
		ATTRIBUTE_DEFINE_CLASS(CGeosetObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CGeosetObject);
		CGeosetObject();
		virtual ~CGeosetObject();
		void attachEntity(ParaXEntity * entity)
		{
			mEntity=entity;
		}
		ParaXEntity * getEntity()const
		{
			return mEntity.get();
		}
		virtual void SetParent(CBaseObject* pParent) override
		{
			mParent=static_cast<CBipedObject*>(pParent);
		}
		virtual CBaseObject* GetParent() override
		{
			return mParent;
		}
		virtual void SetAssetFileName(const std::string& sFilename) override;
		virtual HRESULT Draw(SceneState * sceneState) override;
		virtual bool SetReplaceableTexture(int ReplaceableTextureID,TextureEntity* pTextureEntity) override;
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID) override;
	protected:
		void _draw(SceneState * sceneState,Matrix4 * mxWorld,CParameterBlock* params=NULL);
	};
}