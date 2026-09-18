#include "MujocoRobotSimulation.h"

#ifdef USE_MUJOCO

#include "ParaScriptingMuJoCo.h"
#include "RobotCoordinateConverter.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

		namespace
{
	const char* kDefaultBaseBody = "pelvis";
	const char* kDefaultFoot0 = "left_ankle_roll_link";
	const char* kDefaultFoot1 = "right_ankle_roll_link";

	// Same FNV-1a style fingerprint as Lua RobotContractManifest.ComputeHash for v1.0 H1 text.
	// Kept as a constant so C++/Lua logs can compare without re-encoding the full joint list every call.
	const char* kContractHashH1V1 = "e93d69a8";

	// Canonical actuated hinge order - keep in sync with RobotContractManifest.JointOrder.
	const char* kH1JointOrder[] = {
		"left_hip_yaw_joint", "left_hip_pitch_joint", "left_hip_roll_joint", "left_knee_joint",
		"left_ankle_pitch_joint", "left_ankle_roll_joint",
		"right_hip_yaw_joint", "right_hip_pitch_joint", "right_hip_roll_joint", "right_knee_joint",
		"right_ankle_pitch_joint", "right_ankle_roll_joint",
		"torso_joint",
		"left_shoulder_pitch_joint", "left_shoulder_roll_joint", "left_shoulder_yaw_joint",
		"left_elbow_joint", "left_wrist_roll_joint", "left_wrist_pitch_joint", "left_wrist_yaw_joint",
		"right_shoulder_pitch_joint", "right_shoulder_roll_joint", "right_shoulder_yaw_joint",
		"right_elbow_joint", "right_wrist_roll_joint", "right_wrist_pitch_joint", "right_wrist_yaw_joint",
		"L_index_proximal_joint", "L_index_intermediate_joint",
		"L_middle_proximal_joint", "L_middle_intermediate_joint",
		"L_ring_proximal_joint", "L_ring_intermediate_joint",
		"L_pinky_proximal_joint", "L_pinky_intermediate_joint",
		"L_thumb_proximal_yaw_joint", "L_thumb_proximal_pitch_joint", "L_thumb_intermediate_joint", "L_thumb_distal_joint",
		"R_index_proximal_joint", "R_index_intermediate_joint",
		"R_middle_proximal_joint", "R_middle_intermediate_joint",
		"R_ring_proximal_joint", "R_ring_intermediate_joint",
		"R_pinky_proximal_joint", "R_pinky_intermediate_joint",
		"R_thumb_proximal_yaw_joint", "R_thumb_proximal_pitch_joint", "R_thumb_intermediate_joint", "R_thumb_distal_joint",
	};
	const int kH1JointCount = static_cast<int>(sizeof(kH1JointOrder) / sizeof(kH1JointOrder[0]));

	void RotateVectorByQuatXyZw(double qx, double qy, double qz, double qw, double vx, double vy, double vz, double out[3])
	{
		const double x = qx * 2.0;
		const double y = qy * 2.0;
		const double z = qz * 2.0;
		const double xx = qx * x;
		const double yy = qy * y;
		const double zz = qz * z;
		const double xy = qx * y;
		const double xz = qx * z;
		const double yz = qy * z;
		const double wx = qw * x;
		const double wy = qw * y;
		const double wz = qw * z;
		out[0] = (1.0 - (yy + zz)) * vx + (xy - wz) * vy + (xz + wy) * vz;
		out[1] = (xy + wz) * vx + (1.0 - (xx + zz)) * vy + (yz - wx) * vz;
		out[2] = (xz - wy) * vx + (yz + wx) * vy + (1.0 - (xx + yy)) * vz;
	}

	void ProjectedGravityPara(double qx, double qy, double qz, double qw, double out[3])
	{
		// Inverse rotation of world gravity (0,-1,0) into base frame.
		RotateVectorByQuatXyZw(-qx, -qy, -qz, qw, 0.0, -1.0, 0.0, out);
	}
}

namespace ParaEngine
{
	namespace Robotics
	{
		MujocoRobotSimulation::MujocoRobotSimulation()
			: m_simulation(new ParaScripting::MuJoCoSimulation())
			, m_baseBody(kDefaultBaseBody)
		{
			m_footBodies[0] = kDefaultFoot0;
			m_footBodies[1] = kDefaultFoot1;
			m_info.backend = "mujoco";
			m_info.contractVersion = kRobotContractVersion;
			m_info.fixedDt = 0.002;
		}

		MujocoRobotSimulation::~MujocoRobotSimulation() {}

