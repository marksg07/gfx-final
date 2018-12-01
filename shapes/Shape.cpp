//
// Created by gmarks on 9/30/18.
//

#include "Shape.h"
#include "gl/util/errorchecker.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#include <iterator>
#include <iostream>

std::unique_ptr<OpenGLShape> Shape::getOpenGLShape() {
    std::unique_ptr<OpenGLShape> shape = std::make_unique<OpenGLShape>();
    shape->setVertexData(m_vertexData.data(), m_vertexData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, m_vertexData.size() / 6);
    shape->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    shape->setAttribute(ShaderAttrib::NORMAL, 3, sizeof(GLfloat) * 3, VBOAttribMarker::DATA_TYPE::FLOAT, true);
    shape->buildVAO();
    return shape;
}

void Shape::fillNormals(glm::vec3 n) {
    for(int i = 3; i < m_vertexData.size(); i += 6) {
        m_vertexData[i] = n.x;
        m_vertexData[i + 1] = n.y;
        m_vertexData[i + 2] = n.z;
    }
}

void Shape::fillNormals(glm::vec3 n, int start, int end) {
    for(int i = start + 3; i < end; i += 6) {
        m_vertexData[i] = n.x;
        m_vertexData[i + 1] = n.y;
        m_vertexData[i + 2] = n.z;
    }
}
