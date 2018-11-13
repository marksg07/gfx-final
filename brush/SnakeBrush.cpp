#include "SnakeBrush.h"
#include <iostream>

SnakeBrush::SnakeBrush(BGRA color, int radius)
    : Brush(color, radius)
{
}

SnakeBrush::~SnakeBrush()
{
}


void SnakeBrush::makeMask()
{
}

void SnakeBrush::brushDown(int x, int y, Canvas2D* canvas)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    canvas->getSnakeManager()->addSnake(x, y, m_color);
}

void SnakeBrush::brushDragged(int x, int y, Canvas2D* canvas)
{
}

void SnakeBrush::brushUp(int x, int y, Canvas2D* canvas)
{
}

void SnakeBrush::placePaint(int x, int y, Canvas2D* canvas)
{
}
