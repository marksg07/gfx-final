//
// Created by gmarks on 9/30/18.
//

#ifndef PROJECTS_SELECTABLESHAPE_H
#define PROJECTS_SELECTABLESHAPE_H
#include "Shape.h"

class SelectableShape : public Shape {
 public:
    SelectableShape() {}
    SelectableShape(int tf1, int tf2, GLfloat tf3) { m_tf1 = tf1; m_tf2 = tf2; m_tf3 = tf3; }
 protected:
    int m_tf1, m_tf2;
    GLfloat m_tf3;
};

#endif //PROJECTS_SELECTABLESHAPE_H
