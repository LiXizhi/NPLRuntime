#include "ParaEngine.h"
#include "BMaxModel/BlocksParser.h"
#include "ParaXModel.h"
#include "ParaXSerializer.h"
#include "StringHelper.h"
#include "StringBuilder.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"
#include "CustomCharModelInstance.h"
#include "ParaXModelCanvas.h"
#include "GltfModel.h"
#include "MeshObject.h"

namespace ParaEngine
{

    inline uint32_t GLTFComponentTypeSize(GLTFComponentType type)
	{
		switch (type)
		{
		case GLTFComponentType::GLTF_Byte:
		case GLTFComponentType::GLTF_UnsignedByte:
			return 1;
		case GLTFComponentType::GLTF_Short:
		case GLTFComponentType::GLTF_UnsignedShort:
			return 2;
		case GLTFComponentType::GLTF_UnsignedInt:
		case GLTFComponentType::GLTF_Float:
			return 4;
		default: return 0;
		}
	}

    inline uint32_t GLTFGetComponentTypeCount(GLTFType type) 
    {
        if (type == GLTFType::GLTF_SCALAR) return 1;
        if (type == GLTFType::GLTF_VEC2) return 2;
        if (type == GLTFType::GLTF_VEC3) return 3;
        if (type == GLTFType::GLTF_VEC4) return 4;
        if (type == GLTFType::GLTF_MAT2) return 4;
        if (type == GLTFType::GLTF_MAT3) return 9;
        if (type == GLTFType::GLTF_MAT4) return 16;
        return 0;
    }

    inline const char* GLTFGetTypeString(GLTFType type) 
    {
        if (type == GLTFType::GLTF_SCALAR) return "SCALAR";
        if (type == GLTFType::GLTF_VEC2) return "VEC2";
        if (type == GLTFType::GLTF_VEC3) return "VEC3";
        if (type == GLTFType::GLTF_VEC4) return "VEC4";
        if (type == GLTFType::GLTF_MAT2) return "MAT2";
        if (type == GLTFType::GLTF_MAT3) return "MAT3";
        if (type == GLTFType::GLTF_MAT4) return "MAT4";
        return "";
    }

    GltfModel::GltfModel() : m_isEmbedTexture(true), m_enable_animation(true)
	{

	}	

	GltfModel::~GltfModel() 
	{

	}

	bool GltfModel::IsExportAnimation(CParaXModel* paraXModel)
	{ 
		return m_enable_animation && paraXModel->animated; 
	}

	uint32_t GltfModel::GetAnimationCount(CParaXModel* paraXModel) 
	{
        // uint32_t maxAnimCount = 60;
		// return paraXModel->m_objNum.nAnimations > 1 ? (paraXModel->m_objNum.nAnimations <= maxAnimCount ? paraXModel->m_objNum.nAnimations : maxAnimCount): 0;
		return paraXModel->m_objNum.nAnimations;	
	}

	void GltfModel::Init()
	{
        sceneIndex = 0;
		bufferIndex = 0;
		bufferViewIndex = 0;
		accessorIndex = 0;
		nodeIndex = 0;
		meshIndex = 0;
		skinIndex = 0;
		animationIndex = 0;
        materialIndex = 0;
        imageIndex = 0;
        samplerIndex = 0;
        textureIndex = 0;

        m_json_gltf = Json::Value();
        m_string_builder.clear();
        m_string_builder.reserve(1024 * 1024 * 5); // 5M
        m_gltf = std::make_shared<GLTF>();

		buffer = std::make_shared<GLTFBuffer>();
        buffer->index = bufferIndex++;
        m_gltf->buffers.push_back(buffer);

        m_enable_animation = true;
        m_isEmbedTexture = true;
	}

    void GltfModel::Export(CParaXModel* paraXModel, std::string filepath) 
	{
        m_filename = filepath == "" ? ".temp.gltf" : filepath;
        Init();
        std::shared_ptr<GLTFScene> scene = ExportScene();
        ExportSceneNode(paraXModel, scene);
        ExportJson();
        if (filepath != "") 
        {
            WriteFile(filepath);
        } 
    }

    std::shared_ptr<GLTFNode> ExportCanvasAttachment(CanvasAttachmentPtr canvas, std::shared_ptr<GLTFScene> scene, GltfModel* gltf)  
    {
        std::shared_ptr<GLTFNode> node = nullptr;
        // ATT_ID_HAND_RIGHT 忽略手持物
        if (canvas->id == ATT_ID_HAND_RIGHT) return node;

        if (canvas->model) 
        {
            node = gltf->ExportSceneNode(canvas->model->GetModel(), scene);
        }

        if (canvas->m_pMeshObject) {
            CParaXStaticBase* model = canvas->m_pMeshObject->GetModel();
            if (model->GetAttributeClassID() == ATTRIBUTE_CLASSID_CParaXStaticModel) 
            {
                node = gltf->ExportSceneNode((CParaXStaticModel*)model, scene);
            }
        }

        Matrix4 boneMat;
        boneMat.makeTrans(0, 0, 0);
        if (node && canvas->parent && canvas->parent->model)  
        {
            CParaXModel* pModel = canvas->parent->model->GetModel();
            int id = canvas->id;
            int nAttachmentIndex = pModel->m_attLookup[id];
            if (nAttachmentIndex >= 0) {
                ModelAttachment& att = pModel->m_atts[nAttachmentIndex];
                Matrix4 mat, mat1;
	            mat1 = pModel->bones[att.bone].mat;
    	        mat.makeTrans(att.pos.x, att.pos.y, att.pos.z);
    	        ParaMatrixMultiply(&mat1, &mat, &mat1);
                node->matrix = std::make_shared<Matrix4>();
                boneMat = mat1;
                node->boneIndex = att.bone;
            }
        }
        Matrix4 nodemat, matTmp;
        if (canvas->scale != 1.f) 
        {
            ParaMatrixScaling(&matTmp, canvas->scale, canvas->scale, canvas->scale);
		    nodemat = matTmp * boneMat;
        }
        else 
        {
            nodemat = boneMat;
        }
	    Vector3 vOffset = canvas->m_vOffset;
        if (canvas->m_pMeshObject) vOffset += canvas->m_pMeshObject->GetPosition(); 
        if (vOffset.x != 0 || vOffset.y != 0 || vOffset.z != 0)
	    {
		    matTmp.makeTrans(vOffset.x, vOffset.y, vOffset.z);
	    	nodemat = matTmp * nodemat;
	    }

        if (node) {
            node->matrix = std::make_shared<Matrix4>();
            *(node->matrix) = nodemat;
        } 
        for (unsigned int i = 0; i < canvas->children.size(); i++) 
        {
            std::shared_ptr<GLTFNode> subnode = ExportCanvasAttachment(canvas->children[i], scene, gltf);
        }

        return node;
    }

