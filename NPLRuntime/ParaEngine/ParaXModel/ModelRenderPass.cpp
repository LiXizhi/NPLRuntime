//-----------------------------------------------------------------------------
// Class:	Model render pass
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8, 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EffectManager.h"
#include "SceneObject.h"
#include "TextureEntity.h"
#include "ParaXBone.h"
#include "ParaXModel.h"
#include "ModelRenderPass.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif

using namespace ParaEngine;


ParaEngine::ModelRenderPass::ModelRenderPass()
{
	memset(this, 0, sizeof(ModelRenderPass));
}

bool ParaEngine::ModelRenderPass::init_bmax_FX(CParaXModel *m, SceneState* pSceneState, CParameterBlock* pMaterialParams /*= NULL*/)
{
	if (m->showGeosets[geoset] == false || indexCount == 0)
		return false;
	float materialAlpha = 1.f;
	bool nozwrite_ = nozwrite;
	if (pMaterialParams != NULL)
	{
		CParameter* pParams = pMaterialParams->GetParameter("g_opacity");
		if (pParams)
			materialAlpha = (float)(*pParams);
		pParams = pMaterialParams->GetParameter("zwrite");
		if (pParams && (bool)(*pParams) == false)
			nozwrite_ = true;
	}
	if (materialAlpha <= 0.f)
		return false;

	else if (materialAlpha < 0.999f)
	{
		CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		blendmode |= BM_TEMP_FORCEALPHABLEND;
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pEffect->EnableAlphaBlending(true);
		pEffect->setFloat(CEffectFile::k_opacity, materialAlpha);
	}
	else
	{
		CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		pEffect->setFloat(CEffectFile::k_opacity, 1.f);
		// BMAX_MODEL(.x) exported from CAD, then model shape is a plane, need to set the BackFaceCulling false
		if (blendmode == BM_ALPHA_BLEND)
			CGlobals::GetEffectManager()->SetCullingMode(false);
	}

	CBaseObject* pBaseObj = pSceneState->GetCurrentSceneObject();
	if (pBaseObj != NULL) pBaseObj->ApplyMaterial();
	
	return true;
}

void ParaEngine::ModelRenderPass::deinit_bmax_FX(SceneState* pSceneState, CParameterBlock* pMaterialParams /*= NULL*/)
{
	if ((blendmode & BM_TEMP_FORCEALPHABLEND) == BM_TEMP_FORCEALPHABLEND)
	{
		CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		blendmode &= (~BM_TEMP_FORCEALPHABLEND);
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		pEffect->EnableAlphaBlending(false);
		pEffect->setFloat(CEffectFile::k_opacity, 1.f);
		pEffect->EnableAlphaTesting(false);
	}
	if (blendmode == BM_ALPHA_BLEND)
		CGlobals::GetEffectManager()->SetCullingMode(true);
}


