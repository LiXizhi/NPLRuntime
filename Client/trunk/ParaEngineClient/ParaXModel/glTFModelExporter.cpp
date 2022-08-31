#include "ParaEngine.h"
#include "glTFModelExporter.h"
#include "BMaxModel/BlocksParser.h"
#include "ParaXModel.h"
#include "ParaXSerializer.h"
#include "StringHelper.h"
#include "StringBuilder.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"

namespace ParaEngine
{
	void glTFModelExporter::Init() 
	{
		maxVertex = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		minVertex = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxNormal = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		minNormal = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxColor0 = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		minColor0 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxColor1 = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		minColor1 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxCoord = Vector2(-FLT_MAX, -FLT_MAX);
		minCoord = Vector2(FLT_MAX, FLT_MAX);
		maxJoint = Vector4(0, 0, 0, 0);
		minJoint = Vector4(255, 255, 255, 255);
		maxWeight = Vector4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
		minWeight = Vector4(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		paraXModel = nullptr;
		animProvider = nullptr;
		bufferIndex = 0;
		bufferViewIndex = 0;
		accessorIndex = 0;
		nodeIndex = 0;
		meshIndex = 0;
		skinIndex = 0;
		animationIndex = 0;
		isBinary = false;
		buffer = std::make_shared<Buffer>();
		buffer->index = 0;
	}
	glTFModelExporter::glTFModelExporter(CParaXModel* mesh, bool binary)
	{
		Init();
		isBinary = binary;
		paraXModel = mesh;
		ParseParaXModel();
		ParseAnimationBones();
	}

	glTFModelExporter::glTFModelExporter(CParaXModel* mesh, CParaXModel* anim, bool binary)
	{
		Init();
		isBinary = binary;
		animProvider = anim;
		paraXModel = mesh;
		ParseParaXModel();
		ChangeAnimationBones();
	}

	glTFModelExporter::~glTFModelExporter()
	{

	}

	bool glTFModelExporter::IsExportAnimation()
	{ 
		return paraXModel->animated; 
	}

	uint32_t glTFModelExporter::GetAnimationCount() 
	{
		return paraXModel->m_objNum.nAnimations;	
	}
	void glTFModelExporter::ExportToFile(const std::string& filename)
	{
		fileName = filename;
		ExportMetadata();
		ExportScene();
		if (isBinary)
			WriteGLBFile();
		else
			WriteFile();
	}

	void glTFModelExporter::ParseParaXModel()
	{
		uint32_t numBones = paraXModel->m_objNum.nBones;
		float inverse = 1.0f / 255.0f;
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		for (uint32_t i = 0; i < numVertices; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			Vector3 v = vertex.pos;
			Vector3 n = vertex.normal;
			Vector2 coord = vertex.texcoords;
			DWORD color0 = vertex.color1;
			DWORD color1 = vertex.color0;
			for (uint32_t j = 0; j < 3; j++)
			{
				float val = v[j];
				if (val < minVertex[j]) minVertex[j] = val;
				if (val > maxVertex[j]) maxVertex[j] = val;
				val = n[j];
				if (val < minNormal[j]) minNormal[j] = val;
				if (val > maxNormal[j]) maxNormal[j] = val;
			}
			vertices.push_back(v);
			normals.push_back(n);

			for (uint32_t j = 0; j < 2; j++)
			{
				float val = coord[j];
				if (val < minCoord[j]) minCoord[j] = val;
				if (val > maxCoord[j]) maxCoord[j] = val;
			}
			texcoords.push_back(coord);

			if (paraXModel->m_header.type == PARAX_MODEL_STATIC || paraXModel->m_RenderMethod == CParaXModel::BMAX_MODEL)
			{
				float r = ((color0 >> 16) & 0xff) / 255.0f;
				float g = ((color0 >> 8) & 0xff) / 255.0f;
				float b = (color0 & 0xff) / 255.0f;
				if (r < minColor0[0]) minColor0[0] = r;
				if (g < minColor0[1]) minColor0[1] = g;
				if (b < minColor0[2]) minColor0[2] = b;
				if (r > maxColor0[0]) maxColor0[0] = r;
				if (g > maxColor0[1]) maxColor0[1] = g;
				if (b > maxColor0[2]) maxColor0[2] = b;
				colors0.push_back(Vector3(r, g, b));

				r = ((color1 >> 16) & 0xff) / 255.0f;
				g = ((color1 >> 8) & 0xff) / 255.0f;
				b = (color1 & 0xff) / 255.0f;
				if (r < minColor1[0]) minColor1[0] = r;
				if (g < minColor1[1]) minColor1[1] = g;
				if (b < minColor1[2]) minColor1[2] = b;
				if (r > maxColor1[0]) maxColor1[0] = r;
				if (g > maxColor1[1]) maxColor1[1] = g;
				if (b > maxColor1[2]) maxColor1[2] = b;
				colors1.push_back(Vector3(r, g, b));
			}

			uint8* bone = vertex.bones;
			Vector4 joint(bone[0], bone[1], bone[2], bone[3]);
			for (uint32_t j = 0; j < 4; j++)
			{
				float val = static_cast<float>(joint[j]);
				if (val < minJoint[j]) minJoint[j] = val;
				if (val > maxJoint[j]) maxJoint[j] = val;
			}

			Vector4 weight;
			for (uint32_t j = 0; j < 4; j++)
			{
				float val = vertex.weights[j] * inverse;
				weight[j] = val;
				if (val < minWeight[j]) minWeight[j] = val;
				if (val > maxWeight[j]) maxWeight[j] = val;
			}
			weights.push_back(weight);
		}

	}

	void glTFModelExporter::ParseAnimationBones()
	{
		float inverse = 1.0f / 1000.0f;
		//uint32_t animLength = paraXModel->anims[0].timeEnd - paraXModel->anims[0].timeStart;
		//if (animLength == 0) animLength = 1000;
		boneIndices.clear();
		animOffsets.clear();
		animTimes.clear();
		translations.clear();
		scales.clear();
		rotations.clear();
		uint32_t nAnimations = GetAnimationCount();
		bvTime.resize(nAnimations);
		bvTranslation.resize(nAnimations);
		bvRotation.resize(nAnimations);
		bvScale.resize(nAnimations);
		for (uint32_t animId = 0; animId < nAnimations; animId++)
		{
			std::vector<int> boneIndice;
			std::vector<uint32_t> animOffset;
			std::vector<float> animTime;
			std::vector<Vector3> translation;
			std::vector<Vector3> scale;
			std::vector<Quaternion> rotation;
			for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
			{
				Bone& bone = paraXModel->bones[i];
				if (bone.trans.used || bone.rot.used || bone.scale.used)
				{
					uint32_t firstT = bone.trans.ranges[animId].first;
					uint32_t secondT = bone.trans.ranges[animId].second;
					uint32_t firstR = bone.rot.ranges[animId].first;
					uint32_t secondR = bone.rot.ranges[animId].second;
					uint32_t firstS = bone.scale.ranges[animId].first;
					uint32_t secondS = bone.scale.ranges[animId].second;
					uint32_t offset1 = secondT - firstT + 1;
					uint32_t offset2 = secondR - firstR + 1;
					uint32_t offset3 = secondS - firstS + 1;
					uint32_t animStart = firstT;
					uint32_t animEnd = secondT;
					if (offset1 > offset3 && offset1 > offset2)
					{
						animStart = firstT;
						animEnd = secondT;
					}
					else if (offset2 > offset1 && offset2 > offset3)
					{
						animStart = firstR;
						animEnd = secondR;
					}
					else
					{
						animStart = firstS;
						animEnd = secondS;
					}

					animOffset.push_back(animEnd - animStart + 1);
					int time = 0;
					Vector3 t, s;
					Quaternion q;
					for (uint32_t j = animStart; j <= animEnd; j++)
					{
						if (firstT == animStart && secondT == animEnd)
						{
							time = bone.trans.times[j];
							t = bone.trans.data[j];
							s = bone.scale.getValue(animId, time);
							q = bone.rot.getValue(animId, time);
						}
						else if (firstR == animStart && secondR == animEnd)
						{
							time = bone.rot.times[j];
							t = bone.trans.getValue(animId, time);
							s = bone.scale.getValue(animId, time);
							q = bone.rot.data[j];
						}
						else if (firstS == animStart && secondS == animEnd)
						{
							time = bone.scale.times[j];
							t = bone.trans.getValue(animId, time);
							s = bone.scale.data[j];
							q = bone.rot.getValue(animId, time);
						}

						Matrix4 mat;
						if (bone.bUsePivot)
						{
							mat.makeTrans(bone.pivot * -1.0f);
							mat.setScale(s);
							mat.m[3][0] *= s.x;
							mat.m[3][1] *= s.y;
							mat.m[3][2] *= s.z;
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
							mat.offsetTrans(bone.pivot);
						}
						else
						{
							mat.makeScale(s);
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
						}

						time = time - paraXModel->anims[animId].timeStart;
						animTime.push_back(time * inverse);
						translation.push_back(mat.getTrans());
						rotation.push_back(q);
						scale.push_back(s);
					}
					boneIndice.push_back(bone.nIndex);
				}
			}

			boneIndices.push_back(boneIndice);
			animOffsets.push_back(animOffset);
			animTimes.push_back(animTime);
			translations.push_back(translation);
			scales.push_back(scale);
			rotations.push_back(rotation);
		}
	}

	void glTFModelExporter::ChangeAnimationBones()
	{
		AnimIndex currentAnim(10000, 1, animProvider->anims[0].timeStart, animProvider->anims[0].timeEnd, animProvider->anims[0].loopType);
		float inverse = 1.0f / 1000.0f;
		//uint32_t animLength = paraXModel->anims[0].timeEnd - paraXModel->anims[0].timeStart;
		//if (animLength == 0) animLength = 1000;
		boneIndices.clear();
		animOffsets.clear();
		animTimes.clear();
		translations.clear();
		scales.clear();
		rotations.clear();
		bvTime.resize(1);
		bvTranslation.resize(1);
		bvRotation.resize(1);
		bvScale.resize(1);
		std::vector<int> boneIndice;
		std::vector<uint32_t> animOffset;
		std::vector<float> animTime;
		std::vector<Vector3> translation;
		std::vector<Vector3> scale;
		std::vector<Quaternion> rotation;
		for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
		{
			Bone& bone = paraXModel->bones[i];

			Bone* pCurBone = nullptr;
			if (bone.nBoneID > 0)
			{
				pCurBone = GetBoneByID((KNOWN_BONE_NODES)bone.nBoneID);
			}
			else
			{
				Bone* bone_ = GetBoneByIndex(bone.nIndex);
				if (bone_ != nullptr && bone_->nBoneID <= 0 && bone.parent == bone_->parent)
					pCurBone = bone_;
			}

			if (pCurBone != nullptr)
			{
				if (bone.trans.used || bone.rot.used || bone.scale.used)
				{
					uint32_t firstT = pCurBone->trans.ranges[0].first;
					uint32_t secondT = pCurBone->trans.ranges[0].second;
					uint32_t firstR = pCurBone->rot.ranges[0].first;
					uint32_t secondR = pCurBone->rot.ranges[0].second;
					uint32_t firstS = pCurBone->scale.ranges[0].first;
					uint32_t secondS = pCurBone->scale.ranges[0].second;
					uint32_t animStart = std::min(std::min(firstT, firstR), firstS);
					uint32_t animEnd = std::max(std::max(secondT, secondR), secondS);

					animOffset.push_back(animEnd - animStart + 1);
					int time = 0;
					Vector3 t, s;
					Quaternion q;
					int nCurrentAnim = currentAnim.nIndex;
					for (uint32_t j = animStart; j <= animEnd; j++)
					{
						if (secondT == animEnd)
							time = pCurBone->trans.times[j];
						else if (secondR == animEnd)
							time = pCurBone->rot.times[j];
						else if (secondS == animEnd)
							time = pCurBone->scale.times[j];
						if (bone.bUsePivot)
						{
							q = pCurBone->rot.getValue(0, time);
							if (!(bone.nBoneID == Bone_Root || (bone.nBoneID >= Bone_forehand && bone.nBoneID <= Bone_chin)))
							{
								t = bone.trans.getValue(currentAnim);
							}
							else
							{
								t = pCurBone->trans.getValue(0, time);
								t *= (bone.pivot.y / pCurBone->pivot.y);
							}
						}
						else
						{
							q = bone.rot.getValue(nCurrentAnim, time);
							t = bone.trans.getValue(currentAnim);
						}
						s = bone.scale.getValue(currentAnim);

						Matrix4 mat;
						if (bone.bUsePivot)
						{
							mat.makeTrans(bone.pivot * -1.0f);
							mat.setScale(s);
							mat.m[3][0] *= s.x;
							mat.m[3][1] *= s.y;
							mat.m[3][2] *= s.z;
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
							mat.offsetTrans(bone.pivot);
						}
						else
						{
							mat.makeScale(s);
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
						}

						animTime.push_back(time * inverse);
						translation.push_back(mat.getTrans());
						rotation.push_back(q);
						scale.push_back(s);
					}
					boneIndice.push_back(bone.nIndex);
				}
			}
			else
			{
				if (bone.trans.used || bone.rot.used || bone.scale.used)
				{
					uint32_t firstT = bone.trans.ranges[0].first;
					uint32_t secondT = bone.trans.ranges[0].second;
					uint32_t firstR = bone.rot.ranges[0].first;
					uint32_t secondR = bone.rot.ranges[0].second;
					uint32_t firstS = bone.scale.ranges[0].first;
					uint32_t secondS = bone.scale.ranges[0].second;
					uint32_t animStart = std::min(std::min(firstT, firstR), firstS);
					uint32_t animEnd = std::max(std::max(secondT, secondR), secondS);

					animOffset.push_back(animEnd - animStart + 1);
					int time = 0;
					Vector3 t, s;
					Quaternion q;
					for (uint32_t j = animStart; j <= animEnd; j++)
					{
						if (secondT == animEnd)
						{
							time = bone.trans.times[j];
							t = bone.trans.data[j];
							s = bone.scale.getValue(0, time);
							q = bone.rot.getValue(0, time);
						}
						else if (secondR == animEnd)
						{
							time = bone.rot.times[j];
							t = bone.trans.getValue(0, time);
							s = bone.scale.getValue(0, time);
							q = bone.rot.data[j];
						}
						else if (secondS == animEnd)
						{
							time = bone.scale.times[j];
							t = bone.trans.getValue(0, time);
							s = bone.scale.data[j];
							q = bone.rot.getValue(0, time);
						}

						Matrix4 mat;
						if (bone.bUsePivot)
						{
							mat.makeTrans(bone.pivot * -1.0f);
							mat.setScale(s);
							mat.m[3][0] *= s.x;
							mat.m[3][1] *= s.y;
							mat.m[3][2] *= s.z;
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
							mat.offsetTrans(bone.pivot);
						}
						else
						{
							mat.makeScale(s);
							mat = mat.Multiply4x3(Matrix4(q.invertWinding()));
							mat.offsetTrans(t);
						}

						animTime.push_back(time * inverse);
						translation.push_back(mat.getTrans());
						rotation.push_back(q);
						scale.push_back(s);
					}
					boneIndice.push_back(bone.nIndex);
				}
			}
		}

		boneIndices.push_back(boneIndice);
		animOffsets.push_back(animOffset);
		animTimes.push_back(animTime);
		translations.push_back(translation);
		scales.push_back(scale);
		rotations.push_back(rotation);
	}

	ParaEngine::Bone* glTFModelExporter::GetBoneByIndex(int nIndex)
	{
		return (nIndex >= 0 && nIndex < (int)animProvider->m_objNum.nBones) ? &(animProvider->bones[nIndex]) : nullptr;
	}

	ParaEngine::Bone* glTFModelExporter::GetBoneByID(KNOWN_BONE_NODES boneID)
	{
		int nIndex = animProvider->m_boneLookup[boneID];
		return (nIndex >= 0) ? &(animProvider->bones[nIndex]) : nullptr;
	}

	void glTFModelExporter::CalculateJoint(int id, const Vector3& pivot)
	{
		for (uint32_t i = 1; i < paraXModel->m_objNum.nBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			if (bone.parent == id)
			{
				Vector3 offset = bone.m_finalTrans - pivot;
				bone.matTransform.makeTrans(offset);
				CalculateJoint(bone.nIndex, bone.m_finalTrans);
			}
		}
	}

	void glTFModelExporter::ExportMetadata()
	{
		Json::Value meta;
		meta["version"] = "2.0";
		meta["generator"] = "ParaEngine";
		root["asset"] = meta;
	}

	void glTFModelExporter::ExportScene()
	{
		root["scene"] = 0;
		Json::Value scenes = Json::Value(Json::arrayValue);
		Json::Value nodes = Json::Value(Json::arrayValue);
		Json::Value meshes = Json::Value(Json::arrayValue);
		Json::Value skins = Json::Value(Json::arrayValue);
		Json::Value buffers = Json::Value(Json::arrayValue);
		Json::Value bufferViews = Json::Value(Json::arrayValue);
		Json::Value accessors = Json::Value(Json::arrayValue);
		Json::Value materials = Json::Value(Json::arrayValue);
		Json::Value textures = Json::Value(Json::arrayValue);
		Json::Value images = Json::Value(Json::arrayValue);
		Json::Value samplers = Json::Value(Json::arrayValue);
		Json::Value animations = Json::Value(Json::arrayValue);

		std::shared_ptr<Node> node = ExportNode();
		Json::Value jsonSceneNodes = Json::Value(Json::arrayValue);
		jsonSceneNodes[0u] = node->index;
		std::shared_ptr<Mesh>& mesh = node->mesh;
		Json::Value jsonNodeObj;
		jsonNodeObj["mesh"] = mesh->index;
		if (node->skin != nullptr)
		{
			jsonNodeObj["skin"] = node->skin->index;
			Json::Value jsonNodeChildren = Json::Value(Json::arrayValue);
			for (uint32_t i = 0; i < node->children.size(); i++)
			{
				jsonNodeChildren[i] = node->children[i];
			}
			jsonNodeObj["children"] = jsonNodeChildren;
		}
		nodes[node->index] = jsonNodeObj;

		{
			Json::Value ps = Json::Value(Json::arrayValue);
			for (uint32_t k = 0; k < mesh->primitives.size(); k++)
			{
				Mesh::Primitive& primitive = mesh->primitives[k];
				Json::Value a;
				if (primitive.attributes.position != nullptr)
				{
					a["POSITION"] = primitive.attributes.position->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.position, accessors);
						WriteBufferView(primitive.attributes.position->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.normal != nullptr)
				{
					a["NORMAL"] = primitive.attributes.normal->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.normal, accessors);
						WriteBufferView(primitive.attributes.normal->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.texcoord != nullptr)
				{
					a["TEXCOORD_0"] = primitive.attributes.texcoord->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.texcoord, accessors);
						WriteBufferView(primitive.attributes.texcoord->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.color0 != nullptr)
				{
					a["COLOR_0"] = primitive.attributes.color0->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.color0, accessors);
						WriteBufferView(primitive.attributes.color0->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.color1 != nullptr)
				{
					a["COLOR_1"] = primitive.attributes.color1->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.color1, accessors);
						WriteBufferView(primitive.attributes.color1->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.joints != nullptr)
				{
					a["JOINTS_0"] = primitive.attributes.joints->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.joints, accessors);
						WriteBufferView(primitive.attributes.joints->bufferView, bufferViews);
					}
				}
				if (primitive.attributes.weights != nullptr)
				{
					a["WEIGHTS_0"] = primitive.attributes.weights->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.weights, accessors);
						WriteBufferView(primitive.attributes.weights->bufferView, bufferViews);
					}
				}

