#pragma  once

#include "TileObject.h"
#include "CadModel.h"

namespace ParaEngine
{
	class CadModel;

	class CadModelNode:public CTileObject
	{
	public:
		CadModelNode();
		virtual ~CadModelNode();
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::CadObject;}

	public:
		virtual void SetPosition(const DVector3 & v);

		virtual void Rotate(float x,float y, float z);

		virtual void SetScaling(float scale);

		virtual float GetScaling();

		virtual void Reset();

		virtual void SetRotation(const Quaternion& quat);

		virtual VOID GetRotation(Quaternion* quat);

		virtual bool ViewTouch();

		void Cleanup();


		HRESULT InitObject(CadModel* pModel);

		virtual HRESULT Draw(SceneState *sceneState);

		virtual Matrix4* GetRenderMatrix(Matrix4& out, int nRenderNumber=0);

		virtual bool IsTransparent(){return m_bIsTransparent;};

		virtual void SetTransparent(bool bIsTransparent){m_bIsTransparent = bIsTransparent;};

		void SetLocalTransform(const Matrix4& matrix);

		void GetLocalTransform(Matrix4* matrix);

		void SetLocalTransform(float scale, float rotX, float rotY, float rotZ);
		/** set local transform by first uniform scale, then rotate using a quaternion. */
		void SetLocalTransform(float scale, const Quaternion& quat);
		/** get local transform*/
		void GetLocalTransform(float* scale, Quaternion* quat);

		bool SetParamsFromAsset();

	private:
		asset_ptr<CadModel> m_pModel;
		
		Matrix4 m_mxLocalTransform;
		bool m_bViewClippingUpdated;
		bool m_bIsTransparent;

	};
}