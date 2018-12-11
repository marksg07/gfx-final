#include "kdtree.h"
#include "intersect/implicitshape.h"
#include <algorithm>
#include "Settings.h"

KDTree::KDTree(int nnodes, std::unique_ptr<KDTree> l, std::unique_ptr<KDTree> r, int depth, glm::vec3 mib, glm::vec3 mxb) :
    m_nodecount(nnodes),
    m_nodes(),
    m_minbound(mib),
    m_maxbound(mxb),
    m_depth(depth) {
    m_l = std::move(l);
    m_r = std::move(r);
}

KDTree::KDTree(std::vector<object_node_t> nodes, std::unique_ptr<KDTree> l, std::unique_ptr<KDTree> r, int depth, glm::vec3 mib, glm::vec3 mxb) :
    m_nodecount(nodes.size()),
    m_nodes(nodes),
    m_minbound(mib),
    m_maxbound(mxb),
    m_depth(depth) {
    m_l = std::move(l);
    m_r = std::move(r);
}

double surfaceArea(glm::vec3 box) {
    // 2 * this, we don't care though
    return (box.x * box.y + box.y * box.z + box.x * box.z);
}

struct split {
    double plane;
    int axis;
};

struct split chooseSplit(const std::vector<object_node_t>& m_nodes, int depth, glm::vec3 minbound, glm::vec3 maxbound) {
    int splitAxis = depth % 3;
    double minAxis = minbound[splitAxis];
    double maxAxis = maxbound[splitAxis];
    double axis = NAN;
    double bestCost = m_nodes.size() * surfaceArea(maxbound - minbound);
    for(unsigned long i = 0; i < m_nodes.size(); i++) {
        for(int pn = 0; pn <= 1; pn += 1) {
            double ax = pn ? m_nodes[i].maxbound[splitAxis] : m_nodes[i].minbound[splitAxis];
            //printf("min is %f, max is %f. Considering %f\n", minAxis, maxAxis, ax);
            if(ax < minAxis || ax > maxAxis)
                continue;

            glm::vec3 split1max = maxbound;
            split1max[splitAxis] = ax;
            glm::vec3 split1min = minbound;
            split1min[splitAxis] = ax;

            double leftarea = surfaceArea(split1max - minbound);
            double rightarea = surfaceArea(maxbound - split1min);
            int leftcount = 0;
            int rightcount = 0;

            for(unsigned long j = 0; j < m_nodes.size(); j++) {
                double objbL = m_nodes[j].minbound[splitAxis];
                double objbR = m_nodes[j].maxbound[splitAxis];
                if(!(objbR < minAxis || objbL >= ax)) {
                    leftcount++;
                }
                if(!(objbR < ax || objbL >= maxAxis)) {
                    rightcount++;
                }
            }

            double cost = leftarea * leftcount + rightarea * rightcount;
            if(cost < bestCost) {
                bestCost = cost;
                axis = ax;
            }
        }
    }
    return {axis, splitAxis};
}

// Note: This implementation is based on
// "On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)"
// by Ingo Wald, Vlastimil Havran
// (http://dcgi.felk.cvut.cz/home/havran/ARTICLES/ingo06rtKdtree.pdf)

enum EventType {
    END,
    START
};

struct KDEvent {
    const object_node_t *obj;
    float plane;
    EventType type;
};

double calculateSAH(int l, int r, double plane, int axis, glm::vec3 mnb, glm::vec3 mxb) {
    double relplane = plane - mnb[axis];
    glm::vec3 box = mxb - mnb;
    box[axis] -= relplane; // end - plane, so right side
    double saR = surfaceArea(box);
    box[axis] = relplane; // up to plane, so left side
    double saL = surfaceArea(box);
    return l * saL + r * saR;
}

bool eventLess(const KDEvent& a, const KDEvent& b) {
    return (a.plane < b.plane) || (a.plane == b.plane && a.type < b.type); // end before start
}

struct split chooseSplitNlogN2(const std::vector<object_node_t>& nodes, int depth, glm::vec3 minbound, glm::vec3 maxbound) {
    double bestCost = calculateSAH(0, nodes.size(), minbound[0], 0, minbound, maxbound); //nodes.size() * surfaceArea(maxbound - minbound);
    int bestAxis = -1;
    double bestPlane = NAN;
    int n = nodes.size();

