//-----------------------------------------------------------------------------
// Class:	CBaseCamera
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8
// desc	: Code based on Microsoft Direct3D 9.0b SDK sample framework.
// key mapping modified.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseObject.h"
#include "ParaEngineSettings.h"
#include "SceneObject.h"
#include "Framework/InputSystem/VirtualKey.h"
#include "BaseCamera.h"

#ifndef min 
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

using namespace ParaEngine;

EVirtualKey ParaEngine::CBaseCamera::m_scancode_key_map[] = {
	EVirtualKey::KEY_A,
	EVirtualKey::KEY_D,
	EVirtualKey::KEY_W,
	EVirtualKey::KEY_S,
	EVirtualKey::KEY_E,
	EVirtualKey::KEY_Q,
	EVirtualKey::KEY_SPACE,
	EVirtualKey::KEY_UNKNOWN,
	EVirtualKey::KEY_UNKNOWN,
	EVirtualKey::KEY_INSERT,
	EVirtualKey::KEY_DELETE,
	EVirtualKey::KEY_X,
};

//-----------------------------------------------------------------------------
// Name: CBaseCamera
// Desc: Constructor
//-----------------------------------------------------------------------------
CBaseCamera::CBaseCamera()
{
	ZeroMemory(m_aKeys, sizeof(BYTE)*MAX_KEYS);

	// Set attributes for the view matrix
	DVector3 vEyePt(0.0f, 0.0f, 0.0f);
	DVector3 vLookatPt(0.0f, 0.0f, 1.0f);

#ifdef WIN32
	GetCursorPos(&m_ptLastMousePosition);
#endif
	m_bMouseLButtonDown = false;
	m_bMouseMButtonDown = false;
	m_bMouseRButtonDown = false;
	m_nCurrentButtonMask = 0;
	m_nMouseWheelDelta = 0;

	m_fCameraYawAngle = 0.0f;
	m_fCameraPitchAngle = 0.0f;

	m_vVelocity = Vector3(0, 0, 0);
	m_bMovementDrag = true;
	m_vVelocityDrag = Vector3(0, 0, 0);
	m_fDragTimer = 0.0f;
	m_fTotalDragTimeToZero = 0.5f;
	m_vRotVelocity = Vector2(0, 0);

	m_fRotationScaler = 0.01f;
	m_fMoveScaler = 5.0f;

	m_bInvertPitch = CGlobals::GetSettings().GetMouseInverse();
	m_bEnableYAxisMovement = true;
	m_bEnablePositionMovement = true;

	m_vMouseDelta = Vector2(0, 0);
	m_fFramesToSmoothMouseData = 2.0f;

	m_bClipToBoundary = false;
	m_vMinBoundary = DVector3(-1, -1, -1);
	m_vMaxBoundary = DVector3(1, 1, 1);

	m_bResetCursorAfterMove = false;
	m_fOrthoHeight = 100.f;
	m_fOrthoWidth = 100.f;
	m_bIsPerspectiveView = true;
	m_vUp = Vector3(0, 1.f, 0);

	// Setup the view matrix
	SetViewParams(vEyePt, vLookatPt);

	// Setup the projection matrix
	SetProjParams(MATH_PI / 4, 1.0f, 1.0f, 1000.0f);
}

void ParaEngine::CBaseCamera::CopyCameraParamsFrom(CBaseCamera* pFromCamera)
{
	m_fNearPlane = pFromCamera->m_fNearPlane;
	m_fFarPlane = pFromCamera->m_fFarPlane;
	m_fFOV = pFromCamera->m_fFOV;
	m_fAspect = pFromCamera->m_fAspect;
	m_vDefaultEye = m_vEye = pFromCamera->m_vEye;
	m_vDefaultLookAt = m_vLookAt = pFromCamera->m_vLookAt;
	m_mView = pFromCamera->m_mView;
	m_mProj = pFromCamera->m_mProj;
	m_bIsPerspectiveView = pFromCamera->m_bIsPerspectiveView;
}

VOID CBaseCamera::SetInvertPitch(bool bInvertPitch)
{
	m_bInvertPitch = bInvertPitch;
}

