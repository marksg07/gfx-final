//
// Created by gmarks on 9/29/18.
//

#ifndef SHAPE_H
#define SHAPE_H
#include <vector>
#include "GL/glew.h"
#include "OpenGLScene.h"
#include "openglshape.h"
#include "gl/datatype/VBO.h"
#include "gl/shaders/ShaderAttribLocations.h"

class Shape {
 public:
    std::vector<GLfloat> getVertexData() { return m_vertexData; }
    void setVertexData(std::vector<GLfloat> vd) { m_vertexData = vd; }
    void buildShape() {}
    std::unique_ptr<OpenGLShape> getOpenGLShape();
    void fillNormals(glm::vec3 n);
    void fillNormals(glm::vec3 n, int start, int end);
 protected:
    std::vector<GLfloat> m_vertexData;
};

#endif //SHAPE_H
