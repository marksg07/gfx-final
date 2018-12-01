//
// Created by gmarks on 9/29/18.
//

#include "TriangleHelper.h"
#include <math.h>
#include <vector>

// Let p1 be the "middle" point, so the tesselated vertices
// go on lines p1p2 and p1p3.
// Assume p1->p2->p3, so when we tesselate,
// we do the line on p1p2 first.

TriangleHelper::TriangleHelper(int tFactor, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3):
    m_p1(p1),
    m_p2(p2),
    m_p3(p3),
    m_tf(tFactor)
{
    m_norm = glm::cross(m_p2 - m_p1, m_p3 - m_p2);
    m_vertexData.resize((tFactor * 2 + 4) * 6);
    buildShape();
}

TriangleHelper::TriangleHelper(int tFactor, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 norm):
    m_p1(p1),
    m_p2(p2),
    m_p3(p3),
    m_tf(tFactor),
    m_norm(norm)
{
    m_vertexData.resize((tFactor * 2 + 4) * 6);
    buildShape();
}

void TriangleHelper::buildShape() {
    // Let p1 be the "middle" point, so the tesselated vertices
    // go on lines p1p2 and p1p3.
    // Assume p1->p2->p3, so when we tesselate,
    // we do the line on p1p2 first.
    // To do the triangle strip, go p1->p1p2 point->p1p3 point->
    // next p1p2 point...
    m_vertexData[0] = m_p1.x;
    m_vertexData[1] = m_p1.y;
    m_vertexData[2] = m_p1.z;

    m_vertexData[6] = m_p1.x;
    m_vertexData[7] = m_p1.y;
    m_vertexData[8] = m_p1.z;
    for(int i = 2; i < m_tf + 1; i++) {
        GLfloat lerpFactor = (i - 1) / (float)m_tf;
        glm::vec3 p1p2 = m_p2 * lerpFactor + m_p1 * (1 - lerpFactor);
        glm::vec3 p1p3 = m_p3 * lerpFactor + m_p1 * (1 - lerpFactor);
        m_vertexData[i * 12 - 12] = p1p2.x;
        m_vertexData[i * 12 - 11] = p1p2.y;
        m_vertexData[i * 12 - 10] = p1p2.z;
        m_vertexData[i * 12 - 6]     = p1p3.x;
        m_vertexData[i * 12 - 5] = p1p3.y;
        m_vertexData[i * 12 - 4] = p1p3.z;
    }
    m_vertexData[m_tf * 12] = m_p2.x;
    m_vertexData[m_tf * 12 + 1] = m_p2.y;
    m_vertexData[m_tf * 12 + 2] = m_p2.z;
    m_vertexData[m_tf * 12 + 6] = m_p3.x;
    m_vertexData[m_tf * 12 + 7] = m_p3.y;
    m_vertexData[m_tf * 12 + 8] = m_p3.z;
    m_vertexData[m_tf * 12 + 12] = m_p3.x;
    m_vertexData[m_tf * 12 + 13] = m_p3.y;
    m_vertexData[m_tf * 12 + 14] = m_p3.z;
    m_vertexData[m_tf * 12 + 18] = m_p3.x;
    m_vertexData[m_tf * 12 + 19] = m_p3.y;
    m_vertexData[m_tf * 12 + 20] = m_p3.z;
    fillNormals(m_norm);
    printf("norm.z is %f\n", m_norm.z);
}

void TriangleHelper::setPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    m_p1 = p1;
    m_p2 = p2;
    m_p3 = p3;
}