//-----------------------------------------------------------------------------
// Name: SetViewParams
// Desc: Client can call this to change the position and direction of camera
//-----------------------------------------------------------------------------
void CBaseCamera::SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt, const Vector3* up)
{
	m_vDefaultEye = m_vEye = vEyePt;
	m_vDefaultLookAt = m_vLookAt = vLookatPt;
	// Calculate  the view matrix
	if (up != NULL) {
		m_vUp = *up;
	}
	ParaMatrixLookAtLH(&m_mView, vEyePt, vLookatPt, DVector3(m_vUp));

	Matrix4 mInvView;
	mInvView = m_mView.inverse();

	// The axis basis vectors and camera position are stored inside the 
	// position matrix in the 4 rows of the camera's world matrix.
	// To figure out the yaw/pitch of the camera, we just need the Z basis vector
	Vector3* pZBasis = (Vector3*)&mInvView._31;

	m_fCameraYawAngle = atan2f(pZBasis->x, pZBasis->z);
	float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
	m_fCameraPitchAngle = -atan2f(pZBasis->y, fLen);
}

//-----------------------------------------------------------------------------
// Name: SetProjParams
// Desc: Calculates the projection matrix based on input params
//-----------------------------------------------------------------------------
void CBaseCamera::SetProjParams(FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane,
	FLOAT fFarPlane)
{
	// Set attributes for the projection matrix
	m_fFOV = fFOV;
	m_fAspect = fAspect;
	m_fNearPlane = fNearPlane;
	m_fFarPlane = fFarPlane;

	if (IsPerspectiveView())
	{
		ParaMatrixPerspectiveFovLH(&m_mProj, fFOV, fAspect, fNearPlane, fFarPlane);
	}
	else
	{
		// orthographic projection matrix, if fov is very small.
		ParaMatrixOrthoLH(&m_mProj, GetOrthoWidth(), GetOrthoHeight(), fNearPlane, fFarPlane);
	}
}

void  CBaseCamera::SetIsPerspectiveView(bool bIsPerspectiveView)
{
	m_bIsPerspectiveView = bIsPerspectiveView;
	UpdateProjParams();
}

void CBaseCamera::UpdateProjParams()
{
	SetProjParams(GetFieldOfView(), GetAspectRatio(), GetNearPlane(), GetFarPlane());
}

void CBaseCamera::UpdateProjParams(FLOAT fAspect)
{
	SetAspectRatio(fAspect);
}

void CBaseCamera::SetOrthoHeight(float fHeight)
{
	m_fOrthoHeight = fHeight;
	UpdateProjParams();
}

void CBaseCamera::SetOrthoWidth(float fWidth)
{
	m_fOrthoWidth = fWidth;
	UpdateProjParams();
}

void CBaseCamera::SetAspectRatio(float fAspect)
{
	if (m_fAspect != fAspect)
	{
		m_fAspect = fAspect;
		UpdateProjParams();
	}
}

void CBaseCamera::SetFieldOfView(float fFov)
{
	if (m_fFOV != fFov)
	{
		m_fFOV = fFov;
		UpdateProjParams();
	}
}

void CBaseCamera::SetFieldOfView(float fov_v, float fov_h)
{
	float p_fAspect = tan(fov_h / 2) / tan(fov_v / 2);
	if (m_fFOV != fov_v || m_fAspect!= p_fAspect)
	{
		m_fFOV = fov_v;
		m_fAspect = p_fAspect;
		UpdateProjParams();
	}
}

void CBaseCamera::SetNearPlane(float fNearPlane)
{
	m_fNearPlane = fNearPlane;
	UpdateProjParams();
}

void CBaseCamera::SetFarPlane(float fFarPlane)
{
	m_fFarPlane = fFarPlane;
	UpdateProjParams();
}


