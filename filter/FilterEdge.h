#ifndef FILTEREDGE_H
#define FILTEREDGE_H

#include "Filter.h"

class FilterEdge : public Filter
{
public:
    FilterEdge();

    void apply(Canvas2D* canvas, QPoint start, QPoint end) override;

protected:

    float m_sensitivity;
};

#endif // FILTEREDGE_H