bool ModelRenderPass::init_FX(CParaXModel *m, SceneState* pSceneState,CParameterBlock* pMaterialParams)
{
	if(m->showGeosets[geoset] == false || indexCount == 0)
		return false;
	Vector4 ocol(1,1,1,m->m_trans);
	Vector4 ecol(0,0,0,0);

	// Emissive colors
	if (color!=-1) {
		// TODO: non-local provider?
		Vector3 c = m->colors[color].color.getValue(m->m_CurrentAnim);
		float o = m->colors[color].opacity.getValue(m->m_CurrentAnim);
		ocol.w *= o;
		if (unlit) {
			ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
		} else {
			ocol.x = ocol.y = ocol.z = 0;
		}
		ecol = Vector4(c, ocol.w);
	}


	// opacity
	if (opacity!=-1) {
		ocol.w *= m->transparency[opacity].trans.getValue(m->m_CurrentAnim);
	}

	float materialAlpha = 1.f;
	bool nozwrite_ = nozwrite;
	if(pMaterialParams != NULL)
	{
		CParameter* pParams = pMaterialParams->GetParameter("g_opacity");
		if(pParams)
			materialAlpha = (float)(*pParams);
		pParams = pMaterialParams->GetParameter("zwrite");
		if (pParams && (bool)(*pParams) == false)
			nozwrite_ = true;
	}
	ocol.w *= materialAlpha;

	// if the opacity or Emissive color is 0, we should not draw this pass. 
	if(( (ocol.w > 0) && (color==-1 || (ecol.w > 0)) ) == false)
		return false;

	/// Set the texture
	TextureEntity* bindtex = NULL;
	int32_t tex = GetTexture1();
	if (tex >= CParaXModel::MAX_MODEL_TEXTURES || m->specialTextures[tex] == -1)
		bindtex = m->textures[tex].get();
	else 
	{
		bindtex = m->replaceTextures[m->specialTextures[tex]];
		// use default texture if replaceable texture is not specified. 
		if(bindtex == 0)
			bindtex = m->textures[tex].get();
	}

	// do not render for NULL textures, possibly because the texture is not fully loaded. 
	if (!bindtex || bindtex->GetTexture() == 0)
		return false;

	if(pSceneState->IsIgnoreTransparent() && (blendmode == BM_ADDITIVE || blendmode == BM_ALPHA_BLEND || blendmode == BM_ADDITIVE_ALPHA
		|| materialAlpha < 1))
	{
		return false;
	}

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	PE_ASSERT(pEffect!=0);
	if (!pSceneState->IsShadowPass())
	{
		// blend mode
		switch (blendmode) {
		case BM_TRANSPARENT: // 1
			pEffect->EnableAlphaTesting(true);
			break;
		case BM_ALPHA_BLEND: // 2
			pEffect->EnableAlphaBlending(true);
			break;
		case BM_ADDITIVE: // 3
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		case BM_ADDITIVE_ALPHA: // 4
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		default: // BM_OPAQUE
			// default to OPAQUE
			// Note 2009.8.9: enable alpha test is enabled even for opaque character
			// pEffect->EnableAlphaBlending(false);
			// pEffect->EnableAlphaTesting(false);
			break;
		}

		if (nozwrite_) {
			CGlobals::GetEffectManager()->EnableZWrite(false);
		}

		if (!cull) {
			CGlobals::GetEffectManager()->SetCullingMode(false);
		}

		/*
		if(opacity != -1)
		{
		pEffect->setFloat(CEffectFile::k_opacity, ocol.w);
		}
		*/

		// color
		if (blendmode <= 1 && ocol.w<0.999f)
		{
			blendmode |= BM_TEMP_FORCEALPHABLEND;
			pEffect->EnableAlphaBlending(true);
			pEffect->setFloat(CEffectFile::k_opacity, ocol.w);
		}
		else if (opacity != -1)
		{
			pEffect->setFloat(CEffectFile::k_opacity, ocol.w);
		}

		if (color != -1)
		{
			Vector3 color_(ecol.x, ecol.y, ecol.z);
			pEffect->setParameter(CEffectFile::k_emissiveMaterialColor, &color_);
		}

		if (unlit) {
			pEffect->EnableSunLight(false);
		}

		if (texanim != -1) {

			//if(m->animTexRGB)
			//pEffect->setParameter(CEffectFile::k_bBoolean7,(const float*)&Vector4(1,0,0,0));

			const TextureAnim& texAnim = m->texanims[texanim];
			//OUTPUT_LOG("UV off: %f  %f\n", texAnim.tval.x, texAnim.tval.y);
			// it will interpolate between interval 1/30 second, hence when implementing blinking eye animation, make sure that the animation is still all right with intepolation on 1/30 texture UV animation. 
			Vector4 v(texAnim.tval.x, texAnim.tval.y, 0.f, 0.f);
			pEffect->setParameter(CEffectFile::k_ConstVector0, &v);
			//pEffect->setParameter(CEffectFile::k_ConstVector0, (const float*)&Vector4(0.00000f,0.242f, 0.f, 0.f));
		}

		if (GetCategoryId() > 0)
		{
			Vector4 v((float)GetCategoryId(), 0.f, 0.f, 0.f);
			pEffect->setParameter(CEffectFile::k_ConstVector1, &v);
		}

		if (pEffect->isParameterUsed(CEffectFile::k_transitionFactor) && pMaterialParams != NULL)
		{

			CParameter* pParams = pMaterialParams->GetParameter("transitionFactor");
			if (pParams)
				pEffect->setFloat(CEffectFile::k_transitionFactor, (float)(*pParams));
		}
	}
	else
	{
		if (blendmode != BM_OPAQUE && blendmode != BM_TRANSPARENT)
			return false;

		if (blendmode == BM_TRANSPARENT)
			pEffect->EnableAlphaTesting(true);
	}
	
	/// Set the texture
	pEffect->setTexture(0, bindtex);

	if(is_rigid_body)
	{
		if (m->GetObjectNum().nBones > 0)
		{
			Matrix4 mat, mat1;
			mat1 = m->bones[(m->m_origVertices[m->m_indices[m_nIndexStart] + GetVertexStart(m)]).bones[0]].mat;
			mat = mat1 * CGlobals::GetWorldMatrixStack().SafeGetTop();
			CGlobals::GetWorldMatrixStack().push(mat);
		}
		else
		{
			CGlobals::GetWorldMatrixStack().push(CGlobals::GetWorldMatrixStack().SafeGetTop());
		}
		pEffect->applyWorldMatrices();
	}

	CBaseObject* pBaseObj = pSceneState->GetCurrentSceneObject();
	if (pBaseObj != NULL) 
		pBaseObj->ApplyMaterial();

	return true;
}
void ModelRenderPass::deinit_FX(SceneState* pSceneState, CParameterBlock* pMaterialParams /*= NULL*/)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	PE_ASSERT(pEffect!=0);
	if(is_rigid_body)
	{
		CGlobals::GetWorldMatrixStack().pop();
		pEffect->applyWorldMatrices();
	}
	bool nozwrite_ = nozwrite;
	if (pMaterialParams != NULL)
	{
		CParameter* pParams = NULL;
		pParams = pMaterialParams->GetParameter("zwrite");
		if (pParams && (bool)(*pParams) == false)
			nozwrite_ = true;
	}
	if (!pSceneState->IsShadowPass())
	{
		switch (blendmode) {
		case BM_OPAQUE:
			// Note 2009.8.9: enable alpha blending is enabled even for opaque character, since the shader will use blending afterall for fog effect, this will has no overhead. 
			// pEffect->EnableAlphaBlending(false);
			break;
		case BM_ALPHA_BLEND:
			pEffect->EnableAlphaBlending(false);
			break;
		case BM_TRANSPARENT:
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(false);
			break;
		case BM_ADDITIVE:
		case BM_ADDITIVE_ALPHA:
			pEffect->EnableAlphaBlending(false);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		default:
			break;
		}
		if (nozwrite_) {
			CGlobals::GetEffectManager()->EnableZWrite(true);
		}

		if (!cull){
			CGlobals::GetEffectManager()->SetCullingMode(true);
		}

		if (unlit) {
			pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled());
		}
		if (texanim != -1) {
			pEffect->setParameter(CEffectFile::k_ConstVector0, &Vector4::ZERO);
		}
		if (opacity != -1)
		{
			pEffect->setFloat(CEffectFile::k_opacity, 1.f);
		}
		if (GetCategoryId() > 0)
		{
			pEffect->setParameter(CEffectFile::k_ConstVector1, &Vector4::ZERO);
		}

		if ((blendmode & BM_TEMP_FORCEALPHABLEND) == BM_TEMP_FORCEALPHABLEND)
		{
			blendmode &= (~BM_TEMP_FORCEALPHABLEND);
			pEffect->EnableAlphaBlending(false);
			pEffect->setFloat(CEffectFile::k_opacity, 1);
		}

		if (color != -1)
		{
			Vector3 color_(0.f, 0.f, 0.f);
			pEffect->setParameter(CEffectFile::k_emissiveMaterialColor, &color_);
		}
	}
	else
	{
		if (blendmode == BM_TRANSPARENT)
			pEffect->EnableAlphaTesting(false);
	}
}

