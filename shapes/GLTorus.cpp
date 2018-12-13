#include "GLTorus.h"
#include "GLTriangle.h"
#include "gl/shaders/ShaderAttribLocations.h"
#include "Utils.h"


#include <functional>

GLTorus::GLTorus(int t1, int t2, float t3)
    : GLShape(t1, t2, t3)
{
    if (t1 < 3)
    {
        m_t1 = t1 = 3;
    }

    if (t2 < 3)
    {
        m_t2 = t2 = 3;
    }

    float R = .5;
    float r = R * (t3 / 100.0);

    R = .5 - ((t3 - 50) / 100.0);
    r = R + ((t3 - 50) / 50.0);

    std::function<glm::vec3(float, float)> pos = [R, r](float t, float u)
    {
        return glm::vec3(cos(t) * (R + (r * cos(u))), sin(t) * (R + (r * cos(u))), r * sin(u));
    };

    std::function<glm::vec3(float, float)> norm = [R, r](float t, float u)
    {
        return glm::vec3(cos(t) * (R + r), sin(t) * (R + r), r * sin(u));
    };


    std::vector<GLTriangle> triangles;
    triangles.reserve(t1 * t2 * 2);

    float t_step = 2 * M_PI / t1;
    float u_step = 2 * M_PI / t2;

    for(int i = 0; i < t1; i++)
    {
        for(int j = 0; j < t2; j++)
        {
            GLTriangle ta(pos(i * t_step, j * u_step), pos((i + 1) * t_step, j * u_step), pos((i + 1) * t_step, (j + 1) * u_step));



            GLTriangle tb(pos(i * t_step, j * u_step), pos((i + 1) * t_step, (j + 1) * u_step),  pos(i * t_step, (j + 1) * u_step));

            /*ta.setNormal(glm::vec3(0, 0, 0));
            tb.setNormal(glm::vec3(0, 0, 0));

            ta.a().setNormal(glm::normalize(norm(i * t_step, j * u_step) - ta.a().getPosition()));
            ta.b().setNormal(glm::normalize(norm((i + 1) * t_step, j * u_step) - ta.b().getPosition()));
            ta.c().setNormal(glm::normalize(norm((i + 1) * t_step, (j + 1) * u_step) - ta.c().getPosition()));*/

            triangles.push_back(ta);
            triangles.push_back(tb);
        }
    }

    std::vector<GLfloat> coordinates = GLTriangle::buildVertexData(triangles);


    setVertexData(&coordinates[0], coordinates.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, coordinates.size() / GLVertex::size());
    setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::NORMAL, 3, 12, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::TEXCOORD0, 2, 24, VBOAttribMarker::DATA_TYPE::FLOAT, false);

    buildVAO();
}
