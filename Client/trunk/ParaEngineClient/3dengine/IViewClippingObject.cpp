//-----------------------------------------------------------------------------
// Class:	IViewClippingObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.6
// Revised: 2010.7.28 splited CBaseObject class to this interface. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IBatchedElementDraw.h"
#include "util/StringHelper.h"
#include "VertexFVF.h"
#include "SceneObject.h"
#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "BaseCamera.h"

#include "IViewClippingObject.h"
#include "memdebug.h"

using namespace ParaEngine;

/** #def LINE_OCCLUSION_OBJECT or POINT_OCCLUSION_OBJECT to enable corresponding occlusion testing method */
#define LINE_OCCLUSION_OBJECT
//#define POINT_OCCLUSION_OBJECT

DVector3 IViewClippingObject::GetPosition()
{
	/// this is perhaps only for root scene
	float width,height,facing;
	GetBoundRect(&width,&height,&facing);
	return DVector3(width / 2, height / 2, 0);
}

DVector3 IViewClippingObject::GetObjectCenter()
{
	DVector3 v = GetPosition();
	v += GetLocalAABBCenter();
	return v;
}

void IViewClippingObject::SetObjectCenter(const DVector3 & v)
{
	DVector3 vPos = v;
	vPos.y -= GetHeight()/2;
	SetPosition(vPos);
}


/**
the occlusion object is defined in CBaseObject. However, I may use more simplified model to draw the object. Currently I used the line primitive
of the bounding box. Maybe Point primitive is enough(I tested it. it is not working well with point primitive even with fixed screen point size set.)
*/
void	IViewClippingObject::DrawOcclusionObject(SceneState * sceneState)
{
#ifdef LINE_OCCLUSION_OBJECT
	static const short pIndexBuffer[] = {
		0,1,1,2,2,3,3,0, // bottom
		4,5,5,6,6,7,7,4, // top
		0,4,1,5,2,6,3,7, // sides
	};

	Vector3 pVecBounds[8];
	int nNumVertices;
	GetRenderVertices(pVecBounds, &nNumVertices);
	OCCLUSION_VERTEX pVertices[8];
	for (int i=0;i<nNumVertices;i++)
	{
		pVertices[i].p = pVecBounds[i];
	}
	int nLenCount = 0;
	if(nNumVertices == 4)
		nLenCount = 4;
	else if(nNumVertices == 8)
		nLenCount = 12;
#ifdef USE_DIRECTX_RENDERER
	RenderDevice::DrawIndexedPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_LINELIST, 0, 
		nNumVertices, nLenCount, pIndexBuffer, D3DFMT_INDEX16,pVertices, sizeof(OCCLUSION_VERTEX));
#endif
	
#else 
#ifdef POINT_OCCLUSION_OBJECT
	/* TODO: one may need to turn on the following render state. 
	if(m_bEnableOcclusionQuery && d3dQuery)
	{
	float PointSize = 1.f;
	pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&PointSize));
	pd3dDevice->SetRenderState(D3DRS_POINTSIZE_MIN, *((DWORD*)&PointSize));
	pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, false);
	pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, false);
	}*/
	Vector3 pVecBounds[8];
	int nNumVertices;
	GetRenderVertices(pVecBounds, &nNumVertices);
	
#ifdef USE_DIRECTX_RENDERER
	RenderDevice::DrawPrimitiveUP(sceneState->m_pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_POINTLIST, nNumVertices, 
		pVecBounds, sizeof(OCCLUSION_VERTEX));
#endif
#endif
#endif
}

