#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "GL/glew.h"
#include <memory>
#include <string>
#include <QImage>
#include "gl/shaders/Shader.h"

class CubeMap
{
public:
    CubeMap(std::string path, std::string ext);


    void draw();
    void draw(GLuint handle);

    ~CubeMap()
    {

    }

    GLuint textureId()
    {
        return m_handle;
    }

private:

    GLuint skyboxVAO, skyboxVBO;
    GLuint m_handle;
};

#endif // CUBEMAP_H
