#include "RobotCoordinateConverter.h"

ParaEngine::RobotVector3 ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(double x, double y, double z)
{
	RobotVector3 result = { -y, z, x };
	return result;
}

ParaEngine::RobotQuaternion ParaEngine::RobotCoordinateConverter::MuJoCoQuaternionToParaEngine(double w, double x, double y, double z)
{
	RobotQuaternion result = { y, -z, -x, w };
	return result;
}