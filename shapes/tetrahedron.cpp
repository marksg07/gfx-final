#include "tetrahedron.h"

Tetrahedron::Tetrahedron(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4,
            glm::vec3 n1, glm::vec3 n2, glm::vec3 n3, glm::vec3 n4) {
    m_vertexData.reserve(6 * 6);
    // to get all the faces we need to repeat
    glm::vec3 points[6] = {p1, p2, p3, p4, p1, p2};
    glm::vec3 norms[6] = {n1, n2, n3, n4, n1, n2};
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 3; j++) {
            m_vertexData.push_back(points[i][j]);
        }
        for(int j = 0; j < 3; j++) {
            m_vertexData.push_back(norms[i][j]);
        }
    }
}

// I could compute actual norms here, not necessary though
Tetrahedron::Tetrahedron(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) :
    Tetrahedron(p1, p2, p3, p4, glm::vec3(), glm::vec3(), glm::vec3(), glm::vec3())
{

}

void Tetrahedron::buildShape() {
    this->m_vertexData.clear();
    // lazy reconstruct lol
    this->m_vertexData = std::vector<float>(Tetrahedron(glm::vec3(0, 0.5, 0),
                            glm::vec3(-0.5, -0.5, 0.5),
                            glm::vec3(0.5, -0.5, 0.5),
                            glm::vec3(0, -0.5, -0.5)).m_vertexData);
}
