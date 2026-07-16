#pragma once

#ifdef USE_MUJOCO

#include <memory>
#include <string>

namespace ParaScripting
{
	class MuJoCoSimulation
	{
	public:
		MuJoCoSimulation();
		~MuJoCoSimulation();

		bool Load(const std::string& filename);
		void Reset();
		void Forward();
		void Step(int count);
		bool IsValid() const;
		const std::string& GetLastError() const;
		int GetQPosCount() const;
		int GetQVelCount() const;
		int GetControlCount() const;
		int GetBodyCount() const;
		int GetJointCount() const;
		int GetActuatorCount() const;
		int GetContactCount() const;
		double GetTime() const;
		double GetQPos(int index) const;
		bool SetQPos(int index, double value);
		double GetQVel(int index) const;
		bool SetQVel(int index, double value);
		double GetControl(int index) const;
		bool SetControl(int index, double value);
		int NameToId(int objectType, const std::string& name) const;
		int FindBody(const std::string& name) const;
		int FindJoint(const std::string& name) const;
		int FindActuator(const std::string& name) const;
		double GetBodyPosition(int bodyId, int component) const;
		double GetBodyQuaternion(int bodyId, int component) const;
		double GetBodyParaPosition(int bodyId, int component) const;
		double GetBodyParaQuaternion(int bodyId, int component) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};

	class ParaMuJoCo
	{
	public:
		static int LoadModel(const std::string& filename);
		static bool DeleteModel(int handle);
		static bool IsValid(int handle);
		static void Reset(int handle);
		static void Forward(int handle);
		static void Step(int handle, int count);
		static std::string GetLastError(int handle);
		static int GetQPosCount(int handle);
		static int GetQVelCount(int handle);
		static int GetControlCount(int handle);
		static int GetBodyCount(int handle);
		static int GetJointCount(int handle);
		static int GetActuatorCount(int handle);
		static int GetContactCount(int handle);
		static double GetTime(int handle);
		static double GetQPos(int handle, int index);
		static bool SetQPos(int handle, int index, double value);
		static double GetQVel(int handle, int index);
		static bool SetQVel(int handle, int index, double value);
		static double GetControl(int handle, int index);
		static bool SetControl(int handle, int index, double value);
		static int NameToId(int handle, int objectType, const std::string& name);
		static int FindBody(int handle, const std::string& name);
		static int FindJoint(int handle, const std::string& name);
		static int FindActuator(int handle, const std::string& name);
		static double GetBodyPosition(int handle, int bodyId, int component);
		static double GetBodyQuaternion(int handle, int bodyId, int component);
		static double GetBodyParaPosition(int handle, int bodyId, int component);
		static double GetBodyParaQuaternion(int handle, int bodyId, int component);
	};
}

#endif