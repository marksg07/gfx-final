#include "KDTree.h"
#include "IntersectionManager.h"
#include <glm/gtx/string_cast.hpp>

void KDTree::doSplit(std::vector<CS123Renderable *> renderables, size_t prevrep, size_t prevlen)
{
    std::cout << renderables.size() << std::endl;
    if (renderables.size() <= 4 || m_depth >= MAX_KD_DEPTH)
    {
        m_contained = renderables;
        return;
    }

    Axis axis = getSplitAxis(m_depth);
    float split = getSplitPlane(renderables, m_aabb, axis);

    //std::cout << split << std::endl;

    glm::vec3 min_l_plane = m_aabb.min();
    glm::vec3 max_l_plane = setValByAxis(m_aabb.max(), axis, split);

    glm::vec3 min_r_plane = setValByAxis(m_aabb.min(), axis, split);
    glm::vec3 max_r_plane = m_aabb.max();

    //std::cout << glm::to_string(min_l_plane) << " :  " << glm::to_string(max_l_plane) << " : " << glm::to_string(max_r_plane) << std::endl;

    //assert(getValByAxis(max_l_plane, axis) == getValByAxis(min_r_plane, axis));


    AABB laabb(min_l_plane, max_l_plane);
    AABB raabb(min_r_plane, max_r_plane);

    std::vector<CS123Renderable*> left;
    std::vector<CS123Renderable*> right;

    const size_t num_renderables = renderables.size();
    for(size_t i = 0; i < num_renderables; i++)
    {
        CS123Renderable* renderable = renderables[i];

        //std::cout <<  glm::to_string(renderable->aabb().min()) << " : " << glm::to_string(renderable->aabb().max()) << std::endl;

        bool added = false;
        //assert(inAABB(renderable, m_aabb));

        if (inAABB(renderable, laabb, axis))
        {
            added = true;
            left.push_back(renderable);
        }
        if (inAABB(renderable, raabb, axis))
        {
            added = true;
            right.push_back(renderable);
        }
        assert(added);
    }


    std::cout << "prims: " << num_renderables << ", left: " << left.size() << ", right: " << right.size() << std::endl;

// || (m_depth >= 4 && left.size() == renderables.size())
    if ((prevrep == 3 && right.size() == prevlen))
    {
        m_contained = right;
        return;
    } else if ((prevrep == 3 && left.size() == prevlen))
    {
        m_contained = left;
        return;
    } else {
        assert(!isLeaf());
        m_left = std::make_unique<KDTree>(left, left.size() == renderables.size() ? prevrep + 1 : 0, renderables.size(), m_depth + 1);
        m_right = std::make_unique<KDTree>(right, right.size() == renderables.size() ? prevrep + 1 : 0, renderables.size(), m_depth + 1);
    }

}

std::vector<IlluminateData> KDTree::getAllIntersections(glm::vec4 p, glm::vec4 d)
{
    std::vector<IlluminateData> intersections;

    getIntersections(p, d, intersections);

    return intersections;
}

void KDTree::getIntersections(glm::vec4 p, glm::vec4 d, std::vector<IlluminateData>& intersections)
{
    if (isLeaf())
    {
        size_t len = m_contained.size();

        for(size_t i = 0; i < len; i++)
        {
            IlluminateData ill = IntersectionManager::intersect(p, d, m_contained[i]);

            float t = ill.getT();
            if (t != INFINITY && t > 0)
            {
                intersections.push_back(ill);
            }
        }
    } else {

        float leftDist = (m_left == nullptr) ? INFINITY : intersectsAABB(p, d, m_left->aabb());
        float rightDist = (m_right == nullptr) ? INFINITY : intersectsAABB(p, d, m_right->aabb());

        float backDist = std::max(leftDist, rightDist);

        size_t num_inters = intersections.size();

        KDTree* front = m_right.get();
        KDTree* back = m_left.get();

        if (leftDist <= rightDist)
        {
            front = m_left.get();
            back = m_right.get();
        }

        if (front != nullptr)
        {
            front->getIntersections(p, d, intersections);

            for(size_t i = num_inters; i < intersections.size(); i++)
            {
                if (intersections[i].getT() <= backDist)
                {
                    return;
                }
            }
        }

        if (back != nullptr)
        {
            back->getIntersections(p, d, intersections);
        }
    }
}

float KDTree::intersectsAABB(glm::vec4 p, glm::vec4 d, AABB aabb)
{
    glm::vec4 scale = glm::vec4((aabb.max() - aabb.min()).xyz(), 1);
    glm::vec4 translation = glm::vec4((aabb.min().xyz() + (scale.xyz() / 2.0f)), 0);

    p -= translation;
    p /= scale;
    d /= scale;

    float t = IntersectionManager::intersectsCube(p, d);

    if (t < 0)
    {
        return INFINITY;
    }

    return t;
}
