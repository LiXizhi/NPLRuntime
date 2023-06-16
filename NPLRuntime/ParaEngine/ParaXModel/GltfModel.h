
#pragma once
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif

#include "ParaVector3.h"
#include "ParaQuaternion.h"
#include "ParaXBone.h"
#include "ModelRenderPass.h"
#include "animated.h"
#include "json/json.h"
#include "luabind/object.hpp"
#include "StringBuilder.h"
#include <string>
#include <memory>
namespace ParaEngine {
	struct GLTFHeader
	{
		uint32_t magic;
		uint32_t version;
		uint32_t length;
	};

	struct GLTFChunk
	{
		uint32_t chunkLength;
		uint32_t chunkType;
	};

	enum GLTFChunkType
	{
		GLTF_JSON = 0x4E4F534A,
		GLTF_BIN = 0x004E4942
	};

	enum GLTFBufferViewTarget
	{
		GLTF_NotUse = 0,
		GLTF_ArrayBuffer = 34962,
		GLTF_ElementArrayBuffer = 34963
	};

	enum GLTFSamplerMagFilter
	{
		GLTF_MagNearest = 9728,
		GLTF_MagLinear = 9729
	};

	enum GLTFSamplerMinFilter
	{
		GLTF_MinNearest = 9728,
		GLTF_MinLinear = 9729,
		GLTF_NearestMipMapNearest = 9984,
		GLTF_LinearMipMapNearest = 9985,
		GLTF_NearestMipMapLinear = 9986,
		GLTF_LinearMipMapLinear = 9987
	};

	enum GLTFSamplerWrap
	{
		GLTF_ClampToEdge = 33071,
		GLTF_MirroredRepeat = 33648,
		GLTF_Repeat = 10497
	};

	enum GLTFPrimitiveMode
	{
		GLTF_Points, GLTF_Lines, GLTF_LineLoop, GLTF_LineStrip, GLTF_Triangles, GLTF_Triangle_Strip, GLTF_Triangle_Fan
	};

	enum GLTFComponentType
	{
		GLTF_Byte = 5120, GLTF_UnsignedByte, GLTF_Short, GLTF_UnsignedShort, GLTF_UnsignedInt = 5125, GLTF_Float
	};

	enum GLTFType 
	{
		GLTF_SCALAR, GLTF_VEC2, GLTF_VEC3, GLTF_VEC4, GLTF_MAT2, GLTF_MAT3, GLTF_MAT4
	};

	enum GLTFAnimationPath
	{
		GLTF_Translation, GLTF_Rotation, GLTF_Scale
	};

	enum GLTFInterpolation
	{
		GLTF_LINEAR, GLTF_STEP, GLTF_CUBICSPLINE
	};

	struct GLTFBuffer
	{
	public:
		GLTFBuffer() : byteLength(0), capacity(0), index(0) {}
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

	struct GLTFBufferView
	{
		std::shared_ptr<GLTFBuffer> buffer;
		uint32_t index;
		uint32_t byteOffset;
		uint32_t byteLength;
		uint32_t byteStride;
		GLTFBufferViewTarget target;

		uint32_t byteFill;   // 数据对齐

		const char* data;
		std::vector<float> dataf;
		std::vector<unsigned int> dataui;
		std::vector<unsigned char> datauc;
	};

	struct GLTFAccessor
	{
		std::shared_ptr<GLTFBufferView> bufferView;
		uint32_t index;
		uint32_t byteOffset;
		GLTFComponentType componentType;
		uint32_t count;
		GLTFType type;
		std::vector<float> max;
		std::vector<float> min;
	};

	struct GLTFSampler
	{
		GLTFSamplerMagFilter magFilter;
		GLTFSamplerMinFilter minFilter;
		GLTFSamplerWrap wrapS;
		GLTFSamplerWrap wrapT;
		uint32_t index;
	};

	struct GLTFImage
	{
		std::string filename;
		std::string uri;
		std::shared_ptr<uint8_t> bufferPointer;
		uint32_t bufferSize;
		uint32_t index;
	};

	struct GLTFTexture
	{
		std::shared_ptr<GLTFSampler> sampler;
		std::shared_ptr<GLTFImage> source;
		uint32_t index;
	};

	struct GLTFTextureInfo
	{
		std::shared_ptr<GLTFTexture> texture;
		uint32_t index;
		uint32_t texCoord;
	};

