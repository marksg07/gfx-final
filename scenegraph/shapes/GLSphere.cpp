#include "GLSphere.h"
#include "GLTriangle.h"
#include "gl/shaders/ShaderAttribLocations.h"
#include "Utils.h"


GLSphere::GLSphere(int t1, int t2, float t3)
    : GLShape(t1, t2, t3)
{
    if (t1 < 2)
    {
        m_t1 = t1 = 2;
    }

    if (t2 < 3)
    {
        m_t2 = t2 = 3;
    }


    float r = .5;
    std::function<glm::vec3(float, float)> pos = [r](float p, float t)
    {
        float r_sin = r * sin(p);

        return glm::vec3(-r_sin * cos(t), r * cos(p), r_sin * sin(t));
    };


    std::vector<GLTriangle> v_slice;

    float phi_step = (1.0 / t1) * (M_PI);
    float theta_step = (2 * M_PI) / t2;

    GLTriangle top(
        pos(0, 0),
        pos(phi_step, M_PI),
        pos(phi_step, M_PI + theta_step)
    );

    GLTriangle bottom(
        pos(M_PI - phi_step, M_PI),
        pos(M_PI, 0),
        pos(M_PI - phi_step, M_PI + theta_step)
    );

    v_slice.push_back(top);
    v_slice.push_back(bottom);


    for(int i = 1; i < t1; i++)
    {
        GLTriangle tri1(
            pos(i * phi_step, M_PI),
            pos((i + 1) * phi_step, M_PI),
            pos((i + 1) * phi_step, M_PI + theta_step)
        );


        GLTriangle tri2(
            pos(i * phi_step, M_PI + theta_step),
            pos(i * phi_step, M_PI),
            pos((i + 1) * phi_step, M_PI + theta_step)
        );

        v_slice.push_back(tri1);
        v_slice.push_back(tri2);
    }


    std::vector<GLTriangle> triangles;


    // Set the normals of the the slice.
    for(size_t i = 0; i < v_slice.size(); i++)
    {
        v_slice[i].forEach([](GLVertex& v)
        {
           v.setNormal(glm::normalize(v.getPosition()));
        });
    }

    GLTriangle::rotateAccum(v_slice, glm::vec3(0, 1, 0), theta_step, t2, triangles);

    std::vector<GLfloat> sphereData = GLTriangle::buildVertexData(triangles);

    setVertexData(&sphereData[0], sphereData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, sphereData.size() / GLVertex::size());
    setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::NORMAL, 3, 12, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    setAttribute(ShaderAttrib::TEXCOORD0, 2, 24, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    buildVAO();
}
