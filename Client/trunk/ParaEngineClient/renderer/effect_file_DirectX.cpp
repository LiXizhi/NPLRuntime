//-----------------------------------------------------------------------------
// Class:	CEffectFileDirectX
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.6.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "effect_file_DirectX.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "MirrorSurface.h"
#include "SkyMesh.h"
#include "AutoCamera.h"
#include "SunLight.h"
#ifdef WIN32
#define strcmpi		_strcmpi
#endif

using namespace ParaEngine;
using namespace std;
/**
* Comments by LiXizhi
* The DirectX control panel(in windows control panel) can toggle retail/debug version dll.
*	Only define DEBUG_VS| DEBUG_PS, if you are running with debug version dll. Otherwise the FX file will not be loaded on some machines.
* instructions to debug effect files:
* - Build using "DEBUG shaders" or define the following macros
* - Setting break points in fx files. 
* - Run the program by Debug->Direct3D->Start with Direct3D debugging
* - Run the game to the place, where sharder is used. Set the device to REF
* - now that the debugger will break at the break points
*/

/** Uncomment this line to debug vertex shaders */
//#define DEBUG_VS
/** Uncomment this line to debug pixel shaders */
//#define DEBUG_PS

/**
* if true, the shader will use half precision arithmetics,
* if false, float4 will be used.
*/
bool CEffectFileDirectX::s_bUseHalfPrecision = false;// half will be enough

#define LIGHT_BOOLEAN_BASE			0
/**@def max number of local lights.it can be up to 8 */
#define MAX_SHADER_LIGHT_NUM		4

bool CEffectFileDirectX::g_bTextureEnabled = true;

//
// These macro tables define the SCALAR data type used in the
// shaders to be either float or half precision values.
//
LPCSTR s_fullPrecisionMacroTable[]=
{
	"SCALAR", "float",
	"SCALAR2", "float2",
	"SCALAR3", "float3",
	"SCALAR4", "float4",
	"SCALAR2x2", "float2x2",
	"SCALAR3x3", "float3x3",
	"SCALAR4x4", "float4x4",
	0, 0,
};

LPCSTR s_halfPrecisionMacroTable[]=
{
	"SCALAR", "half",
	"SCALAR2", "half2",
	"SCALAR3", "half3",
	"SCALAR4", "half4",
	"SCALAR2x2", "half2x2",
	"SCALAR3x3", "half3x3",
	"SCALAR4x4", "half4x4",
	0, 0,
};


CEffectFileDirectX::CEffectFileDirectX(const char* filename)
:m_pEffect(0),m_bIsBegin(false),m_bSharedMode(false),m_nTechniqueIndex(0)
{
	m_filename = filename;
	memset(m_paramHandle, 0, sizeof(m_paramHandle));
}

CEffectFileDirectX::CEffectFileDirectX(const AssetKey& key)
:CEffectFileBase(key), m_pEffect(0), m_bIsBegin(false), m_bSharedMode(false), m_nTechniqueIndex(0)
{
	memset(m_paramHandle, 0, sizeof(m_paramHandle));
}

CEffectFileDirectX::~CEffectFileDirectX()
{
	PE_ASSERT(!m_pEffect);
}

void CEffectFileDirectX::EnableShareMode(bool bEnable)
{
	m_bSharedMode = bEnable;
}
bool CEffectFileDirectX::IsInShareMode()
{
	return m_bSharedMode;
}

void CEffectFileDirectX::EnableTextures(bool bEnable)
{
	g_bTextureEnabled = bEnable;
}
bool CEffectFileDirectX::AreTextureEnabled()
{
	return g_bTextureEnabled ;
}

bool CEffectFileDirectX::EnableEnvironmentMapping(bool bEnable)
{
	return SetBoolean(6, bEnable);
}