	struct GLTFPbrMetallicRoughness
	{
		float baseColorFactor[4];
		float metallicFactor;
		float roughnessFactor;
		GLTFTextureInfo baseColorTexture;
		GLTFTextureInfo metallicRoughnessTexture;
	};

	struct GLTFMaterial
	{
		GLTFPbrMetallicRoughness metallicRoughness;
		GLTFTextureInfo normalTexture;
		GLTFTextureInfo occlusionTexture;
		GLTFTextureInfo emissiveTexture;
		float emissiveFactor[3];
		float alphaCutoff;
		bool doubleSide;
		std::string alphaMode;
		uint32_t index;
	};

	struct GLTFPrimitive
	{
		GLTFPrimitiveMode mode;
		std::shared_ptr<GLTFAccessor> indices;
		std::shared_ptr<GLTFMaterial> material;
		struct GLTFAttributes
		{
			std::shared_ptr<GLTFAccessor> position, normal, texcoord, color0, color1, joints, weights;
		} attributes;
	};

	struct GLTFMesh
	{
		std::vector<std::shared_ptr<GLTFPrimitive>> primitives;
		uint32_t index;
		uint16_t* indices;
	};

	struct GLTFNode;
	struct GLTFSkin
	{
		std::shared_ptr<GLTFAccessor> inverseBindMatrices;
		std::vector <std::shared_ptr<GLTFNode>> joints;
		uint32_t index;
	};

	struct GLTFAnimation
	{
		struct GLTFSampler
		{
			bool used;
			std::shared_ptr<GLTFAccessor> input;
			std::shared_ptr<GLTFAccessor> output;
			GLTFInterpolation interpolation;
		};

		struct GLTFTarget
		{
			uint32_t node;
			GLTFAnimationPath path;
		};

		struct GLTFChannel
		{
			uint32_t sampler;
			GLTFTarget target;
		};

		std::vector<GLTFSampler> samplers;
		std::vector<GLTFChannel> channels;

		uint32_t index;
	};

	struct GLTFNode
	{
		std::shared_ptr<GLTFMesh> mesh;
		std::vector<uint32_t> children;
		std::shared_ptr<Vector3> translation;
		std::shared_ptr<Quaternion> rotation;
		std::shared_ptr<Vector3> scale;
		std::shared_ptr<GLTFSkin> skin;
		std::shared_ptr<Matrix4> matrix;
		uint32_t index;
		int32_t boneIndex;  // 骨骼索引
	};

	struct GLTFScene 
	{
		std::vector<std::shared_ptr<GLTFNode>> nodes;
		uint32_t index;
	};

	struct GLTF {
		std::vector<std::shared_ptr<GLTFNode>> nodes;
		std::vector<std::shared_ptr<GLTFMesh>> meshs;
		std::vector<std::shared_ptr<GLTFScene>> scenes;
		std::vector<std::shared_ptr<GLTFBufferView>> bufferViews;
		std::vector<std::shared_ptr<GLTFBuffer>> buffers;
		std::vector<std::shared_ptr<GLTFAccessor>> accessors;
		std::vector<std::shared_ptr<GLTFMaterial>> materials;
		std::vector<std::shared_ptr<GLTFTexture>> textures;
		std::vector<std::shared_ptr<GLTFSampler>> samplers;
		std::vector<std::shared_ptr<GLTFImage>> images;
		std::vector<std::shared_ptr<GLTFAnimation>> animations;
		std::vector<std::shared_ptr<GLTFSkin>> skins;
	};

	class CParaXModel;
	class CharModelInstance;
	class CParaXStaticModel;

    class GltfModel
	{
	public:
		GltfModel();
		~GltfModel();

		void Init();
		// void Export(CParaXStaticModel* paraXStaticModel, std::string filepath);
		void Export(CParaXModel* paraXModel, std::string filepath);
		void Export(const char* blocks, std::string filepath);
		void Export(CharModelInstance* model, std::string filepath);
		static void ExportBlocks(const char* blocks, const char* filepath);
		static void ExportParaXModel(CParaXModel* model, const char* filepath);
		static void ExportCharModel(CharModelInstance* model, const char* filepath);

	public:
    	std::shared_ptr<GLTFNode> ExportSceneNode(CParaXModel* paraXModel, std::shared_ptr<GLTFScene> scene);
    	std::shared_ptr<GLTFNode> ExportSceneNode(CParaXStaticModel* paraXStaticModel, std::shared_ptr<GLTFScene> scene);

