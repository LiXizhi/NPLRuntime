#include "ParaEngine.h"
#include "glTFModelExporter.h"
#include "ParaXModel.h"
#include "ParaXSerializer.h"
#include "StringHelper.h"
#include "StringBuilder.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"

namespace ParaEngine
{

	glTFModelExporter::glTFModelExporter(const std::string& filename, CParaXModel* mesh, bool binary, bool embedded)
		: maxVertex(-FLT_MAX, -FLT_MAX, -FLT_MAX), minVertex(FLT_MAX, FLT_MAX, FLT_MAX),
		maxNormal(-FLT_MAX, -FLT_MAX, -FLT_MAX), minNormal(FLT_MAX, FLT_MAX, FLT_MAX),
		maxColor(-FLT_MAX, -FLT_MAX, -FLT_MAX), minColor(FLT_MAX, FLT_MAX, FLT_MAX),
		maxCoord(-FLT_MAX, -FLT_MAX), minCoord(FLT_MAX, FLT_MAX),
		maxJoint(0, 0, 0, 0), minJoint(255, 255, 255, 255),
		maxWeight(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX), minWeight(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX),
		fileName(filename),
		paraXModel(mesh),
		bufferIndex(0),
		isBinary(binary),
		isEmbedded(embedded),
		buffer(std::make_shared<Buffer>())
	{
		if (!isBinary)
		{
			std::string path = filename.substr(0, filename.rfind(".gltf"));
			std::string name = path.substr(path.find_last_of("/\\") + 1u);
			buffer->filename = path + ".bin";
			buffer->uri = name + ".bin";
		}
		buffer->index = 0;
		ParseParaXModel();

		ExportMetadata();
		ExportScene();
		if (isBinary)
			WriteGLBFile();
		else
			WriteFile();
	}

	glTFModelExporter::~glTFModelExporter()
	{

	}

