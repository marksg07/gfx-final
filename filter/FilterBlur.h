#ifndef FILTERBLUR_H
#define FILTERBLUR_H

#include "Filter.h"
#include <vector>

class FilterBlur : public Filter
{
public:
    FilterBlur();

    void apply(Canvas2D* canvas, QPoint start, QPoint end) override;

protected:

    std::vector<float> m_mask;

    float m_radius;
    float m_sum;
};

#endif // FILTERBLUR_H