// just draw the bounding box
void IViewClippingObject::DebugDraw(IBatchedElementDraw* pBatchedDrawer)
{
	if(!pBatchedDrawer)
		return;
	Vector3 pVecBounds[8];
	int nNumVertices;
	GetRenderVertices(pVecBounds, &nNumVertices);
	if(nNumVertices==1)
	{
		// if it is a point, we will render it as a rectangular with four vertices.
		// with a sizes of 0.1 meters of the half size.
		const float fSize = 0.1f;
		nNumVertices = 4;
		pVecBounds[1] = pVecBounds[0]+Vector3(fSize,fSize, 0);
		pVecBounds[2] = pVecBounds[0]+Vector3(0,fSize*2, 0);
		pVecBounds[3] = pVecBounds[0]+Vector3(-fSize,fSize, 0);
	}
	int nLenCount = 0;
	if(nNumVertices == 4)
		nLenCount = 4;
	else if(nNumVertices == 8)
		nLenCount = 12;

	pBatchedDrawer->DrawPredefinedLines((const PARAVECTOR3*)pVecBounds, nLenCount, pBatchedDrawer->GetDefaultColor());
}

void IViewClippingObject::DrawBoundingBox(SceneState * sceneState,DWORD color)
{
	if(sceneState->GetBatchedElementDrawer() == 0)
		return;

	Vector3 pVecBounds[8];
	int nNumVertices;
	GetRenderVertices(pVecBounds, &nNumVertices);
	if(nNumVertices==1)
	{
		// if it is a point, we will render it as a rectangular with four vertices.
		// with a sizes of 0.1 meters of the half size.
		const float fSize = 0.1f;
		nNumVertices = 4;
		pVecBounds[1] = pVecBounds[0]+Vector3(fSize,fSize, 0);
		pVecBounds[2] = pVecBounds[0]+Vector3(0,fSize*2, 0);
		pVecBounds[3] = pVecBounds[0]+Vector3(-fSize,fSize, 0);
	}
	int nLenCount = 0;
	if(nNumVertices == 4)
		nLenCount = 4;
	else if(nNumVertices == 8)
		nLenCount = 12;

	LinearColor color_(color);
	PARAVECTOR3 line_color(color_.r,color_.g,color_.b);
	sceneState->GetBatchedElementDrawer()->DrawPredefinedLines((const PARAVECTOR3*)pVecBounds, nLenCount, line_color );
}



void IViewClippingObject::GetRenderVertices(Vector3 * pVertices, int* nNumber)
{
	Vector3 v = GetRenderOffset();
	GetVerticesWithOrigin(&v, pVertices, nNumber);
}

void IViewClippingObject::GetVertices(Vector3 * pVertices, int* nNumber)
{
	Vector3 vPos = GetPosition();
	GetVerticesWithOrigin(&vPos, pVertices, nNumber);
}

