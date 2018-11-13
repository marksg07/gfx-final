#include "shapes/GLCube.h"
#include "gl/datatype/VBOAttribMarker.h"
#include "gl/shaders/ShaderAttribLocations.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "GLVertex.h"
#include "GLTriangle.h"
#include "Utils.h"

GLCube::GLCube(int t1, int t2, float t3)
    : GLShape(t1, t2, t3)
{


    std::vector<GLTriangle> triangles;


    // Create a single cube face.
    std::vector<GLTriangle> face;
    float t_d = 1.0 / t1;

    for(int r = 0; r < t1; r++)
    {
        float start_y = .5 - (t_d * r);
        float end_y = start_y - t_d;

        for(int c = 0; c < t1; c++)
        {
            float start_x = -.5 + (t_d * c);
            float end_x = start_x + t_d;


            // Face one
            GLTriangle t1(
            {start_x, start_y,.5},
            {start_x, end_y, .5},
            {end_x, start_y, .5});

            GLTriangle t2(
            {end_x, start_y,.5},
            {start_x, end_y, .5},
            {end_x, end_y, .5});


            face.push_back(t1);
            face.push_back(t2);
        }
    }

    // Rotate to create the 4 faces that can be created by rotating the given face around the y axis.
    GLTriangle::rotateAccum(face, glm::vec3(1, 0, 0), M_PI / 2, 4, triangles);

    // Rotate twice to create the last two faces.
    std::vector<GLTriangle> rotatedFace;
    GLTriangle::rotateAccum(face, glm::vec3(0, 1, 0), M_PI, 2, rotatedFace, M_PI / 2);
    GLTriangle::rotateAccum(rotatedFace, glm::vec3(1, 0, 0), M_PI / 2, 1, triangles);


    std::vector<GLfloat> coordinates = GLTriangle::buildVertexData(triangles);

    setVertexData(&coordinates[0], coordinates.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, coordinates.size() / GLVertex::size());
    setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::NORMAL, 3, 12, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::TEXCOORD0, 2, 24, VBOAttribMarker::DATA_TYPE::FLOAT, false);

    buildVAO();
}
