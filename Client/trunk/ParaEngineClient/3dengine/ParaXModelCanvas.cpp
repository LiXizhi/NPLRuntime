//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/AnimTable.h"
#include "ParaXEntity.h"
#include "SceneState.h"
#include "MeshEntity.h"
#include "MeshObject.h"
#include "ParaXModelCanvas.h"

using namespace ParaEngine;

/************************************************************************/
/* ParaXModelCanvas                                                     */
/************************************************************************/

ParaXModelCanvas::ParaXModelCanvas()
	:m_root(new CanvasAttachment())
{
}

ParaXModelCanvas::~ParaXModelCanvas()
{
	clearAttachments();
}

void ParaXModelCanvas::MountOnModel(ParaXEntity *model, float fScale)
{
	//TODO: 
	if (m_MountModel != model)
		m_MountModel = model;
}

void ParaXModelCanvas::UnmountModel()
{
	//TODO: 
	m_MountModel.reset();
}

bool ParaXModelCanvas::IsMounted()
{
	return m_MountModel;
}

ParaXEntity* ParaXModelCanvas::GetBaseModel()
{
	return m_baseModel.get();
}

ParaXEntity* ParaXModelCanvas::GetAnimModel()
{
	if (IsMounted())
		return m_MountModel.get();
	else
		return m_baseModel.get();
}

void ParaXModelCanvas::SetAutoCharacterModel(bool bAutoCharacterModel)
{
	m_bIsAutoCharacter = bAutoCharacterModel;
	if (m_root)
	{
		m_root->m_bIsAutoCharacter = m_bIsAutoCharacter;
	}
}

bool ParaXModelCanvas::Animate(SceneState * sceneState, CharacterPose* pPose, int nAnimID, IAttributeFields* pAnimInstance)
{
	return m_root->animate(sceneState, pPose, false, nAnimID, pAnimInstance);
}

void ParaXModelCanvas::BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
	if (GetBaseModel() != NULL)
		m_root->BuildShadowVolume(sceneState, pShadowVolume, pLight, mxWorld);
}

void ParaXModelCanvas::Draw(SceneState * sceneState, CParameterBlock* materialParams)
{
	if (GetBaseModel() != NULL)
		m_root->draw(sceneState, this, materialParams);
}

CanvasAttachment* ParaXModelCanvas::addAttachment(ParaXEntity *model, int id, int slot, float fScale)
{
	return m_root->addChild(model, id, slot, fScale);
}
CanvasAttachment* ParaXModelCanvas::addAttachment(MeshEntity *model, int id, int slot, float fScale)
{
	return m_root->addChild(model, id, slot, fScale);
}

void ParaXModelCanvas::clearAttachments()
{
	m_root->delChildren();
}

void ParaXModelCanvas::deleteSlot(int slot)
{
	m_root->delSlot(slot);
}

CanvasAttachment* ParaXModelCanvas::GetChild(int id, int slot /*= -1*/)
{
	return m_root->GetChild(id, slot);
}

void ParaXModelCanvas::deleteAttachment(int nAttachmentID)
{
	m_root->delChildByID(nAttachmentID);
}

bool ParaXModelCanvas::InitBaseModel(ParaXEntity * pModel)
{
	clearAttachments();
	m_baseModel = pModel;
	if (!m_baseModel)
		return false;
	m_root->SetModel(m_baseModel.get());
	return true;
}

/************************************************************************/
/* CanvasAttachment                                                     */
/************************************************************************/

CanvasAttachment::CanvasAttachment()
	:parent(NULL), id(0), slot(-1), scale(1.0f), m_bIsAutoCharacter(false), m_vOffset(0, 0, 0), m_pParamBlock(nullptr)
{

}
CanvasAttachment::CanvasAttachment(CanvasAttachment *parent, ParaXEntity *model, int id, int slot, float scale) :
	parent(parent), model(model), id(id), slot(slot), scale(scale), m_bIsAutoCharacter(false), m_vOffset(0, 0, 0), m_pParamBlock(nullptr)
{
}