	protected:
		bool IsExportAnimation(CParaXModel* paraXModel); 
		uint32_t GetAnimationCount(CParaXModel* paraXModel);

		std::shared_ptr<GLTFScene> ExportScene();
		std::shared_ptr<GLTFNode> ExportNode();
		std::shared_ptr<GLTFMesh> ExportMesh(CParaXModel* paraXModel);
	    std::shared_ptr<GLTFPrimitive> ExportPrimitive(CParaXModel* paraXModel, ModelRenderPass& pass);
		std::shared_ptr<GLTFMaterial> ExportMaterial(CParaXModel* paraXModel, ModelRenderPass& pass);
    	std::shared_ptr<GLTFAccessor> ExportIndices(CParaXModel* paraXModel, ModelRenderPass& pass);
    	std::shared_ptr<GLTFImage> ExportImage(CParaXModel* paraXModel, ModelRenderPass& pass); 
    	std::shared_ptr<GLTFImage> ExportImage(asset_ptr<TextureEntity> texture);
    	std::shared_ptr<GLTFSampler> ExportSampler(DWORD headerType = PARAX_MODEL_ANIMATED);
		std::shared_ptr<GLTFAnimation> ExportAnimation(CParaXModel* paraXModel, uint32_t animId, std::shared_ptr<GLTFSkin> skin);
		std::shared_ptr<GLTFSkin> ExportSkin(CParaXModel* paraXModel, std::shared_ptr<GLTFNode> parentNode);
		std::shared_ptr<GLTFAccessor> ExportMatrices(CParaXModel* paraXModel);

    	std::shared_ptr<GLTFAccessor> ExportVertices(CParaXModel* paraXModel);
    	std::shared_ptr<GLTFAccessor> ExportNormals(CParaXModel* paraXModel);
	    std::shared_ptr<GLTFAccessor> ExportTextureCoords(CParaXModel* paraXModel);
    	std::shared_ptr<GLTFAccessor> ExportColors0(CParaXModel* paraXModel);
    	std::shared_ptr<GLTFAccessor> ExportColors1(CParaXModel* paraXModel);
    	std::shared_ptr<GLTFAccessor> ExportJoints(CParaXModel* paraXModel);
  	  	std::shared_ptr<GLTFAccessor> ExportWeights(CParaXModel* paraXModel);
		std::shared_ptr<GLTFAccessor> ExportMeshPrimitive(CParaXModel* paraXModel, ModelRenderPass& pass);
		std::shared_ptr<GLTFAccessor> ExportAccessor(uint32_t size, GLTFType type, GLTFComponentType compType, GLTFBufferViewTarget target = GLTFBufferViewTarget::GLTF_ArrayBuffer);
		std::shared_ptr<GLTFBufferView> ExportBufferView(uint32_t size, GLTFType type, GLTFComponentType compType, GLTFBufferViewTarget target = GLTFBufferViewTarget::GLTF_NotUse);
	
    	Json::Value& ExportJson();
		Json::Value ExportJsonNode(std::shared_ptr<GLTFNode>& node);
		Json::Value ExportJsonMesh(std::shared_ptr<GLTFMesh>& mesh);
    	Json::Value ExportJsonBufferView(std::shared_ptr<GLTFBufferView>& bufferView);
    	Json::Value ExportJsonAccessor(std::shared_ptr<GLTFAccessor>& accessor);
		Json::Value ExportJsonAnimation(std::shared_ptr<GLTFAnimation>& animation);
    	Json::Value ExportJsonMaterial(std::shared_ptr<GLTFMaterial>& material);
	
		void WriteFile(std::string filename);
	public:
		std::shared_ptr<GLTFBuffer> buffer;
		StringBuilder m_string_builder;
		std::shared_ptr<GLTF> m_gltf;
		Json::Value m_json_gltf;
		std::string m_filename;
		bool m_enable_animation;
		uint32_t bufferIndex;
		uint32_t bufferViewIndex;
		uint32_t sceneIndex;
		uint32_t accessorIndex;
		uint32_t nodeIndex;
		uint32_t meshIndex;
		uint32_t skinIndex;
		uint32_t animationIndex;
		uint32_t materialIndex;
		uint32_t imageIndex;
		uint32_t samplerIndex;
		uint32_t textureIndex;
	};
}