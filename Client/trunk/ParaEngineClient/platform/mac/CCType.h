

#ifndef CCTYPE_H
#define CCTYPE_H


namespace GL {


    void bindTexture2D(GLuint textureId) {}

}

class Ref
{
public:
    void retain();
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
