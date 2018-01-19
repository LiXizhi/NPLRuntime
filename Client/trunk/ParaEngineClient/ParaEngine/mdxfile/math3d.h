

#ifndef __MATH3D_H__
#define __MATH3D_H__
namespace ParaEngine
{
D3DXVECTOR3 * D3DXVec3Bezier(		D3DXVECTOR3 *pOut,
									CONST D3DXVECTOR3 *pV1,
									CONST D3DXVECTOR3 *pT1,
									CONST D3DXVECTOR3 *pV2,
									CONST D3DXVECTOR3 *pT2,
									FLOAT step );
bool MatrixResolve(const D3DXMATRIX* mat, D3DXVECTOR3* t, D3DXVECTOR3* r, D3DXVECTOR3* s);
}
#endif //__MATH3D_H__