    void GltfModel::Export(CharModelInstance* model, std::string filepath)  
    {
        m_filename = filepath == "" ? ".temp.gltf" : filepath;
        CanvasAttachmentPtr canvas = model->m_pModelCanvas->m_root;
        Init();
        m_enable_animation = false;
        std::shared_ptr<GLTFScene> scene = ExportScene();
        std::shared_ptr<GLTFNode> node = ExportCanvasAttachment(canvas, scene, this);
        // 人物会倒, 此变换可摆正, 但与装扮对不上
        // Quaternion q;
        // q.FromAngleAxis(Radian(Degree(90)), Vector3(1,0,0));
        // *(node->matrix) = (*(node->matrix)).Multiply4x3(Matrix4(q.invertWinding()));

        ExportJson();
        if (filepath != "") 
        {
            WriteFile(filepath);
        } 
    }
    
	std::shared_ptr<GLTFBufferView> GltfModel::ExportBufferView(uint32_t size, GLTFType type, GLTFComponentType compType, GLTFBufferViewTarget target)
    {
        const uint32_t compCount = GLTFGetComponentTypeCount(type);
		const uint32_t compSize = GLTFComponentTypeSize(compType);

		std::shared_ptr<GLTFBufferView> bv = std::make_shared<GLTFBufferView>();
        const uint32_t tmp = buffer->byteLength % compSize;
		bv->buffer = buffer;
		bv->index = bufferViewIndex;
        bv->byteFill = tmp == 0 ? 0 : (compSize - tmp);
		bv->byteOffset = buffer->byteLength + bv->byteFill;
		bv->byteLength = size * compCount * compSize;
		bv->byteStride = compCount * compSize;
		bv->target = target;
		buffer->Grow(bv->byteLength);
        bufferViewIndex++;
        m_gltf->bufferViews.push_back(bv);
        return bv;
    }

	std::shared_ptr<GLTFAccessor> GltfModel::ExportAccessor(uint32_t size, GLTFType type, GLTFComponentType compType, GLTFBufferViewTarget target)
    {
        const uint32_t compCount = GLTFGetComponentTypeCount(type);
        std::shared_ptr<GLTFAccessor> acc = std::make_shared<GLTFAccessor>();
		acc->bufferView = ExportBufferView(size, type, compType, target);
		acc->index = accessorIndex;
		acc->byteOffset = 0;
		acc->componentType = compType;
		acc->count = size;
		acc->type = type;
        acc->max.resize(compCount, -FLT_MAX);
        acc->min.resize(compCount, FLT_MAX);
		accessorIndex++;
        m_gltf->accessors.push_back(acc);
		return acc;
    }