//-----------------------------------------------------------------------------
// Name: HandleMessages
// Desc: Call this from your message proc so this class can handle window messages
// this is no longer called.
//-----------------------------------------------------------------------------
LRESULT CBaseCamera::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if defined( USE_DIRECTX_RENDERER)
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(lParam);

	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		// Map this key to a CharacterAndCameraKeysenum and update the
		// state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
		// only if the key is not down
		CharacterAndCameraKeys mappedKey = MapKey((UINT)wParam);
		if (mappedKey != KEY_UNKNOWN)
		{
			if (FALSE == IsKeyDown(m_aKeys[mappedKey]))
				m_aKeys[mappedKey] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
		}
		break;
	}

	case WM_KEYUP:
	{
		// Map this key to a CharacterAndCameraKeysenum and update the
		// state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
		CharacterAndCameraKeys mappedKey = MapKey((UINT)wParam);
		if (mappedKey != KEY_UNKNOWN)
			m_aKeys[mappedKey] &= ~KEY_IS_DOWN_MASK;
		break;
	}

	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
	{
		// Update member var state
		if (uMsg == WM_LBUTTONDOWN) { m_bMouseLButtonDown = true; m_nCurrentButtonMask |= MOUSE_LEFT_BUTTON; }
		if (uMsg == WM_MBUTTONDOWN) { m_bMouseMButtonDown = true; m_nCurrentButtonMask |= MOUSE_MIDDLE_BUTTON; }
		if (uMsg == WM_RBUTTONDOWN) { m_bMouseRButtonDown = true; m_nCurrentButtonMask |= MOUSE_RIGHT_BUTTON; }

		// Capture the mouse, so if the mouse button is 
		// released outside the window, we'll get the WM_LBUTTONUP message
		SetCapture(hWnd);
		GetCursorPos(&m_ptLastMousePosition);
		return TRUE;
	}

	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
	{
		// Update member var state
		if (uMsg == WM_LBUTTONUP) { m_bMouseLButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON; }
		if (uMsg == WM_MBUTTONUP) { m_bMouseMButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON; }
		if (uMsg == WM_RBUTTONUP) { m_bMouseRButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON; }

		// Release the capture if no mouse buttons down
		if (!m_bMouseLButtonDown &&
			!m_bMouseRButtonDown &&
			!m_bMouseMButtonDown)
		{
			ReleaseCapture();
		}
		break;
	}

	case WM_MOUSEWHEEL:
		// Update member var state
		m_nMouseWheelDelta = (short)HIWORD(wParam) / 120;
		break;
	}
#endif
	return FALSE;
}


void CBaseCamera::UpdateMouseDelta(int nDX, int nDY)
{
	POINT ptCurMouseDelta;
	// Calc how far it's moved since last frame
	ptCurMouseDelta.x = nDX;
	ptCurMouseDelta.y = nDY;

	// Smooth the relative mouse data over a few frames so it isn't 
	// jerky when moving slowly at low frame rates.
	float fPercentOfNew = 1.0f / m_fFramesToSmoothMouseData;
	float fPercentOfOld = 1.0f - fPercentOfNew;
	m_vMouseDelta.x = m_vMouseDelta.x*fPercentOfOld + ptCurMouseDelta.x*fPercentOfNew;
	m_vMouseDelta.y = m_vMouseDelta.y*fPercentOfOld + ptCurMouseDelta.y*fPercentOfNew;
	if (Math::Abs(m_vMouseDelta.x - ptCurMouseDelta.x) <= 1.0)
		m_vMouseDelta.x = (float)ptCurMouseDelta.x;
	if (Math::Abs(m_vMouseDelta.y - ptCurMouseDelta.y) <= 1.0)
		m_vMouseDelta.y = (float)ptCurMouseDelta.y;
	m_vRotVelocity = m_vMouseDelta * m_fRotationScaler;
}