void CEffectFileDirectX::SetReflectFactor(float fFactor)
{
	if(CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
		setFloat(k_reflectFactor, fFactor);
}

bool CEffectFileDirectX::EnableReflectionMapping(bool bEnable, float fSurfaceHeight)
{
	if(CGlobals::GetEffectManager()->IsReflectionRenderingEnabled() && SetBoolean(5, bEnable))
	{
		if(bEnable)
		{
			CMirrorSurface* pMirorSurface =  CGlobals::GetScene()->GetMirrorSurface(0);
			if(pMirorSurface != 0)
			{
				// TODO: here I just assume the reflection surface is centered on the world origin of the current rendered object. 
				// TODO: if there are multiple surfaces, why not choose the one closest and above the camera eye? Currently it just choose the one rendered last
				
				DVector3 vPos = CGlobals::GetSceneState()->GetCurrentSceneObject()->GetPosition();
				vPos.y += fSurfaceHeight;
				pMirorSurface->SetPosition(vPos);
				// TODO: here I just assume texture sampler is on s1 register.
				setTexture(1, pMirorSurface->GetReflectionTexture());
			}
		}
		/*else
			setTexture(1, (TextureEntity*)NULL);*/
		return true;
	}
	return false;
}

void CEffectFileDirectX::EnableNormalMap(bool bEnable)
{
	// TODO:
}
void CEffectFileDirectX::EnableLightMap(bool bEnable)
{
	// TODO:
}

/************************************************************************/
/* In line Functions                                                    */
/************************************************************************/
//. Accessors ...................................................
LPD3DXEFFECT CEffectFileDirectX::effect()const
{
	return m_pEffect;
}

bool CEffectFileDirectX::isParameterUsed(eParameterHandles index)const
{
	return m_paramHandle[index] != 0;
}

bool CEffectFileDirectX::isMatrixUsed(eParameterHandles index)const
{
	return m_paramHandle[index] != 0;
}

bool CEffectFileDirectX::isTextureUsed(int index)const
{
	return m_paramHandle[k_tex0 + index] != 0;
}

bool CEffectFileDirectX::isTextureMatrixUsed(int index)const
{
	return m_paramHandle[k_tex_mat0 + index] != 0;
}




bool CEffectFileDirectX::setMatrixArray(eParameterHandles index, const Matrix4* data, UINT32 count)const
{
	if (m_pEffect && isMatrixUsed(index))
	{
		return SUCCEEDED(m_pEffect->SetMatrixArray(m_paramHandle[index], data->GetConstPointer(), count));
	}
	return false;
}

bool CEffectFileDirectX::setMatrixInArray(eParameterHandles index, UINT32 element, const Matrix4* data)const
{
	if (m_pEffect && isMatrixUsed(index))
	{
		D3DXHANDLE subHandle = m_pEffect->GetParameterElement(m_paramHandle[index], element);
		return SUCCEEDED(m_pEffect->SetMatrix(subHandle, data->GetConstPointer()));
	}
	return false;
}

bool CEffectFileDirectX::setVectorArray(eParameterHandles index,const Vector4* pVector,UINT count) const
{
	if (m_pEffect && isParameterUsed(index))
	{
		return SUCCEEDED(m_pEffect->SetVectorArray(m_paramHandle[index], (const DeviceVector4*)pVector, count));
	}
	return false;
}

bool CEffectFileDirectX::setFloatArray(eParameterHandles index, const float* data, UINT32 count)const
{
	if (m_pEffect && isParameterUsed(index))
	{
		return SUCCEEDED(m_pEffect->SetFloatArray(m_paramHandle[index], data, count));
	}
	return false;
}

bool CEffectFileDirectX::setFloatInArray(eParameterHandles index, UINT32 element, float data)const
{
	if (m_pEffect && isParameterUsed(index))
	{
		D3DXHANDLE subHandle = m_pEffect->GetParameterElement(m_paramHandle[index],element);
		return SUCCEEDED(m_pEffect->SetFloat(subHandle, data));
	}
	return false;
}

bool CEffectFileDirectX::setTextureMatrix(int index, const Matrix4* data)const
{
	if (m_pEffect && isTextureMatrixUsed(index))
	{
		return SUCCEEDED(m_pEffect->SetMatrix(m_paramHandle[k_tex_mat0+index], data->GetConstPointer()));
	}
	return false;
}

bool CEffectFileDirectX::setParameter(eParameterHandles index, const void* data, INT32 size)const
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result= SUCCEEDED(m_pEffect->SetValue(m_paramHandle[index], data, size));

		PE_ASSERT(result);
		return result;
	}
	return false;
}
bool CEffectFileDirectX::setBool(eParameterHandles index, BOOL bBoolean) const
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result= SUCCEEDED(m_pEffect->SetBool(m_paramHandle[index], bBoolean));
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileDirectX::setInt(eParameterHandles index, int nValue) const
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result= SUCCEEDED(m_pEffect->SetInt(m_paramHandle[index], nValue));
		PE_ASSERT(result);
		return result;
	}
	return false;
}

bool CEffectFileDirectX::setFloat(eParameterHandles index, float fValue) const
{
	if (m_pEffect && isParameterUsed(index))
	{
		bool result= SUCCEEDED(m_pEffect->SetFloat(m_paramHandle[index], fValue));
		PE_ASSERT(result);
		return result;
	}
	return false;
}


bool CEffectFileDirectX::setMatrix(eParameterHandles index, const Matrix4* data)const
{
	if (m_pEffect && isMatrixUsed(index))
	{
		return SUCCEEDED(m_pEffect->SetMatrix(m_paramHandle[index], data->GetConstPointer()));
	}
	return false;
}

/** load the resource from a file or win32 resource
If the resource file name begins with ':', it is treated as a win32 resource.
e.g.":IDR_FX_OCEANWATER". loads data from a resource of type "TEXTFILE". See MSDN for more information about Windows resources.
*/
HRESULT CEffectFileDirectX::InitDeviceObjects()
{
	m_bIsInitialized =true;
	m_bIsValid = false;//set to true if created successfully.
	LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();

	LPD3DXBUFFER pBufferErrors = NULL;
	
	HRESULT result=E_FAIL;
	CParaFile file(m_filename.c_str());
	if(!file.isEof())
	{
		/*
		// Since we are loading a binary file here and this effect has already been compiled,
		// you can not pass compiler flags here (for example to debug the shaders). 
		// To debug the shaders, one must pass these flags to the compiler that generated the
		// binary (for example fxc.exe). 
		- From within the Solution Explorer window, right click on *.fx and select Properties from the context menu.
		- Select Configuration Properties/Custom Build Step to view the custom build step directives.
		*/
		result = D3DXCreateEffect(
			pD3dDevice, 
			file.getBuffer(),
			(UINT)file.getSize(), 
			NULL, //(D3DXMACRO*) (s_bUseHalfPrecision ? s_halfPrecisionMacroTable : s_fullPrecisionMacroTable), 
			NULL, 
			NULL, // D3DXSHADER_PREFER_FLOW_CONTROL | D3DXSHADER_OPTIMIZATION_LEVEL2,
			0, 
			&m_pEffect, 
			&pBufferErrors );
	}
	else
	{
		OUTPUT_LOG("ERROR: shader file %s not found\n", m_filename.c_str());
		return result;
	}

    if( !SUCCEEDED( result ) )
	{
		char* error = (pBufferErrors!=0) ? (char*)pBufferErrors->GetBufferPointer() : "failed loading effect file\n";
		OUTPUT_LOG("Failed Loading Effect file %s: error is %s\n", m_filename.c_str(), error);
		SAFE_RELEASE(pBufferErrors);
        return result;
	}
	SAFE_RELEASE(pBufferErrors);

    // get the description
	m_pEffect->GetDesc( &m_EffectDesc );

	m_techniques.clear();
	//////////////////////////////////////////////////////////////////////////
	// get all valid techniques
	TechniqueDescDX tech;
	tech.hTechnique = NULL;
	bool bHasDefaultTechnique = false;
	while ((result = m_pEffect->FindNextValidTechnique(tech.hTechnique, &tech.hTechnique)) == S_OK)
	{
		// get some info about the technique
		result = m_pEffect->GetTechniqueDesc(tech.hTechnique, &tech.techniqueDesc);
		if( result == S_OK )
		{
			if(strcmp(tech.techniqueDesc.Name, "GenShadowMap") == 0)
			{
				tech.nCategory = TechCategory_GenShadowMap;
			}
			else if(strcmp(tech.techniqueDesc.Name, "Vegetation") == 0)
			{
				tech.nCategory = TechCategory_Vegetation;
			}
			else if(strcmp(tech.techniqueDesc.Name, "Character") == 0)
			{
				tech.nCategory = TechCategory_Character;
			}
			else if(strcmp(tech.techniqueDesc.Name, "DetailCharacter") == 0)
			{
				tech.nCategory = TechCategory_DetailCharacter;
			}
			else
			{
				tech.nCategory = TechCategory_default;
				bHasDefaultTechnique = true;
			}
			m_techniques.push_back(tech);
		}
		else
		{
			OUTPUT_LOG("ERROR: effect file: %s failed getting its description.\n", m_filename.c_str());
		}
	}
	if(!bHasDefaultTechnique)
	{
		// at least one default technique must be valid. "GenShadowMap" is not a default technique. 
		// So if "GenShadowMap" is valid, but others are not valid, the technique is not considered valid.
		OUTPUT_LOG("ERROR: effect file: %s not supported by your GPU.\n", m_filename.c_str());
		releaseEffect();
		result = E_FAIL;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// activate the first valid technique
		m_nTechniqueIndex = 0;
		result = m_pEffect->SetTechnique(m_techniques[m_nTechniqueIndex].hTechnique);
		if( result == S_OK)
		{
			// parse the effect parameters to build a list of handles
			parseParameters();
			m_bIsValid = true;
		}
		else
		{
			OUTPUT_LOG("ERROR: effect file: %s failed loading its technique.\n", m_filename.c_str());
			releaseEffect();
		}
		result = S_OK;
	}
	
	return result;
}