    std::vector<KDEvent> events;
    // looping per axis performs slower for building than just using balanced splits w/ depth % 3, obviously,
    // and performance gains aren't worth the loss in building.
    int axis = depth % 3;
    for(int i = 0; i < n; i++) {
        if(nodes[i].minbound[axis]> minbound[axis])
            events.push_back((KDEvent){&nodes[i], nodes[i].minbound[axis], START});
        if(nodes[i].maxbound[axis] < maxbound[axis])
            events.push_back((KDEvent){&nodes[i], nodes[i].maxbound[axis], END});
    }
    std::sort(events.begin(), events.end(), eventLess);
    int Nl = 0;
    int Nr = n;
    for(unsigned long i = 0; i < events.size(); i++) {
        KDEvent *event = &events[i];
        assert(event->plane > minbound[axis] && event->plane < maxbound[axis]);
        if(event->type == END)
            Nr--; // this is not included in the right plane anymore
        double cost = calculateSAH(Nl, Nr, event->plane, axis, minbound, maxbound);
        if(cost < bestCost) {
            bestCost = cost;
            bestPlane = event->plane;
            bestAxis = axis;
        }
        if(event->type == START)
            Nl++; // this now has to be included in left plane, but not until next evt

    }
    return {bestPlane, bestAxis};
}

std::unique_ptr<KDTree> KDTree::buildTree(const std::vector<object_node_t>& m_nodes, int depth, glm::vec3 minbound, glm::vec3 maxbound) {
    // depth determines where we split: % 3 = 0 -> x, % 3 = 1 -> y, % 3 = 2 -> z
    if(m_nodes.size() <= 2)
        return std::make_unique<KDTree>(m_nodes, nullptr, nullptr, depth, minbound, maxbound);
    //struct split spl = chooseSplit(m_nodes, depth, minbound, maxbound);
    struct split spl = chooseSplitNlogN2(m_nodes, depth, minbound, maxbound);
    double axis = spl.plane;
    int splitAxis = spl.axis;
    if(std::isnan(axis) || splitAxis == -1) { // no split necessary
        return std::make_unique<KDTree>(m_nodes, nullptr, nullptr, depth, minbound, maxbound);
    }
    double minAxis = minbound[splitAxis];
    double maxAxis = maxbound[splitAxis];
    std::vector<object_node_t> left_nodes;
    std::vector<object_node_t> right_nodes;
    for(unsigned long i = 0; i < m_nodes.size(); i++) {
        double objbL = m_nodes[i].minbound[splitAxis];
        double objbR = m_nodes[i].maxbound[splitAxis];
        if(!((objbL <= minAxis && objbR <= minAxis)
                || (objbL >= axis && objbR >= axis))) {
            left_nodes.push_back(m_nodes[i]);
        }
        if(!((objbL <= axis && objbR <= axis)
             || (objbL >= maxAxis && objbR >= maxAxis))) {
            right_nodes.push_back(m_nodes[i]);
        }
    }
    if(left_nodes.size() == 0) {
        assert(right_nodes.size() == m_nodes.size());
        return std::make_unique<KDTree>(m_nodes, nullptr, nullptr, depth, minbound, maxbound);
    }
    if(right_nodes.size() == 0) {
        assert(left_nodes.size() == m_nodes.size());
        return std::make_unique<KDTree>(m_nodes, nullptr, nullptr, depth, minbound, maxbound);
    }
    glm::vec3 leftmax = maxbound;
    leftmax[splitAxis] = axis;
    glm::vec3 rightmin = minbound;
    rightmin[splitAxis] = axis;
    // i had a threaded impl that would do one of these trees on a thread
    // and the other without and then join the thread, but it was slower
    std::unique_ptr<KDTree> rightTree = buildTree(right_nodes, depth+1, rightmin, maxbound);
    std::unique_ptr<KDTree> leftTree = buildTree(left_nodes, depth+1, minbound, leftmax);
    return std::make_unique<KDTree>(m_nodes.size(), std::move(leftTree), std::move(rightTree), depth, minbound, maxbound);
}

void KDTree::pprint() {
    char spaces[MAX_DEPTH + 2];
    for(int i = 0; i < m_depth; i++) {
        spaces[i] = ' ';
    }
    spaces[m_depth] = 0;

    printf("%sDepth=%d, #nodes = %d, children:\n", spaces, m_depth, m_nodecount);
    if(m_l != nullptr)
        m_l->pprint();
    if(m_r != nullptr)
        m_r->pprint();

}