//-----------------------------------------------------------------------------
// Name: UpdateMouseDelta
// Desc: Figure out the mouse delta based on mouse movement
//-----------------------------------------------------------------------------
void CBaseCamera::UpdateMouseDelta(float fElapsedTime)
{
#ifdef USE_DIRECTX_RENDERER
	UNREFERENCED_PARAMETER(fElapsedTime);

	POINT ptCurMouseDelta;
	POINT ptCurMousePos;

	// Get current position of mouse
	GetCursorPos(&ptCurMousePos);

	// Calc how far it's moved since last frame
	ptCurMouseDelta.x = ptCurMousePos.x - m_ptLastMousePosition.x;
	ptCurMouseDelta.y = ptCurMousePos.y - m_ptLastMousePosition.y;

	// Record current position for next time
	m_ptLastMousePosition = ptCurMousePos;

	if (m_bResetCursorAfterMove)
	{
		// Set position of camera to center of desktop, 
		// so it always has room to move.  This is very useful
		// if the cursor is hidden.  If this isn't done and cursor is hidden, 
		// then invisible cursor will hit the edge of the screen 
		// and the user can't tell what happened
		POINT ptCenter;
		RECT rcDesktop;
		GetWindowRect(GetDesktopWindow(), &rcDesktop);
		ptCenter.x = (rcDesktop.right - rcDesktop.left) / 2;
		ptCenter.y = (rcDesktop.bottom - rcDesktop.top) / 2;
		SetCursorPos(ptCenter.x, ptCenter.y);
		m_ptLastMousePosition = ptCenter;
	}

	// Smooth the relative mouse data over a few frames so it isn't 
	// jerky when moving slowly at low frame rates.
	float fPercentOfNew = 1.0f / m_fFramesToSmoothMouseData;
	float fPercentOfOld = 1.0f - fPercentOfNew;
	m_vMouseDelta.x = m_vMouseDelta.x*fPercentOfOld + ptCurMouseDelta.x*fPercentOfNew;
	m_vMouseDelta.y = m_vMouseDelta.y*fPercentOfOld + ptCurMouseDelta.y*fPercentOfNew;
	if (Math::Abs(m_vMouseDelta.x - ptCurMouseDelta.x) <= 1.0)
		m_vMouseDelta.x = (float)ptCurMouseDelta.x;
	if (Math::Abs(m_vMouseDelta.y - ptCurMouseDelta.y) <= 1.0)
		m_vMouseDelta.y = (float)ptCurMouseDelta.y;
	m_vRotVelocity = m_vMouseDelta * m_fRotationScaler;
#endif
}




//-----------------------------------------------------------------------------
// Name: UpdateVelocity
// Desc: Figure out the velocity based on keyboard input & drag if any
//-----------------------------------------------------------------------------
void CBaseCamera::UpdateVelocity(float fElapsedTime)
{
	Matrix4 mRotDelta;
	Vector3 vAccel = Vector3(0, 0, 0);

	if (m_bEnablePositionMovement)
	{
		// Update acceleration vector based on keyboard state
		if (IsKeyDown(m_aKeys[MOVE_FORWARD]))
			vAccel.z += 1.0f;
		if (IsKeyDown(m_aKeys[MOVE_BACKWARD]))
			vAccel.z -= 1.0f;
		if (m_bEnableYAxisMovement)
		{
			if (IsKeyDown(m_aKeys[SHIFT_RIGHT]))
				vAccel.y += 1.0f;
			if (IsKeyDown(m_aKeys[SHIFT_LEFT]))
				vAccel.y -= 1.0f;
		}
		if (IsKeyDown(m_aKeys[MOVE_RIGHT]))
			vAccel.x += 1.0f;
		if (IsKeyDown(m_aKeys[MOVE_LEFT]))
			vAccel.x -= 1.0f;
	}

	// Normalize vector so if moving 2 dirs (left & forward), 
	// the camera doesn't move faster than if moving in 1 dir
	vAccel.normalise();

	// Scale the acceleration vector
	vAccel *= m_fMoveScaler;

	if (m_bMovementDrag)
	{
		// Is there any acceleration this frame?
		if (vAccel.squaredLength() > 0)
		{
			// If so, then this means the user has pressed a movement key\
			// so change the velocity immediately to acceleration 
			// upon keyboard input.  This isn't normal physics
			// but it will give a quick response to keyboard input
			m_vVelocity = vAccel;
			m_fDragTimer = m_fTotalDragTimeToZero;
			m_vVelocityDrag = vAccel / m_fDragTimer;
		}
		else
		{
			// If no key being pressed, then slowly decrease velocity to 0
			if (m_fDragTimer > 0)
			{
				// Drag until timer is <= 0
				m_vVelocity -= m_vVelocityDrag * fElapsedTime;
				m_fDragTimer -= fElapsedTime;
			}
			else
			{
				// Zero velocity
				m_vVelocity = Vector3(0, 0, 0);
			}
		}
	}
	else
	{
		// No drag, so immediately change the velocity
		m_vVelocity = vAccel;
	}
}

