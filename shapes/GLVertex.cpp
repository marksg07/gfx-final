#include "GLVertex.h"

#include <algorithm>

GLVertex::GLVertex()
{
    m_position = glm::vec3(0, 0, 0);
    m_normal = glm::vec3(1, 0, 0);
    m_texcoord0 = glm::vec2(1.0, 0);
}

GLVertex::GLVertex(const GLVertex& vert)
    : m_position(vert.m_position),
      m_normal(vert.m_normal),
      m_texcoord0(vert.m_texcoord0)
{
    float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    m_texcoord0[0] = r1;
    m_texcoord0[1] = r2;
}

GLVertex::GLVertex(GLfloat x, GLfloat y, GLfloat z)
    : GLVertex()
{
    m_position = glm::vec3(x, y, z);
}

GLVertex::GLVertex(glm::vec3 position)
    : m_position(position),
      m_normal(glm::vec3(1, 0, 0)),
      m_texcoord0(glm::vec2(1.0, 0))
{
    float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    m_texcoord0[0] = r1;
    m_texcoord0[1] = r2;
}


GLVertex::GLVertex(std::initializer_list<GLfloat> lst)
    : GLVertex()
{
    std::vector<GLfloat> v = lst;

    while(v.size() < 3)
    {
        v.push_back(0);
    }

    m_position.x = v[0];
    m_position.y = v[1];
    m_position.z = v[2];

    float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    m_texcoord0[0] = r1;
    m_texcoord0[1] = r2;
}


GLVertex GLVertex::copy()
{
    return GLVertex(*this);
}

glm::vec3& GLVertex::getPosition()
{
    return m_position;
}

GLVertex& GLVertex::setPosition(glm::vec3 position)
{
    m_position = position;

    return *this;
}

GLVertex& GLVertex::setTex0(glm::vec2 tex0)
{
    m_texcoord0 = tex0;

    return *this;
}

glm::vec2 GLVertex::getTex0()
{
    return m_texcoord0;
}

GLVertex& GLVertex::setNormal(glm::vec3 norm)
{
    m_normal = norm;

    return *this;
}

GLVertex& GLVertex::print()
{
    std::cout << "(" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;

    return *this;
}

glm::vec3& GLVertex::getNormal()
{
    return m_normal;
}

std::vector<GLfloat>& GLVertex::appendVertexData(std::vector<GLfloat>& accum, int i)
{
    /*accum.push_back(m_position.x);
    accum.push_back(m_position.y);
    accum.push_back(m_position.z);

    accum.push_back(m_normal.x);
    accum.push_back(m_normal.y);
    accum.push_back(m_normal.z);

    accum.push_back(m_texcoord0.x);
    accum.push_back(m_texcoord0.y);*/

    accum[i] = m_position.x;
    accum[i + 1] = m_position.y;
    accum[i + 2] = m_position.z;

    accum[i + 3] = m_normal.x;
    accum[i + 4] =  m_normal.y;
    accum[i + 5] = m_normal.z;

    accum[i + 6] = m_texcoord0.x;
    accum[i + 7] = m_texcoord0.y;

 /*   accum[] = m_position.x;
    accum[i + 1] = m_position.y;
    accum[i + 2] = m_position.z;*/



    return accum;
}
