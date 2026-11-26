//-----------------------------------------------------------------------------
// File:	PhysicsContraptionTest.cpp
// Desc:	Unit & physics integration tests for:
//			1. BMaxParser concave greedy box compound shapes & BoneBlock joint extraction
//			2. PhysicsBT CompoundShape concave raycast accuracy
//			3. Arbitrary-axis Slider (vertical elevator), Hinge motor, and Spring constraints
//			4. Live Kinematic <-> Dynamic mode switching, SetMass, and GetActiveContacts
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Core/ParaEngineAppBase.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXModel.h"
#include "BMaxModel/BMaxParser.h"
#include "Framework/Interface/IParaPhysics.h"
#include "../Plugins/PhysicsBT/ParaPhysicsWorld.h"
#include "3dengine/PhysicsWorld.h"
#include "3dengine/BaseObject.h"
#include "NPL/NPLHelper.h"
#include "NPL/NPLTable.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace ParaEngine;

namespace
{
	int g_passed = 0;
	int g_failed = 0;

	void Check(bool cond, const char* expr, const char* file, int line)
	{
		if (cond)
		{
			++g_passed;
		}
		else
		{
			++g_failed;
			std::fprintf(stderr, "[FAIL] %s:%d: %s\n", file, line, expr);
		}
	}

#define TEST_CHECK(expr) Check((expr), #expr, __FILE__, __LINE__)

	void TestBMaxConcaveCompoundAndBoneJoint()
	{
		// Build a concave '+' cross-shaped revolving door in BMax XML:
		// X-arm: x in [-2..2], z = 0, y in [0..2]
		// Z-arm: x = 0, z in [-2..2], y in [0..2]
		// Center (0, 0, 0) has a BoneBlock (id=253, data=4 => +Y axis) named "hinge_door".
		// Total occupied voxel cells = (5 + 5 - 1) * 3 = 27 cells out of 5*3*5 = 75 AABB cells.
		std::string xml =
			"<pe:blocktemplate auto_scale=\"false\" physics_property=\"{Mass=8,Restitution=0.35}\">\n"
			"<pe:blocks>{\n"
			"{0,0,0,253,4,{[1]={[1]=\"hinge_door\"},attr={bx=0,by=0,bz=0,class=\"EntityBone\"}}},\n";

		for (int y = 0; y <= 2; ++y)
		{
			for (int x = -2; x <= 2; ++x)
			{
				if (x == 0 && y == 0)
					continue; // BoneBlock already at (0,0,0)
				char buf[64];
				std::snprintf(buf, sizeof(buf), "{%d,%d,0,10,4095},\n", x, y);
				xml += buf;
			}
			for (int z = -2; z <= 2; ++z)
			{
				if (z == 0)
					continue; // intersection already added
				char buf[64];
				std::snprintf(buf, sizeof(buf), "{0,%d,%d,10,3840},\n", y, z);
				xml += buf;
			}
		}
		xml += "}</pe:blocks>\n</pe:blocktemplate>\n";

		BMaxParser parser;
		parser.Load(xml.c_str(), (int)xml.size());
		CParaXModel* pModel = parser.ParseParaXModel();
		TEST_CHECK(pModel != nullptr);
		if (!pModel)
			return;

		TEST_CHECK(pModel->GetDefaultPhysicsProperty() == "{Mass=8,Restitution=0.35}");

		const auto& boxes = pModel->GetPhysicsBoxes();
		// Greedy 3D merger should merge a '+' cross into 3 boxes (center X-bar + 2 Z-wings, or vice-versa)
		TEST_CHECK(boxes.size() >= 2 && boxes.size() <= 5);

		double totalBoxVol = 0.0;
		for (const auto& b : boxes)
		{
			totalBoxVol += (double)(b.m_halfExtents.x * 2.f) * (double)(b.m_halfExtents.y * 2.f) * (double)(b.m_halfExtents.z * 2.f);
		}
		// Unscaled block size is 1.0 when auto_scale="false", so total volume should equal 27.0 (not 75.0!)
		const float modelW = pModel->GetHeader().maxExtent.x - pModel->GetHeader().minExtent.x;
		const float voxelUnit = modelW / 5.0f;
		const double expectedVol = 27.0 * (double)voxelUnit * (double)voxelUnit * (double)voxelUnit;
		TEST_CHECK(std::fabs(totalBoxVol - expectedVol) < 1e-2);

		// Verify BoneBlock joint extraction
		const auto& joints = pModel->GetPhysicsJointDefs();
		TEST_CHECK(joints.size() == 1);
		if (!joints.empty())
		{
			TEST_CHECK(joints[0].m_constraintType == (int)ConstraintType_Hinge);
			TEST_CHECK(std::fabs(joints[0].m_axis.x) < 1e-4f);
			TEST_CHECK(std::fabs(joints[0].m_axis.y - 1.0f) < 1e-4f);
			TEST_CHECK(std::fabs(joints[0].m_axis.z) < 1e-4f);
		}

		// Verify Bullet CompoundShape concave raycast accuracy (arm hits, empty corner quadrant misses)
		CParaPhysicsWorld world;
		world.InitPhysics();

		const float halfH = pModel->GetHeader().maxExtent.y * 0.5f;
		std::vector<ParaPhysicsCompoundChildDesc> children;
		for (const auto& b : boxes)
		{
			ParaPhysicsCompoundChildDesc c;
			c.m_shape = "box";
			c.m_halfExtents = PARAVECTOR3(b.m_halfExtents.x, b.m_halfExtents.y, b.m_halfExtents.z);
			c.m_localOrigin = PARAVECTOR3(b.m_center.x, b.m_center.y - halfH, b.m_center.z);
			children.push_back(c);
		}

		IParaPhysicsShape* compoundShape = world.CreateCompoundShape(children.data(), (int)children.size());
		TEST_CHECK(compoundShape != nullptr);

		ParaPhysicsActorDesc actorDesc;
		actorDesc.m_origin = PARAVECTOR3(0.f, 5.f, 0.f);
		actorDesc.m_mass = 5.0f;
		actorDesc.m_pShape = compoundShape;
		IParaPhysicsActor* crossActor = world.CreateActor(actorDesc);
		TEST_CHECK(crossActor != nullptr);

		RayCastHitResult hitArm;
		IParaPhysicsActor* hit1 = world.RaycastClosestShapeEx(
			PARAVECTOR3(1.8f * voxelUnit, 12.f, 0.f),
			PARAVECTOR3(0.f, -1.f, 0.f), 0, hitArm, -1, 20.f, true);
		TEST_CHECK(hit1 == crossActor);

		RayCastHitResult hitHollowCorner;
		IParaPhysicsActor* hit2 = world.RaycastClosestShapeEx(
			PARAVECTOR3(1.8f * voxelUnit, 12.f, 1.8f * voxelUnit),
			PARAVECTOR3(0.f, -1.f, 0.f), 0, hitHollowCorner, -1, 20.f, true);
		TEST_CHECK(hit2 == nullptr);

		world.ExitPhysics();
		SAFE_DELETE(pModel);
	}

