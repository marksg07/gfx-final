#include "FoodBrush.h"

#include "FoodBrush.h"
#include <iostream>

FoodBrush::FoodBrush(BGRA color, int radius)
    : Brush(color, radius), m_counter(0)
{
}

FoodBrush::~FoodBrush()
{
}


void FoodBrush::makeMask()
{
}

void FoodBrush::brushDown(int x, int y, Canvas2D* canvas)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    //canvas->getSnakeManager()->addFood(x, y);
    canvas->getSnakeManager()->addFood(x, y);
}

void FoodBrush::brushDragged(int x, int y, Canvas2D* canvas)
{
    if (m_counter > 10)
    {
        canvas->getSnakeManager()->addFood(x, y);
        m_counter = 0;
    } else {
        m_counter++;
    }
}

void FoodBrush::brushUp(int x, int y, Canvas2D* canvas)
{
}

void FoodBrush::placePaint(int x, int y, Canvas2D* canvas)
{
}
