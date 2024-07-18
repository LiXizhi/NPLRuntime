//-----------------------------------------------------------------------------
// Class:	CAutoCamera
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.3.8
// Revised: 2005.3.8
// desc	: Auto Camera is designed to handle smooth transitions between 
// supported camera type, such as first person camera, third person camera,
// and rotation camera. It can be set to follow a Biped Object in the 
// scene. The Scene Object automatically creates one instance of this 
// object when loading; and manages the entire camera automatically.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "terrain/GlobalTerrain.h"
#include "SceneObject.h"
#include "BipedStateManager.h"
#include "OceanManager.h"
#include "2dengine/GUIRoot.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockCommon.h"
#include "IParaEngineApp.h"
#include "KeyFrame.h"

#include "MiscEntity.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#include "AISimulator.h"
#include "EventsCenter.h"
#include "AutoCamera.h"
using namespace ParaEngine;


/** if this is defined. "C" and "F" key will be enabled, which allow user to use free camera and non-behind camera follow mode. */
//#define ENABLE_CAMERA_MODE_KEY

/** if this is defined. Character control actions will be handled here, such as jump, toggle running, mount, etc  */
//#define ENABLE_PLAYER_CONTROL_KEY

/**@def define this macro to enable camera constraint when camera is near the water surface.
The constraint will not allow camera to stay near the water's wave [-amplitude,amplitude] range.*/
//#define ENABLE_WATER_CAMERA_CONSTRAINT

/** @def the camera will only automatically stay behind the character, when target is moving */
//#define CAMERA_ONLY_FOLLOW_WHEN_TARGET_IS_MOVING

/** #def default camera zoom in|out speed. */
#define DEFAULT_ZOOM_SPEED	5.0f

/* default keyboard move velocity per second */
#define KEYBOARD_MOVE_VELOCITY	5.f

/** @def the rotation speed of the camera around the Y axis. */
#define CAMERA_Y_ANGLE_SPEED		5.0f

/** @def maximum camera lift up angle. */
#define MAX_CAM_LIFTUP_UPANGLE (MATH_PI*0.45f)
/** @def minimum camera lift up angle. */
#define MIN_CAM_LIFTUP_UPANGLE (MATH_PI*-0.45f)

/** @def the maximum transition time from one camera mode to another*/
#define  MAX_TRANSITION_TIME		5.0f

#define DEFVIEW_MIN_HEIGHT		3.5f
#define FFPVIEW_MIN_HEIGHT		3.0f
/** the default camera rollback speed. */
#define		DEFAULT_ROLLBACK_SPEED	6.0f;

/** if the mouse moves within this distance in pixel during a mouse down and up event, it will be regarded as a mouse click*/
#define MOUSE_CLICK_DISTANCE	2

/** default physics group mask. it will ignore physics group 0 and 2*/
#define DEFAULT_PHYSICS_GROUP_MASK 0xfffffff2

//-----------------------------------------------------------------------------
// Name: ConstrainToBoundary
/// Desc: Clamps pV to lie inside vMinBoundary & MaxBoundary
//-----------------------------------------------------------------------------
void CAutoCamera::CameraConstraint::ConstrainToBoundary(Vector3* pV, const Vector3* pvMin, const Vector3* pvMax)
{
	// Constrain vector to a bounding box 
	pV->x = max(pV->x, pvMin->x);
	pV->y = max(pV->y, pvMin->y);
	pV->z = max(pV->z, pvMin->z);

	pV->x = min(pV->x, pvMax->x);
	pV->y = min(pV->y, pvMax->y);
	pV->z = min(pV->z, pvMax->z);
}

//-----------------------------------------------------------------------------
// Name: ConstrainToBoundary
/// Desc: Clamps pV to lie outside vMinBoundary & MaxBoundary
//-----------------------------------------------------------------------------
void CAutoCamera::CameraConstraint::ConstrainToOuterBoundary(Vector3* pV, const Vector3* pvMin, const Vector3* pvMax)
{
	// Constrain vector to a bounding box 
	pV->x = min(pV->x, pvMin->x);
	pV->y = min(pV->y, pvMin->y);
	pV->z = min(pV->z, pvMin->z);

	pV->x = max(pV->x, pvMax->x);
	pV->y = max(pV->y, pvMax->y);
	pV->z = max(pV->z, pvMax->z);
}

//-----------------------------------------------------------------------------
/// Name: UpdateFocusConstraint
/// Desc: Set internal constraint based on the parameters
/// param: pV is the object point that the camera focus.
/// fLookAtRadius is the Loot at radius
/// fEyeRadius is the Eye radius
/// Note: Radius is half of the side length of the cubic within which camera must be;
/// and both box must be above y=0 plane.
//-----------------------------------------------------------------------------
void CAutoCamera::CameraConstraint::UpdateFocusConstraint(const Vector3* pV,
	FLOAT fLookAtRadius, FLOAT fLookAtMinHeight, FLOAT fLookAtMaxHeight,
	FLOAT fEyeRadius, FLOAT fEyeMinHeight, FLOAT fEyeMaxHeight)
{
	// look at boundary
	m_vMinLookAt.x = pV->x - fLookAtRadius;
	m_vMinLookAt.y = pV->y + fLookAtMinHeight;
	m_vMinLookAt.z = pV->z - fLookAtRadius;

	m_vMaxLookAt.x = pV->x + fLookAtRadius;
	m_vMaxLookAt.y = pV->y + fLookAtMaxHeight;
	m_vMaxLookAt.z = pV->z + fLookAtRadius;

	// eye at boundary
	m_vMinEye.x = pV->x - fEyeRadius;
	m_vMinEye.y = pV->y + fEyeMinHeight;
	m_vMinEye.z = pV->z - fEyeRadius;

	m_vMaxEye.x = pV->x + fEyeRadius;
	m_vMaxEye.y = pV->y + fEyeMaxHeight;
	m_vMaxEye.z = pV->z + fEyeRadius;
}

void CAutoCamera::CameraConstraint::BoundToFocusConstraint(Vector3* pEye, Vector3* pLookAt)
{
	ConstrainToBoundary(pEye, &m_vMinEye, &m_vMaxEye);
	//ConstrainToOuterBoundary(pEye, &m_vMinLookAt, &m_vMaxLookAt);

	ConstrainToBoundary(pLookAt, &m_vMinLookAt, &m_vMaxLookAt);
}

CAutoCamera::CAutoCamera()
	:m_fLookAtShiftY(0), m_event(NULL), m_dwPhysicsGroupMask(DEFAULT_PHYSICS_GROUP_MASK),
	m_bEnableMouseLeftDrag(true), m_bEnableMouseRightDrag(true), m_bUseCharacterLookup(false), m_bUseCharacterLookupWhenMounted(true), m_nCharacterLookupBoneIndex(-1),
	m_bBlockInput(false), m_bAlwaysRotateCameraWhenFPS(false), m_bFirstPerson(false), m_vLookAtOffset(0, 0, 0), m_vAdditionalCameraRotate(0, 0, 0), m_fAllowedCharYShift(0), m_fLastCharY(0), m_fLastUsedCharY(0), m_bipedFlyNormal(0, 1, 0), m_fMaxYShiftSpeed(1.f), m_bEnableBlockCollision(true), m_bEnableTerrainCollision(true), m_bIgnoreEyeBlockCollisionInSunlight(true), m_bLockMouseWhenDragging(false),
	m_fForceOmniCameraObjectDistance(-10000), m_fForceOmniCameraPitch(-10000)
{
	m_bUseRightButtonBipedFacing = true;
	m_bTurnBipedWhenWalkBackward = false;
	m_pTargetObject = NULL;
	m_bIsFollowMode = true;		// default to follow mode: third person
	m_fEyeSpeed = OBJ_UNIT * 4.0f;	// 2 is good
	m_fLookAtSpeed = OBJ_UNIT * 4.0f;	// 2 is good
	m_fLookUpSpeed = 1.0f;
	m_nMouseWheelDelta = 0;
	m_bControlBiped = true;
	m_bAlwaysRun = false;
	m_bIsCamAlwaysBehindObject = true;
	m_dTransitionAmt = 0;
	m_fCameraRollbackSpeed = DEFAULT_ROLLBACK_SPEED;
	m_nForceNoRollbackFrames = 0;
	m_fCamZoomSpeed = DEFAULT_ZOOM_SPEED;

	m_bEnableKeyboard = true;
	m_bEnableMouseLeftButton = true;
	m_bEnableMouseRightButton = true;
	m_bEnableMouseWheel = true;
	m_resetFlyNormal = false;

	// between 0.6 meters and 9 meters
	m_fMinimumHeight = OBJ_UNIT * 0.6f;
	m_fMaximumHeight = 15.f;
	m_fMinCameraObjectDistance = 0.01f;
	m_fMaxCameraObjectDistance = 20.f;
	Vector3 a(0, m_fMinimumHeight, 0);
	Vector3 b(1000.f, m_fMaximumHeight, 1000.f);
	SetClipToBoundary(true, &a, &b);

	SetDefaultAngles();
	SetCameraObjectDistance(FFPVIEW_MIN_HEIGHT * OBJ_UNIT);	// 5 meters
	m_fCameraRotX = 0.f;
	m_fCameraRotY = 0.f;
	m_fCameraRotZ = 0.f;

	m_fKeyboardRotVelocity = MATH_PI / 4;
	m_fKeyboardMovVelocity = OBJ_UNIT * KEYBOARD_MOVE_VELOCITY; /** old value is 5. I set 20 is for debugging purposes*/

	// default mode
	m_lastCameraMode = m_currentCameraMode = CameraFollowFirstPerson;

	m_vEyeReferencePoint = Vector3(0, 0, 0);

	Radian r(Degree(FIELDOFVIEW));
	SetProjParams(r.valueRadians(), 1.0f, NEAR_PLANE, FAR_PLANE);

	m_bEnableStereoVision = false;
	m_fStereoEyeShiftDistance = 0.035f;
	m_fStereoConvergenceOffset = 0.f;
	m_nMouseDragDistance = 0;

	LoadDefaultEventBinding();
}

CAutoCamera::~CAutoCamera(void)
{
	SAFE_DELETE(m_event);
}

void CAutoCamera::LoadDefaultEventBinding()
{
	if (GetEventBinding() == NULL)
		return;
	// default mapping
	m_event->MapEvent(EM_MOUSE_WHEEL, EM_CAM_ZOOM);
	m_event->MapEvent(EM_MOUSE_LEFTDOWN, EM_CAM_LEFTDOWN);
	m_event->MapEvent(EM_MOUSE_LEFTUP, EM_CAM_LEFTUP);
	m_event->MapEvent(EM_MOUSE_RIGHTDOWN, EM_CAM_RIGHTDOWN);
	m_event->MapEvent(EM_MOUSE_RIGHTUP, EM_CAM_RIGHTUP);

	m_event->MapEvent(EM_KEY_W, EM_CAM_FORWARD);
	m_event->MapEvent(EM_KEY_UP, EM_CAM_FORWARD);
	m_event->MapEvent(EM_KEY_NUMPAD8, EM_CAM_FORWARD);

	m_event->MapEvent(EM_KEY_S, EM_CAM_BACKWARD);
	m_event->MapEvent(EM_KEY_DOWN, EM_CAM_BACKWARD);
	m_event->MapEvent(EM_KEY_NUMPAD2, EM_CAM_BACKWARD);

	m_event->MapEvent(EM_KEY_A, EM_CAM_LEFT);
	m_event->MapEvent(EM_KEY_LEFT, EM_CAM_LEFT);
	m_event->MapEvent(EM_KEY_NUMPAD4, EM_CAM_LEFT);

	m_event->MapEvent(EM_KEY_D, EM_CAM_RIGHT);
	m_event->MapEvent(EM_KEY_RIGHT, EM_CAM_RIGHT);
	m_event->MapEvent(EM_KEY_NUMPAD6, EM_CAM_RIGHT);

	m_event->MapEvent(EM_KEY_Q, EM_CAM_SHIFTLEFT);
	m_event->MapEvent(EM_KEY_HOME, EM_CAM_SHIFTLEFT);
	m_event->MapEvent(EM_KEY_NUMPAD7, EM_CAM_SHIFTLEFT);

	m_event->MapEvent(EM_KEY_E, EM_CAM_SHIFTRIGHT);
	m_event->MapEvent(EM_KEY_PAGE_UP, EM_CAM_SHIFTRIGHT);
	m_event->MapEvent(EM_KEY_NUMPAD9, EM_CAM_SHIFTRIGHT);

	// camera
	m_event->MapEvent(EM_KEY_END, EM_CAM_RESET);
	m_event->MapEvent(EM_KEY_C, EM_CAM_MODE_FREE);
	m_event->MapEvent(EM_KEY_F, EM_CAM_MODE_FOLLOW);
	m_event->MapEvent(EM_KEY_ADD, EM_CAM_FOCUS_POS_UP);
	m_event->MapEvent(EM_KEY_SUBTRACT, EM_CAM_FOCUS_POS_DOWN);
	m_event->MapEvent(EM_KEY_INSERT, EM_CAM_ZOOM_IN);
	m_event->MapEvent(EM_KEY_DELETE, EM_CAM_ZOOM_OUT);

	// player control
	m_event->MapEvent(EM_KEY_SPACE, EM_PL_JUMP);
	m_event->MapEvent(EM_KEY_CAPSLOCK, EM_PL_TOGGLE_ALWAYS_RUN);
	m_event->MapEvent(EM_KEY_R, EM_PL_TOGGLE_RUN_WALK);
	m_event->MapEvent(EM_KEY_LSHIFT, EM_PL_MOUNT_ON);
	m_event->MapEvent(EM_KEY_1, EM_PL_ACTION1);
	m_event->MapEvent(EM_KEY_2, EM_PL_ACTION2);
	m_event->MapEvent(EM_KEY_3, EM_PL_ACTION3);
	m_event->MapEvent(EM_KEY_4, EM_PL_ACTION4);
}

