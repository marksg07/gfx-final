#include "DepthTexture.h"
#include "gl/textures/TextureParametersBuilder.h"

DepthTexture::DepthTexture(size_t width, size_t height)
    : Texture2D()
{
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                  GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    CS123::GL::TextureParametersBuilder().build().applyTo(*this);

    unbind();
}
