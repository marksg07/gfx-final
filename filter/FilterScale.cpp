#include "FilterScale.h"
#include "FilterUtils.h"
#include "Settings.h"
#include "Kernel.h"
#include <cmath>
#include <iostream>

inline float tri_height(float center, float width, int x)
{
    // Area = (1/2)w*h = 1
    // h = 2/w

    // y = mx + (2/b)
    // 0 = m(b/2) + (2/b)
    // -2/b = m(b/2)
    // -4/(b^2) = m

    // y = -4/(b^2) * x + (2 / b)
    return std::max((float) 0.0, (float) (((-4.0 / (width * width))*fabs(center - x)) + (2.0 / width)));
}

FilterScale::FilterScale()
{
    m_xscale = settings.scaleX;
    m_yscale = settings.scaleY;



}

void FilterScale::apply(Canvas2D* canvas, QPoint start, QPoint end)
{
    int w = FilterUtils::width(canvas, start, end);
    int h = FilterUtils::height(canvas, start, end);

    int n_w = round(FilterUtils::width(canvas, start, end) * m_xscale);
    int n_h = round(FilterUtils::height(canvas, start, end) * m_yscale);


    std::vector<glm::vec3> data = FilterUtils::canvasToVec3(canvas, start, end);

    float x_filter_width = std::max(2.0f, (2.0f / m_xscale));
    int x_samples = (int) x_filter_width + 2;

    float y_filter_width = std::max(2.0f, (2.0f / m_yscale));
    int y_samples = (int) y_filter_width + 2;


    std::vector<glm::vec3> x_data;
    x_data.resize(h * n_w);

    int idx = 0;
    for(int r = 0; r < h; r++)
    {
        for(int c = 0; c < n_w; c++)
        {
            glm::vec3 sum = glm::vec3(0, 0, 0);

            float center = ((float) c / m_xscale) + ((1 - m_xscale) / (2 * m_xscale));
            int start_pix = ceil(center - (x_filter_width / 2));

            float s = 0;

            for(int sample = 0; sample < x_samples; sample++)
            {
                s += tri_height(center, x_filter_width, start_pix + sample);

                sum += tri_height(center, x_filter_width, start_pix + sample) * Kernel::get(data, w, h, r, start_pix + sample);


            }

            assert(s == 1);
            x_data[idx++] = sum;
        }
    }


    std::vector<glm::vec3> y_data;
    y_data.resize(n_h * n_w);

    idx = 0;
    for(int r = 0; r < n_h; r++)
    {
        for(int c = 0; c < n_w; c++)
        {
            glm::vec3 sum = glm::vec3(0, 0, 0);

            float center = ((float) r / m_yscale) + ((1 - m_yscale) / (2 * m_yscale));
            int start_pix = ceil(center - (y_filter_width / 2));

            //sum = Kernel::get(data, w, h, r, center_pix);

            float s = 0;

            for(int sample = 0; sample < y_samples; sample++)
            {
                s += tri_height(center, y_filter_width, start_pix + sample);

                sum += tri_height(center, y_filter_width, start_pix + sample) * Kernel::get(x_data, n_w, h, start_pix + sample, c);
            }

            assert(s == 1);
            y_data[(r * n_w) + c] = sum;
        }
    }


    canvas->resize(n_w, n_h);

    FilterUtils::toCanvas(canvas, QPoint(0, 0), QPoint(0, 0), y_data);

}
