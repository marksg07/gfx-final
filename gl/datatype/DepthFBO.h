#ifndef DEPTHFBO_H
#define DEPTHFBO_H

#include <memory>
#include "gl/textures/DepthTexture.h"

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

    void bindTexture()
    {
        glActiveTexture(GL_TEXTURE0);

        m_depthTexture->bind();
    }

    void unbindTexture()
    {
        m_depthTexture->unbind();
    }

    void unbind() {
        // TODO [Task 3]
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    DepthTexture& texture()
    {
        return *m_depthTexture.get();
    }


private:

    size_t m_width, m_height;

    GLuint m_handle;

    std::unique_ptr<DepthTexture> m_depthTexture;
};

#endif // DEPTHFBO_H