CEventBinding* CAutoCamera::GetEventBinding()
{
	if (m_event == 0)
	{
		m_event = new CEventBinding();
		m_event->InitEventMappingTable();
		m_event->EnableKeyboard();
		m_event->EnableMouse();
		m_event->DefaultMap_Mouse();
		m_event->DefaultMap_Text();
	}
	return m_event;
}

void CAutoCamera::SetDefaultAngles()
{
	m_fCameraLiftupAngle = MATH_PI / 7;
}


void CAutoCamera::SetCameraMode(CameraMode modeCamera)
{
	m_currentCameraMode = modeCamera;
	if (CGlobals::WillGenReport())
	{
		static const char reportname[] = "camera mode";

		switch (m_currentCameraMode)
		{
		case CameraFollowFirstPerson:
			CGlobals::GetReport()->SetString(reportname, "first person(key: W,A,S,D, hold right mouse)");
			break;
		case CameraFollowDefault:
			CGlobals::GetReport()->SetString(reportname, "Fixed(key: W,A,S,D)");
			break;
		case CameraFollowThirdPerson:
			CGlobals::GetReport()->SetString(reportname, "Third person(key: W,A,S,D, hold right mouse)");
			break;
		case CameraCameraFirstPerson:
			CGlobals::GetReport()->SetString(reportname, "free(key: W,A,S,D, hold right mouse)");
			break;
		default:
			CGlobals::GetReport()->SetString(reportname, "Undefined");
		}
	}
}

CameraMode CAutoCamera::GetCameraMode()
{
	return m_currentCameraMode;
}
//-----------------------------------------------------------------------------
// Name: FollowBiped
/// Desc: Get the mouse pay for object picking
/// [in] dTransitionTime: can be 0 or greater than 0
/// when dTransitionTime is zero, the camera is focuses on the biped from no where
/// if dTransitionTime is greater than 0, the camera will smoothly move to the biped.
//-----------------------------------------------------------------------------
void CAutoCamera::FollowBiped(CBaseObject* pBiped, CameraMode modeCamera, double dTransitionTime)
{
	if (pBiped != NULL)
	{
		if (m_pTargetObject)
		{
			CGlobals::GetScene()->SetCanShowMainPlayer(true);
			if (m_pTargetObject->IsBiped())
			{
				((CBipedObject*)m_pTargetObject)->SetStandingState();
				/*CBipedObject* pBiped = ((CBipedObject*)m_pTargetObject);
				if(!pBiped->IsStanding())
				{
					CBipedStateManager* pState =  pBiped->GetBipedStateManager();
					if(pState)
						pState->AddAction(CBipedStateManager::S_STANDING);
				}*/
			}
		}
		m_pTargetObject = pBiped;
	}

	if (dTransitionTime == 0)
	{
		DVector3 vEye(m_pTargetObject->GetPosition() + Vector3(1.0f, 0.55f, 1.0f));
		DVector3 vAt(m_pTargetObject->GetPosition());
		SetViewParams(vEye, vAt);
		SetScalers(0.01f, 1.0f);  // Camera movement parameters
	}
	else
	{
		// TODO: Thrust camera if destination too far away.
	}
}

/**
// pBiped can be nil, in which case the same biped is followed.
Transition
1 (default) smooth move
 0 intermediate move

Mode
<integer>
 CameraFollowFirstPerson = 2,	// First person view of the Target object
CameraFollowThirdPerson = 3,	// Third person view, allow rotation, character centered
CameraFollowDefault = 5,		// Third person view, disable rotation, character
CameraCameraFirstPerson = 11 	// Camera as a first person.


Parameters means difficult things under different mode
	//-- CameraFollowFirstPerson,	// First person view of the Target object
param0->m_fCameraObjectDistance;	// follow first person radius
param1->m_fCameraLiftupAngle;	// follow first person lift up angle, this is a constant usually Pi/4: 45 degrees
	//-- CameraFollowThirdPerson,	// Third person view, allow rotation, character centered
param0->m_fFTPRadius;	// follow Third person radius
param1->m_fFTPFacing;	// follow Third person: camera facing
param2->m_fFTPAngle;	// follow Third person lift up angle, this is a constant usually Pi/4: 45 degrees
	//-- CameraFollowDefault: Third person view, disable rotation, character restricted to a rectangular
CameraConstraint	m_constrantsFollow; // define the camera constraints for follow default mode
param0->m_fDEFAngle; // follow default lift up angle, this is a constant usually Pi/4: 45 degrees
param1->m_fDEFHeight;
	//-- CameraCameraFirstPerson 	// Camera as a first person.
CameraConstraint	m_constrants;
param0->m_vEye.x + biped.x
param1->m_vEye.y + biped.y
param2->m_vEye.z + biped.z

*/
void CAutoCamera::FollowBiped(CBaseObject* pBiped, int nTranstionMode, int modeCamera, double Param0, double Param1, double Param2)
{
	if ((modeCamera > CameraFollow && modeCamera < CameraCamera) ||
		(modeCamera == CameraCameraFirstPerson))
	{
	}
	else
		modeCamera = (int)m_currentCameraMode;

	/** follow the biped with a camera mode */
	if (nTranstionMode == 0)
		FollowBiped(pBiped, (CameraMode)modeCamera, 0); // immediate move camera
	else
		FollowBiped(pBiped, (CameraMode)modeCamera, 1); // smooth move.

	/** set camera mode */
	SetCameraMode((CameraMode)modeCamera);

	/** set params. */
	if (m_currentCameraMode == CameraFollowFirstPerson) {
		if (Param0 != 0)
			SetCameraObjectDistance((float)Param0);
		if (Param1 != 0)
			m_fCameraLiftupAngle = (float)Param1;
	}
	else if (m_currentCameraMode == CameraFollowThirdPerson) {
		if (Param0 != 0)
			SetCameraObjectDistance((float)Param0);
		if (Param1 != 0)
			m_fCameraLiftupAngle = (float)Param1;
		if (Param2 != 0)
			m_fCameraRotY = (float)Param2;
	}
	else if (m_currentCameraMode == CameraFollowDefault) {

	}
	else if (m_currentCameraMode == CameraCameraFirstPerson) {
		DVector3 pos(0, 0, 0);
		if (m_pTargetObject) {
			pos = m_pTargetObject->GetPosition();
		}
		if (Param0 != 0)
			m_vEye.x = pos.x + Param0;
		if (Param1 != 0)
			m_vEye.y = pos.y + Param1;
		if (Param2 != 0)
			m_vEye.z = pos.z + Param2;
		if (nTranstionMode == 0)
		{
			SetViewParams(m_vEye, pos);
		}
	}
}

bool CAutoCamera::SmoothMove(DVector3* result, DVector3 vPosTarget, DVector3 vPos, float fIncrease, float fTolerance/*=0*/)
{
	DVector3 vSub;
	double fDist;

	// get distance from target
	vSub = vPosTarget - vPos;
	fDist = vSub.squaredLength();
	if (fDist > FLT_TOLERANCE)
		fDist = sqrt(fDist);

	if (fIncrease >= fDist || fTolerance > fDist)
	{
		// we're within reach; set the exact point
		(*result) = vPosTarget;
		return true;
	}
	else
	{
		// moving forward
		vSub.normalise();
		vSub = vSub * fIncrease;
		*result = vPos + vSub;
		return false;
	}
}