	void TestSliderHingeAndSpringConstraints()
	{
		CParaPhysicsWorld world;
		world.InitPhysics();

		ParaPhysicsSimpleShapeDesc boxDesc;
		boxDesc.m_shape = "box";
		boxDesc.m_halfWidth = 1.0f;
		boxDesc.m_halfHeight = 0.25f;
		boxDesc.m_halfLength = 1.0f;
		IParaPhysicsShape* platShape = world.CreateSimpleShape(boxDesc);
		TEST_CHECK(platShape != nullptr);

		// 1. Vertical Slider Elevator along +Y (0, 1, 0)
		ParaPhysicsActorDesc elevDesc;
		elevDesc.m_origin = PARAVECTOR3(0.f, 2.0f, 0.f);
		elevDesc.m_mass = 10.0f;
		elevDesc.m_pShape = platShape;
		IParaPhysicsActor* elevActor = world.CreateActor(elevDesc);
		TEST_CHECK(elevActor != nullptr);

		ParaPhysicsConstraintDesc sliderDesc;
		sliderDesc.m_type = ConstraintType_Slider;
		sliderDesc.m_pActorA = elevActor;
		sliderDesc.m_pActorB = nullptr;
		sliderDesc.m_pivotInA = PARAVECTOR3(0.f, 0.f, 0.f);
		sliderDesc.m_axisInA = PARAVECTOR3(0.f, 1.f, 0.f);
		sliderDesc.m_lowLimit = 0.0f;
		sliderDesc.m_highLimit = 4.0f;
		sliderDesc.m_enableMotor = true;
		sliderDesc.m_motorTargetVelocity = 3.0f;
		sliderDesc.m_maxMotorImpulse = 600.0f;
		IParaPhysicsConstraint* slider = world.CreateConstraint(sliderDesc);
		TEST_CHECK(slider != nullptr);

		for (int i = 0; i < 180; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
		}

		PARAVECTOR3 elevPos = elevActor->GetOrigin();
		// Started at y=2.0, slider upper limit = +4.0 => should climb and stop right around y=6.0!
		TEST_CHECK(elevPos.y > 5.7f && elevPos.y < 6.2f);
		TEST_CHECK(std::fabs(elevPos.x) < 0.05f && std::fabs(elevPos.z) < 0.05f);

		// Reverse motor downward
		slider->EnableSliderMotor(true, -3.0f, 600.0f);
		for (int i = 0; i < 180; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
		}
		elevPos = elevActor->GetOrigin();
		TEST_CHECK(elevPos.y > 1.8f && elevPos.y < 2.3f);

		// 2. Motorized Hinge Revolving Door around +Y
		ParaPhysicsActorDesc doorDesc;
		doorDesc.m_origin = PARAVECTOR3(10.f, 3.0f, 0.f);
		doorDesc.m_mass = 5.0f;
		doorDesc.m_pShape = platShape;
		IParaPhysicsActor* doorActor = world.CreateActor(doorDesc);

		ParaPhysicsConstraintDesc hingeDesc;
		hingeDesc.m_type = ConstraintType_Hinge;
		hingeDesc.m_pActorA = doorActor;
		hingeDesc.m_axisInA = PARAVECTOR3(0.f, 1.f, 0.f);
		hingeDesc.m_enableMotor = true;
		hingeDesc.m_motorTargetVelocity = 3.14159f;
		hingeDesc.m_maxMotorImpulse = 200.0f;
		IParaPhysicsConstraint* hinge = world.CreateConstraint(hingeDesc);
		TEST_CHECK(hinge != nullptr);

		for (int i = 0; i < 60; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
		}
		PARAVECTOR3 doorAngVel = doorActor->GetAngularVelocity();
		PARAVECTOR3 doorOrigin = doorActor->GetOrigin();
		TEST_CHECK(doorAngVel.y > 2.5f);
		TEST_CHECK(std::fabs(doorOrigin.x - 10.f) < 0.05f && std::fabs(doorOrigin.y - 3.0f) < 0.05f);

		// 3. Spring Pusher along +X
		ParaPhysicsActorDesc springBodyDesc;
		springBodyDesc.m_origin = PARAVECTOR3(20.f, 3.0f, 0.f);
		springBodyDesc.m_mass = 2.0f;
		springBodyDesc.m_pShape = platShape;
		IParaPhysicsActor* springActor = world.CreateActor(springBodyDesc);

		ParaPhysicsConstraintDesc springDesc;
		springDesc.m_type = ConstraintType_Spring;
		springDesc.m_pActorA = springActor;
		springDesc.m_axisInA = PARAVECTOR3(1.f, 0.f, 0.f);
		springDesc.m_lowLimit = -2.0f;
		springDesc.m_highLimit = 2.0f;
		springDesc.m_springStiffness = 120.0f;
		springDesc.m_springDamping = 0.05f;
		IParaPhysicsConstraint* spring = world.CreateConstraint(springDesc);
		TEST_CHECK(spring != nullptr);

		// Give it an outward impulse along +X
		springActor->ApplyCentralImpulse(PARAVECTOR3(12.0f, 0.f, 0.f));
		float maxX = 20.f;
		float minX = 20.f;
		for (int i = 0; i < 120; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
			float curX = springActor->GetOrigin().x;
			if (curX > maxX) maxX = curX;
			if (curX < minX) minX = curX;
		}
		// Should have extended forward > 20.3 and recoiled back past equilibrium (< 19.9)
		TEST_CHECK(maxX > 20.3f);
		TEST_CHECK(minX < 19.95f);

		world.ExitPhysics();
	}