bool CEffectFileDirectX::SetTechniqueByIndex(int nIndex)
{
	if(m_nTechniqueIndex == nIndex)
		return true;
	else if(((int)m_techniques.size()>nIndex) && (m_pEffect->SetTechnique(m_techniques[nIndex].hTechnique) == S_OK))
	{
		m_nTechniqueIndex = nIndex;
		return true;
	}
	else
		return false;
}

bool CEffectFileDirectX::SetFirstValidTechniqueByCategory(TechniqueCategory nCat)
{
	if(m_nTechniqueIndex>=(int)m_techniques.size())
		return false;
	if(m_techniques[m_nTechniqueIndex].nCategory == nCat)
		return true;
	vector<TechniqueDescDX>::const_iterator itCur, itEnd = m_techniques.end();
	int i=0;
	for(itCur = m_techniques.begin(); itCur!=itEnd;++itCur,++i)
	{
		if ((*itCur).nCategory == nCat)
		{
			if(m_pEffect->SetTechnique(m_techniques[i].hTechnique) == S_OK)
			{
				m_nTechniqueIndex = i;
				return true;
			}
			else
				return false;
		}
	}
	return false;
}
void CEffectFileDirectX::releaseEffect()
{
	SAFE_RELEASE(m_pEffect);
}

// destroy the resource
HRESULT CEffectFileDirectX::DeleteDeviceObjects()
{
	releaseEffect();
	m_bIsInitialized =false;
	return S_OK;
}

// purge the resource from volatile memory
HRESULT CEffectFileDirectX::InvalidateDeviceObjects()
{
	if (m_pEffect)
	{
		m_pEffect->OnLostDevice();
	}
	return S_OK;
}

// prepare the resource for use (create any volatile memory objects needed)
HRESULT CEffectFileDirectX::RestoreDeviceObjects()
{
	if (m_pEffect)
	{
		m_pEffect->OnResetDevice();
	}
	return S_OK;
}

// save the resource to the file and return the size written
bool CEffectFileDirectX::saveResource(const char* filename)
{
	return 0;
}
int CEffectFileDirectX::totalPasses()const
{
	if(m_pEffect==0)
	{
		int i=0;
		OUTPUT_LOG("error");
	}
	//PE_ASSERT(m_pEffect);
	if(m_nTechniqueIndex<(int)m_techniques.size())
		return m_techniques[m_nTechniqueIndex].techniqueDesc.Passes;
	else
		return 0;
}

bool CEffectFileDirectX::setTexture(int index, TextureEntity* data)
{
	if (g_bTextureEnabled && m_pEffect && isTextureUsed(index) && !IsTextureLocked(index))
	{
		return setTextureInternal(index,(data!=0) ? data->GetTexture(): NULL);
	}
	return false;
}
bool CEffectFileDirectX::setTexture(int index, LPDIRECT3DTEXTURE9 pTex)
{
	if (g_bTextureEnabled && m_pEffect && isTextureUsed(index) && !IsTextureLocked(index))
	{
		return setTextureInternal(index,pTex);
	}
	return false;
}

bool CEffectFileDirectX::setTextureInternal(int index, LPDIRECT3DTEXTURE9 pTex)
{
	if((int)(m_LastTextures.size()) <= index)
	{
		m_LastTextures.resize(index+1, NULL);
	}
	if(m_LastTextures[index] != pTex)
	{
		m_LastTextures[index] = pTex;
		// TODO: implement statemanagerstate to set state, sampler and texture. This may be wrong if shader textures in hlsl are not written in the exact order. 
		// so that they map to s0,s15 in the written order in the compiled shader code. 
		//return SUCCEEDED(m_pEffect->SetTexture(m_paramHandle[k_tex0+index], (data!=0) ? data->GetTexture(): NULL));
		return SUCCEEDED(CGlobals::GetRenderDevice()->SetTexture(index,pTex));
	}
	return true;
}

