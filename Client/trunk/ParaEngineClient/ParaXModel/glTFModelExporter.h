#pragma once
#include "json/json.h"
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
		ArrayBuffer = 34962,
		ElementArrayBuffer = 34963
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

	struct Mesh
	{
		struct Primitive
		{
			PrimitiveMode mode;
			struct Attributes
			{
				std::shared_ptr<Accessor> position, normal, texcoord, color;
			} attributes;
			std::shared_ptr<Accessor> indices;
		};
		std::vector<Primitive> primitives;
		uint32_t index;
	};

	struct Node
	{
		std::vector<std::shared_ptr<Mesh> > meshes;
		uint32_t index;
	};

	struct Scene
	{
		std::vector<std::shared_ptr<Node> > nodes;
	};

	class CParaXModel;

	class glTFModelExporter
	{
	public:
		glTFModelExporter(const std::string& filename, CParaXModel* mesh, bool binary);
		~glTFModelExporter();

	private:
		void ExportMetadata();
		void ExportScene();
		std::shared_ptr<Node> ExportNode();
		std::shared_ptr<Mesh> ExportMesh();
		std::shared_ptr<Accessor> ExportVertices();
		std::shared_ptr<Accessor> ExportNormals();
		std::shared_ptr<Accessor> ExportColors();
		std::shared_ptr<Accessor> ExportIndices();
		void WriteBuffer(Json::Value& obj, uint32_t index);
		void WriteBufferView(std::shared_ptr<BufferView>& bufferView, Json::Value& obj, uint32_t index);
		void WriteAccessor(std::shared_ptr<Accessor>& accessor, Json::Value& obj, uint32_t index);
		void WriteFile();
		void WriteGLBFile();

		std::string fileName;
		CParaXModel* paraXModel;
		std::shared_ptr<Buffer> buffer;
		Json::Value root;
		bool isBinary;

	public:
		static void ParaXExportTo_glTF(const std::string& input, const std::string& output, bool binary = false);
	};
}