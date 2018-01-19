#ifndef __GEOCHUNCK_H__
#define __GEOCHUNCK_H__

#include "utils.h"
#include "ModelUtils.h"

namespace ParaEngine
{
struct AnimExtent{
	D3DXVECTOR3		MinimumExtent;
	D3DXVECTOR3		MaximumExtent;
	float			BoundsRadius;	
};

class CMDXModel;
class CGeoset;
struct ParaXGeoChunk;
class ShadowVolume;
struct LightParams;
/**
This is a mesh object. It contains vertices, faces, etc. The rendering code for meshes are also
implemented in this class. It reads the skin (bone) information, calculate its vertex data
for a specified character pose, and then render it. It is also responsible for building the 
shadow volumes of the associated mesh. It supports multi-layered rendering (multiple textures).
*/
class CGeoChunk
{
public:
	CGeoChunk(void);
	~CGeoChunk(void);

	void Read(TypePointer inP,int inSize);
	void Render(CMDXModel *model,const AnimInfo& animInfo);
	D3DXVECTOR3* GetMaximumExtent();
	D3DXVECTOR3* GetMinimumExtent();

private:
	void CalcGroupMatrix(CMDXModel *model);
	void BuildShadowVolume(CMDXModel *model,const AnimInfo& animInfo,ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld);

private:
	int					numVertices;
	D3DXVECTOR3*		vertices;
	D3DXVECTOR3*		normals;

	int					numTriangles;
	short*				triangles;

	char*				vertexGroups;

	D3DXVECTOR2*		uvs;

	int					numGroups;
	int*				groups;
	int					numMatrixGroups;
	int*				matrixGroups;

	D3DXMATRIX*			matrixes;
	float*				AnimAlphas;

	int					materialID;

	float				boundsRadius;
	D3DXVECTOR3			mins;
	D3DXVECTOR3			maxs;

	int					numAnimExtent;
	AnimExtent*			animExtents;
friend CMDXModel;
friend CGeoset;
};
}
#endif//__GEOCHUNCK_H__