int CAutoCamera::On_FrameMove(float fElapsedTime)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_FrameMove);
	if (pCallback)
	{
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		CGlobals::GetAISim()->NPLDoString(sCode.c_str(), sCode.size());
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FrameMove
/// Desc: Update the view matrix based on user input & elapsed time
//-----------------------------------------------------------------------------
VOID CAutoCamera::FrameMove(FLOAT fElapsedTime)
{
	if (!(CGlobals::GetApp()->IsAppActive()))
	{
		ClearMouseStates();
		ClearKeyStates();
	}

	On_FrameMove(fElapsedTime);

	//TODO: using VOID SetClipToBoundary( bool bClipToBoundary, Vector3* pvMinBoundary, Vector3* pvMaxBoundary ) 
	// { m_bClipToBoundary = bClipToBoundary; if( pvMinBoundary ) m_vMinBoundary = *pvMinBoundary; if( pvMaxBoundary ) 
	//m_vMaxBoundary = *pvMaxBoundary; }

	Vector3 vWorldUp(0, 1.f, 0);
	CBipedObject* pBiped = NULL;
	bool bShowCharacter = true; // if in first person view, it will hide the character.
	// true if the camera needs to be updated
	bool bDoUpdateView = true;
	// whether to ignore eye's near plane above global terrain check.
	bool bIgnoreGlobalTerrain = !IsEnableTerrainCollision();

	if (m_currentCameraMode == CameraCameraFirstPerson)
	{// camera's first person mode
		// update key board move velocity in this mode.
#define ADJUST_KEYBOARD_MOV_SPEED
#ifdef ADJUST_KEYBOARD_MOV_SPEED
		float fCameraSpeedUpFactor = 1.f;
		if (m_nMouseWheelDelta != 0)
		{
			fCameraSpeedUpFactor *= (1 + m_nMouseWheelDelta * 0.1f);
			m_nMouseWheelDelta = 0;
		}
		SetKeyboardMovVelocity(GetKeyboardMovVelocity() * fCameraSpeedUpFactor);
#endif

		// Get amount of velocity based on the keyboard input and drag (if any)
		UpdateVelocity(fElapsedTime);

		// Simple Euler method to calculate position delta
		Vector3 vPosDelta = m_vVelocity * fElapsedTime * m_fKeyboardMovVelocity;

		// If rotating the camera 
		if (GetAlwaysRotateCameraWhenFPS() || m_bMouseRButtonDown)
		{
			// Update the pitch & yaw angle based on mouse movement
			float fYawDelta = m_vRotVelocity.x;
			float fPitchDelta = m_vRotVelocity.y;

			// Invert pitch if requested
			if (m_bInvertPitch)
				fPitchDelta = -fPitchDelta;

			m_fCameraPitchAngle += fPitchDelta;
			m_fCameraYawAngle += fYawDelta;

			// Limit pitch to straight up or straight down
			m_fCameraPitchAngle = max(-MATH_PI / 2.0f, m_fCameraPitchAngle);
			m_fCameraPitchAngle = min(+MATH_PI / 2.0f, m_fCameraPitchAngle);
		}

		// Make a rotation matrix based on the camera's yaw & pitch
		Matrix4 mCameraRot;
		Math::CameraRotMatrixYawPitch(mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle);

		// Transform vectors based on camera's rotation matrix
		Vector3 vWorldAhead;
		Vector3 vLocalUp = Vector3(0, 1, 0);
		Vector3 vLocalAhead = Vector3(0, 0, 1);

		if (GetCharacterLookupBoneIndex() >= 0)
		{
			// modify the camera look up(vLocalUp) to be the same as the character lookup. 
			Matrix4 matWorld;
			pBiped = GetTargetAsBiped();
			if (pBiped && pBiped->GetAttachmentMatrix(matWorld, GetCharacterLookupBoneIndex(), CGlobals::GetSceneState()->GetRenderFrameCount() + 1) != NULL)
			{
				Vector3 v1, v2;

				// compute the camera lookup based on character lookup
				ParaVec3TransformCoord(&v1, &vLocalUp, &matWorld);
				v2.x = matWorld._41;
				v2.y = matWorld._42;
				v2.z = matWorld._43;
				v1 = v1 - v2;
				vLocalUp = v1.normalisedCopy();
			}
		}

		ParaVec3TransformCoord(&vWorldUp, &vLocalUp, &mCameraRot);
		ParaVec3TransformCoord(&vWorldAhead, &vLocalAhead, &mCameraRot);

		// Transform the position delta by the camera's rotation 
		Vector3 vPosDeltaWorld;
		ParaVec3TransformCoord(&vPosDeltaWorld, &vPosDelta, &mCameraRot);
		if (!m_bEnableYAxisMovement)
			vPosDeltaWorld.y = 0.0f;

		// - BUG: sometimes, first camera person view's vEye's Y axis can be invalid. 
		// Move the eye position 
		m_vEye += vPosDeltaWorld;
		// Update the lookAt position based on the eye position 
		m_vLookAt = m_vEye + vWorldAhead;
	}// camera's first person mode
	else if ((m_currentCameraMode > CameraFollow) && m_pTargetObject != NULL)
	{ // all are Camera Follow modes

		IViewClippingObject* pChar = m_pTargetObject->GetViewClippingObject();

		pBiped = GetTargetAsBiped();
		if (pBiped)
		{
			CBipedStateManager* pCharState = pBiped->GetBipedStateManager();
			if (pCharState && pCharState->IsMounted())
			{
				// this is very tricky code: we will animate the next frame. Because we need to get the mount position of the next render frame. 
				// since the camera position is used for the next render frame. 
				pBiped->Animate(fElapsedTime, CGlobals::GetSceneState()->GetRenderFrameCount() + 1);
			}
		}
		/// get character position
		DVector3 vCharPos = pChar->GetPosition();

		double fCharPosY = vCharPos.y;
		// m_fAllowedCharYShift = 1.2f; // for debugging only
		if (m_fAllowedCharYShift > 0 && m_currentCameraMode == CameraFollowThirdPerson)
		{
			double fDist = fabs(m_fLastUsedCharY - fCharPosY);
			if (fDist <= (m_fAllowedCharYShift + 0.001f))
			{
				vCharPos.y = m_fLastUsedCharY;

				if (m_fLastCharY == fCharPosY)
				{
					double fDeltaDist = fElapsedTime * (m_fMaxYShiftSpeed * (0.1f + fDist));
					if (fDist <= fDeltaDist)
					{
						vCharPos.y = fCharPosY;
					}
					else
					{
						// smooth move
						vCharPos.y = vCharPos.y + fDeltaDist * (vCharPos.y > fCharPosY ? -1.f : 1.f);
					}
				}
			}
			else
			{
				fDist = m_fAllowedCharYShift;
				if (vCharPos.y > m_fLastUsedCharY)
				{
					vCharPos.y -= m_fAllowedCharYShift;
				}
				else
				{
					vCharPos.y += m_fAllowedCharYShift;
				}
			}
		}
		m_fLastUsedCharY = vCharPos.y;
		m_fLastCharY = fCharPosY;

		/** get character radius and height */
		float fCharRadius, fCharHeight;
		if (m_pTargetObject->IsBiped())
		{
			fCharRadius = ((CBipedObject*)m_pTargetObject)->GetPhysicsRadius();

			// fixed 2009.4.24: using the art model height instead of character physics height.
			fCharHeight = ((CBipedObject*)m_pTargetObject)->GetAssetHeight();
			//fCharHeight = ((CBipedObject*)m_pTargetObject)->GetPhysicsHeight();
		}
		else
		{
			fCharRadius = 0;
			fCharHeight = 0;
		}
		/// set the camera speed according to the character's speed
		if (m_pTargetObject->IsBiped())
			m_fEyeSpeed = ((CBipedObject*)m_pTargetObject)->GetAbsoluteSpeed() + 1.0f;
		if (m_fEyeSpeed <= OBJ_UNIT * 4.0f)
			m_fEyeSpeed = OBJ_UNIT * 4.0f;
		float fEyeSpeed = m_fEyeSpeed;
		float fLookAtSpeed = m_fLookAtSpeed;


		DVector3 vEye;
		DVector3 vLookAt;
		vWorldUp = Vector3::UNIT_Y;

		double fMinRadius = Math::Max(m_fMinCameraObjectDistance, (double)m_fNearPlane);


		/************************************************************************/
		/*camera is always behind the character                                 */
		/************************************************************************/
		if (m_currentCameraMode == CameraFollowFirstPerson || m_currentCameraMode == CameraFollowThirdPerson)
		{
			/**
			* Adjust parameters from user input
			*/
			{
				// fMinRadius = m_fNearPlane  + fCharRadius/cosf(m_fCameraLiftupAngle);
				double fMaxRadius = m_fMaxCameraObjectDistance; // min(m_fMaximumHeight, CGlobals::GetScene()->GetMinPopUpDistance());
				if (fMaxRadius <= 0.5f)
				{
					m_bFirstPerson = true;
				}
				else {
					m_bFirstPerson = false;
				}
				if (m_bControlBiped && (m_pTargetObject->IsBiped()))
				{
					pBiped = (CBipedObject*)m_pTargetObject;

					bool cameraMoved = false;
					bool bAnyMovementButton = false;
					bool bAnyMovementButtonUp = false;
					bool bInvertLeftRightTurning = false;
					CBipedStateManager* pState = pBiped->GetBipedStateManager();

					if ((pBiped->GetSpeed() != 0) && !m_bAlwaysRun && !pBiped->GetAlwaysFlying())
					{
						// if(pBiped->GetLastSpeed()==0)
						pState->AddAction(CBipedStateManager::S_STANDING);
					}
					/**
					* Process key events.
					*/
					if (IsKeyDown(m_aKeys[MOVE_FORWARD]))
					{
						// pBiped->RemoveWayPoint();
						pBiped->RemoveWayPointByType(BipedWayPoint::COMMAND_POINT);
						pState->AddAction(CBipedStateManager::S_WALK_FORWORD);
						bAnyMovementButton = true;
						m_resetFlyNormal = false;
					}
					else if (WasKeyDown(m_aKeys[MOVE_FORWARD]))
					{
						if (!m_bAlwaysRun)
						{
							pState->RemoveState(CBipedStateManager::STATE_WALK_FORWORD);
							// stop the biped, when the key is up.
							bAnyMovementButtonUp = true;
							m_aKeys[MOVE_FORWARD] &= ~KEY_WAS_DOWN_MASK;
						}

						m_resetFlyNormal = true;
					}

					if (m_bTurnBipedWhenWalkBackward && !pBiped->GetIsFlying())
					{
						if (IsKeyDown(m_aKeys[MOVE_BACKWARD]))
						{
							pBiped->RemoveWayPointByType(BipedWayPoint::COMMAND_POINT);

							pState->AddAction(CBipedStateManager::S_WALK_FORWORD);
							bAnyMovementButton = true;
							m_resetFlyNormal = false;
							bInvertLeftRightTurning = true;
						}
						else if (WasKeyDown(m_aKeys[MOVE_BACKWARD]))
						{
							pState->RemoveState(CBipedStateManager::STATE_WALK_FORWORD);
							pState->RemoveState(CBipedStateManager::STATE_WALK_BACKWORD);
							// stop the biped, when the key is up.
							bAnyMovementButtonUp = true;
							m_aKeys[MOVE_BACKWARD] &= ~KEY_WAS_DOWN_MASK;
						}
					}
					else
					{
						if (IsKeyDown(m_aKeys[MOVE_BACKWARD]))
						{
							pState->AddAction(CBipedStateManager::S_WALK_BACKWORD);
							bAnyMovementButton = true;
						}
						else if (WasKeyDown(m_aKeys[MOVE_BACKWARD]))
						{
							pState->RemoveState(CBipedStateManager::STATE_WALK_BACKWORD);
							// stop the biped, when the key is up.
							bAnyMovementButtonUp = true;
							m_aKeys[MOVE_BACKWARD] &= ~KEY_WAS_DOWN_MASK;
						}
					}


					if (IsKeyDown(m_aKeys[FLY_DOWNWARD]) && pBiped->GetIsFlying())
					{
						bAnyMovementButton = true;
						pState->AddAction(CBipedStateManager::S_FLY_DOWNWARD);
					}
					else if (WasKeyDown(m_aKeys[FLY_DOWNWARD]))
					{
						m_aKeys[FLY_DOWNWARD] &= ~KEY_WAS_DOWN_MASK;
						bAnyMovementButton = true;
					}

					// 2009.9.4: left and right key will always rotate the camera. 
					const bool bRotateCameraWithKey = true;

					// this is tricky: if any movement button is down, we will enforce camera behind. 
					bool bIsCamAlwaysBehindObject = (((bRotateCameraWithKey || !m_bUseRightButtonBipedFacing) && bAnyMovementButton) || m_bIsCamAlwaysBehindObject)/* && !m_bMouseLButtonDown*/;

					if (bIsCamAlwaysBehindObject)
					{
						// use current camera rotation Y as the walking forward direction
						if (IsKeyDown(m_aKeys[MOVE_BACKWARD]) && m_bTurnBipedWhenWalkBackward)
						{
							pState->SetAngleDelta((float)Math::ToStandardAngle(m_fCameraRotY + 3.14f));
						}
						else
						{
							pState->SetAngleDelta((float)m_fCameraRotY);
						}
					}
					else
					{
						// use current character heading as the walking forward direction
						pState->SetAngleDelta(pBiped->GetFacing() + pBiped->GetHeadTurningAngle());
					}

					// left and right key always changes the camera
					if (IsKeyDown(m_aKeys[MOVE_RIGHT]))
					{
						if (!m_bUseRightButtonBipedFacing && (!bRotateCameraWithKey && pBiped->IsStanding()))
						{
							// if character is standing, the left and right button will rotate the character. 
							pBiped->SetHeadTurningAngle(0.f);
							pBiped->SetHeadUpdownAngle(0.f);
							pState->AddAction(CBipedStateManager::S_TURNING);
							pState->SetAngleDelta(CBipedObject::SPEED_TURN * fElapsedTime);
						}
						else
						{
							if (m_bMouseRButtonDown)
							{
								pState->AddAction(CBipedStateManager::S_WALK_RIGHT);
							}
							else if (!bRotateCameraWithKey && !bIsCamAlwaysBehindObject)
							{
								if (pBiped->IsStanding())
								{
									pState->AddAction(CBipedStateManager::S_TURNING);
									pState->SetAngleDelta(CBipedObject::SPEED_TURN * fElapsedTime);
								}
								else
								{
									pState->SetAngleDelta(pState->GetAngleDelta() + CBipedObject::SPEED_TURN * fElapsedTime);
								}
							}
							else
							{
								m_fCameraRotY += CBipedObject::SPEED_TURN * fElapsedTime;
								m_fCameraRotY = Math::ToStandardAngle(m_fCameraRotY);
								cameraMoved = true;
							}
						}
						bAnyMovementButton = true;
					}
					else if (WasKeyDown(m_aKeys[MOVE_RIGHT]))
					{
						// stop the biped, when the key is up.
						bAnyMovementButtonUp = true;
						m_aKeys[MOVE_RIGHT] &= ~KEY_WAS_DOWN_MASK;
					}

					if (IsKeyDown(m_aKeys[MOVE_LEFT]))
					{
						if (!m_bUseRightButtonBipedFacing && (!bRotateCameraWithKey && pBiped->IsStanding()))
						{
							// if character is standing, the left and right button will rotate the character. 
							pBiped->SetHeadTurningAngle(0.f);
							pBiped->SetHeadUpdownAngle(0.f);
							pState->AddAction(CBipedStateManager::S_TURNING);
							pState->SetAngleDelta(-CBipedObject::SPEED_TURN * fElapsedTime);
						}
						else
						{
							if (m_bMouseRButtonDown)
							{
								pState->AddAction(CBipedStateManager::S_WALK_LEFT);
							}
							else if (!bRotateCameraWithKey && !bIsCamAlwaysBehindObject)
							{
								if (pBiped->IsStanding())
								{
									pState->AddAction(CBipedStateManager::S_TURNING);
									pState->SetAngleDelta(-CBipedObject::SPEED_TURN * fElapsedTime);
								}
								else
								{
									pState->SetAngleDelta(pState->GetAngleDelta() - CBipedObject::SPEED_TURN * fElapsedTime);
								}
							}
							else
							{
								m_fCameraRotY -= CBipedObject::SPEED_TURN * fElapsedTime;
								m_fCameraRotY = Math::ToStandardAngle(m_fCameraRotY);

								cameraMoved = true;
							}
						}

						bAnyMovementButton = true;
					}
					else if (WasKeyDown(m_aKeys[MOVE_LEFT]))
					{
						// stop the biped, when the key is up.
						bAnyMovementButtonUp = true;
						m_aKeys[MOVE_LEFT] &= ~KEY_WAS_DOWN_MASK;
					}

					//////////////////////////////////////////////////////////////////////////
					if (IsKeyDown(m_aKeys[SHIFT_RIGHT]))
					{
						if (bInvertLeftRightTurning)
							pState->AddAction(CBipedStateManager::S_WALK_LEFT);
						else
							pState->AddAction(CBipedStateManager::S_WALK_RIGHT);
						bAnyMovementButton = true;
					}
					else if (WasKeyDown(m_aKeys[SHIFT_RIGHT]))
					{
						pState->RemoveState(CBipedStateManager::STATE_WALK_RIGHT);
						// stop the biped, when the key is up.
						bAnyMovementButtonUp = true;
						m_aKeys[SHIFT_RIGHT] &= ~KEY_WAS_DOWN_MASK;
					}

					if (IsKeyDown(m_aKeys[SHIFT_LEFT]))
					{
						if (bInvertLeftRightTurning)
							pState->AddAction(CBipedStateManager::S_WALK_RIGHT);
						else
							pState->AddAction(CBipedStateManager::S_WALK_LEFT);
						bAnyMovementButton = true;
					}
					else if (WasKeyDown(m_aKeys[SHIFT_LEFT]))
					{
						pState->RemoveState(CBipedStateManager::STATE_WALK_LEFT);
						// stop the biped, when the key is up.
						bAnyMovementButtonUp = true;
						m_aKeys[SHIFT_LEFT] &= ~KEY_WAS_DOWN_MASK;
					}

					/** stop the biped, when no movement key is down.*/
					if (!bAnyMovementButton && (pBiped->GetSpeed() != 0 || pBiped->GetLastSpeed() != 0) && !m_bAlwaysRun)
					{
						pState->AddAction(CBipedStateManager::S_STANDING);
						if (pBiped->GetAccelerationDist() > 0)
						{
							// tricky code here: force moving until last speed becomes zero. 
							pBiped->SetSpeed(0.f);

							if (pBiped->GetLastSpeed() != pBiped->GetSpeed())
							{
								pBiped->ForceMove();
							}
						}
					}

					double fCameraZoomDelta = 0.f;
					if (IsKeyDown(m_aKeys[ZOOM_IN]))
					{
						fCameraZoomDelta += -fElapsedTime * m_fCamZoomSpeed;
					}
					else if (IsKeyDown(m_aKeys[ZOOM_OUT]))
					{
						fCameraZoomDelta += fElapsedTime * m_fCamZoomSpeed;
					}

					//------------------------------------------
					// Process mouse events.
					//------------------------------------------

					// Get amount of velocity based on the keyboard input and drag (if any)
					UpdateVelocity(fElapsedTime);

					/**
					* process the mouse wheel delta: changing camera-object distance
					*/
					if (m_nMouseWheelDelta != 0)
					{
						fCameraZoomDelta += -m_nMouseWheelDelta * m_fCameraObjectDistance * 0.1f;
						m_nMouseWheelDelta = 0;
					}
					if (fCameraZoomDelta != 0.f)
					{
						m_fCameraObjectDistance += fCameraZoomDelta;
						m_fCameraObjectDistance = Math::Max(fMinRadius, m_fCameraObjectDistance);
						m_fCameraObjectDistance = Math::Min(fMaxRadius, m_fCameraObjectDistance);
						// 2012.10.10 disable first person, since there are mounted character. 
						// m_bFirstPerson = m_fCameraObjectDistance <= fMinRadius; 
					}

					static int g_nLastMouseButtonDown = 0;
					if (m_bMouseRButtonDown || m_bMouseLButtonDown || (IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
					{
						if (m_bMouseRButtonDown)
							g_nLastMouseButtonDown = MOUSE_RIGHT_BUTTON;
						if (m_bMouseLButtonDown)
							g_nLastMouseButtonDown = MOUSE_LEFT_BUTTON;

						if ((IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()) || (IsEnableMouseRightDrag() && m_bMouseRButtonDown) || (IsEnableMouseLeftDrag() && m_bMouseLButtonDown))
						{
							/**
							* process the mouse Y delta: changing the lift-up angle if the right mouse button is pressed.
							*/
							double fLiftUpAngle;
							fLiftUpAngle = m_vRotVelocity.y / 2;
							if (m_bInvertPitch)
								fLiftUpAngle = -fLiftUpAngle;
							fLiftUpAngle = m_fCameraLiftupAngle + fLiftUpAngle;
							/** angle constraint */
							fLiftUpAngle = Math::Max((double)MIN_CAM_LIFTUP_UPANGLE, fLiftUpAngle);
							fLiftUpAngle = Math::Min((double)MAX_CAM_LIFTUP_UPANGLE, fLiftUpAngle);
							m_fCameraLiftupAngle = fLiftUpAngle;

							/**
							* process the mouse X delta.
							*/
							/// if camera is locked, the mouse x delta will move the camera
							m_fCameraRotY += m_vRotVelocity.x;
							m_fCameraRotY = Math::ToStandardAngle(m_fCameraRotY);
							cameraMoved = true;
						}
					}

					if (cameraMoved)
						UpdateBipedFlyDir(pBiped);


					if (m_resetFlyNormal && pBiped->GetIsFlying())
					{
						Vector3 desireNormal(0, 1, 0);
						Vector3 delta = m_bipedFlyNormal - desireNormal;

						if (fabs(delta.x) < 0.001 && fabs(delta.y) < 0.001 && fabs(delta.z) < 0.001)
						{
							m_bipedFlyNormal = desireNormal;
							pBiped->SetNormal(m_bipedFlyNormal);
							m_resetFlyNormal = false;
						}
						else
						{
							Vector3 current;
							//just a approximate lerp, actual value calc should base on angle
							current = Math::Lerp(m_bipedFlyNormal, desireNormal, 0.1f);
							m_bipedFlyNormal = current;
							pBiped->SetNormal(m_bipedFlyNormal);
						}
					}
				}
			}
#ifdef ENABLE_WATER_CAMERA_CONSTRAINT

			/**
			* the camera water surface constraints: The camera's distance to the object as well as
			* the lift up angle is adjusted accordingly.
			*/
			if (pBiped)
			{
#define CAMERA_ON_WATER_LIFTUP_ANGLE 1.0f
#define CAMERA_ON_WATER_TRANSITION_TIME 2.5f
#define CHARACTER_WELLBELOW_WATER_SHIFT 3.0f
				/// @def this is the average water amplitude.  this is +-1.0f
#define WAVE_AMPLITUDE 2.0f

				Vector3 v1 = m_vEyeLast;
				Vector3 v2 = v1;
				v1.y += fCharHeight + WAVE_AMPLITUDE;
				v2.y -= fCharHeight + WAVE_AMPLITUDE;

				bool bCameraOnWave = ((!CGlobals::GetOceanManager()->IsPointUnderWater(&v1)) && ((CGlobals::GetOceanManager()->IsPointUnderWater(&v2))));
				//bool bCameraWellBelowWater;
				//bool bCameraWellAboveWater;

				v1 = vCharPos;
				v2 = v1;
				v1.y += fCharHeight + WAVE_AMPLITUDE;
				v2.y -= fCharHeight + WAVE_AMPLITUDE;
				bool bCharacterOnWave = ((!CGlobals::GetOceanManager()->IsPointUnderWater(&v1)) && ((CGlobals::GetOceanManager()->IsPointUnderWater(&v2))));
				v1 = vCharPos;
				v1.y += fCharHeight + WAVE_AMPLITUDE + CHARACTER_WELLBELOW_WATER_SHIFT;
				bool bCharacterWellBelowWater = CGlobals::GetOceanManager()->IsPointUnderWater(&v1);
				//bool bCharacterWellAboveWater;

				if (bCameraOnWave || bCharacterOnWave)
				{
					fEyeSpeed *= 1.5f; // increase the eye speed a little bit.
					if (m_fCameraLiftupAngle < CAMERA_ON_WATER_LIFTUP_ANGLE)
					{
						m_fCameraLiftupAngle = CAMERA_ON_WATER_LIFTUP_ANGLE;
						m_dTransitionAmt = CAMERA_ON_WATER_TRANSITION_TIME;
					}
					if (bCharacterWellBelowWater && bCameraOnWave)
					{
						// shorten the line of sight.
						float fDist = max(WAVE_AMPLITUDE / 2 + CHARACTER_WELLBELOW_WATER_SHIFT, fCharHeight);
						if (m_fCameraObjectDistance > fDist)
						{
							m_fCameraObjectDistance = fDist;
							m_dTransitionAmt = CAMERA_ON_WATER_TRANSITION_TIME;
						}
					}
					else // if( !bCharacterWellBelowWater && bCameraOnWave)
					{
						float fDist = WAVE_AMPLITUDE * 2 / sinf(CAMERA_ON_WATER_LIFTUP_ANGLE) + CHARACTER_WELLBELOW_WATER_SHIFT;
						// increase the line of sight
						if (m_fCameraObjectDistance < fDist)
						{
							m_fCameraObjectDistance = fDist;
							m_dTransitionAmt = CAMERA_ON_WATER_TRANSITION_TIME;
						}
					}
					//TODO: when character is deep in water, shorten the distance of m_fCameraObjectDistance .
				}
			}
#endif
			// get rotation Y

			double fCameraObjectDist = m_fCameraObjectDistance;
			// camera at the pre-stored height from the floor.
			double fHeadHeight = fCharHeight * 0.875f + m_fLookAtShiftY;

			double fPitch = m_fCameraRotX + m_fCameraLiftupAngle;

			// look at 7/8 the height of the character: at its neck
			vLookAt = DVector3(vCharPos.x, vCharPos.y + fHeadHeight, vCharPos.z);
			Quaternion qYaw(Vector3::UNIT_Y, (float)m_fCameraRotY);
			Quaternion qPitch(Vector3::UNIT_Z, (float)(-fPitch));
			Quaternion q = qYaw * qPitch;
			Matrix3 mRot;
			q.ToRotationMatrix(mRot);
			vEye = vLookAt - DVector3(Vector3::UNIT_X * mRot) * fCameraObjectDist;
			vWorldUp = vWorldUp * mRot;

			if (IsUseCharacterLookupWhenMounted())
			{
				if (pBiped)
				{
					CBipedStateManager* pCharState = pBiped->GetBipedStateManager();
					SetUseCharacterLookup(pCharState != 0 && pCharState->IsMounted());
				}
			}

			if (pBiped != 0)
			{
				bool bHasLookup = false;
				if (GetCharacterLookupBoneIndex() >= 0)
				{
					// modify the camera look up(vWorldUp) to be the same as the character lookup. 
					Matrix4 matWorld;
					if (pBiped->GetAttachmentMatrix(matWorld, GetCharacterLookupBoneIndex(), CGlobals::GetSceneState()->GetRenderFrameCount() + 1) != NULL)
					{
						Vector3 v1, v2;

						// compute the camera lookup based on character lookup
						ParaVec3TransformCoord(&v1, &vWorldUp, &matWorld);
						v2.x = matWorld._41;
						v2.y = matWorld._42;
						v2.z = matWorld._43;
						v1 = v1 - v2;
						vWorldUp = v1.normalisedCopy();

						// compute the camera eye based on character lookup
						v2 = vEye - vCharPos;
						ParaVec3TransformCoord(&v1, &v2, &matWorld);
						v2.x = matWorld._41;
						v2.y = matWorld._42;
						v2.z = matWorld._43;
						v1 = v1 - v2;
						vEye = vCharPos + v1;

						// look at 7/8 the height of the character: at its neck
						vLookAt = vCharPos + vWorldUp * (float)fHeadHeight;
						bHasLookup = true;
					}
				}
				if (!bHasLookup)
				{
					if (IsUseCharacterLookup())
					{
						// modify the camera look up(vWorldUp) to be the same as the character lookup. 
						Matrix4 matWorld;
						pBiped->GetRenderMatrix(matWorld, CGlobals::GetSceneState()->GetRenderFrameCount() + 1);
						Vector3 v1, v2;

						// compute the camera lookup based on character lookup
						ParaVec3TransformCoord(&v1, &vWorldUp, &matWorld);
						v2.x = matWorld._41;
						v2.y = matWorld._42;
						v2.z = matWorld._43;
						v1 = v1 - v2;
						vWorldUp = v1.normalisedCopy();

						// compute the camera eye based on character lookup
						v2 = vEye - vCharPos;
						ParaVec3TransformCoord(&v1, &v2, &matWorld);
						v2.x = matWorld._41;
						v2.y = matWorld._42;
						v2.z = matWorld._43;
						v1 = v1 - v2;
						vEye = vCharPos + v1;

						// look at 7/8 the height of the character: at its neck
						vLookAt = vCharPos + vWorldUp * (float)fHeadHeight;
					}
					else if (m_fCameraRotZ != 0.f)
					{
						// The order of transformations is roll first, then pitch, then yaw, where:
						// tricky: too many coordinate definitions I used, i just try and error about the signs here.
						Matrix4 matPitch, matYaw, matRoll;
						ParaMatrixRotationY(&matYaw, (float)(-1.57f + m_fCameraRotY));
						ParaMatrixRotationZ(&matRoll, (float)m_fCameraRotZ);
						ParaMatrixRotationX(&matPitch, (float)(-m_fCameraLiftupAngle + m_fCameraRotX));
						matRoll = (matRoll * matPitch) * matYaw;
						ParaVec3TransformCoord(&vWorldUp, &vWorldUp, &matRoll);
					}
				}
			}
		}
		else {
			// For other camera mode. Please refer to my old code, before 2005.10.11
		}

		if (m_bFirstPerson)
		{
			/// the vector from the new look at position to the new camera eye position.
			Vector3 vReverseLineOfSight = vEye - vLookAt;
			ParaVec3Normalize(&vReverseLineOfSight, &vReverseLineOfSight);

			vEye = vLookAt;
			vLookAt = vLookAt - vReverseLineOfSight;
			fEyeSpeed = m_fEyeSpeed * 5.f;
			fLookAtSpeed = m_fLookAtSpeed * 10.f;
			bShowCharacter = false;
		}
		/**
		* Implement the occlusion constraint as below:
		* Input:
		*	- vEye: new eye position
		*	- vLookAt: new look at position
		* We cast a ray from the new look at position to the new eye position.
		* If this ray hits something, let fLineOfSightLen be the distance from the
		* intersection point to the ray origin. If not, the new eye position and look at position is adopted.
		* Further reaction is subject to the value of fLineOfSightLen as below:
		*	- if fLineOfSightLen is larger than the line of sight with the new camera position, then the new
		*		position is adopted.
		*	- if fLineOfSightLen is smaller than the distance from the near camera plane to the camera eye; we can have two solutions:
		*		(1) the camera position does not change (both eye and look at location remain unchanged). (#define CAMERA_NOT_CHANGE)
		*		(2) use a first person camera view.
		*	- if fLineOfSightLen is smaller than the line of sight with the new camera position, but larger than
		*		the distance from the near camera plane to the camera eye, the new look at position is adopted, whereas
		*		the camera eye position is changed to the interaction point, and the camera eye movement speed is set to infinity
		*/
		else
		{
			/// the vector from the new look at position to the new camera eye position.
			Vector3 vReverseLineOfSight = vEye - vLookAt;
			float fDesiredLineOfSightLen = vReverseLineOfSight.length();
			float fMinLineOfSightLen = m_fNearPlane + fCharRadius;
			if (fDesiredLineOfSightLen < fMinLineOfSightLen)
				fDesiredLineOfSightLen = fMinLineOfSightLen;

			//ParaVec3Normalize(&vReverseLineOfSight, &vReverseLineOfSight);
			vReverseLineOfSight.normalise();

			Vector3 vHitPoint, vHitNormal(0.f, 0.f, 0.f);
			float fLineOfSightLen;
			fLineOfSightLen = CGlobals::GetScene()->PickClosest(vLookAt, vReverseLineOfSight, NULL, &vHitPoint, &vHitNormal, false, 0, GetPhysicsGroupMask(), !bIgnoreGlobalTerrain);

			if (((fLineOfSightLen - m_fNearPlane) >= fDesiredLineOfSightLen) || (fLineOfSightLen < 0))
			{
				// the new vEye, vLookAt is adopted.
			}
			else if ((fLineOfSightLen - m_fNearPlane) <= fMinLineOfSightLen)
			{
#ifdef CAMERA_NOT_CHANGE
				// restore to old value, nothing is changed. 
				//vEye = m_vEye;
				//vLookAt = m_vLookAt;
#else

				// hey, here is whole new logic, we will adopt the first person view, once the camera is very close to it.
				vEye = vLookAt;
				vLookAt = vLookAt - vReverseLineOfSight;
				fEyeSpeed = m_fEyeSpeed * 5.f;
				fLookAtSpeed = m_fLookAtSpeed * 10.f;
				if (m_bFirstPerson)
					bShowCharacter = false;
#endif
			}
			else
			{
				float fDistShift = m_fNearPlane;
				if (vHitNormal != Vector3(0.f, 0.f, 0.f))
				{
					// we have normal, let us shift more accurately based on normal. 
					Vector3 vLineOfSight = -vReverseLineOfSight;
					float fCosAngle = vLineOfSight.dotProduct(vHitNormal);
					if (fCosAngle >= 0.f)
					{
						if (fCosAngle <= 0.01f)
						{
							fDistShift = (fLineOfSightLen - m_fNearPlane * 0.5f);
						}
						else
						{
							fDistShift = m_fNearPlane / fCosAngle;
							if (fDistShift > (fLineOfSightLen - 0.01f))
							{
								fDistShift = (fLineOfSightLen - 0.01f);
							}
						}
					}
					// TODO: when the camera eye does not hit anything, there are still occassions where the near plane intersects with walls. 
					// to fix it, we can project another ray side ways at the eye position. 
				}

				// use the (hit point - NearPlane) as the new eye position.
				vEye = vLookAt + vReverseLineOfSight * (fLineOfSightLen - fDistShift);

				// Increase the camera transition speed.
				fEyeSpeed = m_fEyeSpeed * 5.f;
				fLookAtSpeed = m_fLookAtSpeed * 10.f;
			}

			/** correction with block terrain engine */
			double fMinCameraObjectDistance = 0.f;
			BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
			if (IsEnableBlockCollision() && pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
			{
				//Vector3 vReverseLineOfSight = vEye-vLookAt;
				//float fDesiredLineOfSightLen = D3DXVec3Length(&vReverseLineOfSight);
				//float fMinLineOfSightLen = m_fNearPlane;
				//if(fDesiredLineOfSightLen < fMinLineOfSightLen )
				//	fDesiredLineOfSightLen = fMinLineOfSightLen;

				//ParaVec3Normalize(&vReverseLineOfSight, &vReverseLineOfSight);

				PickResult result;
				pBlockWorldClient->SetCubeModePicking(true);
				if (pBlockWorldClient->Pick(vLookAt, vReverseLineOfSight, fDesiredLineOfSightLen, result, BlockTemplate::batt_solid | BlockTemplate::batt_blockcamera)
					&& result.Distance <= fDesiredLineOfSightLen)
				{
					const float camera_box_size = BlockConfig::g_blockSize * 0.15f;

					/*
					if(result.Distance > BlockConfig::g_blockSize && (result.Side != 4 && result.Side != 5) )
					{
						// tricky: this makes the camera away from the vertical block surface.
						if(result.Distance > BlockConfig::g_blockSize*1.25f)
							result.Distance -= BlockConfig::g_blockSize *0.25f;
						else
							result.Distance = BlockConfig::g_blockSize;
					}
					*/
					bool bIgnoreCollision = false;
					if (/*vLookAt.y < vEye.y && */!m_bFirstPerson && IsIgnoreEyeBlockCollisionInSunlight())
					{
						// if eye is above look at point and both eye and look at points are in almost full sunlight, we will ignore collision. 
						Uint16x3  block_eye;
						BlockCommon::ConvertToBlockIndex((float)vEye.x, (float)vEye.y, (float)vEye.z, block_eye.x, block_eye.y, block_eye.z);

						BlockTemplate* pEyeBlock = pBlockWorldClient->GetBlockTemplate(block_eye);
						if (!pEyeBlock || !(pEyeBlock->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_blockcamera)))
						{
							uint8 eye_sunlight = 0;

							Uint16x3  block_lookat;
							BlockCommon::ConvertToBlockIndex((float)vLookAt.x, (float)vLookAt.y, (float)vLookAt.z, block_lookat.x, block_lookat.y, block_lookat.z);
							uint8 lookat_sunlight = 0;

							if (pBlockWorldClient->GetBlockBrightness(block_eye, &eye_sunlight, 1, 1) && eye_sunlight >= (BlockConfig::g_sunLightValue - 2) &&
								pBlockWorldClient->GetBlockBrightness(block_lookat, &lookat_sunlight, 1, 1) && lookat_sunlight >= (BlockConfig::g_sunLightValue))
							{
								PickResult resultEye;
								if (pBlockWorldClient->Pick(vEye, -vReverseLineOfSight, fDesiredLineOfSightLen, resultEye, BlockTemplate::batt_solid | BlockTemplate::batt_blockcamera))
								{
									if (resultEye.Distance > camera_box_size)
									{
										// ignore collision
										bIgnoreCollision = true;
										fMinCameraObjectDistance = fDesiredLineOfSightLen - resultEye.Distance + camera_box_size;
									}
								}
							}
						}
					}

					if (!bIgnoreCollision)
					{
						vEye = vLookAt + vReverseLineOfSight * result.Distance;

						//if(result.Distance < BlockConfig::g_blockSize)
						if (result.Distance < 0.01f)
						{
							vEye = vLookAt;
							vLookAt = vLookAt - vReverseLineOfSight;
							fEyeSpeed = m_fEyeSpeed * 5.f;
							fLookAtSpeed = m_fLookAtSpeed * 10.f;
							bShowCharacter = false;
							m_bFirstPerson = true;
						}
						else if (result.Distance < camera_box_size)
						{
							fEyeSpeed = m_fEyeSpeed * 5.f;
							fLookAtSpeed = m_fLookAtSpeed * 10.f;
							bShowCharacter = false;
							m_bFirstPerson = true;
						}

						if (result.Distance < BlockConfig::g_blockSize * 3)
						{
							bShowCharacter = false;
						}
						if (pBiped != NULL)
						{
							Vector3 vPos = pBiped->GetPosition();
							float fTerrainHeight = CGlobals::GetGlobalTerrain()->GetElevation(vPos.x, vPos.z);
							if (fTerrainHeight > (vPos.y + 0.5) || CGlobals::GetGlobalTerrain()->IsHole(vPos.x, vPos.z))
							{
								bIgnoreGlobalTerrain = true;
							}
						}

						if (!m_bFirstPerson)
						{
							if (result.Side == 0)
							{
								vEye -= vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.x);
								//vEye.x -= quat_block_size;
							}
							else if (result.Side == 1)
							{
								vEye += vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.x);
								//vEye.x += quat_block_size;
							}
							else if (result.Side == 2)
							{
								vEye -= vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.z);
								//vEye.z -= quat_block_size;
							}
							else if (result.Side == 3)
							{
								vEye += vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.z);
								//vEye.z += quat_block_size;
							}
							else if (result.Side == 4)
							{
								vEye -= vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.y);
								//vEye.y -= quat_block_size;
							}
							else if (result.Side == 5)
							{
								vEye += vReverseLineOfSight * (camera_box_size / vReverseLineOfSight.y);
								//vEye.y += quat_block_size;
							}
						}
					}
				}
				pBlockWorldClient->SetCubeModePicking(false);
				if (!m_bFirstPerson)
				{
					// taking block world into consideration
					bool bUseMinMaxBox = false;
					const float block_size = BlockConfig::g_blockSize;
					const float half_block_size = block_size * 0.5f;
					// the camera_box_size thinness is 0.15
					const float camera_dist_size = block_size * (0.5f - 0.15f);

					uint16 block_x, block_y, block_z;
					BlockCommon::ConvertToBlockIndex((float)vEye.x, (float)vEye.y, (float)vEye.z, block_x, block_y, block_z);

					Vector3 vCenterPos = BlockCommon::ConvertToRealPosition(block_x, block_y, block_z);

					Vector3 vInnerAABBMinPos = vCenterPos + Vector3(-camera_dist_size, -camera_dist_size, -camera_dist_size);
					Vector3 vInnerAABBMaxPos = vCenterPos + Vector3(camera_dist_size, camera_dist_size, camera_dist_size);

					int block_x_tmp, block_y_tmp, block_z_tmp;
					bool obstruction_matrix[3][3][3];
					for (int i = 0; i < 3; i++)
					{
						for (int j = 0; j < 3; j++)
						{
							for (int k = 0; k < 3; k++)
							{
								block_x_tmp = block_x + i - 1;
								block_y_tmp = block_y + k - 1;
								block_z_tmp = block_z + j - 1;
								if (block_y_tmp < 0 || block_y_tmp>255)
									obstruction_matrix[i][k][j] = true;
								else
								{
									uint16_t templateId = pBlockWorldClient->GetBlockTemplateIdByIdx((uint16)block_x_tmp, (uint16)block_y_tmp, (uint16)block_z_tmp);
									if (templateId > 0)
									{
										BlockTemplate* temp = pBlockWorldClient->GetBlockTemplate(templateId);
										// camera only collide with solid or blockcamera blocks rather than obstruction block. 
										if (temp && (temp->GetAttFlag() & (BlockTemplate::batt_solid | BlockTemplate::batt_blockcamera)) != 0)
										{
											obstruction_matrix[i][k][j] = true;
											bUseMinMaxBox = true;
										}
										else
											obstruction_matrix[i][k][j] = false;
									}
									else
										obstruction_matrix[i][k][j] = false;
								}
							}
						}
					}

					if (bUseMinMaxBox)
					{
						Vector3 vPos = vEye;

						// directly connected 6 blocks
						if (obstruction_matrix[0][1][1] && vPos.x < vInnerAABBMinPos.x)
							vPos.x = vInnerAABBMinPos.x;
						if (obstruction_matrix[2][1][1] && vPos.x > vInnerAABBMaxPos.x)
							vPos.x = vInnerAABBMaxPos.x;
						if (obstruction_matrix[1][1][0] && vPos.z < vInnerAABBMinPos.z)
							vPos.z = vInnerAABBMinPos.z;
						if (obstruction_matrix[1][1][2] && vPos.z > vInnerAABBMaxPos.z)
							vPos.z = vInnerAABBMaxPos.z;
						if (obstruction_matrix[1][0][1] && vPos.y < vInnerAABBMinPos.y)
							vPos.y = vInnerAABBMinPos.y;
						if (obstruction_matrix[1][2][1] && vPos.y > vInnerAABBMaxPos.y)
							vPos.y = vInnerAABBMaxPos.y;
						// TODO 8 corners?

						vEye = vPos;
						Vector3 vNewLineOfSight = vPos - vLookAt;
						if (vNewLineOfSight.x * vReverseLineOfSight.x < 0 || vNewLineOfSight.y * vReverseLineOfSight.y < 0 || vNewLineOfSight.z * vReverseLineOfSight.z < 0)
						{
							vLookAt = vEye - vReverseLineOfSight * result.Distance;
						}
					}
				}
			}

			/** the following code implements smooth roll back of the camera.
			*/
			if (pBiped != NULL && !m_bFirstPerson)
			{
				double fCameraObjectDist = (vLookAt - vEye).length();
				double fLastCameraObjectDistance = Math::Max(m_fLastCameraObjectDistance, fMinCameraObjectDistance);
				// 1000.f is a large value, beyond which speed rollback is disabled.
				//0.1f is minimum allowable object jerk distance. object smaller than this size will not cause smooth rollback animation to apply.
				if (m_nForceNoRollbackFrames == 0 && m_fCameraRollbackSpeed < 1000.f &&
					(fCameraObjectDist > fLastCameraObjectDistance))
				{
					// used some acceleration based on differences
					double fDelta = (0.1f + m_fCameraRollbackSpeed + m_fCameraRollbackSpeed * (fCameraObjectDist - fLastCameraObjectDistance)) * fElapsedTime;
					if (fCameraObjectDist > (fLastCameraObjectDistance + fDelta))
					{
						double fDist = fLastCameraObjectDistance + fDelta;
						vEye = vLookAt + (vEye - vLookAt) / fCameraObjectDist * fDist;
						m_fLastCameraObjectDistance = fDist;
					}
					else
					{
						m_fLastCameraObjectDistance = fCameraObjectDist;
					}
				}
				else
					m_fLastCameraObjectDistance = fCameraObjectDist;
			}
		}
		/**
		* smooth translate the camera to the new target
		* move eye and look at point smoothly to the new value.
		*/
		if (m_dTransitionAmt > 0)
		{
			if (SmoothMove(&m_vEye, vEye, m_vEye, fEyeSpeed * fElapsedTime, 0.0f))
				// no need to smooth move again, if we have reached the target.
				m_dTransitionAmt = 0;
			else
				m_dTransitionAmt -= fElapsedTime;
		}
		else
			m_vEye = vEye;
		//SmoothMove(&m_vLookAt, vLookAt, m_vLookAt, fLookAtSpeed*fElapsedTime, 0.0f);
		m_vLookAt = vLookAt;
	}// all are Camera Follow modes
	else
	{// the camera is not changed

		bDoUpdateView = false;
	}

	/**
	* For all kinds of cameras, we ensure that the near plane rectangular and the eye position is well above the
	* global terrain surface. This is done by ensuring that all the five points(four for the near plane, one for
	* the camera eye) has height (or Y component) at least larger than the global terrain at their (x,z) location.
	* I have also used a simple but approximated method to avoid camera near frustum intersect with the physics mesh.
	* this is done by casting a ray from the center of the near plane downward to see if it intersects with the physics
	* mesh.
	* Input:
	*	- m_vEye: current eye position
	*	- m_vLookAt: current look at position
	*/
	if (bDoUpdateView)
	{
		/**
		* check for global terrain.
		*/
		// get the view matrix
		Matrix4 MatView;
		ParaMatrixLookAtLH(&MatView, m_vEye, m_vLookAt, DVector3(vWorldUp));
		Matrix4* pMatProj = GetProjMatrix();

		// get the four points on the near frustum plane
		Matrix4 mat;
		ParaMatrixMultiply(&mat, &MatView, pMatProj);
		mat = mat.inverse();

		Vector3 vecFrustum[6];
		vecFrustum[0] = Vector3(-1.0f, -1.0f, 0.0f); // xyz
		vecFrustum[1] = Vector3(1.0f, -1.0f, 0.0f); // Xyz
		vecFrustum[2] = Vector3(-1.0f, 1.0f, 0.0f); // xYz
		vecFrustum[3] = Vector3(1.0f, 1.0f, 0.0f); // XYz
		vecFrustum[5] = m_vEye;

		for (int i = 0; i < 4; i++)
			ParaVec3TransformCoord(&vecFrustum[i], &vecFrustum[i], &mat);

		// one additional point (at the near plane bottom)to add more accuracy
		vecFrustum[4] = (vecFrustum[0] + vecFrustum[1]) / 2;

		float fShiftHeight = -FLT_TOLERANCE;

		if (!bIgnoreGlobalTerrain)
		{
			// for all five point, test
			for (int i = 0; i < 6; i++)
			{
				float fMinHeight = CGlobals::GetGlobalTerrain()->GetElevation(vecFrustum[i].x, vecFrustum[i].z);
				if (fShiftHeight < (fMinHeight - vecFrustum[i].y))
					fShiftHeight = fMinHeight - vecFrustum[i].y;
			}
			/// ignore camera collision with the terrain object, if the camera is well below the terrain surface.(may be in a cave or something)
			/// 2 meters is just an arbitrary value. 
			if (fShiftHeight > 2.0f)
				fShiftHeight = -FLT_TOLERANCE;
		}

		/**
		* check for physical meshes.
		*/
		{
			Vector3 vHitPoint, vPt;
			// the distance to check is m_fNearPlane*2, which is far larger than the near plane height
			/// we will check three points around the near plane.
			vPt = (vecFrustum[0] + vecFrustum[3]) / 2;
			float fDist = CGlobals::GetScene()->PickClosest(vPt, Vector3(0, -1, 0), NULL, &vHitPoint, NULL, false, m_fNearPlane * 2, GetPhysicsGroupMask(), !bIgnoreGlobalTerrain);
			vPt = (vecFrustum[0] + vecFrustum[2]) / 2;
			float fDistTmp = CGlobals::GetScene()->PickClosest(vPt, Vector3(0, -1, 0), NULL, &vHitPoint, NULL, false, m_fNearPlane * 2, GetPhysicsGroupMask(), !bIgnoreGlobalTerrain);
			fDist = min(fDistTmp, fDist);
			vPt = (vecFrustum[1] + vecFrustum[3]) / 2;
			fDistTmp = CGlobals::GetScene()->PickClosest(vPt, Vector3(0, -1, 0), NULL, &vHitPoint, NULL, false, m_fNearPlane * 2, GetPhysicsGroupMask(), !bIgnoreGlobalTerrain);
			fDist = min(fDistTmp, fDist);

			if (fDist > 0)
			{
				float fNearPlaneHeight = (vecFrustum[2] - vecFrustum[0]).length();
				float fShift = fNearPlaneHeight / 2 - fDist + 0.1f;
				if (fShift > fShiftHeight)
				{
					fShiftHeight = fShift;
				}
			}
		}

		/**
		* shift the distance
		*/
		if (fShiftHeight > -FLT_TOLERANCE)
		{
			m_vEye.y += fShiftHeight;
			m_vLookAt.y += fShiftHeight;
		}


		/** should regenerate render origin whenever the camera moves.*/
		CGlobals::GetScene()->RegenerateRenderOrigin(m_vEye);

		/// the following is only here to repair floating point accuracy. Is there a better way in the future?
		/*Vector3 dTempDelta = m_vEye - m_vEyeLast;
		if(fabs(dTempDelta.x)<0.01f || fabs(dTempDelta.y)<0.01f || fabs(dTempDelta.z)<0.01f)
			m_vEye = m_vEyeLast;
		dTempDelta = m_vLookAt - m_vLookAtLast;
		if(fabs(dTempDelta.x)<0.01f || fabs(dTempDelta.y)<0.01f || fabs(dTempDelta.z)<0.01f)
			m_vLookAt = m_vLookAtLast;*/

			/** Update the view matrix */
		ComputeViewMatrix(&m_mView, &(m_vEye), &(m_vLookAt), &vWorldUp);
		m_mCameraWorld = m_mView.inverse();
	}
	else if (IsStereoVisionEnabled())
	{
		/** Update the view matrix */
		ComputeViewMatrix(&m_mView, &(m_vEye), &(m_vLookAt), &vWorldUp);
		m_mCameraWorld = m_mView.inverse();
	}

	if (pBiped)
	{
		//if(pBiped->IsVisible() != bShowCharacter)
		if (pBiped->CanAnimOpacity())
		{
			bool bUseAnimation = false;
			CParameterBlock* pParams = pBiped->GetEffectParamBlock(true);
			if (pParams)
			{
				CParameter* pParam = pParams->GetParameter("g_opacity");
				float fOpacity = 1.f;
				if (pParam != 0)
					fOpacity = (float)(*pParam);

				/** speed to go from opaque to fully transparent and vice versa*/
#define CHAR_OPACITY_TRANSITION_SPEED  2.f
				if (bShowCharacter)
				{
					fOpacity += fElapsedTime * CHAR_OPACITY_TRANSITION_SPEED;
					if (fOpacity > 1.f)
						fOpacity = 1.f;
					if (fOpacity > 0.9)
					{
						if (pParam != 0)
							pParams->SetParameter("g_opacity", 1.f);
					}
					else
					{
						pParams->SetParameter("g_opacity", fOpacity);
						bUseAnimation = true;
					}
				}
				else
				{
					fOpacity -= fElapsedTime * CHAR_OPACITY_TRANSITION_SPEED;
					if (fOpacity < 0.f)
						fOpacity = 0.f;
					if (fOpacity < 0.1)
					{
						pParams->SetParameter("g_opacity", 0.f);
					}
					else
					{
						pParams->SetParameter("g_opacity", fOpacity);
						bUseAnimation = true;
					}
				}
			}
			if (!bUseAnimation)
			{
				CGlobals::GetScene()->SetCanShowMainPlayer(bShowCharacter);
			}
			else
			{
				CGlobals::GetScene()->SetCanShowMainPlayer(true);
			}
		}
	}
	// save eye position
	m_bIsLastFrameChanged = false;
	if (m_vEyeLast != m_vEye)
	{
		m_vEyeLast = m_vEye;
		m_bIsLastFrameChanged = true;
	}
	if (m_vLookAtLast != m_vLookAt)
	{
		m_vLookAtLast = m_vLookAt;
		m_bIsLastFrameChanged = true;
	}
	if (m_vLookUpLast != vWorldUp)
	{
		m_vLookUpLast = vWorldUp;
		m_bIsLastFrameChanged = true;
	}
	if (m_nForceNoRollbackFrames > 0)
		--m_nForceNoRollbackFrames;
