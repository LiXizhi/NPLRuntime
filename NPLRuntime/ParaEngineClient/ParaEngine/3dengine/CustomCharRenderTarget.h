#pragma once

#include "CustomCharCommon.h"
#include "CustomCharFace.h"
#include "TextureComposer.h"

namespace ParaEngine
{
	class CSkinLayers : public TextureComposeTask
	{
	public:
		CSkinLayers(const std::string& name, const std::vector <CharTextureComponent>& layers);
		virtual ~CSkinLayers();
	public:
		virtual bool IsAssetAllLoaded();
		/* in most cases, we will unload all used textures here. */
		virtual void OnTaskCompleted();;
		virtual void DoPaint(CPaintDevice* pd);
		
	protected:
		/** describing how to compose the texture. */
		std::vector <CharTextureComponent> m_layers;
	};

	class CFaceLayers : public TextureComposeTask
	{
	public:
		CFaceLayers(const std::string& name, CartoonFace& faceTexture);
		virtual ~CFaceLayers(){};
		virtual bool IsAssetAllLoaded();
		/* in most cases, we will unload all used textures here. */
		virtual void OnTaskCompleted();;
		virtual void DoPaint(CPaintDevice* pd);
	public:
		std::vector <FaceTextureComponent> m_layers;
	};
}
