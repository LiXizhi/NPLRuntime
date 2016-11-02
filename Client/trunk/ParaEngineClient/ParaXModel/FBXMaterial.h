#pragma once

namespace ParaEngine
{
	struct FBXMaterial
	{
	public:
		FBXMaterial()
		{
			nIsReplaceableIndex = -1;
			bHasOpacityMap = false;
			bHasBumpMap = false;
			bHasLightMap = false;
			bHasDisplacementMap = false;
			bHasReflectionMap = false;
			bUseCubicMap = false;
			bDisableZWrite = false;
			bDisablePhysics = false;
			bForcePhysics = false;
			bForceLocalTranparency = false;
			fReflectivity = 1.0f;
			fAlphaTestingRef = 0.5f;
			bAnimUV = false;
			bUnlit = false;
			bAddictive = false;
			bBillboard = false;
			bAABillboard = false;
			nForceUnique = 0;
			m_nCategoryID = 0;
			bRGBOnlyUVAnim = false;
			bRotateParticle2SpeedVector = false;
			fStripLenght = 0;
			bFacegroupSknningAni = false;
			nIndex = -1;
			m_nOrder = 0;
		}
	public:
		bool isAlphaBlended() {return fAlphaTestingRef == 0.f; }
		const std::string& GetFileName() { return m_filename; };

		operator const std::string& () { return m_filename; }

		/* material category id, may be used by some deferred shading shader. default to 0*/
		int32 GetCategoryID() const { return m_nCategoryID; }
		void SetCategoryID(int32 val) { m_nCategoryID = val; }

	public:
		std::string m_filename;
		// LiXizhi added
		// -1 if it is not replaceable texture.
		int nIsReplaceableIndex;
		// if true, it contains the opacity map
		bool bHasOpacityMap;
		// if true, it contains the bump map
		bool bHasBumpMap;
		// if true, it contains the bump map
		bool bHasLightMap;
		// if true, it contains the displacement map
		bool bHasDisplacementMap;
		// if true, it contains the reflection map
		// this is mutually exclusively with the bUseCubicMap
		bool bHasReflectionMap;
		// if true, it contains the cubic environment map in the reflection map of standard material.
		// this is mutually exclusively with the bHasReflectionMap
		bool bUseCubicMap;
		// whether z buffer is disabled for the given material, default is false, unless you are working on special material such as particles. 
		bool bDisableZWrite;
		// if this is true, we will not sort transparent mesh(with bDisableZWrite) globally, instead it will only be sorted per mesh, hence greatly increased rendering speed. 
		bool bForceLocalTranparency;
		// if true, physics of this material will be disabled.
		bool bDisablePhysics;
		// if true, we will force physics and ignore all physics from other materials 
		bool bForcePhysics;
		// reflection or environment map's reflectivity
		float fReflectivity;
		// alpha testing threshold, if this is 0, alpha testing is disabled. if this is 0.5, alpha testing is enabled. 
		// although this value can be [0,1), the game engine currently only support 0 or 0.5 alpha testing. i.e. boolean value.
		float fAlphaTestingRef;
		// if this is false, it means that no UV animation is used, otherwise UVAnimations contains the model's tex animations.
		bool bAnimUV;
		//if true, only texture alpha channel will have ui animation. --clayman 2011.8.4
		bool bRGBOnlyUVAnim;
		// if true, no lighting is applied to the surface. default to false
		bool bUnlit;
		// if true, additive alpha or one alpha blending is enabled. if bHasOpacityMap, it is BM_ADDITIVE_ALPHA, otherwise, it is BM_ADDITIVE
		bool bAddictive;
		// billboarded
		bool bBillboard;
		// axis aligned billboard, always up. 
		bool bAABillboard;
		//rotate particle align
		bool bRotateParticle2SpeedVector;

		//for fixed texture trip animaiton;
		float fStripLenght;

		bool bFacegroupSknningAni;

		// this will force this material to be unique, since the exporter will materials with the same parameter. 
		int nForceUnique;

		/* material category id, may be used by some deferred shading shader. default to 0*/
		int32 m_nCategoryID;
		int32 m_nOrder;
		
		int nIndex; // index of the material, -1 means no texture associated. 

		// the model transparency animation block. 
		ModelTransparency m_transparency_anim;

		// emissive color animation block. 
		ModelColor m_colors_anim;
	};
}