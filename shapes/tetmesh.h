#ifndef TETMESH_H
#define TETMESH_H
#include <unordered_map>
#include "ui/SupportCanvas3D.h"
#include "scenegraph/Scene.h"
#include "openglshape.h"
#include "ui/mainwindow.h"
#include "gl/shaders/ShaderAttribLocations.h"


const float FLOOR_Y = -4;
const float KILL_FLOOR_Y = -20;
const float FLOOR_RADIUS = 9.0;


// combination hash function that combines hashes of each element
template <typename...> struct hashh;

template<typename T>
struct hashh<T>
    : public std::hash<T>
{
    using std::hash<T>::hash;
};


template <typename T, typename... Rest>
struct hashh<T, Rest...>
{
    inline std::size_t operator()(const T& v, const Rest&... rest) {
        std::size_t seed = hashh<Rest...>{}(rest...);
        seed ^= hashh<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

std::size_t hash_ivec3_fn(const glm::ivec3& v);

struct ivec3_hash : public std::unary_function<glm::ivec3, std::size_t> {
    std::size_t operator()(const glm::ivec3& v) const {
        return hash_ivec3_fn(v);
    }
};

// remember, triangles are in order: 124, 243, 431, 312
typedef struct tet {
    int p1, p2, p3, p4;
    int &operator[] (int index) {
        switch(index) {
        case 0:
            return p1;
        case 1:
            return p2;
        case 2:
            return p3;
        case 3:
            return p4;
        }
        throw std::out_of_range("Index out of range in tet_t[] operator");
    }
} tet_t;

typedef struct materialFEM {
    float incompressibility;
    float rigidity;
    float viscous1, viscous2;
} mat_t;


class TetMesh
{
public:
    TetMesh(){}
    TetMesh(object_node_t node, std::unordered_map<std::string, std::unique_ptr<TetMesh>>& map);
    TetMesh(std::string filename, glm::mat4x4 trans=glm::mat4x4(), std::string nodefile=std::string());
    std::vector<TetMesh> fracture(int tetIdx, glm::vec3 fracNorm);
    bool update(float timestep);
    void draw();
    const object_node_t& getONode() { return m_onode; }
    std::vector<glm::vec3> getFaceTris();
    void offsetPos(glm::vec3 offset);
private:
    void calcFacesAndNorms();
    void calcNorms();
    void computeFracture(const tet_t& tet, glm::mat3x3 stress);
    void computeStressForces(std::vector<glm::vec3>& forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels);
    void computeAllForces(std::vector<glm::vec3>& forcePerNode);
    void computeAllForcesFrom(std::vector<glm::vec3> &forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels);
    void computeCollisionForces(std::vector<glm::vec3>& forcePerNode,  const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels, float floorY);
    void calcBaryTransforms();
    void calcPointMasses();
    bool checkBad();
    int addNewPoint();
    std::vector<glm::vec3> m_points;
    std::vector<bool> m_isCrackTip;
    std::vector<glm::vec3> m_vels;
    std::vector<glm::vec3> m_norms;
    std::vector<tet_t> m_tets;
    std::vector<std::vector<int>> m_pToTMap;
    std::unordered_map<glm::ivec3, bool, ivec3_hash> m_faces;
    std::vector<glm::mat3x3> m_baryTransforms;
    object_node_t m_onode;
    mat_t m_material;
    // using lumped mass model, so rather than store an entire NxN matrix we will just store a vector
    std::vector<float> m_pointMasses;
    bool mustDie;

};

#endif // TETMESH_H