void IViewClippingObject::GetVerticesWithOrigin(const Vector3* vOrigin, Vector3 * pVertices, int* pNumber)
{
	Vector3 vPos = *vOrigin;
	int nNumber;

	float fRadius = GetRadius();
	if(fRadius == 0)
	{
		*pNumber = 1;
		pVertices[0] = vPos;
		return;
	}
	ObjectShape objShape = GetObjectShape();
	switch(objShape)
	{
	case _ObjectShape_Rectangular:
		{
			nNumber = 4;
			float fWidth, fHeight, fFacing;
			GetBoundRect(&fWidth, &fHeight, &fFacing);
			fWidth /= 2;
			fHeight /= 2;

			pVertices[0].x = vPos.x + fWidth;
			pVertices[0].y = vPos.y;
			pVertices[0].z = vPos.z + fHeight;

			pVertices[1].x = vPos.x - fWidth;
			pVertices[1].y = vPos.y;
			pVertices[1].z = vPos.z + fHeight;

			pVertices[2].x = vPos.x - fWidth;
			pVertices[2].y = vPos.y;
			pVertices[2].z = vPos.z - fHeight;

			pVertices[3].x = vPos.x + fWidth;
			pVertices[3].y = vPos.y;
			pVertices[3].z = vPos.z - fHeight;
			
			break;
		}
	case _ObjectShape_Circle:
		{
			nNumber = 4;
			
			pVertices[0].x = vPos.x + fRadius;
			pVertices[0].y = vPos.y;
			pVertices[0].z = vPos.z + fRadius;

			pVertices[1].x = vPos.x - fRadius;
			pVertices[1].y = vPos.y;
			pVertices[1].z = vPos.z + fRadius;

			pVertices[2].x = vPos.x - fRadius;
			pVertices[2].y = vPos.y;
			pVertices[2].z = vPos.z - fRadius;

			pVertices[3].x = vPos.x + fRadius;
			pVertices[3].y = vPos.y;
			pVertices[3].z = vPos.z - fRadius;
			break;
		}
	case _ObjectShape_Sphere:
		{
			nNumber = 8;
			
			pVertices[0].x = vPos.x + fRadius;
			pVertices[0].y = vPos.y;
			pVertices[0].z = vPos.z + fRadius;

			pVertices[1].x = vPos.x - fRadius;
			pVertices[1].y = vPos.y;
			pVertices[1].z = vPos.z + fRadius;

			pVertices[2].x = vPos.x - fRadius;
			pVertices[2].y = vPos.y;
			pVertices[2].z = vPos.z - fRadius;

			pVertices[3].x = vPos.x + fRadius;
			pVertices[3].y = vPos.y;
			pVertices[3].z = vPos.z - fRadius;

			float fDimeter = fRadius*2;
			pVertices[4].x = vPos.x + fRadius;
			pVertices[4].y = vPos.y + fDimeter;
			pVertices[4].z = vPos.z + fRadius;

			pVertices[5].x = vPos.x - fRadius;
			pVertices[5].y = vPos.y + fDimeter;
			pVertices[5].z = vPos.z + fRadius;

			pVertices[6].x = vPos.x - fRadius;
			pVertices[6].y = vPos.y + fDimeter;
			pVertices[6].z = vPos.z - fRadius;

			pVertices[7].x = vPos.x + fRadius;
			pVertices[7].y = vPos.y + fDimeter;
			pVertices[7].z = vPos.z - fRadius;
			break;
		}
	default:/* case _ObjectShape_Box: */
		{
			nNumber = 8;
			float fBX, fBY, fBZ, fFacing;
			GetBoundingBox(&fBX, &fBY, &fBZ, &fFacing);
			fBX /= 2;
			fBZ /= 2;

			pVertices[0].x = vPos.x + fBX;
			pVertices[0].y = vPos.y;
			pVertices[0].z = vPos.z + fBZ;

			pVertices[1].x = vPos.x - fBX;
			pVertices[1].y = vPos.y;
			pVertices[1].z = vPos.z + fBZ;

			pVertices[2].x = vPos.x - fBX;
			pVertices[2].y = vPos.y;
			pVertices[2].z = vPos.z - fBZ;

			pVertices[3].x = vPos.x + fBX;
			pVertices[3].y = vPos.y;
			pVertices[3].z = vPos.z - fBZ;

			pVertices[4].x = vPos.x + fBX;
			pVertices[4].y = vPos.y + fBY;
			pVertices[4].z = vPos.z + fBZ;

			pVertices[5].x = vPos.x - fBX;
			pVertices[5].y = vPos.y + fBY;
			pVertices[5].z = vPos.z + fBZ;

			pVertices[6].x = vPos.x - fBX;
			pVertices[6].y = vPos.y + fBY;
			pVertices[6].z = vPos.z - fBZ;

			pVertices[7].x = vPos.x + fBX;
			pVertices[7].y = vPos.y + fBY;
			pVertices[7].z = vPos.z - fBZ;
			break;
		}
	}
	*pNumber =  nNumber;
}

bool IViewClippingObject::TestShadowSweptSphere(CBaseCamera* pCamera, const Vector3* pvDirection)
{
	Vector3 vCenter = GetRenderOffset();
	vCenter += GetLocalAABBCenter();
	CShapeSphere sphere(vCenter, GetRadius());
	return pCamera->GetShadowFrustum()->TestSweptSphere(&sphere, pvDirection);
}