void CBaseCamera::ConstrainToBoundary(DVector3* pV)
{
	// Constrain vector to a bounding box 
	pV->x = max(pV->x, m_vMinBoundary.x);
	pV->y = max(pV->y, m_vMinBoundary.y);
	pV->z = max(pV->z, m_vMinBoundary.z);

	pV->x = min(pV->x, m_vMaxBoundary.x);
	pV->y = min(pV->y, m_vMaxBoundary.y);
	pV->z = min(pV->z, m_vMaxBoundary.z);
}


//-----------------------------------------------------------------------------
// Name: MapKey
// Desc: Maps a windows virtual key to an enum
//-----------------------------------------------------------------------------
CharacterAndCameraKeys CBaseCamera::MapKey(UINT nKey)
{
#ifdef USE_DIRECTX_RENDERER
	// This could be upgraded to a method that's user-definable but for 
	// simplicity, we'll use a hard-coded mapping.
	switch (nKey)
	{
	case VK_LEFT:  return MOVE_LEFT;
	case VK_RIGHT: return MOVE_RIGHT;
	case VK_UP:    return MOVE_FORWARD;
	case VK_DOWN:  return MOVE_BACKWARD;
	case VK_PRIOR: return SHIFT_RIGHT;        // pgup
	case VK_HOME:  return SHIFT_LEFT;      // pgdn

	case 'A':      return MOVE_LEFT;
	case 'D':      return MOVE_RIGHT;
	case 'W':      return MOVE_FORWARD;
	case 'S':      return MOVE_BACKWARD;
	case 'Q':      return SHIFT_LEFT;
	case 'E':      return SHIFT_RIGHT;
	case 'X':	   return FLY_DOWNWARD;
	case ' ':      return CAM_LOCK;

	case VK_NUMPAD4: return MOVE_LEFT;
	case VK_NUMPAD6: return MOVE_RIGHT;
	case VK_NUMPAD8: return MOVE_FORWARD;
	case VK_NUMPAD2: return MOVE_BACKWARD;
	case VK_NUMPAD9: return SHIFT_RIGHT;
	case VK_NUMPAD7: return SHIFT_LEFT;

	case VK_END:	 return CAM_RESET;
	case VK_CONTROL: return CAM_LOCK;
	}
#endif
	return KEY_UNKNOWN;
}

void CBaseCamera::Reset()
{
	SetViewParams(m_vDefaultEye, m_vDefaultLookAt);
}

CFirstPersonCamera::CFirstPersonCamera()
{
}