bool CEffectFileDirectX::begin(bool bApplyParam, DWORD dwFlag, bool bForceBegin )
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();
	if(m_pEffect!=NULL)
	{
		if(bApplyParam)
		{
			// set the lighting parameters
			// from the global light manager
			applyGlobalLightingData(pScene->GetSunLight());

			// set the camera matrix
			applyCameraMatrices();
		}
		
		if(bForceBegin|| !m_bSharedMode)
		{
			HRESULT result = m_pEffect->Begin(0, dwFlag);
			if( SUCCEEDED( result ) )
			{
				m_bIsBegin = true;
				return true;
			}
			else
			{
				OUTPUT_LOG("error: CEffectFileDirectX::begin failed: %s \n", m_filename.c_str());
				return false;
			}
		}
		return true;
	}
	else
		return false;
}

bool CEffectFileDirectX::BeginPass(int pass,bool bForceBegin )
{
	if(bForceBegin || !m_bSharedMode)
	{
		HRESULT result = m_pEffect->BeginPass(pass);
		if( !SUCCEEDED( result ) )
		{
			OUTPUT_LOG("error: CEffectFileDirectX::BeginPass failed: %s \n", m_filename.c_str());
			return false;
		}
	}
    return true;
}

void CEffectFileDirectX::CommitChanges()
{
	m_pEffect->CommitChanges();
}

void CEffectFileDirectX::EndPass(bool bForceEnd)
{
	if(bForceEnd || !m_bSharedMode)
	{
		if(m_bIsBegin)
		{
			if( ! (m_pEffect && SUCCEEDED(m_pEffect->EndPass())))
			{
				OUTPUT_LOG("error: CEffectFileDirectX::EndPass failed: %s \n", m_filename.c_str());
			}
		}
	}
}

void CEffectFileDirectX::end(bool bForceEnd)
{
	if(bForceEnd || !m_bSharedMode)
	{
		if(m_bIsBegin)
		{
			// tricky: since d3dxeffect->BeginPass(0) will reset states such as Texture to NULL, 
			// we need to call OnSwitchOutShader() when pass is end. 
			if(!m_bSharedMode)
			{
				OnSwitchOutShader();
			}
			if( !(m_pEffect && SUCCEEDED( m_pEffect->End() )) )
			{
				OUTPUT_LOG("error: CEffectFileDirectX::end failed: %s \n", m_filename.c_str());
				return;
			}
			m_bIsBegin = false;
		}
	}
}

int CEffectFileDirectX::BeginWith(LPCSTR str, LPCSTR searchStr)
{
	int i = 0;
	while(str[i]!=0)
	{
		if(searchStr[i]==0)
			return i;
		else if(str[i] !=searchStr[i])
			return -1;
		++i; 
	}
	return -1;
}
bool CEffectFileDirectX::GetNumber(LPCSTR str, int nBeginIndex, int* pOut)
{
	int i = nBeginIndex;
	while(str[i]>='0' && str[i]<='9')
		++i;
	if(i>nBeginIndex)
	{
		int value=0;
		int nCount = i-nBeginIndex;
		for (i=0;i<nCount;++i)
		{
			value+=(int)((str[nBeginIndex+i]-'0'))*(int)pow((float)10,(nCount-i-1));
		}
		*pOut = value;
		return true;
	}
	return false;
}

