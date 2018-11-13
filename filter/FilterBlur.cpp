#include "FilterBlur.h"
#include "Settings.h"
#include "Kernel.h"
#include <cmath>
#include <iostream>
#include <vector>

inline float g(int i, float mean, float stddev)
{
    const float var = (stddev * stddev);

    float a = 1.0 / (sqrt(2 * M_PI * var));

    float b = exp(-(i * i) / (2 * var));

    return a * b;
}


FilterBlur::FilterBlur()
{
    m_radius = settings.blurRadius;

    size_t dim = (2 * m_radius) + 1;
    m_mask.resize(dim);

    float stddev = m_radius / 3.0;
    m_sum = 0;

    int idx = 0;
    for(int i = -m_radius; i <= m_radius; i++)
    {
        float x = g(i, 0, stddev);
        m_sum += x;
        m_mask[idx++] = x;
    }

    for(size_t i = 0; i < dim; i++)
    {
        m_mask[i] = m_mask[i] / m_sum;
    }
}

void FilterBlur::apply(Canvas2D* canvas, QPoint start, QPoint end)
{
    Kernel k1(m_mask, Kernel:: ROW);
    Kernel k2(m_mask, Kernel::COL);

    KernelCtx ctx(canvas, start, end);

    ctx.apply(k1).apply(k2);

    FilterUtils::toCanvas(canvas, start, end, ctx.get());
}
