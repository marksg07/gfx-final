/**
 * @file   LinearBrush.cpp
 *
 * Implementation of a brush with a linear mask distribution.
 *
 * You should fill this file in while completing the Brush assignment.
 */

#include "LinearBrush.h"

#include <math.h>

LinearBrush::LinearBrush(BGRA color, int radius)
    : Brush(color, radius)
{
    // @TODO: [BRUSH] You'll probably want to set up the mask right away.
    makeMask();
}


LinearBrush::~LinearBrush()
{
}

void LinearBrush::makeMask()
{
    // @TODO: [BRUSH] Set up the mask for your Linear brush here...

    float f_radius = m_radius;

    for(int r = -m_radius; r <= m_radius; r++)
    {
        for(int c = -m_radius; c <= m_radius; c++)
        {
            int radius = calcRadius(0, 0, c, r);
            if (radius > m_radius)
            {
                m_mask.push_back(0);
            } else {
                m_mask.push_back(((m_radius - radius) / f_radius));
            }
        }
    }
}


