#ifndef CONE_H
#define CONE_H
#include "SelectableShape.h"
#include "Shape.h"

class Cone : public SelectableShape
{
public:
    Cone(int, int, GLfloat);
    void buildShape();
};

#endif // CONE_H
