#include "ParaEngine.h"
#include "EffectOpenGL.h"
#include "driver.h"
#include "dxeffects.h"
#include "hlsl2glsl.h"
#include "hlslCrossCompiler.h"
#include "hlslLinker.h"
#include "OpenGL.h"
#include "GLShaderDefine.h"
#include "ParseHelper.h"
#include "texture/TextureOpenGL.h"

using namespace ParaEngine;
using namespace IParaEngine;



#define  ATTRIBUTE_NAME_COLOR				"a_color"
#define  ATTRIBUTE_NAME_COLOR1				"a_color2"
#define  ATTRIBUTE_NAME_POSITION			"a_position"
#define  ATTRIBUTE_NAME_TEX_COORD			"a_texCoord"
#define  ATTRIBUTE_NAME_TEX_COORD1			"a_texCoord1"
#define  ATTRIBUTE_NAME_TEX_COORD2			"a_texCoord2"
#define  ATTRIBUTE_NAME_TEX_COORD3			"a_texCoord3"
#define  ATTRIBUTE_NAME_NORMAL				"a_normal"
#define  ATTRIBUTE_NAME_BLEND_WEIGHT		"a_blendWeight"
#define  ATTRIBUTE_NAME_BLEND_INDEX			"a_blendIndex"





namespace ParaEngine
{
	struct UniformInfoGL
	{
		std::string name;
		std::string semantic;
		EShType type;
		int Elements;
		char* initValue;
		int initValueSize;
		bool operator == (const UniformInfoGL& r)
		{
			return r.name == this->name;
		}
		bool operator != (const UniformInfoGL&r)
		{
			return r.name != this->name;
		}
	};
}


inline void GetUniforms(ShHandle parser, std::vector<UniformInfoGL>& uniforms)
{
	int count = Hlsl2Glsl_GetUniformCount(parser);
	if (count > 0)
	{
		const ShUniformInfo* uni = Hlsl2Glsl_GetUniformInfo(parser);

		for (int i = 0; i < count; ++i)
		{
			UniformInfoGL info;
			info.name = uni[i].name;
			if (uni[i].semantic != nullptr) {
				info.semantic = uni[i].semantic;
			}
			info.type = uni[i].type;
			info.Elements = uni[i].arraySize;
			info.initValue = nullptr;
			info.initValueSize = 0;
			if (std::find(uniforms.begin(), uniforms.end(), info) == uniforms.end()) {
				

				if (uni[i].init != nullptr)
				{
					info.initValue = new char[uni[i].initSize];
					info.initValueSize = uni[i].initSize;
					memcpy(info.initValue, uni[i].init, info.initValueSize);
				}
				uniforms.push_back(info);
			}
		}
	}
}


inline const std::string string_to_lower(const std::string& str)
{
	std::string data = str;
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}

inline std::string find_pass_vertex_shader_name(PassNode* pass)
{
	auto states = pass->getStateAssignments();
	std::string vs_codeblock_name = "";
	for (auto state : states)
	{
		const std::string lowerName = string_to_lower(state->getName());
		if (lowerName == "vertexshader")
		{
			auto value = state->getValue();
			if (value->getValueType() == StateValueType::COMPILE)
			{
				auto compileValue = static_cast<const StateCompileValue*>(value);
				if (lowerName == "vertexshader")
				{
					vs_codeblock_name = compileValue->getEntryPoint();
					return vs_codeblock_name;
				}
			}
		}
	}
	return vs_codeblock_name;
}

inline std::string find_pass_pixel_shader_name(PassNode* pass)
{
	auto states = pass->getStateAssignments();
	std::string ps_codeblock_name = "";
	for (auto state : states)
	{
		const std::string lowerName = string_to_lower(state->getName());
		if (lowerName == "pixelshader")
		{
			auto value = state->getValue();
			if (value->getValueType() == StateValueType::COMPILE)
			{
				auto compileValue = static_cast<const StateCompileValue*>(value);
				if (lowerName == "pixelshader")
				{
					ps_codeblock_name = compileValue->getEntryPoint();
					return ps_codeblock_name;
				}
			}
		}
	}
	return ps_codeblock_name;
}


