#pragma once
#include "ParaVector3.h"
#include "ParaQuaternion.h"
#include "ParaXBone.h"
#include "ModelRenderPass.h"
#include "animated.h"
#include "json/json.h"
#include "luabind/object.hpp"
#include <string>
#include <memory>

namespace ParaEngine
{
	struct GLBHeader
	{
		uint32_t magic;
		uint32_t version;
		uint32_t length;
	};

	struct GLBChunk
	{
		uint32_t chunkLength;
		uint32_t chunkType;
	};

	enum ChunkType
	{
		JSON = 0x4E4F534A,
		BIN = 0x004E4942
	};

	enum BufferViewTarget
	{
		NotUse = 0,
		ArrayBuffer = 34962,
		ElementArrayBuffer = 34963
	};

	enum SamplerMagFilter
	{
		MagNearest = 9728,
		MagLinear = 9729
	};

	enum SamplerMinFilter
	{
		MinNearest = 9728,
		MinLinear = 9729,
		NearestMipMapNearest = 9984,
		LinearMipMapNearest = 9985,
		NearestMipMapLinear = 9986,
		LinearMipMapLinear = 9987
	};

	enum SamplerWrap
	{
		ClampToEdge = 33071,
		MirroredRepeat = 33648,
		Repeat = 10497
	};

	enum PrimitiveMode
	{
		Points, Lines, LineLoop, LineStrip, Triangles, Triangle_Strip, Triangle_Fan
	};

	enum ComponentType
	{
		Byte = 5120, UnsignedByte, Short, UnsignedShort, UnsignedInt = 5125, Float
	};

	inline uint32_t ComponentTypeSize(ComponentType type)
	{
		switch (type)
		{
		case ParaEngine::Byte:
		case ParaEngine::UnsignedByte:
			return 1;
		case ParaEngine::Short:
		case ParaEngine::UnsignedShort:
			return 2;
		case ParaEngine::UnsignedInt:
		case ParaEngine::Float:
			return 4;
		default: return 0;
		}
	}

	enum AnimationPath
	{
		Translation, Rotation, Scale
	};

	enum Interpolation
	{
		LINEAR, STEP, CUBICSPLINE
	};

	class AttribType
	{
	public:
		enum Value
		{
			SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4
		};

	private:
		static const size_t NUM_VALUES = static_cast<size_t>(MAT4) + 1;

		struct Info
		{
			const char* name; unsigned int numComponents;
		};

		template<int N> struct data
		{
			static const Info infos[NUM_VALUES];
		};

	public:
		inline static Value FromString(const char* str)
		{
			for (size_t i = 0; i < NUM_VALUES; ++i) {
				if (strcmp(data<0>::infos[i].name, str) == 0) {
					return static_cast<Value>(i);
				}
			}
			return SCALAR;
		}

		inline static const char* ToString(Value type)
		{
			return data<0>::infos[static_cast<size_t>(type)].name;
		}

		inline static unsigned int GetNumComponents(Value type)
		{
			return data<0>::infos[static_cast<size_t>(type)].numComponents;
		}
	};

	template<int N> const AttribType::Info
		AttribType::data<N>::infos[AttribType::NUM_VALUES] = {
			{ "SCALAR", 1 },{ "VEC2", 2 },{ "VEC3", 3 },{ "VEC4", 4 },{ "MAT2", 4 },{ "MAT3", 9 },{ "MAT4", 16 }
	};

	struct Buffer
	{
	public:
		Buffer() : byteLength(0), capacity(0), index(0) {}
		void Grow(uint32_t count)
		{
			byteLength += count;
		}
		uint32_t byteLength;
		uint32_t capacity;
		uint32_t index;
		std::string filename;
		std::string uri;
	};

	struct BufferView
	{
		std::shared_ptr<Buffer> buffer;
		uint32_t index;
		uint32_t byteOffset;
		uint32_t byteLength;
		uint32_t byteStride;
		BufferViewTarget target;
	};

