#ifndef GLCAP_H
#define GLCAP_H

#include "GLTriangle.h"
#include "GLVertex.h"
#include "glm/gtx/transform.hpp"
#include <vector>
#include <functional>

class GLCap : public GLTransformable
{

public:
    GLCap(int t1, int t2);

    std::vector<GLTriangle>& getTriangles();

    GLCap& reflect(glm::vec3 axis) override;

    GLCap& translate(glm::vec3 v) override;

    GLCap& setNormal(glm::vec3 v);

    GLCap& rotate(float angle, glm::vec3 v) override;

    GLCap& apply(glm::mat4 t) override;

    GLCap& appendTriangleData(std::vector<GLTriangle>& accum);

private:

    std::vector<GLTriangle> m_cap;

};

#endif // GLCAP_H