void CEffectFileDirectX::parseParameters()
{
    // Look at parameters for semantics and annotations that we know how to interpret
    D3DXPARAMETER_DESC ParamDesc;
    D3DXHANDLE hParam;
	static char numerals[] = {'0','1','2','3','4','5','6','7','8','9'};
	int nIndex = 0;

	memset(m_paramHandle, 0, sizeof(m_paramHandle));

    for( UINT iParam = 0; iParam < m_EffectDesc.Parameters; iParam++ )
    {
        hParam = m_pEffect->GetParameter ( NULL, iParam );
        m_pEffect->GetParameterDesc( hParam, &ParamDesc );
        if( ParamDesc.Semantic != NULL && 
            ( ParamDesc.Class == D3DXPC_MATRIX_ROWS || ParamDesc.Class == D3DXPC_MATRIX_COLUMNS ) )
        {
            if( strcmpi( ParamDesc.Semantic, "world" ) == 0 )
				m_paramHandle[k_worldMatrix] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "worldinverse" ) == 0 )
				m_paramHandle[k_worldInverseMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "worldview" ) == 0 )
				m_paramHandle[k_worldViewMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "worldviewprojection" ) == 0 )
				m_paramHandle[k_worldViewProjMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "worldmatrixarray" ) == 0 )
				m_paramHandle[k_worldMatrixArray] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "skyboxmatrix" ) == 0 )
				m_paramHandle[k_skyBoxMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "view" ) == 0 )
				m_paramHandle[k_viewMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "projection" ) == 0 )
				m_paramHandle[k_projMatrix] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "viewprojection" ) == 0 )
				m_paramHandle[k_viewProjMatrix] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "texworldviewproj" ) == 0 )
				m_paramHandle[k_TexWorldViewProjMatrix] = hParam;
			


			// look for texture matrices which are named texMatX
			if (_strnicmp( ParamDesc.Semantic, "texmat", 6) ==0)
			{
				string name(ParamDesc.Name);
				int iPos = (int)name.find_first_of (numerals, 0, sizeof(numerals));

				if (iPos != string::npos)
				{
					int iTexture = atoi(&ParamDesc.Name[iPos]);
					if (iTexture >= 0 && iTexture<(k_tex_mat_max - k_tex_mat0))
					{
						m_paramHandle[k_tex_mat0 + iTexture] = hParam;
					}
				}
			}
        }

        if( ParamDesc.Semantic != NULL && 
            ( ParamDesc.Class == D3DXPC_VECTOR ))
        {
			if((nIndex = BeginWith(ParamDesc.Semantic, "material"))>0)
			{
				if( strcmpi( ParamDesc.Semantic, "materialambient" ) == 0 )
					m_paramHandle[k_ambientMaterialColor] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "materialdiffuse" ) == 0 )
					m_paramHandle[k_diffuseMaterialColor] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "materialspecular" ) == 0 )
					m_paramHandle[k_specularMaterialColor] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "materialemissive" ) == 0 )
					m_paramHandle[k_emissiveMaterialColor] = hParam;
			}
            else if( strcmpi( ParamDesc.Semantic, "posScaleOffset" ) == 0 )
                m_paramHandle[k_posScaleOffset] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "uvScaleOffset" ) == 0 )
                m_paramHandle[k_uvScaleOffset] = hParam;

            else if( strcmpi( ParamDesc.Semantic, "flareColor" ) == 0 )
                m_paramHandle[k_lensFlareColor] = hParam;

			//////////////////////////////////////////////////////////////////////////
			// fog
			else if( strcmpi( ParamDesc.Semantic, "fogparameters" ) == 0 )
				m_paramHandle[k_fogParameters] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "fogColor" ) == 0 )
				m_paramHandle[k_fogColor] = hParam;
			//////////////////////////////////////////////////////////////////////////
			//shadow
			else if( strcmpi( ParamDesc.Semantic, "shadowfactor") == 0)
				m_paramHandle[k_shadowFactor] = hParam;

			//////////////////////////////////////////////////////////////////////////
			// lights
			else if( strcmpi( ParamDesc.Semantic, "LightStrength") == 0 )
				m_paramHandle[k_LightStrength] = hParam;
			else if((nIndex = BeginWith(ParamDesc.Semantic, "Light"))>0)
			{
				if( strcmpi( ParamDesc.Semantic, "LightColors" ) == 0 )
					m_paramHandle[k_LightColors] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "LightPositions" ) == 0 )
					m_paramHandle[k_LightPositions] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "LightParams" ) == 0 )
					m_paramHandle[k_LightParams] = hParam;
			}

			else if( strcmpi( ParamDesc.Semantic, "FresnelR0" ) == 0 )
				m_paramHandle[k_fresnelR0] = hParam;
			else if((nIndex = BeginWith(ParamDesc.Semantic, "ConstVector"))>0)
			{
				if( strcmpi( ParamDesc.Semantic, "ConstVector0" ) == 0 )
					m_paramHandle[k_ConstVector0] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "ConstVector1" ) == 0 )
					m_paramHandle[k_ConstVector1] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "ConstVector2" ) == 0 )
					m_paramHandle[k_ConstVector2] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "ConstVector3" ) == 0 )
					m_paramHandle[k_ConstVector3] = hParam;
			}
			
            else if( strcmpi( ParamDesc.Semantic, "sunvector" ) == 0 )
                m_paramHandle[k_sunVector] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "suncolor" ) == 0 )
                m_paramHandle[k_sunColor] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "worldcamerapos" ) == 0 )
                m_paramHandle[k_cameraPos] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "viewdistances" ) == 0 )
                m_paramHandle[k_cameraDistances] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "worldviewvector" ) == 0 )
                m_paramHandle[k_cameraFacing] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "ambientlight" ) == 0 )
                m_paramHandle[k_ambientLight] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "sunlight_inscatter" ) == 0 )
                m_paramHandle[k_sunlightInscatter] = hParam;
            else if( strcmpi( ParamDesc.Semantic, "sunlight_extinction" ) == 0 )
                m_paramHandle[k_sunlightExtinction] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "worldpos" ) == 0 )
				m_paramHandle[k_worldPos] = hParam;
			else if( strcmpi( ParamDesc.Semantic, "texCoordOffset" ) == 0 )
				m_paramHandle[k_texCoordOffset] = hParam;
		}

        if(ParamDesc.Class == D3DXPC_SCALAR)
        {
			if( ParamDesc.Semantic == NULL)
			{
				if( strcmpi( ParamDesc.Name, "curnumbones" ) == 0 )
				{
					m_paramHandle[k_boneInfluenceCount] = hParam;
				}
			}
			else
			{
				if( strcmpi( ParamDesc.Semantic, "fogenable" ) == 0 )
					m_paramHandle[k_fogEnable] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "alphatesting" ) == 0 )
					m_paramHandle[k_bAlphaTesting] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "alphablending" ) == 0 )
					m_paramHandle[k_bAlphaBlending] = hParam;
				else if((nIndex = BeginWith(ParamDesc.Semantic, "boolean"))>0)
				{
					int nvalue;
					if(GetNumber(ParamDesc.Semantic, nIndex, &nvalue))
					{
						PE_ASSERT(0<=nvalue && nvalue<=(k_bBooleanMAX-k_bBoolean0));
						m_paramHandle[k_bBoolean0+nvalue] = hParam;
					}
				}
				else if( strcmpi( ParamDesc.Semantic, "sunlightenable" ) == 0 )
					m_paramHandle[k_bSunlightEnable] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "shadowmapsize" ) == 0 )
					m_paramHandle[k_nShadowmapSize] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "shadowradius" ) == 0 )
					m_paramHandle[k_fShadowRadius] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "materialpower" ) == 0 )
					m_paramHandle[k_specularMaterialPower] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "reflectfactor" ) == 0 )
					m_paramHandle[k_reflectFactor] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "locallightnum" ) == 0 )
					m_paramHandle[k_LocalLightNum] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "layersnum" ) == 0 )
					m_paramHandle[k_LayersNum] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "time" ) == 0 )
					m_paramHandle[k_time] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "opacity" ) == 0 )
					m_paramHandle[k_opacity] = hParam;
				else if( strcmpi( ParamDesc.Semantic, "specularPower" ) == 0 )
					m_paramHandle[k_specularPower] = hParam;
				else if( strcmpi(ParamDesc.Semantic,"transitionFactor") == 0)
					m_paramHandle[k_transitionFactor] = hParam;
			}
		}

        if( ParamDesc.Class == D3DXPC_OBJECT )
        {
			string name(ParamDesc.Name);
			
			if (ParamDesc.Type == D3DXPT_TEXTURE
				|| ParamDesc.Type == D3DXPT_TEXTURE2D
				|| ParamDesc.Type == D3DXPT_TEXTURE3D
				|| ParamDesc.Type == D3DXPT_TEXTURECUBE)
			{
				int iPos = (int)name.find_first_of (numerals, 0, sizeof(numerals));

				if (iPos != string::npos)
				{
					int iTexture = atoi(&ParamDesc.Name[iPos]);
					if (iTexture>=0 && iTexture<(k_tex_max - k_tex0))
					{
						m_paramHandle[k_tex0 + iTexture] = hParam;
					}
				}
			}
        }

		if ( ParamDesc.Class == D3DXPC_STRUCT)
		{
			if( strcmpi( ParamDesc.Semantic, "AtmosphericLightingParams" ) == 0 )
				m_paramHandle[k_atmosphericLighting] = hParam;
			else if ( strcmpi( ParamDesc.Semantic, "patchCorners") == 0 )
				m_paramHandle[k_patchCorners] = hParam;
		}
	}
}