		int MujocoRobotSimulation::ComputeObservationDim(int nu)
		{
			// base_pos(3)+quat(4)+lin(3)+ang(3)+gravity(3)+q(nu)+qd(nu)+last(nu)+feet(2)+force(6)+time(1)
			return 25 + 3 * nu;
		}

		int MujocoRobotSimulation::ComputeActionDim(int nu)
		{
			return nu;
		}

		const std::string& MujocoRobotSimulation::GetLastError() const
		{
			return m_lastError.empty() && m_simulation ? m_simulation->GetLastError() : m_lastError;
		}

		ParaScripting::MuJoCoSimulation* MujocoRobotSimulation::GetSimulation() const
		{
			return m_simulation.get();
		}

		void MujocoRobotSimulation::RefreshModelInfo()
		{
			if (!m_simulation || !m_simulation->IsValid())
			{
				m_info = RobotModelInfo();
				m_info.backend = "mujoco";
				m_info.contractVersion = kRobotContractVersion;
				m_info.fixedDt = 0.002;
				return;
			}

			m_info.backend = "mujoco";
			m_info.contractVersion = kRobotContractVersion;
			m_info.nq = m_simulation->GetQPosCount();
			m_info.nv = m_simulation->GetQVelCount();
			m_info.nu = m_simulation->GetControlCount();
			m_info.nbody = m_simulation->GetBodyCount();
			m_info.actionDim = ComputeActionDim(m_info.nu);
			m_info.observationDim = ComputeObservationDim(m_info.nu);
			m_info.fixedDt = 0.002;
			if (m_info.nu == 51 && m_info.robotId == kRobotIdH1_2)
				m_info.contractHash = kContractHashH1V1;
			else
			{
				char buffer[32];
				std::snprintf(buffer, sizeof(buffer), "nu%d-v%s", m_info.nu, kRobotContractVersion);
				m_info.contractHash = buffer;
			}
			m_lastAction.assign(static_cast<size_t>(m_info.nu), 0.0);
		}

		bool MujocoRobotSimulation::LoadModel(const RobotModelSource& source)
		{
			m_lastError.clear();
			if (!m_simulation)
				m_simulation.reset(new ParaScripting::MuJoCoSimulation());

			if (!m_simulation->Load(source.path))
			{
				m_lastError = m_simulation->GetLastError();
				RefreshModelInfo();
				return false;
			}

			m_info.modelPath = source.path;
			m_info.robotId = source.robotId.empty() ? kRobotIdH1_2 : source.robotId;
			m_jointNames.clear();
			RefreshModelInfo();
			if (m_info.nu == kH1JointCount && (m_info.robotId.empty() || m_info.robotId == kRobotIdH1_2))
			{
				m_jointNames.reserve(static_cast<size_t>(kH1JointCount));
				for (int index = 0; index < kH1JointCount; ++index)
					m_jointNames.push_back(kH1JointOrder[index]);
			}
			return true;
		}

		bool MujocoRobotSimulation::Reset(const RobotResetState& state)
		{
			if (!m_simulation || !m_simulation->IsValid())
			{
				m_lastError = "MuJoCo model is not loaded";
				return false;
			}

			m_simulation->Reset();
			if (!state.useDefaults)
			{
				for (size_t index = 0; index < state.qpos.size(); ++index)
					m_simulation->SetQPos(static_cast<int>(index), state.qpos[index]);
				for (size_t index = 0; index < state.qvel.size(); ++index)
					m_simulation->SetQVel(static_cast<int>(index), state.qvel[index]);
				m_simulation->Forward();
			}
			std::fill(m_lastAction.begin(), m_lastAction.end(), 0.0);
			return true;
		}

		bool MujocoRobotSimulation::ApplyAction(const RobotAction& action)
		{
			if (!m_simulation || !m_simulation->IsValid())
				return false;

			const int nu = m_simulation->GetControlCount();
			if (action.mode != RobotActionMode::JointTorque)
			{
				m_lastError = "only JointTorque action is supported in MujocoRobotSimulation v1";
				return false;
			}
			if (static_cast<int>(action.values.size()) < nu)
			{
				m_lastError = "action.values shorter than nu";
				return false;
			}

			for (int index = 0; index < nu; ++index)
				m_simulation->SetControl(index, action.values[static_cast<size_t>(index)]);
			CaptureLastAction();
			return true;
		}

