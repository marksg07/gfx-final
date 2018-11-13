#ifndef FILTER_H
#define FILTER_H

#include "Canvas2D.h"

class Canvas2D;

class Filter
{
public:

    virtual void apply(Canvas2D* canvas, QPoint start, QPoint end) = 0;

protected:
    Filter();
};

#endif // FILTER_H
