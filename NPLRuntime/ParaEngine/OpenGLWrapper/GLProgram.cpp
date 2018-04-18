#include "ParaEngine.h"

#include "GLProgram.h"

#ifndef WIN32
#include <alloca.h>
#endif

#include "uthash.h"
#include "FileUtils.h"

using namespace ParaEngine;

namespace ParaEngine 
{
	typedef struct _hashUniformEntry
	{
		GLvoid*         value;       // value
		unsigned int    location;    // Key
		UT_hash_handle  hh;          // hash entry
		unsigned int	size;
		GLenum			type;
		bool			dirty;
	} tHashUniformEntry;
}

const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR = "ShaderPositionTextureColor";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP = "ShaderPositionTextureColor_noMVP";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST = "ShaderPositionTextureColorAlphaTest";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV = "ShaderPositionTextureColorAlphaTest_NoMV";
const char* GLProgram::SHADER_NAME_POSITION_COLOR = "ShaderPositionColor";
const char* GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP = "ShaderPositionColor_noMVP";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE = "ShaderPositionTexture";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR = "ShaderPositionTexture_uColor";
const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_A8_COLOR = "ShaderPositionTextureA8Color";
const char* GLProgram::SHADER_NAME_POSITION_U_COLOR = "ShaderPosition_uColor";
const char* GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR = "ShaderPositionLengthTextureColor";

const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL = "ShaderLabelDFNormal";
const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW = "ShaderLabelDFGlow";
const char* GLProgram::SHADER_NAME_LABEL_NORMAL = "ShaderLabelNormal";
const char* GLProgram::SHADER_NAME_LABEL_OUTLINE = "ShaderLabelOutline";

const char* GLProgram::SHADER_3D_POSITION = "Shader3DPosition";
const char* GLProgram::SHADER_3D_POSITION_TEXTURE = "Shader3DPositionTexture";
const char* GLProgram::SHADER_3D_SKINPOSITION_TEXTURE = "Shader3DSkinPositionTexture";


// Attribute names
const char* GLProgram::ATTRIBUTE_NAME_COLOR = "a_color";
const char* GLProgram::ATTRIBUTE_NAME_COLOR1 = "a_color2";
const char* GLProgram::ATTRIBUTE_NAME_POSITION = "a_position";
const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD = "a_texCoord";
const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD1 = "a_texCoord2";
const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD2 = "a_texCoord3";
const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD3 = "a_texCoord4";
const char* GLProgram::ATTRIBUTE_NAME_NORMAL = "a_normal";
const char* GLProgram::ATTRIBUTE_NAME_BLEND_WEIGHT = "a_blendWeight";
const char* GLProgram::ATTRIBUTE_NAME_BLEND_INDEX = "a_blendIndex";

GLProgram* GLProgram::createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray)
{
	auto ret = new (std::nothrow) GLProgram();
	if (ret && ret->initWithByteArrays(vShaderByteArray, fShaderByteArray)) {
		ret->link();
		ret->updateUniforms();
		ret->AddToAutoReleasePool();
		return ret;
	}

	SAFE_DELETE(ret);
	return nullptr;
}

GLProgram* GLProgram::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename)
{
	auto ret = new (std::nothrow) GLProgram();
	if (ret && ret->initWithFilenames(vShaderFilename, fShaderFilename)) {
		ret->link();
		ret->updateUniforms();
		ret->AddToAutoReleasePool();
		return ret;
	}

	SAFE_DELETE(ret);
	return nullptr;
}

GLProgram::GLProgram()
	: _program(0)
	, _vertShader(0)
	, _fragShader(0)
	, _hashForUniforms(nullptr)
	, _dirty(false)
{
	memset(_builtInUniforms, 0, sizeof(_builtInUniforms));
}

