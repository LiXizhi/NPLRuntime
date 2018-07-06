#include "AutoRigger.h"
#include "pinocchioApi.h"
#include "ParaWorldAsset.h"
#include "Matcher.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/ParaXModel.h"

#include <thread>
#include <functional>

using namespace ParaEngine;

class RigHelper
{
public:
	static Mesh* ExtractPataXMesh(CParaXModel* xmodel)
	{
		Mesh* pMesh = new Mesh();	
		int numVert = xmodel->m_objNum.nVertices;
		pMesh->vertices.resize(numVert);
		for (int i = 0; i < numVert; ++i) {
			Vector3& v = xmodel->m_origVertices[i].pos;
			pMesh->vertices[i].pos = PVector3(v.x, v.y, v.z);
		}

		int numEdges = xmodel->m_objNum.nIndices;
		pMesh->edges.resize(numEdges);
		for (int i = 0; i < numEdges; i++) {
			int ver = (i - i % 3) + (i + 1) % 3;
			pMesh->edges[i].vertex = xmodel->m_indices[ver];
		}

		for (int i = 0; i < (int)pMesh->edges.size(); ++i) { //make sure all vertex indices are valid
			if (pMesh->edges[i].vertex < 0 || pMesh->edges[i].vertex >= numVert) {
				Debugging::out() << "Error: invalid vertex index " << pMesh->edges[i].vertex << endl;
				OUT;
			}
		}

		pMesh->fixDupFaces();

		pMesh->computeTopology();

		if (pMesh->integrityCheck())
			int a = 0;// place holder
		else
			int b = 0;// place holder
		pMesh->normalizeBoundingBox();
		pMesh->computeVertexNormals();

		return pMesh;
	}

	static Skeleton* ExtractPataXSkeleton(CParaXModel* xmodel)
	{
		Bone* bones = xmodel->bones;
		int numBones = xmodel->m_objNum.nBones;
		std::vector<int> refMap;
		refMap.resize(numBones);
		for (int i = 0; i < numBones; ++i) {
			int parent = (bones+i)->parent;
			++refMap[bones[i].parent];
		}
		
		std::vector<Bone*> orderedBones;
		orderedBones.reserve(numBones);
		for (int i = 0; i < refMap.size(); ++i) {
			int index = -1,  val = -1;
			for (int j = 0; j < refMap.size(); ++j) {
				if (val <= refMap[j]) {
					val = refMap[j];
					index = j;
				}
			}
			orderedBones.push_back(&bones[index]);
			refMap[index] = -2;
		}
		Skeleton* skeleton = new Skeleton();
		for (int i = 0; i < numBones; ++i) {
			Bone* bone = orderedBones[i];
			std::string parentName;
			if (bone->GetParentIndex() > 0) {
				parentName = bones[bone->GetParentIndex()].GetName();
			}
			const Vector3& v = bone->GetPivotPoint();
			PVector3 pivot(v.x, v.y, v.z);
			skeleton->makeJoint(bone->GetName(), pivot, parentName);
		}

		//symmetry
		//makeSymmetric("lthigh", "rthigh");
		//makeSymmetric("lhknee", "rhknee");
		//makeSymmetric("lhfoot", "rhfoot");

		skeleton->initCompressed();

		//setFoot("lhfoot");
		//setFoot("rhfoot");
		//setFoot("lffoot");
		//setFoot("rffoot");

		//setFat("hips");
		//setFat("shoulders");
		//setFat("head");

		return skeleton;
	}
};

CAutoRigger::CAutoRigger()
	:m_pTargetModel(nullptr)
	, m_ModelTemplates(new ModelTemplateMap())
{
}

CAutoRigger::~CAutoRigger()
{
}

void CAutoRigger::AddModelTemplate(const char* fileName)
{
	ModelTemplateMap::iterator iter = m_ModelTemplates->find(fileName);
	if (iter == m_ModelTemplates->end()) {
		(*m_ModelTemplates)[fileName] = CParaWorldAsset::GetSingleton()->LoadParaX("", std::string(fileName));
		(*m_ModelTemplates)[fileName]->LoadAsset();
	}
}

void CAutoRigger::RemoveModelTemplate(const char* fileName)
{
	ModelTemplateMap::iterator iter = m_ModelTemplates->find(fileName);
	if (iter != m_ModelTemplates->end()) m_ModelTemplates->erase(iter);
}

void CAutoRigger::SetTargetModel(const char* fileName)
{
	m_pTargetModel = CParaWorldAsset::GetSingleton()->LoadParaX("", std::string(fileName));
	m_pTargetModel->LoadAsset();
}

void CAutoRigger::SetThreshold()
{
	OUTPUT_LOG("SetThreshold \n");
}

void CAutoRigger::AutoRigModel()
{
	std::thread workThread = std::thread(std::bind(&CAutoRigger::AutoRigThreadFunc, this));
	workThread.detach();
}

void CAutoRigger::Clear()
{
	m_ModelTemplates->clear();
	m_pTargetModel = nullptr;
}

