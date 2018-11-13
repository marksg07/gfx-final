#ifndef FOODBRUSH_H
#define FOODBRUSH_H

#include "Canvas2D.h"
#include "Brush.h"

class FoodBrush : public Brush
{
public:
    FoodBrush(BGRA color, int radius);

    virtual ~FoodBrush();


protected:
    void makeMask(); // Constructs the mask for this brush.

    void brushDown(int x, int y, Canvas2D* canvas) override;
    void brushDragged(int x, int y, Canvas2D* canvas) override;
    void brushUp(int x, int y, Canvas2D* canvas) override;
    void placePaint(int x, int y, Canvas2D* canvas) override;

    int m_counter;
};

#endif // FOODBRUSH_H
