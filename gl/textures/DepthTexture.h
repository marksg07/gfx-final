#ifndef DEPTHTEXTURE_H
#define DEPTHTEXTURE_H

#include "gl/textures/Texture2D.h"

class DepthTexture : public CS123::GL::Texture
{
public:
    DepthTexture(size_t width, size_t height);


    virtual void bind() const override {
        glBindTexture(GL_TEXTURE_2D, m_handle);
    }

    virtual void unbind() const override {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

protected:

    DepthTexture() :
        Texture() {

    }
};

#endif // DEPTHTEXTURE_H
