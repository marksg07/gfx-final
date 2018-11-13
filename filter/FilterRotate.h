#ifndef FILTERROTATE_H
#define FILTERROTATE_H

#include "Filter.h"

class FilterRotate : public Filter
{
public:
    FilterRotate();

     void apply(Canvas2D* canvas, QPoint start, QPoint end) override;
};

#endif // FILTERROTATE_H
