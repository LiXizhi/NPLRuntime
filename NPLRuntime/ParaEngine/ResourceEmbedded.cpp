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

		ADD_RESOURCE(":IDR_PARAXTEMPLATE", ParaXmodel_templates);
		ADD_RESOURCE(":IDR_DEFAULT_CURSOR", cursor_tga);
		ADD_RESOURCE(":IDR_PARAWORLD_ICO", paraworld_ico);
		ADD_RESOURCE(":IDR_FX_OCEANWATER", ocean_water_fx);
		ADD_RESOURCE(":IDR_FX_OCCLUSION_TEST", occlusion_test_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH", simple_mesh_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL", simple_mesh_normal_fx);
		ADD_RESOURCE(":IDR_FX_SMESH_CTOR", smesh_ctor_fx);
		ADD_RESOURCE(":IDR_FX_TERRAIN_NORMAL", terrain_normal_fx);
		ADD_RESOURCE(":IDR_FX_FULLSCREEN_GLOW", fullscreen_glow_fx);
		ADD_RESOURCE(":IDR_FX_SKY", sky_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_SHADOW", simple_mesh_normal_shadow_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_TEX2", simple_mesh_normal_tex2_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED", simple_mesh_normal_selected_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_BORDER", simple_mesh_normal_border_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_LOW", simple_mesh_normal_low_fx);
		ADD_RESOURCE(":IDR_FX_WATER_RIPPLE", water_ripples_fx);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_HIGH", ocean_water_high_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED", simple_mesh_normal_instanced_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT", simple_mesh_normal_unlit_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_PARTICLE", simple_particle_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_CAD_MODEL", simple_cad_model_fx);
		ADD_RESOURCE(":IDR_FX_DROPSHADOW", dropShadow_fx);
		ADD_RESOURCE(":IDR_FX_SCREENWAVE", screenWave_fx);
		ADD_RESOURCE(":IDR_FX_BLOCK", blockEffect_fx);
		ADD_RESOURCE(":IDR_FX_SHADOWMAP_BLUR", shadowmap_blur_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_VEGETATION", simple_mesh_normal_vegetation_fx);
		ADD_RESOURCE(":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS", simple_mesh_normal_specialeffects_fx);
		ADD_RESOURCE(":IDR_FX_SKYDOME", skydome_fx);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_QUAD", ocean_water_quad_fx);
		ADD_RESOURCE(":IDR_FX_OCEANWATER_CLOUD", ocean_water_cloud_fx);
		ADD_RESOURCE(":IDR_FX_BMAXMODEL", BMaxModel_fx);

#ifdef USE_OPENGL_RENDERER
		ADD_RESOURCE(":IDR_FX_SINGLECOLOR", singleColorEffect_fx);
		ADD_RESOURCE(":IDR_FX_GUI", GuiEffect_fx);
		ADD_RESOURCE(":IDR_FX_GUI_TEXT", GuiTextEffect_fx);
#endif // USE_OPENGL_RENDERER


	}

	CStaticInitRes& CStaticInitRes::StaticInit(){
		static CStaticInitRes _embedded_resource;
		return _embedded_resource;
	}







