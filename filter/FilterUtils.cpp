#include "FilterUtils.h"

namespace FilterUtils
{
    std::vector<BGRA> transform(std::vector<BGRA> image, std::function<BGRA(BGRA)> func)
    {
        std::vector<BGRA> out;
        out.resize(image.size());

        for(size_t i = 0; i < image.size(); i++)
        {
            out[i] = func(image[i]);
        }

        return out;
    }

    std::vector<BGRA> greyscale(std::vector<BGRA> image)
    {
        std::function<float(BGRA)> pixelIntensity = [](BGRA pixel) {
            return (.299 * pixel.r) + (.587 * pixel.g) + (.114 * pixel.b);
        };

        return FilterUtils::transform(image, [pixelIntensity](BGRA pixel)
        {
            float intensity = pixelIntensity(pixel);

            return BGRA(intensity, intensity, intensity);
        });
    }

    std::vector<BGRA> canvasToVector(Canvas2D* canvas, QPoint start, QPoint end)
    {
        std::vector<BGRA> data;
        BGRA* pix = canvas->data();

        size_t s_x = FilterUtils::startX(canvas, start, end);
        size_t s_y = FilterUtils::startY(canvas, start, end);

        size_t e_x = FilterUtils::endX(canvas, start, end);
        size_t e_y = FilterUtils::endY(canvas, start, end);
        size_t vw = FilterUtils::width(canvas, start, end);
        size_t vh = FilterUtils::height(canvas, start, end);
        size_t w = canvas->width();

        data.resize(vw * vh);
        //std::cout << "(" << s_x << ", " << s_y << ") : (" << e_x << ", " << e_y << ")" << std::endl;

        size_t i = 0;
        for(size_t r = s_y; r <= e_y; r++)
        {
            for(size_t c = s_x; c <= e_x; c++)
            {
                data[i++] = pix[(r * w) + c];
            }
        }
        std::cout << i << " : " << data.size() << std::endl;

        canvas->update();
        return data;
    }

    std::vector<glm::vec3> canvasToVec3(Canvas2D* canvas, QPoint start, QPoint end)
    {
        std::vector<glm::vec3> data;
        BGRA* pix = canvas->data();

        size_t s_x = FilterUtils::startX(canvas, start, end);
        size_t s_y = FilterUtils::startY(canvas, start, end);

        size_t e_x = FilterUtils::endX(canvas, start, end);
        size_t e_y = FilterUtils::endY(canvas, start, end);
        size_t vw = FilterUtils::width(canvas, start, end);
        size_t vh = FilterUtils::height(canvas, start, end);
        size_t w = canvas->width();

        data.resize(vw * vh);

        size_t i = 0;
        for(size_t r = s_y; r <= e_y; r++)
        {
            for(size_t col = s_x; col <= e_x; col++)
            {
                BGRA c = pix[(r * w) + col];
                data[i++] = glm::vec3(c.r / 255.0, c.g / 255.0, c.b / 255.0);
            }
        }

        canvas->update();
        return data;
    }

    std::vector<glm::vec3> BGRAToVec3(std::vector<BGRA> vec)
    {
        std::vector<glm::vec3> data;
        data.resize(vec.size());

        for(size_t i = 0; i < vec.size(); i++)
        {
            BGRA c = vec[i];
            data[i] = glm::vec3(c.r / 255.0, c.g / 255.0, c.b / 255.0);
        }

        return data;
    }

    void toCanvas(Canvas2D* canvas, QPoint start, QPoint end, const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, std::function<BGRA(glm::vec3, glm::vec3)> biconsumer)
    {

        BGRA* pix = canvas->data();

        size_t s_x = FilterUtils::startX(canvas, start, end);
        size_t s_y = FilterUtils::startY(canvas, start, end);
        size_t e_x = FilterUtils::endX(canvas, start, end);
        size_t e_y = FilterUtils::endY(canvas, start, end);
        size_t w = canvas->width();

        size_t i = 0;

        for(size_t r = s_y; r <= e_y; r++)
        {
            for(size_t c = s_x; c <= e_x; c++)
            {
                pix[(r * w) + c] = biconsumer(a[i], b[i]);
                i++;
            }
        }
        canvas->update();
    }

    void toCanvas(Canvas2D* canvas, QPoint start, QPoint end, const std::vector<glm::vec3>& data)
    {
        BGRA* pix = canvas->data();

        size_t s_x = FilterUtils::startX(canvas, start, end);
        size_t s_y = FilterUtils::startY(canvas, start, end);
        size_t e_x = FilterUtils::endX(canvas, start, end);
        size_t e_y = FilterUtils::endY(canvas, start, end);
        size_t w = canvas->width();

        size_t i = 0;
        for(size_t r = s_y; r <= e_y; r++)
        {
            for(size_t col = s_x; col <= e_x; col++)
            {
                glm::vec3 v = data[i++];

                pix[(r * w) + col] = BGRA(REAL2byte(v.x), REAL2byte(v.y), REAL2byte(v.z));
            }
        }

        canvas->update();
    }

    size_t width(Canvas2D* canvas, QPoint start, QPoint end)
    {
        return (endX(canvas, start, end) + 1) - startX(canvas, start, end);
    }

    size_t height(Canvas2D* canvas, QPoint start, QPoint end)
    {
        return (endY(canvas, start, end) + 1) - startY(canvas, start, end);
    }

    size_t startX(Canvas2D* canvas, QPoint start, QPoint end)
    {
        if (start == end)
        {
            return 0;
        }

        return std::max(0, std::min(canvas->width() - 1, std::min(start.x(), end.x())));
    }

    size_t startY(Canvas2D* canvas, QPoint start, QPoint end)
    {
        if (start == end)
        {
            return 0;
        }

        return std::max(0, std::min(canvas->height() - 1, std::min(start.y(), end.y())));
    }
    size_t endX(Canvas2D* canvas, QPoint start, QPoint end)
    {
        if (start == end)
        {
            return canvas->width() - 1;
        }

        return std::max(0, std::min(canvas->width() - 1, std::max(start.x(), end.x())));
    }

    size_t endY(Canvas2D* canvas, QPoint start, QPoint end)
    {
        if (start == end)
        {
            return canvas->height() - 1;
        }

        return std::max(0, std::min(canvas->height() - 1, std::max(start.y(), end.y())));
    }
}