void CEffectFileDirectX::EnableSunLight(bool bEnableSunLight)
{
	if (isParameterUsed(k_bSunlightEnable))
	{
		setBool(k_bSunlightEnable, bEnableSunLight);
	}
}

void CEffectFileDirectX::EnableAlphaBlending(bool bAlphaBlending)
{
	if (isParameterUsed(k_bAlphaBlending))
	{
		setBool(k_bAlphaBlending, bAlphaBlending);
	}
}

void CEffectFileDirectX::EnableAlphaTesting(bool bAlphaTesting)
{
	if (isParameterUsed(k_bAlphaTesting) )
	{
		setBool(k_bAlphaTesting, bAlphaTesting);
		if(!CGlobals::GetEffectManager()->IsD3DAlphaTestingDisabled())
			CGlobals::GetRenderDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,  bAlphaTesting );
	}
}

void CEffectFileDirectX::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial,bool bUseGlobalAmbient)
{
	if (pSurfaceMaterial)
	{
		// set material properties
		const ParaMaterial & d3dMaterial = *pSurfaceMaterial;
		
		if (isParameterUsed(k_ambientMaterialColor))
		{
			if(bUseGlobalAmbient && (d3dMaterial.Ambient.r < 0.01f) )
				setParameter(k_ambientMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient);
			else
				setParameter(k_ambientMaterialColor, &d3dMaterial.Ambient);
		}

		if (isParameterUsed(k_diffuseMaterialColor))
		{
			if(CGlobals::GetEffectManager()->GetScene()->GetSceneState()->HasLocalMaterial())
			{
				setParameter(k_diffuseMaterialColor, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Diffuse);
				setParameter(k_LightStrength, &CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentLightStrength());
			}
			else
			{
				setParameter(k_diffuseMaterialColor, &d3dMaterial.Diffuse);
				Vector3 vEmpty(0,0,0);
				setParameter(k_LightStrength, &vEmpty);
			}
		}

		if (isParameterUsed(k_specularMaterialColor))
		{
			setParameter(k_specularMaterialColor, &d3dMaterial.Specular);
		}

		if (isParameterUsed(k_emissiveMaterialColor))
		{
			setParameter(k_specularMaterialColor, &d3dMaterial.Emissive);
		}

		if (isParameterUsed(k_specularMaterialPower))
		{
			setParameter(k_specularMaterialPower, &d3dMaterial.Power);
		}

		// TODO: set textures
		/*
		for (UINT32 i=0;i<k_max_texture_handles;++i)
		{
			if (TEST_BIT(pSurfaceMaterial->textureFlags(),i))
			{
				setTexture(i, pSurfaceMaterial->texture(i));
			}
			if (TEST_BIT(pSurfaceMaterial->textureMatrixFlags(),i))
			{
				setTextureMatrix(i, pSurfaceMaterial->textureMatrix(i));
			}
		}*/
	}
}


void CEffectFileDirectX::applyGlobalLightingData(CSunLight& sunlight)
{
	// pass the lighting structure to the shader
	if (isParameterUsed(k_atmosphericLighting))
	{
		setParameter(
		k_atmosphericLighting, 
		sunlight.GetLightScatteringData()->getShaderData());
	}

	// pass the lighting structure to the shader
	if (isParameterUsed(k_sunColor))
	{
		setParameter(
			k_sunColor, 
			&(sunlight.GetSunColor()));
	}

	if (isParameterUsed(k_sunVector))
	{
		Vector3 vDir = -sunlight.GetSunDirection();
		setParameter(
			k_sunVector, 
			&Vector4(vDir.x, vDir.y, vDir.z, 1.0f));
	}

	if (isParameterUsed(k_ambientLight))
	{
		setParameter(
			k_ambientLight, 
			&CGlobals::GetEffectManager()->GetScene()->GetSceneState()->GetCurrentMaterial().Ambient);
	}
	

	if(isParameterUsed(k_shadowFactor))
	{
		float shadowFactor = sunlight.GetShadowFactor();
		setParameter(
			k_shadowFactor,
			&Vector4(shadowFactor,1-shadowFactor,0,0)
			);
	}
}

void CEffectFileDirectX::applyWorldMatrices()
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();

	CBaseCamera* pCamera = pScene->GetCurrentCamera();
	if (pCamera)
	{
		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		Matrix4 ViewProj;
		// set the world matrix
		if (isMatrixUsed(k_worldMatrix))
		{
			setMatrix(k_worldMatrix, pWorld);
		}

		// set the world inverse matrix
		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse;
			mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}
		// set the world view matrix
		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}
		// set the world view projection matrix
		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}
	}
}