inline void bind_vertx_attrib(GLuint program)
{
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_POSITION, ATTRIBUTE_NAME_POSITION);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_COLOR, ATTRIBUTE_NAME_COLOR);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_COLOR1, ATTRIBUTE_NAME_COLOR1);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_TEX_COORD, ATTRIBUTE_NAME_TEX_COORD);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_TEX_COORD1, ATTRIBUTE_NAME_TEX_COORD1);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_TEX_COORD2, ATTRIBUTE_NAME_TEX_COORD2);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_TEX_COORD3, ATTRIBUTE_NAME_TEX_COORD3);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_NORMAL, ATTRIBUTE_NAME_NORMAL);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_BLEND_WEIGHT, ATTRIBUTE_NAME_BLEND_WEIGHT);
	glBindAttribLocation(program, (GLuint)EGLVertexAttrib::ATTRIB_BLEND_INDEX, ATTRIBUTE_NAME_BLEND_INDEX);
}


bool include_open(bool isSystem, const char* fname, const char* parentfname, const char* parent, std::string& output, void* data)
{
	if (data == nullptr) return true;
	IEffectInclude* include = static_cast<IEffectInclude*>(data);
	void* pData = nullptr;
	uint32_t size = 0;
	if (include->Open(fname, &pData, &size))
	{
		output = std::string((const char*)pData,size);
		free(pData);
		return true;
	}
	return false;

}
void include_close(const char* file, void* data)
{

}





bool hlsl2glsl(const std::string& inCode,
	const std::string& enterpoint,
	EShLanguage toLang,
	ETargetVersion toVersion,
	std::string& outCode,
	std::vector<UniformInfoGL>& uniforms,
	std::string& error,
	IParaEngine::IEffectInclude* include)
{

	Hlsl2Glsl_Initialize();

	ShHandle parser = Hlsl2Glsl_ConstructCompiler(toLang);

	auto linker = parser->GetLinker();
	linker->setUserAttribName(EAttribSemantic::EAttrSemPosition, ATTRIBUTE_NAME_POSITION);
	linker->setUserAttribName(EAttribSemantic::EAttrSemNormal, ATTRIBUTE_NAME_NORMAL);
	linker->setUserAttribName(EAttribSemantic::EAttrSemColor0, ATTRIBUTE_NAME_COLOR);
	linker->setUserAttribName(EAttribSemantic::EAttrSemColor1, ATTRIBUTE_NAME_COLOR1);
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex0, ATTRIBUTE_NAME_TEX_COORD);
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex1, ATTRIBUTE_NAME_TEX_COORD1);
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex2, ATTRIBUTE_NAME_TEX_COORD2);
	linker->setUserAttribName(EAttribSemantic::EAttrSemTex3, ATTRIBUTE_NAME_TEX_COORD3);
	linker->setUserAttribName(EAttribSemantic::EAttrSemBlendWeight, ATTRIBUTE_NAME_BLEND_WEIGHT);
	linker->setUserAttribName(EAttribSemantic::EAttrSemBlendIndices, ATTRIBUTE_NAME_BLEND_INDEX);

	const char* sourceStr = inCode.c_str();
	const char* infoLog = nullptr;
	int opt = ETranslateOpBGRAVertexColor | ETranslateOpFlipUVVertical;
	Hlsl2Glsl_ParseCallbacks callback;

	callback.data = (void*)include;
	callback.includeOpenCallback = include_open;
	callback.includeCloseCallback = include_close;


	int parseOk = Hlsl2Glsl_Parse(parser, sourceStr, toVersion, &callback, opt);
	if (!parseOk) {
		infoLog = Hlsl2Glsl_GetInfoLog(parser);
		error = infoLog;
		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return false;

	}
	int translateOk = Hlsl2Glsl_Translate(parser, enterpoint.c_str(), toVersion, opt);
	if (!translateOk) {
		infoLog = Hlsl2Glsl_GetInfoLog(parser);
		error = infoLog;
		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return false;
	}
	outCode = Hlsl2Glsl_GetShader(parser);

	// remove #line and #version
	std::stringstream outss;
	std::istringstream iss(outCode);
	char buf[1024] = { 0 };
	while (iss.getline(buf, sizeof(buf))) {
		std::string line(buf);
		if (line.find("#line ") != std::string::npos ||
			line.find("#version ") != std::string::npos)
		{
			continue;
		}
		outss << line << std::endl;
	}
	outCode = outss.str();

	GetUniforms(parser, uniforms);
	Hlsl2Glsl_DestructCompiler(parser);
	Hlsl2Glsl_Shutdown();
	return true;
}







