#include "ParaEngine.h"
#include "GLType.h"
#include <cmath>

using namespace std;
using namespace ParaEngine;
CCVector2 CCVector2::ZERO = CCVector2(0,0);

CCVector2::CCVector2() : x(0), y(0)
{
}

CCVector2::CCVector2(float vx, float vy) : x(vx), y(vy)
{
}

bool CCVector2::equals(const CCVector2& v) const
{
	if (v.x != x) return false;
	if (v.y != y) return false;

	return true;
}

static const int MAX_ATTRIBUTES = 16;
static const int MAX_ACTIVE_TEXTURE = 16;

static uint32_t s_attributeFlags = 0;  // 32 attributes max

void  GL::deleteProgram(GLuint program)
{
#if NPLRUNTIME_ENABLE_GL_STATE_CACHE
	if (program == s_currentShaderProgram)
	{
		s_currentShaderProgram = -1;
	}
#endif // NPLRUNTIME_ENABLE_GL_STATE_CACHE

	glDeleteProgram(program);
}

void GL::useProgram(GLuint program)
{
#if NPLRUNTIME_ENABLE_GL_STATE_CACHE
	if (program != s_currentShaderProgram) {
		s_currentShaderProgram = program;
		glUseProgram(program);
	}
#else
	glUseProgram(program);
#endif // NPLRUNTIME_ENABLE_GL_STATE_CACHE
}

void GL::deleteTexture(GLuint textureId)
{
#if NPLRUNTIME_ENABLE_GL_STATE_CACHE
	for (size_t i = 0; i < MAX_ACTIVE_TEXTURE; ++i)
	{
		if (s_currentBoundTexture[i] == textureId)
		{
			s_currentBoundTexture[i] = -1;
		}
	}
#endif // NPLRUNTIME_ENABLE_GL_STATE_CACHE

	glDeleteTextures(1, &textureId);
}


void GL::bindTexture2D(GLuint textureId)
{
	GL::bindTexture2DN(0, textureId);
}

void GL::bindTexture2DN(GLuint textureUnit, GLuint textureId)
{
#if NPLRUNTIME_ENABLE_GL_STATE_CACHE
	CCASSERT(textureUnit < MAX_ACTIVE_TEXTURE, "textureUnit is too big");
	if (s_currentBoundTexture[textureUnit] != textureId)
	{
		s_currentBoundTexture[textureUnit] = textureId;
		activeTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, textureId);
	}
#else
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureId);
#endif
}


void GL::bindVAO(GLuint vaoId)
{
    //if (Configuration::getInstance()->supportsShareableVAO())
	if ( false ) //TODO: wangepng
	{
#if NPLRUNTIME_ENABLE_GL_STATE_CACHE
        if (s_VAO != vaoId)
        {
            s_VAO = vaoId;
            glBindVertexArray(vaoId);
        }
#else
        glBindVertexArray(vaoId);
#endif // NPLRUNTIME_ENABLE_GL_STATE_CACHE

    }
}

// GL Vertex Attrib functions
void GL::enableVertexAttribs(uint32_t flags)
{
    bindVAO(0);

    // hardcoded!
    for(int i=0; i < MAX_ATTRIBUTES; i++) {
        unsigned int bit = 1 << i;
        //FIXME:Cache is disabled, try to enable cache as before
        bool enabled = (flags & bit) != 0;
        bool enabledBefore = (s_attributeFlags & bit) != 0;
        if(enabled != enabledBefore)
        {
            if( enabled )
                glEnableVertexAttribArray(i);
            else
                glDisableVertexAttribArray(i);
        }
    }
    s_attributeFlags = flags;
}

const Size Size::ZERO = Size(0, 0);

Size::Size(void) : width(0), height(0)
{
}

Size::Size(float w, float h) : width(w), height(h)
{
}


bool Size::equals(const Size& v) const
{
	if (v.width != width) return false;
	if (v.height != height) return false;

	return true;
}

CCRect::CCRect(void) : origin(0.0f, 0.0f), size(0.0f, 0.0f)
{
}

CCRect::CCRect(float x, float y, float width, float height)
	: origin(x, y), size(width, height)
{
}

CCRect::CCRect(const CCVector2& pos, const Size& dimension)
	: origin(pos), size(dimension)
{

}

CCRect::CCRect(const CCRect& other)
	: origin(other.origin), size(other.size)
{
}

CCRect& CCRect::operator= (const CCRect& other)
{
	setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
	return *this;
}

void CCRect::setRect(float x, float y, float width, float height)
{
	// CGRect can support width<0 or height<0
	// CCASSERT(width >= 0.0f && height >= 0.0f, "width and height of Rect must not less than 0.");

	origin.x = x;
	origin.y = y;

	size.width = width;
	size.height = height;
}

