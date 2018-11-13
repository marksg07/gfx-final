#ifndef FilterUnsharp_H
#define FilterUnsharp_H

#include "Filter.h"
#include <vector>

class FilterUnsharp : public Filter
{
public:
    FilterUnsharp();

    void apply(Canvas2D* canvas, QPoint start, QPoint end) override;

protected:

    float m_radius;

    std::vector<float> m_mask1;
    std::vector<float> m_mask2;
};

#endif // FilterUnsharp_H
