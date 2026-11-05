#pragma once

#ifdef USE_MUJOCO

#include "IRobotSimulation.h"

#include <memory>
#include <string>
#include <vector>

namespace ParaScripting
{
	class MuJoCoSimulation;
}

namespace ParaEngine
{
	namespace Robotics
	{
		class MujocoRobotSimulation : public IRobotSimulation
		{
		public:
			MujocoRobotSimulation();
			virtual ~MujocoRobotSimulation();

			virtual bool LoadModel(const RobotModelSource& source);
			virtual bool Reset(const RobotResetState& state);
			virtual bool Step(double fixedDeltaTime, const RobotAction& action);
			virtual bool GetObservation(RobotObservation& observation) const;
			virtual bool GetLinkTransforms(RobotLinkTransforms& transforms) const;
			virtual const RobotModelInfo& GetModelInfo() const;
			virtual RobotBackendCapabilities GetCapabilities() const;

			const std::string& GetLastError() const;
			ParaScripting::MuJoCoSimulation* GetSimulation() const;

			static int ComputeObservationDim(int nu);
			static int ComputeActionDim(int nu);

		private:
			void RefreshModelInfo();
			bool ApplyAction(const RobotAction& action);
			void CaptureLastAction();
			int ResolveJointQPosAdr(int actuatedIndex) const;
			int ResolveJointDofAdr(int actuatedIndex) const;
			void AccumulateFootForce(int footBodyId, double force[3]) const;

			std::unique_ptr<ParaScripting::MuJoCoSimulation> m_simulation;
			RobotModelInfo m_info;
			std::vector<double> m_lastAction;
			std::string m_lastError;
			std::vector<std::string> m_jointNames;
			std::string m_baseBody;
			std::string m_footBodies[2];
		};
	}
}

#endif