void CFirstPersonCamera::FrameMove(float fElapsedTime)
{
	if (IsKeyDown(m_aKeys[CAM_RESET]))
		Reset();

	// Get amount of velocity based on the keyboard input and drag (if any)
	UpdateVelocity(fElapsedTime);

	// Simple euler method to calculate position delta
	Vector3 vPosDelta = m_vVelocity * fElapsedTime;

	// If rotating the camera 
	if (m_bMouseLButtonDown || m_bMouseMButtonDown || m_bMouseRButtonDown)
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
	Vector3 vWorldUp, vWorldAhead;
	Vector3 vLocalUp = Vector3(0, 1, 0);
	Vector3 vLocalAhead = Vector3(0, 0, 1);
	ParaVec3TransformCoord(&vWorldUp, &vLocalUp, &mCameraRot);
	ParaVec3TransformCoord(&vWorldAhead, &vLocalAhead, &mCameraRot);

	// Transform the position delta by the camera's rotation 
	Vector3 vPosDeltaWorld;
	ParaVec3TransformCoord(&vPosDeltaWorld, &vPosDelta, &mCameraRot);
	if (!m_bEnableYAxisMovement)
		vPosDeltaWorld.y = 0.0f;

	// Move the eye position 
	m_vEye += vPosDeltaWorld;
	if (m_bClipToBoundary)
		ConstrainToBoundary(&m_vEye);

	// Update the lookAt position based on the eye position 
	m_vLookAt = m_vEye + vWorldAhead;

	Vector3 vEye = m_vEye;
	Vector3 vLookAt = m_vLookAt;
	// Update the view matrix
	ParaMatrixLookAtLH(&m_mView, &vEye, &vLookAt, &vWorldUp);

	m_mCameraWorld = m_mView.inverse();
}
int CBaseCamera::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CTileObject::InstallFields(pClass, bOverride);

	pClass->AddField("NearPlane", FieldType_Float, (void*)SetNearPlane_s, (void*)GetNearPlane_s, NULL, NULL, bOverride);
	pClass->AddField("FarPlane", FieldType_Float, (void*)SetFarPlane_s, (void*)GetFarPlane_s, NULL, NULL, bOverride);
	pClass->AddField("FieldOfView", FieldType_Float, (void*)SetFieldOfView_s, (void*)GetFieldOfView_s, NULL, NULL, bOverride);
	pClass->AddField("AspectRatio", FieldType_Float, (void*)SetAspectRatio_s, (void*)GetAspectRatio_s, NULL, NULL, bOverride);

	pClass->AddField("IsPerspectiveView", FieldType_Bool, (void*)SetIsPerspectiveView_s, (void*)IsPerspectiveView_s, NULL, NULL, bOverride);
	pClass->AddField("OrthoWidth", FieldType_Float, (void*)SetOrthoWidth_s, (void*)GetOrthoWidth_s, NULL, NULL, bOverride);
	pClass->AddField("OrthoHeight", FieldType_Float, (void*)SetOrthoHeight_s, (void*)GetOrthoHeight_s, NULL, NULL, bOverride);

	pClass->AddField("MoveScaler", FieldType_Float, (void*)SetMoveScaler_s, (void*)GetMoveScaler_s, NULL, NULL, bOverride);
	pClass->AddField("RotationScaler", FieldType_Float, (void*)SetRotationScaler_s, (void*)GetRotationScaler_s, NULL, NULL, bOverride);
	pClass->AddField("IsShiftMoveSwitched", FieldType_Bool, (void*)SetShiftMoveSwitched_s, (void*)IsShiftMoveSwitched_s, NULL, NULL, bOverride);

	pClass->AddField("InvertPitch", FieldType_Bool, (void*)SetInvertPitch_s, (void*)IsInvertPitch_s, NULL, NULL, bOverride);
	pClass->AddField("MovementDrag", FieldType_Bool, (void*)SetMovementDrag_s, (void*)GetMovementDrag_s, NULL, NULL, bOverride);
	pClass->AddField("TotalDragTime", FieldType_Float, (void*)SetTotalDragTime_s, (void*)GetTotalDragTime_s, NULL, NULL, bOverride);

	pClass->AddField("SmoothFramesNum", FieldType_Int, (void*)SetNumberOfFramesToSmoothMouseData_s, (void*)GetNumberOfFramesToSmoothMouseData_s, NULL, NULL, bOverride);
	pClass->AddField("Eye position", FieldType_DVector3, (void*)SetEyePosition_s, (void*)GetEyePosition_s, NULL, NULL, bOverride);
	pClass->AddField("Lookat position", FieldType_DVector3, (void*)SetLookAtPosition_s, (void*)GetLookAtPosition_s, NULL, NULL, bOverride);
	pClass->AddField("CameraUp", FieldType_Vector3, (void*)SetCameraUp_s, (void*)GetCameraUp_s, NULL, NULL, bOverride);
	pClass->AddField("FrameMove", FieldType_void, (void*)FrameMove_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("On_FrameMove", FieldType_String, (void*)SetFrameMove_s, (void*)GetFrameMove_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);

	return S_OK;
}

void ParaEngine::CBaseCamera::SetMovementDrag(bool bEnable)
{
	m_bMovementDrag = bEnable;
}

bool ParaEngine::CBaseCamera::GetMovementDrag()
{
	return m_bMovementDrag;
}

void ParaEngine::CBaseCamera::SetTotalDragTime(float fTime)
{
	m_fTotalDragTimeToZero = fTime;
}

