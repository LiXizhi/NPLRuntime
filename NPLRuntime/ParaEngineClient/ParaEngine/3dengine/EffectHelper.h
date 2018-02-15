#pragma once

namespace ParaEngine
{
	class EffectManager;

	/** push/pop texture states */
	class EffectTextureStateStack
	{
	public:
		EffectTextureStateStack(EffectManager* pEffectManager, bool bPointTexture);;
		~EffectTextureStateStack();;
	public:
		DWORD m_nLastMinFilter;
		DWORD m_nLastMagFilter;
		DWORD m_dwFilter;
		EffectManager* m_pEffectManager;
	};
}