				Json::Value p;
				p["attributes"] = a;
				p["mode"] = primitive.mode;
				if (primitive.indices != nullptr)
				{
					WriteAccessor(primitive.indices, accessors);
					WriteBufferView(primitive.indices->bufferView, bufferViews);
					p["indices"] = primitive.indices->index;
				}
				if (primitive.material != nullptr)
				{
					WriteMaterial(primitive.material, materials, textures, samplers, images, k);
					p["material"] = primitive.material->index;
				}
				ps[k] = p;
			}
			Json::Value obj;
			obj["primitives"] = ps;
			meshes[mesh->index] = obj;
		}

		if (node->skin != nullptr)
		{
			Json::Value skin;
			skin["inverseBindMatrices"] = node->skin->inverseBindMatrices->index;
			WriteAccessor(node->skin->inverseBindMatrices, accessors);
			WriteBufferView(node->skin->inverseBindMatrices->bufferView, bufferViews);
			Json::Value joints = Json::Value(Json::arrayValue);
			for (uint32_t i = 0; i < node->skin->joints.size(); i++)
			{
				std::shared_ptr<Node>& joint = node->skin->joints[i];
				joints[i] = joint->index;
				Json::Value t = Json::Value(Json::arrayValue);
				t[0u] = joint->translation[0];
				t[1u] = joint->translation[1];
				t[2u] = joint->translation[2];
				Json::Value r = Json::Value(Json::arrayValue);
				r[0u] = joint->rotation[0];
				r[1u] = joint->rotation[1];
				r[2u] = joint->rotation[2];
				r[3u] = joint->rotation[3];
				Json::Value s = Json::Value(Json::arrayValue);
				s[0u] = joint->scale[0];
				s[1u] = joint->scale[1];
				s[2u] = joint->scale[2];
				Json::Value child;
				if (!joint->children.empty())
				{
					Json::Value c = Json::Value(Json::arrayValue);
					for (uint32_t j = 0; j < joint->children.size(); j++)
					{
						c[j] = joint->children[j];
					}
					child["children"] = c;
				}
				child["translation"] = t;
				child["rotation"] = r;
				child["scale"] = s;
				nodes[joint->index] = child;
			}
			skin["joints"] = joints;
			skins[node->skin->index] = skin;
			root["skins"] = skins;
		}

		if (IsExportAnimation())
		{
			uint32_t nAnimations = GetAnimationCount();
			if (animProvider != nullptr) nAnimations = 1;
			for (uint32_t animId = 0; animId < nAnimations; animId++)
			{
				std::shared_ptr<Animation> animation = ExportAnimations(animId);
				WriteBufferView(bvTime[animId], bufferViews);
				WriteBufferView(bvTranslation[animId], bufferViews);
				WriteBufferView(bvRotation[animId], bufferViews);
				WriteBufferView(bvScale[animId], bufferViews);
				Json::Value saj = Json::Value(Json::arrayValue);
				for (uint32_t i = 0; i < animation->samplers.size(); i++)
				{
					Animation::Sampler& sampler = animation->samplers[i];
					if (sampler.used)
					{
						WriteAccessor(sampler.input, accessors);
					}
					WriteAccessor(sampler.output, accessors);
					Json::Value sj;
					sj["input"] = sampler.input->index;
					sj["output"] = sampler.output->index;
					switch (sampler.interpolation)
					{
					case Interpolation::LINEAR: sj["interpolation"] = "LINEAR"; break;
					case Interpolation::STEP: sj["interpolation"] = "STEP"; break;
					case Interpolation::CUBICSPLINE: sj["interpolation"] = "CUBICSPLINE"; break;
					default: break;
					}
					saj[i] = sj;
				}
				Json::Value caj = Json::Value(Json::arrayValue);
				for (uint32_t i = 0; i < animation->channels.size(); i++)
				{
					Animation::Channel& channel = animation->channels[i];
					Json::Value cj;
					cj["sampler"] = channel.sampler;
					Json::Value tj;
					tj["node"] = channel.target.node;
					switch (channel.target.path)
					{
					case AnimationPath::Translation: tj["path"] = "translation"; break;
					case AnimationPath::Rotation: tj["path"] = "rotation"; break;
					case AnimationPath::Scale: tj["path"] = "scale"; break;
					default: break;
					}
					cj["target"] = tj;
					caj[i] = cj;
				}
				Json::Value aj;
				aj["samplers"] = saj;
				aj["channels"] = caj;
				animations[animation->index] = aj;
			}
			root["animations"] = animations;
		}

		Json::Value jsonScene;
		jsonScene["nodes"] = jsonSceneNodes;
		scenes[0u] = jsonScene;
		WriteBuffer(buffers, 0);

		root["scenes"] = scenes;
		root["nodes"] = nodes;
		root["meshes"] = meshes;
		root["buffers"] = buffers;
		root["bufferViews"] = bufferViews;
		root["accessors"] = accessors;
		if (!materials.empty())
			root["materials"] = materials;
		if (!textures.empty())
			root["textures"] = textures;
		if (!samplers.empty())
			root["samplers"] = samplers;
		if (!images.empty())
			root["images"] = images;
	}

	std::shared_ptr<Node> glTFModelExporter::ExportNode()
	{
		std::shared_ptr<Node> node = std::make_shared<Node>();
		node->index = nodeIndex++;
		node->boneIndex = -1;
		node->mesh = ExportMesh();
		node->skin = nullptr;
		if (IsExportAnimation())
		{
			node->skin = ExportSkin(node);
		}
		return node;
	}

	std::shared_ptr<Mesh> glTFModelExporter::ExportMesh()
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		std::shared_ptr<Accessor> v = ExportVertices();
		std::shared_ptr<Accessor> n = ExportNormals();
		std::shared_ptr<Accessor> t = ExportTextureCoords();
		std::shared_ptr<Accessor> c0 = nullptr;
		std::shared_ptr<Accessor> c1 = nullptr;
		if (paraXModel->m_header.type == PARAX_MODEL_STATIC || paraXModel->m_RenderMethod == CParaXModel::BMAX_MODEL)
		{
			c0 = ExportColors0();
			c1 = ExportColors1();
		}
		std::shared_ptr<Accessor> j = nullptr;
		std::shared_ptr<Accessor> w = nullptr;
		if (IsExportAnimation())
		{
			j = ExportJoints();
			w = ExportWeights();
		}
		unsigned int index = 0;
		for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
		{
			ModelRenderPass& pass = paraXModel->passes[i];
			if (pass.geoset < 0) continue;
			if (paraXModel->showGeosets[pass.geoset])
			{
				Mesh::Primitive primitive;
				primitive.indices = ExportIndices(pass);
				primitive.attributes.position = v;
				primitive.attributes.normal = n;
				primitive.attributes.texcoord = t;
				primitive.attributes.color0 = c0;
				primitive.attributes.color1 = c1;
				if (IsExportAnimation())
				{
					primitive.attributes.joints = j;
					primitive.attributes.weights = w;
				}
				//use blenmode of BM_TRANSPARENT for non-cubemode blocks
				if (paraXModel->m_RenderMethod != CParaXModel::BMAX_MODEL)
					primitive.material = ExportMaterials(pass, index);
				primitive.mode = PrimitiveMode::Triangles;
				mesh->primitives.push_back(primitive);
				index++;
			}
		}
		mesh->index = 0;
		return mesh;
	}

	std::shared_ptr<Skin> glTFModelExporter::ExportSkin(std::shared_ptr<Node> parentNode)
	{
		std::shared_ptr<Skin> skin = std::make_shared<Skin>();
		uint32_t numBones = paraXModel->m_objNum.nBones;
		skin->joints.resize(numBones);
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			std::shared_ptr<Node> node = std::make_shared<Node>();
			node->index = nodeIndex++;
			node->boneIndex = bone.nIndex;
			Quaternion q = Quaternion::IDENTITY;
			Matrix4 m, mLocalRot;
			Vector3 s(1.0f, 1.0f, 1.0f);
			if (bone.rot.used || bone.trans.used || bone.scale.used)
			{
				if (bone.bUsePivot)
				{
					m.makeTrans(bone.pivot * -1.0f);
					if (bone.scale.used)
					{
						s = bone.scale.getValue(0, 0);
						m.setScale(s);
						m.m[3][0] *= s.x;
						m.m[3][1] *= s.y;
						m.m[3][2] *= s.z;
					}
					if (bone.rot.used)
					{
						q = bone.rot.getValue(0, 0);
						m = m.Multiply4x3(Matrix4(q.invertWinding()));
					}
					if (bone.trans.used)
					{
						Vector3 tr = bone.trans.getValue(0, 0);
						m.offsetTrans(tr);
					}
					m.offsetTrans(bone.pivot);
				}
				else
				{
					if (bone.scale.used)
					{
						s = bone.scale.getValue(0, 0);
						m.makeScale(s);
						if (bone.rot.used)
						{
							q = bone.rot.getValue(0, 0);
							m = m.Multiply4x3(Matrix4(q.invertWinding()));
						}
					}
					else
					{
						if (bone.rot.used)
						{
							q = bone.rot.getValue(0, 0);
							m = Matrix4(q.invertWinding());
						}
						else
							m.identity();
					}
					if (bone.trans.used)
					{
						Vector3 tr = bone.trans.getValue(0, 0);
						m.offsetTrans(tr);
					}
				}
			}
			else
				m.identity();

			node->translation = m.getTrans();
			node->rotation = q;
			node->scale = s;
			skin->joints[node->boneIndex] = node;
			// skin->joints.push_back(node);
		}
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			if (bone.parent == -1) 
			{
				parentNode->children.push_back(skin->joints[bone.nIndex]->index);
			}
			else 
			{
				skin->joints[bone.parent]->children.push_back(skin->joints[bone.nIndex]->index);
			}
		}
		skin->inverseBindMatrices = ExportMatrices();
		skin->index = skinIndex++;
		return skin;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportMatrices()
	{
		uint32_t numBones = paraXModel->m_objNum.nBones;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::MAT4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numBones * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numBones;
		acc->type = AttribType::MAT4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(-FLT_MAX);
			acc->min.push_back(FLT_MAX);
		}
		for (uint32_t i = 0; i < numBones; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = paraXModel->bones[i].matOffset._m[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportVertices()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxVertex[i]);
			acc->min.push_back(minVertex[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportNormals()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxNormal[i]);
			acc->min.push_back(minNormal[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTextureCoords()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC2);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC2;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxCoord[i]);
			acc->min.push_back(minCoord[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportColors0()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxColor0[i]);
			acc->min.push_back(minColor0[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportColors1()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxColor1[i]);
			acc->min.push_back(minColor1[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportJoints()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::UnsignedByte);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::UnsignedByte;
		acc->count = numVertices;
		acc->type = AttribType::VEC4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxJoint[i]);
			acc->min.push_back(minJoint[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportWeights()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxWeight[i]);
			acc->min.push_back(minWeight[i]);
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportIndices(ModelRenderPass& pass)
	{
		uint32_t numIndices = pass.indexCount;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::UnsignedInt);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numIndices * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::ElementArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::UnsignedInt;
		acc->type = AttribType::SCALAR;
		acc->count = numIndices;
		acc->max.push_back(0);
		acc->min.push_back(FLT_MAX);
		uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
		for (uint32_t i = pass.m_nIndexStart; i < pass.m_nIndexStart + pass.indexCount; i++)
		{
			uint32_t val = paraXModel->m_indices[i] + vertexOffset;
			if (val < acc->min[0]) acc->min[0] = val;
			if (val > acc->max[0]) acc->max[0] = val;
		}

		bufferViewIndex++;
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Material> glTFModelExporter::ExportMaterials(ModelRenderPass& pass, int index)
	{
		int tex = pass.tex;
		bool cull = pass.cull;
		std::shared_ptr<Material> material = std::make_shared<Material>();
		material->index = index;
		material->alphaMode = "MASK";
		material->alphaCutoff = 0.5;
		material->doubleSide = !cull;
		PbrMetallicRoughness& metallic = material->metallicRoughness;
		metallic.metallicFactor = 0;
		metallic.roughnessFactor = 1;
		metallic.baseColorFactor[0] = 1.0;
		metallic.baseColorFactor[1] = 1.0;
		metallic.baseColorFactor[2] = 1.0;
		metallic.baseColorFactor[3] = 1.0;

		uint32_t numTextures = paraXModel->m_objNum.nTextures;
		if (numTextures > tex)
		{
			std::shared_ptr<Image> img = std::make_shared<Image>();
			if (paraXModel->m_header.type == PARAX_MODEL_STATIC)
			{
				std::string texPath = paraXModel->textures[tex]->m_key;
				if (CParaFile::IsAbsolutePath(texPath) && CParaFile::DoesFileExist(texPath.c_str()))
				{
					img->uri = "Texture/blocks/" + CParaFile::GetFileName(texPath);
					img->filename = CParaFile::GetParentDirectoryFromPath(fileName) + img->uri;
				}
				else
				{
					img->uri = texPath;
					texPath = StringHelper::UTF8ToAnsi(texPath.c_str());
					img->filename = CParaFile::GetParentDirectoryFromPath(fileName) + texPath;
				}
			}
			else
			{
				if (numTextures > 1)
				{
					char tmp[256] = "\0";
					StringHelper::fast_sprintf(tmp, "%s_%d", fileName.substr(0, fileName.find_last_of(".")).c_str(), tex);
					std::string path = tmp;
					std::string name = path.substr(path.find_last_of("/\\") + 1u);
					img->filename = path + ".png";
					img->uri = name + ".png";
				}
				else
				{
					std::string path = fileName.substr(0, fileName.find_last_of("."));
					std::string name = path.substr(path.find_last_of("/\\") + 1u);
					img->filename = path + ".png";
					img->uri = name + ".png";
				}
			}
			img->index = index;
			CParaFile::MakeDirectoryFromFilePath(img->filename.c_str());
			// paraXModel->textures[tex]->SaveToFile(img->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
			if (tex >= CParaXModel::MAX_MODEL_TEXTURES || paraXModel->specialTextures[tex] == -1 || paraXModel->replaceTextures[paraXModel->specialTextures[tex]] == 0) {
				paraXModel->textures[tex]->SaveToFile(img->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
			} else {
				paraXModel->replaceTextures[paraXModel->specialTextures[tex]]->SaveToFile(img->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
			}
			std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>();
			if (paraXModel->m_header.type == PARAX_MODEL_STATIC)
			{
				sampler->magFilter = SamplerMagFilter::MagNearest;
				sampler->minFilter = SamplerMinFilter::NearestMipMapNearest;
				sampler->wrapS = SamplerWrap::Repeat;
				sampler->wrapT = SamplerWrap::Repeat;
				sampler->index = index;
			}
			else
			{
				sampler->magFilter = SamplerMagFilter::MagLinear;
				sampler->minFilter = SamplerMinFilter::NearestMipMapLinear;
				sampler->wrapS = SamplerWrap::Repeat;
				sampler->wrapT = SamplerWrap::Repeat;
				sampler->index = index;
			}

			std::shared_ptr<Texture> texture = std::make_shared<Texture>();
			texture->source = img;
			texture->sampler = sampler;
			metallic.baseColorTexture.texture = texture;
			metallic.baseColorTexture.index = index;
			metallic.baseColorTexture.texCoord = 0;
		}
		return material;
	}

	std::shared_ptr<Animation> glTFModelExporter::ExportAnimations(uint32_t animId)
	{
		std::shared_ptr<Animation> animation = std::make_shared<Animation>();
		animation->index = animationIndex++;
		bvTime[animId] = ExportBufferView(AttribType::SCALAR, animTimes[animId].size());
		bvTranslation[animId] = ExportBufferView(AttribType::VEC3, translations[animId].size());
		bvRotation[animId] = ExportBufferView(AttribType::VEC4, rotations[animId].size());
		bvScale[animId] = ExportBufferView(AttribType::VEC3, scales[animId].size());
		uint32_t offsetTime = 0;
		for (uint32_t i = 0; i < animOffsets[animId].size(); i++)
		{
			uint32_t numDatas = animOffsets[animId][i];
			std::shared_ptr<Accessor> acTime = ExportTimeAccessor(bvTime[animId], offsetTime, numDatas, animId);

			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = true;
				sampler.input = acTime;
				sampler.output = ExportTranslationAccessor(bvTranslation[animId], offsetTime, numDatas, animId);
				Animation::Channel channel;
				channel.target.node = boneIndices[animId][i] + 1;
				channel.target.path = AnimationPath::Translation;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = false;
				sampler.input = acTime;
				sampler.output = ExportRotationAccessor(bvRotation[animId], offsetTime, numDatas, animId);
				Animation::Channel channel;
				channel.target.node = boneIndices[animId][i] + 1;
				channel.target.path = AnimationPath::Rotation;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = false;
				sampler.input = acTime;
				sampler.output = ExportScaleAccessor(bvScale[animId], offsetTime, numDatas, animId);
				Animation::Channel channel;
				channel.target.node = boneIndices[animId][i] + 1;
				channel.target.path = AnimationPath::Scale;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			offsetTime += numDatas;
		}
		return animation;
	}

	std::shared_ptr<ParaEngine::BufferView> glTFModelExporter::ExportBufferView(AttribType::Value type, uint32_t length)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(type);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = length * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);
		bufferViewIndex++;
		return bv;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTimeAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = offset * numComponents * bytesPerComp;
		acc->componentType = ComponentType::Float;
		acc->count = numDatas;
		acc->type = AttribType::SCALAR;
		acc->max.push_back(-FLT_MAX);
		acc->min.push_back(FLT_MAX);
		for (uint32_t i = offset; i < numDatas + offset; i++)
		{
			float val = animTimes[animId][i];
			if (val < acc->min[0]) acc->min[0] = val;
			if (val > acc->max[0]) acc->max[0] = val;
		}
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTranslationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = offset * numComponents * bytesPerComp;
		acc->componentType = ComponentType::Float;
		acc->count = numDatas;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(-FLT_MAX);
			acc->min.push_back(FLT_MAX);
		}
		for (uint32_t i = offset; i < numDatas + offset; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = translations[animId][i][j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}
		accessorIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportRotationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = offset * numComponents * bytesPerComp;
		acc->componentType = ComponentType::Float;
		acc->count = numDatas;
		acc->type = AttribType::VEC4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(-FLT_MAX);
			acc->min.push_back(FLT_MAX);
		}
		for (uint32_t i = offset; i < numDatas + offset; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = rotations[animId][i][j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}

		accessorIndex++;
		return acc;
	}

	std::shared_ptr<ParaEngine::Accessor> glTFModelExporter::ExportScaleAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = accessorIndex;
		acc->byteOffset = offset * numComponents * bytesPerComp;
		acc->componentType = ComponentType::Float;
		acc->count = numDatas;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(-FLT_MAX);
			acc->min.push_back(FLT_MAX);
		}
		for (uint32_t i = offset; i < numDatas + offset; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = scales[animId][i][j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}

		accessorIndex++;
		return acc;
	}

	std::string glTFModelExporter::EncodeBuffer()
	{
		StringBuilder builder;
		uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
		uint32_t sizeUInt = ComponentTypeSize(ComponentType::UnsignedInt);
		uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);

		for (auto it = vertices.begin(); it != vertices.end(); ++it)
		{
			builder.append((const char*)&(it->x), sizeFloat);
			builder.append((const char*)&(it->y), sizeFloat);
			builder.append((const char*)&(it->z), sizeFloat);
		}

		for (auto it = normals.begin(); it != normals.end(); ++it)
		{
			builder.append((const char*)&(it->x), sizeFloat);
			builder.append((const char*)&(it->y), sizeFloat);
			builder.append((const char*)&(it->z), sizeFloat);
		}

		for (auto it = texcoords.begin(); it != texcoords.end(); ++it)
		{
			builder.append((const char*)&(it->x), sizeFloat);
			builder.append((const char*)&(it->y), sizeFloat);
		}

		if (paraXModel->m_header.type == PARAX_MODEL_STATIC || paraXModel->m_RenderMethod == CParaXModel::BMAX_MODEL) {
			for (auto it = colors0.begin(); it != colors0.end(); ++it)
			{
				builder.append((const char*)&(it->x), sizeFloat);
				builder.append((const char*)&(it->y), sizeFloat);
				builder.append((const char*)&(it->z), sizeFloat);
			}

			for (auto it = colors1.begin(); it != colors1.end(); ++it)
			{
				builder.append((const char*)&(it->x), sizeFloat);
				builder.append((const char*)&(it->y), sizeFloat);
				builder.append((const char*)&(it->z), sizeFloat);
			}
		}


		if (IsExportAnimation())
		{
			for (uint32_t i = 0; i < paraXModel->m_objNum.nVertices; i++)
			{
				uint8* bone = paraXModel->m_origVertices[i].bones;
				builder.append((const char*)bone, sizeUByte * 4);
			}
			for (auto it = weights.begin(); it != weights.end(); ++it)
			{
				builder.append((const char*)&(it->x), sizeFloat);
				builder.append((const char*)&(it->y), sizeFloat);
				builder.append((const char*)&(it->z), sizeFloat);
				builder.append((const char*)&(it->w), sizeFloat);
			}
			for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
			{
				ModelRenderPass& pass = paraXModel->passes[i];
				if (pass.geoset < 0 || !paraXModel->showGeosets[pass.geoset]) continue;
				uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
				for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
				{
					uint32_t index = paraXModel->m_indices[j] + vertexOffset;
					builder.append((const char*)&index, sizeUInt);
				}
			}

			uint32_t numBones = paraXModel->m_objNum.nBones;
			for (uint32_t i = 0; i < numBones; i++)
			{
				builder.append((const char*)&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
			}
			for (uint32_t i = 0; i < animTimes.size(); i++)
			{
				for (uint32_t j = 0; j < animTimes[i].size(); j++)
				{
					float val = animTimes[i][j];
					builder.append((const char*)&val, sizeFloat);
				}
				for (uint32_t j = 0; j < translations[i].size(); j++)
				{
					builder.append((const char*)&translations[i][j].x, sizeFloat);
					builder.append((const char*)&translations[i][j].y, sizeFloat);
					builder.append((const char*)&translations[i][j].z, sizeFloat);
				}
				for (uint32_t j = 0; j < rotations[i].size(); j++)
				{
					builder.append((const char*)&rotations[i][j].x, sizeFloat);
					builder.append((const char*)&rotations[i][j].y, sizeFloat);
					builder.append((const char*)&rotations[i][j].z, sizeFloat);
					builder.append((const char*)&rotations[i][j].w, sizeFloat);
				}
				for (uint32_t j = 0; j < scales[i].size(); j++)
				{
					builder.append((const char*)&scales[i][j].x, sizeFloat);
					builder.append((const char*)&scales[i][j].y, sizeFloat);
					builder.append((const char*)&scales[i][j].z, sizeFloat);
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
			{
				ModelRenderPass& pass = paraXModel->passes[i];
				if (pass.geoset < 0) continue;
				uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
				for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
				{
					uint32_t index = paraXModel->m_indices[j] + vertexOffset;
					builder.append((const char*)&index, sizeUInt);
				}
			}
		}
		return StringHelper::base64(builder.ToString());
	}

	void glTFModelExporter::WriteBuffer(Json::Value& obj, uint32_t index)
	{
		Json::Value b;
		b["byteLength"] = buffer->byteLength;
		if (!isBinary)
		{
			b["uri"] = "data:application/octet-stream;base64," + EncodeBuffer();
		}
		obj[index] = b;
	}

	void glTFModelExporter::WriteBufferView(std::shared_ptr<BufferView>& bufferView, Json::Value& obj)
	{
		Json::Value bv;
		bv["buffer"] = bufferView->buffer->index;
		bv["byteOffset"] = bufferView->byteOffset;
		bv["byteLength"] = bufferView->byteLength;
		if (bufferView->byteStride != 0)
			bv["byteStride"] = bufferView->byteStride;
		if (bufferView->target != BufferViewTarget::NotUse)
			bv["target"] = bufferView->target;
		obj[bufferView->index] = bv;
	}

	void glTFModelExporter::WriteAccessor(std::shared_ptr<Accessor>& accessor, Json::Value& obj)
	{
		Json::Value acc;
		acc["bufferView"] = accessor->bufferView->index;
		acc["byteOffset"] = accessor->byteOffset;
		acc["componentType"] = accessor->componentType;
		acc["count"] = accessor->count;
		acc["type"] = AttribType::ToString(accessor->type);
		Json::Value jmax = Json::Value(Json::arrayValue);
		for (uint32_t i = 0; i < accessor->max.size(); i++)
		{
			switch (accessor->componentType)
			{
			case ComponentType::UnsignedByte:
				jmax[i] = (uint8)accessor->max[i];
				break;
			case ComponentType::UnsignedShort:
				jmax[i] = (uint16_t)accessor->max[i];
				break;
			case ComponentType::UnsignedInt:
				jmax[i] = (uint32_t)accessor->max[i];
				break;
			default:
				jmax[i] = accessor->max[i];
				break;
			}
		}
		acc["max"] = jmax;
		Json::Value jmin = Json::Value(Json::arrayValue);
		for (uint32_t i = 0; i < accessor->min.size(); i++)
		{
			switch (accessor->componentType)
			{
			case ComponentType::UnsignedByte:
				jmin[i] = (uint8)accessor->min[i];
				break;
			case ComponentType::UnsignedShort:
				jmin[i] = (uint16_t)accessor->min[i];
				break;
			case ComponentType::UnsignedInt:
				jmin[i] = (uint32_t)accessor->min[i];
				break;
			default:
				jmin[i] = accessor->min[i];
				break;
			}
		}
		acc["min"] = jmin;
		obj[accessor->index] = acc;
	}

	void glTFModelExporter::WriteMaterial(std::shared_ptr<Material>& material, Json::Value& mat, Json::Value& tex, Json::Value& sampler, Json::Value& img, uint32_t index)
	{
		PbrMetallicRoughness& pbr = material->metallicRoughness;
		Json::Value baseTex;
		// baseTex["index"] = pbr.baseColorTexture.index;
		baseTex["index"] = index;
		baseTex["texCoord"] = pbr.baseColorTexture.texCoord;
		Json::Value baseFac = Json::Value(Json::arrayValue);
		baseFac[0u] = pbr.baseColorFactor[0];
		baseFac[1u] = pbr.baseColorFactor[1];
		baseFac[2u] = pbr.baseColorFactor[2];
		baseFac[3u] = pbr.baseColorFactor[3];
		Json::Value metallic;
		metallic["metallicFactor"] = pbr.metallicFactor;
		metallic["roughnessFactor"] = pbr.roughnessFactor;
		metallic["baseColorTexture"] = baseTex;
		metallic["baseColorFactor"] = baseFac;
		Json::Value m;
		m["pbrMetallicRoughness"] = metallic;
		m["alphaMode"] = material->alphaMode;
		m["alphaCutoff"] = material->alphaCutoff;
		m["doubleSided"] = material->doubleSide;
		mat[index] = m;

		shared_ptr<Texture>& texture = pbr.baseColorTexture.texture;
		Json::Value t;
		// t["sampler"] = texture->sampler->index;
		// t["source"] = texture->source->index;
		t["sampler"] = index;
		t["source"] = index;
		texture->sampler->index = index;
		texture->source->index = index;
		tex[index] = t;

		Json::Value s;
		s["magFilter"] = texture->sampler->magFilter;
		s["minFilter"] = texture->sampler->minFilter;
		s["wrapS"] = texture->sampler->wrapS;
		s["wrapT"] = texture->sampler->wrapT;
		sampler[index] = s;

		Json::Value i;
		i["uri"] = texture->source->uri;
		{
			//CParaFile file;
			//if (file.CreateNewFile(texture->source->filename.c_str()))
			//{
			//	file.write(texture->source->bufferPointer.get(), texture->source->bufferSize);
			//	file.close();
			//}
		}
		img[index] = i;
	}

	void glTFModelExporter::WriteFile()
	{
		CParaFile file;
		if (file.CreateNewFile(fileName.c_str()))
		{
			Json::FastWriter writer;
			const std::string& data = writer.write(root);
			file.write(data.c_str(), data.length());
			file.close();

			//if (!isEmbedded && !isBinary)
			//{
			//	WriteRawData();
			//}
		}
	}

	void glTFModelExporter::WriteRawData()
	{
		CParaFile bin;
		if (bin.CreateNewFile(buffer->filename.c_str()))
		{
			uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
			uint32_t sizeUInt = ComponentTypeSize(ComponentType::UnsignedInt);
			uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);

			for (auto it = vertices.begin(); it != vertices.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
				bin.write(&(it->z), sizeFloat);
			}

			for (auto it = normals.begin(); it != normals.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
				bin.write(&(it->z), sizeFloat);
			}

			for (auto it = texcoords.begin(); it != texcoords.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
			}

			for (auto it = colors0.begin(); it != colors0.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
				bin.write(&(it->z), sizeFloat);
			}

			for (auto it = colors1.begin(); it != colors1.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
				bin.write(&(it->z), sizeFloat);
			}

			if (IsExportAnimation())
			{
				for (uint32_t i = 0; i < paraXModel->m_objNum.nVertices; i++)
				{
					uint8* bone = paraXModel->m_origVertices[i].bones;
					bin.write(bone, sizeUByte * 4);
				}
				for (auto it = weights.begin(); it != weights.end(); ++it)
				{
					bin.write(&(it->x), sizeFloat);
					bin.write(&(it->y), sizeFloat);
					bin.write(&(it->z), sizeFloat);
					bin.write(&(it->w), sizeFloat);
				}
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					ModelRenderPass& pass = paraXModel->passes[i];
					if (pass.geoset < 0) continue;
					uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
					for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
					{
						uint32_t index = paraXModel->m_indices[j] + vertexOffset;
						bin.write(&index, sizeUInt);
					}
				}
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					bin.write(&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
				}
				for (uint32_t i = 0; i < animTimes.size(); i++)
				{
					for (uint32_t j = 0; j < animTimes.size(); j++)
					{
						float val = animTimes[i][j];
						bin.write(&val, sizeFloat);
					}
					for (uint32_t j = 0; j < translations.size(); j++)
					{
						bin.write(&translations[i][j].x, sizeFloat);
						bin.write(&translations[i][j].y, sizeFloat);
						bin.write(&translations[i][j].z, sizeFloat);
					}
					for (uint32_t j = 0; j < rotations.size(); j++)
					{
						bin.write(&rotations[i][j].x, sizeFloat);
						bin.write(&rotations[i][j].y, sizeFloat);
						bin.write(&rotations[i][j].z, sizeFloat);
						bin.write(&rotations[i][j].w, sizeFloat);
					}
					for (uint32_t j = 0; j < scales.size(); j++)
					{
						bin.write(&scales[i][j].x, sizeFloat);
						bin.write(&scales[i][j].y, sizeFloat);
						bin.write(&scales[i][j].z, sizeFloat);
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					ModelRenderPass& pass = paraXModel->passes[i];
					if (pass.geoset < 0) continue;
					uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
					for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
					{
						uint32_t index = paraXModel->m_indices[j] + vertexOffset;
						bin.write(&index, sizeUInt);
					}
				}
			}
			bin.close();
		}
	}

	void glTFModelExporter::WriteGLBFile()
	{
		CParaFile file;
		if (file.CreateNewFile(fileName.c_str()))
		{
			Json::FastWriter writer;
			const std::string& data = writer.write(root);
			uint32_t jsonLength = (data.length() + 3) & (~3);
			uint32_t binaryLength = (buffer->byteLength + 3) & (~3);

			GLBHeader header;
			header.magic = 0x46546C67;
			header.version = 2;
			header.length = sizeof(GLBHeader) + 2 * sizeof(GLBChunk) + jsonLength + binaryLength;
			file.write(&header, sizeof(GLBHeader));

			GLBChunk jsonChunk;
			jsonChunk.chunkLength = jsonLength;
			jsonChunk.chunkType = ChunkType::JSON;
			file.write(&jsonChunk, sizeof(GLBChunk));
			file.write(data.c_str(), data.length());
			uint8_t jsonPadding = 0x20;
			uint32_t paddingLength = jsonLength - data.length();
			for (uint32_t i = 0; i < paddingLength; i++)
				file.write(&jsonPadding, 1);

			GLBChunk binaryChunk;
			binaryChunk.chunkLength = binaryLength;
			binaryChunk.chunkType = ChunkType::BIN;
			file.write(&binaryChunk, sizeof(GLBChunk));
			uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
			uint32_t sizeUInt = ComponentTypeSize(ComponentType::UnsignedInt);
			uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);

			for (auto it = vertices.begin(); it != vertices.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
				file.write(&(it->z), sizeFloat);
			}

			for (auto it = normals.begin(); it != normals.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
				file.write(&(it->z), sizeFloat);
			}

			for (auto it = texcoords.begin(); it != texcoords.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
			}

			for (auto it = colors0.begin(); it != colors0.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
				file.write(&(it->z), sizeFloat);
			}

			for (auto it = colors1.begin(); it != colors1.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
				file.write(&(it->z), sizeFloat);
			}

			if (IsExportAnimation())
			{
				for (uint32_t i = 0; i < paraXModel->m_objNum.nVertices; i++)
				{
					uint8* bone = paraXModel->m_origVertices[i].bones;
					file.write(bone, sizeUByte * 4);
				}
				for (auto it = weights.begin(); it != weights.end(); ++it)
				{
					file.write(&(it->x), sizeFloat);
					file.write(&(it->y), sizeFloat);
					file.write(&(it->z), sizeFloat);
					file.write(&(it->w), sizeFloat);
				}
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					ModelRenderPass& pass = paraXModel->passes[i];
					if (pass.geoset < 0) continue;
					uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
					for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
					{
						uint32_t index = paraXModel->m_indices[j] + vertexOffset;
						file.write(&index, sizeUInt);
					}
				}
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					file.write(&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
				}
				for (uint32_t i = 0; i < animTimes.size(); i++)
				{
					for (uint32_t j = 0; j < animTimes[i].size(); j++)
					{
						float val = animTimes[i][j];
						file.write(&val, sizeFloat);
					}
					for (uint32_t j = 0; j < translations[i].size(); j++)
					{
						file.write(&translations[i][j].x, sizeFloat);
						file.write(&translations[i][j].y, sizeFloat);
						file.write(&translations[i][j].z, sizeFloat);
					}
					for (uint32_t j = 0; j < rotations[i].size(); j++)
					{
						file.write(&rotations[i][j].x, sizeFloat);
						file.write(&rotations[i][j].y, sizeFloat);
						file.write(&rotations[i][j].z, sizeFloat);
						file.write(&rotations[i][j].w, sizeFloat);
					}
					for (uint32_t j = 0; j < scales[i].size(); j++)
					{
						file.write(&scales[i][j].x, sizeFloat);
						file.write(&scales[i][j].y, sizeFloat);
						file.write(&scales[i][j].z, sizeFloat);
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					ModelRenderPass& pass = paraXModel->passes[i];
					if (pass.geoset < 0) continue;
					uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
					for (uint32_t j = pass.m_nIndexStart; j < pass.m_nIndexStart + pass.indexCount; j++)
					{
						uint32_t index = paraXModel->m_indices[j] + vertexOffset;
						file.write(&index, sizeUInt);
					}
				}
			}
			uint8_t binaryPadding = 0x00;
			paddingLength = binaryLength - buffer->byteLength;
			for (uint32_t i = 0; i < paddingLength; i++)
				file.write(&binaryPadding, 1);

			file.close();
		}
	}

	void glTFModelExporter::ParaXExportTo_glTF(CParaXModel* parax, const char* output)
	{
		if (parax != nullptr && output != nullptr)
		{
			bool binary = CParaFile::GetFileExtension(output) == "glb";
			glTFModelExporter exporter(parax, binary);
			exporter.ExportToFile(output);
		}
	}

	void glTFModelExporter::BlocksExportTo_glTF(const char* blocks, const char* output)
	{
		BlocksParser parser;
		parser.Load(blocks);
		CParaXModel* mesh = parser.ParseParaXModel();
		if (mesh != nullptr && output != nullptr)
		{
			bool binary = CParaFile::GetFileExtension(output) == "glb";
			glTFModelExporter exporter(mesh, binary);
			exporter.ExportToFile(output);
		}
	}

}