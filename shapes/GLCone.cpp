#include "GLCone.h"
#include "GLTriangle.h"
#include "GLCap.h"
#include "Utils.h"

GLCone::GLCone(int t1, int t2, float t3)
    : GLShape(t1, t2, t3)
{
    if (t2 < 3)
    {
        m_t2 = t2 = 3;
    }

    std::vector<GLTriangle> triangles;

    // This one is easily the messiest of them all, but it works...

    // Compute one face, then translate and rotate to form the others
    std::vector<GLTriangle> face;
    float t = .5 / t1;

    std::function<float(float)> y = [](float x)
    {
        return -2*x + (1.0/2);
    };


    std::function<float(float)> x = [=](float n)
    {
        return -n * cos((M_PI / 2) + (2 * M_PI / t2));
    };

    std::function<float(float)> z = [=](float n)
    {
        return n * sin((M_PI / 2) + (2 * M_PI / t2));
    };


    // Create a segment (such that one leg of the segment is on the x axis) of the body of the cone:

    // Create the top triangle of the sgements
    GLTriangle top(
        {x(0), y(0), 0},
        {x(0), y(t), t},
        {x(t), y(t), z(t)}
    );

    // Compute the normal for the top vertex of the cone segment
    glm::vec3 top_norm = glm::normalize(glm::vec3(cos((M_PI / 2) - (2 * M_PI / (2 * t2))), 0, sin((M_PI / 2) - (2 * M_PI / (2 * t2)))));
    top_norm.y = 1.0/2;
    top_norm = glm::normalize(top_norm);


    // Computer the normals for the other two points on the top triangle.
    // Note these will also be the normals for all other parts of the
    glm::vec3 norm = glm::normalize(glm::vec3(0, 1.0/2, 1.0));
    glm::mat4 m = glm::rotate((float) (2 * M_PI / t2), glm::vec3(0, 1, 0));
    glm::vec3 norm2 = glm::vec3(m * glm::vec4(norm, 0));


    top.a().setNormal(top_norm);
    top.b().setNormal(norm);
    top.c().setNormal(norm2);

    face.push_back(top);

    for(int i = 1; i < m_t1; i++)
    {
        float p = i * t;

        GLTriangle t1(
            GLVertex({0, y(p), p}),
            GLVertex({0, y(p + t), t + p}),
            GLVertex({x(p), y(p), z(p)}));

        GLTriangle t2(
            GLVertex({0, y(p + t), t + p}),
            GLVertex({x(p + t), y(p + t), z(t + p)}),
            GLVertex({x(p), y(p), z(p)}));

        t1.a().setNormal(norm);
        t1.b().setNormal(norm);
        t1.c().setNormal(norm2);

        t2.a().setNormal(norm);
        t2.b().setNormal(norm2);
        t2.c().setNormal(norm2);


        face.push_back(t1);
        face.push_back(t2);
    }

    // Rotate the body segment of the cone around the y axis to create the body of the cone.
    GLTriangle::rotateAccum(face, glm::vec3(0, 1, 0), (2 * M_PI) / t2, t2, triangles);


    /*GLTriangle flat_top(
        {x(0), -.5, 0},
        {x(0), -.5, t},
        {x(t), -.5, z(t)}
    );
    flat_top.setNormal(glm::vec3(0,-1,0));
    base.push_back(flat_top);

    for(int i = 1; i < m_t1; i++)
    {
        float p = i * t;

        GLTriangle t1(
            GLVertex({0, -.5, p}),
            GLVertex({0, -.5, t + p}),
            GLVertex({x(p), -.5, z(p)}));

        GLTriangle t2(
            GLVertex({0, -.5, t + p}),
            GLVertex({x(p + t), -.5, z(t + p)}),
            GLVertex({x(p), -.5, z(p)}));

        // The normals of the base always point down.
        t1.setNormal(glm::vec3(0, -1, 0));
        t2.setNormal(glm::vec3(0, -1, 0));

        base.push_back(t1);
        base.push_back(t2);
    }


    // Create a reflection of the base segment
    for(size_t i = 0; i < base.size(); i++)
    {
        base[i].reflect(glm::vec3(1, 0, 0));
    }*/


    // Create a segment of the base.
    GLCap cap(t1, t2);

    // Reflect, Translate, and update the normals of the bottom cap
    cap.reflect(glm::vec3(1, 0, 0))
       .setNormal(glm::vec3(0, -1, 0))
       .translate(glm::vec3(0, -1, 0))
       .appendTriangleData(triangles);


    // Rotate the base segment around the y axis to create the bottom cap.
    //GLTriangle::rotateAccum(base, glm::vec3(0, 1, 0), (2 * M_PI) / t2, t2, triangles);

    std::vector<GLfloat> coordinates = GLTriangle::buildVertexData(triangles);


    setVertexData(&coordinates[0], coordinates.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, coordinates.size() / GLVertex::size());
    setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::NORMAL, 3, 12, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::TEXCOORD0, 2, 24, VBOAttribMarker::DATA_TYPE::FLOAT, false);

    buildVAO();
}
