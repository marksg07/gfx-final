#ifndef FILTERUTILS_H
#define FILTERUTILS_H

#include <vector>
#include <functional>
#include <iostream>
#include "Canvas2D.h"
#include "BGRA.h"
#include <glm.hpp>

class Canvas2D;

namespace FilterUtils
{

    std::vector<BGRA> transform(std::vector<BGRA> image, std::function<BGRA(BGRA)> func);
    std::vector<BGRA> greyscale(std::vector<BGRA> image);
    std::vector<BGRA> canvasToVector(Canvas2D* canvas, QPoint start, QPoint end);

    std::vector<glm::vec3> canvasToVec3(Canvas2D* canvas, QPoint start, QPoint end);

    std::vector<glm::vec3> BGRAToVec3(std::vector<BGRA> vec);
    void toCanvas(Canvas2D* canvas, QPoint start, QPoint end, const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, std::function<BGRA(glm::vec3, glm::vec3)> biconsumer);

    void toCanvas(Canvas2D* canvas, QPoint start, QPoint end, const std::vector<glm::vec3>& a);

    inline unsigned char REAL2byte(float f) {
        int i = static_cast<int>((f * 255.0 + 0.5));
        return (i < 0) ? 0 : (i > 255) ? 255 : i;
    }

    size_t width(Canvas2D* canvas, QPoint start, QPoint end);
    size_t height(Canvas2D* canvas, QPoint start, QPoint end);

    size_t startX(Canvas2D* canvas, QPoint start, QPoint end);
    size_t startY(Canvas2D* canvas, QPoint start, QPoint end);
    size_t endX(Canvas2D* canvas, QPoint start, QPoint end);
    size_t endY(Canvas2D* canvas, QPoint start, QPoint end);
}

#endif // FILTERUTILS_H