GLProgram::~GLProgram()
{
	OUTPUT_LOG("%s %d deallocing GLProgram: %p", __FUNCTION__, __LINE__, this);

	if (_vertShader)
	{
		glDeleteShader(_vertShader);
	}

	if (_fragShader)
	{
		glDeleteShader(_fragShader);
	}

	_vertShader = _fragShader = 0;

	if (_program)
	{
		GL::deleteProgram(_program);
	}

	tHashUniformEntry *current_element, *tmp;

	// Purge uniform hash
	HASH_ITER(hh, _hashForUniforms, current_element, tmp)
	{
		HASH_DEL(_hashForUniforms, current_element);
		free(current_element->value);
		free(current_element);
	}
}

bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray)
{
	_program = glCreateProgram();
	PE_CHECK_GL_ERROR_DEBUG();

	_vertShader = _fragShader = 0;

	if (vShaderByteArray)
	{
		if (!compileShader(&_vertShader, GL_VERTEX_SHADER, vShaderByteArray))
		{
			OUTPUT_LOG("ERROR: Failed to compile vertex shader");
			return false;
		}
	}

	// Create and compile fragment shader
	if (fShaderByteArray)
	{
		if (!compileShader(&_fragShader, GL_FRAGMENT_SHADER, fShaderByteArray))
		{
			OUTPUT_LOG("ERROR: Failed to compile fragment shader");
			return false;
		}
	}

	if (_vertShader)
	{
		glAttachShader(_program, _vertShader);
	}
	PE_CHECK_GL_ERROR_DEBUG();

	if (_fragShader)
	{
		glAttachShader(_program, _fragShader);
	}
	_hashForUniforms = nullptr;

	PE_CHECK_GL_ERROR_DEBUG();

	return true;
}


bool GLProgram::initWithFilenames(const std::string &vShaderFilename, const std::string &fShaderFilename)
{
	std::string vertexSource = CFileUtils::GetStringFromFile(CFileUtils::GetFullPathForFilename(vShaderFilename));
	std::string fragmentSource = CFileUtils::GetStringFromFile(CFileUtils::GetFullPathForFilename(fShaderFilename));

	return initWithByteArrays(vertexSource.c_str(), fragmentSource.c_str());
}

void GLProgram::bindPredefinedVertexAttribs()
{
	static const struct {
		const char *attributeName;
		int location;
	} attribute_locations[] =
	{
		{ GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION },
		{ GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR },
		{ GLProgram::ATTRIBUTE_NAME_COLOR1, GLProgram::VERTEX_ATTRIB_COLOR1 },
		{ GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORD },
		{ GLProgram::ATTRIBUTE_NAME_TEX_COORD1, GLProgram::VERTEX_ATTRIB_TEX_COORD1 },
		{ GLProgram::ATTRIBUTE_NAME_TEX_COORD2, GLProgram::VERTEX_ATTRIB_TEX_COORD2 },
		{ GLProgram::ATTRIBUTE_NAME_TEX_COORD3, GLProgram::VERTEX_ATTRIB_TEX_COORD3 },
		{ GLProgram::ATTRIBUTE_NAME_NORMAL, GLProgram::VERTEX_ATTRIB_NORMAL },
	};

	const int size = sizeof(attribute_locations) / sizeof(attribute_locations[0]);

	for (int i = 0; i<size;i++) {
		glBindAttribLocation(_program, attribute_locations[i].location, attribute_locations[i].attributeName);
	}
}

void GLProgram::parseVertexAttribs()
{
	_vertexAttribs.clear();

	// Query and store vertex attribute meta-data from the program.
	GLint activeAttributes;
	GLint length;
	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
	if (activeAttributes > 0)
	{
		VertexAttrib attribute;

		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
		if (length > 0)
		{
			GLchar* attribName = (GLchar*)alloca(length + 1);

			for (int i = 0; i < activeAttributes; ++i)
			{
				// Query attribute info.
				glGetActiveAttrib(_program, i, length, nullptr, &attribute.size, &attribute.type, attribName);
				attribName[length] = '\0';
				attribute.name = std::string(attribName);

				// Query the pre-assigned attribute location
				attribute.index = glGetAttribLocation(_program, attribName);
				_vertexAttribs[attribute.name] = attribute;
			}
		}
	}
}

