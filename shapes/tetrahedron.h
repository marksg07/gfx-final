#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

#include <QMainWindow>
#include "Shape.h"
#include "SelectableShape.h"

class Tetrahedron : public Shape
{
public:
   //Tetrahedron(int a, int b, float c) : SelectableShape(a, b, c) {};
   Tetrahedron(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);
   Tetrahedron(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4,
               glm::vec3 n1, glm::vec3 n2, glm::vec3 n3, glm::vec3 n4);

   void buildShape();
private:

};

#endif // TETRAHEDRON_H
