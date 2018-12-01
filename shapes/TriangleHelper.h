//
// Created by gmarks on 9/29/18.
//

#ifndef TriangleHelper_H
#define TriangleHelper_H
#include "Shape.h"
#include "GL/glew.h"
#include "OpenGLScene.h"
#include <vector>

class TriangleHelper : public Shape {
 public:
    TriangleHelper(int tFactor, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    TriangleHelper(int tFactor, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 norm);
    // setPoints doesn't have to realloc vector
    void setPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    void buildShape();
 private:
    glm::vec3 m_p1, m_p2, m_p3, m_norm;
    int m_tf;
};

#endif //TriangleHelper_H
