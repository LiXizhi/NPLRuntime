#pragma once

namespace ParaEngine
{
	struct SceneState;

	/** current billboard info is here. */
	struct BillBoardViewInfo
	{
	public:
		/// the rotation angle around the Y axis
		float		m_billboardYAngles;
		/// a rotation matrix to orient the billboard towards the camera ignoring Y.
		Matrix4		matBillboardMatrix;	
		/// a rotation matrix to orient the billboard towards the camera.
		Matrix4		matBillboardMatrixSphere;	

		/** The X component of the entity's yaw rotation */
		float rotationX;

		/** The combined X and Z components of the entity's pitch rotation */
		float rotationXZ;

		/** The Z component of the entity's yaw rotation */
		float rotationZ;

		/**
		* The Y component (scaled along the Z axis) of the entity's pitch rotation
		*/
		float rotationYZ;

		/**
		* The Y component (scaled along the X axis) of the entity's pitch rotation
		*/
		float rotationXY;

		Vector3 vLookAhead;
		Vector3 vLookUp;
		Vector3 vLookRight;
	public:
		void UpdateBillBoardInfo(SceneState* pSceneState);

		Vector3 TransformVertex(const Vector3& v);
		Vector3 TransformVertexWithoutY(const Vector3& v);
	};
}