	struct Accessor
	{
		std::shared_ptr<BufferView> bufferView;
		uint32_t index;
		uint32_t byteOffset;
		ComponentType componentType;
		uint32_t count;
		AttribType::Value type;
		std::vector<float> max;
		std::vector<float> min;
	};

	struct Sampler
	{
		SamplerMagFilter magFilter;
		SamplerMinFilter minFilter;
		SamplerWrap wrapS;
		SamplerWrap wrapT;
		uint32_t index;
	};

	struct Image
	{
		std::string filename;
		std::string uri;
		std::shared_ptr<uint8_t> bufferPointer;
		uint32_t bufferSize;
		uint32_t index;
	};

	struct Texture
	{
		std::shared_ptr<Sampler> sampler;
		std::shared_ptr<Image> source;
	};

	struct TextureInfo
	{
		std::shared_ptr<Texture> texture;
		uint32_t index;
		uint32_t texCoord;
	};

	struct PbrMetallicRoughness
	{
		float baseColorFactor[4];
		float metallicFactor;
		float roughnessFactor;
		TextureInfo baseColorTexture;
		TextureInfo metallicRoughnessTexture;
	};

	struct Material
	{
		PbrMetallicRoughness metallicRoughness;
		TextureInfo normalTexture;
		TextureInfo occlusionTexture;
		TextureInfo emissiveTexture;
		float emissiveFactor[3];
		float alphaCutoff;
		bool doubleSide;
		std::string alphaMode;
		uint32_t index;
	};

	struct Mesh
	{
		struct Primitive
		{
			PrimitiveMode mode;
			struct Attributes
			{
				std::shared_ptr<Accessor> position, normal, texcoord, color0, color1, joints, weights;
			} attributes;
			std::shared_ptr<Accessor> indices;
			std::shared_ptr<Material> material;
		};
		std::vector<Primitive> primitives;
		uint32_t index;
	};

	struct Node;
	struct Skin
	{
		std::shared_ptr<Accessor> inverseBindMatrices;
		std::vector <std::shared_ptr<Node> > joints;
		uint32_t index;
	};

	struct Animation
	{
		struct Sampler
		{
			bool used;
			std::shared_ptr<Accessor> input;
			std::shared_ptr<Accessor> output;
			Interpolation interpolation;
		};

		struct Target
		{
			uint32_t node;
			AnimationPath path;
		};

		struct Channel
		{
			uint32_t sampler;
			Target target;
		};

		std::vector<Sampler> samplers;
		std::vector<Channel> channels;
	};

	struct Node
	{
		std::shared_ptr<Mesh> mesh;
		std::vector<uint32_t> children;
		Vector3 translation;
		Quaternion rotation;
		Vector3 scale;
		std::shared_ptr<Skin> skin;
		uint32_t index;
	};

	class CParaXModel;

	class glTFModelExporter
	{
	public:
		glTFModelExporter(CParaXModel* mesh, bool binary);
		glTFModelExporter(CParaXModel* mesh, CParaXModel* anim, bool binary);
		~glTFModelExporter();

		void ExportToFile(const std::string& filename);
		std::string ExportToBuffer();

