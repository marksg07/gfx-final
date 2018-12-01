#ifndef KDTREE_H
#define KDTREE_H
#include "scenegraph/Scene.h"
#include <memory>
#include "intersect/implicitshape.h"
#include <thread>
#include <glm/gtx/transform.hpp>
#define MAX_DEPTH 36

struct ixInfo {
    ISPlace place;
    double t;
    const object_node_t *obj;
    glm::vec4 ix;
};


class KDTree
{
public:
    KDTree(std::vector<object_node_t> nodes, std::unique_ptr<KDTree> l, std::unique_ptr<KDTree> r, int depth, glm::vec3 mib, glm::vec3 mxb);
    KDTree(int nodecount, std::unique_ptr<KDTree> l, std::unique_ptr<KDTree> r, int depth, glm::vec3 mib, glm::vec3 mxb);
    KDTree(){};
    static std::unique_ptr<KDTree> buildTree(const std::vector<object_node_t>& m_nodes, int depth, glm::vec3 minbound, glm::vec3 maxbound);
    void pprint();
    struct ixInfo traverse(glm::vec4 P, glm::vec4 d);
private:
    struct ixInfo traverse(glm::vec4 P, glm::vec4 d, glm::vec4 invd, glm::bvec3 dsigns, glm::bvec3 idsigns);
    int m_nodecount;
    std::vector<object_node_t> m_nodes;
    std::unique_ptr<KDTree> m_l, m_r;
    glm::vec3 m_minbound, m_maxbound;
    int m_depth;
};

#endif // KDTREE_H
