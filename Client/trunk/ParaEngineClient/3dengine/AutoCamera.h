#pragma once
#include "BaseCamera.h"

namespace ParaEngine
{
	class CBipedObject;
/**
Auto Camera is designed to handle smooth transitions between 
supported camera type, such as first person camera, third person camera,
and rotation camera. It can be set to follow a Biped Object in the 
scene. The Scene Object automatically creates one instance of this 
object when loading; and manages the entire camera automatically.
*/
class CAutoCamera :	public CFirstPersonCamera
{
public:
	CAutoCamera(void);
	~CAutoCamera(void);

	enum CameraConstraintEnum
	{
		CameraConstraintFocus,
		CameraConstraintNone
	};

	
	struct CameraConstraint
	{
	protected:
		CameraConstraintEnum m_typeConstraint;
	public:
		void SetConstraintType(CameraConstraintEnum typeC){m_typeConstraint = typeC;};
		CameraConstraintEnum GetConstraintType(){return m_typeConstraint;};
		
		/// the box within which the camera lookat point must be
		Vector3 m_vMinLookAt;
		Vector3 m_vMaxLookAt;

		/// the box within which the camera eyeat point must be
		Vector3 m_vMinEye;
		Vector3 m_vMaxEye;
	public:
		void ConstrainToBoundary( Vector3* pV, const Vector3* pvMin, const Vector3* pvMax );
		void ConstrainToOuterBoundary( Vector3* pV, const Vector3* pvMin, const Vector3* pvMax );

		void UpdateFocusConstraint(const Vector3* pV, 
					FLOAT fLookAtRadius, FLOAT fLookAtMinHeight,FLOAT fLookAtMaxHeight,
					FLOAT fEyeRadius, FLOAT fEyeMinHeight,FLOAT fEyeMaxHeight);
		void BoundToFocusConstraint(Vector3* pEye, Vector3* pLookAt);
		
		CameraConstraint()
		{
			m_typeConstraint = CameraConstraintNone;
		};
	};
	
public:
	//////////////////////////////////////////////////////////////////////////
	// implementation of IAttributeFields

