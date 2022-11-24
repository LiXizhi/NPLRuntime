#pragma once
#include <string>
#include <vector>
#include <list>
#include "AssetEntity.h"
#include "ParameterBlock.h"
#include "IObjectScriptingInterface.h"

namespace ParaEngine
{
	using namespace std;
	class CSunLight;

	/**
	* asset entity: CBlockMaterial
	*/
	class CBlockMaterial : public AssetEntity, public IObjectScriptingInterface
	{
	public:
		/** */
		enum MaterialChannel
		{
			MaterialChannel_BaseColor = 0,
			MaterialChannel_Metallic,
			MaterialChannel_Specular,
			MaterialChannel_Roughness,
			MaterialChannel_EmissiveColor,
			MaterialChannel_Opacity,
			// MaterialChannel_OpacityMask,
			MaterialChannel_Normal,
			MaterialChannel_Diffuse,
		};

	public:
		CBlockMaterial();
		CBlockMaterial(const AssetKey& key);

		void InitParamBlock();
		virtual  ~CBlockMaterial();

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_BlockMaterial; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "BlockMaterial"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		ATTRIBUTE_METHOD1(CBlockMaterial, SetMaterialName_s, const char*)		{ *(cls->GetParamBlock()->CreateGetParameter("MaterialName")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetMaterialName_s, const char**)		{ *p1 = (const char*)(*(cls->GetParamBlock()->CreateGetParameter("MaterialName"))); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetBaseColor_s, Vector4)		{ *(cls->GetParamBlock()->CreateGetParameter("BaseColor")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetBaseColor_s, Vector4*)		{ *p1 = *((Vector4*)(cls->GetParamBlock()->CreateGetParameter("BaseColor")->GetRawData())); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetMetallic_s, float)		{ *(cls->GetParamBlock()->CreateGetParameter("Metallic")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetMetallic_s, float*)		{ *p1 = (float)(*(cls->GetParamBlock()->CreateGetParameter("Metallic"))); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetSpecular_s, float)		{ *(cls->GetParamBlock()->CreateGetParameter("Specular")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetSpecular_s, float*)		{ *p1 = (float)(*(cls->GetParamBlock()->CreateGetParameter("Specular"))); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetRoughness_s, float)		{ *(cls->GetParamBlock()->CreateGetParameter("Roughness")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetRoughness_s, float*)		{ *p1 = (float)(*(cls->GetParamBlock()->CreateGetParameter("Roughness"))); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetEmissiveColor_s, Vector4)		{ *(cls->GetParamBlock()->CreateGetParameter("EmissiveColor")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetEmissiveColor_s, Vector4*)		{ *p1 = *((Vector4*)(cls->GetParamBlock()->CreateGetParameter("EmissiveColor")->GetRawData())); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetEmissive_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("Emissive")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetEmissive_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("Emissive")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetEmissiveFullPath_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("EmissiveFullPath")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetEmissiveFullPath_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("EmissiveFullPath")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetOpacity_s, float)		{ *(cls->GetParamBlock()->CreateGetParameter("Opacity")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetOpacity_s, float*)		{ *p1 = (float)(*(cls->GetParamBlock()->CreateGetParameter("Opacity"))); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetNormal_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("Normal")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetNormal_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("Normal")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetNormalFullPath_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("NormalFullPath")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetNormalFullPath_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("NormalFullPath")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetDiffuse_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("Diffuse")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetDiffuse_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("Diffuse")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetDiffuseFullPath_s, const char*)		{ cls->GetParamBlock()->CreateGetParameter("DiffuseFullPath")->SetValueByString(p1, CParameter::PARAMETER_TYPE::PARAM_STRING); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetDiffuseFullPath_s, const char**)		{ static std::string v; v = cls->GetParamBlock()->CreateGetParameter("DiffuseFullPath")->GetValueByString(); *p1 = v.c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, SetMaterialUV_s, Vector4)		{ *(cls->GetParamBlock()->CreateGetParameter("MaterialUV")) = p1; return S_OK; }
		ATTRIBUTE_METHOD1(CBlockMaterial, GetMaterialUV_s, Vector4*)		{ *p1 = *((Vector4*)(cls->GetParamBlock()->CreateGetParameter("MaterialUV")->GetRawData())); return S_OK; }
		virtual AssetType GetType() { return blockmaterial; };
		virtual HRESULT InitDeviceObjects() { return S_OK; };
		virtual HRESULT RestoreDeviceObjects() { return S_OK; };
		virtual HRESULT InvalidateDeviceObjects() { return S_OK; };
		virtual HRESULT DeleteDeviceObjects() { return S_OK; };

		/** get effect parameter block with this object.
		* @param bCreateIfNotExist:
		*/
		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

	private:
		/** parameters that are shared by all objects using this effect.  When you call begin(), these parameters are set.
		* It will overridden predefined parameter names, however most user will set custom shader parameters and textures that are shared by all objects here.
		*/
		CParameterBlock m_SharedParamBlock;

		// BaseColor  vector4
		// Metallic   float
		// Specular   float
		// Roughness  float
		// EmissiveColor vector4
		// Opacity float
		// Normal texture
	};
}
