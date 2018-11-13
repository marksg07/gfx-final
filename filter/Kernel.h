#ifndef KERNEL_H
#define KERNEL_H

#include "BGRA.h"
#include "GL/glew.h"
#include "FilterUtils.h"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

class Kernel
{
public:

    Kernel(std::vector<float> mask, int w, int h)
        : m_mask(mask), m_w(w), m_h(h)
    {
        assert(mask.size() == ((size_t) w) * h);
    }

    enum Orientation
    {
        ROW, COL
    };

    Kernel(std::vector<float> mask, Orientation o);


    static glm::vec3 get(const std::vector<glm::vec3>& img, int w, int h, int r, int c, bool zeros = false);

    std::vector<glm::vec3> getMatrix(const std::vector<glm::vec3>& img, int w, int h, int r, int c, int mw, int mh, bool zeros = false);

    std::vector<bool> getBoundsVec(const std::vector<glm::vec3>& img, int w, int h, int r, int c, int mw, int mh);

    virtual glm::vec3 dot(std::vector<glm::vec3> data, std::vector<float> kernel);

    virtual std::vector<glm::vec3> apply(const std::vector<glm::vec3>& data, int width, int height, bool renorm = false);

protected:

    std::vector<float> m_mask;
    int m_w;
    int m_h;
};

class KernelCtx
{
public:

    KernelCtx(std::vector<BGRA> img, int w, int h)
        : m_w(w), m_h(h)
    {
        m_img = FilterUtils::BGRAToVec3(img);
    }

    KernelCtx(Canvas2D* canvas, QPoint start, QPoint end)
    {
        std::vector<glm::vec3> data = FilterUtils::canvasToVec3(canvas, start, end);

        m_img = data;
        m_w = FilterUtils::width(canvas, start, end);
        m_h = FilterUtils::height(canvas, start, end);
    }

    KernelCtx(std::vector<glm::vec3> img, int w, int h)
        : m_img(img), m_w(w), m_h(h)
    {
    }

    KernelCtx& apply(Kernel& k, bool renorm = false)
    {
        m_img = k.apply(m_img, m_w, m_h, renorm);

        return *this;
    }

    std::vector<glm::vec3>& get()
    {
        return m_img;
    }

private:

    std::vector<glm::vec3> m_img;
    int m_w;
    int m_h;
};

#endif // KERNEL_H
