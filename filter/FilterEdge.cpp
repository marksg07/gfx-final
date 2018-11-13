#include "FilterUtils.h"
#include "FilterEdge.h"
#include "Kernel.h"
#include "Settings.h"
#include <iostream>

FilterEdge::FilterEdge()
{
    m_sensitivity = settings.edgeDetectSensitivity;
}

inline unsigned char clean(float x, float y, float sensitivity)
{
    return FilterUtils::REAL2byte(sqrt((x * x) + (y * y)) * sensitivity);
}


void FilterEdge::apply(Canvas2D* canvas, QPoint start, QPoint end)
{
    std::vector<glm::vec3> grey = FilterUtils::BGRAToVec3(FilterUtils::greyscale(FilterUtils::canvasToVector(canvas, start, end)));

    size_t w = FilterUtils::width(canvas, start, end);
    size_t h = FilterUtils::height(canvas, start, end);

    KernelCtx ctx1(grey, w, h);
    KernelCtx ctx2(grey, w, h);

    Kernel gx1({-1, 0, 1}, Kernel::ROW);
    Kernel gx2({1, 2, 1}, Kernel::COL);

    Kernel gy1({1, 2, 1}, Kernel::ROW);
    Kernel gy2({-1, 0, 1}, Kernel::COL);

    ctx1.apply(gx1).apply(gx2);

    ctx2.apply(gy1).apply(gy2);

    float sensitivity = m_sensitivity;
    FilterUtils::toCanvas(canvas, start, end, ctx1.get(), ctx2.get(), [sensitivity](glm::vec3 a, glm::vec3 b)
    {
        return BGRA(clean(a.x, b.x, sensitivity), clean(a.y, b.y, sensitivity), clean(a.z, b.z, sensitivity));
    });
}
