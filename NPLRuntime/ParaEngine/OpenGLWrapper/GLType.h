#pragma once
#include <string>
#ifdef PARA_ENABLE_OPENGL
#include "OpenGL.h"
#else
#include <stdint.h>
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef char GLchar;
#ifndef GL_NO_ERROR
#define GL_NO_ERROR 0
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_BGRA 0x80E1
#define GL_ALPHA 0x1906
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_FLOAT 0x1406
#define GL_INT 0x1404
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#endif
#define PE_CHECK_GL_ERROR_DEBUG() ((void)0)
#endif
#include "PERef.h"

#define GL_CONTENT_SCALE_FACTOR() 1


namespace ParaEngine {
    

enum class GlyphCollection {
	DYNAMIC,
	NEHE,
	ASCII,
	CUSTOM
};

class GL {
public:
	static void bindTexture2D(GLuint textureId);
	static void bindTexture2DN(GLuint textureUnit, GLuint textureId);

	static void deleteTexture(GLuint textureId);

	static void deleteProgram(GLuint program);
	static void useProgram(GLuint program);
	static void bindVAO(GLuint vaoId);
	static void enableVertexAttribs(uint32_t flags);
	static void ClearCache();
};

typedef struct _ttfConfig
{
	std::string fontFilePath;
	std::string fallbackFontFilePath;
	float fontSize;

	GlyphCollection glyphs;
	const char *customGlyphs;

	bool distanceFieldEnabled;
	int outlineSize;

	bool italics;
	bool bold;
	bool underline;
	bool strikethrough;

	_ttfConfig(const std::string& filePath = "", float size = 12, const GlyphCollection& glyphCollection = GlyphCollection::DYNAMIC,
		const char *customGlyphCollection = nullptr, bool useDistanceField = false, int outline = 0,
		bool useItalics = false, bool useBold = false, bool useUnderline = false, bool useStrikethrough = false)
		: fontFilePath(filePath)
		, fontSize(size)
		, glyphs(glyphCollection)
		, customGlyphs(customGlyphCollection)
		, distanceFieldEnabled(useDistanceField)
		, outlineSize(outline)
		, italics(useItalics)
		, bold(useBold)
		, underline(useUnderline)
		, strikethrough(useStrikethrough)
	{
		if (outline > 0)
		{
			distanceFieldEnabled = false;
		}
	}
} TTFConfig;

struct FontLetterDefinition
{
	unsigned short  letteCharUTF16;
	float U;
	float V;
	float width;
	float height;
	float offsetX;
	float offsetY;
	int textureID;
	bool validDefinition;
	int xAdvance;

	int clipBottom;
};


struct Color4B;
struct Color4F;

/** RGB color composed of bytes 3 bytes
@since v3.0
*/
struct Color3B
{
	Color3B();
	Color3B(GLubyte _r, GLubyte _g, GLubyte _b);
	explicit Color3B(const Color4B& color);
	explicit Color3B(const Color4F& color);

	bool operator==(const Color3B& right) const;
	bool operator==(const Color4B& right) const;
	bool operator==(const Color4F& right) const;
	bool operator!=(const Color3B& right) const;
	bool operator!=(const Color4B& right) const;
	bool operator!=(const Color4F& right) const;

	bool equals(const Color3B& other)
	{
		return (*this == other);
	}

	GLubyte r;
	GLubyte g;
	GLubyte b;

	static const Color3B WHITE;
	static const Color3B YELLOW;
	static const Color3B BLUE;
	static const Color3B GREEN;
	static const Color3B RED;
	static const Color3B MAGENTA;
	static const Color3B BLACK;
	static const Color3B ORANGE;
	static const Color3B GRAY;
};

/** RGBA color composed of 4 bytes
@since v3.0
*/
struct Color4B
{
	Color4B();
	Color4B(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a);
	explicit Color4B(const Color3B& color);
	explicit Color4B(const Color4F& color);

	bool operator==(const Color4B& right) const;
	bool operator==(const Color3B& right) const;
	bool operator==(const Color4F& right) const;
	bool operator!=(const Color4B& right) const;
	bool operator!=(const Color3B& right) const;
	bool operator!=(const Color4F& right) const;

	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;

