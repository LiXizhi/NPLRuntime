#include "ParaScriptingMuJoCo.h"
#include "RobotCoordinateConverter.h"

#include <cmath>
#include <iostream>

namespace
{
	const double Epsilon = 1e-9;

	bool Near(double left, double right)
	{
		return std::abs(left - right) < Epsilon;
	}

	ParaEngine::RobotVector3 Rotate(const ParaEngine::RobotQuaternion& quaternion, const ParaEngine::RobotVector3& vector)
	{
		const double dot = quaternion.x * vector.x + quaternion.y * vector.y + quaternion.z * vector.z;
		const double scale = quaternion.w * quaternion.w - quaternion.x * quaternion.x - quaternion.y * quaternion.y - quaternion.z * quaternion.z;
		ParaEngine::RobotVector3 result = {
			2.0 * dot * quaternion.x + scale * vector.x + 2.0 * quaternion.w * (quaternion.y * vector.z - quaternion.z * vector.y),
			2.0 * dot * quaternion.y + scale * vector.y + 2.0 * quaternion.w * (quaternion.z * vector.x - quaternion.x * vector.z),
			2.0 * dot * quaternion.z + scale * vector.z + 2.0 * quaternion.w * (quaternion.x * vector.y - quaternion.y * vector.x)
		};
		return result;
	}

	bool TestCoordinateConversion()
	{
		using ParaEngine::RobotCoordinateConverter;
		ParaEngine::RobotVector3 xAxis = RobotCoordinateConverter::MuJoCoPositionToParaEngine(1.0, 0.0, 0.0);
		ParaEngine::RobotVector3 yAxis = RobotCoordinateConverter::MuJoCoPositionToParaEngine(0.0, 1.0, 0.0);
		ParaEngine::RobotVector3 zAxis = RobotCoordinateConverter::MuJoCoPositionToParaEngine(0.0, 0.0, 1.0);
		if (!Near(xAxis.z, 1.0) || !Near(yAxis.x, -1.0) || !Near(zAxis.y, 1.0))
			return false;

		ParaEngine::RobotQuaternion identity = RobotCoordinateConverter::MuJoCoQuaternionToParaEngine(1.0, 0.0, 0.0, 0.0);
		if (!Near(identity.x, 0.0) || !Near(identity.y, 0.0) || !Near(identity.z, 0.0) || !Near(identity.w, 1.0))
			return false;

		const double halfSqrt = std::sqrt(0.5);
		ParaEngine::RobotQuaternion rotateX = RobotCoordinateConverter::MuJoCoQuaternionToParaEngine(halfSqrt, halfSqrt, 0.0, 0.0);
		ParaEngine::RobotVector3 rotated = Rotate(rotateX, yAxis);
		return Near(rotated.x, zAxis.x) && Near(rotated.y, zAxis.y) && Near(rotated.z, zAxis.z);
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "expected model path" << std::endl;
		return 1;
	}
	if (!TestCoordinateConversion())
		return 9;

	ParaScripting::MuJoCoSimulation simulation;
	if (!simulation.Load(argv[1]))
	{
		std::cerr << simulation.GetLastError() << std::endl;
		return 2;
	}
	if (simulation.GetQPosCount() != 1 || simulation.GetQVelCount() != 1 || simulation.GetControlCount() != 1)
		return 3;
	int bodyId = simulation.FindBody("link");
	if (simulation.GetBodyCount() != 2 || simulation.GetJointCount() != 1 || simulation.GetActuatorCount() != 1 || bodyId < 0)
		return 7;
	if (!std::isfinite(simulation.GetBodyPosition(bodyId, 2)) || !std::isfinite(simulation.GetBodyQuaternion(bodyId, 0)))
		return 8;
	if (!Near(simulation.GetBodyParaPosition(bodyId, 0), -simulation.GetBodyPosition(bodyId, 1)) ||
		!Near(simulation.GetBodyParaPosition(bodyId, 1), simulation.GetBodyPosition(bodyId, 2)) ||
		!Near(simulation.GetBodyParaPosition(bodyId, 2), simulation.GetBodyPosition(bodyId, 0)))
		return 10;
	if (!simulation.SetQPos(0, 0.1) || !simulation.SetControl(0, 0.2))
		return 4;

	simulation.Forward();
	simulation.Step(5000);
	if (!std::isfinite(simulation.GetQPos(0)) || !std::isfinite(simulation.GetQVel(0)) || simulation.GetTime() <= 0.0)
		return 5;

	simulation.Reset();
	if (simulation.GetTime() != 0.0 || !std::isfinite(simulation.GetQPos(0)))
		return 6;

	std::cout << "MuJoCo smoke test passed" << std::endl;
	return 0;
}