void CEffectFileDirectX::applyCameraMatrices()
{
	IScene* pScene = CGlobals::GetEffectManager()->GetScene();

	CBaseCamera* pCamera = pScene->GetCurrentCamera();
	if (pCamera)
	{
		const Matrix4* pWorld = &(CGlobals::GetEffectManager()->GetWorldTransform());
		const Matrix4* pView = &(CGlobals::GetEffectManager()->GetViewTransform());
		const Matrix4* pProj = &(CGlobals::GetEffectManager()->GetProjTransform());
		Matrix4 ViewProj;
		// set the world matrix
		if (isMatrixUsed(k_worldMatrix))
		{
			setMatrix(k_worldMatrix, pWorld);
		}

		// set the world inverse matrix
		if (isMatrixUsed(k_worldInverseMatrix))
		{
			Matrix4 mWorldInverse;
			mWorldInverse = pWorld->inverse();
			setMatrix(k_worldInverseMatrix, &mWorldInverse);
		}
		// set the world view matrix
		if (isMatrixUsed(k_worldViewMatrix))
		{
			Matrix4 mWorldView;
			ParaMatrixMultiply(&mWorldView, pWorld, pView);
			setMatrix(k_worldViewMatrix, &mWorldView);
		}

		// set the combined matrix
		if (isMatrixUsed(k_viewProjMatrix))
		{
			ParaMatrixMultiply(&ViewProj, pView, pProj);
			setMatrix(k_viewProjMatrix, &ViewProj);
		}

		// set the world view projection matrix
		if (isMatrixUsed(k_worldViewProjMatrix))
		{
			if(!isMatrixUsed(k_viewProjMatrix))
				ParaMatrixMultiply(&ViewProj, pView, pProj);
			Matrix4 mWorldViewProj;
			ParaMatrixMultiply(&mWorldViewProj, pWorld, &ViewProj);
			setMatrix(k_worldViewProjMatrix, &mWorldViewProj);
		}

		// set the view matrix
		if (isMatrixUsed(k_viewMatrix))
		{
			setMatrix(k_viewMatrix, pView);
		}

		// set the projection matrix
		if (isMatrixUsed(k_projMatrix))
		{
			setMatrix(k_projMatrix, pProj);
		}

		// set the tex world view projection matrix
		if (CGlobals::GetEffectManager()->IsUsingShadowMap() && isMatrixUsed(k_TexWorldViewProjMatrix))
		{
			Matrix4 mTex;
			ParaMatrixMultiply(&mTex, pWorld, CGlobals::GetEffectManager()->GetTexViewProjMatrix());
			setMatrix(k_TexWorldViewProjMatrix, &mTex);
		}
		
		// set the world camera position
		if (isParameterUsed(k_cameraPos))
		{
			Vector3 vEye = pCamera->GetRenderEyePosition() - pScene->GetRenderOrigin();
			setParameter(k_cameraPos, &Vector4(vEye.x,vEye.y, vEye.z, 1.0f));
		}
		// set the world camera facing vector
		if (isParameterUsed(k_cameraFacing))
		{
			setParameter(k_cameraFacing, &pCamera->GetWorldAhead());
		}

		//// set the matrix used by sky boxes
		//if (isMatrixUsed(k_skyBoxMatrix))
		//{
		//	setMatrix(k_skyBoxMatrix, &pCamera->skyBoxMatrix());
		//}

		/*cVector4 camDistances(
			pCamera->nearPlane(),
			pCamera->farPlane(),
			pCamera->viewDistance(),
			pCamera->invFarPlane());
			
		if (isParameterUsed(k_cameraDistances))
		{
			setParameter(k_cameraDistances, &camDistances);
		}*/
	}
}

void CEffectFileDirectX::applyFogParameters( bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor)
{
	if (isParameterUsed(k_fogEnable))
	{
		setBool(k_fogEnable, bEnableFog);
	}
	if(bEnableFog)
	{
		if (isParameterUsed(k_fogParameters) && (fogParam!=0))
		{
			setParameter(k_fogParameters, fogParam);
		}

		if (isParameterUsed(k_fogColor) && (fogColor!=0))
		{
			setParameter(k_fogColor, fogColor);
		}
	}
}

void CEffectFileDirectX::applyLocalLightingData(const LightList* plights, int nLightNum)
{
	nLightNum = min(nLightNum, MAX_EFFECT_LIGHT_NUM);
	
	if (isParameterUsed(k_LocalLightNum))
	{
		setInt(k_LocalLightNum, nLightNum);

		//////////////////////////////////////////////////////////////////////////
		// set local lights
		if(plights!=0 && nLightNum>0)
		{
			const LightList& lights = *plights;
			PE_ASSERT(nLightNum <= (int)lights.size());

			Vector4 pos_ranges[MAX_EFFECT_LIGHT_NUM];
			Vector4 colors[MAX_EFFECT_LIGHT_NUM];
			Vector4 params[MAX_EFFECT_LIGHT_NUM];
			for (int i=0;i<nLightNum;++i)
			{
				Vector3 pos = lights[i]->Position;
				pos_ranges[i].x = pos.x;
				pos_ranges[i].y = pos.y;
				pos_ranges[i].z = pos.z;
				pos_ranges[i].w = lights[i]->Range;
				colors[i] = reinterpret_cast<const Vector4&> (lights[i]->Diffuse);
				params[i].x = lights[i]->Attenuation0;
				params[i].y = lights[i]->Attenuation1;
				params[i].z = lights[i]->Attenuation2;
				params[i].w = 1;
			}
			setVectorArray ( (eParameterHandles)(k_LightPositions), pos_ranges, nLightNum);
			setVectorArray ( (eParameterHandles)(k_LightColors), colors, nLightNum);
			setVectorArray ( (eParameterHandles)(k_LightParams), params, nLightNum);
		}
	}
	/*for (int i =0;i<MAX_SHADER_LIGHT_NUM;++i)
	{
		SetBoolean(LIGHT_BOOLEAN_BASE+i, (i<nLightNum));
	}*/
}

void CEffectFileDirectX::applyLayersNum(int nLayers)
{
	if (isParameterUsed(k_LayersNum))
	{
		setInt(k_LayersNum, nLayers);
	}
}

