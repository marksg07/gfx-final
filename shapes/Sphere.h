#ifndef SPHERE_H
#define SPHERE_H
#include "SelectableShape.h"

class Sphere : public SelectableShape
{
public:
    Sphere(int, int, GLfloat);
    void buildShape();
};

#endif // SPHERE_H