	void TestKinematicToggleSetMassAndActiveContacts()
	{
		CParaPhysicsWorld world;
		world.InitPhysics();

		ParaPhysicsSimpleShapeDesc floorShapeDesc;
		floorShapeDesc.m_shape = "box";
		floorShapeDesc.m_halfWidth = 10.f;
		floorShapeDesc.m_halfHeight = 0.5f;
		floorShapeDesc.m_halfLength = 10.f;
		IParaPhysicsShape* floorShape = world.CreateSimpleShape(floorShapeDesc);

		ParaPhysicsActorDesc floorDesc;
		floorDesc.m_origin = PARAVECTOR3(0.f, -0.5f, 0.f);
		floorDesc.m_mass = 0.0f;
		floorDesc.m_pShape = floorShape;
		IParaPhysicsActor* floorActor = world.CreateActor(floorDesc);
		TEST_CHECK(floorActor != nullptr && floorActor->IsStaticObject());

		ParaPhysicsSimpleShapeDesc cylShapeDesc;
		cylShapeDesc.m_shape = "cylinder";
		cylShapeDesc.m_halfWidth = 0.5f;
		cylShapeDesc.m_halfHeight = 0.5f;
		cylShapeDesc.m_halfLength = 0.5f;
		IParaPhysicsShape* cylShape = world.CreateSimpleShape(cylShapeDesc);
		TEST_CHECK(cylShape != nullptr);

		ParaPhysicsActorDesc tileDesc;
		tileDesc.m_origin = PARAVECTOR3(0.f, 2.0f, 0.f);
		tileDesc.m_mass = 4.0f;
		tileDesc.m_pShape = cylShape;
		IParaPhysicsActor* tileActor = world.CreateActor(tileDesc);
		TEST_CHECK(tileActor != nullptr);

		// Switch to kinematic (like a falling-tile waiting to be stepped on)
		tileActor->SetKinematic(true);
		TEST_CHECK(tileActor->IsKinematicObject());
		for (int i = 0; i < 30; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
		}
		TEST_CHECK(std::fabs(tileActor->GetOrigin().y - 2.0f) < 1e-3f);

		// Trigger fall: switch back to dynamic with Mass = 6.0
		tileActor->SetKinematic(false);
		tileActor->SetMass(6.0f);
		TEST_CHECK(!tileActor->IsKinematicObject());

		bool sawImpactContact = false;
		for (int i = 0; i < 90; ++i)
		{
			world.StepSimulation(1.0f / 60.0f);
			std::vector<ParaPhysicsContactPair> contacts;
			if (world.GetActiveContacts(contacts, 0.1f) > 0)
			{
				sawImpactContact = true;
			}
		}
		TEST_CHECK(sawImpactContact);
		TEST_CHECK(tileActor->GetOrigin().y < 0.65f && tileActor->GetOrigin().y > 0.40f);

		world.ExitPhysics();
	}

	//////////////////////////////////////////////////////////////////////////
	// Contraption v2 tests
	//////////////////////////////////////////////////////////////////////////

	const int kCF_NO_CONTACT_RESPONSE = 4;

	IParaPhysicsActor* MakeBox(IParaPhysics* world, float hx, float hy, float hz, const PARAVECTOR3& pos, float mass, short group = 1, short mask = -1)
	{
		ParaPhysicsSimpleShapeDesc d;
		d.m_shape = "box";
		d.m_halfWidth = hx;
		d.m_halfHeight = hy;
		d.m_halfLength = hz;
		IParaPhysicsShape* shape = world->CreateSimpleShape(d);
		if (!shape)
			return nullptr;
		ParaPhysicsActorDesc a;
		a.m_origin = pos;
		a.m_mass = mass;
		a.m_group = group;
		a.m_mask = mask;
		a.m_pShape = shape;
		return world->CreateActor(a);
	}

	IParaPhysicsActor* MakeFloor(IParaPhysics* world)
	{
		return MakeBox(world, 20.f, 0.5f, 20.f, PARAVECTOR3(0.f, -0.5f, 0.f), 0.f);
	}

	/** CPhysicsWorld wired to a private Bullet world (no plugin DLL / scene needed). */
	struct ScopedEnginePhysics
	{
		CPhysicsWorld pw;
		ScopedEnginePhysics()
		{
			pw.m_pPhysicsWorld = new CParaPhysicsWorld();
			pw.m_pPhysicsWorld->InitPhysics();
		}
		~ScopedEnginePhysics()
		{
			pw.ExitPhysics();
		}
		IParaPhysics* bt() { return pw.m_pPhysicsWorld; }
		IParaPhysicsActor* Track(IParaPhysicsActor* a)
		{
			if (a)
				pw.m_mapDynamicActors.insert(a);
			return a;
		}
		void Step(float dt, int n)
		{
			for (int i = 0; i < n; ++i)
				pw.StepSimulation(dt);
		}
	};

	NPL::NPLObjectProxy ParseTable(const char* s)
	{
		return NPL::NPLHelper::StringToNPLTable(s ? s : "", s ? (int)strlen(s) : 0);
	}