bool IViewClippingObject::TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance)
{
	Vector3 vCenter = GetRenderOffset();
	vCenter += GetLocalAABBCenter();
	float fRadius = GetRadius();

	CCameraFrustum* pFrustum = pCamera->GetObjectFrustum();
	float fToNearPlane = pFrustum->GetPlane(0).PlaneDotCoord(vCenter);
	if( (fToNearPlane+fRadius) >= 0.f && fToNearPlane < (fFarPlaneDistance+fRadius) )
	{
		return (pFrustum->GetPlane(2).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(3).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(4).PlaneDotCoord(vCenter) >= -fRadius)
			&& (pFrustum->GetPlane(5).PlaneDotCoord(vCenter) >= -fRadius);
	}
	else
		return false;
}

bool IViewClippingObject::TestCollision(CBaseCamera* pCamera)
{
	PE_ASSERT(pCamera!=0);
	Vector3 pVecBounds[8];
	int nNumber;
	GetRenderVertices(pVecBounds, &nNumber);
	return pCamera->GetObjectFrustum()->CullPointsWithFrustum(pVecBounds, nNumber);	
}

/**
*	- nMethod=0: The object is treated as its original shape.This may be slow.
*	- nMethod=1: both the incoming sphere and the object are treated as 2D circles. This is the fastest method, and is
*		useful for rough object-level culling, even for 3D scenes;
*	- nMethod=2: both the incoming sphere and the object are treated as 3D sphere. This is a all very method, and is
*		useful for rough object-level culling for 3D scenes;
*	- nMethod=3: 
*/
bool IViewClippingObject::TestCollisionSphere(const Vector3* pvCenter, FLOAT radius, BYTE nMethod /** = 1*/)
{
	bool bCollided = false;
	
	
	if(nMethod == 1)
	{
		Vector3 vCenter = GetPosition();
		/// only test in the y=0 plane
		Vector3 vPos = (*pvCenter) - vCenter;		// relative position
		FLOAT fDistSq = Vector2(vPos.x, vPos.z).squaredLength();
		/*if(fDist>FLT_TOLERANCE)
			fDist = sqrt(fDist);
		if(fDist <= (GetRadius()+radius))
			bCollided = true;
		else
			bCollided = false;*/

		float fRange = GetRadius()+radius;
		if(fDistSq <= (fRange*fRange))
			bCollided = true;
		else
			bCollided = false;
	}
	else
	{
		/// we will first do a sphere based rough test
		Vector3 vCenter = GetObjectCenter();
		Vector3 vPos = (*pvCenter) - vCenter;		// relative position
		FLOAT fDistSq = vPos.squaredLength();

		float fRange = GetRadius()+radius;
		if(fDistSq <= (fRange*fRange))
			bCollided = true;
		else
			bCollided = false;

		if((nMethod == 0) && bCollided)
		{
			ObjectShape objShape = GetObjectShape();
			switch(objShape)
			{
			case _ObjectShape_Circle:
			case _ObjectShape_Sphere:
				{
					// the rough test is enough.
					break;
				}
			case _ObjectShape_Box:
				// TODO: box based collision detection. Currently it defaults to rectangular
			case _ObjectShape_Rectangular:
				{
					// we will further test if the rough test returns true.
					// make transform
					FLOAT fWidth ,fHeight, fFacing;
					GetBoundRect(&fWidth ,&fHeight, &fFacing);
					Matrix4 m;
					ParaVec3TransformCoord( &vPos, &vPos, ParaMatrixRotationY( &m, -fFacing ) );

					fWidth /= 2;
					fHeight /= 2;
					if(radius != 0)
					{
						fWidth += radius;
						fHeight += radius;
					}
					//-- now test 
					// Note: I only tested x,z, I care not about the y component, which is related to object's height
					if(((-fWidth)>vPos.x) || (fWidth<vPos.x) || 
						((-fHeight)>vPos.z) || (fHeight<vPos.z))
						bCollided = false;
					else
						bCollided = true;
					break;
				}
			default:
				{
					bCollided = false;
					break;
				}
			}
		}//if((nMethod == 0) && bCollided)
	}
	
	return bCollided;
}