#ifdef _DEBUG
	/*Vector3 vOffset = CGlobals::GetScene()->GetRenderOrigin();
	OUTPUT_LOG("eye: %f %f %f   lookat: %f %f %f   render origin:%f %f %f dt:%f\n", m_vEye.x, m_vEye.y, m_vEye.z,
		m_vLookAt.x, m_vLookAt.y, m_vLookAt.z, vOffset.x, vOffset.y, vOffset.z, fElapsedTime);*/
#endif
}


void CAutoCamera::ComputeViewMatrix(Matrix4* pOut, const DVector3* pEye, const DVector3* pAt, const Vector3* pUp)
{
	DVector3 vEye = *pEye;
	DVector3 vAt = *pAt;
	Vector3 vUp = *pUp;
	Vector3 vOffset = CGlobals::GetScene()->GetRenderOrigin();

	if (m_vAdditionalCameraRotate != Vector3::ZERO)
	{
		Vector3 vDir = (Vector3)(vAt - vEye);
		Matrix4 matRot;
		ParaMatrixRotationRollPitchYaw(&matRot, m_vAdditionalCameraRotate.z, m_vAdditionalCameraRotate.y, m_vAdditionalCameraRotate.x);
		DVector3 vEye1(0, 0, 0);
		DVector3 vAt1 = vAt - vEye;
		Matrix4 matView1;
		Matrix3 matViewRot;
		ParaMatrixLookAtLH(&matView1, vEye1, vAt1, DVector3(vUp));
		matView1 = matView1 * matRot;
		matView1.invert();
		matView1.extract3x3Matrix(matViewRot);
		vAt = (Vector3(0, 0, 1.0f) * matViewRot) * vDir.length() + vEye;
		vUp = Vector3(0, 1.0f, 0) * matViewRot;
	}
	
	/// Update the view matrix
	if (!m_bEnableStereoVision)
	{
		m_vRenderEyePos = vEye;
		// standard way
		vEye = vEye - vOffset;
		vAt = vAt - vOffset;
		ParaMatrixLookAtLH(pOut, vEye, vAt, DVector3(vUp));
	}
	else
	{
		// stereo vision is enabled. Tricky code goes here:
		vEye = (vEye - vOffset);
		vAt = (vAt - vOffset);
		DVector3 vDir = vEye - vAt;
		DVector3 vEyeDirection = vDir.normalisedCopy();

		float fOffset = GetStereoConvergenceOffset();
		if (fOffset != 0.f)
		{
			vAt -= vEyeDirection * fOffset;
		}

		DVector3 vShiftDir(0, 0, 1.f);
		vShiftDir = vEyeDirection.crossProduct(DVector3(vUp)).normalisedCopy();
		vEye += vShiftDir * m_fStereoEyeShiftDistance;
		m_vRenderEyePos = vEye + vOffset;
		ParaMatrixLookAtLH(pOut, vEye, vAt, DVector3(vUp));
	}

	if (m_vLookAtOffset != Vector3::ZERO)
	{
		pOut->_41 += m_vLookAtOffset.x;
		pOut->_42 += m_vLookAtOffset.y;
		pOut->_43 += m_vLookAtOffset.z;
	}
}