CanvasAttachment::CanvasAttachment(CanvasAttachment *parent, MeshEntity *model, int id, int slot, float scale) :
	parent(parent), id(id), slot(slot), scale(scale), m_bIsAutoCharacter(false), m_vOffset(0, 0, 0), m_pParamBlock(nullptr)
{
	SetModel(model);
}

CanvasAttachment::~CanvasAttachment()
{
	delChildren();
	SAFE_DELETE(m_pParamBlock);
}

void CanvasAttachment::SetModel(MeshEntity* pModel)
{
	if (!m_pMeshObject)
	{
		m_pMeshObject.reset(new CMeshObject());
		m_pMeshObject->SetBoundingBox(0.1f, 0.1f, 0.1f, 0);
		m_pMeshObject->SetLocalTransform(*CGlobals::GetIdentityMatrix());
		m_pMeshObject->SetMyType(_House);
		m_pMeshObject->SetCtorPercentage(1.f);
	}
	m_pMeshObject->InitObject(pModel, NULL, Vector3(0, 0, 0));
}

void CanvasAttachment::SetModel(ParaXEntity* pModel)
{
	model = pModel;
}
void CanvasAttachment::delChild(CanvasAttachment* child)
{
	for (size_t i = 0; i < children.size();) {
		if (children[i].get() == child) {
			children.erase(children.begin() + i);
			return;
		}
		else i++;
	}
}

void CanvasAttachment::release()
{
	if (parent)
	{
		parent->delChild(this); // implicitly called delete this;
	}
	else
		delete this;
}

void CanvasAttachment::SetReplaceableTexture(TextureEntity* pTex, int replaceableTextureID)
{
	if (pTex)
	{
		texReplaceables[replaceableTextureID] = pTex;
	}
}

bool CanvasAttachment::SetupParantTransform(SceneState * sceneState, float fCameraToObjectDist)
{
	if (parent == 0)
		return false;
	if (parent->model != 0)
	{
		int nIndex = (sceneState && sceneState->IsLODEnabled()) ? parent->model->GetLodIndex(fCameraToObjectDist) : 0;
		CParaXModel* pModel = parent->model->GetModel(nIndex);
		if (pModel)
		{
			return pModel->SetupTransformByID(id);
		}
	}
	return false;
}

CanvasAttachment* CanvasAttachment::addChild(MeshEntity *m, int id, int slot, float scale)
{
	if (m && id >= 0) {
		CanvasAttachment *att = new CanvasAttachment(this, m, id, slot, scale);
		children.push_back(CanvasAttachmentPtr(att));
		return att;
	}
	else {
		return NULL;
	}
}

CanvasAttachment* CanvasAttachment::addChild(ParaXEntity *m, int id, int slot, float scale)
{
	if (m && id >= 0) {
		CanvasAttachment *att = new CanvasAttachment(this, m, id, slot, scale);
		children.push_back(CanvasAttachmentPtr(att));
		return att;
	}
	else {
		return NULL;
	}
}

void CanvasAttachment::AttachChild(CanvasAttachment* att)
{
	if (att != NULL)
	{
		att->parent = this;
		children.push_back(CanvasAttachmentPtr(att));
	}
}

void CanvasAttachment::delChildren()
{
	children.clear();
}

IAttributeFields * CanvasAttachment::GetAttributeObject()
{
	if (m_pMeshObject)
	{
		return m_pMeshObject.get();
	}
	return NULL;
}

CanvasAttachment* CanvasAttachment::GetChild(int id, int slot /*= -1*/)
{
	for (size_t i = 0; i < children.size(); ++i) {
		if ((children[i]->id == id) && (children[i]->slot == slot)) {
			return children[i].get();
		}
	}
	return NULL;
}

void CanvasAttachment::delChildByID(int nID)
{
	for (size_t i = 0; i < children.size();) {
		if (children[i]->id == nID) {
			children.erase(children.begin() + i);
		}
		else i++;
	}
}
void CanvasAttachment::delSlot(int slot)
{
	for (size_t i = 0; i < children.size();) {
		if (children[i]->slot == slot) {
			children.erase(children.begin() + i);
		}
		else i++;
	}
}

