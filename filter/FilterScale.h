#ifndef FilterScale_H
#define FilterScale_H

#include "Filter.h"
#include <vector>

class FilterScale : public Filter
{
public:
    FilterScale();

    virtual void apply(Canvas2D* canvas, QPoint start, QPoint end) override;

protected:

    float m_xscale;
    float m_yscale;

    std::vector<float> m_xmask;
    std::vector<float> m_ymask;
};

#endif // FilterScale_H