bool ModelRenderPass::init(CParaXModel *m, SceneState* pSceneState)
{
	if(m->showGeosets[geoset] == false)
		return false;
	Vector4 ocol(1,1,1,m->m_trans);
	Vector4 ecol(0,0,0,0);

	// Emissive colors
	if (color!=-1) {
		// TODO: non-local provider?
		Vector3 c = m->colors[color].color.getValue(m->m_CurrentAnim);
		float o = m->colors[color].opacity.getValue(m->m_CurrentAnim);
		ocol.w *= o;
		if (unlit) {
			ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
		} else {
			ocol.x = ocol.y = ocol.z = 0;
		}
		ecol = Vector4(c, ocol.w);
	}

	// opacity
	if (opacity!=-1) {
		// TODO: non-local provider?
		ocol.w *= m->transparency[opacity].trans.getValue(m->m_CurrentAnim);
	}

	if(( (ocol.w > 0) && (color==-1 || (ecol.w > 0)) ) == false)
		return false;

	/// Set the texture
	TextureEntity* bindtex = NULL;
	if (m->specialTextures[tex]==-1) 
		bindtex = m->textures[tex].get();
	else 
	{
		bindtex = m->replaceTextures[m->specialTextures[tex]];
		// use default texture if replaceable texture is not specified. 
		if(bindtex == 0)
			bindtex = m->textures[tex].get();
	}
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DTEXTURE9 pTex = NULL;
	if(bindtex)
		pTex = bindtex->GetTexture();

	// do not render for NULL textures, possibly because the texture is not fully loaded. 
	if(pTex==NULL)
		return false;

	if(pSceneState->IsIgnoreTransparent() && (blendmode == BM_ADDITIVE || blendmode == BM_ALPHA_BLEND || blendmode == BM_ADDITIVE_ALPHA))
	{
		return false;
	}

	auto pd3dDevice = CGlobals::GetRenderDevice();
	// blend mode
	switch (blendmode) {
	case BM_TRANSPARENT: // 1
		pd3dDevice->SetRenderState( ERenderState::ALPHATESTENABLE, TRUE);
		break;
	case BM_ALPHA_BLEND: // 2
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		break;
	case BM_ADDITIVE: // 3
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);	
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		break;
	case BM_ADDITIVE_ALPHA: // 4
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);	
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		break;
	default: // BM_OPAQUE
		// default to OPAQUE
		//pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		//pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE); 
		break;
	}

	if (nozwrite) {
		CGlobals::GetEffectManager()->EnableZWrite(false);
	}

	if (!cull) {
		CGlobals::GetEffectManager()->SetCullingMode(false);
	}

	pd3dDevice->SetTexture(0, pTex);

	if (unlit) {
		pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
	}

	if (texanim!=-1) {
		// it will interpolate between interval 1/30 second, hence when implementing blinking eye animation, make sure that the animation is still all right with interpolation on 1/30 texture UV animation. 
		//// Set up the matrix for the desired transformation.
		Matrix4 texMat;
		const TextureAnim& texAnim = m->texanims[texanim];
		
		// interesting that it uses _31 and _32 instead of _41, _42 for the UV translation.
		texMat = Matrix4::IDENTITY;
		texMat._31 = texAnim.tval.x;
		texMat._32 = texAnim.tval.y;
		GETD3D(pd3dDevice)->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 	D3DTTFF_COUNT2);
		pd3dDevice->SetTransform( ETransformsStateType::TEXTURE0, texMat.GetConstPointer() );
		//pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
	}

	// color
	if (blendmode<=1 && ocol.w<0.99f) 
	{
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
	}

	// TODO: how to programmatically set alpha to ocol.w in fixed function ? right now, it is either on or off. 
	//if(opacity != -1)
	//{
	//	/*pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA );
	//	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	//	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	//	
	//	pd3dDevice->SetRenderState(ERenderState::TEXTUREFACTOR, (DWORD)(LinearColor(1.f, 1.f, 1.f, ocol.w)));*/

	//	ParaMaterial mtrl;
	//	ZeroMemory( &mtrl, sizeof(mtrl) );

	//	// if a material is used, SetRenderState must be used
	//	// vertex color = light diffuse color * material diffuse color
	//	mtrl.Diffuse.r = 1.f;
	//	mtrl.Diffuse.g = 1.f;
	//	mtrl.Diffuse.b = 1.f;
	//	mtrl.Diffuse.a = ocol.w;

	//	/*mtrl.Emissive.r = 1.0f;
	//	mtrl.Emissive.g = 1.0f;
	//	mtrl.Emissive.b = 1.0f;*/
	//	
	//	pd3dDevice->SetMaterial( &mtrl );

	//	pd3dDevice->SetRenderState(ERenderState::COLORVERTEX, TRUE);
	//	
	//	pd3dDevice->SetRenderState(ERenderState::DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	//	pd3dDevice->SetRenderState(ERenderState::EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

	//	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	//	// pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	//}