int CAutoRigger::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);
	pClass->AddField("AddModelTemplate", FieldType_String, (void*)AddModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("RemoveModelTemplate", FieldType_String, (void*)RemoveModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SetTargetModel", FieldType_String, (void*)SetTargetModel_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("AutoRigModel", FieldType_String, (void*)AutoRigModel_s, (void*)0, NULL, "", bOverride);
	return S_OK;
}

void CAutoRigger::AutoRigThreadFunc()
{
	this->addref();
	if (m_pTargetModel == nullptr || m_ModelTemplates->empty()) return;
	while (!m_pTargetModel->IsLoaded()) std::this_thread::sleep_for(std::chrono::milliseconds(25));
	Matcher matcher;

	// constructing target model feature
	matcher.SetModelFeatureType(0);
	CParaXModel* target = m_pTargetModel->GetModel();
	const int numVert = target->m_objNum.nVertices;
	for (int i = 0; i < numVert; ++i) {
		Vector3& v = target->m_origVertices[i].pos;
		Vector3& n = target->m_origVertices[i].normal;
		matcher.AddFeature(v.x, v.y, v.z, n.x, n.y, n.z);
	}
	matcher.CloseFeature();

	int matchness = -1;
	ModelTemplateMap::iterator bestMatch = m_ModelTemplates->end();
	ModelTemplateMap::iterator iter = m_ModelTemplates->begin();
	for (; iter != m_ModelTemplates->end(); ++iter) {
		while (!iter->second->IsLoaded()) std::this_thread::sleep_for(std::chrono::milliseconds(25));
		// constructing source model feature
		matcher.SetModelFeatureType(1);
		CParaXModel* source = iter->second->GetModel();
		const int numVert = source->m_objNum.nVertices;
		for (int i = 0; i < numVert; ++i) {
			Vector3& v = source->m_origVertices[i].pos;
			Vector3& n = source->m_origVertices[i].normal;
			matcher.AddFeature(v.x, v.y, v.z, n.x, n.y, n.z);
		}
		matcher.CloseFeature();
		int temp = matcher.Match();
		if (matchness < temp) {
			matchness = temp;
			bestMatch = iter;
		}
	}

	// auto rigging using the matched source model bones for the target model
	if (bestMatch != m_ModelTemplates->end()) {
		Skeleton* given = RigHelper::ExtractPataXSkeleton(bestMatch->second->GetModel());
		Mesh* mesh = RigHelper::ExtractPataXMesh(bestMatch->second->GetModel());
		PinocchioOutput rig = autorig(*given, *mesh);
		
		int numBones = bestMatch->second->GetModel()->m_objNum.nBones;
		Bone* bones = bestMatch->second->GetModel()->bones;
		for (int i = 0; i < numBones; ++i) {
			bones[i].pivot.x = rig.embedding[i][0];
			bones[i].pivot.y = rig.embedding[i][1];
			bones[i].pivot.z = rig.embedding[i][2];
		}
	}
	this->delref();
}

void CAutoRigger::Rigging(CParaXModel* targetModel, CParaXModel* skeletonModel)
{
	// header settings
	targetModel->m_header.type = skeletonModel->m_header.type;
	targetModel->m_header.IsAnimated = skeletonModel->m_header.IsAnimated;
	targetModel->animated = targetModel->m_header.IsAnimated > 0;
	targetModel->animGeometry = (targetModel->m_header.IsAnimated&(1 << 0)) > 0;
	targetModel->animTextures = (targetModel->m_header.IsAnimated&(1 << 1)) > 0;
	targetModel->animBones = (targetModel->m_header.IsAnimated&(1 << 2)) > 0;
	//to support arg channel only texture animation  -clayman 2011.8.5
	targetModel->animTexRGB = (targetModel->m_header.IsAnimated&(1 << 4)) > 0;
	if (targetModel->IsBmaxModel())
		targetModel->m_RenderMethod = CParaXModel::BMAX_MODEL;
	else if (targetModel->animated)
		targetModel->m_RenderMethod = CParaXModel::SOFT_ANIM;
	else
		targetModel->m_RenderMethod = CParaXModel::NO_ANIM;

	targetModel->m_objNum.nAnimations = skeletonModel->m_objNum.nAnimations;
	targetModel->m_objNum.nBones = skeletonModel->m_objNum.nBones;

	targetModel->bones = skeletonModel->bones;
	targetModel->texanims = skeletonModel->texanims;
	targetModel->anims = skeletonModel->anims;
	//targetModel->specialTextures = skeletonModel->specialTextures;

	targetModel->m_CurrentAnim.Reset(); 
	targetModel->m_NextAnim.MakeInvalid();
	targetModel->m_BlendingAnim.Reset();
	targetModel->blendingFactor = 0;
	targetModel->fBlendingTime = 0.25f;	// this is the default value.

	// skinning
	int numVert = targetModel->m_objNum.nVertices;
	for (int i = 0; i < numVert; ++i) {
		targetModel->m_origVertices[i].bones[0] = 2;
		targetModel->m_origVertices[i].weights[0] = 255;
	}

	targetModel->SaveToDisk("D:/Projects/3rdParty/npl_dev/Fork/NPLRuntime/Client/build/lib/Release/worlds/DesignHouse/test/morph_result2.x");
}