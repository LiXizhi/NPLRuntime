#pragma once
namespace ParaEngine
{
#define D3D9T_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL|D3DFVF_DIFFUSE)

struct my_vertex{
    //FLOAT x, y, z;  
	D3DXVECTOR3	 v;
	//FLOAT nx,ny,nz;
	D3DXVECTOR3	 n;
    DWORD colour;
    FLOAT tu, tv;  
};
}