#ifndef DEPTHFBO_H
#define DEPTHFBO_H

#include <memory>
#include "gl/textures/DepthTexture.h"
#include "GL/glew.h"

class DepthFBO
{
public:
    DepthFBO(size_t width, size_t height);

    void bind() {
        // TODO [Task 3]
        glBindFramebuffer(GL_FRAMEBUFFER, m_handle);

        // TODO [Task 4] // Resize the viewport to our FBO's size
        glViewport(0, 0, m_width, m_height);
    }

    void unbind() {
        // TODO [Task 3]
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attachTexture(std::shared_ptr<DepthTexture> tex)
    {
        m_depthTexture = tex;

        bind();

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture->id(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        unbind();
    }

    GLuint textureID()
    {
        return m_depthTexture->id();
    }

    size_t m_width, m_height;

    GLuint m_handle;

    std::shared_ptr<CS123::GL::Texture> m_depthTexture;
};

#endif // DEPTHFBO_H
