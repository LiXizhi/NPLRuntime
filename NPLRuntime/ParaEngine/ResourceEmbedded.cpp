//-----------------------------------------------------------------------------
// Class:	Resource Embedded
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2017.10.7
// Desc:  Add statically embedded resource name to data mapping here. 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "IO/ResourceEmbedded.h"


using namespace ParaEngine;

	/** only loaded*/
	CStaticInitRes::CStaticInitRes() {
#if defined(USE_DIRECTX_RENDERER)
		ADD_RESOURCE(":IDR_PARAXTEMPLATE", ParaXmodel_templates);
		ADD_RESOURCE(":IDR_DEFAULT_CURSOR", cursor_tga);
		ADD_RESOURCE(":IDR_PARAWORLD_ICO", paraworld_ico);
		ADD_RESOURCE(":IDR_FX_OCEANWATER", ocean_water_fxo);
		ADD_RESOURCE(":IDR_FX_OCCLUSION_TEST", occlusion_test_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH", simple_mesh_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL", simple_mesh_normal_fxo);
		ADD_RESOURCE(":IDR_FX_SMESH_CTOR", smesh_ctor_fxo);
		ADD_RESOURCE(":IDR_FX_TERRAIN_NORMAL", terrain_normal_fxo);
		ADD_RESOURCE(":IDR_FX_FULLSCREEN_GLOW", fullscreen_glow_fxo);
		ADD_RESOURCE(":IDR_FX_SKY", sky_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_SHADOW", simple_mesh_normal_shadow_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_TEX2", simple_mesh_normal_tex2_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED", simple_mesh_normal_selected_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_BORDER", simple_mesh_normal_border_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_LOW", simple_mesh_normal_low_fxo);
		ADD_RESOURCE(":IDR_FX_WATER_RIPPLE", water_ripples_fxo);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_HIGH", ocean_water_high_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED", simple_mesh_normal_instanced_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT", simple_mesh_normal_unlit_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_PARTICLE", simple_particle_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_CAD_MODEL", simple_cad_model_fxo);
		ADD_RESOURCE(":IDR_FX_DROPSHADOW", dropShadow_fxo);
		ADD_RESOURCE(":IDR_FX_SCREENWAVE", screenWave_fxo);
		ADD_RESOURCE(":IDR_FX_BLOCK", blockEffect_fxo);
		ADD_RESOURCE(":IDR_FX_SHADOWMAP_BLUR", shadowmap_blur_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_VEGETATION", simple_mesh_normal_vegetation_fxo);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS", simple_mesh_normal_specialeffects_fxo);
		ADD_RESOURCE(":IDR_FX_SKYDOME", skydome_fxo);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_QUAD", ocean_water_quad_fxo);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_CLOUD", ocean_water_cloud_fxo);
		ADD_RESOURCE(":IDR_FX_BMAXMODEL", BMaxModel_fxo);
#endif

#ifdef USE_OPENGL_RENDERER
		ADD_RESOURCE(":IDR_FX_BLOCK", blockEffect_fx);
		ADD_RESOURCE(":IDR_FX_SINGLECOLOR", singleColorEffect_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL", simple_mesh_normal_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_PARTICLE", simple_particle_fx);
		ADD_RESOURCE(":IDR_FX_GUI", GuiEffect_fx);
		ADD_RESOURCE(":IDR_FX_GUI_TEXT", GuiTextEffect_fx);
		ADD_RESOURCE(":IDR_FX_SKY", sky_fx);
		ADD_RESOURCE(":IDR_FX_SKYDOME", skydome_fx);
		ADD_RESOURCE(":IDR_FX_TERRAIN_NORMAL", terrain_normal_fx);
		ADD_RESOURCE(":IDR_FX_BMAXMODEL", BMaxModel_fx);
		ADD_RESOURCE(":IDR_FX_REDBLUESTEREO", RedBlueStereo_fx);
#endif // USE_OPENGL_RENDERER


	}

	CStaticInitRes& CStaticInitRes::StaticInit(){
		static CStaticInitRes _embedded_resource;
		return _embedded_resource;
	}







