#include "EffectOpenGL.h"
using namespace ParaEngine;
using namespace IParaEngine;
EffectOpenGL::~EffectOpenGL()
{

}
bool EffectOpenGL::GetDesc(IParaEngine::EffectDesc* pOutDesc)
{


	return false;
}

ParaEngine::EffectOpenGL::EffectOpenGL()
{

}

IParaEngine::TechniqueHandle ParaEngine::EffectOpenGL::GetTechnique(uint32_t index)
{
	IParaEngine::TechniqueHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	return handle;

}

bool EffectOpenGL::GetTechniqueDesc(const IParaEngine::TechniqueHandle& handle, IParaEngine::TechniqueDesc* pOutDesc)
{
	if (!pOutDesc)return false;
	if (isValidHandle(handle))
	{

	}
	return false;
}

IParaEngine::ParameterHandle ParaEngine::EffectOpenGL::GetParameter(uint32_t index)
{
	IParaEngine::ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	return handle;
}

bool ParaEngine::EffectOpenGL::GetParameterDesc(const IParaEngine::ParameterHandle & handle, IParaEngine::ParameterDesc * pOutDesc)
{
	if (pOutDesc == NULL)return false;
	if (!isValidHandle(handle))return false;


	return false;
}



bool ParaEngine::EffectOpenGL::SetMatrixArray(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
}


bool ParaEngine::EffectOpenGL::SetMatrix(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data)
{
	if (!isValidHandle(handle))return false;
}


bool ParaEngine::EffectOpenGL::SetVectorArray(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
}


bool ParaEngine::EffectOpenGL::SetFloatArray(const ParameterHandle& handle, const float* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
}


bool ParaEngine::EffectOpenGL::SetValue(const ParameterHandle& handle, const void* data, uint32_t size)
{
	if (!isValidHandle(handle))return false;
}


bool ParaEngine::EffectOpenGL::SetBool(const ParameterHandle& handle, bool value)
{
	if (!isValidHandle(handle))return false;

}


bool ParaEngine::EffectOpenGL::SetInt(const ParameterHandle& handle, int value)
{
	if (!isValidHandle(handle))return false;

}


bool ParaEngine::EffectOpenGL::SetFloat(const ParameterHandle& handle, float value)
{
	if (!isValidHandle(handle))return false;

}


bool ParaEngine::EffectOpenGL::SetTexture(const ParameterHandle& handle, ParaEngine::DeviceTexturePtr_type texture)
{
	if (!isValidHandle(handle))return false;

}


bool ParaEngine::EffectOpenGL::SetRawValue(const ParameterHandle& handle, const void* data, uint32_t offset, uint32_t size)
{
	if (!isValidHandle(handle))return false;

}


IParaEngine::ParameterHandle ParaEngine::EffectOpenGL::GetParameterByName(const char* name)
{
	ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	return handle;
}


bool ParaEngine::EffectOpenGL::SetVector(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data)
{
	if (!isValidHandle(handle))return false;

}


void ParaEngine::EffectOpenGL::OnLostDevice()
{
	
}


void ParaEngine::EffectOpenGL::OnResetDevice()
{
	
}


bool ParaEngine::EffectOpenGL::Begin()
{
	return false;
}


bool ParaEngine::EffectOpenGL::BeginPass(const uint8_t pass)
{
	return false;
}
bool ParaEngine::EffectOpenGL::EndPass()
{
	return false;
}


bool ParaEngine::EffectOpenGL::End()
{
	return false;
}


IParaEngine::TechniqueHandle ParaEngine::EffectOpenGL::GetCurrentTechnique()
{
	TechniqueHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	
	return handle;
}


bool ParaEngine::EffectOpenGL::CommitChanges()
{
	return false;
}

bool ParaEngine::EffectOpenGL::SetRawValue(const char* name, const void* data, uint32_t offset, uint32_t size)
{
	return false;
}

bool ParaEngine::EffectOpenGL::SetTexture(const char* name, ParaEngine::DeviceTexturePtr_type texture)
{
	return false;
}

bool EffectOpenGL::SetTechnique(const TechniqueHandle& handle)
{
	if (isValidHandle(handle))
	{

	}
	return false;
}

