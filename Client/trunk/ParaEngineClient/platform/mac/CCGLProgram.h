/****************************************************************************
Copyright 2011 Jeff Lamarche
Copyright 2012 Goffredo Marocchi
Copyright 2012 Ricardo Quesada
Copyright 2012 cocos2d-x.org
Copyright 2013-2016 Chukong Technologies Inc.


http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CCGLPROGRAM_H__
#define __CCGLPROGRAM_H__

#include <unordered_map>
#include <string>

#include "ParaGLMac.h"

/**
 * @addtogroup renderer
 * @{
 */

NS_CC_BEGIN

class GLProgram;

//FIXME: these two typedefs would be deprecated or removed in version 4.0.
typedef void (*GLInfoFunction)(GLuint program, GLenum pname, GLint* params);
typedef void (*GLLogFunction) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);

/**VertexAttrib is a structure to encapsulate data got from glGetActiveAttrib.*/
struct VertexAttrib
{
    /**Index of attribute, start from 0.*/
    GLuint index;
    /**Number of Data type in the attribute, could range from 0-4.*/
    GLint size;
    /**Data type of the attribute, could be GL_FLOAT, GL_UNSIGNED_BYTE etc.*/
    GLenum type;
    /**The string name in vertex shader.*/
    std::string name;
};
/**Uniform is a structure to encapsulate data got from glGetActiveUniform and glGetUniformLocation.*/
struct Uniform
{
    /**The place where the uniform placed, starts from 0.*/
    GLint location;
    /**Number of data type in attribute.*/
    GLint size;
    /**Data type of the attribute.*/
    GLenum type;
    /**String of the uniform name.*/
    std::string name;
};

/** GLProgram
 Class that implements a glProgram


 @since v2.0.0
 */
class GLProgram : public Ref
{
public:
    /**Enum the preallocated vertex attribute. */
    enum
    {
        /**Index 0 will be used as Position.*/
        VERTEX_ATTRIB_POSITION,
        /**Index 1 will be used as Color.*/
        VERTEX_ATTRIB_COLOR,
        /**Index 2 will be used as Tex coord unit 0.*/
        VERTEX_ATTRIB_TEX_COORD,
        /**Index 3 will be used as Tex coord unit 1.*/
        VERTEX_ATTRIB_TEX_COORD1,
        /**Index 4 will be used as Tex coord unit 2.*/
        VERTEX_ATTRIB_TEX_COORD2,
        /**Index 5 will be used as Tex coord unit 3.*/
        VERTEX_ATTRIB_TEX_COORD3,
        /**Index 6 will be used as Normal.*/
        VERTEX_ATTRIB_NORMAL,
        /**Index 7 will be used as Blend weight for hardware skin.*/
        VERTEX_ATTRIB_BLEND_WEIGHT,
        /**Index 8 will be used as Blend index.*/
        VERTEX_ATTRIB_BLEND_INDEX,
        /**Index 9 will be used as tangent.*/
        VERTEX_ATTRIB_TANGENT,
        /**Index 10 will be used as Binormal.*/
        VERTEX_ATTRIB_BINORMAL,
        VERTEX_ATTRIB_MAX,

        // backward compatibility
        VERTEX_ATTRIB_TEX_COORDS = VERTEX_ATTRIB_TEX_COORD,
    };

    /**Preallocated uniform handle.*/
    enum
    {
        /**Ambient color.*/
        UNIFORM_AMBIENT_COLOR,
        /**Projection matrix.*/
        UNIFORM_P_MATRIX,
        /**Model view matrix.*/
        UNIFORM_MV_MATRIX,
        /**Model view projection matrix.*/
        UNIFORM_MVP_MATRIX,
        /**Normal matrix.*/
        UNIFORM_NORMAL_MATRIX,
        /**Time.*/
        UNIFORM_TIME,
        /**sin(Time).*/
        UNIFORM_SIN_TIME,
        /**cos(Time).*/
        UNIFORM_COS_TIME,
        /**Random number.*/
        UNIFORM_RANDOM01,
        /** @{
        * Sampler 0-3, used for texture.
        */
        UNIFORM_SAMPLER0,
        UNIFORM_SAMPLER1,
        UNIFORM_SAMPLER2,
        UNIFORM_SAMPLER3,
        /**@}*/
        UNIFORM_MAX,
    };