	void glTFModelExporter::ParseParaXModel()
	{
		float inverse = 1.0f / 255.0f;
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		for (uint32_t i = 0; i < numVertices; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			Vector3 v = vertex.pos;
			Vector3 n = vertex.normal;
			Vector2 coord = vertex.texcoords;
			DWORD color = vertex.color0;

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

			if (color != 0)
			{
				float r = ((color >> 16) & 0xff) / 255.0f;
				float g = ((color >> 8) & 0xff) / 255.0f;
				float b = (color & 0xff) / 255.0f;
				if (r < minColor[0]) minColor[0] = r;
				if (g < minColor[1]) minColor[1] = g;
				if (b < minColor[2]) minColor[2] = b;
				if (r > maxColor[0]) maxColor[0] = r;
				if (g > maxColor[1]) maxColor[1] = g;
				if (b > maxColor[2]) maxColor[2] = b;
				colors.push_back(Vector3(r, g, b));
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

		inverse = 1.0f / 1000.0f;
		uint32_t animLength = paraXModel->anims[0].timeEnd - paraXModel->anims[0].timeStart;
		uint32_t numBones = paraXModel->m_objNum.nBones;
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			uint32_t firstT = bone.trans.ranges[0].first;
			uint32_t secondT = bone.trans.ranges[0].second;
			uint32_t firstR = bone.rot.ranges[0].first;
			uint32_t secondR = bone.rot.ranges[0].second;
			if (bone.trans.used && bone.rot.used)
			{
				if (secondR > secondT)
				{
					animOffsets.push_back(secondR - firstR + 1);
					if (firstT == secondT)
					{
						for (uint32_t j = firstR; j <= secondR; j++)
						{
							animTimes.push_back(bone.rot.times[j] * inverse);
							Quaternion q = bone.rot.data[j];
							q.invertWinding();
							Vector3 trans = bone.trans.data[firstT];
							if (bone.bUsePivot)
								trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
							translations.push_back(trans);
							rotations.push_back(q);
						}
					}
					else
					{
						for (uint32_t j = firstR; j <= secondR; j++)
						{
							int time = bone.rot.times[j];
							animTimes.push_back(time * inverse);
							Quaternion q = bone.rot.data[j];
							Vector3 trans = bone.trans.getValue(0, time);
							q.invertWinding();
							if (bone.bUsePivot)
								trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
							translations.push_back(trans);
							rotations.push_back(q);
						}
					}
				}
				else if (secondT > secondR)
				{
					animOffsets.push_back(secondT - firstT + 1);
					if (firstR == secondR)
					{
						Quaternion q = bone.rot.data[firstR];
						for (uint32_t j = firstT; j <= secondT; j++)
						{
							animTimes.push_back(bone.trans.times[j] * inverse);
							Vector3 trans = bone.trans.data[j];
							q.invertWinding();
							if (bone.bUsePivot)
								trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
							translations.push_back(trans);
							rotations.push_back(q);
						}
					}
					else
					{
						for (uint32_t j = firstT; j <= secondT; j++)
						{
							int time = bone.rot.times[j];
							animTimes.push_back(time * inverse);
							Quaternion q = bone.rot.getValue(0, time);
							Vector3 trans = bone.trans.data[j];
							q.invertWinding();
							if (bone.bUsePivot)
								trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
							translations.push_back(trans);
							rotations.push_back(q);
						}
					}
				}
				else
				{
					if (firstR == secondR)
					{
						animOffsets.push_back(2);
						animTimes.push_back(0);
						animTimes.push_back(animLength * inverse);
						Quaternion q = bone.rot.data[firstR];
						Vector3 trans = bone.trans.data[firstT];
						q.invertWinding();
						if (bone.bUsePivot)
							trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
						translations.push_back(trans);
						translations.push_back(trans);
						rotations.push_back(q);
						rotations.push_back(q);
					}
					else
					{
						animOffsets.push_back(secondR - firstR + 1);
						for (uint32_t j = firstR; j <= secondR; j++)
						{
							animTimes.push_back(bone.rot.times[j] * inverse);
							Quaternion q = bone.rot.data[j];
							Vector3 trans = bone.trans.data[j];
							q.invertWinding();
							if (bone.bUsePivot)
								trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
							translations.push_back(trans);
							rotations.push_back(q);
						}
					}
				}
				boneIndices.push_back(bone.nIndex);
			}
			else if (bone.trans.used)
			{
				//if (firstT == secondT)
				//{
				//	animIndices.push_back(std::make_pair(2, 0));
				//	animTimes.push_back(0);
				//	animTimes.push_back(animLength * inverse);
				//	Vector3 trans = bone.trans.data[firstT];
				//	translations.push_back(trans);
				//	translations.push_back(trans);
				//}
				//else
				//{
				//	animIndices.push_back(std::make_pair(secondT - firstT + 1, 0));
				//	for (uint32_t j = firstT; j <= secondT; j++)
				//	{
				//		animTimes.push_back(bone.rot.times[j] * inverse);
				//		Vector3 trans = bone.trans.data[j];
				//		translations.push_back(trans);
				//	}
				//}
				//bones.push_back(bone.nIndex);
			}
			else if (bone.rot.used)
			{
				if (firstR == secondR)
				{
					animOffsets.push_back(2);
					animTimes.push_back(0);
					animTimes.push_back(animLength * inverse);
					Quaternion q = bone.rot.data[firstR];
					Vector3 trans(0, 0, 0);
					q.invertWinding();
					if (bone.bUsePivot)
						trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
					translations.push_back(trans);
					translations.push_back(trans);
					rotations.push_back(q);
					rotations.push_back(q);
				}
				else
				{
					animOffsets.push_back(secondR - firstR + 1);
					for (uint32_t j = firstR; j <= secondR; j++)
					{
						animTimes.push_back(bone.rot.times[j] * inverse);
						Quaternion q = bone.rot.data[j];
						q.invertWinding();
						Vector3 trans(0, 0, 0);
						if (bone.bUsePivot)
							trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
						translations.push_back(trans);
						rotations.push_back(q);
					}
				}
				boneIndices.push_back(bone.nIndex);
			}
		}
	}

	Vector3 glTFModelExporter::CalculatePivot(const Vector3& pivot, const Vector3& trans, const Matrix4& matRot)
	{
		Matrix4 m;
		m.makeTrans(pivot * -1.0f);
		m = m.Multiply4x3(matRot);
		m.offsetTrans(trans);
		m.offsetTrans(pivot);
		return m.getTrans();
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
		Json::Value ns = Json::Value(Json::arrayValue);
		ns[0u] = node->index;
		std::shared_ptr<Mesh>& mesh = node->mesh;
		Json::Value m;
		m["mesh"] = mesh->index;
		if (node->skin != nullptr)
		{
			m["skin"] = node->skin->index;
			Json::Value c = Json::Value(Json::arrayValue);
			for (uint32_t i = 0; i < node->children.size(); i++)
			{
				c[i] = node->children[i];
			}
			m["children"] = c;
		}
		nodes[0u] = m;

		uint32_t index = 0;
		{
			Json::Value ps = Json::Value(Json::arrayValue);
			for (uint32_t k = 0; k < mesh->primitives.size(); k++)
			{
				Mesh::Primitive& primitive = mesh->primitives[k];
				if (primitive.indices != nullptr)
				{
					WriteAccessor(primitive.indices, accessors, index);
					WriteBufferView(primitive.indices->bufferView, bufferViews, index);
					index++;
				}
				Json::Value a;
				if (primitive.attributes.position != nullptr)
				{
					a["POSITION"] = primitive.attributes.position->index;
					WriteAccessor(primitive.attributes.position, accessors, index);
					WriteBufferView(primitive.attributes.position->bufferView, bufferViews, index);
					index++;
				}
				if (primitive.attributes.normal != nullptr)
				{
					a["NORMAL"] = primitive.attributes.normal->index;
					WriteAccessor(primitive.attributes.normal, accessors, index);
					WriteBufferView(primitive.attributes.normal->bufferView, bufferViews, index);
					index++;
				}
				if (primitive.attributes.texcoord != nullptr)
				{
					a["TEXCOORD_0"] = primitive.attributes.texcoord->index;
					WriteAccessor(primitive.attributes.texcoord, accessors, index);
					WriteBufferView(primitive.attributes.texcoord->bufferView, bufferViews, index);
					index++;
				}
				if (primitive.attributes.color != nullptr)
				{
					a["COLOR_0"] = primitive.attributes.color->index;
					WriteAccessor(primitive.attributes.color, accessors, index);
					WriteBufferView(primitive.attributes.color->bufferView, bufferViews, index);
					index++;
				}
				if (primitive.attributes.joints != nullptr)
				{
					a["JOINTS_0"] = primitive.attributes.joints->index;
					WriteAccessor(primitive.attributes.joints, accessors, index);
					WriteBufferView(primitive.attributes.joints->bufferView, bufferViews, index);
					index++;
				}
				if (primitive.attributes.weights != nullptr)
				{
					a["WEIGHTS_0"] = primitive.attributes.weights->index;
					WriteAccessor(primitive.attributes.weights, accessors, index);
					WriteBufferView(primitive.attributes.weights->bufferView, bufferViews, index);
					index++;
				}

				WriteMaterial(primitive.material, materials, textures, samplers, images);
				Json::Value p;
				p["attributes"] = a;
				p["indices"] = primitive.indices->index;
				p["mode"] = primitive.mode;
				p["material"] = primitive.material->index;
				ps[k] = p;
			}
			Json::Value obj;
			obj["primitives"] = ps;
			meshes[0u] = obj;
		}

		if (node->skin != nullptr)
		{
			Json::Value skin;
			skin["inverseBindMatrices"] = node->skin->inverseBindMatrices->index;
			WriteAccessor(node->skin->inverseBindMatrices, accessors, index);
			WriteBufferView(node->skin->inverseBindMatrices->bufferView, bufferViews, index);
			index++;
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
				nodes[i + 1] = child;
			}
			skin["joints"] = joints;
			skins[0u] = skin;
			root["skins"] = skins;
		}

		if (paraXModel->animated)
		{
			std::shared_ptr<Animation> animation = ExportAnimations();
			WriteBufferView(bvTime, bufferViews, index);
			WriteBufferView(bvTranslation, bufferViews, index + 1);
			WriteBufferView(bvRotation, bufferViews, index + 2);
			Json::Value saj = Json::Value(Json::arrayValue);
			for (uint32_t i = 0; i < animation->samplers.size(); i++)
			{
				Animation::Sampler& sampler = animation->samplers[i];
				if (sampler.used)
				{
					WriteAccessor(sampler.input, accessors, index);
					index++;
				}
				WriteAccessor(sampler.output, accessors, index);
				index++;
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
			animations[0u] = aj;
			root["animations"] = animations;
		}

		Json::Value obj;
		obj["nodes"] = ns;
		scenes[0u] = obj;
		WriteBuffer(buffers, 0);

		root["scenes"] = scenes;
		root["nodes"] = nodes;
		root["meshes"] = meshes;
		root["buffers"] = buffers;
		root["bufferViews"] = bufferViews;
		root["accessors"] = accessors;
		root["materials"] = materials;
		root["textures"] = textures;
		root["samplers"] = samplers;
		root["images"] = images;
	}

	std::shared_ptr<Node> glTFModelExporter::ExportNode()
	{
		std::shared_ptr<Node> node = std::make_shared<Node>();
		node->index = 0;
		node->mesh = ExportMesh();
		for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			if (bone.parent == -1)
				node->children.push_back(bone.nIndex + 1);
		}
		if (paraXModel->animated)
		{
			node->skin = ExportSkin();
		}
		return node;
	}