Vector3 ParaEngine::CAutoCamera::GetRenderEyePosition()
{
	return (!m_bEnableStereoVision) ? GetEyePosition() : m_vRenderEyePos;
}

void CAutoCamera::UpdateViewMatrix()
{
	/// Update the view matrix
	ComputeViewMatrix(&m_mView, &m_vEye, &m_vLookAt, &m_vUp);
}

void CAutoCamera::SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt, const Vector3* up)
{
	/** set View */
	m_vDefaultEye = m_vEye = vEyePt;
	m_vDefaultLookAt = m_vLookAt = vLookatPt;

	// Calc the view matrix
	if (up != NULL) {
		m_vUp = *up;
	}
	/// Update the view matrix
	CGlobals::GetScene()->RegenerateRenderOrigin(m_vEye);
	ComputeViewMatrix(&m_mView, &m_vEye, &m_vLookAt, &m_vUp);

	Matrix4 mInvView;
	mInvView = m_mView.inverse();

	// The axis basis vectors and camera position are stored inside the 
	// position matrix in the 4 rows of the camera's world matrix.
	// To figure out the yaw/pitch of the camera, we just need the Z basis vector
	Vector3* pZBasis = (Vector3*)&mInvView._31;

	m_fCameraYawAngle = atan2f(pZBasis->x, pZBasis->z);
	float fLen = sqrtf(pZBasis->z * pZBasis->z + pZBasis->x * pZBasis->x);
	m_fCameraPitchAngle = -atan2f(pZBasis->y, fLen);

	/** save a snapshot*/
	m_vEyeLast = m_vEye;
	m_vLookAtLast = m_vLookAt;
	m_vLookUpLast = Vector3(0.0f, 1.0f, 0.0f);
}