#ifdef TODO
	/// TODO: set texture addressing
	if (swrap) {

	}
	if (twrap) {

	}
	if (useenvmap) {
		// environment mapping
		// TODO: this is not implemented yet.
	}
#endif

	if(is_rigid_body)
	{
		if (m->GetObjectNum().nBones > 0)
		{
			Matrix4 mat, mat1;
			mat1 = m->bones[(m->m_origVertices[m->m_indices[m_nIndexStart] + GetVertexStart(m)]).bones[0]].mat;
			mat = mat1 * CGlobals::GetWorldMatrixStack().SafeGetTop();
			CGlobals::GetWorldMatrixStack().push(mat);
			pd3dDevice->SetTransform(ETransformsStateType::WORLD, mat.GetConstPointer());
		}
		else
		{
			CGlobals::GetWorldMatrixStack().push(CGlobals::GetWorldMatrixStack().SafeGetTop());
			pd3dDevice->SetTransform(ETransformsStateType::WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
		}
	}
#endif
	return true;
}

void ModelRenderPass::deinit()
{
#ifdef USE_DIRECTX_RENDERER
	auto pd3dDevice = CGlobals::GetRenderDevice();
	if(is_rigid_body)
	{
		CGlobals::GetWorldMatrixStack().pop();
		CGlobals::GetRenderDevice()->SetTransform(ETransformsStateType::WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
	}

	switch (blendmode) {
		case BM_OPAQUE:
			break;			
		case BM_ALPHA_BLEND:
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			break;
		case BM_TRANSPARENT:
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			break;
		case BM_ADDITIVE:
		case BM_ADDITIVE_ALPHA:
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		default:
			break;
	}

	if (nozwrite) {
		CGlobals::GetEffectManager()->EnableZWrite(true);
	}

	if(!cull){
		CGlobals::GetEffectManager()->SetCullingMode(true);
	}
	
	if (unlit) {
		pd3dDevice->SetRenderState( ERenderState::LIGHTING, CGlobals::GetScene()->IsLightEnabled() );
	}
	
	if (texanim!=-1) {
		// disable texture transformation in fixed function.
		GETD3D(pd3dDevice)->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 	D3DTTFF_DISABLE );
	}
#ifdef TODO
	// TODO:
	if (useenvmap) {
	}
	if (swrap) {
	}
	if (twrap) {
	}
#endif
#endif
}