		void MujocoRobotSimulation::CaptureLastAction()
		{
			if (!m_simulation || !m_simulation->IsValid())
				return;
			const int nu = m_simulation->GetControlCount();
			m_lastAction.resize(static_cast<size_t>(nu));
			for (int index = 0; index < nu; ++index)
				m_lastAction[static_cast<size_t>(index)] = m_simulation->GetControl(index);
		}

		bool MujocoRobotSimulation::Step(double fixedDeltaTime, const RobotAction& action)
		{
			if (!ApplyAction(action))
				return false;

			const double dt = m_info.fixedDt > 0.0 ? m_info.fixedDt : 0.002;
			int stepCount = static_cast<int>(std::lround(fixedDeltaTime / dt));
			if (stepCount < 1)
				stepCount = 1;
			m_simulation->Step(stepCount);
			return true;
		}

		int MujocoRobotSimulation::ResolveJointQPosAdr(int actuatedIndex) const
		{
			if (!m_simulation || !m_simulation->IsValid())
				return -1;
			const int nu = m_simulation->GetControlCount();
			const int nq = m_simulation->GetQPosCount();
			const int freeQpos = nq - nu;
			if (actuatedIndex < 0 || actuatedIndex >= nu)
				return -1;

			if (!m_jointNames.empty() && static_cast<int>(m_jointNames.size()) == nu)
			{
				const int jointId = m_simulation->FindJoint(m_jointNames[static_cast<size_t>(actuatedIndex)]);
				if (jointId >= 0)
					return m_simulation->GetJointQPosAdr(jointId);
			}
			return freeQpos + actuatedIndex;
		}

		int MujocoRobotSimulation::ResolveJointDofAdr(int actuatedIndex) const
		{
			if (!m_simulation || !m_simulation->IsValid())
				return -1;
			const int nu = m_simulation->GetControlCount();
			const int nv = m_simulation->GetQVelCount();
			const int freeQvel = nv - nu;
			if (actuatedIndex < 0 || actuatedIndex >= nu)
				return -1;

			if (!m_jointNames.empty() && static_cast<int>(m_jointNames.size()) == nu)
			{
				const int jointId = m_simulation->FindJoint(m_jointNames[static_cast<size_t>(actuatedIndex)]);
				if (jointId >= 0)
					return m_simulation->GetJointDofAdr(jointId);
			}
			return freeQvel + actuatedIndex;
		}

		void MujocoRobotSimulation::AccumulateFootForce(int footBodyId, double force[3]) const
		{
			force[0] = force[1] = force[2] = 0.0;
			if (!m_simulation || footBodyId < 0)
				return;
			const int contactCount = m_simulation->GetContactCount();
			for (int contactIndex = 0; contactIndex < contactCount; ++contactIndex)
			{
				const int body1 = m_simulation->GetContactBody1(contactIndex);
				const int body2 = m_simulation->GetContactBody2(contactIndex);
				if (body1 != footBodyId && body2 != footBodyId)
					continue;
				force[0] += m_simulation->GetContactParaForce(contactIndex, 0);
				force[1] += m_simulation->GetContactParaForce(contactIndex, 1);
				force[2] += m_simulation->GetContactParaForce(contactIndex, 2);
			}
		}

