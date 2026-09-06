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
		// Replace only world block geometry, preserving the live integration state.
		bool UpdateBlockCollision(const std::string& boxes, double friction, double rollingFriction);
		bool InitializeBlockCollisionPool(int capacity);
		bool UpdateBlockCollisionPool(const std::string& boxes, double friction, double rollingFriction);
		int GetGeomCount() const;
		int GetContactDimension(int contactIndex) const;
		double GetContactFriction(int contactIndex, int component) const;
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
		int FindSensor(const std::string& name) const;
		int GetSensorDim(int sensorId) const;
		double GetSensorData(int sensorId, int component) const;
		int GetJointQPosAdr(int jointId) const;
		int GetJointDofAdr(int jointId) const;
		double GetJointAxis(int jointId, int component) const;
		double GetJointParaAxis(int jointId, int component) const;
		double GetJointParaAnchor(int jointId, int component) const;
		double GetBodyPosition(int bodyId, int component) const;
		double GetBodyQuaternion(int bodyId, int component) const;
		double GetBodyParaPosition(int bodyId, int component) const;
		double GetBodyParaQuaternion(int bodyId, int component) const;
		double GetBodyLinearVelocity(int bodyId, int component) const;
		double GetBodyAngularVelocity(int bodyId, int component) const;
		double GetBodyParaLinearVelocity(int bodyId, int component) const;
		double GetBodyParaAngularVelocity(int bodyId, int component) const;
		double RaycastDown(double x, double y, double z) const;
		int GetContactGeom1(int contactIndex) const;
		int GetContactGeom2(int contactIndex) const;
		int GetContactBody1(int contactIndex) const;
		int GetContactBody2(int contactIndex) const;
		double GetContactDist(int contactIndex) const;
		double GetContactPosition(int contactIndex, int component) const;
		double GetContactNormal(int contactIndex, int component) const;
		double GetContactParaPosition(int contactIndex, int component) const;
		double GetContactParaNormal(int contactIndex, int component) const;
		double GetContactForce(int contactIndex, int component) const;
		double GetContactParaForce(int contactIndex, int component) const;
		int FindHField(const std::string& name) const;
		int FindGeom(const std::string& name) const;
		int GetHFieldCount() const;
		int GetHFieldNRow(int hfieldId) const;
		int GetHFieldNCol(int hfieldId) const;
		double GetHFieldSize(int hfieldId, int component) const;
		double GetHFieldElevation(int hfieldId, int row, int col) const;
		bool SetHFieldElevation(int hfieldId, int row, int col, double value);
		bool FillHFieldElevation(int hfieldId, double value);
		double GetGeomPosition(int geomId, int component) const;
		bool SetGeomPosition(int geomId, int component, double value);
		double GetGeomParaPosition(int geomId, int component) const;
		bool SetGeomParaPosition(int geomId, double x, double y, double z);
		std::string GetBodyName(int bodyId) const;
		std::string GetJointName(int jointId) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};

	class ParaMuJoCo
	{
	public:
		static int LoadModel(const std::string& filename);
		static bool UpdateBlockCollision(int handle, const std::string& boxes, double friction, double rollingFriction);
		static bool InitializeBlockCollisionPool(int handle, int capacity);
		static bool UpdateBlockCollisionPool(int handle, const std::string& boxes, double friction, double rollingFriction);
		static int GetGeomCount(int handle);
		static int GetContactDimension(int handle, int contactIndex);
		static double GetContactFriction(int handle, int contactIndex, int component);
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
		static int FindSensor(int handle, const std::string& name);
		static int GetSensorDim(int handle, int sensorId);
		static double GetSensorData(int handle, int sensorId, int component);
		static int GetJointQPosAdr(int handle, int jointId);
		static int GetJointDofAdr(int handle, int jointId);
		static double GetJointAxis(int handle, int jointId, int component);
		static double GetJointParaAxis(int handle, int jointId, int component);
		static double GetJointParaAnchor(int handle, int jointId, int component);
		static double GetBodyPosition(int handle, int bodyId, int component);
		static double GetBodyQuaternion(int handle, int bodyId, int component);
		static double GetBodyParaPosition(int handle, int bodyId, int component);
		static double GetBodyParaQuaternion(int handle, int bodyId, int component);
		static double GetBodyLinearVelocity(int handle, int bodyId, int component);
		static double GetBodyAngularVelocity(int handle, int bodyId, int component);
		static double GetBodyParaLinearVelocity(int handle, int bodyId, int component);
		static double GetBodyParaAngularVelocity(int handle, int bodyId, int component);
		static double RaycastDown(int handle, double x, double y, double z);
		static int GetContactGeom1(int handle, int contactIndex);
		static int GetContactGeom2(int handle, int contactIndex);
		static int GetContactBody1(int handle, int contactIndex);
		static int GetContactBody2(int handle, int contactIndex);
		static double GetContactDist(int handle, int contactIndex);
		static double GetContactPosition(int handle, int contactIndex, int component);
		static double GetContactNormal(int handle, int contactIndex, int component);
		static double GetContactParaPosition(int handle, int contactIndex, int component);
		static double GetContactParaNormal(int handle, int contactIndex, int component);
		static double GetContactForce(int handle, int contactIndex, int component);
		static double GetContactParaForce(int handle, int contactIndex, int component);
		static int FindHField(int handle, const std::string& name);
		static int FindGeom(int handle, const std::string& name);
		static int GetHFieldCount(int handle);
		static int GetHFieldNRow(int handle, int hfieldId);
		static int GetHFieldNCol(int handle, int hfieldId);
		static double GetHFieldSize(int handle, int hfieldId, int component);
		static double GetHFieldElevation(int handle, int hfieldId, int row, int col);
		static bool SetHFieldElevation(int handle, int hfieldId, int row, int col, double value);
		static bool FillHFieldElevation(int handle, int hfieldId, double value);
		static double GetGeomPosition(int handle, int geomId, int component);
		static bool SetGeomPosition(int handle, int geomId, int component, double value);
		static double GetGeomParaPosition(int handle, int geomId, int component);
		static bool SetGeomParaPosition(int handle, int geomId, double x, double y, double z);
	};
}

#endif
