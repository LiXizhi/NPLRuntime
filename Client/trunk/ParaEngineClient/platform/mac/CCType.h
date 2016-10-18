

#ifndef CCTYPE_H
#define CCTYPE_H


#include "../PlatformMacro.h"

#include "ParaGLMac.h"

#define Vec2 CCVector2

#define CC_CONTENT_SCALE_FACTOR() 1


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
};

typedef struct _ttfConfig
{
	std::string fontFilePath;
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

class Ref
{
public:

	Ref();

	void retain();
	void release();

	Ref* autorelease();


	inline int getReferenceCount() const { return _refCount;  }

protected:
	~Ref();

	int _refCount;

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



class Rect
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
	Rect();
	/**
	Constructor a rect.
	* @js NA
	*/
	Rect(float x, float y, float width, float height);
	/**
	Constructor a rect.
	* @js NA
	*/
	Rect(const CCVector2& pos, const Size& dimension);
	/**
	Copy constructor.
	* @js NA
	* @lua NA
	*/
	Rect(const Rect& other);
	/**
	* @js NA
	* @lua NA
	*/
	Rect& operator= (const Rect& other);
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
	bool equals(const Rect& rect) const;
	/**
	Check if the points is contained in the rect.
	* @js NA
	*/
	bool containsPoint(const CCVector2& point) const;
	/**
	Check the intersect status of two rects.
	* @js NA
	*/
	bool intersectsRect(const Rect& rect) const;
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
	Rect unionWithRect(const Rect & rect) const;
	/**Compute the min rect which can contain this and rect, assign it to this.*/
	void merge(const Rect& rect);
	/**An empty Rect.*/
	static const Rect ZERO;
};


class Data
{
public:
	/**
	* This parameter is defined for convenient reference if a null Data object is needed.
	*/
	static const Data Null;

	/**
	* Constructor of Data.
	*/
	Data();

	/**
	* Copy constructor of Data.
	*/
	Data(const Data& other);

	/**
	* Copy constructor of Data.
	*/
	Data(Data&& other);

	/**
	* Destructor of Data.
	*/
	~Data();

	/**
	* Overloads of operator=.
	*/
	Data& operator= (const Data& other);

	/**
	* Overloads of operator=.
	*/
	Data& operator= (Data&& other);

	/**
	* Gets internal bytes of Data. It will return the pointer directly used in Data, so don't delete it.
	*
	* @return Pointer of bytes used internal in Data.
	*/
	unsigned char* getBytes() const;

	/**
	* Gets the size of the bytes.
	*
	* @return The size of bytes of Data.
	*/
	ssize_t getSize() const;

	/** Copies the buffer pointer and its size.
	*  @note This method will copy the whole buffer.
	*        Developer should free the pointer after invoking this method.
	*  @see Data::fastSet
	*/
	void copy(const unsigned char* bytes, const ssize_t size);

	/** Fast set the buffer pointer and its size. Please use it carefully.
	*  @param bytes The buffer pointer, note that it have to be allocated by 'malloc' or 'calloc',
	*         since in the destructor of Data, the buffer will be deleted by 'free'.
	*  @note 1. This method will move the ownship of 'bytes'pointer to Data,
	*        2. The pointer should not be used outside after it was passed to this method.
	*  @see Data::copy
	*/
	void fastSet(unsigned char* bytes, const ssize_t size);

	/**
	* Clears data, free buffer and reset data size.
	*/
	void clear();

	/**
	* Check whether the data is null.
	*
	* @return True if the Data is null, false if not.
	*/
	bool isNull() const;

	/**
	* Get the internal buffer of data and set data to empty state.
	*
	* The ownership of the buffer removed from the data object.
	* That is the user have to free the returned buffer.
	* The data object is set to empty state, that is internal buffer is set to nullptr
	* and size is set to zero.
	* Usage:
	* @code
	*  Data d;
	*  // ...
	*  ssize_t size;
	*  unsigned char* buffer = d.takeBuffer(&size);
	*  // use buffer and size
	*  free(buffer);
	* @endcode
	*
	* @param size Will fill with the data buffer size in bytes, if you do not care buffer size, pass nullptr.
	* @return the internal data buffer, free it after use.
	*/
	unsigned char* takeBuffer(ssize_t* size);
private:
	void move(Data& other);

private:
	unsigned char* _bytes;
	ssize_t _size;
};


#endif
