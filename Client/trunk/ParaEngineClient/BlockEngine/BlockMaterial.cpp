//-----------------------------------------------------------------------------
// Class:	Block Material
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2022.11.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockMaterial.h"

using namespace ParaEngine;


CBlockMaterial::CBlockMaterial() 
{
	// m_SharedParamBlock.CreateGetParameter("BaseColor")->SetType(CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
	// m_SharedParamBlock.CreateGetParameter("Metallic")->SetType(CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	// m_SharedParamBlock.CreateGetParameter("Specular")->SetType(CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	// m_SharedParamBlock.CreateGetParameter("Roughness")->SetType(CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	// m_SharedParamBlock.CreateGetParameter("EmissiveColor")->SetType(CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
	// m_SharedParamBlock.CreateGetParameter("Opacity")->SetType(CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	// m_SharedParamBlock.CreateGetParameter("Normal")->SetType(CParameter::PARAMETER_TYPE::PARAM_TEXTURE_ENTITY);
}

CBlockMaterial::CBlockMaterial(const AssetKey& key) :AssetEntity(key) 
{
}

CBlockMaterial::~CBlockMaterial() {
}

CParameterBlock* CBlockMaterial::GetParamBlock(bool bCreateIfNotExist /*= false*/)
{
	return &m_SharedParamBlock;
}