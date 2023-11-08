#pragma once
#include "TileObject.h"
#include "PortalFrustum.h"
#include "CameraFrustum.h"
#include "Framework/InputSystem/VirtualKey.h"
namespace ParaEngine
{
	class CBaseObject;
	class CShapeAABB;
	class CPortalFrustum;

	enum CameraMode {
		CameraFollow = 1,
		/// First person view of the Target object
		CameraFollowFirstPerson = 2,
		/// Third person view, allow rotation, character centered
		CameraFollowThirdPerson = 3,
		/// this value is used for the end of Camera follow modes.
		CameraFollowEnd = 4,
		/// @absoleted: Third person view, disable rotation, character restricted to a rectangular,
		/// if character is reaches the edge of this rect, camera will be centered on it again.
		CameraFollowDefault = 5,

		CameraCamera = 10,
		/// Camera as a first person. 
		CameraCameraFirstPerson = 11
	};


	//-----------------------------------------------------------------------------
	// Name: enum CharacterAndCameraKeys
	// Desc: used by CCamera to map WM_KEYDOWN keys
	//-----------------------------------------------------------------------------
	enum CharacterAndCameraKeys
	{
		MOVE_LEFT = 0,
		MOVE_RIGHT,
		MOVE_FORWARD,
		MOVE_BACKWARD,
		SHIFT_RIGHT,
		SHIFT_LEFT,
		CHARACTER_JUMP,
		CAM_LOCK,
		CAM_RESET,
		ZOOM_IN,
		ZOOM_OUT,
		FLY_DOWNWARD,
		MAX_KEYS,
		KEY_UNKNOWN = 0xFF
	};

#ifndef WM_MOUSEWHEEL  
#define WM_MOUSEWHEEL                   0x020A
#endif

#define KEY_WAS_DOWN_MASK 0x80
#define KEY_IS_DOWN_MASK  0x01

#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_MIDDLE_BUTTON 0x02
#define MOUSE_RIGHT_BUTTON  0x04
#define MOUSE_WHEEL         0x08

	//-----------------------------------------------------------------------------
	// Name: class CBaseCamera
	// Desc: Simple base camera class that moves and rotates.  The base class
	//       records mouse and keyboard input for use by a derived class, and 
	//       keeps common state.
	//-----------------------------------------------------------------------------
	class CBaseCamera : public CTileObject
	{
	public:
		CBaseCamera();
		// mapping from key id to key's direct input's scan code. 
		static EVirtualKey			  m_scancode_key_map[MAX_KEYS];
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CBaseCamera; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CBaseCamera"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CBaseCamera, GetNearPlane_s, float*) { *p1 = cls->GetNearPlane(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetNearPlane_s, float) { cls->SetNearPlane(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetFarPlane_s, float*) { *p1 = cls->GetFarPlane(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetFarPlane_s, float) { cls->SetFarPlane(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetFieldOfView_s, float*) { *p1 = cls->GetFieldOfView(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetFieldOfView_s, float) { cls->SetFieldOfView(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetAspectRatio_s, float*) { *p1 = cls->GetAspectRatio(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetAspectRatio_s, float) { cls->SetAspectRatio(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, IsPerspectiveView_s, bool*) { *p1 = cls->IsPerspectiveView(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetIsPerspectiveView_s, bool) { cls->SetIsPerspectiveView(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetOrthoWidth_s, float*) { *p1 = cls->GetOrthoWidth(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetOrthoWidth_s, float) { cls->SetOrthoWidth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetOrthoHeight_s, float*) { *p1 = cls->GetOrthoHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetOrthoHeight_s, float) { cls->SetOrthoHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetMoveScaler_s, float*) { *p1 = cls->GetMoveScaler(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetMoveScaler_s, float) { cls->SetMoveScaler(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetRotationScaler_s, float*) { *p1 = cls->GetRotationScaler(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetRotationScaler_s, float) { cls->SetRotationScaler(p1); return S_OK; }


