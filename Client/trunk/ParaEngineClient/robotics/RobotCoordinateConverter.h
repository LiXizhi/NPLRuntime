#pragma once

namespace ParaEngine
{
	struct RobotVector3
	{
		double x;
		double y;
		double z;
	};

	struct RobotQuaternion
	{
		double x;
		double y;
		double z;
		double w;
	};

	class RobotCoordinateConverter
	{
	public:
		static RobotVector3 MuJoCoPositionToParaEngine(double x, double y, double z);
		static RobotQuaternion MuJoCoQuaternionToParaEngine(double w, double x, double y, double z);
	};
}