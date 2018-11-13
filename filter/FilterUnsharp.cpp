#include "FilterUnsharp.h"

#include "FilterUtils.h"
#include "Kernel.h"
#include "Settings.h"
#include <iostream>
#include <cmath>

inline float g(int i, float mean, float stddev)
{
    const float var = (stddev * stddev);

    float a = 1.0 / (sqrt(2 * M_PI * var));

    float b = exp(-(i * i) / (2 * var));

    return a * b;
}

FilterUnsharp::FilterUnsharp()
{
    m_radius = settings.blurRadius;

    size_t dim = (2 * m_radius) + 1;
    m_mask1.resize(dim);
    m_mask2.resize(dim);

    float stddev = m_radius / 3.0;
    float sum = 0;

    int idx = 0;
    for(int i = -m_radius; i <= m_radius; i++)
    {
        float x = g(i, 0, stddev);
        sum += x;
        m_mask1[idx] = x;
        m_mask2[idx] = -x;

        idx++;
    }

    for(size_t i = 0; i < dim; i++)
    {
        m_mask1[i] = m_mask1[i] / sum;
        m_mask2[i] = m_mask2[i] / sum;
    }

}


void FilterUnsharp::apply(Canvas2D* canvas, QPoint start, QPoint end)
{

    std::vector<glm::vec3> data = FilterUtils::canvasToVec3(canvas, start, end);

    size_t w = FilterUtils::width(canvas, start, end);
    size_t h = FilterUtils::height(canvas, start, end);

    KernelCtx ctx1(data, w, h);
    KernelCtx ctx2(data, w, h);


    Kernel r1(m_mask1, Kernel::ROW);
    Kernel c1(m_mask2, Kernel::COL);
    ctx1.apply(r1).apply(c1);

    Kernel c2({2}, Kernel::ROW);
    ctx2.apply(c2);

    FilterUtils::toCanvas(canvas, start, end, ctx1.get(), ctx2.get(), [](glm::vec3 a, glm::vec3 b)
    {
        return BGRA(FilterUtils::REAL2byte(a.x + b.x),
                    FilterUtils::REAL2byte(a.y + b.y),
                    FilterUtils::REAL2byte(a.z + b.z));
    });
}