bool CanvasAttachment::animate(SceneState * sceneState, CharacterPose* pPose, bool bUseGlobal, int nAnimID, IAttributeFields* pAnimInstance)
{
	bool res = true;
	// CharacterPose* pPoseInfo=(m_bIsAutoCharacter)?pPose:NULL;
	if (model != 0)
	{
		int nIndex = (sceneState && sceneState->IsLODEnabled()) ? model->GetLodIndex(sceneState->GetCameraToCurObjectDistance()) : 0;
		CParaXModel* pModel = model->GetModel(nIndex);
		if (pModel != NULL)
		{
			if (bUseGlobal && pModel->HasAnimation())
			{
				// play the specified animation if exist, otherwise use standing animation with the global timer. 
				int nAnimID_ = nAnimID;
				do {
					int nAnimIndex = pModel->GetAnimIndexByID(nAnimID_).nIndex;
					if (nAnimIndex >= 0 || nAnimID_ == 0)
						break;
					nAnimID_ = CAnimTable::GetDefaultAnimIDof(nAnimID_);
				} while (true);

				pModel->m_CurrentAnim = pModel->GetAnimIndexByID(nAnimID_);
				if (!(pModel->m_CurrentAnim.IsValid()))
				{
					pModel->m_CurrentAnim = pModel->GetAnimIndexByID(ANIM_STAND);
				}
				const AnimIndex& animIndex = pModel->m_CurrentAnim;
				pModel->m_CurrentAnim.nCurrentFrame = (sceneState != 0 && animIndex.nEndFrame > animIndex.nStartFrame) ? ((int)(sceneState->GetGlobalTime()) % (animIndex.nEndFrame - animIndex.nStartFrame) + animIndex.nStartFrame) : 0;
				pModel->m_NextAnim.nIndex = 0;
				pModel->m_BlendingAnim.MakeInvalid();
				pModel->blendingFactor = 0;
			}

			// animate its children
			pModel->animate(sceneState, pPose, pAnimInstance);
		}
	}

	for (size_t i = 0; i < children.size(); i++)
	{
		res = res && children[i]->animate(sceneState, pPose, true, nAnimID, pAnimInstance);
	}
	return res;
}

// child models are not rendered.
void CanvasAttachment::BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
#ifdef USE_DIRECTX_RENDERER
	if (!model)
		return;

	float fCameraToObjectDist = sceneState->GetCameraToCurObjectDistance();
	// Push matrix: set up transforms for this attached model.
	bool bNeedPop = SetupParantTransform(sceneState, fCameraToObjectDist);

	// scale the model
	Matrix4 mat, matScale;
	if (scale != 1.f)
	{
		ParaMatrixScaling(&matScale, scale, scale, scale);
		mat = matScale * CGlobals::GetWorldMatrixStack().SafeGetTop();
	}
	else
	{
		mat = CGlobals::GetWorldMatrixStack().SafeGetTop();
	}
	mat._41 += m_vOffset.x;
	mat._42 += m_vOffset.y;
	mat._43 += m_vOffset.z;

	CGlobals::GetWorldMatrixStack().push(mat);

	// draw this model
	int nIndex = (sceneState && sceneState->IsLODEnabled()) ? model->GetLodIndex(fCameraToObjectDist) : 0;

	CParaXModel* pModel = model->GetModel(nIndex);
	if (pModel)
	{
		pModel->BuildShadowVolume(pShadowVolume, pLight, &mat);
	}

	// TODO: draw its children
	//for (size_t i=0; i<children.size(); i++) {
	//	children[i]->BuildShadowVolume(pShadowVolume, pLight, &mat);
	//}

	// pop matrix
	CGlobals::GetWorldMatrixStack().pop();
	if (bNeedPop)
		CGlobals::GetWorldMatrixStack().pop();
#endif
}

CParameterBlock * CanvasAttachment::GetParamBlock(bool bCreateIfNotExist)
{
	if (m_pParamBlock)
	{
		return m_pParamBlock;
	}
	else
	{
		if (bCreateIfNotExist)
			m_pParamBlock = new CParameterBlock();
		return m_pParamBlock;
	}
}

void CanvasAttachment::SetOffset(float x, float y, float z)
{
	m_vOffset.x = x;
	m_vOffset.y = y;
	m_vOffset.z = z;
}

