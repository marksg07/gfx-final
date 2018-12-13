#ifndef GLTRIANGLE_H
#define GLTRIANGLE_H

#include "GLVertex.h"
#include "glm/gtx/transform.hpp"
#include <algorithm>
#include <functional>
#include "GLTransformable.h"

class GLTriangle : public GLTransformable
{
public:
    GLTriangle(std::initializer_list<GLfloat> i1, std::initializer_list<GLfloat> i2, std::initializer_list<GLfloat> i3);

    GLTriangle(GLVertex v1, GLVertex v2, GLVertex v3);
    GLTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);

    GLTriangle(const GLTriangle& triangle);

    GLTriangle& forEach(std::function<void(GLVertex&)> func);

    GLTriangle& operator=(const GLTriangle& rhs) &;
    GLTriangle(GLTriangle&&) = default;
    virtual ~GLTriangle() {}

    GLTriangle& setNormal(glm::vec3 normal);

    GLVertex& a();
    GLVertex& b();
    GLVertex& c();

    GLTriangle copy();

    GLTriangle& reflect(glm::vec3 axis);

    GLTriangle& reflectNormal(glm::vec3 axis);

    GLTriangle& setZ(GLfloat z);

    GLTriangle& translate(glm::vec3 v) override;

    GLTriangle& rotate(float angle, glm::vec3 vec) override;

    GLTriangle& apply(glm::mat4 t) override;

    static std::vector<GLTriangle>& rotateAccum(const std::vector<GLTriangle>&in, glm::vec3 axis, float step, size_t steps, std::vector<GLTriangle>& out, float start = 0);

    static std::vector<GLfloat> buildVertexData(const std::vector<GLTriangle>& triangles);

private:

    std::vector<GLfloat>& appendVertexData(std::vector<GLfloat>& accum, int offset);

    std::vector<GLVertex> m_verts;
};

#endif // GLTRIANGLE_H