float ParaEngine::CBaseCamera::GetTotalDragTime()
{
	return m_fTotalDragTimeToZero;
}


void ParaEngine::CBaseCamera::SetKeyMap(CharacterAndCameraKeys key, const EVirtualKey& scancode)
{
	m_scancode_key_map[key] = scancode;
}

ParaEngine::EVirtualKey ParaEngine::CBaseCamera::GetKeyMap(CharacterAndCameraKeys key)
{
	return m_scancode_key_map[key];
}

void ParaEngine::CBaseCamera::FrameMove(FLOAT fElapsedTime)
{
}

VOID ParaEngine::CBaseCamera::SetRotationScaler(FLOAT fRotationScaler /*= 0.01f*/)
{
	m_fRotationScaler = fRotationScaler;
}

float ParaEngine::CBaseCamera::GetRotationScaler()
{
	return m_fRotationScaler;
}

VOID ParaEngine::CBaseCamera::SetMoveScaler(FLOAT fMoveScaler /*= 5.0f*/)
{
	m_fMoveScaler = fMoveScaler;
}

float ParaEngine::CBaseCamera::GetMoveScaler()
{
	return m_fMoveScaler;
}

bool ParaEngine::CBaseCamera::IsShiftMoveSwitched()
{
	return !(m_scancode_key_map[MOVE_LEFT] == EVirtualKey::KEY_A);
}

void ParaEngine::CBaseCamera::SetShiftMoveSwitched(bool bSwitched /*= false*/)
{
	if (bSwitched) {
		m_scancode_key_map[MOVE_LEFT] = EVirtualKey::KEY_Q;
		m_scancode_key_map[MOVE_RIGHT] = EVirtualKey::KEY_E;
		m_scancode_key_map[SHIFT_LEFT] = EVirtualKey::KEY_A;
		m_scancode_key_map[SHIFT_RIGHT] = EVirtualKey::KEY_D;
	}
	else {
		m_scancode_key_map[MOVE_LEFT] = EVirtualKey::KEY_A;
		m_scancode_key_map[MOVE_RIGHT] = EVirtualKey::KEY_D;
		m_scancode_key_map[SHIFT_LEFT] = EVirtualKey::KEY_Q;
		m_scancode_key_map[SHIFT_RIGHT] = EVirtualKey::KEY_E;
	}
}

Vector3 ParaEngine::CBaseCamera::GetRenderEyePosition()
{
	return GetEyePosition();
}

//-----------------------------------------------------------------------------
// Name: GetMouseRay
/// Desc: Get the mouse pay for object picking
/// [in] fCursor: mouse position
/// [in] nWidth, nHeight: Device Screen size
/// [in] world transform matrix
//-----------------------------------------------------------------------------
void CBaseCamera::GetMouseRay(Vector3& vPickRayOrig, Vector3& vPickRayDir, POINT ptCursor, UINT nWidth, UINT nHeight, const Matrix4* pMatWorld)
{
	Matrix4* pMatProj = GetProjMatrix();

	// Compute the vector of the pick ray in screen space
	Vector3 v;
	v.x = (((2.0f * ptCursor.x) / nWidth) - 1.0f) / pMatProj->_11;
	v.y = -(((2.0f * ptCursor.y) / nHeight) - 1.0f) / pMatProj->_22;
	v.z = 1.0f;

	// Get the inverse of the composite view and world matrix
	Matrix4*	pMatView = GetViewMatrix();
	Matrix4	m;
	m = (*pMatWorld)*(*pMatView);
	m.invert();

	// Transform the screen space pick ray into 3D space
	vPickRayDir.x = v.x*m._11 + v.y*m._21 + v.z*m._31;
	vPickRayDir.y = v.x*m._12 + v.y*m._22 + v.z*m._32;
	vPickRayDir.z = v.x*m._13 + v.y*m._23 + v.z*m._33;
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;
	vPickRayDir.normalise();
}

void ParaEngine::CBaseCamera::SetEyePosition(const DVector3& pos)
{
	SetViewParams(pos, m_vLookAt);
}

