//-----------------------------------------------------------------------------
// Class:	CCanvasCamera
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.7.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ShapeSphere.h"

#include "CanvasCamera.h"

using namespace ParaEngine;

/** @def max lift up angle. */
#define MAX_LIFTUP_ANGLE	MATH_PI*0.4f

//////////////////////////////////////////////////////////////////////////
//
// CCanvasCamera
//
//////////////////////////////////////////////////////////////////////////

CCanvasCamera::CCanvasCamera()
:m_pTargetObject(NULL)
{
	m_fMinimumHeight = OBJ_UNIT*0.6f;
	m_fMaximumHeight = 15.f;
	ZoomSphere(CShapeSphere(Vector3(0,1,0), 1));

	// Setup the projection matrix
	SetProjParams( MATH_PI/6, 1.0f, 0.3f, 200.0f );
}
CCanvasCamera::~CCanvasCamera()
{

}

void CCanvasCamera::FrameMove( float fElapsedTime )
{
	Vector3 vWorldUp = Vector3( 0.0f, 1.0f, 0.0f );

	// get rotation Y
	Vector3 vCharFacing = Vector3( 0.0f, 0.0f, 1.0f );
	Matrix4 mat;
	Math::CameraRotMatrixYawPitch(mat, m_fCameraYawAngle, m_fCameraPitchAngle);
	ParaVec3TransformCoord(&vCharFacing, &vCharFacing, &mat);
	vCharFacing *= (float)m_fCameraObjectDistance;
	m_vEye = m_vLookAt-vCharFacing;

	/** should regenerate render origin whenever the camera moves.*/
	CGlobals::GetScene()->RegenerateRenderOrigin(m_vEye);
	/** Update the view matrix */
	ComputeViewMatrix(&m_mView, &(m_vEye), &(m_vLookAt), &vWorldUp);

	m_mCameraWorld = m_mView.inverse();
}

void CCanvasCamera::ComputeViewMatrix(Matrix4 *pOut, const DVector3 *pEye, const DVector3 *pAt, const Vector3 *pUp)
{
	/// Update the view matrix
	Vector3 vOffset = CGlobals::GetScene()->GetRenderOrigin();
	Vector3 eye = (*pEye - vOffset);
	Vector3 at = *pAt - vOffset;
	ParaMatrixLookAtLH( pOut, &eye, &(at), pUp );
}

void CCanvasCamera::SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt)
{
	/** set view */
	m_vDefaultEye = m_vEye = vEyePt;
	m_vDefaultLookAt = m_vLookAt = vLookatPt;

	/// Calculate the view matrix
	Vector3 vUp(0,1,0);
	/// Update the view matrix
	ComputeViewMatrix(&m_mView, &m_vEye, &m_vLookAt, &vUp);

	Matrix4 mInvView;
	mInvView = m_mView.inverse();

	// The axis basis vectors and camera position are stored inside the 
	// position matrix in the 4 rows of the camera's world matrix.
	// To figure out the yaw/pitch of the camera, we just need the Z basis vector
	Vector3* pZBasis = (Vector3*) &mInvView._31;

	m_fCameraYawAngle   = atan2f( pZBasis->x, pZBasis->z );
	float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
		
	m_fCameraPitchAngle = -atan2f( pZBasis->y, fLen );

	// this code is untested.
	m_fCameraObjectDistance = (m_vLookAt-m_vEye).length();

	CGlobals::GetScene()->RegenerateRenderOrigin(m_vEye);
}

void CCanvasCamera::ZoomSphere(const CShapeSphere& sphere)
{
	 Vector3 vLookAt = sphere.Center();
	 m_fCameraPitchAngle = MATH_PI/4;
	 m_fCameraObjectDistance = sphere.GetRadius() / sinf(GetFieldOfView()/2)/sinf(m_fCameraPitchAngle)+GetNearPlane();
	 m_fCameraYawAngle = 0;
	 m_vDefaultLookAt = m_vLookAt = vLookAt;
}

void CCanvasCamera::Zoom(float dAmount)
{
	m_fCameraObjectDistance *= dAmount;
	if(m_fCameraObjectDistance < GetNearPlane())
		m_fCameraObjectDistance = GetNearPlane();
}

void CCanvasCamera::Rotate(float dx, float dy, float dz)
{
	m_fCameraYawAngle += dy;
	m_fCameraYawAngle = Math::ToStandardAngle(m_fCameraYawAngle);

	m_fCameraPitchAngle += dx;
	if(m_fCameraPitchAngle>MAX_LIFTUP_ANGLE)
		m_fCameraPitchAngle = MAX_LIFTUP_ANGLE;
	else if(m_fCameraPitchAngle<-MAX_LIFTUP_ANGLE)
		m_fCameraPitchAngle = -MAX_LIFTUP_ANGLE;
}

void CCanvasCamera::Pan(float dx, float dy)
{
	m_vLookAt.y += dy;
	// TODO: how about dx?
}

void ParaEngine::CCanvasCamera::SetLookAtPos( float x, float y, float z )
{
	m_vDefaultLookAt = m_vLookAt = Vector3(x,y,z);
}

void ParaEngine::CCanvasCamera::SetEyePosByAngle( float fRotY, float fLiftupAngle, float fCameraObjectDist)
{
	m_fCameraYawAngle = fRotY;
	m_fCameraPitchAngle = fLiftupAngle;
	m_fCameraObjectDistance = fCameraObjectDist;
}

void CCanvasCamera::GetEyePosByAngle(float* fRotY, float* fLiftupAngle, float* fCameraObjectDist)
{
	*fRotY = m_fCameraYawAngle;
	*fLiftupAngle = m_fCameraPitchAngle;
	*fCameraObjectDist = (float)m_fCameraObjectDistance;
}