#ifndef FILTERSHARPEN_H
#define FILTERSHARPEN_H

#include "Filter.h"
#include <vector>

class FilterSharpen : public Filter
{
public:
    FilterSharpen();

    void apply(Canvas2D* canvas, QPoint start, QPoint end) override;

protected:

    float m_radius;

    std::vector<float> m_mask1;
    std::vector<float> m_mask2;
};

#endif // FILTERSHARPEN_H
