#ifndef INTERSECTIONMANAGER_H
#define INTERSECTIONMANAGER_H

#include "IlluminateData.h"
#include <memory>
#include "glm.hpp"

#define NO_INTERSECTION glm::vec4(0, 0, 0, INFINITY)


class IntersectionManager
{
public:

    IntersectionManager(std::vector<CS123Renderable>& renderables)
        : m_renderables(renderables)
    {
    }

    inline glm::vec4 r(glm::vec4 p, glm::vec4 d, float t)
    {
        return glm::vec4((p + (t * d)).xyz(), t);
    }


    float inline square(float x)
    {
        return x * x;
    }

    inline glm::vec2 quadraticSolver(float a, float b, float c)
    {
        glm::vec2 sol(-1, -1);

        float bsm4ac = square(b) - (4 * a * c);

        if (bsm4ac >= 0)
        {
            float t1 = (-b + sqrt(bsm4ac)) / (2 * a);
            float t2 = (-b - sqrt(bsm4ac)) / (2 * a);

            sol.x = t1;
            if (t1 != t2)
            {
                sol.y = t2;
            }
        }

        return sol;
    }


    glm::vec4 intersectCap(glm::vec4 p, glm::vec4 d, float y, float rad, CS123Renderable* renderable);

    glm::vec4 intersectCylinderBody(glm::vec4 p, glm::vec4 d, float radius, float max_y, CS123Renderable* renderable);

    float findLinearIntersection(glm::vec4 p, glm::vec4 d, float val, Axis a);

    IlluminateData appendValidIntersections(glm::vec4 p, glm::vec4 d, std::vector<float>& ts, CS123Renderable* renderable, std::function<bool(glm::vec4)> validator);

    IlluminateData intersectCube(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable);

    IlluminateData intersectSphere(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable);
    IlluminateData intersectCylinder(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable);

    IlluminateData intersectCone(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable);


    IlluminateData intersect(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable);

    std::vector<IlluminateData> getIntersections(glm::vec4 p, glm::vec4 d)
    {
        std::vector<IlluminateData> intersections;

        size_t len = m_renderables.size();
        for(size_t i = 0; i < len; i++)
        {
            IlluminateData ill = intersect(p, d, &m_renderables[i]);
            if (ill.getT() != INFINITY)
            {
                intersections.push_back(ill);
            }
        }

        return intersections;
    }

private:

    std::vector<CS123Renderable>& m_renderables;

};

#endif // INTERSECTIONMANAGER_H
