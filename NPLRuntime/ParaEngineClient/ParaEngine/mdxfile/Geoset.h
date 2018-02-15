
#ifndef __GEOSET_H__
#define __GEOSET_H__

#include "utils.h"
#include "GeoChunk.h"
#include "ModelUtils.h"

#include <list>
using namespace std;

namespace ParaEngine
{
class CMDXModel;
class ShadowVolume;
struct LightParams;

#define MAX_CHUNK_NUM 10

/**
An X file model must contain only one geoset which is a collection of geochunks. A geochunk
is a mesh which can be hidden or unhidden by modifying g_showmeshes[MAX_CHUNK_NUM], before
render() is called.
*/
class CGeoset
{
public:
	CGeoset(void);
	~CGeoset(void);
	void Cleanup();
	
	void Read(TypePointer inP,int inSize);
	void Render(CMDXModel *model,const AnimInfo& animInfo);

	int GetNumChunks(){return (int)chunks.size();};
	CGeoChunk* AddNewChunk();

	void BuildShadowVolume(CMDXModel *model,const AnimInfo& animInfo,ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld);
private:
	list <CGeoChunk*> chunks;
};
}
#endif //__GEOSET_H__


