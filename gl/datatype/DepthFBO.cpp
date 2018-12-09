#include "DepthFBO.h"
#include "gl/textures/TextureParametersBuilder.h"
#include "gl/textures/DepthTexture.h"

DepthFBO::DepthFBO(size_t width, size_t height)
    : m_width(width), m_height(height)
{
     glGenFramebuffers(1, &m_handle);
}


