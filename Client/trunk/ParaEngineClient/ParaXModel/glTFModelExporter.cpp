#include "glTFModelExporter.h"
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaXSerializer.h"
#include "StringHelper.h"
#include "StringBuilder.h"
#include "ParaWorldAsset.h"

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
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		scene->nodes.push_back(ExportNode());

		root["scene"] = 0;
		Json::Value scenes = Json::Value(Json::arrayValue);
		Json::Value nodes = Json::Value(Json::arrayValue);
		Json::Value meshes = Json::Value(Json::arrayValue);
		Json::Value buffers = Json::Value(Json::arrayValue);
		Json::Value bufferViews = Json::Value(Json::arrayValue);
		Json::Value accessors = Json::Value(Json::arrayValue);
		Json::Value materials = Json::Value(Json::arrayValue);
		Json::Value textures = Json::Value(Json::arrayValue);
		Json::Value images = Json::Value(Json::arrayValue);
		Json::Value samplers = Json::Value(Json::arrayValue);

		Json::Value ns = Json::Value(Json::arrayValue);
		for (uint32_t i = 0, len = scene->nodes.size(); i < len; i++)
		{
			std::shared_ptr<Node>& node = scene->nodes[i];
			ns[i] = node->index;
			for (uint32_t j =  0; j < node->meshes.size(); j++)
			{
				std::shared_ptr<Mesh>& mesh = node->meshes[j];
				Json::Value m;
				m["mesh"] = mesh->index;
				nodes[j] = m;

				Json::Value ps = Json::Value(Json::arrayValue);
				for (uint32_t k = 0; k < mesh->primitives.size(); k++)
				{
					Mesh::Primitive& primitive = mesh->primitives[k];
					Json::Value a;
					uint32_t index = 0;
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
					if (primitive.indices != nullptr)
					{
						WriteAccessor(primitive.indices, accessors, index);
						WriteBufferView(primitive.indices->bufferView, bufferViews, index);
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
				meshes[j] = obj;
			}
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
		node->meshes.push_back(ExportMesh());
		node->index = 0;
		return node;
	}

	std::shared_ptr<Mesh> glTFModelExporter::ExportMesh()
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		mesh->primitives.resize(1);
		Mesh::Primitive& primitive = mesh->primitives.back();
		primitive.attributes.position = ExportVertices();
		primitive.attributes.normal = ExportNormals();
		primitive.attributes.texcoord = ExportTextureCoords();
		if (paraXModel->m_origVertices[0].color0 != 0)
			primitive.attributes.color = ExportColors();
		primitive.material = ExportMaterials();
		primitive.indices = ExportIndices();
		primitive.mode = PrimitiveMode::Triangles;
		mesh->index = 0;
		return mesh;
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
		acc->max.push_back(FLT_MIN);
		acc->max.push_back(FLT_MIN);
		acc->max.push_back(FLT_MIN);
		acc->min.push_back(FLT_MAX);
		acc->min.push_back(FLT_MAX);
		acc->min.push_back(FLT_MAX);
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
		acc->max.push_back(-1.0);
		acc->max.push_back(-1.0);
		acc->max.push_back(-1.0);
		acc->min.push_back(1.0);
		acc->min.push_back(1.0);
		acc->min.push_back(1.0);
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
		acc->max.push_back(0);
		acc->max.push_back(0);
		acc->min.push_back(1.0);
		acc->min.push_back(1.0);
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
		acc->max.push_back(0);
		acc->max.push_back(0);
		acc->max.push_back(0);
		acc->min.push_back(1.0);
		acc->min.push_back(1.0);
		acc->min.push_back(1.0);
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
			sampler->minFilter = SamplerMinFilter::LinearMipMapLinear;
			sampler->wrapS = SamplerWrap::ClampToEdge;
			sampler->wrapT = SamplerWrap::ClampToEdge;
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

	std::string glTFModelExporter::EncodeBuffer()
	{
		StringBuilder builder;
		uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
		uint32_t sizeUShort = ComponentTypeSize(ComponentType::UnsignedShort);
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
		uint32_t numIndices = paraXModel->m_objNum.nIndices;
		for (uint32_t i = 0; i < numIndices; i++)
		{
			uint16_t index = paraXModel->m_indices[i];
			builder.append((const char*)&index, sizeUShort);
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
			if (accessor->type == AttribType::SCALAR)
				jmax[i] = (uint32_t)accessor->max[i];
			else
				jmax[i] = accessor->max[i];
		}
		acc["max"] = jmax;
		Json::Value jmin = Json::Value(Json::arrayValue);
		for (uint32_t i = 0; i < accessor->min.size(); i++)
		{
			if (accessor->type == AttribType::SCALAR)
				jmin[i] = (uint32_t)accessor->min[i];
			else
				jmin[i] = accessor->min[i];
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
			uint32_t numIndices = paraXModel->m_objNum.nIndices;
			for (uint32_t i = 0; i < numIndices; i++)
			{
				uint16_t index = paraXModel->m_indices[i];
				bin.write(&index, sizeUShort);
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
			uint32_t numIndices = paraXModel->m_objNum.nIndices;
			for (uint32_t i = 0; i < numIndices; i++)
			{
				uint16_t index = paraXModel->m_indices[i];
				file.write(&index, sizeUShort);
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