inline void make_update_parameter_cmd(const ParameterHandle &handle, const void* data, const uint32_t size, UpdateParmeterCommand* pOutCmd)
{
	if (!pOutCmd)return;
	pOutCmd->data = nullptr;
	pOutCmd->size = 0;
	pOutCmd->handle = handle;
	if (!data)return;
	if (size == 0)return;

	pOutCmd->data = malloc(size);
	if (!pOutCmd->data) return;

	memcpy(pOutCmd->data, data, size);
	pOutCmd->size = size;
}

inline void free_update_parameter_cmd(UpdateParmeterCommand& cmd)
{
	if (cmd.data && cmd.size > 0)
	{
		free(cmd.data);
	}
	cmd.data = nullptr;
	cmd.size = 0;
	cmd.handle.idx = PARA_INVALID_HANDLE;
}




EffectOpenGL::~EffectOpenGL()
{
	if (m_FxDesc)
	{
		auto techniques = m_FxDesc->getTechiques();
		for (size_t i = 0; i < techniques.size(); i++)
		{
			auto passes = techniques[i]->getPasses();
			for (size_t j = 0; j < passes.size(); j++)
			{
				GLuint program = m_ShaderPrograms[i][j];
				glDeleteProgram(program);
			}
		}

		for (auto it = m_Uniforms.begin(); it != m_Uniforms.end(); it++)
		{
			if (it->initValue != nullptr)
			{
				delete[] it->initValue;
			}

		}


		delete m_FxDesc;
		m_FxDesc = nullptr;
	}

	memset(m_ShaderPrograms, 0, sizeof(m_ShaderPrograms));
}
bool EffectOpenGL::GetDesc(IParaEngine::EffectDesc* pOutDesc)
{
	if (!pOutDesc) return false;

	pOutDesc->Techniques = (uint16_t)m_FxDesc->getTechiques().size();
	pOutDesc->Parameters = (uint16_t)m_Uniforms.size();

	return true;
}

ParaEngine::EffectOpenGL::EffectOpenGL()
	:m_FxDesc(nullptr)
	, m_IsBeginTechnique(false)
	, m_IsBeginPass(false)
{
	memset(m_ShaderPrograms, 0, sizeof(m_ShaderPrograms));
	memset(m_ParameterCommands, 0, sizeof(m_ParameterCommands));
	m_CurrentTechniqueHandle.idx = PARA_INVALID_HANDLE;
}