void CAutoCamera::UpdateViewProjMatrix()
{
	ParaMatrixMultiply(&m_matViewProj, GetViewMatrix(), GetProjMatrix());
}

Matrix4* CAutoCamera::GetViewProjMatrix()
{
	return &m_matViewProj;
}

bool CAutoCamera::IsUseCharacterLookup()
{
	return  m_bUseCharacterLookup;
}

void CAutoCamera::SetUseCharacterLookup(bool bUseCharacterLookup)
{
	m_bUseCharacterLookup = bUseCharacterLookup;
}

bool CAutoCamera::IsUseCharacterLookupWhenMounted()
{
	return m_bUseCharacterLookupWhenMounted;
}

void CAutoCamera::SetUseCharacterLookupWhenMounted(bool bUseCharacterLookup)
{
	m_bUseCharacterLookupWhenMounted = bUseCharacterLookup;
}

LRESULT CAutoCamera::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

void CAutoCamera::SetKeyDownState(CharacterAndCameraKeys mappedKey, bool bIsKeyDown)
{
	if (bIsKeyDown)
	{
		if (FALSE == IsKeyDown(m_aKeys[mappedKey]))
			m_aKeys[mappedKey] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
	}
	else
	{
		m_aKeys[mappedKey] &= ~KEY_IS_DOWN_MASK;
	}
}