    /** Flags used by the uniforms */
    struct UniformFlags {
        unsigned int usesTime:1;
        unsigned int usesNormal:1;
        unsigned int usesMVP:1;
        unsigned int usesMV:1;
        unsigned int usesP:1;
        unsigned int usesRandom:1;
        // handy way to initialize the bitfield
        UniformFlags() { memset(this, 0, sizeof(*this)); }
    };



    bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
    bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines);

    bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);

    bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines);





    /**@{ Get the uniform or vertex attribute by string name in shader, return null if it does not exist.*/
    Uniform* getUniform(const std::string& name)
	{
		return (Uniform*)0;
	}

    VertexAttrib* getVertexAttrib(const std::string& name)
	{
		return (VertexAttrib*)0;
	}


    void bindAttribLocation(const std::string& attributeName, GLuint index) const
	{

	}


    GLint getAttribLocation(const std::string& attributeName) const
	{
		return 0;
	}
    GLint getUniformLocation(const std::string& attributeName) const
	{
		return 0;
	}

    bool link()
	{
		return true;
	}
    void use() {}
    void updateUniforms() {}
    GLint getUniformLocationForName(const char* name) const
	{
		return 0;
	}



    void setUniformLocationWith1i(GLint location, GLint i1)
	{

	}

    /** calls glUniform2i only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith2i(GLint location, GLint i1, GLint i2)
	{

	}

    /** calls glUniform3i only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3)
	{

	}

    /** calls glUniform4i only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4)
	{

	}


    /** calls glUniform2iv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays)
	{

	}


    /** calls glUniform3iv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays)
	{

	}

    /** calls glUniform4iv only if the values are different than the previous call for this same shader program. */

    void setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays)
	{

	}

    /** calls glUniform1f only if the values are different than the previous call for this same shader program.
     * In js or lua,please use setUniformLocationF32
     * @js NA
     */
    void setUniformLocationWith1f(GLint location, GLfloat f1)
	{

	}

    /** calls glUniform2f only if the values are different than the previous call for this same shader program.
     * In js or lua,please use setUniformLocationF32
     * @js NA
     */
    void setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2)
	{

	}

    /** calls glUniform3f only if the values are different than the previous call for this same shader program.
     * In js or lua,please use setUniformLocationF32
     * @js NA
     */
    void setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3)
	{

	}

    /** calls glUniform4f only if the values are different than the previous call for this same shader program.
     * In js or lua,please use setUniformLocationF32
     * @js NA
     */
    void setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4)
	{

	}

    /** calls glUniformfv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith1fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
	{

	}

    /** calls glUniform2fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
	{

	}

    /** calls glUniform3fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
	{

	}

    /** calls glUniform4fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays)
	{

	}

    /** calls glUniformMatrix2fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices)
	{

	}

    /** calls glUniformMatrix3fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices)
	{

	}

    /** calls glUniformMatrix4fv only if the values are different than the previous call for this same shader program. */
    void setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices)
	{

	}



    /** returns the vertexShader error log */
    std::string getVertexShaderLog() const
	{
		return "";
	}

    /** returns the fragmentShader error log */
    std::string getFragmentShaderLog() const
	{
		return "";
	}


	/** returns the program error log */
    std::string getProgramLog() const
	{
		return "";
	}

    /** Reload all shaders, this function is designed for android
    when opengl context lost, so don't call it.
    */
    void reset()
	{

	}

    /** returns the OpenGL Program object */
    GLuint getProgram() const
	{ return 0; }

    /** returns the Uniform flags */
    const UniformFlags& getUniformFlags() const
	{
		return _flags;
	}

private:
	UniformFlags _flags;

};

NS_CC_END
/**
 end of support group
 @}
 */
#endif /* __CCGLPROGRAM_H__ */
