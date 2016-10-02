


#ifdef PLATFORM_MAC



Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

bool Texture2D::initWithData(const void *data, ssize_t dataLen, Texture2D::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
	return true;
}

GLuint Texture2D::getName() const
{
    return _name;
}

void Texture2D::setTexParameters(const TexParams& texParams)
{

}

#endif
