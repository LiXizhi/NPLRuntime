#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "driver.h"
#include "dxeffects.h"
#include "hlsl2glsl.h"
#include "hlslCrossCompiler.h"
#include "hlslLinker.h"


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


bool include_open(bool isSystem, const char* fname, const char* parentfname, const char* parent, std::string& output, void* data)
{
	std::ifstream in(fname);
	if (!in.good()) return false;

	std::stringstream vShaderStream;
	vShaderStream << in.rdbuf();

	output = vShaderStream.str();

	in.close();

	return true;
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
	std::string& error)
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

	callback.data = nullptr;
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

	GetUniforms(parser, uniforms);
	Hlsl2Glsl_DestructCompiler(parser);
	Hlsl2Glsl_Shutdown();
	return true;
}



int main(int argc, const char** argv)
{



	if (argc < 2) return 0;

	std::string fileName = argv[1];

	// Parse FX
	std::cout << "Parsing fx file.." << std::endl;
	auto fxDesc = new DxEffectsTree;
	DxEffectsParser::Driver fxParseDriver(*fxDesc);
	bool ret = fxParseDriver.parse_file(fileName);
	if (!ret)
	{
		std::cout << "Parse fx filed." << std::endl;
		delete fxDesc;
		return 1;
	}
	// Translate to GLSL
	std::vector<UniformInfoGL> uniforms;
	auto techniques = fxDesc->getTechiques();
	auto codeblock = fxDesc->getCodeBlock();
	std::cout << "Techniques "<<techniques.size() << std::endl;
	std::string error = "";
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
				std::cout << "Vertex Shader Codeblock name can't be empty" << std::endl;
				delete fxDesc;
				return 1;
			}
			if (ps_name == "")
			{
				error = "Pixel Shader Codeblock name can't be empty";
				delete fxDesc;
				return 1;
			}

			std::cout << "Translate Techinue:" << tech->getName() << " Pass:" + pass->getName() << " vert:" 
				<<vs_name <<" frag:"<<ps_name<<std::endl;

			std::string vscode = "";
			std::string pscode = "";
			ETargetVersion targetVersion = ETargetGLSL_110;
#if PARAENGINE_MOBILE
			targetVersion = ETargetGLSL_ES_100;
#endif
			bool ret = hlsl2glsl(codeblock, vs_name, EShLanguage::EShLangVertex, targetVersion, vscode, uniforms, error);
			if (!ret || vscode == "")
			{
				std::cout << "can't translate vertex shader " + vs_name + "\n" + error << std::endl;;
				delete fxDesc;
				return 1;
			}

			std::cout << "Vertex Shader GLSL Code" << std::endl;
			std::cout << "===============================================" << std::endl;
			std::cout << vscode << std::endl;


			ret = hlsl2glsl(codeblock, ps_name, EShLanguage::EShLangFragment, targetVersion, pscode, uniforms, error);
			if (!ret || pscode == "")
			{
				std::cout<<"can't translate fragment shader " + ps_name + "\n" + error<<std::endl;
				delete fxDesc;
				return 1;
			}

			std::cout << "Pixel Shader GLSL Code" << std::endl;
			std::cout << "===============================================" << std::endl;
			std::cout << pscode << std::endl;
		}
	}
	std::cout << "Uniforms:" << std::endl;
	for (size_t i = 0; i < uniforms.size(); i++)
	{
		UniformInfoGL info = uniforms[i];
		std::cout << info.name << std::endl;
	

	}
	delete fxDesc;
	return 0;
}