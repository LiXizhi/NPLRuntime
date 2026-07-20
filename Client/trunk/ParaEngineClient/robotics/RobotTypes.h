#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ParaEngine
{
	namespace Robotics
	{
		// Keep in sync with script/.../Robot/RobotContractManifest.lua Version.
		static const char* kRobotContractVersion = "1.0.0";
		static const char* kRobotIdH1_2 = "unitree_h1_2";

		enum class RobotActionMode
		{
			JointTorque = 0,
			JointPositionTarget = 1,
			JointVelocityTarget = 2,
		};

		struct RobotModelSource
		{
			std::string path;
			std::string robotId;
			std::string backendHint; // "mujoco" | "bullet" | ""
		};

		struct RobotResetState
		{
			std::vector<double> qpos;
			std::vector<double> qvel;
			bool useDefaults;
			RobotResetState() : useDefaults(true) {}
		};

		struct RobotAction
		{
			RobotActionMode mode;
			std::vector<double> values; // canonical joint order
			double timestamp;
			RobotAction() : mode(RobotActionMode::JointTorque), timestamp(0.0) {}
		};

		struct RobotObservation
		{
			// Packed layout matches RobotContractManifest.ObservationSchema (ParaEngine frame).
			std::vector<double> packed;
			std::string contractVersion;
			std::string contractHash;
			double simTime;
			RobotObservation() : simTime(0.0) {}
		};

		struct RobotLinkTransform
		{
			std::string name;
			double pos[3];
			double quatXyZw[4];
			RobotLinkTransform()
			{
				pos[0] = pos[1] = pos[2] = 0.0;
				quatXyZw[0] = quatXyZw[1] = quatXyZw[2] = 0.0;
				quatXyZw[3] = 1.0;
			}
		};

		struct RobotLinkTransforms
		{
			std::vector<RobotLinkTransform> links;
		};

		struct RobotModelInfo
		{
			std::string backend;
			std::string robotId;
			std::string modelPath;
			std::string contractVersion;
			std::string contractHash;
			int nq;
			int nv;
			int nu;
			int nbody;
			int observationDim;
			int actionDim;
			double fixedDt;
			RobotModelInfo()
				: nq(0), nv(0), nu(0), nbody(0), observationDim(0), actionDim(0), fixedDt(0.002) {}
		};

		struct RobotBackendCapabilities
		{
			bool supportsStaticBoxes;
			bool supportsHeightField;
			bool supportsConvexMesh;
			bool supportsDynamicObjects;
			bool supportsRuntimeTerrainUpdate;
			bool supportsPositionControl;
			bool supportsVelocityControl;
			bool supportsTorqueControl;
			bool supportsContactForce;
			RobotBackendCapabilities()
				: supportsStaticBoxes(false)
				, supportsHeightField(false)
				, supportsConvexMesh(false)
				, supportsDynamicObjects(false)
				, supportsRuntimeTerrainUpdate(false)
				, supportsPositionControl(false)
				, supportsVelocityControl(false)
				, supportsTorqueControl(false)
				, supportsContactForce(false)
			{}
		};
	}
}
