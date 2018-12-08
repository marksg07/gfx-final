#include "DepthFBO.h"
#include "gl/textures/TextureParametersBuilder.h"

DepthFBO::DepthFBO(size_t width, size_t height)
    : m_width(width), m_height(height)
{
     glGenFramebuffers(1, &m_handle);

     m_depthTexture = std::make_unique<DepthTexture>(m_width, m_height);

     // hack
     CS123::GL::TextureParametersBuilder().build().applyTo(*m_depthTexture.get());


     // attach depth texture as FBO's depth buffer
     bind();

     glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture->id(), 0);
     glDrawBuffer(GL_NONE);
     glReadBuffer(GL_NONE);

     unbind();
}