    std::shared_ptr<GLTFAccessor> GltfModel::ExportVertices(CParaXModel* paraXModel)
	{
       	uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
        for (uint32_t i = 0; i < size; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
            Vector3 pos = vertex.pos;
            if (paraXModel->m_RenderMethod == CParaXModel::SOFT_ANIM && !IsExportAnimation(paraXModel)) 
            {
                ModelVertex* ov = &vertex;
                float weight = ov->weights[0] * (1 / 255.0f);
                Bone& bone = paraXModel->bones[ov->bones[0]];
                Vector3 v = (ov->pos * bone.mat)*weight;
                for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
                    weight = ov->weights[b] * (1 / 255.0f);
                    Bone& bone = paraXModel->bones[ov->bones[b]];
                    v += (ov->pos * bone.mat) * weight;
                }
                pos = v;
            }
			for (uint32_t j = 0; j < compCount; j++)
			{
				float val = pos[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
    			acc->bufferView->dataf.push_back(val);
			}
        }
        for (uint32_t i = 0; i < size; i++)
        {
            for (uint32_t j = 0; j < compCount; j++)
			{
    			acc->bufferView->dataf[i * compCount + j] -= acc->min[j];
			}
        }
        for (uint32_t j = 0; j < compCount; j++)
        {
            acc->max[j] -= acc->min[j];
            acc->min[j] -= acc->min[j];
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportNormals(CParaXModel* paraXModel)
	{
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
        for (uint32_t i = 0; i < size; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
            Vector3 normal = vertex.normal;
            if (paraXModel->m_RenderMethod == CParaXModel::SOFT_ANIM && !IsExportAnimation(paraXModel)) 
            {
                ModelVertex* ov = &vertex;
                float weight = ov->weights[0] * (1 / 255.0f);
                Bone& bone = paraXModel->bones[ov->bones[0]];
                Vector3 n = ov->normal.TransformNormal(bone.mrot) * weight;
                for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
                    weight = ov->weights[b] * (1 / 255.0f);
                    Bone& bone = paraXModel->bones[ov->bones[b]];
                    n += ov->normal.TransformNormal(bone.mrot) * weight;
                }
                normal = n;
            }
			for (uint32_t j = 0; j < compCount; j++)
			{
				float val = normal[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
    			acc->bufferView->dataf.push_back(val);
			}
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportTextureCoords(CParaXModel* paraXModel)
	{
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC2, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
        for (uint32_t i = 0; i < size; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			for (uint32_t j = 0; j < compCount; j++)
			{
				float val = vertex.texcoords[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
    			acc->bufferView->dataf.push_back(val);
			}
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportColors0(CParaXModel* paraXModel)
	{
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
		for (uint32_t i = 0; i < size; i++)
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
			acc->bufferView->dataf.push_back(r);
			acc->bufferView->dataf.push_back(g);
			acc->bufferView->dataf.push_back(b);
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportColors1(CParaXModel* paraXModel)
	{
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
		for (uint32_t i = 0; i < size; i++)
		{
			DWORD color = paraXModel->m_origVertices[i].color1;
            float r = ((color >> 16) & 0xff) / 255.0f;
            float g = ((color >> 8) & 0xff) / 255.0f;
            float b = (color & 0xff) / 255.0f;
            if (r < acc->min[0]) acc->min[0] = r;
            if (g < acc->min[1]) acc->min[1] = g;
            if (b < acc->min[2]) acc->min[2] = b;
            if (r > acc->max[0]) acc->max[0] = r;
            if (g > acc->max[1]) acc->max[1] = g;
            if (b > acc->max[2]) acc->max[2] = b;
		    acc->bufferView->dataf.push_back(r);
			acc->bufferView->dataf.push_back(g);
			acc->bufferView->dataf.push_back(b);	
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportJoints(CParaXModel* paraXModel)
	{
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC4, GLTFComponentType::GLTF_UnsignedByte);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
        acc->max.resize(compCount, 0);
        acc->min.resize(compCount, 255);
        for (uint32_t i = 0; i < size; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			for (uint32_t j = 0; j < compCount; j++)
			{
				unsigned char val = vertex.bones[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
    			acc->bufferView->datauc.push_back(val);
			}
        }
        acc->bufferView->data = (const char*)acc->bufferView->datauc.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportWeights(CParaXModel* paraXModel)
	{
		float inverse = 1.0f / 255.0f;
        uint32_t size = paraXModel->m_objNum.nVertices;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(size, GLTFType::GLTF_VEC4, GLTFComponentType::GLTF_Float);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
        for (uint32_t i = 0; i < size; i++)
		{
			ModelVertex& vertex = paraXModel->m_origVertices[i];
			for (uint32_t j = 0; j < compCount; j++)
			{
				float val = vertex.weights[j] * inverse;
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
    			acc->bufferView->dataf.push_back(val);
			}
        }
        acc->bufferView->data = (const char*)acc->bufferView->dataf.data();
        return acc;
	}

    std::shared_ptr<GLTFAccessor> GltfModel::ExportIndices(CParaXModel* paraXModel, ModelRenderPass& pass)
    {
    	uint32_t vertexOffset = pass.GetVertexStart(paraXModel);
        uint32_t indexCount = pass.indexCount;
        uint32_t indexStart = pass.m_nIndexStart;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(indexCount, GLTFType::GLTF_SCALAR, GLTFComponentType::GLTF_UnsignedInt, GLTFBufferViewTarget::GLTF_ElementArrayBuffer);
        for (uint32_t i = indexStart; i < indexStart + indexCount; i++)
        {
            uint32_t val = paraXModel->m_indices[i] + vertexOffset;
            if (val < acc->min[0]) acc->min[0] = (float)val;
            if (val > acc->max[0]) acc->max[0] = (float)val;
            acc->bufferView->dataui.push_back(val);
        }
        acc->bufferView->data = (const char*)(acc->bufferView->dataui.data());
        return acc;
    }
	
    std::shared_ptr<GLTFImage> GltfModel::ExportImage(CParaXModel* paraXModel, ModelRenderPass& pass) 
    {
        std::string imagePrefix = "image_";
        int tex = pass.tex;
        std::shared_ptr<GLTFImage> image = std::make_shared<GLTFImage>();
        image->index = imageIndex++;
        image->uri = imagePrefix + std::to_string(image->index) + ".png";
        image->filename = CParaFile::GetParentDirectoryFromPath(m_filename) + image->uri;
        m_gltf->images.push_back(image);
        CParaFile::MakeDirectoryFromFilePath(image->filename.c_str());
        if (tex >= CParaXModel::MAX_MODEL_TEXTURES || paraXModel->specialTextures[tex] == -1 || paraXModel->replaceTextures[paraXModel->specialTextures[tex]] == 0) {
#if defined(USE_DIRECTX_RENDERER)
            paraXModel->textures[tex]->SaveToFile(image->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
#else
            paraXModel->textures[tex]->SaveToFile(image->filename.c_str(), PixelFormat::DXT3, 0, 0);
#endif
        } else {
#if defined(USE_DIRECTX_RENDERER)
            paraXModel->replaceTextures[paraXModel->specialTextures[tex]]->SaveToFile(image->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
#else
            paraXModel->replaceTextures[paraXModel->specialTextures[tex]]->SaveToFile(image->filename.c_str(), PixelFormat::DXT3, 0, 0);
#endif
        }
        if(m_isEmbedTexture) 
		{
            CParaFile file;
            if (file.OpenFile(image->filename.c_str(), true)) 
            {
                int nSize = (int)file.getSize();
                if (nSize > 0)
                {
                    std::string imageData;
                    imageData.resize(nSize);
                    file.read((char*)(&(imageData[0])), nSize);
                    image->uri = std::string("data:image/png;base64,") + ParaEngine::StringHelper::base64(imageData);
                    file.close();
                    CParaFile::DeleteFile(image->filename.c_str());
                }
            }
            
		}
        return image;
    }

    std::shared_ptr<GLTFImage> GltfModel::ExportImage(asset_ptr<TextureEntity> texture) 
    {
        std::string imagePrefix = "image_";
        std::shared_ptr<GLTFImage> image = std::make_shared<GLTFImage>();
        image->index = imageIndex++;
        image->uri = imagePrefix + std::to_string(image->index) + ".png";
        image->filename = CParaFile::GetParentDirectoryFromPath(m_filename) + image->uri;
        m_gltf->images.push_back(image);
        CParaFile::MakeDirectoryFromFilePath(image->filename.c_str());
#if defined(USE_DIRECTX_RENDERER)
            texture->SaveToFile(image->filename.c_str(), D3DFORMAT::D3DFMT_DXT3, 0, 0);
#else
            texture->SaveToFile(image->filename.c_str(), PixelFormat::DXT3, 0, 0);
#endif
        return image;
    }

    std::shared_ptr<GLTFSampler> GltfModel::ExportSampler(DWORD headerType)
    {
        std::shared_ptr<GLTFSampler> sampler = std::make_shared<GLTFSampler>();
        m_gltf->samplers.push_back(sampler);
        if (headerType == PARAX_MODEL_STATIC)
        {
            sampler->magFilter = GLTFSamplerMagFilter::GLTF_MagNearest;
            sampler->minFilter = GLTFSamplerMinFilter::GLTF_NearestMipMapNearest;
            sampler->wrapS = GLTFSamplerWrap::GLTF_Repeat;
            sampler->wrapT = GLTFSamplerWrap::GLTF_Repeat;
            sampler->index = samplerIndex++;
        }
        else
        {
            sampler->magFilter = GLTFSamplerMagFilter::GLTF_MagLinear;
            sampler->minFilter = GLTFSamplerMinFilter::GLTF_NearestMipMapLinear;
            sampler->wrapS = GLTFSamplerWrap::GLTF_Repeat;
            sampler->wrapT = GLTFSamplerWrap::GLTF_Repeat;
            sampler->index = samplerIndex++;
        }
        return sampler;
    }

    std::shared_ptr<GLTFMaterial> GltfModel::ExportMaterial(CParaXModel* paraXModel, ModelRenderPass& pass)
    {
        std::shared_ptr<GLTFMaterial> material = std::make_shared<GLTFMaterial>();
        m_gltf->materials.push_back(material);
        uint32_t tex = pass.tex;
        bool cull = pass.cull;
        material->index = materialIndex++;
        material->alphaMode = "MASK";
        material->alphaCutoff = 0.5;
        material->doubleSide = !cull;
        GLTFPbrMetallicRoughness& metallic = material->metallicRoughness;
        metallic.metallicFactor = 0;
        metallic.roughnessFactor = 1;
        metallic.baseColorFactor[0] = 1.0;
        metallic.baseColorFactor[1] = 1.0;
        metallic.baseColorFactor[2] = 1.0;
        metallic.baseColorFactor[3] = 1.0;

        uint32_t numTextures = paraXModel->m_objNum.nTextures;
        if (numTextures > tex)
        {
            std::shared_ptr<GLTFImage> img = ExportImage(paraXModel, pass);
            std::shared_ptr<GLTFSampler> sampler = ExportSampler();
            std::shared_ptr<GLTFTexture> texture = std::make_shared<GLTFTexture>();
            m_gltf->textures.push_back(texture);
            texture->source = img;
            texture->sampler = sampler;
            texture->index = textureIndex++;
            metallic.baseColorTexture.texture = texture;
            metallic.baseColorTexture.index = texture->index;
            metallic.baseColorTexture.texCoord = 0;
        }

        return material;
    }

    std::shared_ptr<GLTFPrimitive> GltfModel::ExportPrimitive(CParaXModel* paraXModel, ModelRenderPass& pass)
    {
        std::shared_ptr<GLTFPrimitive> primitive = std::make_shared<GLTFPrimitive>();
        primitive->mode = GLTFPrimitiveMode::GLTF_Triangles;
        primitive->indices = ExportIndices(paraXModel, pass);
        if (paraXModel->m_RenderMethod != CParaXModel::BMAX_MODEL) 
        {
            primitive->material = ExportMaterial(paraXModel, pass);
        }
        return primitive;
    }

    std::shared_ptr<GLTFMesh> GltfModel::ExportMesh(CParaXModel* paraXModel)
    {
		std::shared_ptr<GLTFMesh> mesh = std::make_shared<GLTFMesh>();
        mesh->index = meshIndex++;
        m_gltf->meshs.push_back(mesh);

	    std::shared_ptr<GLTFAccessor> v = nullptr;    
	    std::shared_ptr<GLTFAccessor> n = nullptr;    
	    std::shared_ptr<GLTFAccessor> t = nullptr;    
	    std::shared_ptr<GLTFAccessor> c0 = nullptr;    
	    std::shared_ptr<GLTFAccessor> c1 = nullptr;    
	    std::shared_ptr<GLTFAccessor> j = nullptr;    
	    std::shared_ptr<GLTFAccessor> w = nullptr;    
        v = ExportVertices(paraXModel);
        n = ExportNormals(paraXModel);
        t = ExportTextureCoords(paraXModel);

        if (paraXModel->m_header.type == PARAX_MODEL_STATIC || paraXModel->m_RenderMethod == CParaXModel::BMAX_MODEL) 
        {
            c0 = ExportColors0(paraXModel);
            c1 = ExportColors1(paraXModel);
        }
        if (IsExportAnimation(paraXModel)) 
        {
            j = ExportJoints(paraXModel);
            w = ExportWeights(paraXModel);
        }

        for (uint32_t i = 0; i < paraXModel->passes.size(); i++)
		{
			ModelRenderPass& pass = paraXModel->passes[i];
			if (pass.geoset < 0 || !paraXModel->showGeosets[pass.geoset]) continue;
            std::shared_ptr<GLTFPrimitive> primitive = ExportPrimitive(paraXModel, pass);
            primitive->attributes.position = v;
            primitive->attributes.normal = n;
            primitive->attributes.texcoord = t;
            primitive->attributes.color0 = c0;
            primitive->attributes.color1 = c1;
            primitive->attributes.joints = j;
            primitive->attributes.weights = w;
            mesh->primitives.push_back(primitive);
        }

		return mesh;
    }

	std::shared_ptr<GLTFAccessor> GltfModel::ExportMatrices(CParaXModel* paraXModel)
	{
		uint32_t numBones = paraXModel->m_objNum.nBones;
        std::shared_ptr<GLTFAccessor> acc = ExportAccessor(numBones, GLTFType::GLTF_MAT4, GLTFComponentType::GLTF_Float, GLTFBufferViewTarget::GLTF_NotUse);
        uint32_t compCount = GLTFGetComponentTypeCount(acc->type);
		for (uint32_t i = 0; i < numBones; i++)
		{
			for (uint32_t j = 0; j < compCount; j++)
			{
				float val = paraXModel->bones[i].matOffset._m[j];
				if (val < acc->min[j]) acc->min[j] = val;
				if (val > acc->max[j]) acc->max[j] = val;
                acc->bufferView->dataf.push_back(val);
			}
		}
        acc->bufferView->data = (const char*)(acc->bufferView->dataf.data());
		return acc;
	}

    std::shared_ptr<GLTFSkin> GltfModel::ExportSkin(CParaXModel* paraXModel, std::shared_ptr<GLTFNode> parentNode)
    {
		uint32_t numBones = paraXModel->m_objNum.nBones;
        std::shared_ptr<GLTFSkin> skin = std::make_shared<GLTFSkin>();
        m_gltf->skins.push_back(skin);
		skin->inverseBindMatrices = ExportMatrices(paraXModel);
		skin->index = skinIndex++;
		skin->joints.resize(numBones);

        // 默认动画ID
        uint32_t animId = 0;
        for (uint32_t i = 0; i < paraXModel->m_objNum.nAnimations; i++) 
        {
            AnimIndex index = paraXModel->GetAnimIndexByID(i);
            animId = index.nIndex;
            if (index.IsValid()) break;
        }

		for (uint32_t i = 0; i < numBones; i++)
		{
			Bone& bone = paraXModel->bones[i];
			std::shared_ptr<GLTFNode> node = ExportNode();
			node->boneIndex = bone.nIndex;
            node->translation = std::make_shared<Vector3>();
            node->rotation = std::make_shared<Quaternion>();
            node->scale = std::make_shared<Vector3>();

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
						s = bone.scale.getValue(animId, 0);
						m.setScale(s);
						m.m[3][0] *= s.x;
						m.m[3][1] *= s.y;
						m.m[3][2] *= s.z;
					}
					if (bone.rot.used)
					{
						q = bone.rot.getValue(animId, 0);
						m = m.Multiply4x3(Matrix4(q.invertWinding()));
					}
					if (bone.trans.used)
					{
						Vector3 tr = bone.trans.getValue(animId, 0);
						m.offsetTrans(tr);
					}
					m.offsetTrans(bone.pivot);
				}
				else
				{
					if (bone.scale.used)
					{
						s = bone.scale.getValue(animId, 0);
						m.makeScale(s);
						if (bone.rot.used)
						{
							q = bone.rot.getValue(animId, 0);
							m = m.Multiply4x3(Matrix4(q.invertWinding()));
						}
					}
					else
					{
						if (bone.rot.used)
						{
							q = bone.rot.getValue(animId, 0);
							m = Matrix4(q.invertWinding());
						}
						else
							m.identity();
					}
					if (bone.trans.used)
					{
						Vector3 tr = bone.trans.getValue(animId, 0);
						m.offsetTrans(tr);
					}
				}
			}
			else
				m.identity();

			*(node->translation) = m.getTrans();
			*(node->rotation) = q;
			*(node->scale) = s;
			skin->joints[node->boneIndex] = node;
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
		return skin;
    }

    std::shared_ptr<GLTFAnimation> GltfModel::ExportAnimation(CParaXModel* paraXModel, uint32_t animId, std::shared_ptr<GLTFSkin> skin)
    {
		float inverse = 1.0f / 255.0f;
		uint32_t nAnimations = GetAnimationCount(paraXModel);
        std::shared_ptr<GLTFAnimation> animation = std::make_shared<GLTFAnimation>();
        m_gltf->animations.push_back(animation);
		animation->index = animationIndex++;

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
                int time = 0;
                uint32_t animSize = animEnd - animStart + 1;
                Vector3 t, s;
                Quaternion q;
                std::shared_ptr<GLTFAccessor> timeAccessor = ExportAccessor(animSize, GLTFType::GLTF_SCALAR, GLTFComponentType::GLTF_Float, GLTFBufferViewTarget::GLTF_NotUse);
                uint32_t timeCompCount = GLTFGetComponentTypeCount(timeAccessor->type);
                std::shared_ptr<GLTFAccessor> transAccessor = ExportAccessor(animSize, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float, GLTFBufferViewTarget::GLTF_NotUse);
                uint32_t transCompCount = GLTFGetComponentTypeCount(transAccessor->type);
                std::shared_ptr<GLTFAccessor> rotAccessor = ExportAccessor(animSize, GLTFType::GLTF_VEC4, GLTFComponentType::GLTF_Float, GLTFBufferViewTarget::GLTF_NotUse);
                uint32_t rotCompCount = GLTFGetComponentTypeCount(rotAccessor->type);
                std::shared_ptr<GLTFAccessor> scaleAccessor = ExportAccessor(animSize, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float, GLTFBufferViewTarget::GLTF_NotUse);
                uint32_t scaleCompCount = GLTFGetComponentTypeCount(scaleAccessor->type);
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
                    time = time - paraXModel->anims[animId].timeStart;
                    time = (int)(time * inverse);

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
                    t = mat.getTrans();

                    uint32_t compCount = timeCompCount;
                    std::shared_ptr<GLTFAccessor> acc = timeAccessor;
                    for (unsigned int k = 0; k < compCount; k++)
                    {
                        float val = (float)time;
                        if (val < acc->min[k]) acc->min[k] = val;
                        if (val > acc->max[k]) acc->max[k] = val;
                        acc->bufferView->dataf.push_back(val);
                    }
                    acc->bufferView->data = (const char*)(acc->bufferView->dataf.data());

                    compCount = transCompCount;
                    acc = transAccessor;
                    for (unsigned int k = 0; k < compCount; k++)
                    {
                        float val = t[k];
                        if (val < acc->min[k]) acc->min[k] = val;
                        if (val > acc->max[k]) acc->max[k] = val;
                        acc->bufferView->dataf.push_back(val);
                    }
                    acc->bufferView->data = (const char*)(acc->bufferView->dataf.data());
                   
                    compCount = rotCompCount;
                    acc = rotAccessor;
                    for (unsigned int k = 0; k < compCount; k++)
                    {
                        float val = q[k];
                        if (val < acc->min[k]) acc->min[k] = val;
                        if (val > acc->max[k]) acc->max[k] = val;
                        acc->bufferView->dataf.push_back(val);
                    }
                    acc->bufferView->data = (const char*)(acc->bufferView->dataf.data());

                    compCount = scaleCompCount;
                    acc = scaleAccessor;
                    for (unsigned int k = 0; k < compCount; k++)
                    {
                        float val = s[k];
                        if (val < acc->min[k]) acc->min[k] = val;
                        if (val > acc->max[k]) acc->max[k] = val;
                        acc->bufferView->dataf.push_back(val);
                    }
                    acc->bufferView->data = (const char*)(acc->bufferView->dataf.data());
                }
                {
                    GLTFAnimation::GLTFSampler sampler;
                    sampler.interpolation = GLTFInterpolation::GLTF_LINEAR;
                    sampler.used = true;
                    sampler.input = timeAccessor;
                    sampler.output = transAccessor;
                    GLTFAnimation::GLTFChannel channel;
                    channel.target.node = skin->joints[bone.nIndex]->index;  // 根据骨骼索引获取节点索引
                    channel.target.path = GLTFAnimationPath::GLTF_Translation;
                    channel.sampler = (uint32_t)animation->samplers.size();
                    animation->samplers.push_back(sampler);
                    animation->channels.push_back(channel);
                }
                {
                    GLTFAnimation::GLTFSampler sampler;
                    sampler.interpolation = GLTFInterpolation::GLTF_LINEAR;
                    sampler.used = true;
                    sampler.input = timeAccessor;
                    sampler.output = rotAccessor;
                    GLTFAnimation::GLTFChannel channel;
                    channel.target.node = skin->joints[bone.nIndex]->index;  // 根据骨骼索引获取节点索引
                    channel.target.path = GLTFAnimationPath::GLTF_Rotation;
                    channel.sampler = (uint32_t)animation->samplers.size();
                    animation->samplers.push_back(sampler);
                    animation->channels.push_back(channel);
                }
                {
                    GLTFAnimation::GLTFSampler sampler;
                    sampler.interpolation = GLTFInterpolation::GLTF_LINEAR;
                    sampler.used = true;
                    sampler.input = timeAccessor;
                    sampler.output = scaleAccessor;
                    GLTFAnimation::GLTFChannel channel;
                    channel.target.node = skin->joints[bone.nIndex]->index;  // 根据骨骼索引获取节点索引
                    channel.target.path = GLTFAnimationPath::GLTF_Scale;
                    channel.sampler = (uint32_t)animation->samplers.size();
                    animation->samplers.push_back(sampler);
                    animation->channels.push_back(channel);
                }
            }
        }
		return animation;   
    }
	std::shared_ptr<GLTFNode> GltfModel::ExportNode()
    {
        std::shared_ptr<GLTFNode> node = std::make_shared<GLTFNode>();
		node->index = nodeIndex++;
		node->boneIndex = -1;
		node->mesh = nullptr;
        node->skin = nullptr;
        node->translation = nullptr;
        node->rotation = nullptr;
        node->scale = nullptr;
        m_gltf->nodes.push_back(node);
        return node;
    } 

	std::shared_ptr<GLTFScene> GltfModel::ExportScene()
	{
        std::shared_ptr<GLTFScene> scene = std::make_shared<GLTFScene>();
        scene->index = sceneIndex++;
        m_gltf->scenes.push_back(scene);
		return scene;
	}

    std::shared_ptr<GLTFNode> GltfModel::ExportSceneNode(CParaXModel* paraXModel, std::shared_ptr<GLTFScene> scene)
	{
        std::shared_ptr<GLTFNode> node = ExportNode();
        scene->nodes.push_back(node);
		node->mesh = ExportMesh(paraXModel);
		if (IsExportAnimation(paraXModel)) 
        {
            node->skin = ExportSkin(paraXModel, node);
            uint32_t nAnimations = GetAnimationCount(paraXModel);
            for (uint32_t animId = 0; animId < nAnimations; animId++) {
                AnimIndex animIndex = paraXModel->GetAnimIndexByID(animId);
                if (animIndex.IsValid()) {
                    m_gltf->animations.push_back(ExportAnimation(paraXModel, animIndex.nIndex, node->skin));
                }
                // m_gltf->animations.push_back(ExportAnimation(paraXModel, animId, node->skin));
            }
        } 
		return node;
	}

    Json::Value GltfModel::ExportJsonNode(std::shared_ptr<GLTFNode>& node) {
        Json::Value jsonNode;
        if (node->mesh) jsonNode["mesh"] = node->mesh->index;
        if (node->skin) jsonNode["skin"] = node->skin->index;
        if (!node->children.empty())
        {
            Json::Value jsonNodeChildren = Json::Value(Json::arrayValue);
            for (uint32_t i = 0; i < node->children.size(); i++)
			{
				jsonNodeChildren[i] = node->children[i];
			}
			jsonNode["children"] = jsonNodeChildren;
        }
        if (node->matrix) 
        {
            Json::Value t = Json::Value(Json::arrayValue);
			for (uint32_t i = 0; i < 4; i++)
            {
                for (uint32 j = 0; j < 4; j++)
                {
                    t[i * 4 + j] = (*(node->matrix))[i][j];
                }
            } 
            jsonNode["matrix"] = t;
        }
        if (node->translation) 
        {
            Json::Value t = Json::Value(Json::arrayValue);
			t[0u] = (*(node->translation))[0];
			t[1u] = (*(node->translation))[1];
			t[2u] = (*(node->translation))[2];
            jsonNode["translation"] = t;
        }
        if (node->rotation)
        {
            Json::Value r = Json::Value(Json::arrayValue);
            r[0u] = (*(node->rotation))[0];
            r[1u] = (*(node->rotation))[1];
            r[2u] = (*(node->rotation))[2];
            r[3u] = (*(node->rotation))[3];
            jsonNode["rotation"] = r;
        }
        if (node->scale)
        {
            Json::Value s = Json::Value(Json::arrayValue);
            s[0u] = (*(node->scale))[0];
            s[1u] = (*(node->scale))[1];
            s[2u] = (*(node->scale))[2];
            jsonNode["scale"] = s;
        }
        return jsonNode;
    }

    Json::Value GltfModel::ExportJsonBufferView(std::shared_ptr<GLTFBufferView>& bufferView)
	{
		Json::Value jsonBufferView;
		jsonBufferView["buffer"] = bufferView->buffer->index;
		jsonBufferView["byteOffset"] = bufferView->byteOffset;
		jsonBufferView["byteLength"] = bufferView->byteLength;
		if (bufferView->byteStride != 0 && bufferView->target == GLTFBufferViewTarget::GLTF_ArrayBuffer)
        {
			jsonBufferView["byteStride"] = bufferView->byteStride;
        }
		if (bufferView->target != GLTFBufferViewTarget::GLTF_NotUse)
        {
			jsonBufferView["target"] = bufferView->target;
        }

        if (bufferView->byteFill != 0) {
            std::vector<char> vec;
            vec.resize(bufferView->byteFill, (char)0);
            m_string_builder.append(vec.data(), bufferView->byteFill);
        }
        
        m_string_builder.append(bufferView->data, bufferView->byteLength);

        return jsonBufferView;
	}


    Json::Value GltfModel::ExportJsonAccessor(std::shared_ptr<GLTFAccessor>& accessor)
	{
		Json::Value jsonAccessor;
		jsonAccessor["bufferView"] = accessor->bufferView->index;
		jsonAccessor["byteOffset"] = accessor->byteOffset;
		jsonAccessor["componentType"] = accessor->componentType;
		jsonAccessor["count"] = accessor->count;
		jsonAccessor["type"] = GLTFGetTypeString(accessor->type);
        uint32_t compCount = GLTFGetComponentTypeCount(accessor->type);
		Json::Value jmax = Json::Value(Json::arrayValue);
		Json::Value jmin = Json::Value(Json::arrayValue);
		for (uint32_t i = 0; i < compCount; i++)
		{
			switch (accessor->componentType)
			{
			case GLTFComponentType::GLTF_UnsignedByte:
				jmax[i] = (uint8_t)accessor->max[i];
				jmin[i] = (uint8)accessor->min[i];
				break;
			case GLTFComponentType::GLTF_UnsignedShort:
				jmax[i] = (uint16_t)accessor->max[i];
				jmin[i] = (uint16_t)accessor->min[i];
				break;
			case GLTFComponentType::GLTF_UnsignedInt:
				jmax[i] = (uint32_t)accessor->max[i];
				jmin[i] = (uint32_t)accessor->min[i];
				break;
			default:
				jmax[i] = accessor->max[i];
				jmin[i] = accessor->min[i];
				break;
			}
		}
		jsonAccessor["max"] = jmax;
		jsonAccessor["min"] = jmin;
        return jsonAccessor;
	}

    Json::Value GltfModel::ExportJsonMaterial(std::shared_ptr<GLTFMaterial>& material) {
        GLTFPbrMetallicRoughness& pbr = material->metallicRoughness;
		Json::Value jsonBaseColorTexture;
		jsonBaseColorTexture["index"] = pbr.baseColorTexture.index;
		jsonBaseColorTexture["texCoord"] = pbr.baseColorTexture.texCoord;
		Json::Value jsonBaseColorFactor = Json::Value(Json::arrayValue);
		jsonBaseColorFactor[0u] = pbr.baseColorFactor[0];
		jsonBaseColorFactor[1u] = pbr.baseColorFactor[1];
		jsonBaseColorFactor[2u] = pbr.baseColorFactor[2];
		jsonBaseColorFactor[3u] = pbr.baseColorFactor[3];
		Json::Value jsonMetallic;
		jsonMetallic["metallicFactor"] = pbr.metallicFactor;
		jsonMetallic["roughnessFactor"] = pbr.roughnessFactor;
		jsonMetallic["baseColorTexture"] = jsonBaseColorTexture;
		jsonMetallic["baseColorFactor"] = jsonBaseColorFactor;
		Json::Value jsonMaterial;
		jsonMaterial["pbrMetallicRoughness"] = jsonMetallic;
		jsonMaterial["alphaMode"] = material->alphaMode;
		jsonMaterial["alphaCutoff"] = material->alphaCutoff;
		jsonMaterial["doubleSided"] = material->doubleSide;
		return jsonMaterial;
    }

    Json::Value GltfModel::ExportJsonMesh(std::shared_ptr<GLTFMesh>& mesh) {
        Json::Value jsonMesh;
        Json::Value jsonMeshPrimitives = Json::Value(Json::arrayValue);
        for (uint32_t i = 0; i < mesh->primitives.size(); i++)
        {
            std::shared_ptr<GLTFPrimitive> primitive = mesh->primitives[i];
            Json::Value jsonMeshPrimitiveAttributes;
            if (primitive->attributes.position != nullptr)
            {
                jsonMeshPrimitiveAttributes["POSITION"] = primitive->attributes.position->index;
            }
            if (primitive->attributes.normal != nullptr)
            {
                jsonMeshPrimitiveAttributes["NORMAL"] = primitive->attributes.normal->index;
            }
            if (primitive->attributes.texcoord != nullptr)
            {
                jsonMeshPrimitiveAttributes["TEXCOORD_0"] = primitive->attributes.texcoord->index;
            }
            if (primitive->attributes.color0 != nullptr)
            {
                jsonMeshPrimitiveAttributes["COLOR_0"] = primitive->attributes.color0->index;
            }
            if (primitive->attributes.color1 != nullptr)
            {
                jsonMeshPrimitiveAttributes["COLOR_1"] = primitive->attributes.color1->index;
            }
            if (primitive->attributes.joints != nullptr)
            {
                jsonMeshPrimitiveAttributes["JOINTS_0"] = primitive->attributes.joints->index;
            }
            if (primitive->attributes.weights != nullptr)
            {
                jsonMeshPrimitiveAttributes["WEIGHTS_0"] = primitive->attributes.weights->index;
            }

            Json::Value jsonMeshPrimitive;
            jsonMeshPrimitive["attributes"] = jsonMeshPrimitiveAttributes;
            jsonMeshPrimitive["mode"] = primitive->mode;
            if (primitive->indices != nullptr)
            {
                jsonMeshPrimitive["indices"] = primitive->indices->index;
            }
            if (primitive->material != nullptr)
            {
                jsonMeshPrimitive["material"] = primitive->material->index;
            }
            jsonMeshPrimitives[i] = jsonMeshPrimitive;
        }
        jsonMesh["primitives"] = jsonMeshPrimitives;
        return jsonMesh;
    }

    Json::Value GltfModel::ExportJsonAnimation(std::shared_ptr<GLTFAnimation>& animation)
    {
        Json::Value jsonAnimation;
        Json::Value jsonAnimationSamplers = Json::Value(Json::arrayValue);
        for (uint32_t i = 0; i < animation->samplers.size(); i++)
        {
            GLTFAnimation::GLTFSampler& sampler = animation->samplers[i];
            Json::Value jsonAnimationSampler;
            jsonAnimationSampler["input"] = sampler.input->index;
            jsonAnimationSampler["output"] = sampler.output->index;
            switch (sampler.interpolation)
            {
            case GLTFInterpolation::GLTF_LINEAR: jsonAnimationSampler["interpolation"] = "LINEAR"; break;
            case GLTFInterpolation::GLTF_STEP: jsonAnimationSampler["interpolation"] = "STEP"; break;
            case GLTFInterpolation::GLTF_CUBICSPLINE: jsonAnimationSampler["interpolation"] = "CUBICSPLINE"; break;
            default: break;
            }
            jsonAnimationSamplers[i] = jsonAnimationSampler;
        }
        Json::Value jsonAnimationChannels = Json::Value(Json::arrayValue);
        for (uint32_t i = 0; i < animation->channels.size(); i++)
        {
            GLTFAnimation::GLTFChannel& channel = animation->channels[i];
            Json::Value jsonAnimationChannel;
            jsonAnimationChannel["sampler"] = channel.sampler;
            Json::Value jsonAnimationChannelTarget;
            jsonAnimationChannelTarget["node"] = channel.target.node;
            switch (channel.target.path)
            {
            case GLTFAnimationPath::GLTF_Translation: jsonAnimationChannelTarget["path"] = "translation"; break;
            case GLTFAnimationPath::GLTF_Rotation: jsonAnimationChannelTarget["path"] = "rotation"; break;
            case GLTFAnimationPath::GLTF_Scale: jsonAnimationChannelTarget["path"] = "scale"; break;
            default: break;
            }
            jsonAnimationChannel["target"] = jsonAnimationChannelTarget;
            jsonAnimationChannels[i] = jsonAnimationChannel;
        }
        jsonAnimation["samplers"] = jsonAnimationSamplers;
        jsonAnimation["channels"] = jsonAnimationChannels;
        return jsonAnimation;
    }

    Json::Value& GltfModel::ExportJson()
	{
        // asset
        Json::Value jsonAsset;
		jsonAsset["version"] = "2.0";
		jsonAsset["generator"] = "ParaEngine";
		m_json_gltf["asset"] = jsonAsset;

        m_json_gltf["scene"] = 0;
        // scenes
        Json::Value scenes = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->scenes.size(); i++) 
        {
            Json::Value jsonScene;
		    Json::Value jsonSceneNodes = Json::Value(Json::arrayValue);
            std::shared_ptr<GLTFScene> scene = m_gltf->scenes[i];
            for (unsigned int j = 0; j < scene->nodes.size(); j++)
            {
                std::shared_ptr<GLTFNode> node = scene->nodes[j];
                jsonSceneNodes[j] = node->index;
            }
            jsonScene["nodes"] = jsonSceneNodes;
            scenes[scene->index] = jsonScene;
        }
        m_json_gltf["scenes"] = scenes;

        // nodes
		Json::Value nodes = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->nodes.size(); i++)
        {
            std::shared_ptr<GLTFNode> node = m_gltf->nodes[i];
            nodes[node->index] = ExportJsonNode(node);
        }
        m_json_gltf["nodes"] = nodes;

        // meshs
		Json::Value meshes = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->meshs.size(); i++)
        {
            std::shared_ptr<GLTFMesh> mesh = m_gltf->meshs[i];
            meshes[mesh->index] = ExportJsonMesh(mesh);
        }
        m_json_gltf["meshes"] = meshes;

        // materials
		Json::Value materials = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->materials.size(); i++)
        {
            std::shared_ptr<GLTFMaterial> material = m_gltf->materials[i];
            materials[material->index] = ExportJsonMaterial(material);
        }
        if (m_gltf->materials.size() > 0) m_json_gltf["materials"] = materials;

        // textures
		Json::Value textures = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->textures.size(); i++)
        {
            std::shared_ptr<GLTFTexture> texture = m_gltf->textures[i];
            Json::Value jsonTexture;
            jsonTexture["sampler"] = texture->sampler->index;
            jsonTexture["source"] = texture->source->index;
            textures[texture->index] = jsonTexture;
        }
        if (m_gltf->textures.size() > 0) m_json_gltf["textures"] = textures;

        // images
		Json::Value images = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->images.size(); i++)
        {
            std::shared_ptr<GLTFImage> image = m_gltf->images[i];
            Json::Value jsonImage;
            jsonImage["uri"] = image->uri;
            images[image->index] = jsonImage;
        }
        if (m_gltf->images.size() > 0) m_json_gltf["images"] = images;

        // samplers
		Json::Value samplers = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->samplers.size(); i++)
        {
            std::shared_ptr<GLTFSampler> sampler = m_gltf->samplers[i];
            Json::Value jsonSampler;
            jsonSampler["magFilter"] = sampler->magFilter;
            jsonSampler["minFilter"] = sampler->minFilter;
            jsonSampler["wrapS"] = sampler->wrapS;
            jsonSampler["wrapT"] = sampler->wrapT;
            samplers[sampler->index] = jsonSampler;
        }
        if (m_gltf->samplers.size() > 0) m_json_gltf["samplers"] = samplers;

        // accessors
        Json::Value accessors = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->accessors.size(); i++)
        {
            std::shared_ptr<GLTFAccessor> accessor = m_gltf->accessors[i];
            accessors[accessor->index] = ExportJsonAccessor(accessor);
        }
        m_json_gltf["accessors"] = accessors;

        // bufferViews
        Json::Value bufferViews = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->bufferViews.size(); i++)
        {
            std::shared_ptr<GLTFBufferView> bufferView = m_gltf->bufferViews[i];
            bufferViews[bufferView->index] = ExportJsonBufferView(bufferView);
        }
        m_json_gltf["bufferViews"] = bufferViews;

