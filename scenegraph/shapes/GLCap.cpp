#include "GLCap.h"

GLCap::GLCap(int t1, int t2)
{
    std::vector<GLTriangle> cap_segment;
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

    float theta_step = (2 * M_PI) / t2;


    // Create the top triangle of a cap segment (wedge)
    GLTriangle top(
        {x(0), .5, 0},
        {x(0), .5, t},
        {x(t), .5, z(t)}
    );
    cap_segment.push_back(top);



    // Create the rest of the cap segment
    for(int i = 1; i < t1; i++)
    {
        float p = i * t;

        GLTriangle t1(
            GLVertex({0, .5, p}),
            GLVertex({0, .5, t + p}),
            GLVertex({x(p), .5, z(p)}));

        t1.setNormal(glm::vec3(0, 1, 0));

        GLTriangle t2(
            GLVertex({0, .5, t + p}),
            GLVertex({x(p + t), .5, z(t + p)}),
            GLVertex({x(p), .5, z(p)}));

        t2.setNormal(glm::vec3(0, 1, 0));

        cap_segment.push_back(t1);
        cap_segment.push_back(t2);
    }


    // Create the cap by rotating the cap segment.
    GLTriangle::rotateAccum(cap_segment, glm::vec3(0, 1, 0), theta_step, t2, m_cap);
}

std::vector<GLTriangle>& GLCap::getTriangles()
{
    return m_cap;
}

GLCap& GLCap::reflect(glm::vec3 axis)
{
    for(size_t i = 0; i < m_cap.size(); i++)
    {
        m_cap[i].reflect(axis);
    }

    return *this;
}

GLCap& GLCap::translate(glm::vec3 v)
{
    for(size_t i = 0; i < m_cap.size(); i++)
    {
        m_cap[i].translate(v);
    }

    return *this;
}

GLCap& GLCap::setNormal(glm::vec3 v)
{
    for(size_t i = 0; i < m_cap.size(); i++)
    {
        m_cap[i].setNormal(v);
    }

    return *this;
}

GLCap& GLCap::appendTriangleData(std::vector<GLTriangle>& accum)
{
    accum.reserve(accum.size() + m_cap.size());
    accum.insert(accum.end(), m_cap.begin(), m_cap.end());

    return *this;
}


GLCap& GLCap::rotate(float angle, glm::vec3 v)
{
    for(size_t i = 0; i < m_cap.size(); i++)
    {
        m_cap[i].rotate(angle, v);
    }

    return *this;
}

GLCap& GLCap::apply(glm::mat4 t)
{
    for(size_t i = 0; i < m_cap.size(); i++)
    {
        m_cap[i].apply(t);
    }

    return *this;
}