bool CCRect::equals(const CCRect& rect) const
{
	return (origin.equals(rect.origin) &&
		size.equals(rect.size));
}



float CCRect::getMaxX() const
{
	return origin.x + size.width;
}

float CCRect::getMidX() const
{
	return origin.x + size.width / 2.0f;
}

float CCRect::getMinX() const
{
	return origin.x;
}

float CCRect::getMaxY() const
{
	return origin.y + size.height;
}

float CCRect::getMidY() const
{
	return origin.y + size.height / 2.0f;
}

float CCRect::getMinY() const
{
	return origin.y;
}

bool CCRect::containsPoint(const CCVector2& point) const
{
	bool bRet = false;

	if (point.x >= getMinX() && point.x <= getMaxX()
		&& point.y >= getMinY() && point.y <= getMaxY())
	{
		bRet = true;
	}

	return bRet;
}

bool CCRect::intersectsRect(const CCRect& rect) const
{
	return !(getMaxX() < rect.getMinX() ||
		rect.getMaxX() <      getMinX() ||
		getMaxY() < rect.getMinY() ||
		rect.getMaxY() <      getMinY());
}

bool CCRect::intersectsCircle(const CCVector2& center, float radius) const
{
	CCVector2 rectangleCenter((origin.x + size.width / 2),
		(origin.y + size.height / 2));

	float w = size.width / 2;
	float h = size.height / 2;

	float dx = std::abs(center.x - rectangleCenter.x);
	float dy = std::abs(center.y - rectangleCenter.y);

	if (dx > (radius + w) || dy > (radius + h))
	{
		return false;
	}

	CCVector2 circleDistance(std::abs(center.x - origin.x - w),
		std::abs(center.y - origin.y - h));

	if (circleDistance.x <= (w))
	{
		return true;
	}

	if (circleDistance.y <= (h))
	{
		return true;
	}

	float cornerDistanceSq = powf(circleDistance.x - w, 2) + powf(circleDistance.y - h, 2);

	return (cornerDistanceSq <= (powf(radius, 2)));
}

void CCRect::merge(const CCRect& rect)
{
	float minX = min(getMinX(), rect.getMinX());
	float minY = min(getMinY(), rect.getMinY());
	float maxX = max(getMaxX(), rect.getMaxX());
	float maxY = max(getMaxY(), rect.getMaxY());
	setRect(minX, minY, maxX - minX, maxY - minY);
}

CCRect CCRect::unionWithRect(const CCRect & rect) const
{
	float thisLeftX = origin.x;
	float thisRightX = origin.x + size.width;
	float thisTopY = origin.y + size.height;
	float thisBottomY = origin.y;

	if (thisRightX < thisLeftX)
	{
		std::swap(thisRightX, thisLeftX);   // This rect has negative width
	}

	if (thisTopY < thisBottomY)
	{
		std::swap(thisTopY, thisBottomY);   // This rect has negative height
	}

	float otherLeftX = rect.origin.x;
	float otherRightX = rect.origin.x + rect.size.width;
	float otherTopY = rect.origin.y + rect.size.height;
	float otherBottomY = rect.origin.y;

	if (otherRightX < otherLeftX)
	{
		std::swap(otherRightX, otherLeftX);   // Other rect has negative width
	}

	if (otherTopY < otherBottomY)
	{
		std::swap(otherTopY, otherBottomY);   // Other rect has negative height
	}

	float combinedLeftX = min(thisLeftX, otherLeftX);
	float combinedRightX = max(thisRightX, otherRightX);
	float combinedTopY = max(thisTopY, otherTopY);
	float combinedBottomY = min(thisBottomY, otherBottomY);

	return CCRect(combinedLeftX, combinedBottomY, combinedRightX - combinedLeftX, combinedTopY - combinedBottomY);
}

const CCRect CCRect::ZERO = CCRect(0, 0, 0, 0);




/*
* Color3B
*/

Color3B::Color3B()
	: r(0)
	, g(0)
	, b(0)
{}

Color3B::Color3B(GLubyte _r, GLubyte _g, GLubyte _b)
	: r(_r)
	, g(_g)
	, b(_b)
{}

Color3B::Color3B(const Color4B& color)
	: r(color.r)
	, g(color.g)
	, b(color.b)
{}

Color3B::Color3B(const Color4F& color)
	: r((GLubyte)(color.r * 255.0f))
	, g((GLubyte)(color.g * 255.0f))
	, b((GLubyte)(color.b * 255.0f))
{}

bool Color3B::operator==(const Color3B& right) const
{
	return (r == right.r && g == right.g && b == right.b);
}

bool Color3B::operator==(const Color4B& right) const
{
	return (r == right.r && g == right.g && b == right.b && 255 == right.a);
}

bool Color3B::operator==(const Color4F& right) const
{
	return (right.a == 1.0f && Color4F(*this) == right);
}

