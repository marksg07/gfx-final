#include "FilterSharpen.h"
#include "FilterUtils.h"
#include "Settings.h"
#include "Kernel.h"

FilterSharpen::FilterSharpen()
{
    m_radius = settings.blurRadius;

    size_t dim = (2 * m_radius) + 1;
    m_mask1.resize(dim);
    m_mask2.resize(dim);

    int idx = 0;
    for(int i = -m_radius; i <= m_radius; i++)
    {
        float x = 1.0 / dim;
        m_mask1[idx] = -x;
        m_mask2[idx] = x;

        idx++;
    }

}

// This filter is from the textbook section 19.6
void FilterSharpen::apply(Canvas2D* canvas, QPoint start, QPoint end)
{
    std::vector<glm::vec3> data = FilterUtils::canvasToVec3(canvas, start, end);

    size_t w = FilterUtils::width(canvas, start, end);
    size_t h = FilterUtils::height(canvas, start, end);

    KernelCtx ctx1(data, w, h);
    KernelCtx ctx2(data, w, h);

    // Kernel:
    /* essentially (where d is the h):
     * [ -1/d, -1/d, -1/d ]
     * [ -1/d, -1/d, -1/d ]
     * [ -1/d, -1/d, -1/d ]
     */
    // Since we have -1 total values here,
    // for our second kernel, we can use the value 2.

    /* (where d is the h * w):
     * [0, 0, 0]   [ -1/d, -1/d, -1/d ]   [ -1/d, -1/d, -1/d ]
     * [0, 2, 0] + [ -1/d, -1/d, -1/d ] = [ -1/d, 2 -(1/d), -1/d ]
     * [0, 0, 0]   [ -1/d, -1/d, -1/d ]   [ -1/d, -1/d, -1/d ]
     */
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
