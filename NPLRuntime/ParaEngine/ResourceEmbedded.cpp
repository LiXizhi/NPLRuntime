//-----------------------------------------------------------------------------
// Class:	Resource Embedded
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2017.10.7
// Desc:	Add statically embedded resource name to data mapping here.
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "IO/ResourceEmbedded.h"

using namespace ParaEngine;

CStaticInitRes::CStaticInitRes()
{
#ifndef NPLRUNTIME_SERVER
	// Core embedded templates & icons
	ADD_RESOURCE(":IDR_PARAXTEMPLATE", ParaXmodel_templates);
	ADD_RESOURCE(":IDR_DEFAULT_CURSOR", cursor_tga);
	ADD_RESOURCE(":IDR_PARAWORLD_ICO", paraworld_ico);
	ADD_RESOURCE(":IDR_DB_NPC_TABLE", db_create_npc_table_sql);
	ADD_RESOURCE(":IDR_NPL_CHARACTER_EVENTS", character_script_template_txt);

	// Embedded shader #include headers
	ADD_RESOURCE(":IDR_SHADER_INC_COMMON_SHADER_DATA", common_shader_data_h);
	ADD_RESOURCE(":IDR_SHADER_INC_LIGHT_SCATTERING_CONSTANTS", light_scattering_constants_h);
	ADD_RESOURCE(":IDR_SHADER_INC_MESH_LOCAL_LIGHTS", mesh_local_lights_h);
	ADD_RESOURCE(":IDR_SHADER_INC_PATCH_DEFORM", patch_deform_h);
	ADD_RESOURCE(":IDR_SHADER_INC_SIMPLE_MESH_NORMAL_VEGETATION", simple_mesh_normal_vegetation_h);
	ADD_RESOURCE(":IDR_SHADER_INC_COMMON_DEFINE", CommonDefine_h);
	ADD_RESOURCE(":IDR_SHADER_INC_COMMON_FUNCTION", CommonFunction_h);
	ADD_RESOURCE(":IDR_SHADER_INC_SCREEN_SPACE_REFLECTION", screen_space_reflection_h);
	ADD_RESOURCE(":IDR_SHADER_INC_FXAA", FXAA_h);

	// Unified ParaRHI .fx shaders (core, GPU-skinned, instanced, Fancy MRT & post-processing)
	ADD_RESOURCE(":IDR_FX_BMAXMODEL", BMaxModel_fx);
	ADD_RESOURCE(":IDR_FX_BMAX_MODEL_SKINNED", bmax_model_skinned_fx);
	ADD_RESOURCE(":IDR_FX_CARTOON_EFFECT", CartoonEffect_fx);
	ADD_RESOURCE(":IDR_FX_CHARACTER_SKINNED", character_skinned_fx);
	ADD_RESOURCE(":IDR_FX_CHARACTER_SKINNED_LOW", character_skinned_low_fx);
	ADD_RESOURCE(":IDR_FX_COLOR_EFFECT", ColorEffect_fx);
	ADD_RESOURCE(":IDR_FX_COMPOSITE", composite_fx);
	ADD_RESOURCE(":IDR_FX_COMPOSITE_TERRAIN", compositeTerrain_fx);
	ADD_RESOURCE(":IDR_FX_COMPOSITE_LITE", compositeLite_fx);
	ADD_RESOURCE(":IDR_FX_DEFERRED_DIRECTIONAL_LIGHTING", DeferredShadingDirectionalLighting_fx);
	ADD_RESOURCE(":IDR_FX_DEFERRED_POINT_LIGHTING", DeferredShadingPointLighting_fx);
	ADD_RESOURCE(":IDR_FX_DEFERRED_SPOT_LIGHTING", DeferredShadingSpotLighting_fx);
	ADD_RESOURCE(":IDR_FX_FINAL", final_fx);
	ADD_RESOURCE(":IDR_FX_GREY", Grey_fx);
	ADD_RESOURCE(":IDR_FX_GREY_BLUR", GreyBlur_fx);
	ADD_RESOURCE(":IDR_FX_GUI_SIMPLE", GUI_simple_fx);
	ADD_RESOURCE(":IDR_FX_GUI", GuiEffect_fx);
	ADD_RESOURCE(":IDR_FX_GUI_BATCH", GuiBatch_fx);
	ADD_RESOURCE(":IDR_FX_GUI_TEXT", GuiTextEffect_fx);
	ADD_RESOURCE(":IDR_FX_MRT_BLOCKS", mrt_blocks_fx);
	ADD_RESOURCE(":IDR_FX_MRT_BMAX_MODEL", mrt_bmax_model_fx);
	ADD_RESOURCE(":IDR_FX_MRT_BMAX_MODEL_SKINNED", mrt_bmax_model_skinned_fx);
	ADD_RESOURCE(":IDR_FX_MRT_CHARACTER_SKINNED", mrt_character_skinned_fx);
	ADD_RESOURCE(":IDR_FX_MRT_MESH_NORMAL", mrt_mesh_normal_fx);
	ADD_RESOURCE(":IDR_FX_MRT_MESH_NORMAL_INSTANCED", mrt_mesh_normal_instanced_fx);
	ADD_RESOURCE(":IDR_FX_MRT_TERRAIN_NORMAL", mrt_terrain_normal_fx);
	ADD_RESOURCE(":IDR_FX_ODS_STEREO_SINGLE_EYE", ODSStereoSingleEye_fx);
	ADD_RESOURCE(":IDR_FX_REDBLUESTEREO", RedBlueStereo_fx);
	ADD_RESOURCE(":IDR_FX_SCENE_EFFECT", SceneEffect_fx);
	ADD_RESOURCE(":IDR_FX_SUPER_MULTIVIEW_STEREO", SuperMultiviewStereo_fx);
	ADD_RESOURCE(":IDR_FX_VIGNETTING", Vignetting_fx);
	ADD_RESOURCE(":IDR_FX_BLOCK", blockEffect_fx);
	ADD_RESOURCE(":IDR_FX_DROPSHADOW", dropShadow_fx);
	ADD_RESOURCE(":IDR_FX_FULLSCREEN_GLOW", fullscreen_glow_fx);
	ADD_RESOURCE(":IDR_FX_OCCLUSION_TEST", occlusion_test_fx);
	ADD_RESOURCE(":IDR_FX_OCEANWATER", ocean_water_fx);
	ADD_RESOURCE(":IDR_FX_OCEANWATER_CLOUD", ocean_water_cloud_fx);
	ADD_RESOURCE(":IDR_FX_OCEANWATER_HIGH", ocean_water_high_fx);
	ADD_RESOURCE(":IDR_FX_OCEANWATER_QUAD", ocean_water_quad_fx);
	ADD_RESOURCE(":IDR_FX_SCREENWAVE", screenWave_fx);
	ADD_RESOURCE(":IDR_FX_SHADOWMAP_BLUR", shadowmap_blur_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_VS", simple_VS_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_CAD_MODEL", simple_cad_model_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH", simple_mesh_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL", simple_mesh_normal_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_BORDER", simple_mesh_normal_border_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED", simple_mesh_normal_instanced_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_LOW", simple_mesh_normal_low_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED", simple_mesh_normal_selected_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_SHADOW", simple_mesh_normal_shadow_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS", simple_mesh_normal_specialeffects_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_TEX2", simple_mesh_normal_tex2_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT", simple_mesh_normal_unlit_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_VEGETATION", simple_mesh_normal_vegetation_fx);
	ADD_RESOURCE(":IDR_FX_SIMPLE_PARTICLE", simple_particle_fx);
	ADD_RESOURCE(":IDR_FX_SINGLECOLOR", singleColorEffect_fx);
	ADD_RESOURCE(":IDR_FX_SKY", sky_fx);
	ADD_RESOURCE(":IDR_FX_SKYDOME", skydome_fx);
	ADD_RESOURCE(":IDR_FX_SMESH_CTOR", smesh_ctor_fx);
	ADD_RESOURCE(":IDR_FX_TERRAIN_NORMAL", terrain_normal_fx);
	ADD_RESOURCE(":IDR_FX_TERRAIN_NORMAL_DEPTH_SHADOWMAP", terrain_normal_depth_shadowmap_fx);
	ADD_RESOURCE(":IDR_FX_UNLIT_TEXTURE", unlit_texture_fx);
	ADD_RESOURCE(":IDR_FX_WATER_RIPPLE", water_ripples_fx);
#endif
}

CStaticInitRes& CStaticInitRes::StaticInit()
{
	static CStaticInitRes _embedded_resource;
	return _embedded_resource;
}