void CAutoCamera::HandleUserInput()
{
	if (IsBlockInput())
		return;

	//////////////////////////////////////////////////////////////////////////
	// process mouse input
	int dx = 0;
	int dy = 0;
	CGUIKeyboardVirtual* pKeyboard = (CGUIKeyboardVirtual*)(CGlobals::GetGUI()->m_pKeyboard);

	// Fixed: 2009.11.11. I used to use this via event, however, some key event may be lost, so I switched to hardware key query. 
	bool bIsKeyProcessed = CGlobals::GetGUI()->IsKeyboardProcessed();

	// we will prevent camera movement if either ctrl, alt, is pressed.
	bool bAlterKeyPressed = false;
	if (pKeyboard)
	{
		bAlterKeyPressed = pKeyboard->IsKeyPressed(EVirtualKey::KEY_LCONTROL) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_RCONTROL) ||
			// we still needs shift toggle run/walk movement
			// pKeyboard->IsKeyPressed(EVirtualKey::DIK_LSHIFT) || pKeyboard->IsKeyPressed(EVirtualKey::DIK_RSHIFT) ||
			pKeyboard->IsKeyPressed(EVirtualKey::KEY_LMENU) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_RMENU);
		bIsKeyProcessed = bIsKeyProcessed || bAlterKeyPressed;
	}

	SetKeyDownState(MOVE_FORWARD, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(MOVE_FORWARD)/*DIK_W*/) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD8) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_UP)));
	SetKeyDownState(MOVE_BACKWARD, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(MOVE_BACKWARD)/*DIK_S*/) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD2) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD5) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_DOWN)));

	if (IsShiftMoveSwitched())
	{
		SetKeyDownState(SHIFT_LEFT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(SHIFT_LEFT)) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD4)));
		SetKeyDownState(SHIFT_RIGHT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(SHIFT_RIGHT)) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD6)));
		SetKeyDownState(MOVE_LEFT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD7) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_LEFT)));
		SetKeyDownState(MOVE_RIGHT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD9) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_RIGHT)));
	}
	else
	{
		SetKeyDownState(SHIFT_LEFT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD4) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_LEFT)));
		SetKeyDownState(SHIFT_RIGHT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD6) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_RIGHT)));
		SetKeyDownState(MOVE_LEFT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(MOVE_LEFT)/*DIK_A*/) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD7)));
		SetKeyDownState(MOVE_RIGHT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(MOVE_RIGHT)/*DIK_D*/) || pKeyboard->IsKeyPressed(EVirtualKey::KEY_NUMPAD9)));
	}
	SetKeyDownState(ZOOM_IN, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(ZOOM_IN)/*DIK_INSERT*/)));
	SetKeyDownState(ZOOM_OUT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(ZOOM_OUT)/*DIK_DELETE*/)));
	SetKeyDownState(ZOOM_OUT, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(ZOOM_OUT)/*DIK_DELETE*/)));
	SetKeyDownState(FLY_DOWNWARD, !bIsKeyProcessed && (pKeyboard->IsKeyPressed(GetKeyMap(FLY_DOWNWARD))));


	CGUIMouseVirtual* pMouse = (CGUIMouseVirtual*)(CGlobals::GetGUI()->m_pMouse);
	if (pMouse && CGlobals::GetGUI()->m_events.size() > 0)
	{
		GUIMsgEventList_type::const_iterator iter = CGlobals::GetGUI()->m_events.begin(), iterend = CGlobals::GetGUI()->m_events.end();
		const MSG* pMsg;
		for (; iter != iterend; ++iter)
		{
			pMsg = &(*iter);
			//////////////////////////////////////////////////////////////////////////
			//
			// handle mouse event
			//
			//////////////////////////////////////////////////////////////////////////
			if (m_event->IsMapTo(pMsg->message, EM_MOUSE))
			{
				if (!CGlobals::GetScene()->IsPickingObject()) {
					//if (m_event->IsMapTo(pMsg->message,EM_MOUSE_MOVE))
					//{
					//	if (m_bMouseLButtonDown || m_bMouseRButtonDown)
					//	{
					//		// lock the mouse position and hide the cursor
					//		dx+=(int)pMsg->lParam;
					//		dy+=(int)pMsg->wParam;
					//	}
					//}
					//else 
					if (m_event->IsMapTo(pMsg->message, EM_CAM_LEFTDOWN) && GetEnableMouseLeftButton())
					{
						//do something
						m_bMouseLButtonDown = true;
						m_nMouseDragDistance = 0;
					}
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_LEFTUP) && GetEnableMouseLeftButton())
					{
						m_bMouseLButtonDown = false;

						if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
						{
							// unlock the mouse position and show the cursor
							SetMouseDragLock(false);
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_RIGHTDOWN) && GetEnableMouseRightButton())
					{
						//do something
						m_bMouseRButtonDown = true;
						m_nMouseDragDistance = 0;
					}
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_RIGHTUP) && GetEnableMouseRightButton())
					{
						m_bMouseRButtonDown = false;

						if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
						{
							// unlock the mouse position and show the cursor
							SetMouseDragLock(false);
						}
					}
				}
				if (m_event->IsMapTo(pMsg->message, EM_CAM_ZOOM))
				{
					if (GetEnableMouseWheel())
					{
						int nDelta = ((int32)(pMsg->lParam)) / 120;
						if (nDelta == 0)
							nDelta = ((int32)(pMsg->lParam)) > 0 ? 1 : -1;
						m_nMouseWheelDelta = nDelta;
						m_nForceNoRollbackFrames = 1;
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//
			// handle key events
			//
			//////////////////////////////////////////////////////////////////////////
			else if (GetEnableKeyboard() && !bAlterKeyPressed && m_event->IsMapTo(pMsg->message, EM_KEY))
			{
				bool bIsKeyDown = IS_KEYDOWN(pMsg->lParam);
				//////////////////////////////////////////////////////////////////////////
				// state based key events: allowing holding state
				/*
				if (m_event->IsMapTo(pMsg->message,EM_CAM_FORWARD))
					SetKeyDownState(MOVE_FORWARD, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_BACKWARD))
					SetKeyDownState(MOVE_BACKWARD, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_LEFT))
					SetKeyDownState(MOVE_LEFT, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_RIGHT))
					SetKeyDownState(MOVE_RIGHT, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_SHIFTLEFT))
					SetKeyDownState(SHIFT_LEFT, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_SHIFTRIGHT))
					SetKeyDownState(SHIFT_RIGHT, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_ZOOM_IN))
					SetKeyDownState(ZOOM_IN, bIsKeyDown);
				else if (m_event->IsMapTo(pMsg->message,EM_CAM_ZOOM_OUT))
					SetKeyDownState(ZOOM_OUT, bIsKeyDown);
				*/
				//////////////////////////////////////////////////////////////////////////
				// camera control's ordinary key events
				if (bIsKeyDown)
				{
					if (m_event->IsMapTo(pMsg->message, EM_CAM_RESET))
					{ // reset camera
						Reset();
					}
#ifdef ENABLE_CAMERA_MODE_KEY
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_MODE_FOLLOW))
					{ // follow mode
						FollowMode();
					}
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_MODE_FREE))
					{ // Free camera mode
						FreeCameraMode();
					}

					else if (m_event->IsMapTo(pMsg->message, EM_CAM_FOCUS_POS_UP))
					{ // follow camera height shift Y
						m_fLookAtShiftY += 0.03f;
					}
					else if (m_event->IsMapTo(pMsg->message, EM_CAM_FOCUS_POS_DOWN))
					{ // follow camera height shift Y
						m_fLookAtShiftY -= 0.03f;
					}
#endif
#ifdef ENABLE_PLAYER_CONTROL_KEY
					//////////////////////////////////////////////////////////////////////////
					// player control
					else if (m_event->IsMapTo(pMsg->message, EM_PL_JUMP))
					{
						// jump upward the current character with an impulse
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							CBipedStateManager* pCharState = ((CBipedObject*)m_pTargetObject)->GetBipedStateManager();
							if (pCharState)
								pCharState->AddAction(CBipedStateManager::S_JUMP_START);
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_TOGGLE_ALWAYS_RUN))
					{ // toggle always running.
						m_bAlwaysRun = !m_bAlwaysRun;
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_TOGGLE_RUN_WALK))
					{ // Toggle always run or walk.
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							CBipedStateManager* pCharState = ((CBipedObject*)m_pTargetObject)->GetBipedStateManager();
							if (pCharState)
							{
								pCharState->AddAction(CBipedStateManager::S_ACTIONKEY,
									&ActionKey(pCharState->WalkingOrRunning() ? ActionKey::TOGGLE_TO_RUN : ActionKey::TOGGLE_TO_WALK));
							}
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_MOUNT_ON))
					{ // mount on closest character.
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							//m_pTargetObject->MountOn(NULL);
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_ACTION1))
					{ // attack 1
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							CBipedStateManager* pCharState = ((CBipedObject*)m_pTargetObject)->GetBipedStateManager();
							if (pCharState)
								pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &ActionKey("a1"));
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_ACTION2))
					{
						// attack 2
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							CBipedStateManager* pCharState = ((CBipedObject*)m_pTargetObject)->GetBipedStateManager();
							if (pCharState)
								pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &ActionKey("a2"));
						}
					}
					else if (m_event->IsMapTo(pMsg->message, EM_PL_ACTION3))
					{
						// dance
						if (m_pTargetObject && m_pTargetObject->IsBiped())
						{
							CBipedStateManager* pCharState = ((CBipedObject*)m_pTargetObject)->GetBipedStateManager();
							if (pCharState)
								pCharState->AddAction(CBipedStateManager::S_ACTIONKEY, &ActionKey("e_d"));
						}
					}
#endif

				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// some post process
	dx = pMouse->GetMouseXDeltaSteps();
	dy = pMouse->GetMouseYDeltaSteps();
	if ((m_bMouseRButtonDown && IsEnableMouseRightDrag()) || (m_bMouseLButtonDown && IsEnableMouseLeftDrag()))
	{
		// we will read immediate mouse state, instead of using windows message, which is inaccurate at low frame rate. 
		int dragDelta = (abs(dx) + abs(dy));
		m_nMouseDragDistance += dragDelta;
		if (m_nMouseDragDistance > 1)
		{
			if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
			{
				SetMouseDragLock(true);
			}

			// if user is dragging the right mouse button. uncomment following, if u do not wants it
			if (m_bUseRightButtonBipedFacing && m_bMouseRButtonDown && (m_currentCameraMode == CameraFollowFirstPerson || m_currentCameraMode == CameraFollowThirdPerson))
			{
				if (m_pTargetObject && m_pTargetObject->IsBiped())
				{
					CBipedObject* pBiped = ((CBipedObject*)m_pTargetObject);
					if (pBiped->IsStanding() && !pBiped->GetIsFlying())
					{
						// use current camera rotation Y as the walking forward direction
						pBiped->SetHeadTurningAngle(0.f);
						pBiped->SetHeadUpdownAngle(0.f);
						pBiped->SetFacing((float)m_fCameraRotY);
					}
					else if (pBiped->GetIsFlying())
					{
						Matrix4 rotMat;
						ParaMatrixRotationRollPitchYaw(&rotMat, (float)m_fCameraRotZ, (float)(-m_fCameraLiftupAngle), (float)(-1.57f + m_fCameraRotY));
						Vector3 normal;
						normal = Vector3(0, 1, 0) * rotMat;
						pBiped->SetNormal(normal);
						m_bipedFlyNormal = normal;

						pBiped->SetFacing((float)m_fCameraRotY);

						normal = Vector3(0, 0, -1) * rotMat;
						pBiped->SetFlyingDirection(&normal);
					}
				}
			}
		}
	}
	UpdateMouseDelta(dx, dy);

	// double check the device if the mouse button is down, in case we lost the mouse focus.
	if (m_bMouseLButtonDown && !(pMouse->IsButtonDown(EMouseButton::LEFT)))
	{
		m_bMouseLButtonDown = false;
		if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
		{
			// unlock the mouse position and show the cursor
			SetMouseDragLock(false);
		}
	}
	if (m_bMouseRButtonDown && !(pMouse->IsButtonDown(EMouseButton::RIGHT)))
	{
		m_bMouseRButtonDown = false;
		if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
		{
			// unlock the mouse position and show the cursor
			SetMouseDragLock(false);
		}
	}

#ifndef PARAENGINE_MOBILE
	// added a movement mode, if both left and right button are down, we will assume the move forward
	if (m_bMouseLButtonDown && m_bMouseRButtonDown)
	{
		SetKeyDownState(MOVE_FORWARD, true);
		if (m_pTargetObject && m_pTargetObject->IsBiped())
		{
			CBipedObject* pBiped = ((CBipedObject*)m_pTargetObject);
			pBiped->SetFacing((float)m_fCameraRotY);
		}
	}
#endif // PARAENGINE_MOBILE
}

void CAutoCamera::SetCameraObjectDistance(double fDistance)
{
	if (m_fForceOmniCameraObjectDistance > -9999) {
		return;
	}
	m_fCameraObjectDistance = m_fLastCameraObjectDistance = fDistance;
}

void CAutoCamera::SetCameraLiftupAngle(double fValue) {
	if (m_fForceOmniCameraPitch > -9999) {
		return;
	}
	m_fCameraLiftupAngle = fValue;
}

void CAutoCamera::SetCameraRotX(double fValue) {
	if (m_fForceOmniCameraPitch > -9999) {
		return;
	}
	m_fCameraRotX = fValue;
}

void CAutoCamera::SetForceOmniCameraObjectDistance(double fDist)
{
	m_fForceOmniCameraObjectDistance = fDist;
	if (m_fForceOmniCameraObjectDistance > -9999) {
		m_fCameraObjectDistance = m_fCameraObjectDistance = fDist;
	}
}
void CAutoCamera::SetForceOmniCameraPitch(double fPitch)
{
	m_fForceOmniCameraPitch = fPitch;
	if (m_fForceOmniCameraPitch > -9999) {
		m_fCameraLiftupAngle = m_fCameraRotX = fPitch;
	}
}

void CAutoCamera::Reset()
{
	m_dTransitionAmt = 0;
	m_currentCameraMode = CameraFollowFirstPerson;
}

void CAutoCamera::FreeCameraMode()
{
	m_bIsFollowMode = false;
	SetCameraMode(CameraCameraFirstPerson);
	m_lastCameraMode = m_currentCameraMode;
	/// reset view parameters for camera, so to begin free camera navigation from where the camera is positioned.
	SetViewParams(m_vEye, m_vLookAt);		// reset eye position
}

void CAutoCamera::FollowMode()
{
	CameraMode currentCameraMode = CameraFollowFirstPerson;
	if (m_lastCameraMode == CameraFollowFirstPerson)
		m_bIsCamAlwaysBehindObject = !m_bIsCamAlwaysBehindObject;
	else
		m_dTransitionAmt = MAX_TRANSITION_TIME; // give it 4 seconds to catch up the camera

	m_lastCameraMode = currentCameraMode;
	SetCameraMode(currentCameraMode);
}


bool ParaEngine::CAutoCamera::IsLockMouseWhenDragging() const
{
	return m_bLockMouseWhenDragging;
}

void ParaEngine::CAutoCamera::SetLockMouseWhenDragging(bool val)
{
	m_bLockMouseWhenDragging = val;
}

float ParaEngine::CAutoCamera::GetKeyboardMovVelocity()
{
	return m_fKeyboardMovVelocity;
}

void ParaEngine::CAutoCamera::SetKeyboardMovVelocity(float fValue)
{
	m_fKeyboardMovVelocity = fValue;
}

float ParaEngine::CAutoCamera::GetKeyboardRotVelocity()
{
	return m_fKeyboardRotVelocity;
}

void ParaEngine::CAutoCamera::SetKeyboardRotVelocity(float fValue)
{
	m_fKeyboardRotVelocity = fValue;
}

void ParaEngine::CAutoCamera::EnableMouseLeftButton(bool bValue)
{
	m_bEnableMouseLeftButton = bValue;
	if (!bValue)
	{
		CGUIMouseVirtual* pMouse = (CGUIMouseVirtual*)(CGlobals::GetGUI()->m_pMouse);
		m_bMouseRButtonDown = false;
		if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
		{
			// unlock the mouse position and show the cursor
			SetMouseDragLock(false);
		}
	}
}

void ParaEngine::CAutoCamera::EnableMouseRightButton(bool bValue)
{
	m_bEnableMouseRightButton = bValue;
	if (!bValue)
	{
		CGUIMouseVirtual* pMouse = (CGUIMouseVirtual*)(CGlobals::GetGUI()->m_pMouse);
		m_bMouseLButtonDown = false;
		if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
		{
			// unlock the mouse position and show the cursor
			SetMouseDragLock(false);
		}
	}
}

void ParaEngine::CAutoCamera::EnableKeyboard(bool bValue)
{
	if (m_bEnableKeyboard != bValue)
	{
		m_bEnableKeyboard = bValue;
		if (!m_bEnableKeyboard)
		{
			// clear all state. 
			ClearKeyStates();
		}
	}
}

