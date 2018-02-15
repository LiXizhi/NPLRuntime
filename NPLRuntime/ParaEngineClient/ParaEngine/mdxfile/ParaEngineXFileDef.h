#pragma once

namespace ParaEngine
{
struct ParaXMeshFace
{
    DWORD nFaceVertexIndices;
    DWORD faceVertexIndices[3];
};

struct ParaXTexUV
{
	DWORD nTextureCoords;
	D3DXVECTOR2* vertices;
public:
	ParaXTexUV()
	{
		vertices = NULL;
	}
	~ParaXTexUV()
	{
		if(vertices)
			delete [] vertices;
	}
};

struct ParaXMaterial
{
	D3DXCOLOR faceColor;
	FLOAT power;
    D3DXCOLOR specularColor;
    D3DXCOLOR emissiveColor;
	char * TextureFilename;
public:
	ParaXMaterial()
	{
		TextureFilename = NULL;
	}
	~ParaXMaterial()
	{
		if(TextureFilename)
			delete []TextureFilename;
	}
};

struct ParaXMeshMaterialList
{
	DWORD nMaterials;
    DWORD nFaceIndexes;
    DWORD* faceIndexes;
	vector <ParaXMaterial*> materials;
public:
	ParaXMeshMaterialList()
	{
		faceIndexes = NULL;
	}
	~ParaXMeshMaterialList()
	{
		if(faceIndexes)
			delete [] faceIndexes;

		{
			vector< ParaXMaterial* >::iterator itCurCP, itEndCP = materials.end();

			for( itCurCP = materials.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				delete (*itCurCP);
			}
			materials.clear();
		}
	}
};

struct ParaXMesh
{
	DWORD nVertices;
	D3DXVECTOR3* vertices;
	DWORD nFaces;
	ParaXMeshFace* faces;
public:
	ParaXMesh()
	{
		faces = NULL;
		vertices = NULL;
	}
	~ParaXMesh()
	{
		if(vertices)
			delete [] vertices;
		if(faces)
			delete [] faces;
	}
};

struct ParaXNorm
{
	DWORD nNormals;
    D3DXVECTOR3* normals;
    DWORD nFaces;
    ParaXMeshFace* faces;
public:
	ParaXNorm()
	{
		normals = NULL;
		faces = NULL;
	}
	~ParaXNorm()
	{
		if(normals)
			delete [] normals;
		if(faces)
			delete [] faces;
	}
};

struct ParaXSkinWeight
{
	char transformNodeName[80];
	int		transformNodeID;
	DWORD nWeights;
	DWORD* vertexIndices;
	FLOAT* weights;
	D3DXMATRIX matrixOffset;
public:
	ParaXSkinWeight()
	{
		vertexIndices = NULL;
		weights = NULL;
	}
	~ParaXSkinWeight()
	{
		if(vertexIndices)
			delete [] vertexIndices;
		if(weights)
			delete [] weights;
	}
};

struct FrameBind
{
// we allow each vertex to bind to at most BIND_BONE_NUM bones at a time
// if more than three bones are available, we will select the three with the highest weight
#define BIND_BONE_NUM	3

	int nFrameIndex;
	float fWeight;
};


struct ParaXGeoChunk
{
	ParaXMesh mesh; 
	ParaXNorm norm;
	ParaXTexUV uvs;
	int nParentFrame;
	ParaXMeshMaterialList matList;

public:
	list <ParaXSkinWeight*> listWeights;

	ParaXGeoChunk()
	{
		nParentFrame = 0;
	}
	~ParaXGeoChunk()
	{
		list< ParaXSkinWeight* >::iterator itCurCP, itEndCP = listWeights.end();

		for( itCurCP = listWeights.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		listWeights.clear();
	}
};
struct ParaXKeyFrame3
{
	int frameNum;
	int numKeySize;
	FLOAT key[3];
	bool comparekey(const ParaXKeyFrame3* key1)
	{
		if ((key[0] != key1->key[0]) ||
			(key[1] != key1->key[1]) ||
			(key[2] != key1->key[2]))
		{
			return false;
		}
		return true;
	}
};

struct ParaXKeyFrame4
{
	int frameNum;
	int numKeySize;
	FLOAT key[4];
	bool comparekey(const ParaXKeyFrame4* key1)
	{
		if ((key[0] != key1->key[0]) ||
			(key[1] != key1->key[1]) ||
			(key[2] != key1->key[2]) ||
			(key[3] != key1->key[3]) )
		{
			return false;
		}
		return true;
	}
};

struct ParaXFrame
{
	char name[80];
	D3DXMATRIX matrix;
	D3DXMATRIX			offsetMatrix; // for bones
	int	parentID;

	enum MotionType{
		KGTR=0,
		KGRT,
		KGSC,
		MAX_MOTION
	};
	//key frames count
	struct Animation
	{
		int    timeOffset;
		int					keyFrameSize[MAX_MOTION];
		void*				keyFrames[MAX_MOTION];
	public:
		Animation()
		{
			timeOffset = 0;
			for(int i=0;i<MAX_MOTION;i++)
			{
				keyFrameSize[i]=0;
				keyFrames[i]=NULL;
			}
		}

		~Animation()
		{
			for(int i=0;i<MAX_MOTION;i++)
			{
				if(keyFrames[i])
					delete [] keyFrames[i];
			}
		}
	};
	list <Animation*>	animations;
	
public:
	ParaXFrame()
	{
		D3DXMatrixIdentity(&offsetMatrix);
	}
	~ParaXFrame()
	{
		list< Animation* >::iterator itCurCP, itEndCP = animations.end();

		for( itCurCP = animations.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		animations.clear();
	}
};
struct ParaXAnim
{
	char* name;
	DWORD from;
	DWORD to;
	int nextAnim;
	FLOAT moveSpeed;
	D3DXVECTOR3 minEntent;
	D3DXVECTOR3 maxEntent;
	FLOAT boundsRadius;
public:
	ParaXAnim(){name = NULL;}
	~ParaXAnim(){
		if(name)
			delete [] name;
	}
};

/// this is intermediary structure for parsing Para X file.
struct ParaXFile
{
public:
	vector <ParaXGeoChunk*> geoChunks;
	vector <ParaXFrame*> frames;
	vector <ParaXAnim*> anims;
public:
	// return the index
	int GetFrameByName(const char * name)
	{
		vector< ParaXFrame* >::iterator itCurCP, itEndCP = frames.end();

		int i=0;
		for( itCurCP = frames.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			if( strcmp((*itCurCP)->name, name) == 0)
				return i;
			i++;
		}
		return -1;
	}

	ParaXFile()
	{
	}

	~ParaXFile()
	{
		{
			vector< ParaXGeoChunk* >::iterator itCurCP, itEndCP = geoChunks.end();

			for( itCurCP = geoChunks.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				delete (*itCurCP);
			}
			geoChunks.clear();
		}
		{
			vector< ParaXFrame* >::iterator itCurCP, itEndCP = frames.end();

			for( itCurCP = frames.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				delete (*itCurCP);
			}
			frames.clear();
		}
		{
			vector< ParaXAnim* >::iterator itCurCP, itEndCP = anims.end();

			for( itCurCP = anims.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				delete (*itCurCP);
			}
			anims.clear();
		}

	}
};
}