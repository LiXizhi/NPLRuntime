#pragma once
#include "BaseCamera.h"

namespace ParaEngine
{
	class CShapeSphere;
	/** Used for viewing objects on 3D canvas.
	*/
	class CCanvasCamera : public CBaseCamera
	{
	public:
		CCanvasCamera();
		~CCanvasCamera();

		void ComputeViewMatrix(Matrix4 *pOut, const DVector3 *pEye, const DVector3 *pAt, const Vector3 *pUp);

		virtual void FrameMove( float fElapsedTime );

		/**
		* reset the camera parameters to view the entire sphere at best (default) distance
		* @param sphere 
		*/
		void ZoomSphere(const CShapeSphere& sphere);

		/**
		* Zoom the camera
		* @param dAmount relative amount: such as 1.1, 0.9
		*/
		void Zoom(float dAmount);

		/**
		* rotate camera
		* @param dy relative amount in radian. 
		*/
		void Rotate(float dx, float dy, float dz);

		/**
		* pan the camera
		* @param dx relative amount in meters
		* @param dy relative amount in meters
		*/
		void Pan(float dx, float dy);


		/**
		* set the camera look at position
		*/
		void SetLookAtPos(float x, float y, float z);

		/**
		* set the camera eye position
		* @param fRotY rotation of the camera around the Y axis, in the world coordinate. 
		* @param fLiftupAngle lift up angle of the camera.
		* @param fCameraObjectDist the distance from the camera eye to the object being followed.
		*/
		void SetEyePosByAngle(float fRotY, float fLiftupAngle, float fCameraObjectDist);
		void GetEyePosByAngle(float* fRotY, float* fLiftupAngle, float* fCameraObjectDist);

		/**
		* only internally used. 
		* @param pvEyePt 
		* @param pvLookatPt 
		*/
		virtual void SetViewParams(const DVector3& vEyePt, const DVector3& vLookatPt);

	private:
		/// OBSOLETED: this is the object that we should view. 
		CBaseObject*		m_pTargetObject;


		/// should be less than the height of the sky box.
		FLOAT				m_fMaximumHeight;	
		/// should be above the floor.
		FLOAT				m_fMinimumHeight;

		/// the distance from the camera eye to the object being followed.
		double				m_fCameraObjectDistance;
		/// lift up angle of the camera.
		//FLOAT				m_fCameraLiftupAngle;
		/// rotation of the camera around the Y axis, in the world coordinate. 
		//FLOAT				m_fCameraRotY;
	};

}
