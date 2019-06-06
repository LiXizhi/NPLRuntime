#include "glTFModelExporter.h"
#include "ParaXModel.h"
#include "ParaXSerializer.h"
#include "StringHelper.h"
#include "StringBuilder.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"

namespace ParaEngine
{

	glTFModelExporter::glTFModelExporter(const std::string& filename, CParaXModel* mesh, bool binary, bool encode)
		: fileName(filename),
		paraXModel(mesh),
		buffer(std::make_shared<Buffer>()),
		bufferIndex(0),
		isBinary(binary),
		willEncode(encode)
	{
		if (!isBinary)
		{
			std::string path = filename.substr(0, filename.rfind(".gltf"));
			std::string name = path.substr(path.find_last_of("/\\") + 1u);
			buffer->filename = path + ".bin";
			buffer->uri = name + ".bin";
		}
		buffer->index = 0;

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
		//paraXModel->m_CurrentAnim = paraXModel->GetAnimIndexByID(0);
		//for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
		//{
		//	Bone& bone = paraXModel->bones[i];
		//	bone.calcMatrix(paraXModel->bones, paraXModel->m_CurrentAnim, paraXModel->m_BlendingAnim, paraXModel->blendingFactor);
		//}
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
			acc->max.push_back(-FLT_MAX);
			acc->min.push_back(FLT_MAX);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = paraXModel->m_origVertices[i].pos[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
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
			acc->max.push_back(-1.0);
			acc->min.push_back(1.0);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = paraXModel->m_origVertices[i].normal[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
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
			acc->max.push_back(0.0);
			acc->min.push_back(1.0);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			for (uint32_t j = 0; j < numComponents; j++)
			{
				float val = paraXModel->m_origVertices[i].texcoords[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
			}
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
			acc->max.push_back(0.0);
			acc->min.push_back(1.0);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			DWORD color = paraXModel->m_origVertices[i].color0;
			float r = ((color >> 16) & 0xff) / 255.0f;
			float g = ((color >> 8) & 0xff) / 255.0f;
			float b = (color & 0xff) / 255.0f;
			if (r < acc->min[0]) acc->min[0] = r;
			if (g < acc->min[1]) acc->min[1] = g;
			if (b < acc->min[2]) acc->min[2] = b;
			if (r > acc->max[0]) acc->max[0] = r;
			if (g > acc->max[1]) acc->max[1] = g;
			if (b > acc->max[2]) acc->max[2] = b;
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
			acc->max.push_back(0);
			acc->min.push_back(255);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			uint8* bone = paraXModel->m_origVertices[i].bones;
			uint8 a = bone[0];
			uint8 b = bone[1];
			uint8 c = bone[2];
			uint8 d = bone[3];
			if (a < acc->min[0]) acc->min[0] = a;
			if (b < acc->min[1]) acc->min[1] = b;
			if (c < acc->min[2]) acc->min[2] = c;
			if (d < acc->min[3]) acc->min[3] = d;
			if (a > acc->max[0]) acc->max[0] = a;
			if (b > acc->max[1]) acc->max[1] = b;
			if (c > acc->max[2]) acc->max[2] = c;
			if (d > acc->max[3]) acc->max[3] = d;
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
			acc->max.push_back(0.0);
			acc->min.push_back(1.0);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			uint8* weight = paraXModel->m_origVertices[i].weights;
			float a = weight[0] * (1 / 255.0f);
			float b = weight[1] * (1 / 255.0f);
			float c = weight[2] * (1 / 255.0f);
			float d = weight[3] * (1 / 255.0f);
			if (a < acc->min[0]) acc->min[0] = a;
			if (b < acc->min[1]) acc->min[1] = b;
			if (c < acc->min[2]) acc->min[2] = c;
			if (d < acc->min[3]) acc->min[3] = d;
			if (a > acc->max[0]) acc->max[0] = a;
			if (b > acc->max[1]) acc->max[1] = b;
			if (c > acc->max[2]) acc->max[2] = c;
			if (d > acc->max[3]) acc->max[3] = d;
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

		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::VEC3);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		uint32_t numBones = paraXModel->m_objNum.nBones;
		uint32_t animLength = paraXModel->anims[0].timeEnd - paraXModel->anims[0].timeStart;
		std::vector<int> bones;
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
					animIndices.push_back(std::make_pair(secondR - firstR + 1, secondR - firstR + 1));
					if (firstT == secondT)
					{
						for (uint32_t j = firstR; j <= secondR; j++)
						{
							animTimes.push_back(bone.rot.times[j] * 1.0f / 1000.0f);
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
							animTimes.push_back(time * 1.0f / 1000.0f);
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
					animIndices.push_back(std::make_pair(secondT - firstT + 1, secondT - firstT + 1));
					if (firstR == secondR)
					{
						Quaternion q = bone.rot.data[firstR];
						for (uint32_t j = firstT; j <= secondT; j++)
						{
							animTimes.push_back(bone.trans.times[j] * 1.0f / 1000.0f);
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
							animTimes.push_back(time * 1.0f / 1000.0f);
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
						animIndices.push_back(std::make_pair(2, 2));
						animTimes.push_back(0);
						animTimes.push_back(animLength * 1.0f / 1000.0f);
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
						animIndices.push_back(std::make_pair(secondR - firstR + 1, secondR - firstR + 1));
						for (uint32_t j = firstR; j <= secondR; j++)
						{
							animTimes.push_back(bone.rot.times[j] * 1.0f / 1000.0f);
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
				bones.push_back(bone.nIndex);
			}
			else if (bone.trans.used)
			{
				//if (firstT == secondT)
				//{
				//	animIndices.push_back(std::make_pair(2, 0));
				//	animTimes.push_back(0);
				//	animTimes.push_back(animLength * 1.0f / 1000.0f);
				//	Vector3 trans = bone.trans.data[firstT];
				//	translations.push_back(trans);
				//	translations.push_back(trans);
				//}
				//else
				//{
				//	animIndices.push_back(std::make_pair(secondT - firstT + 1, 0));
				//	for (uint32_t j = firstT; j <= secondT; j++)
				//	{
				//		animTimes.push_back(bone.rot.times[j] * 1.0f / 1000.0f);
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
					animIndices.push_back(std::make_pair(2, 2));
					animTimes.push_back(0);
					animTimes.push_back(animLength * 1.0f / 1000.0f);
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
					animIndices.push_back(std::make_pair(secondR - firstR + 1, secondR - firstR + 1));
					for (uint32_t j = firstR; j <= secondR; j++)
					{
						animTimes.push_back(bone.rot.times[j] * 1.0f / 1000.0f);
						Quaternion q = bone.rot.data[j];
						q.invertWinding();
						Vector3 trans(0, 0, 0);
						if (bone.bUsePivot)
							trans = CalculatePivot(bone.pivot, trans, Matrix4(q));
						translations.push_back(trans);
						rotations.push_back(q);
					}
				}
				bones.push_back(bone.nIndex);
			}
		}

		bvTime = ExportTimeBuffer();
		bvTranslation = ExportTranslationBuffer();
		bvRotation = ExportRotationBuffer();
		uint32_t offsetTime = 0;
		uint32_t offsetTranslation = 0;
		uint32_t offsetRotation = 0;
		for (uint32_t i = 0; i < animIndices.size(); i++)
		{
			uint32_t numTranslations = animIndices[i].first;
			uint32_t numRotations = animIndices[i].second;
			uint32_t numDatas = std::max(numTranslations, numRotations);
			std::shared_ptr<Accessor> acTime = ExportTimeAccessor(bvTime, offsetTime, numDatas);
			offsetTime += numDatas;

			if (numTranslations > 0)
			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = true;
				sampler.input = acTime;
				sampler.output = ExportTranslationAccessor(bvTranslation, offsetTranslation, numDatas);
				offsetTranslation += numDatas;
				Animation::Channel channel;
				channel.target.node = bones[i] + 1;
				channel.target.path = AnimationPath::Translation;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			if (numRotations > 0)
			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = numTranslations == 0;
				sampler.input = acTime;
				sampler.output = ExportRotationAccessor(bvRotation, offsetRotation, numDatas);
				offsetRotation += numDatas;
				Animation::Channel channel;
				channel.target.node = bones[i] + 1;
				channel.target.path = AnimationPath::Rotation;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
		}
		return animation;
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
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		for (uint32_t i = 0; i < numVertices; i++)
		{
			const ModelVertex& vertex = paraXModel->m_origVertices[i];
			builder.append((const char*)&vertex.pos.x, sizeFloat);
			builder.append((const char*)&vertex.pos.y, sizeFloat);
			builder.append((const char*)&vertex.pos.z, sizeFloat);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			const ModelVertex& vertex = paraXModel->m_origVertices[i];
			builder.append((const char*)&vertex.normal.x, sizeFloat);
			builder.append((const char*)&vertex.normal.y, sizeFloat);
			builder.append((const char*)&vertex.normal.z, sizeFloat);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			const ModelVertex& vertex = paraXModel->m_origVertices[i];
			builder.append((const char*)&vertex.texcoords.x, sizeFloat);
			builder.append((const char*)&vertex.texcoords.y, sizeFloat);
		}
		for (uint32_t i = 0; i < numVertices; i++)
		{
			DWORD color = paraXModel->m_origVertices[i].color0;
			if (color == 0) break;
			float r = ((color >> 16) & 0xff) / 255.0f;
			float g = ((color >> 8) & 0xff) / 255.0f;
			float b = (color & 0xff) / 255.0f;
			builder.append((const char*)&r, sizeFloat);
			builder.append((const char*)&g, sizeFloat);
			builder.append((const char*)&b, sizeFloat);
		}
		if (paraXModel->animated)
		{
			for (uint32_t i = 0; i < numVertices; i++)
			{
				uint8* bone = paraXModel->m_origVertices[i].bones;
				builder.append((const char*)bone, sizeUByte * 4);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				uint8* weight = paraXModel->m_origVertices[i].weights;
				float a = weight[0] * (1 / 255.0f);
				float b = weight[1] * (1 / 255.0f);
				float c = weight[2] * (1 / 255.0f);
				float d = weight[3] * (1 / 255.0f);
				builder.append((const char*)&a, sizeFloat);
				builder.append((const char*)&b, sizeFloat);
				builder.append((const char*)&c, sizeFloat);
				builder.append((const char*)&d, sizeFloat);
			}
			uint32_t numBones = paraXModel->m_objNum.nBones;
			for (uint32_t i = 0; i < numBones; i++)
			{
				Matrix4 mat = paraXModel->bones[i].matOffset;
				for (uint32_t j = 0; j < 16; j++)
				{
					builder.append((const char*)&mat._m[j], sizeFloat);
				}
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
			if (willEncode)
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
		if (willEncode)
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

			if (!willEncode)
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
			uint32_t numVertices = paraXModel->m_objNum.nVertices;
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				bin.write(&vertex.pos.x, sizeFloat);
				bin.write(&vertex.pos.y, sizeFloat);
				bin.write(&vertex.pos.z, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				bin.write(&vertex.normal.x, sizeFloat);
				bin.write(&vertex.normal.y, sizeFloat);
				bin.write(&vertex.normal.z, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				bin.write(&vertex.texcoords.x, sizeFloat);
				bin.write(&vertex.texcoords.y, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				DWORD color = paraXModel->m_origVertices[i].color0;
				if (color == 0) break;
				float r = ((color >> 16) & 0xff) / 255.0f;
				float g = ((color >> 8) & 0xff) / 255.0f;
				float b = (color & 0xff) / 255.0f;
				bin.write(&r, sizeFloat);
				bin.write(&g, sizeFloat);
				bin.write(&b, sizeFloat);
			}
			if (paraXModel->animated)
			{
				for (uint32_t i = 0; i < numVertices; i++)
				{
					uint8* bone = paraXModel->m_origVertices[i].bones;
					bin.write(bone, sizeUByte * 4);
				}
				for (uint32_t i = 0; i < numVertices; i++)
				{
					uint8* weight = paraXModel->m_origVertices[i].weights;
					float a = weight[0] * (1 / 255.0f);
					float b = weight[1] * (1 / 255.0f);
					float c = weight[2] * (1 / 255.0f);
					float d = weight[3] * (1 / 255.0f);
					bin.write(&a, sizeFloat);
					bin.write(&b, sizeFloat);
					bin.write(&c, sizeFloat);
					bin.write(&d, sizeFloat);
				}
				for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
				{
					Matrix4 mat = paraXModel->bones[i].matOffset;
					mat.transpose();
					for (uint32_t j = 0; j < 16; j++)
					{
						bin.write(&mat._m[j], sizeFloat);
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
			uint32_t numVertices = paraXModel->m_objNum.nVertices;
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				file.write(&vertex.pos.x, sizeFloat);
				file.write(&vertex.pos.y, sizeFloat);
				file.write(&vertex.pos.z, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				file.write(&vertex.normal.x, sizeFloat);
				file.write(&vertex.normal.y, sizeFloat);
				file.write(&vertex.normal.z, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				const ModelVertex& vertex = paraXModel->m_origVertices[i];
				file.write(&vertex.texcoords.x, sizeFloat);
				file.write(&vertex.texcoords.y, sizeFloat);
			}
			for (uint32_t i = 0; i < numVertices; i++)
			{
				DWORD color = paraXModel->m_origVertices[i].color0;
				if (color == 0) break;
				float r = ((color >> 16) & 0xff) / 255.0f;
				float g = ((color >> 8) & 0xff) / 255.0f;
				float b = (color & 0xff) / 255.0f;
				file.write(&r, sizeFloat);
				file.write(&g, sizeFloat);
				file.write(&b, sizeFloat);
			}
			if (paraXModel->animated)
			{
				for (uint32_t i = 0; i < numVertices; i++)
				{
					uint8* bone = paraXModel->m_origVertices[i].bones;
					file.write(bone, sizeUByte * 4);
				}
				for (uint32_t i = 0; i < numVertices; i++)
				{
					uint8* weight = paraXModel->m_origVertices[i].weights;
					float a = weight[0] * (1 / 255.0f);
					float b = weight[1] * (1 / 255.0f);
					float c = weight[2] * (1 / 255.0f);
					float d = weight[3] * (1 / 255.0f);
					file.write(&a, sizeFloat);
					file.write(&b, sizeFloat);
					file.write(&c, sizeFloat);
					file.write(&d, sizeFloat);
				}
				for (uint32_t i = 0; i < paraXModel->m_objNum.nBones; i++)
				{
					Matrix4 mat = paraXModel->bones[i].matOffset;
					mat.transpose();
					for (uint32_t j = 0; j < 16; j++)
					{
						file.write(&mat._m[j], sizeFloat);
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

	void glTFModelExporter::ParaXExportTo_glTF(const std::string& input, const std::string& output, bool binary, bool encode)
	{
		CParaFile file(input.c_str());
		CGlobals::GetAssetManager()->SetAsyncLoading(false);
		CParaXSerializer serializer;
		CParaXModel* mesh = (CParaXModel*)serializer.LoadParaXMesh(file);
		if (mesh != nullptr)
		{
			std::string filename = output.empty() ? (input.substr(0, input.rfind(".x")) + ".gltf") : output;
			glTFModelExporter exporter(filename, mesh, binary, encode);
			delete mesh;
			mesh = nullptr;
		}
	}

}