// NOTE: There are definitely more efficient ways of doing cube intersection, and as it stands,
// this is 33.3% of my processing time on shape-heavy scenes. But this works well enough.
double intersectAABB(glm::vec4 P, glm::vec4 d, glm::vec3 minbound, glm::vec3 maxbound) {
    glm::vec4 scale(1.f/(maxbound - minbound), 1.f);
    glm::vec4 trans(-(maxbound + minbound)/2.f, 0.f);
    auto t_d = ImplicitShape::cubeIntersectT_neg((P + trans) * scale, d * scale);
    double t = t_d.t;
    return t;
}


struct ixInfo findIntersect(glm::vec4 P, glm::vec4 d, const std::vector<object_node_t>& nodes) {
    double smallestT = INFINITY;
    ISPlace isectPlace = UNDEF;
    const object_node_t *front_obj = NULL;
    glm::vec4 os_intersect;
    //printf("itering thru obj, n_objs = %d\n", m_nodes.size());
    for(unsigned long i = 0; i < nodes.size(); i++) {
        const object_node_t *obj = &nodes[i];
        glm::vec4 eye_os = obj->invtrans * P;
        glm::vec4 v_dir_os = obj->invtrans * d;
        auto t_p = ImplicitShape::getIntersectT(obj->primitive.type, eye_os, v_dir_os);
        double t = t_p.t;
        if(t >= 0 && t < smallestT) {
            //printf("found new smallest intersection at %f\n", t);
            isectPlace = t_p.pl;
            smallestT = t;
            front_obj = obj;
            os_intersect = eye_os + glm::vec4(smallestT, smallestT, smallestT, 0) * v_dir_os;
        }
    }
    struct ixInfo ixi = {isectPlace, smallestT, front_obj, os_intersect};
    return ixi;
}

struct ixInfo KDTree::traverse(glm::vec4 P, glm::vec4 d) {
    glm::vec4 invd = 1.f/d;
    glm::bvec3 dsigns(invd.x >= 0, invd.y >= 0, invd.z >= 0);
    glm::bvec3 idsigns(!dsigns.x, !dsigns.y, !dsigns.z);
    return traverse(P, d, invd, dsigns, idsigns);
}

struct ixInfo KDTree::traverse(glm::vec4 P, glm::vec4 d, glm::vec4 invd, glm::bvec3 dsigns, glm::bvec3 idsigns) {
    if(m_l == nullptr)
        if(m_r == nullptr){
            // leaf
            struct ixInfo i = findIntersect(P, d, m_nodes);
            return i;
        }
        else {
            return m_r->traverse(P, d, invd, dsigns, idsigns);
    }
    else if(m_r == nullptr) {
        return m_l->traverse(P, d, invd, dsigns, idsigns);
    }
    else {
        struct ixInfo ixi1, ixi2;
        ixi1.t = INFINITY;
        ixi2.t = INFINITY;
        ixi1.place = UNDEF;
        ixi2.place = UNDEF;
        ixi1.obj = NULL;
        ixi2.obj = NULL;
        double t1 = ImplicitShape::AABBIntersectT(P, d, invd, dsigns, idsigns, m_l->m_minbound, m_l->m_maxbound);
        double t2 = ImplicitShape::AABBIntersectT(P, d, invd, dsigns, idsigns, m_r->m_minbound, m_r->m_maxbound);
        if(t1 < t2) { // we hit left first
            ixi1 = m_l->traverse(P, d, invd, dsigns, idsigns);
            if(t2 < ixi1.t) // if we hit an object past the bbox of right, we must traverse it too (edge case)
                ixi2 = m_r->traverse(P, d, invd, dsigns, idsigns);
        }
        else if(t1 > t2) { // hit right first
            ixi2 = m_r->traverse(P, d, invd, dsigns, idsigns);
            if(t1 < ixi2.t) // similar edge case
                ixi1 = m_l->traverse(P, d, invd, dsigns, idsigns);
        }
        else if(std::isinf(t1)) {
            return ixi1;
        }
        else { // hits both at the same time? weird, but could happen... Check both
            ixi1 = m_l->traverse(P, d, invd, dsigns, idsigns);
            ixi2 = m_r->traverse(P, d, invd, dsigns, idsigns);
        }

        if(ixi1.t < ixi2.t) {
            return ixi1;
        }
        return ixi2;
    }
}


//KDTree buildTree()
