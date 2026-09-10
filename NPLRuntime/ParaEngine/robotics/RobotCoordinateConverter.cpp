#include "RobotCoordinateConverter.h"

// MuJoCo RH Z-up -> ParaEngine LH Y-up.
// Position: (x,y,z)_mj -> (-y, z, x)_pe
//   MuJoCo +X (forward) -> Para +Z
//   MuJoCo +Y (left)    -> Para -X
//   MuJoCo +Z (up)      -> Para +Y
// Rotation: R_para = C * R_mujoco * inv(C), exposed as quaternion shuffle
//   MuJoCo (w,x,y,z) -> ParaEngine (x,y,z,w) = (y, -z, -x, w)
// Validated by cmake/mujoco-smoke (unit axes, identity, 90deg) and LiveTest mesh sync.
// ParaX bone locals must consume GetBodyPara* as-is — do not add SetFacing/root-yaw on top.

ParaEngine::RobotVector3 ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(double x, double y, double z)
{
	RobotVector3 result = { -y, z, x };
	return result;
}

ParaEngine::RobotVector3 ParaEngine::RobotCoordinateConverter::ParaEnginePositionToMuJoCo(double x, double y, double z)
{
	// Inverse of (x,y,z)_mj -> (-y, z, x)_pe
	RobotVector3 result = { z, -x, y };
	return result;
}

ParaEngine::RobotQuaternion ParaEngine::RobotCoordinateConverter::MuJoCoQuaternionToParaEngine(double w, double x, double y, double z)
{
	RobotQuaternion result = { y, -z, -x, w };
	return result;
}
