#ifndef PARAENGINE_PLATFORM_THIRDPARTY_S3TC_
#define PARAENGINE_PLATFORM_THIRDPARTY_S3TC_

#include <stdint.h>





enum class S3TCDecodeFlag
{
    DXT1 = 1,
    DXT3 = 3,
    DXT5 = 5,
};

//Decode S3TC encode data to RGB32
void s3tc_decode(uint8_t *encode_data,
                 uint8_t *decode_data,
                 const int pixelsWidth,
                 const int pixelsHeight,
                 S3TCDecodeFlag decodeFlag
                 );
#endif
