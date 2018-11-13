/**
 * @file   Brush.cpp
 *
 * Implementation of common functionality of bitmap brushes.
 *
 * You should fill this file in while scompleting the Brush assignment.
 */

#include <iostream>
#include <cmath>
#include <QMutex>
#include "Brush.h"
#include "Settings.h"
#include "Canvas2D.h"

static unsigned char lerp(unsigned char a, unsigned char b, float percent) {
    float fa = static_cast<float>(a) / 255.0f;
    float fb = static_cast<float>(b) / 255.0f;
    return static_cast<unsigned char>((fa + (fb - fa) * percent) * 255.0f + 0.5f);
}

Brush::Brush(BGRA color, int radius) :
    // Pro-tip: Initialize all variables in the initialization list
    m_color(color),
    m_radius(radius)
{
    // Pro-tip: By the time you get to the constructor body, all of the
    // member variables have already been initialized.
}


Brush::~Brush()
{
    // Pro-tip: The destructor (here) is where you free all the resources
    // you allocated during the lifetime of the class
    // Pro-tip: If you use std::unique_ptr or std::vector, you probabably
    // won't need to put anything in here.
}

int Brush::getAlpha() const {
    return m_color.a;
}

BGRA Brush::getBGRA() const {
    return m_color;
}

int Brush::getBlue() const {
    return m_color.b;
}

int Brush::getGreen() const {
    return m_color.g;
}

int Brush::getRadius() const {
    return m_radius;
}

int Brush::getRed() const {
    return m_color.r;
}

void Brush::setAlpha(int alpha) {
    m_color.a = alpha;
}

void Brush::setBGRA(const BGRA &bgra) {
    m_color = bgra;
}

void Brush::setBlue(int blue) {
    m_color.b = blue;
}

void Brush::setGreen(int green) {
    m_color.g = green;
}

void Brush::setRed(int red) {
    m_color.r = red;
}

void Brush::setRadius(int radius) {
    m_radius = radius;
    makeMask();
}

BGRA Brush::blendColor(BGRA canvas, BGRA brush, float mask)
{
    BGRA out;

    /*out.r = (brush.r * mask) + (canvas.r * (1 - mask));
    out.g = (brush.g * mask) + (canvas.g * (1 - mask));
    out.b = (brush.b * mask) + (canvas.b * (1 - mask));*/


    out.r = lerp(canvas.r, brush.r, mask);
    out.g = lerp(canvas.g, brush.g, mask);
    out.b = lerp(canvas.b, brush.b, mask);

    return out;
}

int Brush::calcRadius(int x, int y, int c, int r)
{
    double x_diff = (x - c) * (x - c);
    double y_diff = (y - r) * (y - r);

    return (int) round(sqrt(x_diff + y_diff));
}

void Brush::brushDragged(int mouseX, int mouseY, Canvas2D* canvas) {
    // @TODO: [BRUSH] You can do any painting on the canvas here. Or, you can
    //        override this method in a subclass and do the painting there.
    //
    // Example: You'll want to delete or comment out this code, which
    // sets all the pixels on the canvas to red.
    //
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    /*BGRA *pix = canvas->data();
    int size = canvas->width() * canvas->height();
    for (int i = 0; i < size; i++) {
        pix[i] = BGRA(255, 0, 0, 255);
    }
    canvas->update();*/

    placePaint(mouseX, mouseY, canvas);
}

void Brush::placePaint(int mouseX, int mouseY, Canvas2D* canvas)
{
    BGRA* pix = canvas->data();

    int rowstart = std::max(0, mouseY - m_radius);
    int rowend = std::min(canvas->height(), mouseY + m_radius + 1);
    int colstart = std::max(0, mouseX - m_radius);
    int colend = std::min(canvas->width(), mouseX + m_radius + 1);

    int c_start = (mouseX - m_radius);
    int r_start = (mouseY - m_radius);
    int mask_w = (2 * m_radius) + 1;

    for(int r = rowstart; r < rowend; r++)
    {
        for(int c = colstart; c < colend; c++)
        {
            int m_idx = ((r - r_start) * mask_w) + (c - c_start);

            int i = r * canvas->width() + c;

            if (settings.fixAlphaBlending)
            {
                // If we clicked on this pixel, we'd get this color. Call this the `ideal` pixel.
                // By blending with this ideal pixel the color on our canvas will approach the ideal,
                // but it will never exccede it. This allows us to draw over the paint we have already
                // drawn, without the alpha blending issue.
                BGRA ideal = blendColor(m_old[i], m_color, m_color.a / 255.0);

                pix[i] = blendColor(pix[i], ideal, m_mask[m_idx]);

                continue;
            }

            pix[i] = blendColor(pix[i], m_color, (m_color.a / 255.0) * m_mask[m_idx]);
        }
    }
    canvas->update();
}

void Brush::brushDown(int x, int y, Canvas2D *canvas)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    if (settings.fixAlphaBlending)
    {
        int i = canvas->width() * canvas->height();

        m_old = new BGRA[i];
        memcpy(m_old, canvas->data(), sizeof(BGRA) * i);
    }
}

void Brush::brushUp(int x, int y, Canvas2D *canvas)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    if (settings.fixAlphaBlending)
    {
        delete[] m_old;
    }
}