	std::shared_ptr<Mesh> glTFModelExporter::ExportMesh()
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		mesh->primitives.resize(1);
		Mesh::Primitive& primitive = mesh->primitives.back();
		primitive.indices = ExportIndices();
		primitive.attributes.position = ExportVertices();
		primitive.attributes.normal = ExportNormals();
		primitive.attributes.texcoord = ExportTextureCoords();
		if (paraXModel->m_origVertices[0].color0 != 0)
			primitive.attributes.color = ExportColors();
		if (paraXModel->animated)
		{
			primitive.attributes.joints = ExportJoints();
			primitive.attributes.weights = ExportWeights();
		}
		primitive.material = ExportMaterials();
		primitive.mode = PrimitiveMode::Triangles;
		mesh->index = 0;
		return mesh;
	}

	std::shared_ptr<Skin> glTFModelExporter::ExportSkin()
	{
		std::shared_ptr<Skin> skin = std::make_shared<Skin>();
		uint32_t numBones = paraXModel->m_objNum.nBones;
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			std::shared_ptr<Node> node = std::make_shared<Node>();
			node->index = bone.nIndex + 1;
			node->translation = bone.m_finalTrans;
			node->rotation = bone.m_finalRot;
			node->scale = bone.m_finalScaling;
			skin->joints.push_back(node);
		}
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			if (bone.parent != -1)
				skin->joints[bone.parent]->children.push_back(bone.nIndex + 1);
		}
		skin->inverseBindMatrices = ExportMatrices();
		skin->index = 0;
		return skin;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportMatrices()
	{
		uint32_t numBones = paraXModel->m_objNum.nBones;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::MAT4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numBones * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
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

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportVertices()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxVertex[i]);
			acc->min.push_back(minVertex[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportNormals()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxNormal[i]);
			acc->min.push_back(minNormal[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTextureCoords()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC2);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC2;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxCoord[i]);
			acc->min.push_back(minCoord[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportColors()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC3;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxColor[i]);
			acc->min.push_back(minCoord[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportJoints()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::UnsignedByte);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::UnsignedByte;
		acc->count = numVertices;
		acc->type = AttribType::VEC4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxJoint[i]);
			acc->min.push_back(minJoint[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportWeights()
	{
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numVertices * numComponents * bytesPerComp;
		bv->byteStride = numComponents * bytesPerComp;
		bv->target = BufferViewTarget::ArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::Float;
		acc->count = numVertices;
		acc->type = AttribType::VEC4;
		for (uint32_t i = 0; i < numComponents; i++)
		{
			acc->max.push_back(maxWeight[i]);
			acc->min.push_back(minWeight[i]);
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportIndices()
	{
		uint32_t numIndices = paraXModel->m_objNum.nIndices;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::UnsignedShort);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numIndices * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::ElementArrayBuffer;
		buffer->Grow(bv->byteLength);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::UnsignedShort;
		acc->type = AttribType::SCALAR;
		acc->count = numIndices;
		acc->max.push_back(0);
		acc->min.push_back(0);
		for (unsigned int i = 0; i < numIndices; i++)
		{
			uint16_t val = paraXModel->m_indices[i];
			if (val > acc->max[0]) acc->max[0] = val;
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Material> glTFModelExporter::ExportMaterials()
	{
		std::shared_ptr<Material> material = std::make_shared<Material>();
		material->index = 0;
		material->alphaMode = "MASK";
		material->alphaCutoff = 0.5;
		material->doubleSide = true;
		PbrMetallicRoughness& metallic = material->metallicRoughness;
		metallic.metallicFactor = 0;
		metallic.roughnessFactor = 1;
		metallic.baseColorFactor[0] = 1.0;
		metallic.baseColorFactor[1] = 1.0;
		metallic.baseColorFactor[2] = 1.0;
		metallic.baseColorFactor[3] = 1.0;

		uint32_t numTextures = paraXModel->m_objNum.nTextures;
		if (numTextures > 0)
		{
			std::shared_ptr<Image> img = std::make_shared<Image>();
			LPD3DXBUFFER texBuffer;
			D3DXSaveTextureToFileInMemory(&texBuffer, D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, paraXModel->textures[0]->GetTexture(), nullptr);
			img->bufferPointer = texBuffer->GetBufferPointer();
			img->bufferSize = texBuffer->GetBufferSize();
			if (!isBinary)
			{
				std::string path = fileName.substr(0, fileName.rfind(".gltf"));
				std::string name = path.substr(path.find_last_of("/\\") + 1u);
				img->filename = path + ".png";
				img->uri = name + ".png";
			}
			img->index = 0;

			std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>();
			sampler->magFilter = SamplerMagFilter::MagLinear;
			sampler->minFilter = SamplerMinFilter::NearestMipMapLinear;
			sampler->wrapS = SamplerWrap::Repeat;
			sampler->wrapT = SamplerWrap::Repeat;
			sampler->index = 0;

			std::shared_ptr<Texture> texture = std::make_shared<Texture>();
			texture->source = img;
			texture->sampler = sampler;
			metallic.baseColorTexture.texture = texture;
			metallic.baseColorTexture.index = 0;
			metallic.baseColorTexture.texCoord = 0;
		}
		return material;
	}

	std::shared_ptr<Animation> glTFModelExporter::ExportAnimations()
	{
		std::shared_ptr<Animation> animation = std::make_shared<Animation>();

		bvTime = ExportTimeBuffer();
		bvTranslation = ExportTranslationBuffer();
		bvRotation = ExportRotationBuffer();
		uint32_t offsetTime = 0;
		uint32_t offsetData = 0;
		for (uint32_t i = 0; i < animOffsets.size(); i++)
		{
			uint32_t numDatas = animOffsets[i];
			std::shared_ptr<Accessor> acTime = ExportTimeAccessor(bvTime, offsetTime, numDatas);
			offsetTime += numDatas;

			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = true;
				sampler.input = acTime;
				sampler.output = ExportTranslationAccessor(bvTranslation, offsetData, numDatas);
				Animation::Channel channel;
				channel.target.node = boneIndices[i] + 1;
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
				sampler.output = ExportRotationAccessor(bvRotation, offsetData, numDatas);
				Animation::Channel channel;
				channel.target.node = boneIndices[i] + 1;
				channel.target.path = AnimationPath::Rotation;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			offsetData += numDatas;
		}
		return animation;
	}

	std::shared_ptr<BufferView> glTFModelExporter::ExportTimeBuffer()
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = animTimes.size() * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);
		return bv;
	}

	std::shared_ptr<BufferView> glTFModelExporter::ExportTranslationBuffer()
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex + 1;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = translations.size() * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);
		return bv;
	}

	std::shared_ptr<BufferView> glTFModelExporter::ExportRotationBuffer()
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex + 2;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = rotations.size() * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::NotUse;
		buffer->Grow(bv->byteLength);
		return bv;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTimeAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = offset * numComponents * bytesPerComp;
		acc->componentType = ComponentType::Float;
		acc->count = numDatas;
		acc->type = AttribType::SCALAR;
		acc->max.push_back(-FLT_MAX);
		acc->min.push_back(FLT_MAX);
		for (uint32_t i = offset; i < numDatas + offset; i++)
		{
			float val = animTimes[i];
			if (val < acc->min[0]) acc->min[0] = val;
			if (val > acc->max[0]) acc->max[0] = val;
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportTranslationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
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
				float val = translations[i][j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Accessor> glTFModelExporter::ExportRotationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC4);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
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
				float val = rotations[i][j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
		}

		bufferIndex++;
		return acc;
	}

	std::string glTFModelExporter::EncodeBuffer()
	{
		StringBuilder builder;
		uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
		uint32_t sizeUShort = ComponentTypeSize(ComponentType::UnsignedShort);
		uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);

		uint32_t numIndices = paraXModel->m_objNum.nIndices;
		for (uint32_t i = 0; i < numIndices; i++)
		{
			uint16_t index = paraXModel->m_indices[i];
			builder.append((const char*)&index, sizeUShort);
		}

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

		for (auto it = colors.begin(); it != colors.end(); ++it)
		{
			builder.append((const char*)&(it->x), sizeFloat);
			builder.append((const char*)&(it->y), sizeFloat);
			builder.append((const char*)&(it->z), sizeFloat);
		}

		if (paraXModel->animated)
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
			uint32_t numBones = paraXModel->m_objNum.nBones;
			for (uint32_t i = 0; i < numBones; i++)
			{
				builder.append((const char*)&paraXModel->bones[i].matOffset, sizeFloat * 16);
			}
			for (uint32_t i = 0; i < animTimes.size(); i++)
			{
				float val = animTimes[i];
				builder.append((const char*)&val, sizeFloat);
			}
			for (uint32_t i = 0; i < translations.size(); i++)
			{
				builder.append((const char*)&translations[i].x, sizeFloat);
				builder.append((const char*)&translations[i].y, sizeFloat);
				builder.append((const char*)&translations[i].z, sizeFloat);
			}
			for (uint32_t i = 0; i < rotations.size(); i++)
			{
				builder.append((const char*)&rotations[i].x, sizeFloat);
				builder.append((const char*)&rotations[i].y, sizeFloat);
				builder.append((const char*)&rotations[i].z, sizeFloat);
				builder.append((const char*)&rotations[i].w, sizeFloat);
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
			if (isEmbedded)
			{
				b["uri"] = "data:application/octet-stream;base64," + EncodeBuffer();
			}
			else
				b["uri"] = buffer->uri;
		}
		obj[index] = b;
	}

	void glTFModelExporter::WriteBufferView(std::shared_ptr<BufferView>& bufferView, Json::Value& obj, uint32_t index)
	{
		Json::Value bv;
		bv["buffer"] = bufferView->buffer->index;
		bv["byteOffset"] = bufferView->byteOffset;
		bv["byteLength"] = bufferView->byteLength;
		if (bufferView->byteStride != 0)
			bv["byteStride"] = bufferView->byteStride;
		if (bufferView->target != BufferViewTarget::NotUse)
			bv["target"] = bufferView->target;
		obj[index] = bv;
	}

	void glTFModelExporter::WriteAccessor(std::shared_ptr<Accessor>& accessor, Json::Value& obj, uint32_t index)
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
		obj[index] = acc;
	}

	void glTFModelExporter::WriteMaterial(std::shared_ptr<Material>& material, Json::Value& mat, Json::Value& tex, Json::Value& sampler, Json::Value& img)
	{
		PbrMetallicRoughness& pbr = material->metallicRoughness;
		Json::Value baseTex;
		baseTex["index"] = pbr.baseColorTexture.index;
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
		mat[0u] = m;

		shared_ptr<Texture>& texture = pbr.baseColorTexture.texture;
		Json::Value t;
		t["sampler"] = texture->sampler->index;
		t["source"] = texture->source->index;
		tex[0u] = t;

		Json::Value s;
		s["magFilter"] = texture->sampler->magFilter;
		s["minFilter"] = texture->sampler->minFilter;
		s["wrapS"] = texture->sampler->wrapS;
		s["wrapT"] = texture->sampler->wrapT;
		sampler[0u] = s;

		Json::Value i;
		if (isEmbedded)
		{
			StringBuilder builder;
			builder.append((const char*)texture->source->bufferPointer, texture->source->bufferSize);
			i["uri"] = "data:image/png;base64," + StringHelper::base64(builder.ToString());
		}
		else
		{
			i["uri"] = texture->source->uri;
			CParaFile file;
			if (file.CreateNewFile(texture->source->filename.c_str()))
			{
				file.write(texture->source->bufferPointer, texture->source->bufferSize);
				file.close();
			}
		}
		img[0u] = i;
	}

	void glTFModelExporter::WriteFile()
	{
		CParaFile file;
		if (file.CreateNewFile(fileName.c_str()))
		{
			Json::StyledWriter writer;
			std::string& data = writer.write(root);
			file.write(data.c_str(), data.length());
			file.close();

			if (!isEmbedded)
			{
				WriteRawData();
			}
		}
	}

	void glTFModelExporter::WriteRawData()
	{
		CParaFile bin;
		if (bin.CreateNewFile(buffer->filename.c_str()))
		{
			uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
			uint32_t sizeUShort = ComponentTypeSize(ComponentType::UnsignedShort);
			uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);
			uint32_t numIndices = paraXModel->m_objNum.nIndices;
			for (uint32_t i = 0; i < numIndices; i++)
			{
				uint16_t index = paraXModel->m_indices[i];
				bin.write(&index, sizeUShort);
			}
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

			for (auto it = colors.begin(); it != colors.end(); ++it)
			{
				bin.write(&(it->x), sizeFloat);
				bin.write(&(it->y), sizeFloat);
				bin.write(&(it->z), sizeFloat);
			}

			if (paraXModel->animated)
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
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					bin.write(&paraXModel->bones[i].matOffset, sizeFloat * 16);
				}
				for (uint32_t i = 0; i < animTimes.size(); i++)
				{
					float val = animTimes[i];
					bin.write(&val, sizeFloat);
				}
				for (uint32_t i = 0; i < translations.size(); i++)
				{
					bin.write(&translations[i].x, sizeFloat);
					bin.write(&translations[i].y, sizeFloat);
					bin.write(&translations[i].z, sizeFloat);
				}
				for (uint32_t i = 0; i < rotations.size(); i++)
				{
					bin.write(&rotations[i].x, sizeFloat);
					bin.write(&rotations[i].y, sizeFloat);
					bin.write(&rotations[i].z, sizeFloat);
					bin.write(&rotations[i].w, sizeFloat);
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
			std::string& data = writer.write(root);
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
			uint32_t sizeUShort = ComponentTypeSize(ComponentType::UnsignedShort);
			uint32_t sizeUByte = ComponentTypeSize(ComponentType::UnsignedByte);
			uint32_t numIndices = paraXModel->m_objNum.nIndices;
			for (uint32_t i = 0; i < numIndices; i++)
			{
				uint16_t index = paraXModel->m_indices[i];
				file.write(&index, sizeUShort);
			}
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

			for (auto it = colors.begin(); it != colors.end(); ++it)
			{
				file.write(&(it->x), sizeFloat);
				file.write(&(it->y), sizeFloat);
				file.write(&(it->z), sizeFloat);
			}

			if (paraXModel->animated)
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
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					file.write(&paraXModel->bones[i].matOffset, sizeFloat * 16);
				}
				for (uint32_t i = 0; i < animTimes.size(); i++)
				{
					float val = animTimes[i];
					file.write(&val, sizeFloat);
				}
				for (uint32_t i = 0; i < translations.size(); i++)
				{
					file.write(&translations[i].x, sizeFloat);
					file.write(&translations[i].y, sizeFloat);
					file.write(&translations[i].z, sizeFloat);
				}
				for (uint32_t i = 0; i < rotations.size(); i++)
				{
					file.write(&rotations[i].x, sizeFloat);
					file.write(&rotations[i].y, sizeFloat);
					file.write(&rotations[i].z, sizeFloat);
					file.write(&rotations[i].w, sizeFloat);
				}
			}
			uint8_t binaryPadding = 0x00;
			paddingLength = binaryLength - buffer->byteLength;
			for (uint32_t i = 0; i < paddingLength; i++)
				file.write(&binaryPadding, 1);

			file.close();
		}
	}

	void glTFModelExporter::ParaXExportTo_glTF(const std::string& input, const std::string& output, bool binary, bool embedded)
	{
		CParaFile file(input.c_str());
		CGlobals::GetAssetManager()->SetAsyncLoading(false);
		CParaXSerializer serializer;
		CParaXModel* mesh = (CParaXModel*)serializer.LoadParaXMesh(file);
		if (mesh != nullptr)
		{
			std::string filename = output.empty() ? (input.substr(0, input.rfind(".x")) + ".gltf") : output;
			glTFModelExporter exporter(filename, mesh, binary, embedded);
			delete mesh;
			mesh = nullptr;
		}
	}

}