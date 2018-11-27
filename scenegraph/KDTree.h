#ifndef KDTREE_H
#define KDTREE_H

#include <algorithm>
#include "CS123SceneData.h"

class KDTree
{
public:

    const size_t MAX_KD_DEPTH = 40;

    void minmax(AABB box, glm::vec2& x, glm::vec2& y, glm::vec2& z)
    {
        x.x = std::min(box.min().x, x.x);
        x.y = std::max(box.max().x, x.y);

        y.x = std::min(box.min().y, y.x);
        y.y = std::max(box.max().y, y.y);

        z.x = std::min(box.min().z, z.x);
        z.y = std::max(box.max().z, z.y);
    }

    KDTree(std::vector<CS123Renderable*> renderables, size_t depth = 0)
        : m_depth(depth)
    {

        m_left = nullptr;
        m_right = nullptr;

        glm::vec2 x(INFINITY, -INFINITY);
        glm::vec2 y(INFINITY, -INFINITY);
        glm::vec2 z(INFINITY, -INFINITY);

        for(auto r : renderables)
        {
            minmax(r->aabb(), x, y, z);
        }

        m_aabb = AABB(glm::vec3(x.x, y.x, z.x), glm::vec3(x.y, y.y, z.y));

        doSplit(renderables);
    }

    KDTree(AABB aabb, std::vector<CS123Renderable*> renderables, size_t depth)
        : m_aabb(aabb), m_depth(depth)
    {

        m_left = nullptr;
        m_right = nullptr;

        doSplit(renderables);
    }

    void doSplit(std::vector<CS123Renderable*> renderables);

    bool isLeaf()
    {
        return m_contained.size() != 0;
    }

    static inline bool less(glm::vec3 x, glm::vec3 y)
    {
        return (x.x < y.x && x.y < y.y && x.z < y.z);
    }

    static inline bool greater(glm::vec3 x, glm::vec3 y)
    {
        return (x.x > y.x && x.y > y.y && x.z > y.z);
    }

    static inline bool overlapAABB(AABB x, AABB y)
    {

        assert(less(glm::vec3(-0.750000, 0.375000, -6.375000), glm::vec3(0.000000, 8.250000, 8.250000)));

        //fvec3(-8.250000, -8.250000, -8.250000) : fvec3(0.000000, 8.250000, 8.250000) : fvec3(8.250000, 8.250000, 8.250000)
        //fvec3(-1.500000, -0.375000, -7.125000) : fvec3(-0.750000, 0.375000, -6.375000)
        // Less than

        /*std::cout << glm::to_string(y.min()) << " : " << glm::to_string(y.max()) << std::endl;


        std::cout << glm::to_string(x.max()) << "(" << less(x.max(), y.min()) << ") : " << glm::to_string(x.min()) << "(" << greater(x.min(), y.max()) << ")" << std::endl;


        if (less(x.max(), y.min()) || greater(x.min(), y.max()))
        {
            return false;
        }*/

        return true;
    }

    inline AABB aabb()
    {
        return m_aabb;
    }

    static float intersectsAABB(glm::vec4 p, glm::vec4 d, AABB aabb);

    static bool inAABB(CS123Renderable* renderable, AABB aabb, Axis axis)
    {
        return !(getValByAxis(renderable->aabb().max(), axis) < getValByAxis(aabb.min(), axis) ||
                 getValByAxis(renderable->aabb().min(), axis) > getValByAxis(aabb.max(), axis));

        //return overlapAABB(renderable->aabb(), aabb);
    }


