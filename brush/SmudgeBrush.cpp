/**
 * @file   Brush.cpp
 *
 * Implementation of a brush that smudges the image on the canvas as the mouse is dragged.
 *
 * You should fill this file in for the Brush assignment.
 */

#include <iostream>
#include <cmath>
#include "SmudgeBrush.h"
#include "Canvas2D.h"


SmudgeBrush::SmudgeBrush(BGRA color, int radius)
    : Brush(color, radius)
{
    // @TODO: [BRUSH] Initialize any memory you are going to use here. Hint - you are going to
    //       need to store temporary image data in memory. Remember to use automatically managed memory!

    makeMask();
}


SmudgeBrush::~SmudgeBrush()
{
    // @TODO: [BRUSH] Be sure not to leak memory!  If you're using automatic memory management everywhere,
    //       this will be empty.
}

void SmudgeBrush::brushUp(int x, int y, Canvas2D* canvas)
{

}


//! create a mask with a distribution of your choice (probably want to use quadratic for best results)
void SmudgeBrush::makeMask() {
    // @TODO: [BRUSH] Set up the mask for your brush here. For this brush you will probably want
    //        to use a quadratic distribution for the best results. Linear or Gaussian would
    //        work too, however. Feel free to paste your code from the Linear or Quadratic brushes
    //        or modify the class inheritance to be able to take advantage of one of those class's
    //        existing implementations. The choice is yours!
    //

    // Reserve enough space
    m_buffer.reserve(pow((2 * m_radius) + 1, 2));

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
                m_mask.push_back(pow((m_radius - radius) / f_radius, 2));
            }
        }
    }
}

void SmudgeBrush::brushDown(int x, int y, Canvas2D *canvas) {
    pickUpPaint(x, y, canvas);
}

// Inline for speed
static inline bool outOfBounds(int x, int y, int w, int h)
{
    if (x < 0 || x >= w)
    {
        return true;
    }

    if (y < 0 || y >= h)
    {
        return true;
    }

    return false;
}

//! Picks up paint from the canvas before drawing begins.
void SmudgeBrush::pickUpPaint(int x, int y, Canvas2D* canvas) {
    int width = canvas->width();
    int height = canvas->height();

    // @TODO: [BRUSH] Perform the "pick up paint" step described in the assignment handout here.
    //        In other words, you should store the "paint" under the brush mask in a temporary
    //        buffer (which you'll also have to figure out where and how to allocate). Then,
    //        in the paintOnce() method, you'll paste down the paint that you picked up here.
    //

    m_buffer.clear();

    BGRA* pix = canvas->data();

    int rowstart = y - m_radius;
    int rowend = y + m_radius + 1;
    int colstart = x - m_radius;
    int colend = x + m_radius + 1;

    BGRA dfl = BGRA(0, 0, 0);

    for(int r = rowstart; r < rowend; r++)
    {
        for(int c = colstart; c < colend; c++)
        {
            // Pick up black paint if we are outside the bounds of the canvas.
            if (outOfBounds(c, r, width, height))
            {
                m_buffer.push_back(dfl);
                continue;
            }

            int i = (r * width) + c;
            m_buffer.push_back(pix[i]);
        }
    }
}

void SmudgeBrush::brushDragged(int mouseX, int mouseY, Canvas2D* canvas) {
    // @TODO: [BRUSH] Here, you'll need to paste down the paint you picked up in
    //        the previous method. Be sure to take the mask into account! You can
    //        ignore the alpha parameter, but you can also use it (smartly) if you
    //        would like to.

    // now pick up paint again...
    placePaint(mouseX, mouseY, canvas);

    pickUpPaint(mouseX, mouseY, canvas);

}
void SmudgeBrush::placePaint(int x, int y, Canvas2D *canvas)
{
    BGRA* pix = canvas->data();

    int rowstart = std::max(0, y - m_radius);
    int rowend = std::min(canvas->height(), y + m_radius + 1);
    int colstart = std::max(0, x - m_radius);
    int colend = std::min(canvas->width(), x + m_radius + 1);

    int c_start = (x - m_radius);
    int r_start = (y - m_radius);
    int mask_w = (2 * m_radius) + 1;

    for(int r = rowstart; r < rowend; r++)
    {
        for(int c = colstart; c < colend; c++)
        {
            int i = r * canvas->width() + c;

            int m_idx = ((r - r_start) * mask_w) + (c - c_start);

            pix[i] = blendColor(pix[i], m_buffer[m_idx], m_mask[m_idx]);
        }
    }
    canvas->update();

}