void CanvasAttachment::draw(SceneState * sceneState, ParaXModelCanvas *c, CParameterBlock* materialParams)
{
	if (model == 0 && !m_pMeshObject)
		return;

	float fCameraToObjectDist = sceneState->GetCameraToCurObjectDistance();
	// Push matrix: set up transforms for this attached model.
	bool bNeedPop = SetupParantTransform(sceneState, fCameraToObjectDist);
	// scale the model
	Matrix4 mat, matTmp;
	if (scale != 1.f)
	{
		ParaMatrixScaling(&matTmp, scale, scale, scale);
		mat = matTmp * CGlobals::GetWorldMatrixStack().SafeGetTop();
	}
	else
	{
		mat = CGlobals::GetWorldMatrixStack().SafeGetTop();
	}
	Vector3 vOffset = m_vOffset;
	if (m_pMeshObject)
	{
		Vector3 vPos = m_pMeshObject->GetPosition();
		vOffset += vPos;
	}
	if (vOffset.x != 0 || vOffset.y != 0 || vOffset.z != 0)
	{
		matTmp.makeTrans(vOffset.x, vOffset.y, vOffset.z);
		mat = matTmp * mat;
	}


	CGlobals::GetWorldMatrixStack().push(mat);

	// draw this model
	if (model != 0)
	{
		int nIndex = (sceneState && sceneState->IsLODEnabled()) ? model->GetLodIndex(fCameraToObjectDist) : 0;
		CParaXModel* pModel = model->GetModel(nIndex);

		if (pModel)
		{
			for (auto const & tex_pair : texReplaceables)
			{
				pModel->replaceTextures[tex_pair.first] = tex_pair.second.get();
			}

			CParameterBlock * param_block = materialParams;
			if (materialParams && GetParamBlock())
			{
				param_block = new CParameterBlock(*materialParams);
				for (auto const & param : GetParamBlock()->m_params)
					param_block->SetParameter(param.first, param.second);
			}
			else if (GetParamBlock())
			{
				param_block = GetParamBlock();
			}

			pModel->draw(sceneState, param_block);

			if (materialParams && GetParamBlock())
				delete param_block;

			for (auto const & tex_pair : texReplaceables)
			{
				pModel->replaceTextures[tex_pair.first] = nullptr;
			}
		}
	}
	if (m_pMeshObject)
	{
		for (auto const & tex_pair : texReplaceables)
		{
			m_pMeshObject->SetReplaceableTexture(tex_pair.first, tex_pair.second.get());
		}
		m_pMeshObject->DrawInner(sceneState, NULL, fCameraToObjectDist, materialParams);
	}

	// draw its children
	for (size_t i = 0; i < children.size(); i++) {
		children[i]->draw(sceneState, c, materialParams);
	}

	// pop matrix
	CGlobals::GetWorldMatrixStack().pop();
	if (bNeedPop)
		CGlobals::GetWorldMatrixStack().pop();
}


int ParaEngine::ParaXModelCanvas::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	return S_OK;
}

IAttributeFields* ParaEngine::ParaXModelCanvas::GetChildAttributeObject(const char * sName)
{
	return m_root.get();
}

IAttributeFields* ParaEngine::ParaXModelCanvas::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		return m_root.get();
	}
	return NULL;
}

int ParaEngine::ParaXModelCanvas::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return m_root ? 1 : 0;
	return 0;
}

int ParaEngine::ParaXModelCanvas::GetChildAttributeColumnCount()
{
	return 1;
}

int ParaEngine::CanvasAttachment::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	return S_OK;
}

IAttributeFields* ParaEngine::CanvasAttachment::GetChildAttributeObject(const char * sName)
{
	return NULL;
}

IAttributeFields* ParaEngine::CanvasAttachment::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		return (nRowIndex < (int)children.size()) ? children[nRowIndex].get() : NULL;
	}
	return NULL;
}

int ParaEngine::CanvasAttachment::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return (int)children.size();
	return 0;
}

int ParaEngine::CanvasAttachment::GetChildAttributeColumnCount()
{
	return 1;
}





