#pragma once

#include "RobotTypes.h"

namespace ParaEngine
{
	namespace Robotics
	{
		// Model-level robot dynamics API. Do not fold into IParaPhysics actor/vehicle semantics.
		// Lua mirror: script/apps/Aries/Creator/Game/Robot/MuJoCoRobotSimulation.lua
		class IRobotSimulation
		{
		public:
			virtual ~IRobotSimulation() {}

			virtual bool LoadModel(const RobotModelSource& source) = 0;
			virtual bool Reset(const RobotResetState& state) = 0;
			virtual bool Step(double fixedDeltaTime, const RobotAction& action) = 0;
			virtual bool GetObservation(RobotObservation& observation) const = 0;
			virtual bool GetLinkTransforms(RobotLinkTransforms& transforms) const = 0;
			virtual const RobotModelInfo& GetModelInfo() const = 0;
			virtual RobotBackendCapabilities GetCapabilities() const = 0;
		};
	}
}
