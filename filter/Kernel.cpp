#include "Kernel.h"

Kernel::Kernel(std::vector<float> mask, Orientation o)
    : m_mask(mask)
{
    if (o == ROW)
    {
        m_w = mask.size();
        m_h = 1;
    } else {
        m_w = 1;
        m_h = mask.size();
    }
}

std::vector<bool> Kernel::getBoundsVec(const std::vector<glm::vec3>& img, int w, int h, int r, int c, int mw, int mh)
{
    std::vector<bool> mat;
    mat.resize(mw * mh);

    int idx = 0;
    for(int i = -mh / 2; i <= mh / 2; i++)
    {
        for(int j = -mw / 2; j <= mw / 2; j++)
        {
            bool x = true;

            if ((r + i < 0) || (r + i >= h)) {
                x = false;
            } else if ((c + j < 0) ||(c + j >= w)) {
                x = false;
            }

            mat[idx++] = x;
        }
    }

    return mat;
}

glm::vec3 Kernel::get(const std::vector<glm::vec3>& img, int w, int h, int r, int c, bool zeros)
{
    if (r < 0)
    {
        if (zeros) {
            return glm::vec3(0, 0, 0);
        }
        r = 0;
    } else if (r >= h) {
        if (zeros) {
            return glm::vec3(0, 0, 0);
        }
        r = h - 1;
    }

    if (c < 0)
    {
        if (zeros) {
            return glm::vec3(0, 0, 0);
        }
        c = 0;
    } else if (c >= w) {
        if (zeros) {
            return glm::vec3(0, 0, 0);
        }
        c = w - 1;
    }

    return img[(r * w) + c];
}

std::vector<glm::vec3> Kernel::getMatrix(const std::vector<glm::vec3>& img, int w, int h, int r, int c, int mw, int mh, bool zeros)
{
    std::vector<glm::vec3> mat;
    mat.resize(mw * mh);

    int idx = 0;
    for(int i = -mh / 2; i <= mh / 2; i++)
    {
        for(int j = -mw / 2; j <= mw / 2; j++)
        {
            mat[idx++] = get(img, w, h, r + i, c + j, zeros);
        }
    }

    return mat;
}

glm::vec3 Kernel::dot(std::vector<glm::vec3> data, std::vector<float> kernel)
{
    glm::vec3 out;

    size_t sz = kernel.size();
    for(size_t i = 0; i < sz; i++)
    {
        out += kernel[i] * data[i];
    }

    return out;
}


std::vector<glm::vec3> Kernel::apply(const std::vector<glm::vec3>& data, int width, int height, bool renorm)
{
    std::vector<glm::vec3> out;
    out.resize(data.size());

    int idx = 0;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {

            glm::vec3 pix = glm::vec3(0, 0, 0);

            int midx = 0;
            for(int mi = -m_h / 2; mi <= m_h / 2; mi++)
            {
                for(int mj = -m_w / 2; mj <= m_w / 2; mj++)
                {
                    pix += get(data, width, height, i + mi, j + mj, renorm) * m_mask[midx++];
                }
            }

            out[idx++] = pix;


#if 0
            if (renorm)
            {
                float sum = 0;
                std::vector<bool> bounds = getBoundsVec(data, width, height, i, j, m_w, m_h);
                for(int i = 0; i < bounds.size(); i++)
                {
                    sum += (bounds[i]) ? m_mask[i] : 0;
                }
<
                //std::cout << "norm sum: " << sum << std::endl;

                out[idx++] = dot(vecImg, m_mask) / sum;
            } else {
                out[idx++] = dot(vecImg, m_mask);
            }
#endif
        }
    }


    return out;
}