std::shared_ptr<EffectOpenGL> ParaEngine::EffectOpenGL::Create(const std::string& src, IParaEngine::IEffectInclude* include, std::string& error)
{

	shared_ptr<EffectOpenGL> pRet = make_shared<EffectOpenGL>();

	// Parse FX
	pRet->m_FxDesc = new DxEffectsTree();
	DxEffectsParser::Driver parseDriver(*pRet->m_FxDesc);
	bool ret = parseDriver.parse_string(src, "");
	if (!ret)
	{
		error = "Parse FX failed!";
		return nullptr;
	}

	// Translate To OpenGL
	std::vector<UniformInfoGL> uniforms;
	auto techniques = pRet->m_FxDesc->getTechiques();
	auto codeblock = pRet->m_FxDesc->getCodeBlock();
	std::vector<GLuint> programs;
	for (size_t idxTech = 0; idxTech < techniques.size(); idxTech++)
	{
		TechniqueNode* tech = techniques[idxTech];
		auto passes = tech->getPasses();
		for (size_t idxPass = 0; idxPass < passes.size(); idxPass++)
		{
			auto pass = passes[idxPass];
			std::string vs_name = find_pass_vertex_shader_name(pass);
			std::string ps_name = find_pass_pixel_shader_name(pass);
			if (vs_name == "")
			{
				error = "Vertex Shader Codeblock name can't be empty";
				return nullptr;
			}
			if (ps_name == "")
			{
				error = "Pixel Shader Codeblock name can't be empty";
				return nullptr;
			}
			std::string vscode = "";
			std::string pscode = "";
			ETargetVersion targetVersion = ETargetGLSL_110;
#if PARAENGINE_MOBILE
			targetVersion = ETargetGLSL_ES_100;
#endif
			bool ret = hlsl2glsl(codeblock, vs_name, EShLanguage::EShLangVertex, targetVersion, vscode, uniforms, error, include);
			if (!ret || vscode == "")
			{
				error = "can't translate vertex shader " + vs_name + "\n" + error;
				return false;
			}
			ret = hlsl2glsl(codeblock, ps_name, EShLanguage::EShLangFragment, targetVersion, pscode, uniforms, error, include);
			if (!ret || pscode == "")
			{
				error = "can't translate fragment shader " + ps_name + "\n" + error;
				return false;
			}

			GLuint vertex, fragment;
			GLint  sucess;
			GLchar infoLog[512];
			const GLchar* vShaderCode = vscode.c_str();
			const GLchar* fShaderCode = pscode.c_str();

			// vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &sucess);
			if (!sucess)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				glDeleteShader(vertex);
				error = infoLog;
				return false;
			}
			// pixel shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &sucess);
			if (!sucess)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				glDeleteShader(fragment);
				error = infoLog;
				return false;
			}
			// link,shader program
			GLuint program = glCreateProgram();
			glAttachShader(program, vertex);
			glAttachShader(program, fragment);

			bind_vertx_attrib(program);

			glLinkProgram(program);
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			glGetProgramiv(program, GL_LINK_STATUS, &sucess);
			if (!sucess)
			{
				glGetProgramInfoLog(program, 512, NULL, infoLog);
				glDeleteProgram(program);
				error = infoLog;
				return false;
			}


			programs.push_back(program);
			pRet->m_ShaderPrograms[idxTech][idxPass] = program;
		}
	}

	pRet->m_Uniforms = uniforms;

	

	for (size_t i = 0; i < uniforms.size(); i++)
	{
		UniformInfoGL info = uniforms[i];

		// texture slot
		if (info.type == EShType::EShTypeSampler ||
			info.type == EShType::EShTypeSampler1D ||
			info.type == EShType::EShTypeSampler2D ||
			info.type == EShType::EShTypeSampler3D ||
			info.type == EShType::EShTypeSamplerCube)
		{
			int nSlots = pRet->m_TextureSlotMap.size();
			pRet->m_TextureSlotMap[info.name] = nSlots;
			for (size_t j=0;j<programs.size();j++)
			{
				glUseProgram(programs[j]);
				GLint location = glGetUniformLocation(programs[j], info.name.c_str());
				glUniform1i(location,nSlots);
			}


		}
		// apply init value
		if (info.initValue != nullptr)
		{
			pRet->SetRawValue(info.name.c_str(),info.initValue,0, info.initValueSize);
		}

	}

	glUseProgram(0);
	return pRet;
}


IParaEngine::TechniqueHandle ParaEngine::EffectOpenGL::GetTechnique(uint32_t index)
{
	IParaEngine::TechniqueHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	auto techniques = m_FxDesc->getTechiques();
	if (index >= 0 && index < techniques.size())
	{
		handle.idx = index;
	}

	return handle;

}


