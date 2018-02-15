#pragma once
#include "RenderTarget.h"
#include "TextureEntity.h"
namespace ParaEngine
{
	/** base class for texture composer using using a render target.
	*/
	class TextureComposeTask : public CRefCounted
	{
	public:
		virtual ~TextureComposeTask(){};
		virtual bool IsAssetAllLoaded() { return false; };
		/* in most cases, we will unload all used textures here. */
		virtual void OnTaskCompleted() {};
		virtual void DoPaint(CPaintDevice* pd) {};
		virtual TextureEntity* GetTexture();
		/**
		* @param colorKey: transparent color key. if 0, it means no color key. 
		*/
		bool UpdateToStaticTexture(CRenderTarget* pRenderTarget, Color colorKey = 0);

		const std::string& GetName() const { return m_name; }
		void SetName(const std::string& val) { m_name = val; }

		bool IsDirty() const { return m_dirty; }
		void SetDirty(bool val) { m_dirty = val; }


	protected:
		TextureComposeTask() :m_dirty(true){};

	protected:
		bool m_dirty;
		std::string m_name;
		ref_ptr<TextureEntity> m_composedTexture;
	};

	/** a special render target for composing the multilayer character skin or facial texture. */
	class TextureComposeRenderTarget : public CRenderTarget
	{
	public:
		TextureComposeRenderTarget();
		virtual ~TextureComposeRenderTarget();

		ATTRIBUTE_DEFINE_CLASS(TextureComposeRenderTarget);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(TextureComposeRenderTarget);
	public:
		void AddTask(TextureComposeTask* pTask);

		/** called when object is dirty. */
		virtual HRESULT Draw(SceneState * sceneState);
		virtual bool IsAssetAllLoaded();

		/** if we use opengl render target directly, we need to flipY during rendering. */
		bool IsFlipY() const { return m_bFlipY; }
		void SetFlipY(bool val) { m_bFlipY = val; }
		
		/*
		* @param colorKey: transparent color key. if 0, it means no color key.
		*/
		void SetColorKey(const Color& val) { m_colorKey = val; }
		Color GetColorKey() const { return m_colorKey; }

	protected:
		Color m_colorKey;
		bool m_bFlipY;

		/** all layers */
		std::map < std::string, ref_ptr<TextureComposeTask> > m_all_layers;
		std::map < std::string, ref_ptr<TextureComposeTask> > m_dirty_layers;
		/** pending to be composed layers. */
		std::vector < ref_ptr<TextureComposeTask> > m_pending_layers;
		std::vector < ref_ptr<TextureComposeTask> > m_completed_layers;
	};
}