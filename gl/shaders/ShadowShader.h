#ifndef SHADOWSHADER_H
#define SHADOWSHADER_H

#include "Shader.h"

class ShadowShader : public CS123::GL::Shader
{
public:

    ShadowShader(const std::string &vertexSource, const std::string &fragmentSource) :
        Shader(vertexSource, fragmentSource)
    {
        init();
    }

    ShadowShader(const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource) :
        Shader(vertexSource, geometrySource, fragmentSource)
    {
        init();
    }

    void init()
    {
        glGenFramebuffers(1, &fbo);
    }

private:

    GLuint fbo;

};

#endif // SHADOWSHADER_H