bool EffectOpenGL::GetTechniqueDesc(const IParaEngine::TechniqueHandle& handle, IParaEngine::TechniqueDesc* pOutDesc)
{
	if (!pOutDesc)return false;
	if (!(isValidHandle(handle) && handle.idx >= 0 && handle.idx < m_FxDesc->getTechiques().size())) return false;
	auto tech = m_FxDesc->getTechiques()[handle.idx];
	pOutDesc->Name = tech->getName();
	pOutDesc->Passes = (uint8_t)tech->getPasses().size();
	return true;
}

IParaEngine::ParameterHandle ParaEngine::EffectOpenGL::GetParameter(uint32_t index)
{
	IParaEngine::ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;
	if (index >= 0 && index < m_Uniforms.size())
	{
		handle.idx = index;
	}

	return handle;
}

bool ParaEngine::EffectOpenGL::GetParameterDesc(const IParaEngine::ParameterHandle & handle, IParaEngine::ParameterDesc * pOutDesc)
{
	if (pOutDesc == NULL)return false;
	if (!isValidHandle(handle) || handle.idx < 0 || handle.idx >= m_Uniforms.size()) return false;
	UniformInfoGL uniform = m_Uniforms[handle.idx];

	pOutDesc->Name = uniform.name;
	pOutDesc->Semantic = uniform.semantic;
	pOutDesc->Elements = uniform.Elements;
	pOutDesc->Type = EParameterType::PT_UNSUPPORTED;
	switch (uniform.type)
	{
	default:
		break;
	case EShTypeVoid:
		pOutDesc->Type = EParameterType::PT_VOID;
		break;
	case EShTypeBool:
		pOutDesc->Type = EParameterType::PT_BOOL;
		break;
	case EShTypeInt:
		pOutDesc->Type = EParameterType::PT_INT;
		break;
	case EShTypeFloat:
		pOutDesc->Type = EParameterType::PT_FLOAT;
		break;
	case EShTypeVec2:
		pOutDesc->Type = EParameterType::PT_FLOAT2;
		break;
	case EShTypeVec3:
		pOutDesc->Type = EParameterType::PT_FLOAT3;
		break;
	case EShTypeVec4:
		pOutDesc->Type = EParameterType::PT_FLOAT4;
		break;
	case EShTypeMat2:
		pOutDesc->Type = EParameterType::PT_FLOAT2x2;
		break;
	case EShTypeMat2x3:
		pOutDesc->Type = EParameterType::PT_FLOAT2x3;
		break;
	case EShTypeMat2x4:
		pOutDesc->Type = EParameterType::PT_FLOAT2x4;
		break;
	case EShTypeMat3:
		pOutDesc->Type = EParameterType::PT_FLOAT3x3;
		break;
	case EShTypeMat3x4:
		pOutDesc->Type = EParameterType::PT_FLOAT3x4;
		break;
	case EShTypeMat4x2:
		pOutDesc->Type = EParameterType::PT_FLOAT4x2;
		break;
	case EShTypeMat4x3:
		pOutDesc->Type = EParameterType::PT_FLOAT4x3;
		break;
	case EShTypeMat4x4:
		pOutDesc->Type = EParameterType::PT_FLOAT4x4;
		break;
	case EShTypeSampler:
		pOutDesc->Type = EParameterType::PT_TEXTURE;
		break;
	case EShTypeSampler1D:
		pOutDesc->Type = EParameterType::PT_TEXTURE1D;
		break;
	case EShTypeSampler2D:
		pOutDesc->Type = EParameterType::PT_TEXTURE2D;
		break;
	case EShTypeSampler3D:
		pOutDesc->Type = EParameterType::PT_TEXTURE3D;
		break;
	case EShTypeSamplerCube:
		pOutDesc->Type = EParameterType::PT_TEXTURECUBE;
		break;
	case EShTypeStruct:
		pOutDesc->Type = EParameterType::PT_STRUCT;
		break;
	}


	return true;
}



