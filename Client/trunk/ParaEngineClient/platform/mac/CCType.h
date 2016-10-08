

#ifndef CCTYPE_H
#define CCTYPE_H


#include "ParaGLMac.h"

namespace GL {


    void bindTexture2D(GLuint textureId);

}

struct CCVector2
{
    float x;
    float y;
};

class Ref
{
public:
    void retain()  {}
    void release() {}

    Ref* autorelease() {return this;}

};

class Size
{
public:

    Size();

	Size(float width, float height);



    float width;
    float height;

    static const Size ZERO;
};

#endif