void ParaEngine::CBaseCamera::SetLookAtPosition(const DVector3& pos)
{
	SetViewParams(m_vEye, pos);
}

void ParaEngine::CBaseCamera::SetCameraUp(const Vector3& pos)
{
	SetViewParams(m_vEye, m_vLookAt, &pos);
}

Vector3 ParaEngine::CBaseCamera::GetCameraUp()
{
	return m_vUp;
}

//-----------------------------------------------------------------------------
// Name: UpdateFrustum()
/// Desc: Sets up the frustum planes, endpoints, and center for the frustum
///       defined by a given view matrix and projection matrix.  This info will 
///       be used when culling each object in CullObject().
/// Update the cull planes.
/// currently only use the front plane
//-------------------------------------------------------------------
void CBaseCamera::UpdateFrustum()
{
	const Matrix4& matView = CGlobals::GetViewMatrixStack().SafeGetTop();
	const Matrix4& matProj = CGlobals::GetProjectionMatrixStack().SafeGetTop();

	Matrix4 matViewProj_inverse;
	ParaMatrixMultiply(&matViewProj_inverse, &matView, &matProj);
	matViewProj_inverse = matViewProj_inverse.inverse();

	float fNearPlane = 0.f;

	// update the camera view frustum
	m_frustum.UpdateFrustum(&matViewProj_inverse, true, fNearPlane, 1.f);

	// update the object view frustum
	if (IsPerspectiveView())
	{
		// perspective view
		float fFarPlane = CGlobals::GetScene()->GetFogEnd();
		fFarPlane = min(fFarPlane, m_fFarPlane);
		fFarPlane = (matProj._33 * fFarPlane + matProj._43) / fFarPlane;
		fFarPlane = min(fFarPlane, 1.0f);
		m_frustum_object.UpdateFrustum(&matViewProj_inverse, true, fNearPlane, fFarPlane);

		// update the shadow view frustum
		fFarPlane = CGlobals::GetScene()->GetShadowRadius();
		fFarPlane = min(fFarPlane, m_fFarPlane);
		fFarPlane = (matProj._33 * fFarPlane + matProj._43) / fFarPlane;
		fFarPlane = min(fFarPlane, 1.0f);
		m_frustum_shadow.UpdateFrustum(&matViewProj_inverse, true, fNearPlane, fFarPlane);
	}
	else
	{
		// orthogonal view. 
		m_frustum_object.UpdateFrustum(&matViewProj_inverse, true, fNearPlane, 1.f);
		m_frustum_shadow.UpdateFrustum(&matViewProj_inverse, true, fNearPlane, 1.f);
	}

	// update portal frustum
	Vector3 vEyePos(0, 0, 0);
#ifdef USE_OPENGL_RENDERER
	// opengl, z axis is in range -1,1
	vEyePos.z = -1.f;
#endif
	ParaVec3TransformCoord(&vEyePos, &vEyePos, &matViewProj_inverse);
	m_frustum_portal.setOrigin(vEyePos);
	m_frustum_portal.RemoveAllExtraCullingPlanes();
	// the near plane of the portal frustum must be zero, so shift m_fNearPlane
	m_frustum_portal.AddFrustum(&m_frustum_object, m_fNearPlane);

	// fog plane is a plane whose positive side is along the eye direction and distance to eye is the fog far end.
	const Plane& plane = m_frustum.planeFrustum[0]; // near plane
	m_fog_plane.Set(plane.a(), plane.b(), plane.c(), (plane.d - CGlobals::GetScene()->GetFogEnd()));
}

/**
* Check if the camera can see the object.
* the bounding shape such as a sphere or a box is used to test whether the camera can see
* the object.
* @param pObj: the scene object to be tested
* @return return true if any portion of the object can be seen by the camera
* @see void CBaseObject::GetVertices(Vector3 * pVertices, int* pNumber)
*/

//-------------------------------------------------------------------
bool CBaseCamera::CanSeeObject(IViewClippingObject* pObj)
{
	Vector3 pVecBounds[8];
	int nNumber;
	pObj->GetRenderVertices(pVecBounds, &nNumber);

	return GetObjectFrustum()->CullPointsWithFrustum(pVecBounds, nNumber);
}

