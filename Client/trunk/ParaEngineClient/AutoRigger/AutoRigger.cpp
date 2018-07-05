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

		pMesh->fixDupFaces();

		pMesh->computeTopology();

		if (pMesh->integrityCheck())
			int a = 0;
		else
			int b = 0;
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

		}
	}
};

CAutoRigger::CAutoRigger()
	:m_pTargetModel(nullptr)
{
}

CAutoRigger::~CAutoRigger()
{
}

void CAutoRigger::AddModelTemplate(const char* fileName)
{
	ModelTemplateMap::iterator iter = m_ModelTemplates.find(fileName);
	if (iter == m_ModelTemplates.end()) {
		m_ModelTemplates[fileName] = CParaWorldAsset::GetSingleton()->LoadParaX("", std::string(fileName));
		m_ModelTemplates[fileName]->LoadAsset();
	}
}

void CAutoRigger::RemoveModelTemplate(const char* fileName)
{
	ModelTemplateMap::iterator iter = m_ModelTemplates.find(fileName);
	if (iter != m_ModelTemplates.end()) m_ModelTemplates.erase(iter); 
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
}

void CAutoRigger::Clear()
{
	m_ModelTemplates.clear();
	m_pTargetModel = nullptr;
}

int CAutoRigger::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);
	pClass->AddField("AddModelTemplate", FieldType_String, (void*)AddModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("RemoveModelTemplate", FieldType_String, (void*)RemoveModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SetTargetModel", FieldType_String, (void*)SetTargetModel_s, (void*)0, NULL, "", bOverride);
	return S_OK;
}

void CAutoRigger::AutoRigThreadFunc()
{
	if (m_pTargetModel == nullptr || m_ModelTemplates.empty()) return;
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
	ModelTemplateMap::iterator bestMatch = m_ModelTemplates.end();
	ModelTemplateMap::iterator iter = m_ModelTemplates.begin();
	for (; iter != m_ModelTemplates.end(); ++iter) {
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
	if (bestMatch != m_ModelTemplates.end()) {
		Skeleton given;
		Mesh mesh;
		PinocchioOutput rigOutput = autorig(given, mesh);

	}
}