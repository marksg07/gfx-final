#include "Sphere.h"
#include "SquareGrid.h"

Sphere::Sphere(int p1, int p2, GLfloat p3) : SelectableShape(p1, p2, p3)
{
    m_tf1 = p1;
    m_tf2 = p2;
    m_tf3 = p3;
}

glm::vec4 mercatorProject(glm::vec4 v) {
    // we do the inverse mercator projection mapping for a sphere
    // to get long and lat
    GLfloat longitude = v.x * 2 * M_PI + M_PI;
    GLfloat latitude = -M_PI / 2 + 2 * glm::atan(glm::sinh(v.y * 2 * M_PI + M_PI));
    // now using this long and lat we map onto the sphere
    v.x = 0.5 * glm::cos(latitude) * glm::cos(longitude);
    v.y = 0.5 * glm::cos(latitude) * glm::sin(longitude);
    v.z = 0.5 * glm::sin(latitude);
    v.w = 1.f; // just to make sure
    return v;
}

glm::vec4 getNormal(glm::vec4 v) {
    v.w = 0.f;
    return v; // straight out from center of sphere
}

void Sphere::buildShape() {
    SquareGrid g(m_tf1, m_tf2, glm::mat4(), glm::vec3(1,0,0), mercatorProject, getNormal);
    g.buildShape();
    m_vertexData = g.getVertexData();
}