	static const Color4B WHITE;
	static const Color4B YELLOW;
	static const Color4B BLUE;
	static const Color4B GREEN;
	static const Color4B RED;
	static const Color4B MAGENTA;
	static const Color4B BLACK;
	static const Color4B ORANGE;
	static const Color4B GRAY;
};


/** RGBA color composed of 4 floats
@since v3.0
*/
struct Color4F
{
	Color4F();
	Color4F(float _r, float _g, float _b, float _a);
	explicit Color4F(const Color3B& color);
	explicit Color4F(const Color4B& color);

	bool operator==(const Color4F& right) const;
	bool operator==(const Color3B& right) const;
	bool operator==(const Color4B& right) const;
	bool operator!=(const Color4F& right) const;
	bool operator!=(const Color3B& right) const;
	bool operator!=(const Color4B& right) const;

	bool equals(const Color4F &other)
	{
		return (*this == other);
	}

	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;

	static const Color4F WHITE;
	static const Color4F YELLOW;
	static const Color4F BLUE;
	static const Color4F GREEN;
	static const Color4F RED;
	static const Color4F MAGENTA;
	static const Color4F BLACK;
	static const Color4F ORANGE;
	static const Color4F GRAY;
};




class CCVector2
{
public:


    float x;
    float y;

	CCVector2();
	CCVector2(float vx, float vy);

	bool equals(const CCVector2& v) const;

	static CCVector2 ZERO;

};

class Size
{
public:

    Size();

	Size(float width, float height);


	bool equals(const Size& v) const;


    float width;
    float height;

    static const Size ZERO;
};



class CCRect
{
public:
	/**Low left point of rect.*/
	CCVector2 origin;
	/**Width and height of the rect.*/
	Size  size;

public:
	/**
	Constructor an empty Rect.
	* @js NA
	*/
	CCRect();
	/**
	Constructor a rect.
	* @js NA
	*/
	CCRect(float x, float y, float width, float height);
	/**
	Constructor a rect.
	* @js NA
	*/
	CCRect(const CCVector2& pos, const Size& dimension);
	/**
	Copy constructor.
	* @js NA
	* @lua NA
	*/
	CCRect(const CCRect& other);
	/**
	* @js NA
	* @lua NA
	*/
	CCRect& operator= (const CCRect& other);
	/**
	Set the x, y, width and height of Rect.
	* @js NA
	* @lua NA
	*/
	void setRect(float x, float y, float width, float height);
	/**
	Get the left of the rect.
	* @js NA
	*/
	float getMinX() const; /// return the leftmost x-value of current rect
						   /**
						   Get the X coordinate of center point.
						   * @js NA
						   */
	float getMidX() const; /// return the midpoint x-value of current rect
						   /**
						   Get the right of rect.
						   * @js NA
						   */
	float getMaxX() const; /// return the rightmost x-value of current rect
						   /**
						   Get the bottom of rect.
						   * @js NA
						   */
	float getMinY() const; /// return the bottommost y-value of current rect
						   /**
						   Get the Y coordinate of center point.
						   * @js NA
						   */
	float getMidY() const; /// return the midpoint y-value of current rect
						   /**
						   Get top of rect.
						   * @js NA
						   */
	float getMaxY() const; /// return the topmost y-value of current rect
						   /**
						   Compare two rects.
						   * @js NA
						   */
	bool equals(const CCRect& rect) const;
	/**
	Check if the points is contained in the rect.
	* @js NA
	*/
	bool containsPoint(const CCVector2& point) const;
	/**
	Check the intersect status of two rects.
	* @js NA
	*/
	bool intersectsRect(const CCRect& rect) const;
	/**
	Check the intersect status of the rect and a circle.
	* @js NA
	*/
	bool intersectsCircle(const CCVector2& center, float radius) const;
	/**
	Get the min rect which can contain this and rect.
	* @js NA
	* @lua NA
	*/
	CCRect unionWithRect(const CCRect & rect) const;
	/**Compute the min rect which can contain this and rect, assign it to this.*/
	void merge(const CCRect& rect);
	/**An empty Rect.*/
	static const CCRect ZERO;
};
}
