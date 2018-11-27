#include "IntersectionManager.h"

glm::vec4 IntersectionManager::intersectCap(glm::vec4 p, glm::vec4 d, float y, float rad, CS123Renderable* renderable)
{
    float t = (y - p.y) / d.y;

    glm::vec4 pos = r(p, d, t);

    // Check if the xz-vector is in the bounds of the circle.
    if (glm::length(pos.xz()) <= rad)
    {
        return pos;
    }

    return NO_INTERSECTION;
}

glm::vec4 IntersectionManager::intersectCylinderBody(glm::vec4 p, glm::vec4 d, float radius, float max_y, CS123Renderable* renderable)
{
    glm::vec4 ds = d * d;
    glm::vec4 ps = p * p;

    float a = ds.x + ds.z;
    float b = (2 * p.x *  d.x) + (2 * p.z * d.z);
    float c = ps.x + ps.z - square(radius);

    glm::vec2 sol = quadraticSolver(a, b, c);

    glm::vec4 min_i = NO_INTERSECTION;

    if (sol.x > 0)
    {
        glm::vec4 i1 = r(p, d, sol.x);

        if (fabs(i1.y) < max_y)
        {
            min_i = i1;
        }
    }

    if (sol.y > 0)
    {
        glm::vec4 i2 = r(p, d, sol.y);
        if (fabs(i2.y) < max_y)
        {
            if (i2.w < min_i.w)
            {
                min_i = i2;
            }
        }
    }

    return min_i;
}


float IntersectionManager::findLinearIntersection(glm::vec4 p, glm::vec4 d, float val, Axis a)
{
    if (a == Axis::X)
    {
        return (val - p.x) / d.x;
    } else if (a == Axis::Y)
    {
        return (val - p.y) / d.y;
    }
    return (val - p.z) / d.z;
}


float IntersectionManager::intersectsCube(glm::vec4 p, glm::vec4 d)
{
    // Find intersections with every face of the cube
    std::vector<float> ts = {
        findLinearIntersection(p, d, .5, Axis::X),
        findLinearIntersection(p, d, -.5, Axis::X),
        findLinearIntersection(p, d, .5, Axis::Y),
        findLinearIntersection(p, d, -.5, Axis::Y),
        findLinearIntersection(p, d, .5, Axis::Z),
        findLinearIntersection(p, d, -.5, Axis::Z)
    };

    // Find the smallest valid t.
    glm::vec4 i = NO_INTERSECTION;
    for(float t : ts)
    {
        if (t < 0 || t > i.w || t == INFINITY)
        {
            continue;
        }

        glm::vec4 v = r(p, d, t);

        if (fabs(v.x) <= .5 + EPSILON && fabs(v.y) <= .5 + EPSILON && fabs(v.z) <= .5 + EPSILON)
        {
            i = v;
        }
    }

    return i.w;
}


IlluminateData IntersectionManager::intersectCube(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable)
{
    // Find intersections with every face of the cube
    std::vector<float> ts = {
        findLinearIntersection(p, d, .5, Axis::X),
        findLinearIntersection(p, d, -.5, Axis::X),
        findLinearIntersection(p, d, .5, Axis::Y),
        findLinearIntersection(p, d, -.5, Axis::Y),
        findLinearIntersection(p, d, .5, Axis::Z),
        findLinearIntersection(p, d, -.5, Axis::Z)
    };

    // Find the smallest valid t.
    glm::vec4 i = NO_INTERSECTION;
    for(float t : ts)
    {
        if (t < 0 || t > i.w || t == INFINITY)
        {
            continue;
        }

        glm::vec4 v = r(p, d, t);

        if (fabs(v.x) <= .5 + EPSILON && fabs(v.y) <= .5 + EPSILON && fabs(v.z) <= .5 + EPSILON)
        {
            i = v;
        }
    }

    return IlluminateData(i, renderable);
}

IlluminateData IntersectionManager::intersectSphere(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable)
{
    glm::vec4 ds = d * d;
    glm::vec4 ps = p * p;

    float a = (ds.x + ds.y + ds.z);

    float b = 2 * (p.x * d.x + p.y * d.y + p.z * d.z);

    float c = ps.x + ps.y + ps.z - square(.5);

    glm::vec2 sol = quadraticSolver(a, b, c);

    glm::vec4 min_i = NO_INTERSECTION;

    if (sol.x > 0)
    {
        min_i = r(p, d, sol.x);
    }

    if (sol.y > 0 && sol.y < min_i.w)
    {
        min_i = r(p, d, sol.y);
    }

    return IlluminateData(min_i, renderable);
}

IlluminateData IntersectionManager::intersectCylinder(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable)
{
    glm::vec4 i1 = intersectCylinderBody(p, d, .5, .5, renderable);
    glm::vec4 i2 = intersectCap(p, d, .5, .5, renderable);
    glm::vec4 i3 = intersectCap(p, d, -.5, .5, renderable);

    glm::vec4 min_i = i1;

    if (i2.w < min_i.w)
    {
        min_i = i2;
    }

    if (i3.w < min_i.w)
    {
        min_i = i3;
    }

    return IlluminateData(min_i, renderable);
}


IlluminateData IntersectionManager::intersectCone(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable)
{
    glm::vec4 ds = d * d;
    glm::vec4 ps = p * p;
    // Cone body
    float a = ds.x + ds.z - (ds.y / 4);
    float b = (2 * p.x * d.x) + (2 * p.z * d.z) - ((p.y * d.y) / 2) + (d.y / 4);
    float c = ps.x + ps.z - (ps.y / 4) + (p.y / 4) - (1.0 / 16);

    glm::vec2 sol = quadraticSolver(a, b, c);


    glm::vec4 min_i = NO_INTERSECTION;

    if (sol.x > 0)
    {
        glm::vec4 i1 = r(p, d, sol.x);
        if (fabs(i1.y) <= .5)
        {
            min_i = i1;
        }
    }

    if (sol.y > 0)
    {
        glm::vec4 i2 = r(p, d, sol.y);
        if (fabs(i2.y) <= .5 && i2.w < min_i.w)
        {
            min_i = i2;
        }
    }

    // Cone base cap
    glm::vec4 i3 = intersectCap(p, d, -.5, .5, renderable);

    if (i3.w < min_i.w)
    {
        min_i = i3;
    }

    return IlluminateData(min_i, renderable);
}


IlluminateData IntersectionManager::intersect(glm::vec4 p, glm::vec4 d, CS123Renderable* renderable)
{
    p = renderable->inv_transform * p;
    d = renderable->inv_transform * d;

    PrimitiveType type = renderable->primitive.type;

    if (type == PrimitiveType::PRIMITIVE_CONE)
    {
        return intersectCone(p, d, renderable);
    } else if (type == PrimitiveType::PRIMITIVE_CYLINDER)
    {
        return intersectCylinder(p, d, renderable);
    } else if (type == PrimitiveType::PRIMITIVE_SPHERE)
    {
        return intersectSphere(p, d, renderable);
    } else if (type == PrimitiveType::PRIMITIVE_CUBE) {
        return intersectCube(p, d, renderable);
    }

    return IlluminateData(NO_INTERSECTION, renderable);
}
