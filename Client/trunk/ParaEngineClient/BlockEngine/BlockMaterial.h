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
			MaterialChannel_OpacityMask,
			MaterialChannel_Normal,
		};

	public:
		CBlockMaterial();
		CBlockMaterial(const AssetKey& key);
		virtual  ~CBlockMaterial();

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
	};
}