bool ParaEngine::EffectOpenGL::SetMatrixArray(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data, uint32_t count)
{
	return SetRawValue(handle, data, 0, sizeof(DeviceMatrix) * count);
}


bool ParaEngine::EffectOpenGL::SetMatrix(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data)
{
	return SetRawValue(handle, data, 0, sizeof(DeviceMatrix));
}


bool ParaEngine::EffectOpenGL::SetVectorArray(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data, uint32_t count)
{
	return SetRawValue(handle, data, 0, sizeof(DeviceVector4) * count);
}


bool ParaEngine::EffectOpenGL::SetFloatArray(const ParameterHandle& handle, const float* data, uint32_t count)
{
	return SetRawValue(handle, data, 0, sizeof(float) * count);

}


bool ParaEngine::EffectOpenGL::SetValue(const ParameterHandle& handle, const void* data, uint32_t size)
{
	return SetRawValue(handle, data, 0, size);
}


bool ParaEngine::EffectOpenGL::SetBool(const ParameterHandle& handle, bool value)
{
	return SetRawValue(handle, &value, 0, sizeof(bool));

}


bool ParaEngine::EffectOpenGL::SetInt(const ParameterHandle& handle, int value)
{
	return SetRawValue(handle, &value, 0, sizeof(value));
}


bool ParaEngine::EffectOpenGL::SetFloat(const ParameterHandle& handle, float value)
{
	return SetRawValue(handle, &value, 0, sizeof(value));
}


bool ParaEngine::EffectOpenGL::SetTexture(const ParameterHandle& handle, IParaEngine::ITexture* texture)
{
	intptr_t* pptr = (intptr_t*)&texture;
	return SetRawValue(handle, pptr, 0, sizeof(intptr_t));
}


bool ParaEngine::EffectOpenGL::SetRawValue(const ParameterHandle& handle, const void* data, uint32_t offset, uint32_t size)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_Uniforms.size()) return false;
	if (!data || size == 0)return false;

	UpdateParmeterCommand cmd;
	make_update_parameter_cmd(handle, data, size, &cmd);
	free_update_parameter_cmd(m_ParameterCommands[handle.idx]);
	m_ParameterCommands[handle.idx] = cmd;
	return  true;
}


IParaEngine::ParameterHandle ParaEngine::EffectOpenGL::GetParameterByName(const char* name)
{
	ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;
	for (size_t i = 0; i < m_Uniforms.size(); i++)
	{
		if (m_Uniforms[i].name == name)
		{
			handle.idx = (uint16_t)i;
			return handle;
		}
	}
	return handle;
}


bool ParaEngine::EffectOpenGL::SetVector(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data)
{
	return SetRawValue(handle, data, 0, sizeof(ParaEngine::DeviceVector4));
}


void ParaEngine::EffectOpenGL::OnLostDevice()
{

}


void ParaEngine::EffectOpenGL::OnResetDevice()
{

}


bool ParaEngine::EffectOpenGL::Begin()
{
	if (m_IsBeginTechnique) return false;
	if (m_IsBeginPass) return false;
	if (!isValidHandle(m_CurrentTechniqueHandle) || m_CurrentTechniqueHandle.idx < 0 || m_CurrentTechniqueHandle.idx >= m_FxDesc->getTechiques().size())
		return false;

	m_IsBeginTechnique = true;

	return true;
}