void GLProgram::parseUniforms()
{
	_userUniforms.clear();

	// Query and store uniforms from the program.
	GLint activeUniforms;
	glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &activeUniforms);
	if (activeUniforms > 0)
	{
		GLint length;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
		if (length > 0)
		{
			Uniform uniform;

			GLchar* uniformName = (GLchar*)alloca(length + 1);

			for (int i = 0; i < activeUniforms; ++i)
			{
				// Query uniform info.
				glGetActiveUniform(_program, i, length, nullptr, &uniform.size, &uniform.type, uniformName);
				uniformName[length] = '\0';
				{
					// remove possible array '[]' from uniform name
					if (uniform.size > 1 && length > 3)
					{
						char* c = strrchr(uniformName, '[');
						if (c)
						{
							*c = '\0';
						}
					}
					uniform.name = std::string(uniformName);
					uniform.location = glGetUniformLocation(_program, uniformName);
					GLenum __gl_error_code = glGetError();
					if (__gl_error_code != GL_NO_ERROR)
					{
						OUTPUT_LOG("error: 0x%x %s", (int)__gl_error_code, uniformName);
					}

					//TODO:wangpeng assert(__gl_error_code == GL_NO_ERROR);

					_userUniforms[uniform.name] = uniform;
				}
			}
		}
	}
}

Uniform* GLProgram::getUniform(const std::string &name)
{
	const auto itr = _userUniforms.find(name);
	if (itr != _userUniforms.end())
		return &itr->second;
	return nullptr;
}

VertexAttrib* GLProgram::getVertexAttrib(const std::string &name)
{
	const auto itr = _vertexAttribs.find(name);
	if (itr != _vertexAttribs.end())
		return &itr->second;
	return nullptr;
}


bool GLProgram::compileShader(GLuint * shader, GLenum type, const GLchar* source)
{
	GLint status;

	if (!source)
	{
		return false;
	}
    

	const GLchar *sources[] = {
#ifdef PARAENGINE_MOBILE
		"precision highp float;\n"
#endif
		"//\n",
		source,
	};

	*shader = glCreateShader(type);
	glShaderSource(*shader, sizeof(sources) / sizeof(*sources), sources, nullptr);
	glCompileShader(*shader);

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

	if (status!=GL_TRUE)
	{
		GLsizei length;
		glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
		GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);

		glGetShaderSource(*shader, length, nullptr, src);
		OUTPUT_LOG("ERROR: Failed to compile shader:\n%s", src);

		if (type == GL_VERTEX_SHADER)
		{
			OUTPUT_LOG("VertexShaderError:\n %s", getVertexShaderLog().c_str());
		}
		else
		{
			OUTPUT_LOG("FragmentShaderError:\n %s", getFragmentShaderLog().c_str());
		}
		free(src);

		return false;;
	}
	return (status == GL_TRUE);
}

GLint GLProgram::getAttribLocation(const std::string &attributeName) const
{
	return glGetAttribLocation(_program, attributeName.c_str());
}

GLint GLProgram::getUniformLocation(const std::string &attributeName) const
{
	return glGetUniformLocation(_program, attributeName.c_str());
}

void GLProgram::bindAttribLocation(const std::string &attributeName, GLuint index) const
{
	glBindAttribLocation(_program, index, attributeName.c_str());
}

void GLProgram::updateUniforms()
{
	_builtInUniforms[UNIFORM_SAMPLER0] = glGetUniformLocation(_program, "tex0Sampler");
	_builtInUniforms[UNIFORM_SAMPLER1] = glGetUniformLocation(_program, "tex1Sampler");
	_builtInUniforms[UNIFORM_SAMPLER2] = glGetUniformLocation(_program, "tex2Sampler");
	_builtInUniforms[UNIFORM_SAMPLER3] = glGetUniformLocation(_program, "tex3Sampler");

	this->use();

	// Since sample most probably won't change, set it to 0,1,2,3 now.
	if (_builtInUniforms[UNIFORM_SAMPLER0] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER0], 0);
	if (_builtInUniforms[UNIFORM_SAMPLER1] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER1], 1);
	if (_builtInUniforms[UNIFORM_SAMPLER2] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER2], 2);
	if (_builtInUniforms[UNIFORM_SAMPLER3] != -1)
		setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER3], 3);

	// clear any glErrors created by any not found uniforms
	glGetError();
}