	private:
		void ParseParaXModel();
		void ParseAnimationBones();
		void ChangeAnimationBones();
		Bone* GetBoneByIndex(int nIndex);
		Bone* GetBoneByID(KNOWN_BONE_NODES boneID);
		void CalculateJoint(int id, const Vector3& pivot);
		void ExportMetadata();
		void ExportScene();
		std::shared_ptr<Node> ExportNode();
		std::shared_ptr<Mesh> ExportMesh();
		std::shared_ptr<Skin> ExportSkin();
		std::shared_ptr<Accessor> ExportMatrices();
		std::shared_ptr<Accessor> ExportVertices();
		std::shared_ptr<Accessor> ExportNormals();
		std::shared_ptr<Accessor> ExportTextureCoords();
		std::shared_ptr<Accessor> ExportColors0();
		std::shared_ptr<Accessor> ExportColors1();
		std::shared_ptr<Accessor> ExportJoints();
		std::shared_ptr<Accessor> ExportWeights();
		std::shared_ptr<Accessor> ExportIndices(ModelRenderPass& pass);
		std::shared_ptr<Material> ExportMaterials(int tex, int index, bool cull);
		std::shared_ptr<Animation> ExportAnimations(uint32_t animId);
		std::shared_ptr<BufferView> ExportBufferView(AttribType::Value type, uint32_t length, uint32_t index);
		std::shared_ptr<Accessor> ExportTimeAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId);
		std::shared_ptr<Accessor> ExportTranslationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId);
		std::shared_ptr<Accessor> ExportRotationAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId);
		std::shared_ptr<Accessor> ExportScaleAccessor(std::shared_ptr<BufferView>& bv, uint32_t offset, uint32_t numDatas, uint32_t animId);
		std::string EncodeBuffer();
		void WriteBuffer(Json::Value& obj, uint32_t index);
		void WriteBufferView(std::shared_ptr<BufferView>& bufferView, Json::Value& obj, uint32_t index);
		void WriteAccessor(std::shared_ptr<Accessor>& accessor, Json::Value& obj, uint32_t index);
		void WriteMaterial(std::shared_ptr<Material>& material, Json::Value& mat, Json::Value& tex, Json::Value& sampler, Json::Value& img, uint32_t index);
		void WriteFile();
		void WriteRawData();
		void WriteGLBFile();

		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector3> colors0;
		std::vector<Vector3> colors1;
		std::vector<Vector2> texcoords;
		std::vector<Vector4> weights;
		Vector3 maxVertex;
		Vector3 minVertex;
		Vector3 maxNormal;
		Vector3 minNormal;
		Vector3 maxColor0;
		Vector3 minColor0;
		Vector3 maxColor1;
		Vector3 minColor1;
		Vector2 maxCoord;
		Vector2 minCoord;
		Vector4 maxJoint;
		Vector4 minJoint;
		Vector4 maxWeight;
		Vector4 minWeight;
		std::vector<std::vector<int>> boneIndices;
		std::vector<std::vector<uint32_t>> animOffsets;
		std::vector<std::vector<float>> animTimes;
		std::vector<std::vector<Vector3>> translations;
		std::vector<std::vector<Vector3>> scales;
		std::vector<std::vector<Quaternion>> rotations;

		CParaXModel* paraXModel;
		CParaXModel* animProvider;
		std::string fileName;
		Json::Value root;
		uint32_t bufferIndex;
		uint32_t accessorIndex;
		bool isBinary;
		std::shared_ptr<Buffer> buffer;
		std::vector<std::shared_ptr<BufferView>> bvTime;
		std::vector<std::shared_ptr<BufferView>> bvTranslation;
		std::vector<std::shared_ptr<BufferView>> bvRotation;
		std::vector<std::shared_ptr<BufferView>> bvScale;

	public:
		//static luabind::object GetParaXTexturesFromFile(const std::string& input, lua_State* L);
		//static luabind::object GetParaXTexturesFromBuffer(const char* buffer, int size, lua_State* L);
		//static void ParaXFileExportTo_glTF_File(const std::string& input, const std::string& output, const luabind::object& texObject, bool binary, bool embedded = true);
		//static void ParaXBufferExportTo_glTF_File(const char* buffer, int size, const std::string& output, const luabind::object& texObject, bool binary, bool embedded = true);
		//static luabind::object ParaXFileExportTo_glTF_String(const std::string& input, bool binary, const luabind::object& texObject, lua_State* L);
		//static luabind::object ParaXBufferExportTo_glTF_String(const char* buffer, int size, bool binary, const luabind::object& texObject, lua_State* L);
		//static luabind::object ParaXFileChangeAnimation(
		//	const std::string& paraXFile, const std::string& animFile, bool binary, const luabind::object& texObject, lua_State* L);
		//static luabind::object ParaXBufferChangeAnimation(
		//	const char* paraXBuffer, int paraXSize, const char* animBuffer, int animSize, bool binary, const luabind::object& texObject, lua_State* L);
		static void ParaXExportTo_glTF(CParaXModel* parax, const char* output);
		static void BlocksExportTo_glTF(const char* blocks, const char* output);
	};
}
