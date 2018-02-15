
#include "DxStdAfx.h"

namespace ParaEngine
{

D3DXVECTOR3 * D3DXVec3Bezier(		D3DXVECTOR3 *pOut,
							 CONST D3DXVECTOR3 *pV1,
							 CONST D3DXVECTOR3 *pT1,
							 CONST D3DXVECTOR3 *pV2,
							 CONST D3DXVECTOR3 *pT2,
							 FLOAT step )
{
	D3DXVECTOR3 v1t1,t1v1,v2t2,v1t1v2,t1v2t2;
	D3DXVec3Lerp (&v1t1, pV1, pT1, step);
	D3DXVec3Lerp (&t1v1, pT1, pV2, step);
	D3DXVec3Lerp (&v2t2, pV2, pT2, step);
	D3DXVec3Lerp (&v1t1v2, &v1t1, &t1v1, step);
	D3DXVec3Lerp (&t1v2t2, &t1v1, &v2t2, step);
	D3DXVec3Lerp (pOut, &v1t1v2, &t1v2t2, step);

	return pOut;
}

bool MatrixResolve(const D3DXMATRIX* mat, D3DXVECTOR3* t, D3DXVECTOR3* r, D3DXVECTOR3* s)
{
	if(t!=NULL){
		// Trans
		t->x = mat->m[3][0];
		t->y = mat->m[3][1];
		t->z = mat->m[3][2];
		//r->w = 1.0F;
	}

	if(s!=NULL){
		// Scale
		float *n = &s->x;
		for( int i=0; i<3; i++ )
		{
			n[i] = sqrtf(mat->m[0][i]*mat->m[0][i] + mat->m[1][i]*mat->m[1][i] + mat->m[2][i]*mat->m[2][i]);
		}
		//r->w = 1.0F;
	}

	if(r!=NULL){
		float ca = sqrtf( mat->m[0][0]*mat->m[0][0] + mat->m[0][1]*mat->m[0][1] );
		r->y = atan2f( -mat->m[0][2], ca );
		if( ca == 0.0F ){
			r->x = 0.0F;
			r->z = atan2f( -mat->m[1][0], mat->m[1][1] );
		} else {
			r->x = atan2f( mat->m[1][2]/ca, mat->m[2][2]/ca );
			r->z = atan2f( mat->m[0][1]/ca, mat->m[0][0]/ca );
		}
		//r->w = 1.0F;
	}

	return true;
}
}