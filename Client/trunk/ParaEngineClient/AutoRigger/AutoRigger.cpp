#include "ParaEngine.h"
#include "AutoRigger.h"
#include "ParaWorldAsset.h"
#include "Matcher.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/ParaXModel.h"
#include "AISimulator.h"
#include "pinocchioApi.h"
#include "skeleton.h"
#include "ShapeAABB.h"

#include <thread>
#include <functional>
#include <list>
#include <fstream>
#include <algorithm>

using namespace ParaEngine;
using namespace Pinocchio;

const float VERY_SMALL = 1e-8;

struct MMData
{
	float data[4];
};
typedef std::vector<MMData> FieldData;

void ExtractPoints(TreeType::Node* node, FieldData& fieldData)
{
	if (node == nullptr) return;
	for (int i = 0; i < 8; ++i) {
		MMData e;
		const PVector3& v = node->getRect().getCorner(i);
		e.data[0] = (float)v[0];
		e.data[1] = (float)v[1];
		e.data[2] = (float)v[2];
		e.data[3] = (float)node->getValue(i);
		fieldData.push_back(e);
	}
	for (int i = 0; i < 8; ++i) {
		ExtractPoints(node->getChild(i), fieldData);
	}
}

class RigHelper
{
public:
	static void OutputTriangles(const CParaXModel* xmodel, const std::string& fileName)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < xmodel->m_objNum.nIndices; ++i) {
			int j = xmodel->m_indices[i];
			fout.write((char*)&(xmodel->m_origVertices[j].pos.x), sizeof(float));
			fout.write((char*)&(xmodel->m_origVertices[j].pos.y), sizeof(float));
			fout.write((char*)&(xmodel->m_origVertices[j].pos.z), sizeof(float));
		}
		fout.close();
	}

	static void OutputTriangles(const Mesh* pMesh, const std::string& fileName)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < pMesh->m_Edges.size(); ++i) {
			int j = pMesh->m_Edges[i].vertex;
			float x = (float)pMesh->m_Vertices[j].pos[0];
			float y = (float)pMesh->m_Vertices[j].pos[1];
			float z = (float)pMesh->m_Vertices[j].pos[2];

			fout.write((char*)&x, sizeof(float));
			fout.write((char*)&y, sizeof(float));
			fout.write((char*)&z, sizeof(float));
		}
		fout.close();
	}

	static void OutputVertices(const std::vector<PVector3>& vertices, const std::string& fileName, PVector3 offset = PVector3(), float scale = 1.0)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < (int)vertices.size(); ++i) {
			//vertices[i] = offset + vertices[i] * scale;
			float x = (float)vertices[i][0] * scale + offset[0];
			float y = (float)vertices[i][1] * scale + offset[0];
			float z = (float)vertices[i][2] * scale + offset[0];

			fout.write((char*)&x, sizeof(float));
			fout.write((char*)&y, sizeof(float));
			fout.write((char*)&z, sizeof(float));
		}
		fout.close();
	}

	static void OutputDistantField(TreeType* df)
	{
		FieldData data;
		ExtractPoints(df, data);

		// output original triangles
		std::string fileName("D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.field");
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < data.size(); ++i) {
			fout.write((char*)data[i].data, sizeof(MMData));
		}

		fout.close();
	}

	static void OutputBoneRelation(const Skeleton* given, const std::string& fileName)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < given->fGraph().verts.size(); ++i) {
			int j = given->fPrev()[i];
			if (j >= 0) {
				float x = (float)given->fGraph().verts[i][0];
				float y = (float)given->fGraph().verts[i][1];
				float z = (float)given->fGraph().verts[i][2];

				fout.write((char*)&x, sizeof(float));
				fout.write((char*)&y, sizeof(float));
				fout.write((char*)&z, sizeof(float));

				x = (float)given->fGraph().verts[j][0];
				y = (float)given->fGraph().verts[j][1];
				z = (float)given->fGraph().verts[j][2];

				fout.write((char*)&x, sizeof(float));
				fout.write((char*)&y, sizeof(float));
				fout.write((char*)&z, sizeof(float));
			}
		}
		fout.close();
	}

	static void OutputBoneRelation(const CParaXModel* skeletonModel, const std::string& fileName)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < skeletonModel->m_objNum.nBones; ++i) {
			int j = skeletonModel->bones[i].GetParentIndex();
			if (j >= 0) {
				float x = skeletonModel->bones[i].pivot.x;
				float y = skeletonModel->bones[i].pivot.y;
				float z = skeletonModel->bones[i].pivot.z;

				fout.write((char*)&x, sizeof(float));
				fout.write((char*)&y, sizeof(float));
				fout.write((char*)&z, sizeof(float));

				x = skeletonModel->bones[j].pivot.x;
				y = skeletonModel->bones[j].pivot.y;
				z = skeletonModel->bones[j].pivot.z;

				fout.write((char*)&x, sizeof(float));
				fout.write((char*)&y, sizeof(float));
				fout.write((char*)&z, sizeof(float));
			}
		}
		fout.close();
	}

	static void OutputSkinningRelation(const CParaXModel* targetModel, const std::string& fileName)
	{
		std::ofstream fout(fileName, std::ios::binary | std::ios::out);
		for (int i = 0; i < targetModel->m_objNum.nVertices; ++i) {
			if (targetModel->m_origVertices[i].bones[0] > 200) {
				// beyond 200 means this bone is unbinded
				continue;
			}
			float x = (float)targetModel->m_origVertices[i].pos[0];
			float y = (float)targetModel->m_origVertices[i].pos[1];
			float z = (float)targetModel->m_origVertices[i].pos[2];

			fout.write((char*)&x, sizeof(float));
			fout.write((char*)&y, sizeof(float));
			fout.write((char*)&z, sizeof(float));

			Vector3 piviot = targetModel->bones[targetModel->m_origVertices[i].bones[0]].pivot;

			x = piviot.x;
			y = piviot.y;
			z = piviot.z;

			fout.write((char*)&x, sizeof(float));
			fout.write((char*)&y, sizeof(float));
			fout.write((char*)&z, sizeof(float));
		}
		fout.close();
	}

	static Mesh* ExtractParaXMesh(CParaXModel* xmodel, bool rawMesh = false)
	{
		// squeeze points that very close to each other into one 
		int numVert = xmodel->m_objNum.nVertices;
		// refDis is 1/10 of cubic edge length
		// two points would be considered as one if the distance between each other less than refDis 
		// we keep this info in idx vector
		float refDis = xmodel->m_origVertices[0].pos.distance(xmodel->m_origVertices[1].pos)*0.1;
		std::vector<int> idx(numVert, 0);
		for (int i = 0; i < numVert; ++i) {
			Vector3& v = xmodel->m_origVertices[i].pos;
			idx[i] = i;
			for (int j = i - 1; j >= 0; --j) {
				Vector3& pre = xmodel->m_origVertices[j].pos;
				if (idx[j] == j && pre.distance(v) < refDis) {
					idx[i] = j;
					break;
				}
			}
		}

		// populate the squeezed points into mesh
		// why bother to squeeze points? cause we want a well-connected mesh
		// the original parax model has too many points in the same posiotn
		// which produces a lot redundant connections
		// thoese redundant connections make the mesh traversal almost impossible
		Mesh* pMesh = new Mesh();
		for (int i = 0; i < idx.size(); ++i) {
			if (idx[i] != i) continue;
			MeshVertex vertex;
			Vector3& v = xmodel->m_origVertices[i].pos;
			vertex.pos = PVector3(v.x, v.y, v.z);// Y up
			pMesh->m_Vertices.push_back(vertex);
		}

		// fix the indices after squeezing
		std::vector<int> fixValue(idx.size(), 0);
		for (int i = 0, j = 0; i < idx.size(); ++i) {
			fixValue[i] = j;
			if (idx[i] != i)++j;
		}
		std::vector<int> newIndices(xmodel->m_objNum.nIndices, -1);
		for (int i = 0; i < xmodel->m_objNum.nIndices; ++i) {
			int j = idx[xmodel->m_indices[i]];
			newIndices[i] = j - fixValue[j];
		}

#ifdef OUTPUT_DEBUG_FILE
		{
			// output the squeezed points for visulization
			std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.vert";
			RigHelper::OutputTriangles(pMesh, fileName);
		}
#endif // OUTPUT_DEBUG_FILE
	
		struct TRIANGLE {
			TRIANGLE() :_computedNormal(false) {}

			void ComputeNormal(Mesh* pMesh)
			{
				if (!_computedNormal) {
					PVector3 p01 = pMesh->m_Vertices[_v[1]].pos - pMesh->m_Vertices[_v[0]].pos;
					PVector3 p02 = pMesh->m_Vertices[_v[2]].pos - pMesh->m_Vertices[_v[0]].pos;
					_n = (p01 % p02).normalize();
					_computedNormal = true;
				}
			}

			bool IsOverlap(const TRIANGLE& rhs, Mesh* pMesh)
			{
				int sharedVerts = 0;
				int M[3] = { -1, -1, -1 };
				int N[3] = { -1, -1, -1 };
				for (int i = 0; i < 3 && sharedVerts < 2; ++i) {
					for (int j = 0; j < 3; ++j) {
						if (_v[i] == rhs._v[j]) {
							++sharedVerts;
							M[i] = j;
							N[j] = i;
						}
					}
				}

				if (sharedVerts < 2) {
					return false;
				}else if (sharedVerts > 2) {
					return true;
				}else {
					PVector3 p0, p1, p2;
					for (int i = 0; i < 3; ++i) {
						if (M[i] > 0) p0 = pMesh->m_Vertices[_v[i]].pos;
						if (M[i] < 0) p1 = pMesh->m_Vertices[_v[i]].pos;
						if (N[i] < 0) p2 = pMesh->m_Vertices[rhs._v[i]].pos;
					}
					double len = (_n % rhs._n).length();
					double dot = (p1 - p0) * (p2 - p0);
					return len < VERY_SMALL && dot > VERY_SMALL ? true : false;
				}


			}
			int _e[3];
			int _v[3];
			PVector3 _n;
			bool _computedNormal;
		};

		// populate the original edges and triangles
		std::vector<MeshEdge> originalEdges(newIndices.size(), MeshEdge());
		std::vector<TRIANGLE> tris(newIndices.size() / 3, TRIANGLE());
		for (int i = 0; i < newIndices.size(); ++i) {
			int id = i / 3;
			originalEdges[i].vertex = newIndices[i];
			tris[id]._e[i % 3] = i;
			tris[id]._v[i % 3] = newIndices[i];
		}

		// compute triangles normals
		for (int i = 0; i < tris.size(); ++i) {
			tris[i].ComputeNormal(pMesh);
		}

		// mark the triangles to remove
		std::vector<bool> TrisToRemove(tris.size(), false);
		for (int i = 0; i < tris.size(); ++i) {
			if (TrisToRemove[i])continue;
			int k = -1;
			for (int j = i + 1; j < tris.size(); ++j) {
				if (TrisToRemove[j])continue;
				if (tris[i].IsOverlap(tris[j], pMesh)) {
					k = j;
					break;
				}
			}
			if (k > 0) {
				TrisToRemove[i] = true;
				TrisToRemove[k] = true;
			}
		}

		// mark the edges to remove
		std::vector<bool> edgesToRemove(originalEdges.size(), false);
		for (int i = 0; i < tris.size(); ++i) {
			if (TrisToRemove[i]) {
				edgesToRemove[tris[i]._e[0]] = true;
				edgesToRemove[tris[i]._e[1]] = true;
				edgesToRemove[tris[i]._e[2]] = true;
			}
		}

		// populate the edges to pMesh after removing the undesired ones
		for (int i = 0; i < originalEdges.size(); ++i) {
			if (edgesToRemove[i])continue;
			pMesh->m_Edges.push_back(originalEdges[i]);
		}

		if (!rawMesh) {
			pMesh->fixDupFaces();
			pMesh->computeTopology();
			if (!pMesh->integrityCheck()) {
				OUTPUT_LOG("Failed to pass mesh integrity check!\n");
				delete pMesh;
				pMesh = nullptr;
				return pMesh;
			}

			pMesh->computeVertexNormals();
			pMesh->normalizeBoundingBox();

			if (pMesh->m_Vertices.empty()) {
				// delete the bad mesh 
				delete pMesh;
				pMesh = nullptr;
			}
#ifdef OUTPUT_DEBUG_FILE
			{
				// output the trimed triangles
				std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.mesh";
				RigHelper::OutputTriangles(pMesh, fileName);
			}
#endif // OUTPUT_DEBUG_FILE
		}

		return pMesh;
	}

	static Skeleton* ExtractPataXSkeleton(CParaXModel* xmodel, const PVector3& off, double scale)
	{
		Skeleton* skeleton = new Skeleton();

		Bone* bones = xmodel->bones;
		int numBones = xmodel->m_objNum.nBones;
		std::vector<int> refMap;
		refMap.resize(numBones);
		for (int i = 0; i < numBones; ++i) {
			int parent = (bones + i)->parent;
			++refMap[bones[i].parent];
		}
	
		std::list<Bone*> orderedBones;
		for (int i = 0; i < refMap.size(); ++i) {
			int index = -1, val = -1;
			for (int j = 0; j < refMap.size(); ++j) {
				if (val <= refMap[j]) {
					val = refMap[j];
					index = j;
				}
			}
			orderedBones.push_back(&bones[index]);
			refMap[index] = -2;
		}

		// move root node to beginning position
		for (std::list<Bone*>::iterator iter = orderedBones.begin();
			iter != orderedBones.end(); ++iter) {
			if ((*iter)->GetParentIndex() == -1 && iter != orderedBones.begin()) {
				orderedBones.insert(orderedBones.begin(), *iter);
				orderedBones.erase(iter);
				break;
			}
		}
		
		orderedBones.reverse();	
		// sort by parent index
		for (std::list<Bone*>::iterator iter = orderedBones.begin(); 
			iter != orderedBones.end(); ++iter) {
			bool parentAhead = false;
			std::list<Bone*>::iterator fIter = iter;
			while (++fIter != orderedBones.end()) {
				if ((*fIter)->GetBoneIndex() == (*iter)->GetParentIndex()) {
					parentAhead = true;
					break;
				}
			}
			if (!parentAhead) {
				std::list<Bone*>::iterator bIter = orderedBones.begin();
				while (bIter != iter) {
					if ((*bIter)->GetBoneIndex() == (*iter)->GetParentIndex()) {
						std::list<Bone*>::iterator ffIter = fIter;
						++ffIter;
						orderedBones.insert(ffIter, *bIter);
						orderedBones.erase(bIter);
						break;
					}
					++bIter;
				}
			}
		}
		
		orderedBones.reverse();
		
		for (std::list<Bone*>::iterator iter = orderedBones.begin(); iter != orderedBones.end(); ++iter) {
			Bone* bone = *iter;
			std::string parentName;
			if (bone->GetParentIndex() > 0) {
				parentName = bones[bone->GetParentIndex()].GetName();
			}
			const Vector3& v = bone->GetPivotPoint();
			PVector3 pivot(v.x, v.y, v.z);// Y up
			pivot = off + pivot * scale;
			skeleton->MakeJoint(bone->GetName(), pivot, parentName);
		}

		if (!skeleton->IsWellConnected()) {
			delete skeleton;
			skeleton = nullptr;
			return skeleton;
		}

		// this method must called before InitCompressed()
		skeleton->MakeSymmetric();

		// this method must called before MarkLabels()
		skeleton->InitCompressed();

		skeleton->MarkLabels();

		return skeleton;
	}

	static void RefineEmbedding(CParaXModel* xmodel, Mesh* m, vector<PVector3>& embedding, VisTester<TreeType>* tester)
	{
		std::vector<PVector3> vertices;
		int numVert = xmodel->m_objNum.nVertices;
		for (int i = 0; i < numVert; i++) {
			Vector3& v = xmodel->m_origVertices[i].pos;
			PVector3 p(v.x, v.y, v.z);
			vertices.push_back(p);
		}

		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] = vertices[i] * m->m_Scale + m->m_ToAdd;
		}

