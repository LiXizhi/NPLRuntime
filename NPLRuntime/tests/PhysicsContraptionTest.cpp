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
}

int main()
{
	CParaEngineAppBase application;
	CParaWorldAsset assetManager;

	TestBMaxConcaveCompoundAndBoneJoint();
	TestSliderHingeAndSpringConstraints();
	TestKinematicToggleSetMassAndActiveContacts();

	std::printf("PhysicsContraptionTest: %d passed, %d failed\n", g_passed, g_failed);
	return (g_failed == 0) ? 0 : 1;
}
