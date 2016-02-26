#pragma once
namespace ParaEngine
{
	/** ParaEngine reserved technique handle:
	* the following technique Handles are reserved for ParaEngine internal usages.
	Scene object in brackets are by default assigned the specified value */
	enum TechniqueHandle
	{
		///the object is not renderable, thus having no technique. (CBaseObject)
		TECH_NONE = 0,
		///the object is rendered for occlusion testing. Z-write is disabled and pixel shader is simplified.
		TECH_OCCLUSION_TEST,
		///vertex declaration is POSITION | TEXCord1, these are usually for static meshes. (CMeshObject)
		TECH_SIMPLE_MESH,
		///vertex declaration is POSITION | NORMAL | TEXCord1, these are usually for static meshes. For (CMeshObject) whose asset ParaX file contains the normal data.
		TECH_SIMPLE_MESH_NORMAL,
		///it shares the same shader with TECH_SIMPLE_MESH_NORMAL
		TECH_SIMPLE_MESH_NORMAL_VEGETATION,
		/// same as TECH_SIMPLE_MESH_NORMAL, except that instancing is used. 
		TECH_SIMPLE_MESH_NORMAL_INSTANCED,
		///this is the same as TECH_SIMPLE_MESH_NORMAL except that it receive shadows and can not be reflective. 
		TECH_SIMPLE_MESH_NORMAL_SHADOW,
		///this is similar to TECH_SIMPLE_MESH_NORMAL 
		TECH_SIMPLE_MESH_NORMAL_TRANSPARENT,
		///vertex declaration is POSITION | NORMAL | TEXCord1, these are usually for static meshes.It displays an animation in shader when the mesh is being constructed.
		TECH_SIMPLE_MESH_NORMAL_CTOR,
		///vertex declaration is POSITION | NORMAL | TEXCord1, without any lighting computation.
		TECH_SIMPLE_MESH_NORMAL_UNLIT,
		///vertex declaration is POSITION | NORMAL | TEXCord1, these are usually for selected meshes. 
		TECH_SIMPLE_MESH_NORMAL_SELECTED,
		///vertex declaration is POSITION | NORMAL | TEXCord1 | TEXCord2, these are usually for  meshes with a second UV set.
		TECH_SIMPLE_MESH_NORMAL_TEX2,
		///vertex declaration is POSITION | NORMAL | TEXCord1| TEXCord2 , usually for animated character model. (CBipedObject, CMissileObject)
		TECH_CHARACTER,
		///vertex declaration is POSITION | NORMAL | TEXCord1 | Color1, usually for animated particle model.
		TECH_PARTICLES,
		///vertex declaration is POSITION | NORMAL | TEXCord1 | Color1, usually for animated particle model.
		TECH_WATER_RIPPLE,
		/// vertex declaration is POSITION | TEXCord1|COLOR, for sprite object. (CSpriteObject)
		TECH_SPRITE,
		///vertex declaration is POSITION | TEXCord1, for sky mesh object. (CSkyObject)
		TECH_SKY_MESH,
		///vertex declaration is POSITION | TEXCord1, for sky mesh object. (CSkyObject)
		TECH_SKY_DOME,
		///vertex declaration is POSITION | TEXCord1 | TEXCord2, usually used for terrain surface drawing.
		TECH_TERRAIN,
		/// default ocean with shader 3.0
		TECH_OCEAN,
		/// requires just shader 2.0
		TECH_OCEAN_SIMPLE,
		/// under water effect: currently there is only a fixed function pipeline version
		TECH_OCEAN_UNDERWATER,
		/// full screen glow effect shader.
		TECH_FULL_SCREEN_GLOW,
		/// full screen shadow map blur effect shader.
		TECH_SHADOWMAP_BLUR,
		/// GUI rendering
		TECH_GUI,
		///vertex declaration is POSITION | NORMAL | TEXCord1, 
		TECH_SIMPLE_MESH_NORMAL_BORDER,
		///vertex declaration is POSITION | NORMAL | TEXCord1, these are usually for static meshes. For (CMeshObject) whose asset ParaX file contains the normal data.
		TECH_SIMPLE_CAD_MODEL,
		///
		TECH_SIMPLE_DROPSHADOW,
		///screen water wave effect shader
		TECH_SCREEN_WAVE,
		//block rendering
		TECH_BLOCK,
		// rendering cloud ocean
		TECH_OCEAN_CLOUD,
		// render fancy blocks
		TECH_BLOCK_FANCY,
		// single color: POSITION | COLOR
		TECH_SINGLE_COLOR,
		// text effect
		TECH_GUI_TEXT,
		// block max model
		TECH_BMAX_MODEL,
		// for external unknown effect. 
		TECH_UNKNOWN,
	};
}