//------------------------------------------------------------------------------
// name: TestCollisionObject
/// desc: test if the object collide with this object in the world view. Currently
/// the pObj is regarded as a spherical/circular object. mostly it's a biped object. And this
/// object can have shape of any type.
//------------------------------------------------------------------------------
bool IViewClippingObject::TestCollisionObject(IViewClippingObject* pObj)
{
	Vector3 v =	pObj->GetPosition();
	return TestCollisionSphere(&v, pObj->GetRadius(),0);
}

//------------------------------------------------------------------------------
// name: GetSphereCollisionDepth
/// desc: this is similar to TestCollisionSphere, but does more than that. a depth value is returned
/// test if the object collide with this object in the world view. Currently
/// the pObj is regarded as a spherical/circular object. mostly it's a biped object. And this
/// object can have shape of any type.
/// 
/// pvCenter[in/out]:
/// bSolveDepth: true, the pvCenter will be changed to align to the nearest edge, if 
/// the original depth value is greater than 0. after alignment the value will be 0.
/// 
/// return the depth of collision, the large this value, the deeper the two object run into each other
/// if this value is smaller than or equal to 0, then no collision is detected
//------------------------------------------------------------------------------
FLOAT IViewClippingObject::GetSphereCollisionDepth(Vector3* pvCenter, FLOAT radius, bool bSolveDepth/* = false*/)
{
	FLOAT fDepth = 0;			// depth of collision

	// -- we will first do a sphere based rough test
	Vector3 vCenter = GetPosition();
	Vector3 vPos = (*pvCenter) - vCenter;		// relative position
	
	ObjectShape objShape = GetObjectShape();

	switch(objShape)
	{
	case _ObjectShape_Circle:
	case _ObjectShape_Sphere:
		{
			FLOAT fDist = vPos.squaredLength();
			if(fDist>0.00001f)
				fDist = sqrt(fDist);

			// Radius sum minus distance between centers of the two objects
			fDepth = (GetRadius()+radius) - fDist;

			break;
		}
	case _ObjectShape_Rectangular:
	case _ObjectShape_Box:
		{
			// we will further test if the rough test returns true.
			
			//-- make transform
			FLOAT fWidth ,fHeight, fFacing;
			GetBoundRect(&fWidth ,&fHeight, &fFacing);
			Matrix4 mRot;
			if(fFacing!=0.f)
			{
				ParaVec3TransformCoord( &vPos, &vPos, ParaMatrixRotationY( &mRot, -fFacing ) );
			}

			fWidth /= 2;
			fHeight /= 2;
			if(radius != 0)
			{
				fWidth += radius;
				fHeight += radius;
			}

			//-- now test 
			// Note: I only tested x,z, I care not about the y component, which is related to object's height
			FLOAT min1 = min( (vPos.x + fWidth),  (fWidth-vPos.x) );
			FLOAT min2 = min( (vPos.z + fHeight),  (fHeight-vPos.z) );
			fDepth = min(min1, min2);
		
			if(bSolveDepth && fDepth>0)
			{
				/*pvCenter will be changed to align to the nearest edge, if 
				the original depth value is greater than 0. after alignment 
				the new value will be 0.*/

				/* axis
				 ^ z
				 |
				 |
				 |------>x
				 */
				if(fDepth == min1)
				{
					if(min1 == (fWidth-vPos.x))
					{
						/* right edge */
						vPos.x = fWidth;
					}
					else
					{
						/* left edge */
						vPos.x = -fWidth;
					}
				}
				else
				{
					if(min2 ==(fHeight-vPos.z))
					{
						/* top edge */
						vPos.z = fHeight;
					}
					else
					{
						/* bottom edge */
						vPos.z = -fHeight;
					}
				}
				/* map back to world coordinates */
				if(fFacing!=0.f)
				{
					ParaVec3TransformCoord( &vPos, &vPos, ParaMatrixRotationY( &mRot, fFacing ) );
				}
				*pvCenter = vCenter + vPos;
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return fDepth;
}

//------------------------------------------------------------------------------
// name: GetObjectCollisionDepth
/// desc: test if the circle in the y=0 plane collide with this object in the world view
/// Note: radius can be zero. We assume that test is done in the y=0 plane only
//------------------------------------------------------------------------------
FLOAT IViewClippingObject::GetObjectCollisionDepth(IViewClippingObject* pObj)
{
	Vector3 v = pObj->GetPosition();
	return GetSphereCollisionDepth(&v, pObj->GetRadius(), false);
}

float IViewClippingObject::GetObjectToPointDistance(const Vector3* pPoint)
{
	Vector3 vCenter = GetObjectCenter();
	float fDist = ((vCenter - (*pPoint))).squaredLength();
	if(fDist>0.00001f)
		fDist = sqrt(fDist);
	return fDist - GetRadius();
}
//-----------------------------------------------------------------------------
// Name: TestCollisionRay()
/// Desc: Check the ray intersection with the object  
/// input:vPickRayOrig,vPickRayDir: mouse ray params
/// output: fDistance: the distance between the eye and the intersection point.
///      this is only set when it's intersection in the first place
/// return: true if they collide.
/// note: TODO: currently the caller should always be the biped class
//-----------------------------------------------------------------------------
bool IViewClippingObject::TestCollisionRay(const Vector3& vPickRayOrig, const Vector3& vPickRayDir_, FLOAT* fDistance)
{
	bool bCollided = false;

	Vector3 vPickRayDir = vPickRayDir_.normalisedCopy();
	
	// center of this object
	Vector3 vCenter = GetPosition();
	float radius = GetRadius();
	vCenter.y += radius;

	Vector3 tmpDiff = vPickRayOrig - vCenter;
	/**
	Let:a = Dx2 + Dy2 + Dz2; (for normalized rays, a=1)
		b = 2[ Dx(x0 每cx) + Dy(y0 每cy) + Dz(z0 每cz)]
		c = (x0 每cx)2 +(y0 每cy)2 +(z0 每cz)2-r2
	Then: t = (-b ㊣ sqrt(b2 每 4ac))/2a
		if (b2 每 4ac) < 0, the ray does not intersect the sphere;
		if (b2 每 4ac) = 0, the ray grazes the sphere;
		if (b2 每 4ac) > 0, the ray enters and passes through the sphere, exiting on the other side.
	Use the smallest positive t to find the closest visible ray/sphere intersection point x(t), y(t), z(t),
	*/
	float a, b, c;
	a = 1.0f;
	b = 2*(vPickRayDir.x*tmpDiff.x + 
		   vPickRayDir.y*tmpDiff.y + 
		   vPickRayDir.z*tmpDiff.z);
	c = tmpDiff.squaredLength() - radius*radius;

	float delta = b*b-4*a*c;
	if(delta>=0)
	{
		// collided
		bCollided = true;
		
		delta = sqrt(delta);
		*fDistance = min(abs((-b+delta)/2*a), abs((-b-delta)/2*a));
	}
	return bCollided;
}

Matrix4* ParaEngine::IViewClippingObject::GetWorldTransform(Matrix4& matOut, int nRenderNumber /*= 0*/)
{
	GetRenderMatrix(matOut);
	Vector3 vOffset = CGlobals::GetScene()->GetRenderOrigin();
	matOut._41 += vOffset.x;
	matOut._42 += vOffset.y;
	matOut._43 += vOffset.z;
	return &matOut;
}

Matrix4* ParaEngine::IViewClippingObject::GetRenderMatrix( Matrix4& out, int nRenderNumber)
{
	// render offset
	Vector3 vPos = GetRenderOffset();
	// get world transform matrix
	ParaMatrixRotationY(&out, GetFacing());/** set facing and rotate local matrix round y axis*/

	// world translation
	out._41 += vPos.x;
	out._42 += vPos.y;
	out._43 += vPos.z;
	return &out;
}

void ParaEngine::IViewClippingObject::SetScaling(float s)
{

}

float ParaEngine::IViewClippingObject::GetScaling()
{
	return 1.0f;
}

void ParaEngine::IViewClippingObject::SetTransform(Matrix4* pLocalTransform, const DVector3& pGlobalPos, float* pRotation)
{

}

void ParaEngine::IViewClippingObject::SetTransform( Matrix4* pWorldTransform )
{

}

void ParaEngine::IViewClippingObject::GetFacing3D( Vector3 *pV )
{
	Matrix4 m;
	float fFacing = GetFacing();
	if(fFacing != 0.f)
	{
		ParaVec3TransformCoord( pV, &Vector3::UNIT_X, ParaMatrixRotationY( &m, fFacing) );
	}
	else
	{
		*pV = Vector3::UNIT_X;
	}
}

void ParaEngine::IViewClippingObject::GetBoundRect( FLOAT* fWidth ,FLOAT* fHeight, FLOAT* fFacing )
{
	*fWidth = GetWidth();
	*fHeight = GetHeight();
	*fFacing = GetFacing();
}

void ParaEngine::IViewClippingObject::GetBoundingBox( float* fOBB_X, float* fOBB_Y, float* fOBB_Z, float* fFacing )
{
	*fOBB_X = GetWidth();
	*fOBB_Y = GetHeight();
	*fOBB_Z = GetDepth();
	*fFacing = GetFacing(); 
}

void IViewClippingObject::SetPosition(const DVector3 & v)
{

}

void IViewClippingObject::Rotate( float x, float y, float z )
{

}

void IViewClippingObject::SetRotation( const Quaternion& quat )
{

}

void IViewClippingObject::GetRotation(Quaternion* quat)
{
	quat->x = 0;
	quat->y = 0;
	quat->z = 0;
	quat->w = 1;
}

Vector3 IViewClippingObject::GetRenderOffset()
{
	DVector3 vPos = GetPosition();
	return (vPos-CGlobals::GetScene()->GetRenderOrigin());
}

void IViewClippingObject::GetOBB(CShapeOBB* obb)
{
	obb->mCenter = GetObjectCenter();
	float fBX, fBY, fBZ, fFacing;
	GetBoundingBox(&fBX, &fBY, &fBZ, &fFacing);
	obb->mExtents=Vector3(fBX/2, fBY/2, fBZ/2);
	Matrix3 rot(Matrix3::IDENTITY);
	rot.RotY(fFacing);
	obb->mRot = rot;
}

void IViewClippingObject::GetAABB(CShapeAABB* aabb)
{
	Vector3 vCenter = GetObjectCenter();
	float fBX, fBY, fBZ, fFacing;
	GetBoundingBox(&fBX, &fBY, &fBZ, &fFacing);
	aabb->SetCenterExtents(vCenter, Vector3(fBX/2, fBY/2, fBZ/2));
}

void ParaEngine::IViewClippingObject::SetAABB(const Vector3 *vMin, const Vector3 *vMax)
{

}

ParaEngine::Vector3 ParaEngine::IViewClippingObject::GetLocalAABBCenter()
{
	return Vector3(0, GetHeight()*0.5f, 0);
}

void ParaEngine::IViewClippingObject::SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing)
{

}