void CEffectFileDirectX::applyTexWorldViewProj(const Matrix4* mat)
{
	if (isMatrixUsed(k_TexWorldViewProjMatrix))
	{
		setMatrix(k_TexWorldViewProjMatrix, mat);
	}
}

void CEffectFileDirectX::SetShadowMapSize(int nsize)
{
	if (isParameterUsed(k_nShadowmapSize))
	{
		setInt(k_nShadowmapSize, nsize);
	}
}

void CEffectFileDirectX::SetShadowRadius(float fRadius)
{
	if (isParameterUsed(k_fShadowRadius))
	{
		setFloat(k_fShadowRadius, fRadius);
	}
}

void CEffectFileDirectX::EnableShadowmap(int nShadowMethod)
{
#define SHADOW_BOOLEAN_BASE			8
	SetBoolean(SHADOW_BOOLEAN_BASE, nShadowMethod > 0);
	SetBoolean(SHADOW_BOOLEAN_BASE+1, nShadowMethod == 1);
}
bool CEffectFileDirectX::SetBoolean(int nIndex, bool value)
{
	PE_ASSERT(nIndex< (k_bBooleanMAX-k_bBoolean0));
	if (isParameterUsed((eParameterHandles)(k_bBoolean0+nIndex)))
	{
		return setBool((eParameterHandles)(k_bBoolean0+nIndex), value);
	}
	return false;
}
int CEffectFileDirectX::GetCurrentTechniqueIndex()
{
	return m_nTechniqueIndex;
}
const CEffectFileDirectX::TechniqueDesc* CEffectFileDirectX::GetCurrentTechniqueDesc()
{
	const static TechniqueDesc g_techdesc; 
	if(m_nTechniqueIndex<(int)m_techniques.size())
		return &(m_techniques[m_nTechniqueIndex]);
	else
		return &g_techdesc;
}

CParameterBlock* ParaEngine::CEffectFileDirectX::GetParamBlock( bool bCreateIfNotExist /*= false*/ )
{
	return &m_SharedParamBlock;
}

void ParaEngine::CEffectFileDirectX::LockTexture( int nIndex )
{
	if(nIndex <0)
	{
		// lock all 9 textures 
		m_LockedTextures.resize(9);
		int nSize = (int)m_LockedTextures.size();
		for (int i=0;i<nSize;++i)
		{
			m_LockedTextures[i] = true;
		}
	}
	else
	{
		if(nIndex >= (int)(m_LockedTextures.size()))
		{
			m_LockedTextures.resize(nIndex+1, false);
		}
		m_LockedTextures[nIndex] = true;
	}
}

void ParaEngine::CEffectFileDirectX::UnLockTexture( int nIndex )
{
	if(nIndex <0)
	{
		int nSize = (int)m_LockedTextures.size();
		for (int i=0;i<nSize;++i)
		{
			m_LockedTextures[i] = false;
		}
	}
	else
	{
		if(nIndex < (int)(m_LockedTextures.size()))
		{
			m_LockedTextures[nIndex] = false;
		}
	}
}

bool ParaEngine::CEffectFileDirectX::IsTextureLocked( int nIndex ) const
{
	return (nIndex < (int)(m_LockedTextures.size())) && m_LockedTextures[nIndex];
}

HRESULT ParaEngine::CEffectFileDirectX::SetRawValue( D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes )
{
	return GetDXEffect()->SetRawValue(hParameter,pData, ByteOffset, Bytes);
}

bool ParaEngine::CEffectFileDirectX::SetBool( D3DXHANDLE hParameter, BOOL bBoolean )
{
	return SUCCEEDED(SetRawValue(hParameter, &bBoolean, 0, sizeof(bBoolean)));
}

bool ParaEngine::CEffectFileDirectX::SetInt( D3DXHANDLE hParameter, int nValue )
{
	return SUCCEEDED(SetRawValue(hParameter, &nValue, 0, sizeof(nValue)));
}

bool ParaEngine::CEffectFileDirectX::SetFloat( D3DXHANDLE hParameter, float fValue )
{
	return SUCCEEDED(SetRawValue(hParameter, &fValue, 0, sizeof(fValue)));
}

bool ParaEngine::CEffectFileDirectX::SetVector2( D3DXHANDLE hParameter, const Vector2& vValue )
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileDirectX::SetVector3( D3DXHANDLE hParameter, const Vector3& vValue )
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileDirectX::SetVector4( D3DXHANDLE hParameter, const Vector4& vValue )
{
	return SUCCEEDED(SetRawValue(hParameter, &vValue, 0, sizeof(vValue)));
}

bool ParaEngine::CEffectFileDirectX::SetMatrix( D3DXHANDLE hParameter, const Matrix4& data )
{
	return SUCCEEDED(SetRawValue(hParameter, &data, 0, sizeof(data)));
}

void ParaEngine::CEffectFileDirectX::OnSwitchInShader()
{
}

void ParaEngine::CEffectFileDirectX::OnSwitchOutShader()
{
	vector<void*>::iterator itCur, itEnd = m_LastTextures.end();
	for (itCur = m_LastTextures.begin(); itCur!=itEnd; ++itCur)
	{
		(*itCur) = NULL;
	}
}

bool ParaEngine::CEffectFileDirectX::BeginSharePassMode( bool bApplyParam /*= true*/, DWORD flag/*=D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESAMPLERSTATE|D3DXFX_DONOTSAVESHADERSTATE*/, bool bForceBegin /*= true*/ )
{
	EnableShareMode(true);
	if(begin(bApplyParam, flag, bForceBegin))
	{
		return BeginPass(0, true);
	}
	return false;
}

void ParaEngine::CEffectFileDirectX::EndSharePassMode()
{
	EnableShareMode(false);
	EndPass(true);
	end(true);
}

void ParaEngine::CEffectFileDirectX::SetFileName(const std::string& filename)
{
	m_filename = filename;
}

D3DXHANDLE& ParaEngine::CEffectFileDirectX::GetTextureHandle(int nIndex)
{
	return m_paramHandle[k_tex0 + nIndex];
}

#endif