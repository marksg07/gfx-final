#ifndef RAY_H
#define RAY_H

#include "Canvas2D.h"
#include "glm.hpp"

class Ray
{
public:
    Ray(Canvas2D* canvas, size_t r, size_t c, glm::vec4 vec)
        : m_canvas(canvas), m_r(r), m_c(c), m_vec(vec)
    {
    }

private:

    Canvas2D* m_canvas;
    size_t m_r;
    size_t m_c;
    glm::vec4 m_vec;
};

#endif // RAY_H
