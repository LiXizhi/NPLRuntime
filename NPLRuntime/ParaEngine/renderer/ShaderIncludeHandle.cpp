#include "ParaEngine.h"
#include "ShaderIncludeHandle.h"
using namespace ParaEngine;

ParaEngine::ShaderIncludeHandle::ShaderIncludeHandle(const std::string& sDir)
{
	m_ShaderDir = sDir + "/";
}

bool ParaEngine::ShaderIncludeHandle::Open(const std::string& filename, void** ppData, uint32_t* pBytes)
{
	std::string path = m_ShaderDir + filename;
	CParaFile file(path.c_str());
	if (file.isEof())
	{
		return false;
	}


	void* buffer = malloc(file.getSize());
	memcpy(buffer, file.getBuffer(), file.getSize());

	*ppData = buffer;
	*pBytes = file.getSize();

	return true;
}

void ParaEngine::ShaderIncludeHandle::Close(void* pData)
{
	if (pData)
		free((void*)pData);
}
