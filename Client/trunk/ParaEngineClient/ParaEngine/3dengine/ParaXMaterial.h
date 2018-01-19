#pragma once
#include "TextureEntity.h"

namespace ParaEngine
{
	/**
	* the material information for ParaX file
	*/
	struct CParaXMaterial
	{
		/// basic material information in the original DirectX format.
		ParaMaterial	mat;

		enum AttributeBits{
			/// whether to enable alpha blending, i.e. whether the texture alpha is used for blending.
			MATERIAL_BLENDING = 0x1,
			/// whether to enable alpha testing. i.e. whether the texture alpha is used for alpha testing.
			MATERIAL_ALPHATESTING = 0x1 << 1,
			/// whether to turn off z buffer write, when it is rendered with alpha blending.
			/// if this attribute bit is set, z-buffer write will be turned off when the material has blending attribute.
			MATERIAL_ZMASKING = 0x1 << 2,

			/// whether has normal mapping. The normal map always has the same texture file name plus a "Norm". 
			/// e.g. main texture is "face.dds", normal map is "faceNorm.dds".
			MATERIAL_NORMALMAP = 0x1 << 3,
			/// whether do environment mapping. The reflectivity of the map is encoded in the alpha value of specular light color. 
			/// The environment map always has the same texture file name plus a "Env".
			/// e.g. main texture is "face.dds", environment map is "faceEnv.dds".
			MATERIAL_ENVIRONMENTMAP = 0x1 << 4,
			/// whether do reflection mapping. The reflectivity of the map is encoded in the red value of the emissive color. 
			MATERIAL_REFLECTION = 0x1 << 5,
			/// whether do light mapping. The light map always has the same texture file name plus a "LightMap".
			/// the mesh must contain a second set of UV in order to use light map.
			MATERIAL_LIGHTMAP = 0x1 << 6,
			/// whether physics is disabled.
			MATERIAL_DISABLE_PHYSICS = 0x1 << 7,
			/// whether lighting is disabled.
			MATERIAL_DISABLE_LIGHTING = 0x1 << 8,
			/// whether additive alpha or one alpha blending is enabled. if MATERIAL_BLENDING, it is ADDITIVE_ALPHA, otherwise, it is ADDITIVE_ONE
			MATERIAL_ADDITIVE = 0x1 << 9,
			/// billboarded
			MATERIAL_BILLBOARD = 0x1 << 10,
			/// axis aligned billboarded
			MATERIAL_BILLBOARD_AA = 0x1 << 11,

			/////////////////////////////////////////
			/// (added automatically) whether use translucent sorting. If set, it will use face group sorting for translucent face groups. 
			MATERIAL_TRANSLUCENT_SORT = 0x1 << 12,
			/// (added automatically if mesh texture file ends with _pg[1-9].xxx). fully transparent mesh face such as clickthrough mesh physics faces.
			/// skip rendering (in non-debug mode) regardless of what the texture is. 
			MATERIAL_SKIP_RENDER = 0x1 << 13,
		};
		/** a bitwise mask of AttributeBits. */
		DWORD dwAttribute;

		/// first texture.
		asset_ptr<TextureEntity> m_pTexture1;
		/// secondary texture: such as cube map and normal map. 
		asset_ptr<TextureEntity> m_pTexture2;
		/** if this is -1, no replaceable texture is used. if not, it is the ID or index into the replaceable texture array.
		generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		*/
		int m_nReplaceableTextureID;

		/** index into the mesh object's m_faceGroups. This value is only valid when MATERIAL_TRANSLUCENT_SORT attribute is set. */
		int m_nTranslucentFaceGroupIndex;
		/** the physics group. default to 0. */
		int m_nPhysicsGroup;
		/** reflection surface height in model space */
		float m_fRelectionHeight;
		/** only used in billboarded and axis aligned billboarded materials. */
		Vector3 m_vPivot;

	public:
		/** default constructor*/
		CParaXMaterial() :m_nReplaceableTextureID(-1), dwAttribute(0), m_nTranslucentFaceGroupIndex(-1), m_fRelectionHeight(0.f), m_vPivot(0, 0, 0), m_nPhysicsGroup(0){};
		~CParaXMaterial(){};

		/**Get the reflectivity of environment or reflection map. */
		float GetReflectivity(){ return mat.Emissive.r; }

		/**Set the reflectivity of environment or reflection map. */
		void SetReflectivity(float fReflectivity){ mat.Emissive.r = fReflectivity; }

		inline bool hasAlphaBlending(){ return CheckAttribute(MATERIAL_BLENDING); };
		inline bool hasAlphaTesting(){ return CheckAttribute(MATERIAL_ALPHATESTING); };
		inline bool hasZMask(){ return CheckAttribute(MATERIAL_ZMASKING); };

		inline bool hasNormalMap(){ return CheckAttribute(MATERIAL_NORMALMAP); };
		inline bool hasEnvironmentMap(){ return CheckAttribute(MATERIAL_ENVIRONMENTMAP); };
		inline bool hasReflectionMap(){ return CheckAttribute(MATERIAL_REFLECTION); };
		inline bool hasLightMap(){ return CheckAttribute(MATERIAL_LIGHTMAP); };
		inline bool hasPhysics(){ return !CheckAttribute(MATERIAL_DISABLE_PHYSICS); };
		inline bool hasLighting(){ return !CheckAttribute(MATERIAL_DISABLE_LIGHTING); };
		inline bool hasTranslucentSort(){ return CheckAttribute(MATERIAL_TRANSLUCENT_SORT); };
		inline bool hasAdditive(){ return CheckAttribute(MATERIAL_ADDITIVE); };

		inline bool hasAnyBillboard(){ return CheckAttribute(MATERIAL_BILLBOARD | MATERIAL_BILLBOARD_AA); };
		inline bool hasBillboard(){ return CheckAttribute(MATERIAL_BILLBOARD); };
		inline bool hasBillboardAA(){ return CheckAttribute(MATERIAL_BILLBOARD_AA); };
		inline bool isSkipRendering(){ return CheckAttribute(MATERIAL_SKIP_RENDER); };
		/**
		* whether a material attribute is enabled.
		* @param attribute
		* @return true if enabled
		*/
		inline bool CheckAttribute(DWORD attribute){
			return (dwAttribute & (DWORD)attribute) > 0;
		}

		/** enable or disable a given attribute */
		void EnableAttribute(DWORD attribute, bool bTurnOn = true){
			if (bTurnOn)
				dwAttribute |= attribute;
			else
				dwAttribute &= ~attribute;
		}

		/** get the physics group. */
		int GetPhysicsGroup(){ return m_nPhysicsGroup; }

		/** read from material, whether to use blend, alpha test or global textures, etc.
		* these information is currently encoded in the bits of specular power component of mat.
		* TODO: I guess X file reference will support it in its future release.
		* or maybe I shall manually parse X-file to add explicit support.
		*/
		void SetMaterial(const ParaMaterial& material){
			mat = material;
#define MAX_MATERIAL_POWER				1000.f
			dwAttribute = (int)(mat.Power / MAX_MATERIAL_POWER);
			mat.Power -= dwAttribute*MAX_MATERIAL_POWER;
		}
	};

}