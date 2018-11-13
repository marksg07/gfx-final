#ifndef SNAKEBRUSH_H
#define SNAKEBRUSH_H

#include "Canvas2D.h"
#include "Brush.h"

class SnakeBrush : public Brush
{
public:
    SnakeBrush(BGRA color, int radius);

    virtual ~SnakeBrush();


protected:
    void makeMask(); // Constructs the mask for this brush.

    void brushDown(int x, int y, Canvas2D* canvas) override;
    void brushDragged(int x, int y, Canvas2D* canvas) override;
    void brushUp(int x, int y, Canvas2D* canvas) override;
    void placePaint(int x, int y, Canvas2D* canvas) override;
};

#endif // SNAKEBRUSH_H
