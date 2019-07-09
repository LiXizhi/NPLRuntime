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

	glTFModelExporter::glTFModelExporter(CParaXModel* mesh, bool binary, bool embedded /*= true*/)
		: maxVertex(-FLT_MAX, -FLT_MAX, -FLT_MAX), minVertex(FLT_MAX, FLT_MAX, FLT_MAX),
		maxNormal(-FLT_MAX, -FLT_MAX, -FLT_MAX), minNormal(FLT_MAX, FLT_MAX, FLT_MAX),
		maxColor(-FLT_MAX, -FLT_MAX, -FLT_MAX), minColor(FLT_MAX, FLT_MAX, FLT_MAX),
		maxCoord(-FLT_MAX, -FLT_MAX), minCoord(FLT_MAX, FLT_MAX),
		maxJoint(0, 0, 0, 0), minJoint(255, 255, 255, 255),
		maxWeight(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX), minWeight(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX),
		paraXModel(mesh),
		bufferIndex(0),
		isBinary(binary),
		isEmbedded(embedded),
		buffer(std::make_shared<Buffer>())
	{
		buffer->index = 0;
		ParseParaXModel();
	}

	glTFModelExporter::~glTFModelExporter()
	{

	}

	void glTFModelExporter::ExportToFile(const std::string& filename)
	{
		fileName = filename;
		if (!isBinary)
		{
			std::string path = filename.substr(0, filename.rfind(".gltf"));
			std::string name = path.substr(path.find_last_of("/\\") + 1u);
			buffer->filename = path + ".bin";
			buffer->uri = name + ".bin";
		}

		ExportMetadata();
		ExportScene();
		if (isBinary)
			WriteGLBFile();
		else
			WriteFile();
	}

	std::string glTFModelExporter::ExportToBuffer()
	{
		ExportMetadata();
		ExportScene();
		if (isBinary)
		{
			Json::FastWriter writer;
			std::string& data = writer.write(root);
			uint32_t jsonLength = (data.length() + 3) & (~3);
			uint32_t binaryLength = (buffer->byteLength + 3) & (~3);

			StringBuilder builder;
			GLBHeader header;
			header.magic = 0x46546C67;
			header.version = 2;
			header.length = sizeof(GLBHeader) + 2 * sizeof(GLBChunk) + jsonLength + binaryLength;
			builder.append((const char*)&header, sizeof(GLBHeader));

			GLBChunk jsonChunk;
			jsonChunk.chunkLength = jsonLength;
			jsonChunk.chunkType = ChunkType::JSON;
			builder.append((const char*)&jsonChunk, sizeof(GLBChunk));
			builder.append(data.c_str(), data.length());
			uint8_t jsonPadding = 0x20;
			uint32_t paddingLength = jsonLength - data.length();
			for (uint32_t i = 0; i < paddingLength; i++)
				builder.append((const char*)&jsonPadding, 1);

			GLBChunk binaryChunk;
			binaryChunk.chunkLength = binaryLength;
			binaryChunk.chunkType = ChunkType::BIN;
			builder.append((const char*)&binaryChunk, sizeof(GLBChunk));
			uint32_t sizeFloat = ComponentTypeSize(ComponentType::Float);
			uint32_t sizeUShort = ComponentTypeSize(ComponentType::UnsignedShort);
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
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						builder.append((const char*)&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						builder.append("\0\0", sizeUShort);
				}
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					builder.append((const char*)&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
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
				for (uint32_t i = 0; i < scales.size(); i++)
				{
					builder.append((const char*)&scales[i].x, sizeFloat);
					builder.append((const char*)&scales[i].y, sizeFloat);
					builder.append((const char*)&scales[i].z, sizeFloat);
				}
			}
			else
			{
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						builder.append((const char*)&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						builder.append("\0\0", sizeUShort);
				}
			}
			uint8_t binaryPadding = 0x00;
			paddingLength = binaryLength - buffer->byteLength;
			for (uint32_t i = 0; i < paddingLength; i++)
				builder.append((const char*)&binaryPadding, 1);
			return builder.ToString();
		}
		else
		{
			Json::FastWriter writer;
			return writer.write(root);
		}
	}

	void glTFModelExporter::ParseParaXModel()
	{
		uint32_t numBones = paraXModel->m_objNum.nBones;
		//// calculate bones' absolute position
		//paraXModel->m_CurrentAnim = paraXModel->GetAnimIndexByID(0);
		//for (uint32_t i = 0; i < numBones; i++)
		//{
		//	paraXModel->bones[i].calcMatrix(paraXModel->bones, paraXModel->m_CurrentAnim, paraXModel->m_BlendingAnim, paraXModel->blendingFactor);
		//	paraXModel->bones[i].m_finalTrans = paraXModel->bones[i].GetAnimatedPivotPoint();
		//}
		//for (uint32_t i = 0; i < numBones; i++)
		//{
		//	Bone& bone = paraXModel->bones[i];
		//	if (bone.parent == -1)
		//	{
		//		bone.matTransform.makeTrans(bone.m_finalTrans);
		//		CalculateJoint(bone.nIndex, bone.m_finalTrans);
		//	}
		//}

		float inverse = 1.0f / 255.0f;
		uint32_t numVertices = paraXModel->m_objNum.nVertices;
		for (uint32_t i = 0; i < numVertices; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			Vector3 v = vertex.pos;
			Vector3 n = vertex.normal;
			Vector2 coord = vertex.texcoords;
			DWORD color = vertex.color0;
			{
				//float weight = vertex.weights[0] * inverse;
				//Bone& bone = paraXModel->bones[vertex.bones[0]];
				//v = (vertex.pos * bone.mat)*weight;
				//for (int b = 1; b < 4 && vertex.weights[b]>0; b++) {
				//	weight = vertex.weights[b] * inverse;
				//	Bone& bone = paraXModel->bones[vertex.bones[b]];
				//	v += (vertex.pos * bone.mat) * weight;
				//}
			}
			{
				//float weight = vertex.weights[0] * inverse;
				//Bone& bone = paraXModel->bones[vertex.bones[0]];
				//v -= bone.m_finalTrans * weight;
				//for (int b = 1; b < 4 && vertex.weights[b]>0; b++) {
				//	weight = vertex.weights[b] * inverse;
				//	Bone& bone = paraXModel->bones[vertex.bones[b]];
				//	v -= bone.m_finalTrans * weight;
				//}
			}

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
		if (animLength == 0) animLength = 1000;
		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
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

				animOffsets.push_back(animEnd - animStart + 1);
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

					animTimes.push_back(time * inverse);
					translations.push_back(mat.getTrans());
					rotations.push_back(q);
					scales.push_back(s);
				}
				boneIndices.push_back(bone.nIndex);
			}
		}
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
				Json::Value a;
				if (primitive.attributes.position != nullptr)
				{
					a["POSITION"] = primitive.attributes.position->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.position, accessors, index);
						WriteBufferView(primitive.attributes.position->bufferView, bufferViews, index);
						index++;
					}
				}
				if (primitive.attributes.normal != nullptr)
				{
					a["NORMAL"] = primitive.attributes.normal->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.normal, accessors, index);
						WriteBufferView(primitive.attributes.normal->bufferView, bufferViews, index);
						index++;
					}
				}
				if (primitive.attributes.texcoord != nullptr)
				{
					a["TEXCOORD_0"] = primitive.attributes.texcoord->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.texcoord, accessors, index);
						WriteBufferView(primitive.attributes.texcoord->bufferView, bufferViews, index);
						index++;
					}
				}
				if (primitive.attributes.color != nullptr)
				{
					a["COLOR_0"] = primitive.attributes.color->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.color, accessors, index);
						WriteBufferView(primitive.attributes.color->bufferView, bufferViews, index);
						index++;
					}
				}
				if (primitive.attributes.joints != nullptr)
				{
					a["JOINTS_0"] = primitive.attributes.joints->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.joints, accessors, index);
						WriteBufferView(primitive.attributes.joints->bufferView, bufferViews, index);
						index++;
					}
				}
				if (primitive.attributes.weights != nullptr)
				{
					a["WEIGHTS_0"] = primitive.attributes.weights->index;
					if (k == 0)
					{
						WriteAccessor(primitive.attributes.weights, accessors, index);
						WriteBufferView(primitive.attributes.weights->bufferView, bufferViews, index);
						index++;
					}
				}

				if (primitive.indices != nullptr)
				{
					WriteAccessor(primitive.indices, accessors, index);
					WriteBufferView(primitive.indices->bufferView, bufferViews, index);
					index++;
				}
				WriteMaterial(primitive.material, materials, textures, samplers, images, k);
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
			WriteBufferView(bvScale, bufferViews, index + 3);
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
		std::shared_ptr<Accessor> v = ExportVertices();
		std::shared_ptr<Accessor> n = ExportNormals();
		std::shared_ptr<Accessor> t = ExportTextureCoords();
		std::shared_ptr<Accessor> c = nullptr;
		if (paraXModel->m_origVertices[0].color0 != 0)
			c = ExportColors();
		std::shared_ptr<Accessor> j = ExportJoints();
		std::shared_ptr<Accessor> w = ExportWeights();
		for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
		{
			const ModelRenderPass& pass = paraXModel->passes[i];
			Mesh::Primitive primitive;
			primitive.indices = ExportIndices(pass);
			primitive.attributes.position = v;
			primitive.attributes.normal = n;
			primitive.attributes.texcoord = t;
			primitive.attributes.color = c;
			primitive.attributes.joints = j;
			primitive.attributes.weights = w;
			primitive.material = ExportMaterials(pass.tex, i);
			primitive.mode = PrimitiveMode::Triangles;
			mesh->primitives.push_back(primitive);
		}
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
			acc->min.push_back(minColor[i]);
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

	std::shared_ptr<Accessor> glTFModelExporter::ExportIndices(const ModelRenderPass& pass)
	{
		uint32_t numIndices = pass.indexCount;
		const uint32_t numComponents = AttribType::GetNumComponents(AttribType::SCALAR);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::UnsignedShort);

		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = numIndices * numComponents * bytesPerComp;
		bv->byteStride = 0;
		bv->target = BufferViewTarget::ElementArrayBuffer;
		buffer->Grow(bv->byteLength + bv->byteLength % 4);

		std::shared_ptr<Accessor> acc = std::make_shared<Accessor>();
		acc->bufferView = bv;
		acc->index = bufferIndex;
		acc->byteOffset = 0;
		acc->componentType = ComponentType::UnsignedShort;
		acc->type = AttribType::SCALAR;
		acc->count = numIndices;
		acc->max.push_back(0);
		acc->min.push_back(FLT_MAX);
		for (uint32_t i = pass.indexStart; i < pass.indexStart + pass.indexCount; i++)
		{
			uint16_t val = paraXModel->m_indices[i];
			if (val < acc->min[0]) acc->min[0] = val;
			if (val > acc->max[0]) acc->max[0] = val;
		}

		bufferIndex++;
		return acc;
	}

	std::shared_ptr<Material> glTFModelExporter::ExportMaterials(int tex, int index)
	{
		std::shared_ptr<Material> material = std::make_shared<Material>();
		material->index = index;
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
		if (numTextures > tex)
		{
			std::shared_ptr<Image> img = std::make_shared<Image>();
			LPD3DXBUFFER texBuffer;
			D3DXSaveTextureToFileInMemory(&texBuffer, D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, paraXModel->textures[tex]->GetTexture(), nullptr);
			img->bufferPointer = texBuffer->GetBufferPointer();
			img->bufferSize = texBuffer->GetBufferSize();
			if (!isBinary)
			{
				std::string path = fileName.substr(0, fileName.rfind(".gltf"));
				std::string name = path.substr(path.find_last_of("/\\") + 1u);
				img->filename = path + ".png";
				img->uri = name + ".png";
			}
			img->index = index;

			std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>();
			sampler->magFilter = SamplerMagFilter::MagLinear;
			sampler->minFilter = SamplerMinFilter::NearestMipMapLinear;
			sampler->wrapS = SamplerWrap::Repeat;
			sampler->wrapT = SamplerWrap::Repeat;
			sampler->index = index;

			std::shared_ptr<Texture> texture = std::make_shared<Texture>();
			texture->source = img;
			texture->sampler = sampler;
			metallic.baseColorTexture.texture = texture;
			metallic.baseColorTexture.index = index;
			metallic.baseColorTexture.texCoord = 0;
		}
		return material;
	}

	std::shared_ptr<Animation> glTFModelExporter::ExportAnimations()
	{
		std::shared_ptr<Animation> animation = std::make_shared<Animation>();

		bvTime = ExportBufferView(AttribType::SCALAR, animTimes.size(), 0);
		bvTranslation = ExportBufferView(AttribType::VEC3, translations.size(), 1);
		bvRotation = ExportBufferView(AttribType::VEC4, rotations.size(), 2);
		bvScale = ExportBufferView(AttribType::VEC3, scales.size(), 3);
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
			{
				Animation::Sampler sampler;
				sampler.interpolation = Interpolation::LINEAR;
				sampler.used = false;
				sampler.input = acTime;
				sampler.output = ExportScaleAccessor(bvScale, offsetData, numDatas);
				Animation::Channel channel;
				channel.target.node = boneIndices[i] + 1;
				channel.target.path = AnimationPath::Scale;
				channel.sampler = animation->samplers.size();
				animation->samplers.push_back(sampler);
				animation->channels.push_back(channel);
			}
			offsetData += numDatas;
		}
		return animation;
	}

	std::shared_ptr<ParaEngine::BufferView> glTFModelExporter::ExportBufferView(AttribType::Value type, uint32_t length, uint32_t index)
	{
		const uint32_t numComponents = AttribType::GetNumComponents(type);
		const uint32_t bytesPerComp = ComponentTypeSize(ComponentType::Float);
		std::shared_ptr<BufferView> bv = std::make_shared<BufferView>();
		bv->buffer = buffer;
		bv->index = bufferIndex + index;
		bv->byteOffset = buffer->byteLength;
		bv->byteLength = length * numComponents * bytesPerComp;
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

	std::shared_ptr<ParaEngine::Accessor> glTFModelExporter::ExportScaleAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas)
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
				float val = scales[i][j];
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
			for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
			{
				const ModelRenderPass& pass = paraXModel->passes[i];
				for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
				{
					uint16_t index = paraXModel->m_indices[j];
					builder.append((const char*)&index, sizeUShort);
				}
				if (pass.indexCount % 2 != 0)
					builder.append("\0\0", sizeUShort);
			}

			uint32_t numBones = paraXModel->m_objNum.nBones;
			for (uint32_t i = 0; i < numBones; i++)
			{
				builder.append((const char*)&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
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
			for (uint32_t i = 0; i < scales.size(); i++)
			{
				builder.append((const char*)&scales[i].x, sizeFloat);
				builder.append((const char*)&scales[i].y, sizeFloat);
				builder.append((const char*)&scales[i].z, sizeFloat);
			}
		}
		else
		{
			for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
			{
				const ModelRenderPass& pass = paraXModel->passes[i];
				for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
				{
					uint16_t index = paraXModel->m_indices[j];
					builder.append((const char*)&index, sizeUShort);
				}
				if (pass.indexCount % 2 != 0)
					builder.append("\0\0", sizeUShort);
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

	void glTFModelExporter::WriteMaterial(std::shared_ptr<Material>& material, Json::Value& mat, Json::Value& tex, Json::Value& sampler, Json::Value& img, uint32_t index)
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
		mat[index] = m;

		shared_ptr<Texture>& texture = pbr.baseColorTexture.texture;
		Json::Value t;
		t["sampler"] = texture->sampler->index;
		t["source"] = texture->source->index;
		tex[index] = t;

		Json::Value s;
		s["magFilter"] = texture->sampler->magFilter;
		s["minFilter"] = texture->sampler->minFilter;
		s["wrapS"] = texture->sampler->wrapS;
		s["wrapT"] = texture->sampler->wrapT;
		sampler[index] = s;

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
		img[index] = i;
	}

	void glTFModelExporter::WriteFile()
	{
		CParaFile file;
		if (file.CreateNewFile(fileName.c_str()))
		{
			Json::FastWriter writer;
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
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						bin.write(&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						bin.write("\0\0", sizeUShort);
				}
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					bin.write(&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
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
				for (uint32_t i = 0; i < scales.size(); i++)
				{
					bin.write(&scales[i].x, sizeFloat);
					bin.write(&scales[i].y, sizeFloat);
					bin.write(&scales[i].z, sizeFloat);
				}
			}
			else
			{
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						bin.write(&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						bin.write("\0\0", sizeUShort);
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
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						file.write(&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						file.write("\0\0", sizeUShort);
				}
				uint32_t numBones = paraXModel->m_objNum.nBones;
				for (uint32_t i = 0; i < numBones; i++)
				{
					file.write(&paraXModel->bones[i].matOffset._m, sizeFloat * 16);
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
				for (uint32_t i = 0; i < scales.size(); i++)
				{
					file.write(&scales[i].x, sizeFloat);
					file.write(&scales[i].y, sizeFloat);
					file.write(&scales[i].z, sizeFloat);
				}
			}
			else
			{
				for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
				{
					const ModelRenderPass& pass = paraXModel->passes[i];
					for (uint32_t j = pass.indexStart; j < pass.indexStart + pass.indexCount; j++)
					{
						uint16_t index = paraXModel->m_indices[j];
						file.write(&index, sizeUShort);
					}
					if (pass.indexCount % 2 != 0)
						file.write("\0\0", sizeUShort);
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
			glTFModelExporter exporter(mesh, binary, embedded);
			exporter.ExportToFile(filename);
			delete mesh;
			mesh = nullptr;
		}
	}

	luabind::object glTFModelExporter::ParaXExportTo_glTF_String(const std::string& input, bool binary, lua_State* L)
	{
		std::string buffer;
		CParaFile file(input.c_str());
		CGlobals::GetAssetManager()->SetAsyncLoading(false);
		CParaXSerializer serializer;
		CParaXModel* mesh = (CParaXModel*)serializer.LoadParaXMesh(file);
		if (mesh != nullptr)
		{
			glTFModelExporter exporter(mesh, binary);
			buffer = exporter.ExportToBuffer();
			delete mesh;
			mesh = nullptr;
		}
		lua_pushlstring(L, buffer.c_str(), buffer.length());
		luabind::object o(luabind::from_stack(L, -1));
		lua_pop(L, 1);
		return o;
	}

}