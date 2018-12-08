#ifndef DEPTHTEXTURE_H
#define DEPTHTEXTURE_H

#include "gl/textures/Texture2D.h"

class DepthTexture : public CS123::GL::Texture2D
{
public:
    DepthTexture(size_t width, size_t height);


private:

    size_t m_width, m_height;
};

#endif // DEPTHTEXTURE_H