        // buffer
		Json::Value buffers = Json::Value(Json::arrayValue);
        Json::Value jsonBuffer;
        jsonBuffer["byteLength"] = buffer->byteLength;
		jsonBuffer["uri"] = "data:application/octet-stream;base64," + StringHelper::base64(m_string_builder.ToString());
        buffers[buffer->index] = jsonBuffer;
        m_json_gltf["buffers"] = buffers;

        // skins
        Json::Value skins = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->skins.size(); i++)
        {
            std::shared_ptr<GLTFSkin> skin = m_gltf->skins[i];
            Json::Value jsonSkin;
            jsonSkin["inverseBindMatrices"] = skin->inverseBindMatrices->index;
            Json::Value jsonSkinJoints = Json::Value(Json::arrayValue);
            for (int j = 0; j < skin->joints.size(); j++) 
            {
                jsonSkinJoints[j] = skin->joints[j]->index;
            }
            jsonSkin["joints"] = jsonSkinJoints;
            skins[skin->index] = jsonSkin;
        }
        if (m_gltf->skins.size() > 0) m_json_gltf["skins"] = skins;

        // animations
		Json::Value animations = Json::Value(Json::arrayValue);
        for (unsigned int i = 0; i < m_gltf->animations.size(); i++)
        {
            std::shared_ptr<GLTFAnimation> animation = m_gltf->animations[i];
            animations[animation->index] = ExportJsonAnimation(animation);
        }
        if (m_gltf->animations.size() > 0) m_json_gltf["animations"] = animations;

        return m_json_gltf;
	}

    void GltfModel::WriteFile(std::string filename)
	{
		CParaFile file;
		if (file.CreateNewFile(filename.c_str()))
		{
			Json::FastWriter writer;
			const std::string& data = writer.write(m_json_gltf);
			file.write(data.c_str(), (int)data.length());
			file.close();
		}
	}


    std::shared_ptr<GLTFNode> GltfModel::ExportSceneNode(CParaXStaticModel* paraXStaticModel, std::shared_ptr<GLTFScene> scene)
	{
        std::shared_ptr<GLTFNode> node = ExportNode();
        scene->nodes.push_back(node);
		
        std::shared_ptr<GLTFMesh> mesh = std::make_shared<GLTFMesh>();
        mesh->index = meshIndex++;
        m_gltf->meshs.push_back(mesh);

   	    uint32_t size = (uint32_t)paraXStaticModel->m_vertices.size();
        std::shared_ptr<GLTFAccessor> v = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        std::shared_ptr<GLTFAccessor> n = ExportAccessor(size, GLTFType::GLTF_VEC3, GLTFComponentType::GLTF_Float);
        std::shared_ptr<GLTFAccessor> t = ExportAccessor(size, GLTFType::GLTF_VEC2, GLTFComponentType::GLTF_Float);
        for (uint32_t i = 0; i < size; i++)
		{
			mesh_vertex_normal& vertex = paraXStaticModel->m_vertices[i];
            uint32_t vCompCount = GLTFGetComponentTypeCount(v->type);
			for (uint32_t j = 0; j < vCompCount; j++)
			{
				float val = vertex.p[j];
				if (val < v->min[j]) v->min[j] = val;
				if (val > v->max[j]) v->max[j] = val;
    			v->bufferView->dataf.push_back(val);
			}

            uint32_t nCompCount = GLTFGetComponentTypeCount(n->type);
			for (uint32_t j = 0; j < nCompCount; j++)
			{
				float val = vertex.n[j];
				if (val < n->min[j]) n->min[j] = val;
				if (val > n->max[j]) n->max[j] = val;
    			n->bufferView->dataf.push_back(val);
			}
            uint32_t tCompCount = GLTFGetComponentTypeCount(t->type);
			for (uint32_t j = 0; j < tCompCount; j++)
			{
				float val = vertex.uv[j];
				if (val < t->min[j]) t->min[j] = val;
				if (val > t->max[j]) t->max[j] = val;
    			t->bufferView->dataf.push_back(val);
			}
        }
        v->bufferView->data = (const char*)v->bufferView->dataf.data();
        n->bufferView->data = (const char*)n->bufferView->dataf.data();
        t->bufferView->data = (const char*)t->bufferView->dataf.data();

        for (uint32_t i = 0; i < paraXStaticModel->m_passes.size(); i++)
		{
			ParaXStaticModelRenderPass& pass = paraXStaticModel->m_passes[i];
            CParaXMaterial* pMaterial = &(pass.m_material);
            if (pMaterial->isSkipRendering()) continue;

            std::shared_ptr<GLTFPrimitive> primitive = std::make_shared<GLTFPrimitive>();
            primitive->mode = GLTFPrimitiveMode::GLTF_Triangles;
            uint32_t indexCount = pass.indexCount;
            uint32_t indexStart = pass.indexStart;
            std::shared_ptr<GLTFAccessor> acc = ExportAccessor(indexCount, GLTFType::GLTF_SCALAR, GLTFComponentType::GLTF_UnsignedInt, GLTFBufferViewTarget::GLTF_ElementArrayBuffer);
            for (uint32_t i = indexStart; i < indexStart + indexCount; i++)
            {
                uint32_t val = paraXStaticModel->m_indices[i];
                if (val < acc->min[0]) acc->min[0] = (float)val;
                if (val > acc->max[0]) acc->max[0] = (float)val;
                acc->bufferView->dataui.push_back(val);
            }
            acc->bufferView->data = (const char*)(acc->bufferView->dataui.data());
            primitive->indices = acc;

            std::shared_ptr<GLTFMaterial> material = std::make_shared<GLTFMaterial>();
            m_gltf->materials.push_back(material);
            material->index = materialIndex++;
            material->alphaMode = "MASK";
            material->alphaCutoff = 0.5;
            material->doubleSide = false;
            GLTFPbrMetallicRoughness& metallic = material->metallicRoughness;
            metallic.metallicFactor = 0;
            metallic.roughnessFactor = 1;
            metallic.baseColorFactor[0] = 1.0;
            metallic.baseColorFactor[1] = 1.0;
            metallic.baseColorFactor[2] = 1.0;
            metallic.baseColorFactor[3] = 1.0;

            if (pMaterial->m_pTexture1)
            {
                std::shared_ptr<GLTFImage> img = ExportImage(pMaterial->m_pTexture1);
                std::shared_ptr<GLTFSampler> sampler = ExportSampler(PARAX_MODEL_STATIC);
                std::shared_ptr<GLTFTexture> texture = std::make_shared<GLTFTexture>();
                m_gltf->textures.push_back(texture);
                texture->source = img;
                texture->sampler = sampler;
                texture->index = textureIndex++;
                metallic.baseColorTexture.texture = texture;
                metallic.baseColorTexture.index = texture->index;
                metallic.baseColorTexture.texCoord = 0;
            }

            primitive->material = material;
            primitive->attributes.position = v;
            primitive->attributes.normal = n;
            primitive->attributes.texcoord = t;
            primitive->attributes.color0 = nullptr;
            primitive->attributes.color1 = nullptr;
            primitive->attributes.joints = nullptr;
            primitive->attributes.weights = nullptr;
            mesh->primitives.push_back(primitive);
        }

		node->mesh = mesh;
		return node;
	}

    void GltfModel::Export(const char* blocks, std::string filepath)
	{
		BlocksParser parser;
		parser.Load(blocks);
		CParaXModel* mesh = parser.ParseParaXModel();
		if (mesh != nullptr)
		{
            Export(mesh, filepath);
		}
	}

    void GltfModel::ExportBlocks(const char* blocks, const char* filepath) 
    {
        GltfModel gltf;
        gltf.Export(blocks, filepath);
    }
    void GltfModel::ExportParaXModel(CParaXModel* model, const char* filepath) 
    {
        GltfModel gltf;
        gltf.Export(model, filepath);
    }
    void GltfModel::ExportCharModel(CharModelInstance* model, const char* filepath)
    {
        GltfModel gltf;
        gltf.Export(model, filepath);
    }

} // namespace ParaEngine
