#ifndef DEPTHTEXTURE_H
#define DEPTHTEXTURE_H

#include "gl/textures/Texture2D.h"

class DepthTexture : public CS123::GL::Texture2D
{
public:
    DepthTexture(size_t width, size_t height);


protected:

    DepthTexture() {

    }
};

#endif // DEPTHTEXTURE_H