		ATTRIBUTE_METHOD1(CBaseCamera, IsShiftMoveSwitched_s, bool*) { *p1 = cls->IsShiftMoveSwitched(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetShiftMoveSwitched_s, bool) { cls->SetShiftMoveSwitched(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, IsInvertPitch_s, bool*) { *p1 = cls->IsInvertPitch(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetInvertPitch_s, bool) { cls->SetInvertPitch(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetNumberOfFramesToSmoothMouseData_s, int*) { *p1 = cls->GetNumberOfFramesToSmoothMouseData(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetNumberOfFramesToSmoothMouseData_s, int) { cls->SetNumberOfFramesToSmoothMouseData(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetEyePosition_s, DVector3*) { *p1 = cls->GetEyePosition(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetEyePosition_s, DVector3) { cls->SetEyePosition(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetLookAtPosition_s, DVector3*) { *p1 = cls->GetLookAtPosition(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetLookAtPosition_s, DVector3) { cls->SetLookAtPosition(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetCameraUp_s, Vector3*) { *p1 = cls->GetCameraUp(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetCameraUp_s, Vector3) { cls->SetCameraUp(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetMovementDrag_s, bool*) { *p1 = cls->GetMovementDrag(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetMovementDrag_s, bool) { cls->SetMovementDrag(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseCamera, GetTotalDragTime_s, float*) { *p1 = cls->GetTotalDragTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseCamera, SetTotalDragTime_s, float) { cls->SetTotalDragTime(p1); return S_OK; }

		ATTRIBUTE_METHOD(CBaseCamera, FrameMove_s) { cls->FrameMove(0.f); return S_OK; }


		DEFINE_SCRIPT_EVENT(CBaseCamera, FrameMove);

		// Call these from client and use Get*Matrix() to read new matrices
		virtual LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void    FrameMove(FLOAT fElapsedTime);

		/** update the culling planes according to view and projection matrix on top of the global stack*/
		void UpdateFrustum();

		/** get the camera view frustum info */
		inline CCameraFrustum* GetFrustum() { return &m_frustum; };

		/** get the camera view frustum info */
		inline CPortalFrustum* GetPortalFrustum() { return &m_frustum_portal; };

		/** get the object view frustum info
		object view frustum is smaller than m_frustum; its far plane is set at fog end
		*/
		inline CCameraFrustum* GetObjectFrustum() { return &m_frustum_object; };

		/** get the shadow view frustum info
		* shadow view frustum is smaller than m_frustum_object; its far plane is set at a fixed size, such as the smaller of 50 and fog end
		*/
		inline CCameraFrustum* GetShadowFrustum() { return &m_frustum_shadow; };

		/** test again view frustum */
		bool  CanSeeObject(IViewClippingObject* pObj);

		/** test to see if any of the vertices are inside the fog plane */
		inline Plane& GetFogPlane() { return m_fog_plane; };

		/** Reset the camera's position back to the default*/
		virtual void Reset();
		virtual void SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt, const Vector3* up = NULL);
		void SetProjParams(FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane);

		virtual void SetFollowTarget(CBaseObject* pObj) {};
		virtual CBaseObject* GetFollowTarget() { return NULL; };
		virtual CameraMode GetCameraMode() { return CameraFollowFirstPerson; }

		/**
		* update the projection matrix using the current field of view and near and far plane values.
		* this function should be called whenever field of view, near and far plane values, or aspect ratio changes.
		* @param fAspect: the aspect ratio.
		*/
		void UpdateProjParams();
		void UpdateProjParams(FLOAT fAspect);

		// Functions to change behavior
		VOID SetInvertPitch(bool bInvertPitch);
		bool IsInvertPitch() { return m_bInvertPitch; }
		VOID SetDrag(bool bMovementDrag, FLOAT fTotalDragTimeToZero = 0.25f) { m_bMovementDrag = bMovementDrag; m_fTotalDragTimeToZero = fTotalDragTimeToZero; }
		VOID SetEnableYAxisMovement(bool bEnableYAxisMovement) { m_bEnableYAxisMovement = bEnableYAxisMovement; }
		VOID SetEnablePositionMovement(bool bEnablePositionMovement) { m_bEnablePositionMovement = bEnablePositionMovement; }
		VOID SetClipToBoundary(bool bClipToBoundary, Vector3* pvMinBoundary, Vector3* pvMaxBoundary) { m_bClipToBoundary = bClipToBoundary; if (pvMinBoundary) m_vMinBoundary = *pvMinBoundary; if (pvMaxBoundary) m_vMaxBoundary = *pvMaxBoundary; }
		VOID SetScalers(FLOAT fRotationScaler = 0.01f, FLOAT fMoveScaler = 5.0f) { m_fRotationScaler = fRotationScaler; m_fMoveScaler = fMoveScaler; }

		VOID SetRotationScaler(FLOAT fRotationScaler = 0.01f);
		float GetRotationScaler();
		VOID SetMoveScaler(FLOAT fMoveScaler = 5.0f);
		float GetMoveScaler();

		VOID SetNumberOfFramesToSmoothMouseData(int nFrames) { if (nFrames > 0) m_fFramesToSmoothMouseData = (float)nFrames; }
		int GetNumberOfFramesToSmoothMouseData() { return (int)m_fFramesToSmoothMouseData; }
		VOID SetResetCursorAfterMove(bool bResetCursorAfterMove) { m_bResetCursorAfterMove = bResetCursorAfterMove; }
		/** whether the AD and QE key are switched for character movement. */
		bool IsShiftMoveSwitched();
		/** whether the AD and QE key are switched for character movement. */
		void SetShiftMoveSwitched(bool bSwitched = false);

		// Functions to get state
		inline Matrix4*  GetViewMatrix() { return &m_mView; }
		inline Matrix4*  GetProjMatrix() { return &m_mProj; }
		inline bool IsBeingDragged() { return (m_bMouseLButtonDown || m_bMouseMButtonDown || m_bMouseRButtonDown); }
		inline bool IsMouseLButtonDown() { return m_bMouseLButtonDown; }
		inline bool IsMouseMButtonDown() { return m_bMouseMButtonDown; }
		inline bool IsMouseRButtonDown() { return m_bMouseRButtonDown; }

		void GetMouseRay(Vector3& vPickRayOrig, Vector3& vPickRayDir, POINT ptCursor, UINT nWidth, UINT nHeight, const Matrix4* pMatWorld);

		// Get camera parameters
		inline DVector3 GetEyePosition() { return m_vEye; };
		virtual void SetEyePosition(const DVector3& pos);
		inline DVector3 GetLookAtPosition() { return m_vLookAt; };
		virtual void SetLookAtPosition(const DVector3& pos);
		
		Vector3 GetCameraUp();
		virtual void SetCameraUp(const Vector3& pos);

		inline float GetFieldOfView() { return m_fFOV; };
		inline float GetAspectRatio() { return m_fAspect; };
		inline float GetNearPlane() { return m_fNearPlane; };
		inline float GetFarPlane() { return m_fFarPlane; };
		inline float GetOrthoHeight() { return m_fOrthoHeight; };
		inline float GetOrthoWidth() { return m_fOrthoWidth; };

		/** get  whether we use perspective view or orthogonal view */
		inline bool IsPerspectiveView() { return m_bIsPerspectiveView; }
		/** set whether we use perspective view or orthogonal view */
		void  SetIsPerspectiveView(bool bIsPerspectiveView);

		/** change the field of view . it will internally call UpdateProjParams()*/
		void SetFieldOfView(float fFieldofview);
		void SetFieldOfView(float fov_v,float fov_h);
		/** change the near plane. it will internally call UpdateProjParams()*/
		void SetNearPlane(float fDist);
		/** change the far plane. it will internally call UpdateProjParams()*/
		void SetFarPlane(float fDist);
		/** change the aspect ratio. it will internally call UpdateProjParams()*/
		void SetAspectRatio(float fAspect);
		/** change orthographic width of view volumn. it will internally call UpdateProjParams()*/
		void SetOrthoWidth(float fWidth);
		/** change orthographic height of view volumn. it will internally call UpdateProjParams()*/
		void SetOrthoHeight(float fHeight);

		/** copy only basic parameters like projection and camera transforms, etc.*/
		void CopyCameraParamsFrom(CBaseCamera* pFromCamera);

		// Functions to get state
		inline Vector3  GetWorldRight() { return Vector3(m_mCameraWorld._11, m_mCameraWorld._12, m_mCameraWorld._13); }
		inline Vector3  GetWorldUp() { return Vector3(m_mCameraWorld._21, m_mCameraWorld._22, m_mCameraWorld._23); }
		inline Vector3  GetWorldAhead() { return Vector3(m_mCameraWorld._31, m_mCameraWorld._32, m_mCameraWorld._33); }
		inline Vector3  GetEyePt() { return Vector3(m_mCameraWorld._41, m_mCameraWorld._42, m_mCameraWorld._43); }

		virtual Vector3 GetRenderEyePosition();
		// we can alter key map at runtime
		static void SetKeyMap(CharacterAndCameraKeys key, const EVirtualKey& scancode);
		// get scancode from key id
		static EVirtualKey GetKeyMap(CharacterAndCameraKeys key);

	protected:

		// Functions to map a WM_KEYDOWN key to a CharacterAndCameraKeysenum
		virtual CharacterAndCameraKeys MapKey(UINT nKey);
		BOOL IsKeyDown(BYTE key) { return((key & KEY_IS_DOWN_MASK) == KEY_IS_DOWN_MASK); }
		BOOL WasKeyDown(BYTE key) { return((key & KEY_WAS_DOWN_MASK) == KEY_WAS_DOWN_MASK); }

		/** Clamps pV to lie inside m_vMinBoundary & m_vMaxBoundary */
		void ConstrainToBoundary(DVector3* pV);
		void UpdateMouseDelta(float fElapsedTime);
		/**
		* @param nDX: the x delta in multiple of 120.
		* @param nDY: the y delta in multiple of 120.
		*/
		void UpdateMouseDelta(int nDX, int nDY);
		void UpdateVelocity(float fElapsedTime);


		/**
		* If true, then camera movement will slow to a stop otherwise movement is instant
		* @param bEnable
		*/
		void SetMovementDrag(bool bEnable);
		bool GetMovementDrag();

		/**
		* Time it takes for velocity to go from full to 0. default value is 0.25 seconds
		* @param fTime
		*/
		void SetTotalDragTime(float fTime);
		float GetTotalDragTime();

		/** camera view frustum */
		CCameraFrustum m_frustum;

		/** object view frustum, this is smaller than m_frustum; its far plane is set at fog end */
		CCameraFrustum m_frustum_object;

		/** shadow view frustum, this is smaller than m_frustum_object; its far plane is set at a fixed size, such as the smaller of 50 and fog end*/
		CCameraFrustum m_frustum_shadow;

		/** portal frustum with many planes formed by the portal vertices and the camera eye position.*/
		CPortalFrustum m_frustum_portal;

		/** this is the fog end plane, it is plane with normal of the eye vector and distance of the fog range
		* because fogs in ParaEngine is emulated linearly along the camera z axis, we need this to decide whether a shape is completely in fog or not.
		*/
		Plane		m_fog_plane;

		/**
		* @obsolete: the eye position is always added by the m_vEyeReferencePoint when changed
		* it is initialized to be (0,0,0). Call ChangeEyeReferencePoint() at any time
		* to change it. The effect will take place in the following frames
		*/
		DVector3			m_vEyeReferencePoint;

		Matrix4            m_mView;              // View matrix 
		Matrix4            m_mProj;              // Projection matrix

		BYTE                  m_aKeys[MAX_KEYS];  // State of input - KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK
		POINT                 m_ptLastMousePosition;  // Last absolute position of mouse cursor
		bool                  m_bMouseLButtonDown;    // True if left button is down 
		bool                  m_bMouseMButtonDown;    // True if middle button is down 
		bool                  m_bMouseRButtonDown;    // True if right button is down 
		int                   m_nCurrentButtonMask;   // mask of which buttons are down
		int                   m_nMouseWheelDelta;     // Amount of middle wheel scroll (+/-) 
		Vector2           m_vMouseDelta;          // Mouse relative delta smoothed over a few frames
		float                 m_fFramesToSmoothMouseData; // Number of frames to smooth mouse data over

		DVector3           m_vDefaultEye;          // Default camera eye position
		DVector3           m_vDefaultLookAt;       // Default LookAt position
		DVector3           m_vEye;                 // Camera eye position
		DVector3           m_vLookAt;              // LookAt position
		Vector3		   m_vUp;                  // Up vector of camera
		float                 m_fCameraYawAngle;      // Yaw angle of camera
		float                 m_fCameraPitchAngle;    // Pitch angle of camera

		Vector3           m_vVelocity;            // Velocity of camera
		bool                  m_bMovementDrag;        // If true, then camera movement will slow to a stop otherwise movement is instant
		Vector3           m_vVelocityDrag;        // Velocity drag force
		FLOAT                 m_fDragTimer;           // Countdown timer to apply drag
		FLOAT                 m_fTotalDragTimeToZero; // Time it takes for velocity to go from full to 0
		Vector2           m_vRotVelocity;         // Velocity of camera

		float                 m_fFOV;                 // Field of view
		float                 m_fAspect;              // Aspect ratio
		float                 m_fNearPlane;           // Near plane
		float                 m_fFarPlane;            // Far plane
		float				  m_fOrthoWidth;		  // width of the orthographic view volumn
		float				  m_fOrthoHeight;		  // height of the orthographic view volumn
		bool				  m_bIsPerspectiveView;	  // whether to use perspective view. 

		float                 m_fRotationScaler;      // Scaler for rotation
		float                 m_fMoveScaler;          // Scaler for movement

		bool                  m_bInvertPitch;         // Invert the pitch axis
		bool                  m_bEnablePositionMovement; // If true, then the user can translate the camera/model 
		bool                  m_bEnableYAxisMovement; // If true, then camera can move in the y-axis

		bool                  m_bClipToBoundary;      // If true, then the camera will be clipped to the boundary
		DVector3           m_vMinBoundary;         // Min point in clip boundary
		DVector3           m_vMaxBoundary;         // Max point in clip boundary

		bool                  m_bResetCursorAfterMove;// If true, the class will reset the cursor position so that the cursor always has space to move 

		Matrix4 m_mCameraWorld;       // World matrix of the camera (inverse of the view matrix)
	};

	//-----------------------------------------------------------------------------
	// Name: class CFirstPersonCamera
	// Desc: Simple first person camera class that moves and rotates.
	//       It allows yaw and pitch but not roll.  It uses WM_KEYDOWN and 
	//       GetCursorPos() to respond to keyboard and mouse input and updates the 
	//       view matrix based on input.  
	//-----------------------------------------------------------------------------
	class CFirstPersonCamera : public CBaseCamera
	{
	public:
		CFirstPersonCamera();

		// Call these from client and use Get*Matrix() to read new matrices
		virtual void FrameMove(float fElapsedTime);


	protected:
		//Matrix4 m_mCameraWorld;       // World matrix of the camera (inverse of the view matrix)

	};
}
