//
// Created by gmarks on 9/29/18.
//

#ifndef CIRCLEHELPER_H
#define CIRCLEHELPER_H
#include "SquareGrid.h"
#include "Shape.h"
#include "GL/glew.h"
#include "OpenGLScene.h"

class CircleHelper : public Shape {
 public:
    CircleHelper(GLfloat radius, int npoints, int tf);
    CircleHelper(GLfloat radius, int npoints, int tf, glm::mat4 trans);
    //void setRadius(GLfloat m_radius);
    //void setNPoints(int npoints);
    //std::vector<glm::vec3> getOuterPoints();
    void buildShape();
    SquareGrid asSquareGrid();
 private:
    GLfloat m_radius;
    int m_npoints;
    int m_tf;
    glm::mat4 m_trans;
};

#endif //CIRCLEHELPER_H