	/** attribute class ID should be identical, unless one knows how overriding rules work.*/
	virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CAutoCamera;}
	/** a static string, describing the attribute class object's name */
	virtual const char* GetAttributeClassName(){static const char name[] = "CAutoCamera"; return name;}
	/** a static string, describing the attribute class object */
	virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
	/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
	virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

	
	ATTRIBUTE_METHOD1(CAutoCamera, IsAlwaysRun_s, bool*)	{*p1 = cls->IsAlwaysRun(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetAlwaysRun_s, bool)	{cls->SetAlwaysRun(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsCamAlwaysBehindObject_s, bool*)	{*p1 = cls->IsCamAlwaysBehindObject(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetCamAlwaysBehindObject_s, bool)	{cls->SetCamAlwaysBehindObject(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsUseRightButtonBipedFacing_s, bool*)	{*p1 = cls->IsUseRightButtonBipedFacing(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetUseRightButtonBipedFacing_s, bool)	{cls->SetUseRightButtonBipedFacing(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsTurnBipedWhenWalkBackward_s, bool*) { *p1 = cls->IsTurnBipedWhenWalkBackward(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetTurnBipedWhenWalkBackward_s, bool) { cls->SetTurnBipedWhenWalkBackward(p1); return S_OK; }
	

	ATTRIBUTE_METHOD(CAutoCamera, Reset_s)	{cls->Reset(); return S_OK;}
	ATTRIBUTE_METHOD(CAutoCamera, FreeCameraMode_s)	{cls->FreeCameraMode(); return S_OK;}
	ATTRIBUTE_METHOD(CAutoCamera, FollowMode_s)	{cls->FollowMode(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraMode_s, int*)	{*p1 = (int)(cls->GetCameraMode()); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraMode_s, int)	{cls->SetCameraMode((CameraMode)p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetKeyboardMovVelocity_s, float*)	{*p1 = cls->GetKeyboardMovVelocity(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetKeyboardMovVelocity_s, float)	{cls->SetKeyboardMovVelocity(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetKeyboardRotVelocity_s, float*)	{*p1 = cls->GetKeyboardRotVelocity(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetKeyboardRotVelocity_s, float)	{cls->SetKeyboardRotVelocity(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraObjectDistance_s, double*)	{ *p1 = cls->GetCameraObjectDistance(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraObjectDistance_s, double)	{ cls->SetCameraObjectDistance(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraLiftupAngle_s, double*)	{ *p1 = cls->GetCameraLiftupAngle(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraLiftupAngle_s, double)	{ cls->SetCameraLiftupAngle(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraRotX_s, double*)	{ *p1 = cls->GetCameraRotX(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraRotX_s, double)	{ cls->SetCameraRotX(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraRotY_s, double*)	{ *p1 = cls->GetCameraRotY(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraRotY_s, double)	{ cls->SetCameraRotY(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraRotZ_s, double*)	{ *p1 = cls->GetCameraRotZ(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraRotZ_s, double)	{ cls->SetCameraRotZ(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetLookAtShiftY_s, double*)	{ *p1 = cls->GetLookAtShiftY(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetLookAtShiftY_s, double)	{ cls->SetLookAtShiftY(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetEnableKeyboard_s, bool*)	{*p1 = cls->GetEnableKeyboard(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableKeyboard_s, bool)	{cls->EnableKeyboard(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetEnableMouseLeftButton_s, bool*)	{*p1 = cls->GetEnableMouseLeftButton(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableMouseLeftButton_s, bool)	{cls->EnableMouseLeftButton(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetEnableMouseRightButton_s, bool*)	{*p1 = cls->GetEnableMouseRightButton(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableMouseRightButton_s, bool)	{cls->EnableMouseRightButton(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetEnableMouseWheel_s, bool*)	{*p1 = cls->GetEnableMouseWheel(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableMouseWheel_s, bool)	{cls->EnableMouseWheel(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetPhysicsGroupMask_s, DWORD*)	{*p1 = cls->GetPhysicsGroupMask(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetPhysicsGroupMask_s, DWORD)	{cls->SetPhysicsGroupMask(p1); return S_OK;}
	
	ATTRIBUTE_METHOD1(CAutoCamera, IsEnableMouseRightDrag_s, bool*)	{*p1 = cls->IsEnableMouseRightDrag(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableMouseRightDrag_s, bool)	{cls->EnableMouseRightDrag(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsEnableMouseLeftDrag_s, bool*)	{*p1 = cls->IsEnableMouseLeftDrag(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableMouseLeftDrag_s, bool)	{cls->EnableMouseLeftDrag(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsLockMouseWhenDragging_s, bool*) { *p1 = cls->IsLockMouseWhenDragging(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetLockMouseWhenDragging_s, bool) { cls->SetLockMouseWhenDragging(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, IsUseCharacterLookup_s, bool*)	{*p1 = cls->IsUseCharacterLookup(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetUseCharacterLookup_s, bool)	{cls->SetUseCharacterLookup(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetAlwaysRotateCameraWhenFPS_s, bool*)	{*p1 = cls->GetAlwaysRotateCameraWhenFPS(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, EnableAlwaysRotateCameraWhenFPS_s, bool)	{cls->EnableAlwaysRotateCameraWhenFPS(p1); return S_OK;}


	ATTRIBUTE_METHOD1(CAutoCamera, IsUseCharacterLookupWhenMounted_s, bool*)	{*p1 = cls->IsUseCharacterLookupWhenMounted(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetUseCharacterLookupWhenMounted_s, bool)	{cls->SetUseCharacterLookupWhenMounted(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetCharacterLookupBoneIndex_s, int*)	{*p1 = cls->GetCharacterLookupBoneIndex(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetCharacterLookupBoneIndex_s, int)	{cls->SetCharacterLookupBoneIndex(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsCameraMoved_s, bool*)	{*p1 = cls->IsCameraMoved(); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsBlockInput_s, bool*)	{*p1 = cls->IsBlockInput(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetBlockInput_s, bool)	{cls->SetBlockInput(p1); return S_OK;}
	
	ATTRIBUTE_METHOD1(CAutoCamera, GetMaxCameraObjectDistance_s, double*)	{*p1 = cls->GetMaxCameraObjectDistance(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetMaxCameraObjectDistance_s, double)	{ cls->SetMaxCameraObjectDistance(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetMinCameraObjectDistance_s, double*)	{ *p1 = cls->GetMinCameraObjectDistance(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetMinCameraObjectDistance_s, double)	{ cls->SetMinCameraObjectDistance(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraRollbackSpeed_s, float*)	{ *p1 = cls->GetCameraRollbackSpeed(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraRollbackSpeed_s, float)	{ cls->SetCameraRollbackSpeed(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetCameraLookatOffset_s, Vector3*)	{*p1 = cls->GetCameraLookatOffset(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetCameraLookatOffset_s, Vector3)	{cls->SetCameraLookatOffset(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetAdditionalCameraRotate_s, Vector3*)	{ *p1 = cls->GetAdditionalCameraRotate(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetAdditionalCameraRotate_s, Vector3)	{ cls->SetAdditionalCameraRotate(p1); return S_OK; }
	
	ATTRIBUTE_METHOD1(CAutoCamera, GetMaxYShiftSpeed_s, float*)	{*p1 = cls->GetMaxYShiftSpeed(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetMaxYShiftSpeed_s, float)	{cls->SetMaxYShiftSpeed(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, GetMaxAllowedYShift_s, float*)	{*p1 = cls->GetMaxAllowedYShift(); return S_OK;}
	ATTRIBUTE_METHOD1(CAutoCamera, SetMaxAllowedYShift_s, float)	{cls->SetMaxAllowedYShift(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CAutoCamera, IsControlBiped_s, bool*)	{ *p1 = cls->IsControlBiped(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetControlBiped_s, bool)	{ cls->SetControlBiped(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, IsEnableBlockCollision_s, bool*)	{ *p1 = cls->IsEnableBlockCollision(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetEnableBlockCollision_s, bool)	{ cls->SetEnableBlockCollision(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, IsEnableTerrainCollision_s, bool*) { *p1 = cls->IsEnableTerrainCollision(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetEnableTerrainCollision_s, bool) { cls->SetEnableTerrainCollision(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, IsIgnoreEyeBlockCollisionInSunlight_s, bool*)	{ *p1 = cls->IsIgnoreEyeBlockCollisionInSunlight(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, SetIgnoreEyeBlockCollisionInSunlight_s, bool)	{ cls->SetIgnoreEyeBlockCollisionInSunlight(p1); return S_OK; }

	ATTRIBUTE_METHOD1(CAutoCamera, GetRenderOrigin_s, Vector3*)	{ *p1 = cls->GetRenderOrigin(); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, GetViewProjMatrix_s, Matrix4*)	{ *p1 = *(cls->GetViewProjMatrix()); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, GetViewMatrix_s, Matrix4*)	{ *p1 = *(cls->GetViewMatrix()); return S_OK; }
	ATTRIBUTE_METHOD1(CAutoCamera, GetProjMatrix_s, Matrix4*)	{ *p1 = *(cls->GetProjMatrix()); return S_OK; }
	
	ATTRIBUTE_METHOD(CAutoCamera, UpdateBipedFlyDir_s)	{ cls->UpdateBipedFlyDir(); return S_OK; }

	
	/** Get the current event binding object. this may return NULL. */
	CEventBinding * GetEventBinding();
	/** load the default camera key and mouse event binding.*/
	void LoadDefaultEventBinding();
	
	/// Call these from client and use Get*Matrix() to read new matrices
    virtual VOID FrameMove( FLOAT fElapsedTime );

	void UpdateBipedFlyDir(CBipedObject * pBiped = NULL);
	CBipedObject * GetTargetAsBiped();

	virtual LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt, const Vector3* up = NULL);
	
	void FollowBiped( CBaseObject* pBiped, CameraMode modeCamera, double dTransitionTime);
	void FollowBiped( CBaseObject* pBiped, int nTranstionMode, int modeCamera, double Param0, double Param1, double Param2);
	void SetCameraMode(CameraMode modeCamera);
	virtual CameraMode GetCameraMode();

	virtual void SetFollowTarget(CBaseObject* pObj){m_pTargetObject = pObj;};
	virtual CBaseObject* GetFollowTarget(){return m_pTargetObject;};

	/**
	* linearly (smoothly) move vPos to vPosTarget by the amount fIncrease
	* @return: return  true if we have reached the target.
	* if the distance between target and the pos is within fTolerance, result is set to target
	*/
	bool SmoothMove(DVector3* result, DVector3 vPosTarget, DVector3 vPos, float fIncrease, float fTolerance = 0);

	void UpdateViewProjMatrix();
	Matrix4* GetViewProjMatrix();
	/** compute view matrix by coordinate in world space. This will internally transform according to current render offset.
	* in case stereo vision is enabled, the eye shift distance is applied secretly here. This is tricky for generating stereo image.
	*/
	void ComputeViewMatrix(Matrix4 *pOut, const DVector3 *pEye, const DVector3 *pAt, const Vector3 *pUp);

	/** it calls ComputeViewMatrix() with the current eye settings. 
	* In most cases, this function is called automatically. However, when rendering stereo vision, one may need to call this function manually.*/
	void UpdateViewMatrix();

	/** Handle the user input from direct input: update the key state, process key event, process mouse event. */
	void HandleUserInput();

	/**
	* always running 
	* @return 
	*/
	bool IsAlwaysRun() {return  m_bAlwaysRun;}
	void SetAlwaysRun(bool bAlwaysRun) {m_bAlwaysRun = bAlwaysRun;}

	/** whether camera moved since last frame */
	bool IsCameraMoved();

	/** current render origin. */
	Vector3 GetRenderOrigin();

	/**
	* whether camera is locked behind the current character. 
	* @return 
	*/
	bool IsCamAlwaysBehindObject() {return  m_bIsCamAlwaysBehindObject;}
	void SetCamAlwaysBehindObject(bool bIsCamAlwaysBehindObject) {m_bIsCamAlwaysBehindObject = bIsCamAlwaysBehindObject;}

	/** whether the camera look up vector will use the character look up. Default to false, where the look up vector is always (0,1,0) */
	bool IsUseCharacterLookup();

	/** whether the camera look up vector will use the character look up. Default to false, where the look up vector is always (0,1,0) */
	void SetUseCharacterLookup(bool bUseCharacterLookup);

	/** Default to true. It will enable character lookup when biped is mounted. */
	bool IsUseCharacterLookupWhenMounted();

	/** Default to true. It will enable character lookup when biped is mounted. */
	void SetUseCharacterLookupWhenMounted(bool bUseCharacterLookup);

	/** -1 to disable, otherwise this is the attachment bone id to which we will bind the camera's lookup to. */
	int GetCharacterLookupBoneIndex();

	/** -1 to disable, otherwise this is the attachment bone id to which we will bind the camera's lookup to. */
	void SetCharacterLookupBoneIndex(int nIndex);

	/**
	* when set to true, the right mouse drag will change character facing when biped is standing still.
	*/
	bool IsUseRightButtonBipedFacing() {return  m_bUseRightButtonBipedFacing;}
	void SetUseRightButtonBipedFacing(bool bUseRightButtonBipedFacing) {m_bUseRightButtonBipedFacing = bUseRightButtonBipedFacing;}

	/**
	* when set to true, the right mouse drag will change character facing when biped is standing still.
	*/
	bool IsTurnBipedWhenWalkBackward() { return  m_bTurnBipedWhenWalkBackward; }
	void SetTurnBipedWhenWalkBackward(bool bTurnBipedWhenWalkBackward) { m_bTurnBipedWhenWalkBackward = bTurnBipedWhenWalkBackward; }

	/** whether camera dragging with right button if enabled. default to true */
	bool IsEnableMouseRightDrag() {return  m_bEnableMouseRightDrag;}

	/** set whether camera dragging with right button if enabled. default to true */
	void EnableMouseRightDrag(bool bEnabled) {m_bEnableMouseRightDrag = bEnabled;}

	/** whether camera dragging with left button if enabled. default to true */
	bool IsEnableMouseLeftDrag() {return  m_bEnableMouseLeftDrag;}

	/** set whether camera dragging with left button if enabled. default to true */
	void EnableMouseLeftDrag(bool bEnabled) {m_bEnableMouseLeftDrag = bEnabled;}
	
	/** whether to lock mouse during dragging, default to false. On FPS mode, one should enable or manually lock the mouse. */
	bool IsLockMouseWhenDragging() const;
	void SetLockMouseWhenDragging(bool val);

	/**
	* reset camera, without playing transitions.
	*/
	void Reset();
	/**
	* switch to free camera mode
	*/
	void FreeCameraMode();
	/**
	* switch to follow mode.
	*/
	void FollowMode();

	float GetKeyboardMovVelocity();
	/**
	* max speed of movement velocity using keyboard. this is used in free camera mode.
	* @param fValue 
	*/
	void SetKeyboardMovVelocity(float fValue);


	float GetKeyboardRotVelocity();
	/**
	* max speed of rotational velocity using keyboard. this is used in free camera mode.
	* @param fValue 
	*/
	void SetKeyboardRotVelocity(float fValue);

	/** whether to enable stereo vision. When stereo vision is enabled via software mode, 
	* we will shift the eye position relative to the look at position by stereo shift distance, 
	* when computing the view matrix. 
	* @note: please see ComputeViewMatrix() for more information. */
	void EnableStereoVision(bool bEnable) {m_bEnableStereoVision = bEnable;};
	bool IsStereoVisionEnabled() {return m_bEnableStereoVision;};
	
	/** this is usually half of the Stereo eye separation distance. negative value is left eye, positive value is right eye.*/
	void SetStereoEyeShiftDistance(float fDist) {m_fStereoEyeShiftDistance = fDist;} 
	float GetStereoEyeShiftDistance() {return m_fStereoEyeShiftDistance;} 

	void SetStereoConvergenceOffset(float fDist) { m_fStereoConvergenceOffset = fDist; }
	float GetStereoConvergenceOffset() { return m_fStereoConvergenceOffset; }
	
	/** the camera object distance after all kinds of constraints are applied.*/
	void SetCameraObjectDistance(double fDist);
	double GetCameraObjectDistance() { return m_fCameraObjectDistance; }

	void SetForceOmniCameraObjectDistance(double fDist);
	void SetForceOmniCameraPitch(double fPitch);

	/** lift up angle of the camera. Camera Pitch */
	void SetCameraLiftupAngle(double fValue);
	double GetCameraLiftupAngle() {return m_fCameraLiftupAngle;} 

	/** rotation of the camera around the X axis, in the world coordinate.  Camera Pitch */
	void SetCameraRotX(double fValue);
	double GetCameraRotX() { return m_fCameraRotX; }

	/** rotation of the camera around the Y axis, in the world coordinate.  Camera Yaw */
	void SetCameraRotY(double fValue) { m_fCameraRotY = fValue; }
	double GetCameraRotY() { return m_fCameraRotY; }

	/** rotation of the camera around the Z axis, in the world coordinate.  Camera Roll */
	void SetCameraRotZ(double fValue) { m_fCameraRotZ = fValue; }
	double GetCameraRotZ() { return m_fCameraRotZ; }
	
	/** the Y shift of the camera look at position during follow mode.
	the biped's height will add this value to get the real look at position
	*/
	void SetLookAtShiftY(double fValue) { m_fLookAtShiftY = fValue; }
	double GetLookAtShiftY() { return m_fLookAtShiftY; }

	/** whether handler keyboard message */
	void EnableKeyboard(bool bValue); 
	bool GetEnableKeyboard() {return m_bEnableKeyboard;} 

	/** whether Enable Mouse Left Button */
	void EnableMouseLeftButton(bool bValue); 
	bool GetEnableMouseLeftButton() {return m_bEnableMouseLeftButton;} 

	/** whether Enable Mouse Right Button */
	void EnableMouseRightButton(bool bValue); 
	bool GetEnableMouseRightButton() {return m_bEnableMouseRightButton;} 

	/** Default to true. if the camera mode is CameraCameraFirstPerson, moving the mouse will rotate the camera, otherwise only rotate when right mousebutton is down. */
	void EnableAlwaysRotateCameraWhenFPS(bool bValue); 
	bool GetAlwaysRotateCameraWhenFPS(); 

	/** whether Enable Mouse wheel Button */
	void EnableMouseWheel(bool bValue) {m_bEnableMouseWheel = bValue;} 
	bool GetEnableMouseWheel() {return m_bEnableMouseWheel;} 

	/** set groups Mask used to filter physics objects, default to 0xffffffff*/
	void SetPhysicsGroupMask(DWORD dwValue) {m_dwPhysicsGroupMask = dwValue;} 

	/** get groups Mask used to filter physics objects, default to 0xffffffff*/
	DWORD GetPhysicsGroupMask() {return m_dwPhysicsGroupMask;} 

	/** whether to block all user input (both key and mouse). Input is usually blocked as a game logics, such as we do not want the user to control the player anymore. */
	bool IsBlockInput();
	void SetBlockInput(bool bBlockInput);

	/** max camera object distance when mouse wheel is used for zooming. */
	void SetMaxCameraObjectDistance(double fValue) { m_fMaxCameraObjectDistance = fValue; }
	double GetMaxCameraObjectDistance() { return m_fMaxCameraObjectDistance; }

	/** min camera object distance when mouse wheel is used for zooming. */
	void SetMinCameraObjectDistance(double fValue) { m_fMinCameraObjectDistance = fValue; }
	double GetMinCameraObjectDistance() { return m_fMinCameraObjectDistance; }

	/** the camera roll back speed in meters per second. if this value is larger than INFINITY, there will be on smooth animations. */
	void SetCameraRollbackSpeed(float fValue) { m_fCameraRollbackSpeed = fValue; } 
	float GetCameraRollbackSpeed() { return m_fCameraRollbackSpeed; } 

	/** additional camera offset in camera space.  */
	void SetCameraLookatOffset(const Vector3& v) { m_vLookAtOffset = v; } 
	Vector3 GetCameraLookatOffset() { return m_vLookAtOffset; } 

	/** additional camera rotation in camera space. default to 0 */
	Vector3 GetAdditionalCameraRotate() const { return m_vAdditionalCameraRotate; }
	void SetAdditionalCameraRotate(const Vector3& val) { m_vAdditionalCameraRotate = val; }

	/** we will smooth the camera movement on the y direction when the followed biped's y movement is smaller than this value. */
	float GetMaxAllowedYShift();
	void SetMaxAllowedYShift(float fValue);

	/** the max speed camera interpolation speed along y axis. this is actually MaxAllowedYShift*MaxYShiftSpeed.  */
	float GetMaxYShiftSpeed();
	void SetMaxYShiftSpeed(float fValue);

	/** whether first person view mode*/
	bool IsFirstPersonView();

	/** whether the camera is able to control the biped movement.*/
	bool IsControlBiped() const;
	/** whether the camera is able to control the biped movement.*/
	void SetControlBiped(bool val);

	/** called every frame move when this character is sentient.
	* This is most likely used by active AI controllers, such as  movie controller. */
	virtual int On_FrameMove(float fElapsedTime);

	/** enabled by default. otherwise camera will pass through blocks*/
	bool IsEnableBlockCollision() const;
	void SetEnableBlockCollision(bool val);

	/** enabled by default. otherwise camera will pass through global terrain*/
	bool IsEnableTerrainCollision() const;
	void SetEnableTerrainCollision(bool val);

	/** enabled by default. if enabled, we will ignore camera-block collision,
	* when both eye and the look-at point are in sunlight.
	*/
	bool IsIgnoreEyeBlockCollisionInSunlight() const { return m_bIgnoreEyeBlockCollisionInSunlight; }
	void SetIgnoreEyeBlockCollisionInSunlight(bool val) { m_bIgnoreEyeBlockCollisionInSunlight = val; }

	virtual Vector3 GetRenderEyePosition();

private:
	/** Set the key down state according to bIsKeyDown
	*@param nKey: the camera key to set. 
	*@param bIsKeyDown: whether the key is being pressed when the function is called. 
	*/
	void SetKeyDownState(CharacterAndCameraKeys nKey, bool bIsKeyDown);

	void SetDefaultAngles();

	/* as if no key button is pressed. used before disable key board*/
	void ClearKeyStates();

	/* as if no mouse button is down. used before disable mouse */
	void ClearMouseStates();

	void SetMouseDragLock(bool bLock);
	
public:
	/// true to follow target
	BOOL				m_bIsFollowMode;			
	/// true if the camera is changed since last frame
	bool				m_bIsLastFrameChanged;	

private: 
	Matrix4 m_matViewProj;
	bool m_bEnableKeyboard;
	bool m_bEnableMouseLeftButton;
	bool m_bEnableMouseRightButton;
	bool m_bEnableMouseWheel;

	/// During follow mode, this is the biped that we should follow.
	CBaseObject*		m_pTargetObject;			
	/// When it's not in mode transition, this represents the current Camera mode
	CameraMode			m_currentCameraMode;		
	
	/// the CameraMode that should be replace by the target mode.
	CameraMode			m_lastCameraMode;			
	/// --Count down: the amount of time left in seconds, to transit from 
	/// the old camera mode to the target one.
	double				m_dTransitionAmt;			


	/// last eye position, just for handling smooth transition in follow mode
	DVector3			m_vEyeLast;					
	/// speed of camera eye movement
	FLOAT				m_fEyeSpeed;				
	/// last look at position, just for handling smooth transition in follow mode
	DVector3			m_vLookAtLast;				
	/// this should be same as eye pos except that when in stereo mode, it is either left or right eye pos. 
	DVector3			m_vRenderEyePos;
	/// speed of camera look at  movement
	FLOAT				m_fLookAtSpeed;				
	/// last look up position, just for handling smooth transition in follow mode
	Vector3			m_vLookUpLast;				
	/// speed of camera look at  movement
	FLOAT				m_fLookUpSpeed;				

	/*parameter about each of the camera mode*/
	/// should be less than the height of the sky box.
	FLOAT				m_fMaximumHeight;	
	/// should be above the floor.
	FLOAT				m_fMinimumHeight;	
	/// rad per second
	FLOAT				m_fKeyboardRotVelocity; 
	/// rad per second
	FLOAT				m_fKeyboardMovVelocity; 
	///-- CameraFollowFirstPerson,		
	/// First person view of the Target object
	/// the distance from the camera eye to the object being followed.
	double				m_fCameraObjectDistance;
	/** max camera object distance when mouse wheel is used for zooming. */
	double				m_fMaxCameraObjectDistance;
	/** min camera object distance when mouse wheel is used for zooming. */
	double				m_fMinCameraObjectDistance;
	/** the last camera object distance after all kinds of constraints are applied.*/
	double				m_fLastCameraObjectDistance;
	/// lift up angle of the camera. This is camera pitch
	double				m_fCameraLiftupAngle;
	/// rotation of the camera around the X axis that is added to m_fCameraLiftupAngle, in the world coordinate. This is camera pitch
	double				m_fCameraRotX;
	/// rotation of the camera around the Y axis, in the world coordinate. This is camera Yaw
	double				m_fCameraRotY;
	/// rotation of the camera around the Z (depth) axis. This is camera roll
	double				m_fCameraRotZ;
	/// set whether the camera will always behind the object.
	bool				m_bIsCamAlwaysBehindObject;
	/// the Y shift of the camera look at position during follow mode.
	/// the biped's height will add this value to get the real look at position
	double m_fLookAtShiftY;

	/** camera zoom in/out speed. */
	float m_fCamZoomSpeed;

	/** whether the camera is able to control the biped movement.*/
	bool				m_bControlBiped;
	
	/** always the biped under control will always be running, once started. */
	bool				m_bAlwaysRun;
	///-- CameraCameraFirstPerson 		
	/// Camera as a first person. 
	CameraConstraint	m_constrants;

	CEventBinding *m_event;
	/** the camera roll back speed in meters per second. if this value is larger than INFINITY, there will be on smooth animations. */
	float m_fCameraRollbackSpeed;
	/** the number of frame moves in which the smooth roll back is not applied. default value is 0, in which case there is always smooth animation.
	* the frame move function will decrease this function by 1 until it is zero again. */
	int m_nForceNoRollbackFrames;

	/** whether to enable stereo vision. When stereo vision is enabled via software mode, 
	* we will shift the eye position relative to the look at position by stereo shift distance, 
	* when computing the view matrix. */
	bool m_bEnableStereoVision;

	/** this is usually half of the Stereo eye separation distance. negative value is left eye, positive value is right eye.*/
	float m_fStereoEyeShiftDistance;
	/** offset along the eye look at position. this will make the stereo scene pop out of paralex convergence plane */
	float m_fStereoConvergenceOffset;

	// the pixel mouse drag distance from a mouse down and mouse move
	int  m_nMouseDragDistance;    

	/** groups Mask used to filter physics objects, default to 0xffffffff*/
	DWORD m_dwPhysicsGroupMask;

	/** when set to true, the right mouse drag will change character facing when biped is standing still. */
	bool m_bUseRightButtonBipedFacing;

	/** if true, walking backward will actually turn the biped and walk forward*/
	bool m_bTurnBipedWhenWalkBackward;

	/** camera dragging with left button if enabled. */
	bool m_bEnableMouseLeftDrag;

	/** camera dragging with right button if enabled. */
	bool m_bEnableMouseRightDrag;

	/** whether the camera look up vector will use the character look up. Default to false, where the look up vector is always (0,1,0) */
	bool m_bUseCharacterLookup;

	/** Default to true. It will enable character lookup when biped is mounted. */
	bool m_bUseCharacterLookupWhenMounted;

	/** -1 to disable, otherwise this is the attachment bone id to which we will bind the camera's lookup to. */
	int m_nCharacterLookupBoneIndex;

	/** Default to true. if the camera mode is CameraCameraFirstPerson, moving the mouse will rotate the camera, otherwise only rotate when right mousebutton is down. */
	bool m_bAlwaysRotateCameraWhenFPS;

	/** whether to block all user input (both key and mouse). Input is usually blocked as a game logics, such as we do not want the user to control the player anymore. */
	bool m_bBlockInput;

	//whether to 
	bool m_resetFlyNormal;

	/** whether it is first person view. */
	bool m_bFirstPerson;

	/** enabled by default. otherwise camera will pass through blocks*/
	bool m_bEnableBlockCollision;

	/** enabled by default. otherwise camera will pass through global terrain */
	bool m_bEnableTerrainCollision;

	/** enabled by default. if enabled, we will ignore camera-block collision, 
	* when both eye and the look-at point are in sunlight. 
	*/
	bool m_bIgnoreEyeBlockCollisionInSunlight;

	/** whether to lock mouse during dragging, default to false. On FPS mode, one should enable or manually lock the mouse. */
	bool m_bLockMouseWhenDragging;
	
	Vector3 m_bipedFlyNormal;

	/** additional camera offset in camera space. default to 0 */
	Vector3 m_vLookAtOffset;
	/** additional camera rotation in camera space. default to 0 */
	Vector3 m_vAdditionalCameraRotate;
	
	/** the camera will not move. */
	float m_fAllowedCharYShift;
	float m_fMaxYShiftSpeed;
	double m_fLastCharY;
	double m_fLastUsedCharY;

	double				m_fForceOmniCameraObjectDistance;
	double				m_fForceOmniCameraPitch;
public:
	friend CSceneObject;
	friend ShadowVolume;
};

}