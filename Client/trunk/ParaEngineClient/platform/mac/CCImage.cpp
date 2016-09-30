
#ifdef PLATFORM_MAC


bool Image::initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
{
    return true;
}

bool Image::saveToFile(const std::string &filename, bool isToRGB = true);
{
    return true;
}

#endif