bool ParaEngine::ModelRenderPass::IsAlphaBlended()
{
	// opaque and alpha tested passes are not alpha blended. 
	return blendmode != BM_OPAQUE && blendmode != BM_TRANSPARENT;
}

int ParaEngine::ModelRenderPass::GetPhysicsGroup()
{
	// TODO: support more physics group?
	return 0;
}

bool ParaEngine::ModelRenderPass::hasPhysics()
{
	return force_physics || (!IsAlphaBlended() && !disable_physics);
}

void ParaEngine::ModelRenderPass::SetStartIndex(int32 nIndex)
{
	m_nIndexStart = nIndex;
	indexStart = (nIndex >= 0xffff) ? 0xffff : (uint16)nIndex;
}

int32 ParaEngine::ModelRenderPass::GetStartIndex()
{
	return m_nIndexStart;
}

int32 ParaEngine::ModelRenderPass::GetVertexStart(CParaXModel *m)
{
	return m->geosets[geoset].GetVertexStart();
}

int32 ParaEngine::ModelRenderPass::GetCategoryId()
{
	return has_category_id ? (int)m_fCategoryID : 0;
}

void ParaEngine::ModelRenderPass::SetCategoryId(int32 nCategoryID)
{
	if (nCategoryID == 0)
	{
		has_category_id = false;
	}
	else
	{
		m_fCategoryID = (float)nCategoryID;
		has_category_id = true;
	}
}