bool ParaEngine::EffectOpenGL::BeginPass(const uint8_t pass)
{
	if (!m_IsBeginTechnique) return false;
	if (m_IsBeginPass) return false;
	if (!isValidHandle(m_CurrentTechniqueHandle) || m_CurrentTechniqueHandle.idx < 0 || m_CurrentTechniqueHandle.idx >= m_FxDesc->getTechiques().size())
		return false;

	TechniqueNode* techNode = m_FxDesc->getTechiques()[m_CurrentTechniqueHandle.idx];
	if (!techNode)return false;
	int nPass = techNode->getPasses().size();
	if (pass < 0 || pass >= nPass) return false;

	GLuint program = m_ShaderPrograms[m_CurrentTechniqueHandle.idx][pass];

	glUseProgram(program);

	m_IsBeginPass = true;

	m_CurrentPass = pass;

	m_LastRenderState = GetCurrentRenderState();
	auto passState = GetPassRenderState(techNode->getPasses()[pass]);
	ApplyRenderState(passState);

	return true;
}
bool ParaEngine::EffectOpenGL::EndPass()
{
	if (!m_IsBeginTechnique) return false;
	if (!m_IsBeginPass) return false;

	glUseProgram(0);


	m_IsBeginPass = false;
	m_CurrentPass = 0;
	ApplyRenderState(m_LastRenderState);
	return true;
}


bool ParaEngine::EffectOpenGL::End()
{
	if (m_IsBeginPass) return false;
	if (!m_IsBeginTechnique) return false;

	m_IsBeginTechnique = false;
	return true;
}


IParaEngine::TechniqueHandle ParaEngine::EffectOpenGL::GetCurrentTechnique()
{
	return m_CurrentTechniqueHandle;
}


bool ParaEngine::EffectOpenGL::CommitChanges()
{

	if (!m_IsBeginTechnique) return false;
	if (!m_IsBeginPass) return false;

	int nUniforms = m_Uniforms.size();
	for (int i = 0; i < nUniforms; i++)
	{
		UpdateParmeterCommand cmd = m_ParameterCommands[i];
		if (!isValidHandle(cmd.handle)) continue;
		if (cmd.handle.idx != i) continue;
		if (cmd.data == nullptr) continue;
		if (cmd.size == 0) continue;
		UniformInfoGL uniform = m_Uniforms[i];
		GLuint program = m_ShaderPrograms[m_CurrentTechniqueHandle.idx][m_CurrentPass];
		const GLchar* name = uniform.name.c_str();
		GLint location = glGetUniformLocation(program, name);
		switch (uniform.type)
		{
		case EShTypeBool:
		{
			auto value = *((bool*)cmd.data);
			glUniform1i(location, value ? 1 : 0);
			break;
		}
		case EShTypeInt:
		{
			auto value = *((int*)cmd.data);
			glUniform1i(location, value);
			break;
		}
		case EShTypeFloat:
		{
			auto value = *((float*)cmd.data);
			glUniform1f(location, value);
			break;
		}
		case EShTypeVec2:
		{

			float* value = (float*)cmd.data;
			glUniform2fv(location, uniform.Elements + 1, value);
			break;
		}
		case EShTypeVec3:
		{

			float* value = (float*)cmd.data;
			glUniform3fv(location, uniform.Elements + 1, value);
			break;
		}
		case EShTypeVec4:
		{

			float* value = (float*)cmd.data;
			glUniform4fv(location, uniform.Elements + 1, value);
			break;
		}
		case EShTypeMat2:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix2fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat2x3:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix2x3fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat2x4:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix2x4fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat3:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix3fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat3x4:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix3x4fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat4x2:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix4x2fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat4x3:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix4x3fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeMat4x4:
		{

			float* value = (float*)cmd.data;
			glUniformMatrix4fv(location, uniform.Elements + 1, GL_FALSE, value);
			break;
		}
		case EShTypeSampler:
		case EShTypeSampler1D:
		case EShTypeSampler2D:
		case EShTypeSampler3D:
		case EShTypeSamplerCube:
		{
			auto it = m_TextureSlotMap.find(name);
			if (it != m_TextureSlotMap.end())
			{
				GLuint slot = it->second;
				intptr_t* ptr = (intptr_t*)cmd.data;
				IParaEngine::ITexture* texture = (IParaEngine::ITexture*)*ptr;
				TextureOpenGL* tex = static_cast<TextureOpenGL*>(texture);
				if (tex != nullptr)
				{
					glActiveTexture(GL_TEXTURE0 + slot);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, tex->GetTextureID());
				}
				else {
					glActiveTexture(GL_TEXTURE0 + slot);
					glDisable(GL_TEXTURE_2D);
				}

			}
			else {
				return false;
			}
			break;
		}
		default:
			return false;
		}

		//auto error = glGetError();
		//if (error != GL_NO_ERROR)
		//{

		//	return false;
		//}
	}

	return true;
}


