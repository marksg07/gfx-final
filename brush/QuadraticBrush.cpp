/**
 * @file   QuadraticBrush.cpp
 *
 * Implementation of a brush with a quadratic mask distribution.
 *
 * You should fill this file in while completing the Brush assignment.
 */

#include "QuadraticBrush.h"

#include <math.h>

QuadraticBrush::QuadraticBrush(BGRA color, int radius)
    : Brush(color, radius)
{
    // @TODO: [BRUSH] You'll probably want to set up the mask right away.
    makeMask();
}

QuadraticBrush::~QuadraticBrush()
{
}

void QuadraticBrush::makeMask() {
    // @TODO: [BRUSH] Set up the mask for your Quadratic brush here...

    float f_radius = m_radius;

    // ((R - x) / R)^2 which is quadratic (if you check desmos)
    // I liked the one above more tham 1 - (r / R)^2

    for(int r = -m_radius; r <= m_radius; r++)
    {
        for(int c = -m_radius; c <= m_radius; c++)
        {
            int radius = calcRadius(0, 0, c, r);
            if (radius > m_radius)
            {
                m_mask.push_back(0);
            } else {
                m_mask.push_back(pow((m_radius - radius) / f_radius, 2));
            }
        }
    }
}


