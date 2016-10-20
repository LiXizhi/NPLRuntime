

#include "ParaEngine.h"


#ifdef PLATFORM_MAC

#include "CCType.h"

#include <cmath>
using namespace std;

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


void  GL::deleteProgram(GLuint program)
{
#if CC_ENABLE_GL_STATE_CACHE
	if (program == s_currentShaderProgram)
	{
		s_currentShaderProgram = -1;
	}
#endif // CC_ENABLE_GL_STATE_CACHE

	glDeleteProgram(program);
}

void GL::useProgram(GLuint program)
{
#if CC_ENABLE_GL_STATE_CACHE
	if (program != s_currentShaderProgram) {
		s_currentShaderProgram = program;
		glUseProgram(program);
	}
#else
	glUseProgram(program);
#endif // CC_ENABLE_GL_STATE_CACHE
}

void GL::deleteTexture(GLuint textureId)
{
#if CC_ENABLE_GL_STATE_CACHE
	for (size_t i = 0; i < MAX_ACTIVE_TEXTURE; ++i)
	{
		if (s_currentBoundTexture[i] == textureId)
		{
			s_currentBoundTexture[i] = -1;
		}
	}
#endif // CC_ENABLE_GL_STATE_CACHE

	glDeleteTextures(1, &textureId);
}


void GL::bindTexture2D(GLuint textureId)
{
	GL::bindTexture2DN(0, textureId);
}

void GL::bindTexture2DN(GLuint textureUnit, GLuint textureId)
{
#if CC_ENABLE_GL_STATE_CACHE
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


Ref::Ref()
{
	_refCount = 1;
}

Ref::~Ref()
{
}


void Ref::retain()
{
	++_refCount;
}

void Ref::release()
{
	if (_refCount > 0)
	{
		--_refCount;
		if (_refCount == 0)
		{
			delete this;
		}
	}
}

Ref* Ref::autorelease()
{
	return this;
}



Rect::Rect(void) : origin(0.0f, 0.0f), size(0.0f, 0.0f)
{
}

Rect::Rect(float x, float y, float width, float height)
	: origin(x, y), size(width, height)
{
}

Rect::Rect(const CCVector2& pos, const Size& dimension)
	: origin(pos), size(dimension)
{

}

Rect::Rect(const Rect& other)
	: origin(other.origin), size(other.size)
{
}

Rect& Rect::operator= (const Rect& other)
{
	setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
	return *this;
}

void Rect::setRect(float x, float y, float width, float height)
{
	// CGRect can support width<0 or height<0
	// CCASSERT(width >= 0.0f && height >= 0.0f, "width and height of Rect must not less than 0.");

	origin.x = x;
	origin.y = y;

	size.width = width;
	size.height = height;
}

bool Rect::equals(const Rect& rect) const
{
	return (origin.equals(rect.origin) &&
		size.equals(rect.size));
}



float Rect::getMaxX() const
{
	return origin.x + size.width;
}

float Rect::getMidX() const
{
	return origin.x + size.width / 2.0f;
}

float Rect::getMinX() const
{
	return origin.x;
}

float Rect::getMaxY() const
{
	return origin.y + size.height;
}

float Rect::getMidY() const
{
	return origin.y + size.height / 2.0f;
}

float Rect::getMinY() const
{
	return origin.y;
}

bool Rect::containsPoint(const CCVector2& point) const
{
	bool bRet = false;

	if (point.x >= getMinX() && point.x <= getMaxX()
		&& point.y >= getMinY() && point.y <= getMaxY())
	{
		bRet = true;
	}

	return bRet;
}

bool Rect::intersectsRect(const Rect& rect) const
{
	return !(getMaxX() < rect.getMinX() ||
		rect.getMaxX() <      getMinX() ||
		getMaxY() < rect.getMinY() ||
		rect.getMaxY() <      getMinY());
}

bool Rect::intersectsCircle(const CCVector2& center, float radius) const
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

void Rect::merge(const Rect& rect)
{
	float minX = min(getMinX(), rect.getMinX());
	float minY = min(getMinY(), rect.getMinY());
	float maxX = max(getMaxX(), rect.getMaxX());
	float maxY = max(getMaxY(), rect.getMaxY());
	setRect(minX, minY, maxX - minX, maxY - minY);
}

Rect Rect::unionWithRect(const Rect & rect) const
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

	return Rect(combinedLeftX, combinedBottomY, combinedRightX - combinedLeftX, combinedTopY - combinedBottomY);
}

const Rect Rect::ZERO = Rect(0, 0, 0, 0);





const Data Data::Null;

Data::Data() :
	_bytes(nullptr),
	_size(0)
{

}

Data::Data(Data&& other) :
	_bytes(nullptr),
	_size(0)
{
	move(other);
}

Data::Data(const Data& other) :
	_bytes(nullptr),
	_size(0)
{

	copy(other._bytes, other._size);
}

Data::~Data()
{
	clear();
}

Data& Data::operator= (const Data& other)
{
	copy(other._bytes, other._size);
	return *this;
}

Data& Data::operator= (Data&& other)
{
	move(other);
	return *this;
}

void Data::move(Data& other)
{
	clear();

	_bytes = other._bytes;
	_size = other._size;

	other._bytes = nullptr;
	other._size = 0;
}

bool Data::isNull() const
{
	return (_bytes == nullptr || _size == 0);
}

unsigned char* Data::getBytes() const
{
	return _bytes;
}

ssize_t Data::getSize() const
{
	return _size;
}

void Data::copy(const unsigned char* bytes, const ssize_t size)
{
	clear();

	if (size > 0)
	{
		_size = size;
		_bytes = (unsigned char*)malloc(sizeof(unsigned char) * _size);
		memcpy(_bytes, bytes, _size);
	}
}

void Data::fastSet(unsigned char* bytes, const ssize_t size)
{
	_bytes = bytes;
	_size = size;
}

void Data::clear()
{
	free(_bytes);
	_bytes = nullptr;
	_size = 0;
}

unsigned char* Data::takeBuffer(ssize_t* size)
{
	auto buffer = getBytes();
	if (size)
		*size = getSize();
	fastSet(nullptr, 0);
	return buffer;
}



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
	: r(color.r * 255.0f)
	, g(color.g * 255.0f)
	, b(color.b * 255.0f)
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
	: r(color.r * 255)
	, g(color.g * 255)
	, b(color.b * 255)
	, a(color.a * 255)
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



#endif