    static float getSplitPlane(std::vector<CS123Renderable*> renderables, AABB aabb, Axis axis)
    {
        std::sort(renderables.begin(), renderables.end(), [axis](CS123Renderable* a, CS123Renderable* b) {
           return getValByAxis(a->aabb().min(), axis) < getValByAxis(b->aabb().min(), axis);
        });

        std::vector<float> planes;
        planes.reserve(renderables.size() * 2);
        for(auto r : renderables)
        {
            planes.push_back(getValByAxis(r->aabb().min(), axis));
            planes.push_back(getValByAxis(r->aabb().max(), axis));
        }

        std::sort(planes.begin(), planes.end(), [axis](float a, float b)
        {
           return a < b;
        });

        float min_split_cost = INFINITY;
        int min_split_idx = -1;

        float sa = aabb.sa();

        for(size_t i = 0; i < planes.size(); i++)
        {
            float p = planes[i];

            glm::vec3 min_l_plane = aabb.min();
            glm::vec3 max_l_plane = setValByAxis(aabb.max(), axis, p);

            glm::vec3 min_r_plane = setValByAxis(aabb.min(), axis, p);
            glm::vec3 max_r_plane = aabb.max();

            float left_sa = AABB(min_l_plane, max_l_plane).sa() / sa;
            float right_sa = AABB(min_r_plane, max_r_plane).sa() / sa;

            size_t left = 0;
            for(; left < renderables.size(); left++)
            {
                if (p < getValByAxis(renderables[left]->aabb().min(), axis))
                {
                    break;
                }
            }

            //std::cout << left << std::endl;
            float cost = left_sa * left + (right_sa * (renderables.size() - left));
            if (cost < min_split_cost)
            {
                min_split_cost = cost;
                min_split_idx = i;
            }
        }

        /*for(size_t i = 0; i < renderables.size() - 1; i++)
        {
            CS123Renderable* r = renderables[i];

            glm::vec3 min_l_plane = aabb.min();
            glm::vec3 max_l_plane = setValByAxis(aabb.max(), axis, getValByAxis(r->aabb().min(), axis));

            glm::vec3 min_r_plane = setValByAxis(aabb.min(), axis, getValByAxis(r->aabb().min(), axis));

            assert(getValByAxis(min_r_plane, axis) == getValByAxis(max_l_plane, axis));

            glm::vec3 max_r_plane = aabb.max();

            float left_sa = AABB(min_l_plane, max_l_plane).sa() / sa;
            float right_sa = AABB(min_r_plane, max_r_plane).sa() / sa;

            float cost = left_sa * i + (right_sa * (renderables.size() - i));
            if (cost < min_split_cost)
            {
                min_split_cost = cost;
                min_split_idx = i;
            }
        }*/

        return planes[min_split_idx];

        //return (getValByAxis(renderables[renderables.size() / 2]->aabb().min(), axis) + getValByAxis(renderables[renderables.size() / 2]->aabb().max(), axis)) / 2.0;


        /*float max = getValByAxis(aabb.max(), axis);
        float min = getValByAxis(aabb.min(), axis);

        return min + ((max - min) / 2.0);*/
    }

    static inline float getValByAxis(glm::vec3 v, Axis a)
    {
        if (a == Axis::X)
        {
            return v.x;
        } else if (a == Axis::Y)
        {
            return v.y;
        }
        return v.z;
    }

    static glm::vec3 setValByAxis(glm::vec3 v, Axis a, float f)
    {
        if (a == Axis::X)
        {
            v.x = f;
        } else if (a == Axis::Y)
        {
            v.y = f;
        } else {
            v.z = f;
        }

        return v;
    }

    static Axis getSplitAxis(size_t depth)
    {
        std::cout << "axis: " << static_cast<int>(static_cast<Axis>(depth % 3)) << std::endl;
        return static_cast<Axis>(depth % 3);
    }

    std::vector<IlluminateData> getAllIntersections(glm::vec4 p, glm::vec4 d);

    void getIntersections(glm::vec4 p, glm::vec4 d, std::vector<IlluminateData>& intersections);
private:

    AABB m_aabb;

    std::unique_ptr<KDTree> m_left = nullptr;
    std::unique_ptr<KDTree> m_right = nullptr;

    size_t m_depth;

    std::vector<CS123Renderable*> m_contained;
};

#endif // KDTREE_H