bool Color3B::operator!=(const Color3B& right) const
{
	return !(*this == right);
}

bool Color3B::operator!=(const Color4B& right) const
{
	return !(*this == right);
}

bool Color3B::operator!=(const Color4F& right) const
{
	return !(*this == right);
}

/**
* Color4B
*/

Color4B::Color4B()
	: r(0)
	, g(0)
	, b(0)
	, a(0)
{}

Color4B::Color4B(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a)
	: r(_r)
	, g(_g)
	, b(_b)
	, a(_a)
{}

Color4B::Color4B(const Color3B& color)
	: r(color.r)
	, g(color.g)
	, b(color.b)
	, a(255)
{}

Color4B::Color4B(const Color4F& color)
	: r((GLubyte)(color.r * 255))
	, g((GLubyte)(color.g * 255))
	, b((GLubyte)(color.b * 255))
	, a((GLubyte)(color.a * 255))
{}

bool Color4B::operator==(const Color4B& right) const
{
	return (r == right.r && g == right.g && b == right.b && a == right.a);
}

bool Color4B::operator==(const Color3B& right) const
{
	return (r == right.r && g == right.g && b == right.b && a == 255);
}

bool Color4B::operator==(const Color4F& right) const
{
	return (*this == Color4B(right));
}

bool Color4B::operator!=(const Color4B& right) const
{
	return !(*this == right);
}

bool Color4B::operator!=(const Color3B& right) const
{
	return !(*this == right);
}

bool Color4B::operator!=(const Color4F& right) const
{
	return !(*this == right);
}

/**
* Color4F
*/

Color4F::Color4F()
	: r(0.0f)
	, g(0.0f)
	, b(0.0f)
	, a(0.0f)
{}

Color4F::Color4F(float _r, float _g, float _b, float _a)
	: r(_r)
	, g(_g)
	, b(_b)
	, a(_a)
{}

Color4F::Color4F(const Color3B& color)
	: r(color.r / 255.0f)
	, g(color.g / 255.0f)
	, b(color.b / 255.0f)
	, a(1.0f)
{}

Color4F::Color4F(const Color4B& color)
	: r(color.r / 255.0f)
	, g(color.g / 255.0f)
	, b(color.b / 255.0f)
	, a(color.a / 255.0f)
{}

bool Color4F::operator==(const Color4F& right) const
{
	return (r == right.r && g == right.g && b == right.b && a == right.a);
}

bool Color4F::operator==(const Color3B& right) const
{
	return (a == 1.0f && Color3B(*this) == right);
}

bool Color4F::operator==(const Color4B& right) const
{
	return (*this == Color4F(right));
}

bool Color4F::operator!=(const Color4F& right) const
{
	return !(*this == right);
}

bool Color4F::operator!=(const Color3B& right) const
{
	return !(*this == right);
}

bool Color4F::operator!=(const Color4B& right) const
{
	return !(*this == right);
}

/**
* Color constants
*/

const Color3B Color3B::WHITE(255, 255, 255);
const Color3B Color3B::YELLOW(255, 255, 0);
const Color3B Color3B::GREEN(0, 255, 0);
const Color3B Color3B::BLUE(0, 0, 255);
const Color3B Color3B::RED(255, 0, 0);
const Color3B Color3B::MAGENTA(255, 0, 255);
const Color3B Color3B::BLACK(0, 0, 0);
const Color3B Color3B::ORANGE(255, 127, 0);
const Color3B Color3B::GRAY(166, 166, 166);

const Color4B Color4B::WHITE(255, 255, 255, 255);
const Color4B Color4B::YELLOW(255, 255, 0, 255);
const Color4B Color4B::GREEN(0, 255, 0, 255);
const Color4B Color4B::BLUE(0, 0, 255, 255);
const Color4B Color4B::RED(255, 0, 0, 255);
const Color4B Color4B::MAGENTA(255, 0, 255, 255);
const Color4B Color4B::BLACK(0, 0, 0, 255);
const Color4B Color4B::ORANGE(255, 127, 0, 255);
const Color4B Color4B::GRAY(166, 166, 166, 255);

const Color4F Color4F::WHITE(1, 1, 1, 1);
const Color4F Color4F::YELLOW(1, 1, 0, 1);
const Color4F Color4F::GREEN(0, 1, 0, 1);
const Color4F Color4F::BLUE(0, 0, 1, 1);
const Color4F Color4F::RED(1, 0, 0, 1);
const Color4F Color4F::MAGENTA(1, 0, 1, 1);
const Color4F Color4F::BLACK(0, 0, 0, 1);
const Color4F Color4F::ORANGE(1, 0.5f, 0, 1);
const Color4F Color4F::GRAY(0.65f, 0.65f, 0.65f, 1);