		bool MujocoRobotSimulation::GetObservation(RobotObservation& observation) const
		{
			observation = RobotObservation();
			if (!m_simulation || !m_simulation->IsValid())
				return false;

			const int nu = m_simulation->GetControlCount();
			const int dim = ComputeObservationDim(nu);
			observation.packed.assign(static_cast<size_t>(dim), 0.0);
			observation.contractVersion = kRobotContractVersion;
			observation.contractHash = m_info.contractHash;
			observation.simTime = m_simulation->GetTime();

			const int baseId = m_simulation->FindBody(m_baseBody);
			double* packed = observation.packed.data();
			int offset = 0;

			auto write3 = [&](double a, double b, double c)
			{
				packed[offset++] = a;
				packed[offset++] = b;
				packed[offset++] = c;
			};

			if (baseId >= 0)
			{
				write3(
					m_simulation->GetBodyParaPosition(baseId, 0),
					m_simulation->GetBodyParaPosition(baseId, 1),
					m_simulation->GetBodyParaPosition(baseId, 2));
				packed[offset++] = m_simulation->GetBodyParaQuaternion(baseId, 0);
				packed[offset++] = m_simulation->GetBodyParaQuaternion(baseId, 1);
				packed[offset++] = m_simulation->GetBodyParaQuaternion(baseId, 2);
				packed[offset++] = m_simulation->GetBodyParaQuaternion(baseId, 3);
				write3(
					m_simulation->GetBodyParaLinearVelocity(baseId, 0),
					m_simulation->GetBodyParaLinearVelocity(baseId, 1),
					m_simulation->GetBodyParaLinearVelocity(baseId, 2));
				write3(
					m_simulation->GetBodyParaAngularVelocity(baseId, 0),
					m_simulation->GetBodyParaAngularVelocity(baseId, 1),
					m_simulation->GetBodyParaAngularVelocity(baseId, 2));

				double gravity[3] = { 0, 0, 0 };
				ProjectedGravityPara(
					m_simulation->GetBodyParaQuaternion(baseId, 0),
					m_simulation->GetBodyParaQuaternion(baseId, 1),
					m_simulation->GetBodyParaQuaternion(baseId, 2),
					m_simulation->GetBodyParaQuaternion(baseId, 3),
					gravity);
				write3(gravity[0], gravity[1], gravity[2]);
			}
			else
			{
				offset += 16; // zeros already
			}

			for (int index = 0; index < nu; ++index)
			{
				const int qAdr = ResolveJointQPosAdr(index);
				packed[offset++] = qAdr >= 0 ? m_simulation->GetQPos(qAdr) : 0.0;
			}
			for (int index = 0; index < nu; ++index)
			{
				const int dofAdr = ResolveJointDofAdr(index);
				packed[offset++] = dofAdr >= 0 ? m_simulation->GetQVel(dofAdr) : 0.0;
			}
			for (int index = 0; index < nu; ++index)
			{
				const double value = index < static_cast<int>(m_lastAction.size()) ? m_lastAction[static_cast<size_t>(index)] : 0.0;
				packed[offset++] = value;
			}

			int footIds[2] = {
				m_simulation->FindBody(m_footBodies[0]),
				m_simulation->FindBody(m_footBodies[1])
			};
			double footForce[2][3] = { { 0, 0, 0 }, { 0, 0, 0 } };
			for (int foot = 0; foot < 2; ++foot)
			{
				bool inContact = false;
				if (footIds[foot] >= 0)
				{
					AccumulateFootForce(footIds[foot], footForce[foot]);
					const int contactCount = m_simulation->GetContactCount();
					for (int contactIndex = 0; contactIndex < contactCount; ++contactIndex)
					{
						if (m_simulation->GetContactBody1(contactIndex) == footIds[foot] ||
							m_simulation->GetContactBody2(contactIndex) == footIds[foot])
						{
							inContact = true;
							break;
						}
					}
				}
				packed[offset++] = inContact ? 1.0 : 0.0;
			}
			for (int foot = 0; foot < 2; ++foot)
				write3(footForce[foot][0], footForce[foot][1], footForce[foot][2]);

			packed[offset++] = observation.simTime;
			return offset == dim;
		}

		bool MujocoRobotSimulation::GetLinkTransforms(RobotLinkTransforms& transforms) const
		{
			transforms.links.clear();
			if (!m_simulation || !m_simulation->IsValid())
				return false;

			const int bodyCount = m_simulation->GetBodyCount();
			transforms.links.reserve(static_cast<size_t>(std::max(0, bodyCount - 1)));
			for (int bodyId = 1; bodyId < bodyCount; ++bodyId)
			{
				RobotLinkTransform link;
				link.name = m_simulation->GetBodyName(bodyId);
				link.pos[0] = m_simulation->GetBodyParaPosition(bodyId, 0);
				link.pos[1] = m_simulation->GetBodyParaPosition(bodyId, 1);
				link.pos[2] = m_simulation->GetBodyParaPosition(bodyId, 2);
				link.quatXyZw[0] = m_simulation->GetBodyParaQuaternion(bodyId, 0);
				link.quatXyZw[1] = m_simulation->GetBodyParaQuaternion(bodyId, 1);
				link.quatXyZw[2] = m_simulation->GetBodyParaQuaternion(bodyId, 2);
				link.quatXyZw[3] = m_simulation->GetBodyParaQuaternion(bodyId, 3);
				transforms.links.push_back(link);
			}
			return true;
		}

		const RobotModelInfo& MujocoRobotSimulation::GetModelInfo() const
		{
			return m_info;
		}

		RobotBackendCapabilities MujocoRobotSimulation::GetCapabilities() const
		{
			RobotBackendCapabilities capabilities;
			capabilities.supportsHeightField = true;
			capabilities.supportsRuntimeTerrainUpdate = true;
			capabilities.supportsTorqueControl = true;
			capabilities.supportsContactForce = true;
			return capabilities;
		}
	}
}

#endif
