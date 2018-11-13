#ifndef GLVERTEX_H
#define GLVERTEX_H

#include <iostream>
#include <vector>
#include <bitset>
#include <functional>
#include "gl/shaders/ShaderAttribLocations.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

class GLVertex
{
public:

    enum AXIS
    {
        X = 0,
        Y = 1,
        Z = 2,
        NUM_AXES = 3
    };

    GLVertex();

    GLVertex(const GLVertex& vert);

    GLVertex(GLfloat x, GLfloat y, GLfloat z);

    GLVertex(glm::vec3 position);

    GLVertex(std::initializer_list<GLfloat> lst);

    GLVertex copy();

    glm::vec3& getPosition();

    GLVertex& setPosition(glm::vec3 position);

    GLVertex& setNormal(glm::vec3 norm);

    GLVertex& print();

    glm::vec3& getNormal();

    GLVertex& setTex0(glm::vec2 tex0);

    glm::vec2 getTex0();

    std::vector<GLfloat>& appendVertexData(std::vector<GLfloat>& accum, int offset);

    static int size()
    {
        return 8;
    }

private:

    glm::vec3 m_position;

    glm::vec3 m_normal;

    glm::vec2 m_texcoord0;
};

#endif // GLVERTEX_H
