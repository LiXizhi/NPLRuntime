#pragma once
#include "Framework/Interface/Render/IEffect.h"
namespace ParaEngine
{
	class ShaderIncludeHandle :public IParaEngine::IEffectInclude
	{

	public:
		ShaderIncludeHandle(const std::string& sDir);
		virtual bool Open(const std::string& filename, void** ppData, uint32_t* pBytes) override;
		virtual void Close(void* pData) override;

	private:
		std::string m_ShaderDir;

	};


}