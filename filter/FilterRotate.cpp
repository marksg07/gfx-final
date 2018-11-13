#include "FilterRotate.h"
#include "FilterUtils.h"
#include "Settings.h"
#include "Kernel.h"
#include <glm/gtx/transform.hpp>

FilterRotate::FilterRotate()
{
}

// from lab
inline float bicubic(float x)
{
    return x * x * (3-(2*x));
}

// basically from lab
inline float interp(float a, float b, float c, float d, float x, float y)
{
    float ab = glm::mix(a, b, bicubic(glm::fract(x)));
    float cd = glm::mix(c, d, bicubic(glm::fract(x)));

    return glm::mix(ab, cd, bicubic(glm::fract(y)));
}

// interpolate
inline glm::vec3 interpVec(std::vector<glm::vec3>& data, int ow, int oh, float rf, float cf)
{
    int row = floor(rf);
    int col = floor(cf);

    if (rf < 0 || rf >= oh)
    {
        return glm::vec3(0, 0, 0);
    }

    if (cf < 0 || cf >= ow)
    {
        return glm::vec3(0, 0, 0);
    }

    glm::vec3 a = Kernel::get(data, ow, oh, row, col);
    glm::vec3 b = Kernel::get(data, ow, oh, row, col + 1);
    glm::vec3 c = Kernel::get(data, ow, oh, row + 1, col);
    glm::vec3 d = Kernel::get(data, ow, oh, row + 1, col + 1);

    float r_interp = interp(a.x, b.x, c.x, d.x, cf, rf);
    float g_interp = interp(a.y, b.y, c.y, d.y, cf, rf);
    float b_interp = interp(a.z, b.z, c.z, d.z, cf, rf);

    return glm::vec3(r_interp, g_interp, b_interp);
}


void FilterRotate::apply(Canvas2D* canvas, QPoint start, QPoint end)
{
    // For some reason if I rotate + my rotation is backwards. Clearly some bug somewhere...
    glm::mat4 mat = glm::rotate((float) -settings.rotateAngle * (float) (M_PI / 180), glm::vec3(0, 0, 1));
    glm::mat4 mat_r = glm::rotate((float) settings.rotateAngle * (float) (M_PI / 180), glm::vec3(0, 0, 1));


    int h = canvas->height();
    int w = canvas->width();

    float o_x = w / 2.0;
    float o_y = h / 2.0;

    // Get the vector to the corners of the image.
    glm::vec4 tl = glm::vec4(-o_x, o_y, 0, 0);
    glm::vec4 tr = glm::vec4(o_x, o_y, 0, 0);
    glm::vec4 bl = glm::vec4(-o_x, -o_y, 0, 0);
    glm::vec4 br = glm::vec4(o_x, -o_y, 0, 0);

    // Find the locations of the corners in the new image.
    tr = mat * tr;
    tl = mat * tl;
    bl = mat * bl;
    br = mat * br;

    // Get the new size of the image
    float n_h = std::ceil(std::max(fabs(tl.y - br.y), fabs(tr.y - bl.y)));
    float n_w = std::ceil(std::max(fabs(tl.x - br.x), fabs(tr.x - bl.x)));

    std::vector<glm::vec3> img = FilterUtils::canvasToVec3(canvas, QPoint(0, 0), QPoint(0, 0));
    std::vector<glm::vec3> data;
    data.reserve(n_w * n_h);

    // new center x and new center y.
    float n_cx = n_w / 2.0;
    float n_cy = n_h / 2.0;

    glm::vec3 v_center = glm::vec3(n_cx, n_cy, 0);

    // Iterate over the pixels
    for(size_t r = 0; r < n_h; r++)
    {
        for(size_t c = 0; c < n_w; c++)
        {
            // Find the location of the pixel in the original image...
            glm::vec4 v = glm::vec4(glm::vec3(c, r, 0) - v_center, 0);
            v = mat_r * v;

            // Bicubically interpolate over the nearest 4 pixels (just like the interpolation in the lab)
            data[(r * n_w) + c] = interpVec(img, w, h, o_y + v.y, o_x + v.x);
        }
    }

    // resize and update
    canvas->resize(n_w, n_h);

    FilterUtils::toCanvas(canvas, QPoint(0, 0), QPoint(0, 0), data);
}