bool GLProgram::link()
{
	// Cannot link invalid program
	PE_ASSERT(_program != 0);


	GLint status = GL_TRUE;

	bindPredefinedVertexAttribs();

	glLinkProgram(_program);

	parseVertexAttribs();
	parseUniforms();

	if (_vertShader)
	{
		glDeleteShader(_vertShader);
	}

	if (_fragShader)
	{
		glDeleteShader(_fragShader);
	}

	_vertShader = _fragShader = 0;

	return (status == GL_TRUE);
}

void GLProgram::use()
{
	GL::useProgram(_program);
}

std::string GLProgram::logForOpenGLObject(GLuint object, GLInfoFunction infoFunc, GLLogFunction logFunc) const
{
	std::string ret;
	GLint logLength = 0, charsWritten = 0;
	infoFunc(object, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength < 1)
		return "";

	char *logBytes = (char*)malloc(logLength);
	logFunc(object, logLength, &charsWritten, logBytes);

	ret = logBytes;

	free(logBytes);
	return ret;
}

std::string GLProgram::getVertexShaderLog() const
{
	return this->logForOpenGLObject(_vertShader, (GLInfoFunction)&glGetShaderiv, (GLLogFunction)&glGetShaderInfoLog);
}

std::string GLProgram::getFragmentShaderLog() const
{
	return this->logForOpenGLObject(_fragShader, (GLInfoFunction)&glGetShaderiv, (GLLogFunction)&glGetShaderInfoLog);
}

std::string GLProgram::getProgramLog() const
{
	return this->logForOpenGLObject(_program, (GLInfoFunction)&glGetProgramiv, (GLLogFunction)&glGetProgramInfoLog);
}

// Uniform cache

bool GLProgram::updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes, GLenum type, bool direct)
{
	if (location < 0)
	{
		return false;
	}

	bool updated = true;
	tHashUniformEntry *element = nullptr;
	HASH_FIND_INT(_hashForUniforms, &location, element);

	if (!element)
	{
		element = (tHashUniformEntry*)malloc(sizeof(*element));

		// key
		element->location = location;

		// size
		element->size = bytes;

		// 
		element->type = type;

		if (!direct)
		{
			//
			element->dirty = true;
			_dirty = true;
		}
		else
		{
			element->dirty = false;
		}

		// value
		element->value = malloc(bytes);
		memcpy(element->value, data, bytes);

		HASH_ADD_INT(_hashForUniforms, location, element);
	}
	else
	{
		if (bytes == element->size && memcmp(element->value, data, bytes) == 0)
		{
			updated = false;
		}
		else
		{
			element->value = realloc(element->value, bytes);
			element->size = bytes;

			if (!direct)
			{
				element->dirty = true;
				_dirty = true;
			}
			else
			{
				element->dirty = false;
			}

			memcpy(element->value, data, bytes);
		}
	}

	return updated;
}

GLint GLProgram::getUniformLocationForName(const char* name) const
{
	// Invalid uniform name
	PE_ASSERT(name != nullptr);
	// Invalid operation. Cannot get uniform location when program is not initialized
	PE_ASSERT(_program != 0);

	auto ret = glGetUniformLocation(_program, name);
	PE_CHECK_GL_ERROR_DEBUG();
	return ret;
}

