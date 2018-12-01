//
// Created by gmarks on 9/30/18.
//

#ifndef PROJECTS_CUBE_H
#define PROJECTS_CUBE_H
#include "Shape.h"
#include "SelectableShape.h"

class Cube : public SelectableShape {
 public:
    Cube(int a, int b, float c) : SelectableShape(a, b, c) {};
    void buildShape();
 private:
    int drawFace(int offset, glm::mat4 trans, glm::vec3 normal);
    std::vector<glm::vec4> makeGrid(glm::mat4 trans);
};

#endif //PROJECTS_CUBE_H
