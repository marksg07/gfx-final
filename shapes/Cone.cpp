#include "Cone.h"
#include "CircleHelper.h"
#include "SquareGrid.h"
#include "glm/gtx/transform.hpp"
Cone::Cone(int a, int b, GLfloat c) : SelectableShape(a,b,c) {}

void Cone::buildShape() {
    auto pointToConeBarrel = [&](glm::vec4 point) {
        glm::vec4 out;
        out.w = 1;
        out.y = -point.y;
        out.x = 0.5 * (point.y + 0.5) * sin(point.x * 2 * M_PI);
        out.z = 0.5 * (point.y + 0.5) * cos(point.x * 2 * M_PI);
        return out;
    };
    // slope of cone: y goes from -0.5 to 0.5 = 1, x goes from
    // 0 t 0.5 = 1/2
    // slope is -2., slope of norm is 1/2
    // so y^2 = |V| = |H|/2 = (x^2 + z^2) / 2
    // y pointing up, so positive
    auto barrelToNorm = [&](glm::vec4 point) {
        point.w = 0;
        point.y = glm::sqrt((point.x * point.x + point.z * point.z) / 2);
        return point;
    };
    SquareGrid circ = CircleHelper(0.5, m_tf2, m_tf1, glm::translate(glm::vec3(0, -0.5, 0)) * glm::rotate((GLfloat)M_PI, glm::vec3((m_tf2 + 1) % 2, 0, m_tf2 % 2))).asSquareGrid();
    SquareGrid barrel = SquareGrid(m_tf2, m_tf1, glm::mat4(), glm::vec3(0,0,0), pointToConeBarrel, barrelToNorm);
    // barrel norm postprocessing: points at the tip aren't given norms because their x and z = 0.
    std::vector<std::tuple<glm::vec4, glm::vec4>>& grid_ref = barrel.m_grid;
    // the problematic points have initial y = -0.5, so they are top row
    for(int i = 0; i < m_tf2 + 1; i++) {
        GLfloat fakeX = GLfloat(i) / m_tf2 - 0.5;
        glm::vec4 fakePoint(sin(fakeX * 2 * M_PI), 0, cos(fakeX * 2 * M_PI), 1);
        glm::vec4 fixedNorm = barrelToNorm(fakePoint);
        std::get<1>(grid_ref[i]) = fixedNorm;
    }
    circ.buildShape();
    barrel.buildShape();
    std::vector<GLfloat> circVD = circ.getVertexData();
    std::vector<GLfloat> barrelVD = barrel.getVertexData();
    m_vertexData.resize(circVD.size() + barrelVD.size() - 6);
    std::copy(barrelVD.begin() + 6, barrelVD.end(), m_vertexData.begin());
    std::copy(circVD.begin(), circVD.end(), m_vertexData.begin() + barrelVD.size() - 6);
}