const ParaEngine::EffectRenderState EffectOpenGL::GetPassRenderState(PassNode* pass)
{

	auto states = pass->getStateAssignments();
	EffectRenderState rs;
	for (size_t i = 0; i < states.size(); i++)
	{
		StateAssignmentNode* state = states[i];
		const std::string name = string_to_lower(state->getName());
		if (name == "cullmode")
		{
			std::string value = string_to_lower(state->getValue()->toString());
			if (value == "none")
			{
				rs.CullMode = ECullMode::None;
			}
			else if (value == "cw")
			{
				rs.CullMode = ECullMode::CW;
			}
			else if (value == "ccw")
			{
				rs.CullMode = ECullMode::CCW;
			}
		}
		else if(name == "zenable")
		{
			bool value = static_cast<const StateBooleanValue*>(state->getValue())->getValue();
			rs.ZTest = value;
		}
		else if (name == "zwriteenable")
		{
			bool value = static_cast<const StateBooleanValue*>(state->getValue())->getValue();
			rs.ZWrite = value;
		}
		else if (name == "alphablendenable")
		{
			bool value = static_cast<const StateBooleanValue*>(state->getValue())->getValue();
			rs.AlphaBlend = value;
		}

		

	}
	return rs;
}


const ParaEngine::EffectRenderState ParaEngine::EffectOpenGL::GetCurrentRenderState()
{

	EffectRenderState rs;

	GLboolean ZTest = false;
	glGetBooleanv(GL_DEPTH_TEST, &ZTest);
	rs.ZTest = ZTest;

	GLboolean ZWrite = false;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &ZWrite);
	rs.ZWrite = ZWrite;

	GLboolean AlphaBlend = false;
	glGetBooleanv(GL_BLEND, &AlphaBlend);
	rs.AlphaBlend = AlphaBlend;

	GLboolean Cull = false;
	glGetBooleanv(GL_CULL_FACE, &Cull);

	GLint face = 0;
	if (Cull)
	{
		glGetIntegerv(GL_FRONT_FACE, &face);
		if (face == GL_CW)
		{
			rs.CullMode = ECullMode::CW;
		}else if (face = GL_CCW)
		{
			rs.CullMode = ECullMode::CCW;
		}
		else {
			rs.CullMode = ECullMode::None;
		}
	}
	else {
		rs.CullMode = ECullMode::None;
	}
	return rs;

}


void ParaEngine::EffectOpenGL::ApplyRenderState(const EffectRenderState& rs)
{
	if (rs.ZTest)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(rs.ZWrite ? GL_TRUE : GL_FALSE);

	if (rs.AlphaBlend)
	{
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	if (rs.CullMode == ECullMode::None)
	{
		glDisable(GL_CULL_FACE);
	}
	else if (rs.CullMode == ECullMode::CW)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}else if (rs.CullMode == ECullMode::CCW)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}

}

bool ParaEngine::EffectOpenGL::SetRawValue(const char* name, const void* data, uint32_t offset, uint32_t size)
{
	auto handle = GetParameterByName(name);
	return SetRawValue(handle, data, offset, size);
}

bool ParaEngine::EffectOpenGL::SetTexture(const char* name, IParaEngine::ITexture* texture)
{
	auto handle = GetParameterByName(name);
	return SetTexture(handle, texture);
}

bool EffectOpenGL::SetTechnique(const TechniqueHandle& handle)
{
	if (isValidHandle(handle) && handle.idx >= 0 && handle.idx < m_FxDesc->getTechiques().size())
	{
		m_CurrentTechniqueHandle.idx = handle.idx;
		return true;
	}
	return false;
}
