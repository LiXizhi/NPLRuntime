#pragma once

#ifndef EPSILON
#define EPSILON 0.00001f
#endif

#ifndef FLOAT_POS_INFINITY
#define FLOAT_POS_INFINITY 999999999.0f
#endif

namespace ParaEngine
{

	class CMath
	{
	public:
		CMath(void);
		~CMath(void);
	public:
		/** change src from src to dest, by a maximum of fMaxStep. If dest has been reached, return true; otherwise return false.*/
		static bool SmoothMoveFloat1(float& src, const float dest, const float fMaxStep);

		/** change src from src to dest, by a maximum of fMaxStep. If dest has been reached, return true; otherwise return false.*/
		static bool SmoothMoveAngle1(float& src, const float dest, const float fMaxStep);

		/**
		* linearly (smoothly) move vPos to vPosTarget by the amount fIncrease return true if we have reached the target. 
		* if the distance between target and the pos is within fTolerance, result is set to target
		*/
		static bool SmoothMoveVec3(Vector3* result, const Vector3& vPosTarget, const Vector3& vPos, FLOAT fIncrease, FLOAT fTolerance/*=0*/);

		/** make sure that the angle is in the range (-Pi,Pi]*/
		static float ToStandardAngle(float fAngle);
		
		/** check if the matchStr matches the string pattern in matchPattern
		* @param matchPattern: the match pattern. It may end with "*"
		*	in which case "*" will match any character(s). 
		*	e.g "Li*" will match both "LiXizhi" and "LiTC", whereas "Li" will only match "Li"
		* @param matchStr: the match string.
		* @return: return true if the two string matches. */
		static bool MatchString(const std::string& matchPattern, const std::string& matchStr);

		/** facing target in xz plane
		* @params fFacing: [out] it is in the range [0, 2PI]
		* return true if target!=source(a valid facing can be computed)
		*/
		static bool ComputeFacingTarget(const Vector3& target, const Vector3& source, FLOAT& fFacing);
		/**
		* get the closest integer near the specified float number.
		* E.g. Round(1.49)==1, Round(1.51)==2, ToInt(
		*/
		static inline int Round(float fValue);

		/// Returns MIN(x, y, z);
		static float MinVec3(const Vector3& v);
		/// Returns MAX(x, y, z);
		static float MaxVec3(const Vector3& v);

		/** return true if equal. T is a 3D vector, such as Vector3 */
		template <class T>
		static inline bool CompareXZ(const T& a, const T& b)
		{
			return (a.x==b.x && a.z==b.z);
		}

		/** return true if equal. T is a 3D vector, such as Vector3 */
		template <class T>
		static bool CompareXZ(const T& a, const T& b, float epsilon)
		{
			return (fabs(a.x-b.x) + fabs(a.z-b.z)) < epsilon;
		}

		/**
		* Get the scaling factor from globalMat.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
		* it is assumed that components of globalMat satisfies the following equation:
		* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
		* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
		* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
		* 
		* @param globalMat the input matrix
		* @param fScalingX [out] maybe NULL
		* @param fScalingY [out] maybe NULL
		* @param fScalingZ [out] maybe NULL
		*/
		static void GetMatrixScaling(const Matrix4& globalMat, float* fScalingX, float* fScalingY,float* fScalingZ);

		/** create a billboard matrix 
		* @param pOut: [out], the output billboard matrix
		* @param matModelview: the current model view matrix. if NULL, the current model and view matrix on the global stack is used
		* @param vBillboardPos: the billboard position in model space. if NULL, (0,0,0) is used. 
		* @param bAxisAligned: if true, it will create a billboard with Y (up) axis fixed. Default to false.
		* @return pOut is returned. 
		*/
		static Matrix4* CreateBillboardMatrix(Matrix4* pOut, const Matrix4* matModelview, const Vector3* vBillboardPos, bool bAxisAligned = false);

		// Make a rotation matrix based on the camera's yaw & pitch
		static void CameraRotMatrixYawPitch(Matrix4& out, float fYaw, float fPitch);
	};

	inline int Math::Round(float fValue)
	{
		return int(fValue > 0.0f ? fValue + 0.5f : fValue - 0.5f);
		/*if(fValue>=0)
			return (((fValue-(int)fValue)>0.5f)?((int)fValue+1):(int)fValue);
		else
			return -ToInt(-fValue);*/
	}
}