void ParaEngine::CAutoCamera::SetBlockInput(bool bBlockInput)
{
	if (m_bBlockInput != bBlockInput)
	{
		m_bBlockInput = bBlockInput;

		if (m_bBlockInput)
		{
			// // clear any pressed state.
			ClearMouseStates();
			ClearKeyStates();
		}
	}
}

bool ParaEngine::CAutoCamera::IsBlockInput()
{
	return m_bBlockInput;
}

void ParaEngine::CAutoCamera::ClearMouseStates()
{
	// clear any pressed state. 
	m_bMouseLButtonDown = false;
	m_bMouseRButtonDown = false;
	m_nMouseDragDistance = 0;
	if (!(IsFirstPersonView() && GetAlwaysRotateCameraWhenFPS()))
	{
		SetMouseDragLock(false);
	}
}

void ParaEngine::CAutoCamera::SetMouseDragLock(bool bLock)
{
	if (IsLockMouseWhenDragging())
	{
		CGUIMouseVirtual* pMouse = (CGUIMouseVirtual*)(CGlobals::GetGUI()->m_pMouse);
		if (pMouse) {
			pMouse->SetLock(bLock);
			//pMouse->ShowCursor(!bLock);
		}
	}
}

void ParaEngine::CAutoCamera::ClearKeyStates()
{
	SetKeyDownState(MOVE_FORWARD, false);
	SetKeyDownState(MOVE_BACKWARD, false);
	SetKeyDownState(MOVE_LEFT, false);
	SetKeyDownState(MOVE_RIGHT, false);
	SetKeyDownState(SHIFT_LEFT, false);
	SetKeyDownState(SHIFT_RIGHT, false);
	SetKeyDownState(ZOOM_IN, false);
	SetKeyDownState(ZOOM_OUT, false);
	SetKeyDownState(FLY_DOWNWARD, false);
}

void ParaEngine::CAutoCamera::EnableAlwaysRotateCameraWhenFPS(bool bValue)
{
	m_bAlwaysRotateCameraWhenFPS = bValue;
}

bool ParaEngine::CAutoCamera::GetAlwaysRotateCameraWhenFPS()
{
	return m_bAlwaysRotateCameraWhenFPS;
}

int ParaEngine::CAutoCamera::GetCharacterLookupBoneIndex()
{
	return m_nCharacterLookupBoneIndex;
}

void ParaEngine::CAutoCamera::SetCharacterLookupBoneIndex(int nIndex)
{
	m_nCharacterLookupBoneIndex = nIndex;
}

float ParaEngine::CAutoCamera::GetMaxAllowedYShift()
{
	return m_fAllowedCharYShift;
}

void ParaEngine::CAutoCamera::SetMaxAllowedYShift(float fValue)
{
	m_fAllowedCharYShift = fValue;
}

float ParaEngine::CAutoCamera::GetMaxYShiftSpeed()
{
	return m_fMaxYShiftSpeed;
}

void ParaEngine::CAutoCamera::SetMaxYShiftSpeed(float fValue)
{
	m_fMaxYShiftSpeed = fValue;
}

bool ParaEngine::CAutoCamera::IsCameraMoved()
{
	return CGlobals::GetSceneState()->m_bCameraMoved;
}

bool ParaEngine::CAutoCamera::IsFirstPersonView()
{
	return GetMaxCameraObjectDistance() < 1 || m_currentCameraMode == CameraFollowFirstPerson;
}

bool ParaEngine::CAutoCamera::IsControlBiped() const
{
	return m_bControlBiped;
}

void ParaEngine::CAutoCamera::SetControlBiped(bool val)
{
	m_bControlBiped = val;
}

bool ParaEngine::CAutoCamera::IsEnableBlockCollision() const
{
	return m_bEnableBlockCollision;
}

void ParaEngine::CAutoCamera::SetEnableBlockCollision(bool val)
{
	m_bEnableBlockCollision = val;
}

bool ParaEngine::CAutoCamera::IsEnableTerrainCollision() const
{
	return m_bEnableTerrainCollision;
}

void ParaEngine::CAutoCamera::SetEnableTerrainCollision(bool val)
{
	m_bEnableTerrainCollision = val;
}

void ParaEngine::CAutoCamera::UpdateBipedFlyDir(CBipedObject* pBiped)
{
	if (!pBiped)
		pBiped = GetTargetAsBiped();

	if (pBiped && pBiped->GetIsFlying() && pBiped->GetFlyUsingCameraDir())
	{
		Matrix4 rotMat, matPitch, matYaw, matRoll;
		ParaMatrixRotationY(&matYaw, (float)(-1.57f + m_fCameraRotY));
		ParaMatrixRotationZ(&matRoll, (float)(m_fCameraRotZ));
		ParaMatrixRotationX(&matPitch, (float)(-m_fCameraLiftupAngle));
		rotMat = (matRoll * matPitch) * matYaw;
		Vector3 normal;
		normal = Vector3(0, 1, 0) * rotMat;
		pBiped->SetNormal(normal);
		m_bipedFlyNormal = normal;

		pBiped->SetFacing((float)m_fCameraRotY);

		normal = Vector3(0, 0, -1) * rotMat;
		pBiped->SetFlyingDirection(&normal);
	}
}

CBipedObject* ParaEngine::CAutoCamera::GetTargetAsBiped()
{
	if (m_pTargetObject && m_pTargetObject->IsBiped())
		return (CBipedObject*)m_pTargetObject;
	else
		return NULL;
}

ParaEngine::Vector3 ParaEngine::CAutoCamera::GetRenderOrigin()
{
	return CGlobals::GetScene()->GetRenderOrigin();
}

int CAutoCamera::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CFirstPersonCamera::InstallFields(pClass, bOverride);

	pClass->AddField("KeyboardMovVelocity", FieldType_Float, (void*)SetKeyboardMovVelocity_s, (void*)GetKeyboardMovVelocity_s, NULL, NULL, bOverride);
	pClass->AddField("KeyboardRotVelocity", FieldType_Float, (void*)SetKeyboardRotVelocity_s, (void*)GetKeyboardRotVelocity_s, NULL, NULL, bOverride);

	pClass->AddField("AlwaysRun", FieldType_Bool, (void*)SetAlwaysRun_s, (void*)IsAlwaysRun_s, NULL, NULL, bOverride);
	pClass->AddField("Reset", FieldType_void, (void*)Reset_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("FreeCameraMode", FieldType_void, (void*)FreeCameraMode_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("FollowMode", FieldType_void, (void*)FollowMode_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("CameraMode", FieldType_Int, (void*)SetCameraMode_s, (void*)GetCameraMode_s, NULL, NULL, bOverride);
	pClass->AddField("CamAlwaysBehindObject", FieldType_Bool, (void*)SetCamAlwaysBehindObject_s, (void*)IsCamAlwaysBehindObject_s, NULL, NULL, bOverride);
	pClass->AddField("UseRightButtonBipedFacing", FieldType_Bool, (void*)SetUseRightButtonBipedFacing_s, (void*)IsUseRightButtonBipedFacing_s, NULL, NULL, bOverride);
	pClass->AddField("TurnBipedWhenWalkBackward", FieldType_Bool, (void*)SetTurnBipedWhenWalkBackward_s, (void*)IsTurnBipedWhenWalkBackward_s, NULL, NULL, bOverride);

	pClass->AddField("CameraObjectDistance", FieldType_Double, (void*)SetCameraObjectDistance_s, (void*)GetCameraObjectDistance_s, NULL, NULL, bOverride);
	pClass->AddField("CameraLiftupAngle", FieldType_Double, (void*)SetCameraLiftupAngle_s, (void*)GetCameraLiftupAngle_s, NULL, NULL, bOverride);
	pClass->AddField("CameraRotX", FieldType_Double, (void*)SetCameraRotX_s, (void*)GetCameraRotX_s, NULL, NULL, bOverride);
	pClass->AddField("CameraRotY", FieldType_Double, (void*)SetCameraRotY_s, (void*)GetCameraRotY_s, NULL, NULL, bOverride);
	pClass->AddField("CameraRotZ", FieldType_Double, (void*)SetCameraRotZ_s, (void*)GetCameraRotZ_s, NULL, NULL, bOverride);
	pClass->AddField("LookAtShiftY", FieldType_Double, (void*)SetLookAtShiftY_s, (void*)GetLookAtShiftY_s, NULL, NULL, bOverride);


	pClass->AddField("EnableKeyboard", FieldType_Bool, (void*)EnableKeyboard_s, (void*)GetEnableKeyboard_s, NULL, NULL, bOverride);
	pClass->AddField("EnableMouseLeftButton", FieldType_Bool, (void*)EnableMouseLeftButton_s, (void*)GetEnableMouseLeftButton_s, NULL, NULL, bOverride);
	pClass->AddField("EnableMouseRightButton", FieldType_Bool, (void*)EnableMouseRightButton_s, (void*)GetEnableMouseRightButton_s, NULL, NULL, bOverride);
	pClass->AddField("EnableMouseWheel", FieldType_Bool, (void*)EnableMouseWheel_s, (void*)GetEnableMouseWheel_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsGroupMask", FieldType_DWORD, (void*)SetPhysicsGroupMask_s, (void*)GetPhysicsGroupMask_s, NULL, NULL, bOverride);
	pClass->AddField("BlockInput", FieldType_Bool, (void*)SetBlockInput_s, (void*)IsBlockInput_s, NULL, NULL, bOverride);

	pClass->AddField("EnableMouseRightDrag", FieldType_Bool, (void*)EnableMouseRightDrag_s, (void*)IsEnableMouseRightDrag_s, NULL, NULL, bOverride);
	pClass->AddField("EnableMouseLeftDrag", FieldType_Bool, (void*)EnableMouseLeftDrag_s, (void*)IsEnableMouseLeftDrag_s, NULL, NULL, bOverride);

	pClass->AddField("LockMouseWhenDragging", FieldType_Bool, (void*)SetLockMouseWhenDragging_s, (void*)IsLockMouseWhenDragging_s, NULL, NULL, bOverride);

	pClass->AddField("UseCharacterLookup", FieldType_Bool, (void*)SetUseCharacterLookup_s, (void*)IsUseCharacterLookup_s, NULL, NULL, bOverride);
	pClass->AddField("UseCharacterLookupWhenMounted", FieldType_Bool, (void*)SetUseCharacterLookupWhenMounted_s, (void*)IsUseCharacterLookupWhenMounted_s, NULL, NULL, bOverride);
	pClass->AddField("CharacterLookupBoneIndex", FieldType_Int, (void*)SetCharacterLookupBoneIndex_s, (void*)GetCharacterLookupBoneIndex_s, NULL, NULL, bOverride);

	pClass->AddField("IsAlwaysRotateCameraWhenFPS", FieldType_Bool, (void*)EnableAlwaysRotateCameraWhenFPS_s, (void*)GetAlwaysRotateCameraWhenFPS_s, NULL, NULL, bOverride);

	pClass->AddField("IsCameraMoved", FieldType_Bool, NULL, (void*)IsCameraMoved_s, NULL, NULL, bOverride);

	pClass->AddField("MinCameraObjectDistance", FieldType_Double, (void*)SetMinCameraObjectDistance_s, (void*)GetMinCameraObjectDistance_s, NULL, NULL, bOverride);
	pClass->AddField("MaxCameraObjectDistance", FieldType_Double, (void*)SetMaxCameraObjectDistance_s, (void*)GetMaxCameraObjectDistance_s, NULL, NULL, bOverride);

	pClass->AddField("CameraRollbackSpeed", FieldType_Float, (void*)SetCameraRollbackSpeed_s, (void*)GetCameraRollbackSpeed_s, NULL, NULL, bOverride);
	pClass->AddField("CameraLookatOffset", FieldType_Vector3, (void*)SetCameraLookatOffset_s, (void*)GetCameraLookatOffset_s, NULL, NULL, bOverride);
	pClass->AddField("AdditionalCameraRotate", FieldType_Vector3, (void*)SetAdditionalCameraRotate_s, (void*)GetAdditionalCameraRotate_s, NULL, NULL, bOverride);

	pClass->AddField("MaxYShiftSpeed", FieldType_Float, (void*)SetMaxYShiftSpeed_s, (void*)GetMaxYShiftSpeed_s, NULL, NULL, bOverride);
	pClass->AddField("MaxAllowedYShift", FieldType_Float, (void*)SetMaxAllowedYShift_s, (void*)GetMaxAllowedYShift_s, NULL, NULL, bOverride);

	pClass->AddField("ControlBiped", FieldType_Bool, (void*)SetControlBiped_s, (void*)IsControlBiped_s, NULL, NULL, bOverride);
	pClass->AddField("EnableBlockCollision", FieldType_Bool, (void*)SetEnableBlockCollision_s, (void*)IsEnableBlockCollision_s, NULL, NULL, bOverride);
	pClass->AddField("EnableTerrainCollision", FieldType_Bool, (void*)SetEnableTerrainCollision_s, (void*)IsEnableTerrainCollision_s, NULL, NULL, bOverride);
	pClass->AddField("IgnoreEyeBlockCollisionInSunlight", FieldType_Bool, (void*)SetIgnoreEyeBlockCollisionInSunlight_s, (void*)IsIgnoreEyeBlockCollisionInSunlight_s, NULL, NULL, bOverride);
	pClass->AddField("UpdateBipedFlyDir", FieldType_void, (void*)UpdateBipedFlyDir_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("RenderOrigin", FieldType_Vector3, (void*)0, (void*)GetRenderOrigin_s, NULL, NULL, bOverride);
	pClass->AddField("ViewProjMatrix", FieldType_Matrix4, (void*)0, (void*)GetViewProjMatrix_s, NULL, NULL, bOverride);
	pClass->AddField("ViewMatrix", FieldType_Matrix4, (void*)0, (void*)GetViewMatrix_s, NULL, NULL, bOverride);
	pClass->AddField("ProjMatrix", FieldType_Matrix4, (void*)0, (void*)GetProjMatrix_s, NULL, NULL, bOverride);
	return S_OK;
}