#ifdef OUTPUT_DEBUG_FILE
		{
			// output the trimed triangles
			std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.inter";
			RigHelper::OutputVertices(vertices, fileName);
		}
#endif // OUTPUT_DEBUG_FILE

		double cubeEdgeLength = (vertices[1] - vertices[0]).length();
		typedef std::pair<int, double> DisIndexPair;
		for (int i = 0; i < embedding.size(); ++i) {
			std::vector<DisIndexPair> disList;
			for (int j = 0; j < vertices.size(); ++j) {
				DisIndexPair dis;
				dis.first = j;
				dis.second = (vertices[j] - embedding[i]).length();
				disList.push_back(dis);
			}
			std::sort(disList.begin(), disList.end(), [](DisIndexPair& p1, DisIndexPair& p2) -> bool {return p1.second < p2.second; });

			DisIndexPair nearest[8];
			nearest[0] = disList[0];
			for (int j = 1, k = 1; k < 8 && j < disList.size(); ++j) {
				bool needContinue = false;
				for (int t = k; t > 0; --t) {
					double dis = (vertices[disList[j].first] - vertices[nearest[t].first]).length();
					if (dis < cubeEdgeLength*0.1) {
						// we ignore the points that too close
						needContinue = true;
						break;
					}
				}
				if (needContinue)continue;

				PVector3 v = vertices[disList[j].first] - embedding[i];
				v[0] = std::abs(v[0]) - cubeEdgeLength;
				v[1] = std::abs(v[1]) - cubeEdgeLength;
				v[2] = std::abs(v[2]) - cubeEdgeLength;

				if (v[0] > 0.0 || v[1] > 0.0 || v[2] > 0.0) continue;

				nearest[k++] = disList[j];
			}

			Rect3 bb;
			for (int j = 0; j < 8; ++j) {
				bb |= vertices[nearest[j].first];
			}
			//avg /= 8.0;
			embedding[i] = bb.getCenter();
		}
	}

	static void NormalizeVertices(vector<PVector3>& verts)
	{
		Rect3 boundingBox = Rect3(verts.begin(), verts.end());
		PVector3 extents = boundingBox.getSize();
		PVector3 center = boundingBox.getCenter();
		double scale = std::max(std::max(extents[0], extents[1]), extents[2]);
		std::transform(verts.begin(), verts.end(), verts.begin(),
			[center, scale](const PVector3& v) { return (v - center) / scale; });
	}

	static bool RayIntersectsTriangle(Vector3 rayOrigin,
		Vector3 rayVector,
		int tri,
		CParaXModel* xmodel,
		Vector3& outIntersectionPoint)
	{
		const float EPSILON = 0.0000001;
		Vector3 v0 = xmodel->m_origVertices[xmodel->m_indices[tri]].pos;
		Vector3 v1 = xmodel->m_origVertices[xmodel->m_indices[tri + 1]].pos;
		Vector3 v2 = xmodel->m_origVertices[xmodel->m_indices[tri + 2]].pos;
		Vector3 e1, e2, h, s, q;
		float a, f, u, v;
		e1 = v1 - v0;
		e2 = v2 - v0;
		h = rayVector.crossProduct(e2);
		a = e1.dotProduct(h);
		if (std::abs(a) < EPSILON)return false;

		f = 1 / a;
		s = rayOrigin - v0;
		u = f * (s.dotProduct(h));
		if (u < 0.0 || u > 1.0)return false;

		q = s.crossProduct(e1);
		v = f * rayVector.dotProduct(q);
		if (v < 0.0 || u + v > 1.0)return false;

		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * e2.dotProduct(q);
		if (t > EPSILON) {// ray intersection
			outIntersectionPoint = rayOrigin + rayVector * t;
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
			return false;
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
	m_pTargetModel->SetMergeCoplanerBlockFace(false);
	m_pTargetModel->LoadAsset();
}

void  CAutoRigger::SetOutputFilePath(const char* filePath)
{
	m_OutputFilePath = filePath;
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

void CAutoRigger::On_AddRiggedFile()
{
	ScriptCallback* pCallback = GetScriptCallback(Type_AddRiggedFile);
	if (pCallback)
	{
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		CGlobals::GetAISim()->NPLActivate(sFile.c_str(), sCode.c_str(), sCode.size());
	}
}

int CAutoRigger::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);
	pClass->AddField("AddModelTemplate", FieldType_String, (void*)AddModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("RemoveModelTemplate", FieldType_String, (void*)RemoveModelTemplate_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SetTargetModel", FieldType_String, (void*)SetTargetModel_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SetOutputFilePath", FieldType_String, (void*)SetOutputFilePath_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("AutoRigModel", FieldType_String, (void*)AutoRigModel_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("On_AddRiggedFile", FieldType_String, (void*)SetAddRiggedFile_s, (void*)GetAddRiggedFile_s, NULL, "", bOverride);
	return S_OK;
}

CAutoRigger::ModelTemplateMap::iterator CAutoRigger::FindBestMatch(Mesh* targetMesh)
{
	Matcher matcher;
	// constructing target model feature
	matcher.SetModelFeatureType(0);
	const int numVert = targetMesh->m_Vertices.size();
	for (int i = 0; i < numVert; ++i) {
		PVector3 v = targetMesh->m_Vertices[i].pos;
		PVector3 n = targetMesh->m_Vertices[i].normal;
		v = (v - targetMesh->m_ToAdd) / targetMesh->m_Scale;
		matcher.AddFeature(v[0], v[1], v[2], n[0], n[1], n[2]);
	}

	//CParaXModel* target = m_pTargetModel->GetModel();
	//const int numVert = target->m_objNum.nVertices;
	//for (int i = 0; i < numVert; ++i) {
	//	Vector3& v = target->m_origVertices[i].pos;
	//	Vector3& n = target->m_origVertices[i].normal;
	//	matcher.AddFeature(v.x, v.y, v.z, n.x, n.y, n.z);
	//}

	//matcher.CloseFeature();

	// match the most close model template to the target model
	int matchness = std::numeric_limits<int>::max();
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
		//matcher.CloseFeature();
		int temp = matcher.Match();
		if (matchness > temp) {
			matchness = temp;
			bestMatch = iter;
		}
	}
	
	return bestMatch;
}

CAutoRigger::ModelTemplateMap::iterator CAutoRigger::FindBestMatch2(Mesh* targetMesh)
{
	//Matcher matcher;
	//// normalize the target mesh vertices
	//vector<PVector3> targetVerts;
	//targetVerts.reserve(targetMesh->m_Vertices.size());
	//for (int i = 0; i < (int)targetMesh->m_Vertices.size(); ++i) {
	//	targetVerts.push_back(targetMesh->m_Vertices[i].pos);
	//}
	//RigHelper::NormalizeVertices(targetVerts);

	// match the most close model template to the target model
	// we take the minmum matchness as the best match candidate
	double matchness = std::numeric_limits<double>::max();
	TreeType* tarDistField = ConstructDistanceField(*targetMesh);
	ModelTemplateMap::iterator bestMatch = m_ModelTemplates->end();
	ModelTemplateMap::iterator iter = m_ModelTemplates->begin();
	for (; iter != m_ModelTemplates->end(); ++iter) {
		while (!iter->second->IsLoaded()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
		// constructing source model feature	
		CParaXModel* source = iter->second->GetModel();
		Mesh* srcMesh = RigHelper::ExtractParaXMesh(source, true);
		srcMesh->normalizeBoundingBox();

		double curDisSum = 0.0;
		for (int i = 0; i < srcMesh->m_Vertices.size(); ++i) {
			PVector3 v = srcMesh->m_Vertices[i].pos;
			double dis = tarDistField->locate(v)->evaluate(v);
			curDisSum += std::abs(dis);
		}
		//curDisSum /= srcMesh->m_Vertices.size();

		if (matchness > curDisSum) {
			matchness = curDisSum;
			bestMatch = iter;
		}
		
		delete srcMesh;
	}
	delete tarDistField;
	
	return bestMatch;
	
}

void CAutoRigger::AutoRigThreadFunc()
{
	this->addref();

	if (m_pTargetModel == nullptr || m_ModelTemplates->empty()) {
		this->delref();
		return;
	}

	while (!m_pTargetModel->IsLoaded()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	Mesh* targetMesh = RigHelper::ExtractParaXMesh(m_pTargetModel->GetModel());
	if (targetMesh == nullptr) {
		OUTPUT_LOG( "Target bmax model yields a bad mesh. Try another model...\n");
		this->delref();
		return;
	}

	// auto rigging using the matched source model bones for the target model
	ModelTemplateMap::iterator bestMatch = this->FindBestMatch2(targetMesh);
	if (bestMatch != m_ModelTemplates->end()) {
		// prepare mesh
		Mesh newMesh = PrepareMesh(*targetMesh);
		if (newMesh.m_Vertices.empty()) {
			OUTPUT_LOG("Target mesh: failed to pass connection test.\n");
			this->delref();
			return;
		}
		// prepare skeleton
		Mesh* srcMesh = RigHelper::ExtractParaXMesh(bestMatch->second->GetModel(), true);
		srcMesh->normalizeBoundingBox();

		Skeleton* given = RigHelper::ExtractPataXSkeleton(bestMatch->second->GetModel(), srcMesh->m_ToAdd, srcMesh->m_Scale);
		if (given == nullptr) {
			OUTPUT_LOG("Failed to extract template parax model skeleton, %s.\n", bestMatch->second->GetAttributeClassName());
			this->delref();
			return;
		}
		
		TreeType* distanceField = ConstructDistanceField(newMesh);

		// output distant field 
		RigHelper::OutputDistantField(distanceField);

		std::vector<PSphere> medialSurface = SampleMedialSurface(distanceField);
		std::vector<PSphere> spheres = PackSpheres(medialSurface);
		PtGraph graph = ConnectSamples(distanceField, spheres);

		//discrete embedding
		std::vector<std::vector<int>> possibilities = ComputePossibilities(graph, spheres, *given);

		//constraints can be set by respecifying possibilities for skeleton joints:
		//to constrain joint i to sphere j, use: possiblities[i] = std::vector<int>(1, j);
		std::vector<int> embeddingIndices = DiscreteEmbed(graph, spheres, *given, possibilities);

		if (embeddingIndices.size() == 0) { // failure
			delete distanceField;
			OUTPUT_LOG("Failed to embed given seleton to target model.\n");
			this->delref();
			return;
		}

		PinocchioOutput rigger;
		rigger.embedding = SplitPaths(embeddingIndices, graph, *given);

		VisTester<TreeType>* tester = new VisTester<TreeType>(distanceField);
		RigHelper::RefineEmbedding(m_pTargetModel->GetModel(), targetMesh, rigger.embedding, tester);

		CParaXModel* skeletonModel = bestMatch->second->GetModel();
		
		// recover the bone pivots position from transformed coordinates
		std::map<string, int> boneNameIndexMap;
		for (int i = 0; i < skeletonModel->m_objNum.nBones; ++i) {
			boneNameIndexMap[skeletonModel->bones[i].GetName()] = i;
		}

		for (int i = 0; i < rigger.embedding.size(); ++i) {
			int j = boneNameIndexMap[given->m_IndexNameMap[i]];
			skeletonModel->bones[j].pivot.x = rigger.embedding[i][0];
			skeletonModel->bones[j].pivot.y = rigger.embedding[i][1];
			skeletonModel->bones[j].pivot.z = rigger.embedding[i][2];
		}

		this->FullRigging(m_pTargetModel->GetModel(), skeletonModel, newMesh, tester);

		this->On_AddRiggedFile();

#ifdef OUTPUT_DEBUG_FILE
		{
			// output the discreted graph
			std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.graph";
			RigHelper::OutputVertices(graph.verts, fileName);

			// output bone connections
			fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.boneconct";
			RigHelper::OutputBoneRelation(skeletonModel, fileName);

			// output the bones joints
			fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.bone";
			RigHelper::OutputVertices(rigger.embedding, fileName);

			// output distant field 
			RigHelper::OutputDistantField(distanceField);
		}
#endif // OUTPUT_DEBUG_FILE

		//cleanup
		delete srcMesh;
		delete tester;
		delete distanceField;
	}

	this->delref();
}

void CAutoRigger::RiggingWithoutColorAssignement(CParaXModel* targetModel, CParaXModel* skeletonModel, Mesh& newMesh, void* vistester)
{
	VisTester<TreeType>* tester = static_cast<VisTester<TreeType>*>(vistester);
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

										// modify vertices
	int countSee = 0;
	std::vector<ModelVertex> newVertices;
	newVertices.reserve(newMesh.m_Vertices.size());
	for (int i = 0; i < newMesh.m_Vertices.size(); ++i) {
		PVector3 v = newMesh.m_Vertices[i].pos;
		ModelVertex modelVertex;
		memset(&modelVertex, 0, sizeof(ModelVertex));
		modelVertex.pos.x = v[0];
		modelVertex.pos.y = v[1];
		modelVertex.pos.z = v[2];

		PVector3& n = newMesh.m_Vertices[i].normal;
		modelVertex.normal.x = n[0];
		modelVertex.normal.y = n[1];
		modelVertex.normal.z = n[2];

		modelVertex.color0 = 123;

		newVertices.push_back(modelVertex);
	}
	targetModel->initVertices(newVertices.size(), &(newVertices[0]));
	targetModel->geosets[0].icount = newVertices.size();


	// moddify indices
	vector<uint16> newIndices(newMesh.m_Edges.size(), 0);
	for (int i = 0; i < newMesh.m_Edges.size(); ++i) {
		newIndices[i] = (uint16)newMesh.m_Edges[i].vertex;
	}
	targetModel->initIndices(newIndices.size(), &(newIndices[0]));
	targetModel->passes[0].indexCount = newIndices.size();

	std::vector<int> omitedInFirstRound;
	// first round 
	for (int i = 0; i < targetModel->m_objNum.nVertices; ++i) {
		ModelVertex& modelVertex = targetModel->m_origVertices[i];
		int nearest = -1;
		float minDis = 999.0;
		for (int k = 0; k < targetModel->m_objNum.nBones; ++k) {
			Vector3 p = targetModel->bones[k].pivot;
			Vector3 v = modelVertex.pos;
			float dis = p.distance(v);

			v = p + (v - p)*0.95;

			bool canSee = tester->canSee(PVector3(v.x, v.y, v.z), PVector3(p.x, p.y, p.z));

			if (dis < minDis && canSee) {
				minDis = dis;
				nearest = k;
			}
		}
		if (nearest < 0) {
			// if couldn't find the direct bone just continue and wait for the next round to fixed it
			countSee++;
			modelVertex.bones[0] = 255;
			omitedInFirstRound.push_back(i);
			continue;
		}
		//int j = boneNameIndexMap[given->indexNameMap[nearest]];
		modelVertex.bones[0] = nearest;
		modelVertex.weights[0] = 255;
	}

	// second round
	for (int i = 0; i < omitedInFirstRound.size(); ++i) {
		int index = omitedInFirstRound[i];
		ModelVertex& modelVertex = targetModel->m_origVertices[index];
		int nearest = -1;
		float minDis = 1.0;
		for (int j = 0; j < targetModel->m_objNum.nVertices; ++j) {
			if (targetModel->m_origVertices[j].bones[0] > 200) {
				// beyond 200 means this bone slot is unbinded
				continue;
			}
			float dist = modelVertex.pos.distance(targetModel->m_origVertices[j].pos);
			if (dist < minDis) {
				nearest = j;
				minDis = dist;
			}
		}
		if (nearest > 0) {
			modelVertex.bones[0] = targetModel->m_origVertices[nearest].bones[0];
			modelVertex.weights[0] = 255;
		}
		else {
			modelVertex.bones[0] = 0;
			modelVertex.weights[0] = 255;
		}
	}

#ifdef OUTPUT_DEBUG_FILE
	std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.skin";
	RigHelper::OutputSkinningRelation(targetModel,fileName);
#endif

	// recover coordinates
	Vector3 offset((float)newMesh.m_ToAdd[0], (float)newMesh.m_ToAdd[1], (float)newMesh.m_ToAdd[2]);
	for (int i = 0; i < targetModel->m_objNum.nVertices; ++i) {
		ModelVertex& modelVertex = targetModel->m_origVertices[i];
		modelVertex.pos = (modelVertex.pos - offset) / newMesh.m_Scale;
	}
	for (int i = 0; i < targetModel->m_objNum.nBones; ++i) {
		targetModel->bones[i].pivot = (targetModel->bones[i].pivot - offset) / newMesh.m_Scale;
	}

	targetModel->SaveToDisk(m_OutputFilePath.c_str());
}

void CAutoRigger::FullRigging(CParaXModel* targetModel, CParaXModel* skeletonModel, Mesh& newMesh, void* vistester)
{
	VisTester<TreeType>* tester = static_cast<VisTester<TreeType>*>(vistester);
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


	// transform vertices into distance field space 
	int numVerts = targetModel->m_objNum.nVertices;
	Vector3 toAdd(newMesh.m_ToAdd[0], newMesh.m_ToAdd[1], newMesh.m_ToAdd[2]);
	for (int i = 0; i < numVerts; ++i) {
		//m_Vertices[i].pos = ctoAdd + m_Vertices[i].pos * cscale;
		Vector3& pos = targetModel->m_origVertices[i].pos;
		pos = toAdd + pos * newMesh.m_Scale;
	}
	float edgeLen = 1.05 * (targetModel->m_origVertices[0].pos - targetModel->m_origVertices[1].pos).length();
	
	struct Cube
	{
		int verts[32];
		Vector3 center;
		uint8 bones[4];
		DWORD color;// debug use
		Color cc;// debug use
		float neighborRange;
		bool rigged;

		bool IsNeighborOf(const Cube& other) 
		{
			return (this->center - other.center).length() < neighborRange;
		}

		bool IsSameColor(const Cube& other)
		{
			LinearColor c1(color);
			LinearColor c2(other.color);

			if (c1 == LinearColor::Black && c2 != LinearColor::Black || c1 != LinearColor::Black && c2 == LinearColor::Black) return false;

			std::vector<float> ratio(3, 0.0);
			if (c1.r > c2.r || c1.g > c2.g || c1.b > c2.b) {
				ratio[0] = std::abs((c2.r != 0.0) ? c1.r / c2.r : 0.0);
				ratio[1] = std::abs((c2.g != 0.0) ? c1.g / c2.g : 0.0);
				ratio[2] = std::abs((c2.b != 0.0) ? c1.b / c2.b : 0.0);
			}else {
				ratio[0] = std::abs((c1.r != 0.0) ? c2.r / c1.r : 0.0);
				ratio[1] = std::abs((c1.g != 0.0) ? c2.g / c1.g : 0.0);
				ratio[2] = std::abs((c1.b != 0.0) ? c2.b / c1.b : 0.0);
			}
			std::sort(ratio.begin(), ratio.end());

			float verySmall = 0.000001;

			if (ratio[0] > 0.0) {
				return (ratio[1] - ratio[0] < verySmall && ratio[2] - ratio[1] < verySmall) ? true : false;
			}else if (ratio[1] > 0.0) {
				return (ratio[2] - ratio[1] < verySmall) ? true : false;
			}else {// what if the color has only one component above zero? is still the same color? hope for the best.
				return true;
			}	
		}
	};
	typedef std::vector<Cube> CubeVector;
	CubeVector cubes;
	int numVertPerCube = 24;
	int numIndicesPerCube = 36;
	int numIndices = targetModel->m_objNum.nIndices;
	for (int i = 0; i < numIndices; i += numIndicesPerCube) {
		std::set<int> filter;
		for (int j = 0; j < numIndicesPerCube; ++j) {
			filter.insert((int)(targetModel->m_indices[i+j]));
		}
		if (filter.size() != numVertPerCube) {
			OUTPUT_LOG("Wrong cube vertex number.\n");
		}else {
			Cube cube;
			CShapeBox bb;
			std::set<int>::iterator iter = filter.begin();
			for (int k = 0; k < numVertPerCube; ++k, ++iter) {
				cube.verts[k] = (*iter);
				bb.Extend(targetModel->m_origVertices[(*iter)].pos);
			}
			cube.color = targetModel->m_origVertices[*(filter.begin())].color0;
			cube.cc = Color(cube.color);
			cube.center = bb.GetCenter();
			cube.rigged = false;
			cube.neighborRange = edgeLen;
			cubes.push_back(cube);
			PVector3 pv(cube.center.x, cube.center.y, cube.center.z );
		}
	}

	// first round 
	for (int i = 0; i < cubes.size(); ++i) {
		int nearest = -1;
		float minDis = 999.0;
		for (int k = 0; k < targetModel->m_objNum.nBones; ++k) {
			Vector3 p = targetModel->bones[k].pivot;
			Vector3 v = cubes[i].center;
			float dis = p.distance(v);
			v = p + (v - p)*0.95;
			bool canSee = tester->canSee(PVector3(v.x, v.y, v.z), PVector3(p.x, p.y, p.z));

			if (canSee && dis < minDis ) {
				minDis = dis;
				nearest = k;
			}
		}
		if (nearest >= 0 && minDis < 0.2*edgeLen) {
			cubes[i].rigged = true;
			cubes[i].bones[0] = (uint8)nearest;
			for (int j = 0; j < numVertPerCube; ++j) {
				targetModel->m_origVertices[cubes[i].verts[j]].bones[0] = nearest;
				targetModel->m_origVertices[cubes[i].verts[j]].weights[0] = 255;
			}
		}else {
			// if couldn't find the direct bone just continue and wait for the next round to fixed it
		}
	}// end for


	// second round
	typedef std::vector<std::vector<int>> ColorVector;
	ColorVector colors;
	for (int i = 0; i < cubes.size(); ++i) {
		bool clustered = false;
		for (auto& group : colors) {
			if (cubes[group[0]].IsSameColor(cubes[i])) {
				group.push_back(i);
				clustered = true;
				break;
			}
		}
		if (!clustered) {
			colors.resize(colors.size() + 1);
			colors.back().push_back(i);
		}
	}

	std::vector<std::vector<int>> clusters;
	for (ColorVector::iterator iter = colors.begin(); iter != colors.end(); ++iter) {
		std::vector<int>& blocks = *iter;
		while (!blocks.empty()) {
			std::vector<int> cluster;
			std::queue<int> todo;
			std::vector<bool> taken(blocks.size(), false);
			cluster.push_back(blocks[0]);
			todo.push(blocks[0]);
			taken[0] = true;
			while (!todo.empty()) {
				int cur = todo.front();
				todo.pop();
				// find one neighbor
				for (int i = 0; i < blocks.size(); ++i ) {
					if ( !taken[i] && cubes[blocks[i]].IsNeighborOf(cubes[cur])) {
						cluster.push_back(blocks[i]);
						todo.push(blocks[i]);
						taken[i] = true;
					}
				}
			}
			clusters.emplace_back(cluster);
			std::vector<int> remains;
			for (int i = 0; i < blocks.size(); ++i) {
				if (!taken[i])remains.push_back(blocks[i]);
			}
			blocks.swap(remains);	
		}// outer while
	}

	std::list<int> leftClusters;
	std::vector<int> riggedClusters;
	for (int c = 0; c < clusters.size(); ++c) {
		int rigged = -1;
		for (auto index : clusters[c]) {
			if (cubes[index].rigged) {
				rigged = index;
				break;
			}
		}

		if (rigged >= 0) {
			for (int i = 0; i < clusters[c].size(); ++i) {
				int idx = clusters[c][i];
				for (int j = 0; j < numVertPerCube; ++j) {
					targetModel->m_origVertices[cubes[idx].verts[j]].bones[0] = cubes[rigged].bones[0];
					targetModel->m_origVertices[cubes[idx].verts[j]].weights[0] = 255;
				}
				cubes[idx].rigged = true;
				cubes[idx].bones[0] = cubes[rigged].bones[0];
			}
			riggedClusters.push_back(c);
		}else {
			leftClusters.push_back(c);
		}
	}

	while (!leftClusters.empty()) {
		std::vector<int>& left = clusters[leftClusters.back()];
		int neighbor = -1;
		for (int i = 0; i < riggedClusters.size(); ++i) {
			std::vector<int>& riggedCluster = clusters[riggedClusters[i]];
			for (int j = 0; j < left.size(); ++j) {
				for (int k = 0; k < riggedCluster.size(); ++k) {
					if (cubes[left[j]].IsNeighborOf(cubes[riggedCluster[k]])) {
						neighbor = riggedClusters[i];
						break;
					}
				}
				if (neighbor >= 0)break;
			}
			if (neighbor >= 0)break;
		}
		if (neighbor >= 0) {
			int dst = clusters[neighbor].front();
			for (int i = 0; i < left.size(); ++i) {
				int idx = left[i];
				for (int j = 0; j < numVertPerCube; ++j) {
					targetModel->m_origVertices[cubes[idx].verts[j]].bones[0] = cubes[dst].bones[0];
					targetModel->m_origVertices[cubes[idx].verts[j]].weights[0] = 255;
				}
				cubes[idx].rigged = true;
				cubes[idx].bones[0] = cubes[dst].bones[0];
			}
		}
		leftClusters.pop_back();
	}

	if (0) {
		std::list<int> leftList;
		// first round 
		for (int i = 0; i < cubes.size(); ++i) {
			int nearest = -1;
			float minDis = 999.0;
			for (int k = 0; k < targetModel->m_objNum.nBones; ++k) {
				Vector3 p = targetModel->bones[k].pivot;
				Vector3 v = cubes[i].center;
				float dis = p.distance(v);
				v = p + (v - p)*0.95;
				bool canSee = tester->canSee(PVector3(v.x, v.y, v.z), PVector3(p.x, p.y, p.z));

				if (canSee && dis < minDis) {
					minDis = dis;
					nearest = k;
				}
			}
			if (nearest >= 0) {
				cubes[i].rigged = true;
				cubes[i].bones[0] = (uint8)nearest;
				for (int j = 0; j < numVertPerCube; ++j) {
					targetModel->m_origVertices[cubes[i].verts[j]].bones[0] = nearest;
					targetModel->m_origVertices[cubes[i].verts[j]].weights[0] = 255;
				}
			}
			else {
				// if couldn't find the direct bone just continue and wait for the next round to fixed it
				leftList.push_back(i);
			}
		}// end for


		while (!leftList.empty()) {
			std::list<int>::iterator iter = leftList.begin();
			int firstNearest = -1;
			for (; iter != leftList.end(); ++iter) {
				firstNearest = -1;
				std::vector<int> candidates;
				for (int i = 0; i < cubes.size(); ++i) {
					if (!cubes[i].rigged || i == *iter)continue;
					if ((cubes[i].center - cubes[*iter].center).length() < edgeLen /*&& cubes[i].color == cubes[*iter].color*/) {
						firstNearest = i;
						candidates.push_back(i);
					}
				}
				if (candidates.size() > 1) {

				}
				else {

				}
			}

			if (firstNearest >= 0) {
				for (int j = 0; j < numVertPerCube; ++j) {
					targetModel->m_origVertices[cubes[*iter].verts[j]].bones[0] = cubes[firstNearest].bones[0];
					targetModel->m_origVertices[cubes[*iter].verts[j]].weights[0] = 255;
				}
				cubes[*iter].rigged = true;
				cubes[*iter].bones[0] = cubes[firstNearest].bones[0];
				leftList.erase(iter);
			}
		}// end while
	}


#ifdef OUTPUT_DEBUG_FILE
	std::string fileName = "D:/Projects/3rdParty/OpenSceneGraph/bin/big_cube.skin";
	RigHelper::OutputSkinningRelation(targetModel, fileName);
#endif

	//// recover coordinates
	//Vector3 offset((float)newMesh.m_ToAdd[0], (float)newMesh.m_ToAdd[1], (float)newMesh.m_ToAdd[2]);
	//for (int i = 0; i < targetModel->m_objNum.nVertices; ++i) {
	//	ModelVertex& modelVertex = targetModel->m_origVertices[i];
	//	modelVertex.pos = (modelVertex.pos - offset) / newMesh.m_Scale;
	//}
	//for (int i = 0; i < targetModel->m_objNum.nBones; ++i) {
	//	targetModel->bones[i].pivot = (targetModel->bones[i].pivot - offset) / newMesh.m_Scale;
	//}

	targetModel->SaveToDisk(m_OutputFilePath.c_str());
}