void GLProgram::setUniformLocationWith1i(GLint location, GLint i1, bool direct)
{
	bool updated = updateUniformLocation(location, &i1, sizeof(i1) * 1, GL_INT, direct);

	if (updated && direct)
	{
		glUniform1i((GLint)location, i1);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith2i(GLint location, GLint i1, GLint i2, bool direct)
{
	GLint ints[2] = { i1,i2 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints), GL_INT, direct);

	if (updated && direct)
	{
		glUniform2i((GLint)location, i1, i2);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3, bool direct)
{
	GLint ints[3] = { i1,i2,i3 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints), GL_INT, direct);

	if (updated && direct)
	{
		glUniform3i((GLint)location, i1, i2, i3);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4, bool direct)
{
	GLint ints[4] = { i1,i2,i3,i4 };
	bool updated = updateUniformLocation(location, ints, sizeof(ints), GL_INT, direct);

	if (updated && direct)
	{
		glUniform4i((GLint)location, i1, i2, i3, i4);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 2 * numberOfArrays, GL_INT_VEC2, direct);

	if (updated && direct)
	{
		glUniform2iv((GLint)location, (GLsizei)numberOfArrays, ints);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 3 * numberOfArrays, GL_INT_VEC3, direct);

	if (updated && direct)
	{
		glUniform3iv((GLint)location, (GLsizei)numberOfArrays, ints);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, ints, sizeof(int) * 4 * numberOfArrays, GL_INT_VEC4, direct);

	if (updated && direct)
	{
		glUniform4iv((GLint)location, (GLsizei)numberOfArrays, ints);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith1f(GLint location, GLfloat f1, bool direct)
{
	bool updated = updateUniformLocation(location, &f1, sizeof(f1) * 1, GL_FLOAT, direct);

	if (updated && direct)
	{
		glUniform1f((GLint)location, f1);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2, bool direct)
{
	GLfloat floats[2] = { f1,f2 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats), GL_FLOAT, direct);

	if (updated && direct)
	{
		glUniform2f((GLint)location, f1, f2);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, bool direct)
{
	GLfloat floats[3] = { f1,f2,f3 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats), GL_FLOAT, direct);

	if (updated && direct)
	{
		glUniform3f((GLint)location, f1, f2, f3);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4, bool direct)
{
	GLfloat floats[4] = { f1,f2,f3,f4 };
	bool updated = updateUniformLocation(location, floats, sizeof(floats), GL_FLOAT, direct);

	if (updated && direct)
	{
		glUniform4f((GLint)location, f1, f2, f3, f4);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 2 * numberOfArrays, GL_FLOAT_VEC2, direct);

	if (updated && direct)
	{
		glUniform2fv((GLint)location, (GLsizei)numberOfArrays, floats);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 3 * numberOfArrays, GL_FLOAT_VEC3, direct);

	if (updated && direct)
	{
		glUniform3fv((GLint)location, (GLsizei)numberOfArrays, floats);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays, bool direct)
{
	bool updated = updateUniformLocation(location, floats, sizeof(float) * 4 * numberOfArrays, GL_FLOAT_VEC4, direct);

	if (updated && direct)
	{
		glUniform4fv((GLint)location, (GLsizei)numberOfArrays, floats);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices, bool direct) {
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 4 * numberOfMatrices, GL_FLOAT_MAT2, direct);

	if (updated && direct)
	{
		glUniformMatrix2fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
		PE_CHECK_GL_ERROR_DEBUG(); 
	}
}

void GLProgram::setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices, bool direct) {
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 9 * numberOfMatrices, GL_FLOAT_MAT3, direct);

	if (updated && direct)
	{
		glUniformMatrix3fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}


void GLProgram::setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices, bool direct)
{
	bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 16 * numberOfMatrices, GL_FLOAT_MAT4, direct);

	if (updated && direct)
	{
		glUniformMatrix4fv((GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

void GLProgram::commit()
{
	if (!_dirty)
		return;

	tHashUniformEntry *current_element, *tmp;

	// Purge uniform hash
	HASH_ITER(hh, _hashForUniforms, current_element, tmp)
	{
		if (!current_element->dirty)
			continue;

		switch (current_element->type)
		{
			case GL_INT:
			{
				auto count = current_element->size / sizeof(GLint);

				if (count == 1)
				{
					// do not use GLint i = *((GLint*)current_element->value);
					// it may be crash on android
					GLint i;
					memcpy(&i, current_element->value, sizeof(GLint));
					glUniform1i(current_element->location, i);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 2)
				{
					GLint i[2];
					memcpy(i, current_element->value, sizeof(GLint) * 2);
					glUniform2i(current_element->location, i[0], i[1]);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 3)
				{
					GLint i[3];
					memcpy(i, current_element->value, sizeof(GLint) * 3);
					glUniform3i(current_element->location, i[0], i[1], i[2]);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 4)
				{
					GLint i[4];
					memcpy(i, current_element->value, sizeof(GLint) * 4);
					glUniform4i(current_element->location, i[0], i[1], i[2], i[3]);
					PE_CHECK_GL_ERROR_DEBUG();
				}

				break;
			}
			case GL_FLOAT:
			{
				auto count = current_element->size / sizeof(GLfloat);

				if (count == 1)
				{
					GLfloat i;
					memcpy(&i, current_element->value, sizeof(GLfloat));
					glUniform1f(current_element->location, i);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 2)
				{
					GLfloat i[2];
					memcpy(i, current_element->value, sizeof(GLfloat) * 2);
					glUniform2f(current_element->location, i[0], i[1]);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 3)
				{
					GLfloat i[3];
					memcpy(i, current_element->value, sizeof(GLfloat) * 3);
					glUniform3f(current_element->location, i[0], i[1], i[2]);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				else if (count == 4)
				{
					GLfloat i[4];
					memcpy(i, current_element->value, sizeof(GLfloat) * 4);
					glUniform4i(current_element->location, i[0], i[1], i[2], i[3]);
					PE_CHECK_GL_ERROR_DEBUG();
				}
				break;
			}

			case GL_FLOAT_VEC2:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 2);
				glUniform2fv(current_element->location, (GLsizei)count, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_FLOAT_VEC3:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 3);
				glUniform3fv(current_element->location, (GLsizei)count, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_FLOAT_VEC4:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 4);
				glUniform4fv(current_element->location, (GLsizei)count, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_INT_VEC2:
			{
				auto count = current_element->size / (sizeof(GLint) * 2);
				glUniform2iv(current_element->location, (GLsizei)count, (GLint*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_INT_VEC3:
			{
				auto count = current_element->size / (sizeof(GLint) * 3);
				glUniform3iv(current_element->location, (GLsizei)count, (GLint*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_INT_VEC4:
			{
				auto count = current_element->size / (sizeof(GLint) * 4);
				glUniform4iv(current_element->location, (GLsizei)count, (GLint*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_FLOAT_MAT2:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 4);
				glUniformMatrix2fv(current_element->location, (GLsizei)count, GL_FALSE, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_FLOAT_MAT3:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 9);
				glUniformMatrix3fv(current_element->location, (GLsizei)count, GL_FALSE, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

			case GL_FLOAT_MAT4:
			{
				auto count = current_element->size / (sizeof(GLfloat) * 16);
				glUniformMatrix4fv(current_element->location, (GLsizei)count, GL_FALSE, (GLfloat*)current_element->value);
				PE_CHECK_GL_ERROR_DEBUG();
				break;
			}

		default:
			break;
		}

		current_element->dirty = false;
	}

	_dirty = false;
}


void GLProgram::reset()
{
	_vertShader = _fragShader = 0;
	memset(_builtInUniforms, 0, sizeof(_builtInUniforms));


	// it is already deallocated by android
	//GL::deleteProgram(_program);
	_program = 0;


	tHashUniformEntry *current_element, *tmp;

	// Purge uniform hash
	HASH_ITER(hh, _hashForUniforms, current_element, tmp)
	{
		HASH_DEL(_hashForUniforms, current_element);
		free(current_element->value);
		free(current_element);
	}
	_hashForUniforms = nullptr;

	_dirty = false;
}