	bool Near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) <= eps; }
	bool NearV(const Vector3& a, const Vector3& b, float eps = 1e-3f) { return Near(a.x, b.x, eps) && Near(a.y, b.y, eps) && Near(a.z, b.z, eps); }

	void TestGroupMaskAndSensor()
	{
		CParaPhysicsWorld world;
		world.InitPhysics();
		IParaPhysicsActor* floor = MakeFloor(&world); // group 1 => bit 0x2
		TEST_CHECK(floor != nullptr);

		// Default filter (-1) still collides with everything (behavior unchanged)
		IParaPhysicsActor* normal = MakeBox(&world, 0.5f, 0.5f, 0.5f, PARAVECTOR3(0.f, 2.f, 0.f), 1.f, 0, -1);
		// group 4 box whose mask excludes the floor group bit falls through it
		IParaPhysicsActor* ghost = MakeBox(&world, 0.5f, 0.5f, 0.5f, PARAVECTOR3(3.f, 2.f, 0.f), 1.f, 4, (short)~(1 << 1));
		TEST_CHECK(normal->GetCollisionMask() == -1 && ghost->GetCollisionGroup() == 4);
		// runtime filter change: this one starts colliding, then is told to ignore the floor
		IParaPhysicsActor* late = MakeBox(&world, 0.5f, 0.5f, 0.5f, PARAVECTOR3(6.f, 2.f, 0.f), 1.f, 0, -1);
		for (int i = 0; i < 90; ++i)
			world.StepSimulation(1.f / 60.f);
		TEST_CHECK(Near(normal->GetOrigin().y, 0.5f, 0.05f));
		TEST_CHECK(ghost->GetOrigin().y < -1.0f);
		TEST_CHECK(Near(late->GetOrigin().y, 0.5f, 0.05f));
		late->SetCollisionFilter(5, (short)~(1 << 1));
		for (int i = 0; i < 60; ++i)
			world.StepSimulation(1.f / 60.f);
		TEST_CHECK(late->GetOrigin().y < 0.0f);

		// Sensor: static trigger volume, a dynamic ball passes through but contacts are still reported
		IParaPhysicsActor* sensor = MakeBox(&world, 1.f, 0.5f, 1.f, PARAVECTOR3(10.f, 3.f, 0.f), 0.f);
		sensor->SetCollisionFlags(sensor->GetCollisionFlags() | kCF_NO_CONTACT_RESPONSE);
		IParaPhysicsActor* ball = MakeBox(&world, 0.2f, 0.2f, 0.2f, PARAVECTOR3(10.f, 5.f, 0.f), 1.f);
		bool sawSensorContact = false;
		for (int i = 0; i < 120; ++i)
		{
			world.StepSimulation(1.f / 60.f);
			std::vector<ParaPhysicsContactPair> contacts;
			world.GetActiveContacts(contacts, 0.f);
			for (const auto& c : contacts)
			{
				if (c.m_pActorA == sensor || c.m_pActorB == sensor)
					sawSensorContact = true;
			}
		}
		TEST_CHECK(sawSensorContact);
		TEST_CHECK(Near(ball->GetOrigin().y, 0.2f, 0.06f)); // rests on the floor, not on the sensor

		// rays ignore sensors
		RayCastHitResult hit;
		IParaPhysicsActor* rayHit = world.RaycastClosestShapeEx(PARAVECTOR3(10.5f, 6.f, 0.5f), PARAVECTOR3(0.f, -1.f, 0.f), 0, hit, -1, 20.f, true);
		TEST_CHECK(rayHit == floor);

		PARAVECTOR3 mn, mx;
		TEST_CHECK(sensor->GetAabb(mn, mx) && mn.x < 9.1f && mx.x > 10.9f);
		world.ExitPhysics();
	}

	void TestPathModeDeterministic()
	{
		std::vector<Vector3> pts;
		TEST_CHECK(CPhysicsWorld::ParsePathPoints("0,0,0; 4,0,0;4, 0 ,4", pts) == 3);
		TEST_CHECK(pts.size() == 3 && NearV(pts[2], Vector3(4.f, 0.f, 4.f)));
		TEST_CHECK(CPhysicsWorld::ParsePathPoints("{1,2,3};{4,5,6};bad;7,8", pts) == 2);

		CPhysicsWorld::KinematicMotionDesc km;
		km.m_mode = CPhysicsWorld::KinematicMotion_Path;
		km.m_baseOrigin = Vector3(10.f, 1.f, 0.f);
		km.m_pathPoints = { Vector3::ZERO, Vector3(4.f, 0.f, 0.f), Vector3(4.f, 0.f, 4.f) };
		km.m_pathLoop = CPhysicsWorld::KinematicPathLoop_PingPong;
		km.m_pathEasing = CPhysicsWorld::KinematicEasing_Linear;
		km.m_speed = 2.f;
		km.m_segmentPause = 0.5f;
		// legs: 2s, pause .5, 2s, pause .5, (back) 2s, .5, 2s, .5 => cycle 10s
		Vector3 o, lv, av;
		Quaternion q;
		TEST_CHECK(CPhysicsWorld::EvaluateKinematicMotion(km, 1.f, o, q, lv, av));
		TEST_CHECK(NearV(o, Vector3(12.f, 1.f, 0.f)) && NearV(lv, Vector3(2.f, 0.f, 0.f)));
		CPhysicsWorld::EvaluateKinematicMotion(km, 2.25f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 0.f)) && NearV(lv, Vector3::ZERO));
		CPhysicsWorld::EvaluateKinematicMotion(km, 3.5f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 2.f)) && NearV(lv, Vector3(0.f, 0.f, 2.f)));
		// Return leg (4,0,4)->(4,0,0) runs on [5.0, 7.0) after the [4.5, 5.0) turnaround pause:
		// midpoint (14,1,2) is at t=6.0, and 3/4 return (14,1,1) is at t=6.5.
		CPhysicsWorld::EvaluateKinematicMotion(km, 6.0f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 2.f)) && NearV(lv, Vector3(0.f, 0.f, -2.f)));
		CPhysicsWorld::EvaluateKinematicMotion(km, 6.5f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 1.f)) && NearV(lv, Vector3(0.f, 0.f, -2.f)));
		Vector3 o11;
		CPhysicsWorld::EvaluateKinematicMotion(km, 11.f, o11, q, lv, av);
		TEST_CHECK(NearV(o11, Vector3(12.f, 1.f, 0.f)));
		// phase offset is a fraction of the full cycle (0.5 * 10s = +5.0s, so t=1.0 == t=6.0)
		km.m_phaseOffset = 0.5f;
		CPhysicsWorld::EvaluateKinematicMotion(km, 1.0f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 2.f)) && NearV(lv, Vector3(0.f, 0.f, -2.f)));
		km.m_phaseOffset = 0.f;

		// Points are world-frame offsets relative to m_baseOrigin (NOT rotated by m_baseRot)
		km.m_baseRot.FromAngleAxis(Radian(MATH_PI * 0.5f), Vector3::UNIT_Y);
		CPhysicsWorld::EvaluateKinematicMotion(km, 1.f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(12.f, 1.f, 0.f)) && NearV(lv, Vector3(2.f, 0.f, 0.f)));
		km.m_baseRot = Quaternion::IDENTITY;

		// determinism: identical bits for identical inputs
		Vector3 a1, a2, v1, v2;
		CPhysicsWorld::EvaluateKinematicMotion(km, 7.123f, a1, q, v1, av);
		CPhysicsWorld::EvaluateKinematicMotion(km, 7.123f, a2, q, v2, av);
		TEST_CHECK(std::memcmp(&a1, &a2, sizeof(Vector3)) == 0 && std::memcmp(&v1, &v2, sizeof(Vector3)) == 0);

		// loop closes back to origin; once holds at the end
		km.m_pathLoop = CPhysicsWorld::KinematicPathLoop_Loop;
		km.m_segmentPause = 0.f;
		float closeDur = std::sqrt(32.f) / 2.f;
		CPhysicsWorld::EvaluateKinematicMotion(km, 4.f + closeDur * 0.5f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(12.f, 1.f, 2.f)));
		km.m_pathLoop = CPhysicsWorld::KinematicPathLoop_Once;
		CPhysicsWorld::EvaluateKinematicMotion(km, 100.f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(14.f, 1.f, 4.f)) && NearV(lv, Vector3::ZERO));
		// smooth easing: zero velocity at waypoints, mid-segment peak = pi/2 * avg speed
		km.m_pathEasing = CPhysicsWorld::KinematicEasing_Smooth;
		CPhysicsWorld::EvaluateKinematicMotion(km, 1.f, o, q, lv, av);
		TEST_CHECK(NearV(o, Vector3(12.f, 1.f, 0.f)) && Near(lv.x, 3.14159f, 1e-2f));
		CPhysicsWorld::EvaluateKinematicMotion(km, 1e-4f, o, q, lv, av);
		TEST_CHECK(lv.length() < 1e-2f);
		km.m_pathEasing = CPhysicsWorld::KinematicEasing_Bounce;
		CPhysicsWorld::EvaluateKinematicMotion(km, 1.999f, o, q, lv, av);
		TEST_CHECK(Near(o.x, 14.f, 0.02f));

		// legacy pingpong is unchanged: closed form of the cosine easing
		CPhysicsWorld::KinematicMotionDesc pp;
		pp.m_mode = CPhysicsWorld::KinematicMotion_PingPong;
		pp.m_linearOffset = Vector3(0.f, 4.f, 0.f);
		pp.m_speed = 2.f;
		pp.m_pauseTime = 0.5f;
		CPhysicsWorld::EvaluateKinematicMotion(pp, 0.7f, o, q, lv, av);
		float s = 0.7f / 2.f;
		TEST_CHECK(o.y == pp.m_linearOffset.y * (0.5f * (1.f - std::cos(s * MATH_PI))));
	}

	void TestKinematicTriggerClockAndPathActor()
	{
		ScopedEnginePhysics e;
		IParaPhysicsActor* plat = e.Track(MakeBox(e.bt(), 1.f, 0.2f, 1.f, PARAVECTOR3(0.f, 2.f, 0.f), 1.f));
		e.pw.SetActorPhysicsProperty(plat, "{method='SetKinematicMotion',Mode='path',Points='0,0,0;0,3,0',Loop='once',Speed=3,Easing='linear',StartOnTrigger=true}");
		TEST_CHECK(plat->IsKinematicObject());
		e.Step(1.f / 60.f, 60);
		TEST_CHECK(Near(plat->GetOrigin().y, 2.f, 1e-3f)); // waits for trigger
		NPL::NPLObjectProxy st = ParseTable(e.pw.GetActorPhysicsProperty(plat, "{method='KinematicState'}"));
		TEST_CHECK((bool)st["HasController"] && !(bool)st["Running"] && st["Mode"] == "path");

		e.pw.SetActorPhysicsProperty(plat, "{method='StartKinematicMotion'}");
		e.Step(1.f / 60.f, 30); // 0.5s at 3 m/s => +1.5
		TEST_CHECK(Near(plat->GetOrigin().y, 3.5f, 0.02f));
		e.pw.SetActorPhysicsProperty(plat, "{method='StopKinematicMotion'}");
		float yStop = plat->GetOrigin().y;
		e.Step(1.f / 60.f, 30);
		TEST_CHECK(Near(plat->GetOrigin().y, yStop, 1e-4f));
		e.pw.SetActorPhysicsProperty(plat, "{method='SetKinematicControllerEnabled',Enabled=true}");
		e.Step(1.f / 60.f, 120);
		TEST_CHECK(Near(plat->GetOrigin().y, 5.f, 1e-3f)); // once: holds at last point
		// ResetSimulationTime re-arms trigger controllers back to the base pose
		e.pw.ResetSimulationTime(0.0);
		e.Step(1.f / 60.f, 2);
		TEST_CHECK(Near(plat->GetOrigin().y, 2.f, 1e-3f));

		// ControllerEnabled=false pauses a legacy rotate controller
		IParaPhysicsActor* spinner = e.Track(MakeBox(e.bt(), 1.f, 0.2f, 0.2f, PARAVECTOR3(5.f, 2.f, 0.f), 1.f));
		e.pw.SetActorPhysicsProperty(spinner, "{method='SetKinematicMotion',Mode='rotate',Speed=2,ControllerEnabled=false}");
		e.Step(1.f / 60.f, 10);
		TEST_CHECK(spinner->GetAngularVelocity().y == 0.f);
		e.pw.SetActorPhysicsProperty(spinner, "{method='StartKinematicMotion'}");
		e.Step(1.f / 60.f, 2);
		TEST_CHECK(Near(spinner->GetAngularVelocity().y, 2.f, 1e-3f));
	}

	void TestSubstepKinematicNoTunnel()
	{
		float plateXWith = 0.f, plateXWithout = 0.f;
		for (int pass = 0; pass < 2; ++pass)
		{
			ScopedEnginePhysics e;
			e.pw.SetKinematicSubstepEnabled(pass == 0);
			MakeFloor(e.bt());
			IParaPhysicsActor* plate = e.Track(MakeBox(e.bt(), 0.05f, 0.5f, 0.5f, PARAVECTOR3(2.f, 0.5f, 0.f), 1.f));
			plate->SetActivationState(4); // DISABLE_DEACTIVATION
			IParaPhysicsActor* sweeper = e.Track(MakeBox(e.bt(), 0.2f, 0.45f, 0.6f, PARAVECTOR3(0.f, 0.55f, 0.f), 1.f));
			// 4 m in 0.4 s (peak ~15.7 m/s) -> up to 1.3 m per 1/12 s render frame, but only ~0.26 m per 1/60 substep
			e.pw.SetActorPhysicsProperty(sweeper, "{method='SetKinematicMotion',Mode='pingpong',OffsetX=4,OffsetY=0,OffsetZ=0,Speed=10,PauseTime=5}");
			e.Step(1.f / 12.f, 6);
			float x = plate->GetOrigin().x;
			if (pass == 0)
				plateXWith = x;
			else
				plateXWithout = x;
		}
		std::printf("substep_kinematic_no_tunnel: plate x with substeps=%.3f, without=%.3f\n", plateXWith, plateXWithout);
		TEST_CHECK(plateXWith > 2.5f);           // swept away by the arm
		TEST_CHECK(plateXWithout < plateXWith);  // legacy per-frame pose tunnels (or pushes less)
	}

	void TestForceZones()
	{
		ScopedEnginePhysics e;
		CPhysicsWorld& pw = e.pw;
		pw.SetForceZoneStr("{method='add',name='fan',center={0,2,0},halfSize={2,2,2},force={0,25,0},mode='wind'}");
		pw.SetForceZoneStr("{method='add',name='water',center={10,1,0},halfSize={2,1,2},mode='buoyancy',force={0,20,0}}");
		pw.SetForceZoneStr("{method='add',name='tmp',center={x=50,y=0,z=0},halfSize={x=1,y=1,z=1},mode='radial',strength=5}");
		TEST_CHECK(pw.GetForceZoneCount() == 3);
		pw.SetForceZoneStr("{method='add',name='tmp',center={60,0,0},halfSize={1,1,1},mode='radial',strength=-5,falloff=1}"); // replace by name
		TEST_CHECK(pw.GetForceZoneCount() == 3);

		Vector3 a;
		float drag = 0.f;
		TEST_CHECK(pw.ComputeForceZoneAcceleration(Vector3(0.f, 1.f, 0.f), 0.5f, false, a, drag) && NearV(a, Vector3(0.f, 25.f, 0.f)));
		TEST_CHECK(!pw.ComputeForceZoneAcceleration(Vector3(3.f, 1.f, 0.f), 0.5f, false, a, drag));
		// radial pull toward center with linear falloff
		TEST_CHECK(pw.ComputeForceZoneAcceleration(Vector3(60.5f, 0.f, 0.f), 0.f, true, a, drag) && a.x < 0.f);
		// buoyancy: fully submerged => full force, default drag
		TEST_CHECK(pw.ComputeForceZoneAcceleration(Vector3(10.f, 0.5f, 0.f), 0.2f, false, a, drag) && NearV(a, Vector3(0.f, 20.f, 0.f)) && drag > 0.f);
		// yaw-rotated wind with falloff: strongest at the inlet face, weaker downstream
		pw.SetForceZoneStr("{method='add',name='side',center={0,10,20},halfSize={4,1,1},yaw=1.5707963,force={0,0,-8},falloff=1}");
		Vector3 aIn, aOut;
		TEST_CHECK(pw.ComputeForceZoneAcceleration(Vector3(0.f, 10.f, 23.5f), 0.f, false, aIn, drag));
		TEST_CHECK(pw.ComputeForceZoneAcceleration(Vector3(0.f, 10.f, 16.5f), 0.f, false, aOut, drag));
		TEST_CHECK(std::fabs(aIn.z) > std::fabs(aOut.z) && aIn.z < 0.f);
		NPL::NPLObjectProxy zones = ParseTable(pw.GetForceZonesStr());
		TEST_CHECK((double)zones["count"] == 4.0);

		// a fan lifts a resting dynamic body (gravity -10 + 25 up)
		MakeFloor(e.bt());
		IParaPhysicsActor* box = e.Track(MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(0.f, 0.3f, 0.f), 2.f));
		IParaPhysicsActor* outside = e.Track(MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(5.f, 0.3f, 0.f), 2.f));
		e.Step(1.f / 60.f, 30);
		TEST_CHECK(box->GetOrigin().y > 1.5f);
		TEST_CHECK(Near(outside->GetOrigin().y, 0.3f, 0.05f));
		pw.SetForceZoneStr("{method='remove',name='fan'}");
		TEST_CHECK(pw.GetForceZoneCount() == 3);
		pw.SetForceZoneStr("{method='clear'}");
		TEST_CHECK(pw.GetForceZoneCount() == 0);
	}

	void TestPhysicsEventsAndLimits()
	{
		ScopedEnginePhysics e;
		CPhysicsWorld& pw = e.pw;
		IParaPhysicsActor* floor = MakeFloor(e.bt());
		(void)floor;

		// joint break: heavy box hanging from a weak point-to-point joint
		IParaPhysicsActor* hanging = e.Track(MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(0.f, 5.f, 0.f), 10.f));
		IParaPhysicsConstraint* joint = pw.CreateConstraint(ConstraintType_PointToPoint, hanging, nullptr,
			Vector3(0.f, 0.3f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 5.3f, 0.f), Vector3(0.f, 1.f, 0.f), true);
		TEST_CHECK(joint != nullptr);
		pw.SetConstraintProperty(joint, "{BreakForce=0.5}");
		e.Step(1.f / 60.f, 5);
		TEST_CHECK(!joint->IsEnabled());
		NPL::NPLObjectProxy ev = ParseTable(pw.PopPhysicsEventsStr());
		TEST_CHECK((double)ev["count"] == 1.0 && ev[1]["type"] == "jointBreak");
		TEST_CHECK((double)ParseTable(pw.PopPhysicsEventsStr())["count"] == 0.0); // popped
		NPL::NPLObjectProxy jp = ParseTable(pw.GetConstraintProperty(joint));
		TEST_CHECK((bool)jp["Broken"]);
		// a user-disabled breakable joint is not reported as broken
		IParaPhysicsActor* h2 = e.Track(MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(3.f, 5.f, 0.f), 1.f));
		IParaPhysicsConstraint* j2 = pw.CreateConstraint(ConstraintType_PointToPoint, h2, nullptr,
			Vector3(0.f, 0.3f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(3.f, 5.3f, 0.f), Vector3(0.f, 1.f, 0.f), true);
		pw.SetConstraintProperty(j2, "{BreakingThreshold=1000,Enabled=false}");
		e.Step(1.f / 60.f, 2);

		// sensor enter / exit with a named falling ball
		IParaPhysicsActor* sensor = e.Track(MakeBox(e.bt(), 1.f, 0.4f, 1.f, PARAVECTOR3(10.f, 2.f, 0.f), 0.f));
		pw.SetActorPhysicsProperty(sensor, "{Sensor=true}");
		NPL::NPLObjectProxy sp = ParseTable(pw.GetActorPhysicsProperty(sensor));
		TEST_CHECK((bool)sp["Sensor"]);
		CBaseObject* ballObj = new CBaseObject();
		ballObj->SetIdentifier("ball");
		IParaPhysicsActor* ball = MakeBox(e.bt(), 0.15f, 0.15f, 0.15f, PARAVECTOR3(10.f, 4.f, 0.f), 1.f);
		ball->SetUserData(ballObj);
		bool sawEnter = false, sawExit = false;
		for (int i = 0; i < 90; ++i)
		{
			e.Step(1.f / 60.f, 1);
			NPL::NPLObjectProxy evs = ParseTable(pw.PopPhysicsEventsStr());
			int n = (int)(double)evs["count"];
			for (int k = 1; k <= n; ++k)
			{
				if (evs[k]["type"] == "sensorEnter" && evs[k]["b"] == "ball")
					sawEnter = true;
				if (evs[k]["type"] == "sensorExit" && evs[k]["b"] == "ball")
					sawExit = true;
				TEST_CHECK(!(evs[k]["type"] == "jointBreak"));
			}
		}
		TEST_CHECK(sawEnter && sawExit);
		TEST_CHECK(ball->GetOrigin().y < 0.5f); // fell through the sensor

		// impact events are opt-in
		pw.SetImpactEventThreshold(1.0f);
		IParaPhysicsActor* crate = MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(-5.f, 4.f, 0.f), 5.f);
		CBaseObject* crateObj = new CBaseObject();
		crateObj->SetIdentifier("crate");
		crate->SetUserData(crateObj);
		bool sawImpact = false;
		for (int i = 0; i < 90 && !sawImpact; ++i)
		{
			e.Step(1.f / 60.f, 1);
			NPL::NPLObjectProxy evs = ParseTable(pw.PopPhysicsEventsStr());
			int n = (int)(double)evs["count"];
			for (int k = 1; k <= n; ++k)
				if (evs[k]["type"] == "impact" && (evs[k]["a"] == "crate" || evs[k]["b"] == "crate"))
					sawImpact = true;
		}
		TEST_CHECK(sawImpact);
		pw.SetImpactEventThreshold(0.f);
		// queue is capped at 64
		for (int i = 0; i < 100; ++i)
			pw.PushPhysicsEvent("impact", "x", "y", 1.f);
		TEST_CHECK(pw.GetPhysicsEventCount() == 64);
		pw.PopPhysicsEventsStr();

		// 6DoF 3D limits round-trip through Set/GetConstraintProperty
		IParaPhysicsActor* slider = e.Track(MakeBox(e.bt(), 0.3f, 0.3f, 0.3f, PARAVECTOR3(20.f, 3.f, 0.f), 1.f));
		IParaPhysicsConstraint* dof = pw.CreateConstraint(ConstraintType_Generic6Dof, slider, nullptr,
			Vector3(0.f, 0.f, 0.f), Vector3(1.f, 0.f, 0.f), Vector3(20.f, 3.f, 0.f), Vector3(1.f, 0.f, 0.f), true);
		TEST_CHECK(dof != nullptr);
		pw.SetConstraintProperty(dof, "{LinearLowerX=-1,LinearUpperX=1,LinearLowerY=0,LinearUpperY=0,AngularLowerZ=-0.5,AngularUpperZ=0.5}");
		NPL::NPLObjectProxy dp = ParseTable(pw.GetConstraintProperty(dof));
		TEST_CHECK(Near((float)(double)dp["LinearLowerX"], -1.f) && Near((float)(double)dp["LinearUpperX"], 1.f));
		TEST_CHECK(Near((float)(double)dp["AngularLowerZ"], -0.5f) && Near((float)(double)dp["AngularUpperZ"], 0.5f));
		e.Step(1.f / 60.f, 60);
		TEST_CHECK(Near(slider->GetOrigin().y, 3.f, 0.05f)); // Y locked against gravity

		// ConeTwist swing limit keeps a pendulum within ~SwingSpan
		IParaPhysicsActor* bob = e.Track(MakeBox(e.bt(), 0.2f, 0.2f, 0.2f, PARAVECTOR3(30.f, 3.f, 0.f), 1.f));
		IParaPhysicsConstraint* cone = pw.CreateConstraint(ConstraintType_ConeTwist, bob, nullptr,
			Vector3(0.f, 1.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(30.f, 4.f, 0.f), Vector3(0.f, 1.f, 0.f), true);
		TEST_CHECK(cone != nullptr);
		pw.SetConstraintProperty(cone, "{SwingSpan1=0.3,SwingSpan2=0.3,TwistSpan=0.2}");
		bob->ApplyCentralImpulse(PARAVECTOR3(6.f, 0.f, 0.f));
		float maxDx = 0.f;
		for (int i = 0; i < 120; ++i)
		{
			e.Step(1.f / 60.f, 1);
			maxDx = (std::max)(maxDx, std::fabs(bob->GetOrigin().x - 30.f));
		}
		TEST_CHECK(maxDx < 0.6f); // sin(0.3) * 1m ~= 0.30 (+ softness); an unlimited swing reaches > 0.9

		// angular spring on a Spring joint returns toward rest
		IParaPhysicsActor* tilt = e.Track(MakeBox(e.bt(), 1.f, 0.1f, 1.f, PARAVECTOR3(40.f, 3.f, 0.f), 2.f));
		tilt->SetGravity(PARAVECTOR3(0.f, 0.f, 0.f));
		IParaPhysicsConstraint* spring = pw.CreateConstraint(ConstraintType_Spring, tilt, nullptr,
			Vector3(0.f, 0.f, 0.f), Vector3(1.f, 0.f, 0.f), Vector3(40.f, 3.f, 0.f), Vector3(1.f, 0.f, 0.f), true);
		pw.SetConstraintProperty(spring, "{LowLimit=0,HighLimit=0,AngularLowerX=-1,AngularUpperX=1,AngularStiffnessX=200,AngularDampingX=0.5}");
		tilt->ApplyTorqueImpulse(PARAVECTOR3(1.f, 0.f, 0.f));
		float minW = 1e9f, maxW = -1e9f;
		for (int i = 0; i < 90; ++i)
		{
			e.Step(1.f / 60.f, 1);
			float w = tilt->GetAngularVelocity().x;
			minW = (std::min)(minW, w);
			maxW = (std::max)(maxW, w);
		}
		TEST_CHECK(maxW > 0.05f && minW < -0.05f); // oscillates back

		TEST_CHECK(std::strstr(CPhysicsWorld::GetPhysicsFeatures(), "forcezone") != nullptr);
	}

	void TestAutoCCD()
	{
		ScopedEnginePhysics e;
		IParaPhysicsActor* small = e.Track(MakeBox(e.bt(), 0.1f, 0.1f, 0.1f, PARAVECTOR3(0.f, 5.f, 0.f), 1.f));
		IParaPhysicsActor* big = e.Track(MakeBox(e.bt(), 1.f, 1.f, 1.f, PARAVECTOR3(5.f, 5.f, 0.f), 1.f));
		e.pw.ApplyAutoCCD(small, true, true);
		e.pw.ApplyAutoCCD(big, true, true);
		TEST_CHECK(small->GetCcdMotionThreshold() > 0.f && small->GetCcdSweptSphereRadius() > 0.f && small->GetCcdSweptSphereRadius() < 0.1f);
		TEST_CHECK(big->GetCcdMotionThreshold() == 0.f);
		e.pw.SetActorPhysicsProperty(big, "{AutoCCD=true}");
		TEST_CHECK(big->GetCcdMotionThreshold() > 0.f);
		e.pw.SetActorPhysicsProperty(big, "{AutoCCD=false}");
		TEST_CHECK(big->GetCcdMotionThreshold() == 0.f);

		// a fast small bullet does not tunnel through a thin static wall
		MakeBox(e.bt(), 0.05f, 2.f, 2.f, PARAVECTOR3(3.f, 5.f, 0.f), 0.f);
		small->SetGravity(PARAVECTOR3(0.f, 0.f, 0.f));
		small->SetLinearVelocity(PARAVECTOR3(120.f, 0.f, 0.f)); // 2 m per substep
		e.Step(1.f / 60.f, 10);
		TEST_CHECK(small->GetOrigin().x < 3.f);
	}
}

int main()
{
	CParaEngineAppBase application;
	CParaWorldAsset assetManager;

	TestBMaxConcaveCompoundAndBoneJoint();
	TestSliderHingeAndSpringConstraints();
	TestKinematicToggleSetMassAndActiveContacts();
	TestGroupMaskAndSensor();
	TestPathModeDeterministic();
	TestKinematicTriggerClockAndPathActor();
	TestSubstepKinematicNoTunnel();
	TestForceZones();
	TestPhysicsEventsAndLimits();
	TestAutoCCD();

	std::printf("PhysicsContraptionTest: %d passed, %d failed\n", g_passed, g_failed);
	return (g_failed == 0) ? 0 : 1;
}
