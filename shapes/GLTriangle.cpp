#include "GLTriangle.h"

GLTriangle::GLTriangle(std::initializer_list<GLfloat> i1, std::initializer_list<GLfloat> i2, std::initializer_list<GLfloat> i3)
{
    GLVertex v1(i1);
    GLVertex v2(i2);
    GLVertex v3(i3);


    v1.setNormal(glm::normalize(glm::cross(v2.getPosition() - v1.getPosition(), v3.getPosition() - v1.getPosition())));
    v2.setNormal(glm::normalize(-glm::cross(v1.getPosition() - v2.getPosition(), v3.getPosition() - v2.getPosition())));
    v3.setNormal(glm::normalize(glm::cross(v1.getPosition() - v3.getPosition(), v2.getPosition() - v3.getPosition())));


    m_verts.push_back(v1);
    m_verts.push_back(v2);
    m_verts.push_back(v3);
}

GLTriangle::GLTriangle(GLVertex v1, GLVertex v2, GLVertex v3)
{
    m_verts.push_back(v1);
    m_verts.push_back(v2);
    m_verts.push_back(v3);
}

GLTriangle::GLTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    m_verts.push_back(GLVertex(v1));
    m_verts.push_back(GLVertex(v2));
    m_verts.push_back(GLVertex(v3));
}


GLTriangle::GLTriangle(const GLTriangle& triangle)
    : m_verts(triangle.m_verts)
{
}

GLTriangle& GLTriangle::forEach(std::function<void(GLVertex&)> func)
{
    for(int i = 0; i < 3; i++)
    {
        func(m_verts[i]);
    }

    return *this;
}

GLTriangle& GLTriangle::operator=(const GLTriangle& rhs) &
{
    m_verts = rhs.m_verts;

    return *this;
}

GLTriangle& GLTriangle::setNormal(glm::vec3 normal)
{
    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setNormal(normal);
    }

    return *this;
}

GLVertex& GLTriangle::a()
{
    return m_verts[0];
}

GLVertex& GLTriangle::b()
{
    return m_verts[1];
}


GLVertex& GLTriangle::c()
{
    return m_verts[2];
}


GLTriangle GLTriangle::copy()
{
    return GLTriangle(*this);
}

GLTriangle& GLTriangle::reflect(glm::vec3 axis)
{
    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setPosition(glm::reflect(m_verts[i].getPosition(), axis));
        m_verts[i].setNormal(glm::reflect(m_verts[i].getNormal(), axis));
    }

    return *this;
}

GLTriangle& GLTriangle::reflectNormal(glm::vec3 axis)
{
    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setNormal(glm::reflect(m_verts[i].getNormal(), axis));
    }

    return *this;
}

GLTriangle& GLTriangle::setZ(GLfloat z)
{
    for(int i = 0; i < 3; i++)
    {
        m_verts[i].getPosition().z = z;
    }

    return *this;
}

GLTriangle& GLTriangle::translate(glm::vec3 v)
{
    glm::mat4 t = glm::translate(v);

    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setPosition(glm::vec3(t * glm::vec4(m_verts[i].getPosition(), 1)));
    }

    return *this;
}


GLTriangle& GLTriangle::rotate(float angle, glm::vec3 vec)
{
    glm::mat4 t = glm::rotate(angle, vec);

    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setPosition(glm::vec3(t * glm::vec4(m_verts[i].getPosition(), 1)));
        m_verts[i].setNormal(glm::vec3(t * glm::vec4(m_verts[i].getNormal(), 0)));
    }

    return *this;
}

GLTriangle& GLTriangle::apply(glm::mat4 t)
{
    for(int i = 0; i < 3; i++)
    {
        m_verts[i].setPosition(glm::vec3(t * glm::vec4(m_verts[i].getPosition(), 1)));
        m_verts[i].setNormal(glm::vec3(t * glm::vec4(m_verts[i].getNormal(), 0)));
    }

    return *this;
}

std::vector<GLTriangle>& GLTriangle::rotateAccum(const std::vector<GLTriangle>&in, glm::vec3 axis, float step, size_t steps, std::vector<GLTriangle>& out, float start)
{
    size_t offset = out.size();

    out.reserve(out.size() + (steps * in.size()));
    for(size_t i = 0; i <= steps; i++)
    {
        out.insert(out.end(), in.begin(), in.end());
    }

    for(size_t i = 0; i <= steps; i++)
    {
        glm::mat4 rot = glm::rotate(start + (i * step), axis);

        for(size_t j = 0; j < in.size(); j++)
        {
            out[offset + (i * in.size()) + j].apply(rot);
        }
    }

    return out;
}

std::vector<GLfloat> GLTriangle::buildVertexData(const std::vector<GLTriangle>& triangles)
{
   // std::cout << __PRETTY_FUNCTION__ << std::endl;

    std::vector<GLfloat> out;

    // Use reserve and offset for speed...
    out.resize(triangles.size() * 3 * GLVertex::size());

    int t = 0;
    for(auto triangle : triangles)
    {
        triangle.appendVertexData(out, t * 3 * GLVertex::size());

        t++;
    }

    return out;
}

std::vector<GLfloat>& GLTriangle::appendVertexData(std::vector<GLfloat>& accum, int i)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    int v = 0;
    for(auto vert : m_verts)
    {
        vert.appendVertexData(accum, i + (v * GLVertex::size()));

        v++;
    }

    return accum;
}
