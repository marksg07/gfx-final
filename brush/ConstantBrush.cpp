/**
 * @file   ConstantBrush.cpp
 *
 * Implementation of a brush with a constant mask distribution.
 *
 * You should fill this file in while completing the Brush assignment.
 */

#include <iostream>
#include "ConstantBrush.h"
#include "Settings.h"
#include "Canvas2D.h"

ConstantBrush::ConstantBrush(BGRA color, int radius)
    : Brush(color, radius)
{
    // @TODO: [BRUSH] You'll probably want to set up the mask right away.
    makeMask();
}


ConstantBrush::~ConstantBrush()
{
}

void ConstantBrush::makeMask()
{
    // @TODO: [BRUSH] Set up the mask for your Constant brush here...

    for(int r = -m_radius; r <= m_radius; r++)
    {
        for(int c = -m_radius; c <= m_radius; c++)
        {
            int radius = calcRadius(0, 0, c, r);
            if (radius > m_radius)
            {
                m_mask.push_back(0);
            } else {
                m_mask.push_back(1);
            }
        }
    }
}
