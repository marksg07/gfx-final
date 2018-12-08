#include "DepthTexture.h"

DepthTexture::DepthTexture(size_t width, size_t height)
    : Texture2D(), m_width(width), m_height(height)
{
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,
                  GL_DEPTH_COMPONENT, GL_FLOAT, NULL);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);


    unbind();
}
