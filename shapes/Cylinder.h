//
// Created by gmarks on 9/30/18.
//

#ifndef PROJECTS_CYLINDER_H
#define PROJECTS_CYLINDER_H
#include "Shape.h"
#include "SelectableShape.h"

class Cylinder : public SelectableShape {
 public:
    Cylinder(int a, int b, GLfloat c) : SelectableShape(a, b, c) {}
    void buildShape();
 private:
};


#endif //PROJECTS_CYLINDER_H
