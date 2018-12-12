#include "GLCylinder.h"
#include "GLTriangle.h"
#include "Utils.h"
#include "GLCap.h"
#include <glm/gtx/rotate_vector.hpp>

GLCylinder::GLCylinder(int t1, int t2, float t3)
    : GLShape(t1, t2, t3)
{
    if (t2 < 3)
    {
        m_t2 = t2 = 3;
    }

    std::vector<GLTriangle> triangles;

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

    float theta_step = (2 * M_PI) / t2;

    GLCap cap(t1, t2);

    cap.appendTriangleData(triangles);

    cap.reflect(glm::vec3(1, 0, 0))
       .setNormal(glm::vec3(0, -1, 0))
       .translate(glm::vec3(0, -1, 0))
       .appendTriangleData(triangles);

    // BODY:

    // x and z positions for the body.
    std::function<float(float)> x1 = [=](float n)
    {
        return cos((M_PI / 2) + (2 * M_PI / t2));
    };

    std::function<float(float)> z1 = [=](float n)
    {
        return sin((M_PI / 2) + (2 * M_PI / t2));
    };

    // Panel:
    std::vector<GLTriangle> panel;

    float step = (1.0 / t1);
    for(int i = 0; i < t1; i++)
    {
        float p = i * step;

        GLTriangle t1(
                    GLVertex({0, .5f - p, .5}),
                    GLVertex({0, .5f - (p + step), .5f}),
                    GLVertex({x(.5), .5f - p, z(.5)}));

        GLTriangle t2(
                    GLVertex({x(.5f), .5f - p, z(.5f)}),
                    GLVertex({0, .5f - (p + step), .5f}),
                    GLVertex({x(.5f), .5f - (p + step), z(.5f)}));

        t1.setNormal(glm::vec3(0, 0, 0));
        t2.setNormal(glm::vec3(0, 0, 0));


        panel.push_back(t1);
        panel.push_back(t2);
    }

    for(size_t i = 0; i < panel.size(); i++)
    {
        // Note the &
        panel[i].forEach([](GLVertex& v) {
            v.setNormal(glm::normalize(glm::vec3(v.getPosition().x, 0, v.getPosition().z)));
        });
    }

    // Rotate the outer panel around the y axis.
    GLTriangle::rotateAccum(panel, glm::vec3(0, 1, 0), theta_step, t2, triangles);

    std::vector<GLfloat> coordinates = GLTriangle::buildVertexData(triangles);


    setVertexData(&coordinates[0], coordinates.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, coordinates.size() / GLVertex::size());
    setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::NORMAL, 3, 12, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::TEXCOORD0, 2, 24, VBOAttribMarker::DATA_TYPE::FLOAT, false);

    buildVAO();
}
