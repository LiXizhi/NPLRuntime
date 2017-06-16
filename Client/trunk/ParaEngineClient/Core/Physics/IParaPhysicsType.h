#pragma once


namespace ParaEngine {

	/*  physics world type */
	enum ParaPhysicsWorldType
	{
		PPWT_Dynamics = 0,
		PPWT_SoftRigidDynamics,
	};

	enum PhysicsDebugDrawModes
	{
		PDDM_NoDebug = 0,
		PDDM_DrawWireframe = 1,
		PDDM_DrawAabb = 2,
		PDDM_DrawFeaturesText = 4,
		PDDM_DrawContactPoints = 8,
		PDDM_NoDeactivation = 16,
		PDDM_NoHelpText = 32,
		PDDM_DrawText = 64,
		PDDM_ProfileTimings = 128,
		PDDM_EnableSatComparison = 256,
		PDDM_DisableBulletLCP = 512,
		PDDM_EnableCCD = 1024,
		PDDM_DrawConstraints = (1 << 11),
		PDDM_DrawConstraintLimits = (1 << 12),
		PDDM_FastWireframe = (1 << 13),
		PDDM_DrawNormals = (1 << 14),
		PDDM_DrawFrames = (1 << 15),
		PDDM_MAX_DEBUG_DRAW_MODE
	};
}

