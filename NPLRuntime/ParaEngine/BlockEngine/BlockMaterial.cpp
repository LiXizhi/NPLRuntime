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
	InitParamBlock();
}

CBlockMaterial::CBlockMaterial(const AssetKey& key) :AssetEntity(key) 
{
	InitParamBlock();
}

CBlockMaterial::~CBlockMaterial() {
}

void CBlockMaterial::InitParamBlock()
{
	// 设置材质名默认值
	// 世界保存(blockworld), 渲染着色器参数传递(blockworldclient render shader), 脚本获取设置(installField)
	m_SharedParamBlock.CreateGetParameter("MaterialName")->SetValueByString("", CParameter::PARAMETER_TYPE::PARAM_STRING);
	m_SharedParamBlock.CreateGetParameter("BaseColor")->SetValueByString("1.0, 1.0, 1.0, 1.0", CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
	m_SharedParamBlock.CreateGetParameter("Metallic")->SetValueByString("0.0", CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	m_SharedParamBlock.CreateGetParameter("Specular")->SetValueByString("0.0", CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	m_SharedParamBlock.CreateGetParameter("Roughness")->SetValueByString("0.0", CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	m_SharedParamBlock.CreateGetParameter("EmissiveColor")->SetValueByString("0.0, 0.0, 0.0, 0.0", CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
	m_SharedParamBlock.CreateGetParameter("Opacity")->SetValueByString("1.0", CParameter::PARAMETER_TYPE::PARAM_FLOAT);
	m_SharedParamBlock.CreateGetParameter("Normal")->SetValueByString("", CParameter::PARAMETER_TYPE::PARAM_STRING);    // 法线贴图
	m_SharedParamBlock.CreateGetParameter("Diffuse")->SetValueByString("", CParameter::PARAMETER_TYPE::PARAM_STRING);   // 漫反射贴图
	m_SharedParamBlock.CreateGetParameter("MaterialUV")->SetValueByString("1.0, 1.0, 0.0, 0.0", CParameter::PARAMETER_TYPE::PARAM_VECTOR4);   // 漫反射贴图
}

CParameterBlock* CBlockMaterial::GetParamBlock(bool bCreateIfNotExist /*= false*/)
{
	return &m_SharedParamBlock;
}

int CBlockMaterial::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	AssetEntity::InstallFields(pClass, bOverride);
	pClass->AddField("MaterialName", FieldType_String, (void*)SetMaterialName_s, (void*)GetMaterialName_s, NULL, NULL, bOverride);
	pClass->AddField("BaseColor", FieldType_Vector4, (void*)SetBaseColor_s, (void*)GetBaseColor_s, NULL, NULL, bOverride);
	pClass->AddField("Metallic", FieldType_Float, (void*)SetMetallic_s, (void*)GetMetallic_s, NULL, NULL, bOverride);
	pClass->AddField("Specular", FieldType_Float, (void*)SetSpecular_s, (void*)GetSpecular_s, NULL, NULL, bOverride);
	pClass->AddField("Roughness", FieldType_Float, (void*)SetRoughness_s, (void*)GetRoughness_s, NULL, NULL, bOverride);
	pClass->AddField("EmissiveColor", FieldType_Vector4, (void*)SetEmissiveColor_s, (void*)GetEmissiveColor_s, NULL, NULL, bOverride);
	pClass->AddField("Opacity", FieldType_Float, (void*)SetOpacity_s, (void*)GetOpacity_s, NULL, NULL, bOverride);
	pClass->AddField("Normal", FieldType_String, (void*)SetNormal_s, (void*)GetNormal_s, NULL, NULL, bOverride);
	pClass->AddField("Diffuse", FieldType_String, (void*)SetDiffuse_s, (void*)GetDiffuse_s, NULL, NULL, bOverride);
	pClass->AddField("MaterialUV", FieldType_String, (void*)SetMaterialUV_s, (void*)GetMaterialUV_s, NULL, NULL, bOverride);
	return S_OK;
}