#include "tetmesh.h"
#include <unistd.h>
#include <tetgen.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <functional>
#include <glm/gtx/random.hpp>
#include <glm/gtx/transform.hpp>
#include <unordered_set>
#include "Settings.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <Eigen/Cholesky>
#include "timing.h"
#include "tetmeshparser.h"
#include <glm/gtc/matrix_access.hpp>
/*
 * Incompressibility: 1000
 * Rigidity: 1000
 * Viscosity: 10
 * Density: 1200
 */

std::size_t hash_ivec3_fn(const glm::ivec3& v) {
    hashh<int, int, int> hasher;
    return hasher(v.x, v.y, v.z);

}

std::size_t hash_ivec2_fn(const glm::ivec2& v) {
    hashh<int, int, int> hasher;
    return hasher(v.x, v.y, 0);

}

namespace {
inline glm::vec3 getPoint(const tetgenio& t, int i) {
    assert(i < t.numberofpoints);
    return glm::vec3(t.pointlist[3 * i], t.pointlist[3 * i + 1], t.pointlist[3 * i + 2]);
}

std::vector<tet_t> getTets(const tetgenio& t) {
    std::vector<tet_t> ret;
    ret.reserve(t.numberoftetrahedra);
    for(int i = 0; i < t.numberoftetrahedra; i++) {
        int p1idx = t.tetrahedronlist[i * 4];
        int p2idx = t.tetrahedronlist[i * 4 + 1];
        int p3idx = t.tetrahedronlist[i * 4 + 2];
        int p4idx = t.tetrahedronlist[i * 4 + 3];
        ret.push_back((tet_t){p1idx, p2idx, p3idx, p4idx});
    }
    return ret;
}

std::vector<glm::vec3> getPoints(const tetgenio& t) {
    std::vector<glm::vec3> ret;
    ret.reserve(t.numberofpoints);
    for(int i = 0; i < t.numberofpoints; i++) {
        ret.push_back(getPoint(t, i));
    }
    return ret;
}

std::vector<std::unordered_set<glm::ivec2, ivec2_hash>> tetsTouchingPoint(const std::vector<tet_t>& tets, int psize) {
    std::vector<std::unordered_set<glm::ivec2, ivec2_hash>> pToTMap;
    pToTMap.resize(psize);
    for(long unsigned int i = 0; i < tets.size(); i++) {
        tet_t tet = tets[i];
        for(int j = 0; j < 4; j++) {
            int pidx = tet[j];
            pToTMap[pidx].insert(glm::ivec2(i, j));
        }
    }
    return pToTMap;
}
}

Eigen::Matrix3f glmToEigen(glm::mat3x3 mat) {
    Eigen::Matrix3f out;
    out << mat[0][0], mat[1][0], mat[2][0],
            mat[0][1], mat[1][1], mat[2][1],
            mat[0][2], mat[1][2], mat[2][2];
    return out;
}

glm::mat3x3 eigenToGlm(Eigen::Matrix3f mat) {
    return glm::mat3x3(mat(0), mat(3), mat(6),
                    mat(1), mat(4), mat(7),
                    mat(2), mat(5), mat(8));
}

void TetMesh::calcMatspaceCrosses() {
    for(int i = 0; i < m_tets.size(); i++) {
        tet_t tet = m_tets[i];
        auto p1 = m_points[tet.p1];
        auto p2 = m_points[tet.p2];
        auto p3 = m_points[tet.p3];
        auto p4 = m_points[tet.p4];

        glm::vec3 p1an = -glm::cross(p4 - p2, p3 - p2);
        glm::vec3 p2an = -glm::cross(p4 - p3, p1 - p3);
        glm::vec3 p3an = -glm::cross(p4 - p1, p2 - p1);
        m_matspace_crosses[i] = glmToEigen(glm::mat3x3(p1an, p2an, p3an));
    }
}

// XXX material and nodefile unused
TetMesh::TetMesh(std::string filename, std::string nodefile) {
    // material unused for now
    // m_material = {0.16, 0.003, 0.16, 0.003};
    tetgenio out;
    TetmeshParser::parse(filename, &out);
    m_tets = getTets(out);
    m_baryTransforms.resize(m_tets.size());
    m_points = getPoints(out);
    for(int i = 0; i < m_points.size(); i++) {
        //m_points[i] = glm::vec3(glm::rotate((float)M_PI/6.f, glm::vec3(1, 0, 0)) * glm::rotate((float)M_PI/6.f, glm::vec3(0, 1, 0)) * glm::vec4(m_points[i], 0.f));
    }
    m_isCrackTip.resize(m_points.size());
    m_norms.resize(m_points.size());
    m_vels.resize(m_points.size());
    m_pointMasses.resize(m_points.size());
    m_pToTMap = tetsTouchingPoint(m_tets, m_points.size());
    m_matspace_crosses.resize(m_tets.size());
    float avg = 0;
    for(int i = 0; i < m_points.size(); i++) {
        avg += m_pToTMap[i].size();
    }
    printf("Avg #tets touching point is %f\n", avg / m_points.size());
    fflush(stdout);
    calcFacesAndNorms();

    printf("Tets loaded: %lu\n", m_tets.size());
    printf("m_faces size is now %lu\n", m_faces.size());
    calcBaryTransforms();
    calcMatspaceCrosses();
    calcPointMasses();
    std::fill(m_isCrackTip.begin(), m_isCrackTip.end(), false);
    srand(time(NULL));

    // balloon everything out a bit
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        //m_vels[i] = -m_points[i] / 1.f;
    }
    calcNorms();
    printf("N surface faces: %lu\n", m_faces.size());
}

TetMesh::TetMesh(object_node_t node, std::unordered_map<std::string, std::unique_ptr<TetMesh>>& map) {
    m_onode = node;
    if(!map.count(node.primitive.meshfile)) {
        map[node.primitive.meshfile] = std::make_unique<TetMesh>(node.primitive.meshfile);
    }
    TetMesh* copyFrom = map[node.primitive.meshfile].get();
    // copy everything from the template except the node
    m_faces = copyFrom->m_faces;
    m_points = copyFrom->m_points;
    m_norms = copyFrom->m_norms;
    m_tets = copyFrom->m_tets;
    m_pToTMap = copyFrom->m_pToTMap;
    m_baryTransforms = copyFrom->m_baryTransforms;
    m_pointMasses = copyFrom->m_pointMasses;
    m_vels = copyFrom->m_vels;
    m_isCrackTip = copyFrom->m_isCrackTip;
    m_matspace_crosses = copyFrom->m_matspace_crosses;
}

namespace {
inline float randFloat() {
    return ((float)rand())/RAND_MAX*2-1;
}

bool tetInverted(const std::vector<glm::vec3> &points, tet_t tet) {
    auto p1 = points[tet.p1];
    auto p2 = points[tet.p2];
    auto p3 = points[tet.p3];
    auto p4 = points[tet.p4];
    glm::vec3 cross123 = glm::cross(p2 - p1, p3 - p1);
    return glm::dot(cross123, p4 - p1) < 0;
}

glm::vec3 getCenter(const std::vector<glm::vec3> &points, tet_t tet) {
    return (points[tet.p1] + points[tet.p2] + points[tet.p3] + points[tet.p4]) / 4.f;
}

bool hasFace(tet_t tet, int p1, int p2, int p3) {
    std::unordered_set<int> s = {tet.p1, tet.p2, tet.p3, tet.p4};
    return s.count(p1) && s.count(p2) && s.count(p3);
}
}

int TetMesh::addNewPoint() {
    m_points.push_back(glm::vec3());
    m_norms.push_back(glm::vec3());
    m_isCrackTip.push_back(false);
    m_vels.push_back(glm::vec3());
    m_pToTMap.push_back(std::unordered_set<glm::ivec2, ivec2_hash>());
    m_pointMasses.push_back(0);
    return m_points.size() - 1;
}

// Given F, the deformation gradient, and V, the velocity gradient, calculate the stress.
glm::mat3x3 getStress(glm::mat3x3 F, glm::mat3x3 V) {
    glm::mat3x3 id = glm::mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    glm::mat3x3 Ft = glm::transpose(F);
    glm::mat3x3 Vt = glm::transpose(V);


    glm::mat3x3 strain = Ft * F - id;
    glm::mat3x3 trace_matrix = (strain[0][0] + strain[1][1] + strain[2][2]) * id;

    glm::mat3x3 stress_elastic = 2 * settings.femRigidity * strain
            + settings.femIncompressibility * trace_matrix;

    glm::mat3x3 strain_rate = Ft * V + Vt * F;
    glm::mat3x3 sr_trace_matrix = (strain_rate[0][0] + strain_rate[1][1] + strain_rate[2][2]) * id;

    glm::mat3x3 stress_viscous = 2 * settings.femShearViscosity * strain_rate
            + settings.femBulkViscosity * sr_trace_matrix;

    return F*(stress_elastic + stress_viscous);
}

glm::mat3x3 getStressOpposingInversion(glm::mat3x3 F, glm::mat3x3 V, Eigen::Matrix3f Bm) {
    auto mat = F;
    /*printf("F: %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
           mat[0][0], mat[1][0], mat[2][0],
                       mat[0][1], mat[1][1], mat[2][1],
                       mat[0][2], mat[1][2], mat[2][2]);*/
    Eigen::Matrix3f eigF = glmToEigen(F);
    //std::cout << "eigF: " << eigF;
    auto solver = Eigen::LDLT<Eigen::Matrix3f>(eigF.transpose() * eigF);
    Eigen::Vector3f Fhatsquared = solver.vectorD();
    //std::cout << "Fhatsquared: " << Fhatsquared;
    Eigen::Matrix3f rotV = solver.matrixL();
    float det = rotV.determinant();
    if(det < 0) {
        rotV.col(0) *= -1;
    }
    //printf("det of rotV is %f\n", det);
    //assert(fabs(det) < 1.1 && fabs(det) > 0.9);
    Eigen::Vector3f Fhat(sqrt(Fhatsquared(0)), sqrt(Fhatsquared(1)), sqrt(Fhatsquared(2)));
    Eigen::DiagonalMatrix<float, 3> intermed(Fhat);
    Eigen::Matrix3f Fhatmat(intermed);
    Eigen::DiagonalMatrix<float, 3> invinter(Fhat.cwiseInverse());
    Eigen::Matrix3f Fhatmatcwise(invinter);
    Eigen::Matrix3f rotU = eigF * rotV * Fhatmatcwise;
    float uDet = rotU.determinant();
    if(uDet < 0) {
        int mindx = 0;
        if(Fhat(1) < Fhat(0))
            mindx = 1;
        if(mindx == 0) {
            if(Fhat(2) < Fhat(0))
                mindx = 2;
        }
        else {
            if(Fhat(2) < Fhat(1))
                mindx = 2;
        }
        rotU.col(mindx) *= -1;
        Fhatmat(mindx*4) *= -1;
    }
    //printf("udet: %f, vdet: %f\n", fabs(uDet), fabs(det));
    Eigen::Matrix3f id = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f strainhat = Fhatmat - id;
    Eigen::Matrix3f Phat = 2 * settings.femRigidity * strainhat
            + settings.femIncompressibility * strainhat.trace() * id;
    //std::cout << "F: " << eigF << "\nRotU: " << rotU << "\nPhat: " << Phat << "\nrotV transpose: " << rotV.transpose();
    Eigen::Matrix3f eigV = glmToEigen(V);
    Eigen::Matrix3f Vhat = rotU.transpose() * eigV * rotV;
    Eigen::Matrix3f PVhat = settings.femShearViscosity * (Vhat + Vhat.transpose())
            + settings.femBulkViscosity * Vhat.trace() * id;
    return -eigenToGlm(rotU * Phat * rotV.transpose() * Bm
                      + rotU * PVhat * rotV.transpose() * Bm);
}

void TetMesh::computeStressForces(std::vector<glm::vec3>& forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels) {
    // total force = gravity/other global forces + stress per element
    // stress = elastic stress + viscous stress
    // elastic stress = incompressibility * trace(strain) * ID_3x3 + 2*rigidity * strain
    // viscous stress = coeff_visc_1 * trace(strain rate) * ID_3x3 + 2*coeff_visc_2 * strain rate
    // strain = Ftrans*F - I = dx/du - I, where dx/du = P*barytrans, where P is [p1 - p4, p2 - p4, p3 - p4]
    // strain rate = (dx/du)T * (dv/du) + (dv/du)T * (dx/du), where dv/du = V*barytrans, where V is [v1 - v4, v2 - v4, v3 - v4], v velocities
    // so by computing that, we can get force for each node
    glm::mat3x3 id = glm::mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    std::mutex forcesMutex;
    //for(long unsigned int i = 0;i < m_tets.size(); i++) {
    auto calc_forces_i = [&](int i) {
        auto tet = m_tets[i];
        if(tetInverted(points, tet)) {
            printf("Inverted tet (#%d) found!\n", i);
            fflush(stdout);
        }

        auto p1 = points[tet.p1];
        auto p2 = points[tet.p2];
        auto p3 = points[tet.p3];
        auto p4 = points[tet.p4];

        auto v1 = vels[tet.p1];
        auto v2 = vels[tet.p2];
        auto v3 = vels[tet.p3];
        auto v4 = vels[tet.p4];

        glm::mat3x3 P(p1 - p4, p2 - p4, p3 - p4);
        glm::mat3x3 F = glm::transpose(P * m_baryTransforms[i]);

        glm::mat3x3 D(v1 - v4, v2 - v4, v3 - v4);
        glm::mat3x3 V = glm::transpose(D * m_baryTransforms[i]);

        auto mat = F;
        /*printf("F: %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
               mat[0][0], mat[1][0], mat[2][0],
                           mat[0][1], mat[1][1], mat[2][1],
                           mat[0][2], mat[1][2], mat[2][2]);*/
        /*glm::mat3x3 stress = getStressOpposingInversion(F, V, m_matspace_crosses[i]);

        glm::vec3 p1force = glm::column(stress, 0);
        glm::vec3 p2force = glm::column(stress, 1);
        glm::vec3 p3force = glm::column(stress, 2);
        glm::vec3 p4force = -(p1force + p2force + p3force);*/

        glm::mat3x3 stress = getStress(F, V);

        glm::vec3 p1force = stress * -glm::cross(p4 - p2, p3 - p2);
        glm::vec3 p2force = stress * -glm::cross(p4 - p3, p1 - p3);
        glm::vec3 p3force = stress * -glm::cross(p4 - p1, p2 - p1);
        glm::vec3 p4force = stress * -glm::cross(p2 - p1, p3 - p1);
        //forcesMutex.lock();
        forcePerNode[tet.p1] += p1force;
        forcePerNode[tet.p2] += p2force;
        forcePerNode[tet.p3] += p3force;
        forcePerNode[tet.p4] += p4force;
        //forcesMutex.unlock();
    };

    for(long unsigned int i = 0;i < m_tets.size(); i++) {
        calc_forces_i(i);
    }

}

// number of newtons to apply when penetrated 1  meter^2
#define PENALTY_ACCEL_K 50000.f

void TetMesh::computeCollisionForces(std::vector<glm::vec3> &forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels, float floorY) {
    for(long unsigned int i = 0;i < points.size(); i++) {
        if(points[i].y < floorY) {
            glm::vec3 penaltyForce = m_pointMasses[i] * glm::vec3(0, PENALTY_ACCEL_K * (floorY - points[i].y), 0);
            forcePerNode[i] += penaltyForce;
        }
    }
}

void TetMesh::computeAllForces(std::vector<glm::vec3> &forcePerNode) {
    std::fill(forcePerNode.begin(), forcePerNode.end(), glm::vec3());
    // first add grav
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        forcePerNode[i] += glm::vec3(0, -0.1, 0) * m_pointMasses[i];
    }
    computeStressForces(forcePerNode, m_points, m_vels);
    computeCollisionForces(forcePerNode, m_points, m_vels, -2);
}

void TetMesh::computeAllForcesFrom(std::vector<glm::vec3> &forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels) {
    static int ncalls = 0;
    ncalls++;
    std::fill(forcePerNode.begin(), forcePerNode.end(), glm::vec3());
    // first add grav
    for(long unsigned int i = 0; i < points.size(); i++) {
        forcePerNode[i] += glm::vec3(0, -0.1, 0) * m_pointMasses[i];
        //forcePerNode[i] += glm::vec3(m_points[i].x >= 0 ? 5 : -5, 0, 0) * m_pointMasses[i];
    }
    computeStressForces(forcePerNode, points, vels);
    computeCollisionForces(forcePerNode, points, vels, -2);
}

#define MIN_STRESS_FRACTURE 5000

/*struct tetface {
    int tetidx;
    int p, p2, p3;
    int vertExcluded;
    int pIdx;
};*/

bool TetMesh::markIfContains(const std::unordered_set<glm::ivec2, ivec2_hash>& set, glm::ivec2 vec) {
    if(set.count(vec) || set.count(glm::ivec2(vec.y, vec.x))) {
        m_isCrackTip[vec.x] = true;
        m_isCrackTip[vec.y] = true;
    }
    return true;
}

void TetMesh::markTouchingFaces(const std::unordered_set<glm::ivec2, ivec2_hash>& faceSet, tet_t& tet, int pidx) {
    switch(pidx) {
    case 0:
        markIfContains(faceSet, glm::ivec2(tet[1], tet[2]));
        markIfContains(faceSet, glm::ivec2(tet[2], tet[3]));
        markIfContains(faceSet, glm::ivec2(tet[1], tet[3]));
        break;
    case 1:
        markIfContains(faceSet, glm::ivec2(tet[0], tet[2]));
        markIfContains(faceSet, glm::ivec2(tet[2], tet[3]));
        markIfContains(faceSet, glm::ivec2(tet[0], tet[3]));
        break;
    case 2:
        markIfContains(faceSet, glm::ivec2(tet[0], tet[1]));
        markIfContains(faceSet, glm::ivec2(tet[1], tet[3]));
        markIfContains(faceSet, glm::ivec2(tet[0], tet[3]));
        break;
    case 3:
        markIfContains(faceSet, glm::ivec2(tet[0], tet[1]));
        markIfContains(faceSet, glm::ivec2(tet[1], tet[2]));
        markIfContains(faceSet, glm::ivec2(tet[0], tet[2]));
    }
}

void addTouchingFaces(std::unordered_set<glm::ivec2, ivec2_hash>& faceSet, tet_t& tet, int pidx) {
    switch(pidx) {
    case 0:
        faceSet.insert(glm::ivec2(tet[1], tet[2]));
        faceSet.insert(glm::ivec2(tet[2], tet[3]));
        faceSet.insert(glm::ivec2(tet[1], tet[3]));
        break;
    case 1:
        faceSet.insert(glm::ivec2(tet[0], tet[2]));
        faceSet.insert(glm::ivec2(tet[2], tet[3]));
        faceSet.insert(glm::ivec2(tet[0], tet[3]));
        break;
    case 2:
        faceSet.insert(glm::ivec2(tet[0], tet[1]));
        faceSet.insert(glm::ivec2(tet[1], tet[3]));
        faceSet.insert(glm::ivec2(tet[0], tet[3]));
        break;
    case 3:
        faceSet.insert(glm::ivec2(tet[0], tet[1]));
        faceSet.insert(glm::ivec2(tet[1], tet[2]));
        faceSet.insert(glm::ivec2(tet[0], tet[2]));
    }
}

void TetMesh::computeFracturing() {
    // first, mark points for fracturing.
    static int hasCracked = 0;
    if(hasCracked)
        return;

    float maxEig = -1;
    int bestTip = -1;
    glm::vec3 bestSplit;
    std::unordered_map<int, glm::vec3> toFracture;
    for(int i = 0; i < m_tets.size(); i++) {
        tet_t tet = m_tets[i];

        auto p1 = m_points[tet.p1];
        auto p2 = m_points[tet.p2];
        auto p3 = m_points[tet.p3];
        auto p4 = m_points[tet.p4];

        auto v1 = m_vels[tet.p1];
        auto v2 = m_vels[tet.p2];
        auto v3 = m_vels[tet.p3];
        auto v4 = m_vels[tet.p4];

        glm::mat3x3 P(p1 - p4, p2 - p4, p3 - p4);
        glm::mat3x3 F = glm::transpose(P * m_baryTransforms[i]);

        glm::mat3x3 D(v1 - v4, v2 - v4, v3 - v4);
        glm::mat3x3 V = glm::transpose(D * m_baryTransforms[i]);

        // have to get in world space by premult by F so our split plane is right
        glm::mat3x3 stress = F * getStress(F, V);
        Eigen::Matrix3f rho = glmToEigen(stress);
        Eigen::EigenSolver<Eigen::Matrix3f> solver(rho, true);
        Eigen::Vector3cf eig_values = solver.eigenvalues();
        int maxidx = -1;
        float maxval = -1;
        for(long unsigned int i = 0;i < 3; i++) {
            float val = std::abs(eig_values(i));
            if(val > maxval) {
                maxval = val;
                maxidx = i;
            }
        }

        if(maxval > MIN_STRESS_FRACTURE && maxval > maxEig) {
            maxEig = maxval;

            int tip;
            std::vector<int> crackTips;
            crackTips.reserve(4);
            if(m_isCrackTip[tet.p1])
                crackTips.push_back(tet.p1);
            if(m_isCrackTip[tet.p2])
                crackTips.push_back(tet.p2);
            if(m_isCrackTip[tet.p3])
                crackTips.push_back(tet.p3);
            if(m_isCrackTip[tet.p4])
                crackTips.push_back(tet.p4);

            if(crackTips.size() == 0) {
                tip = (int[]){tet.p1, tet.p2, tet.p3, tet.p4}[rand() % 4];
            }
            else {
                tip = crackTips[rand() % crackTips.size()];
            }
            Eigen::Vector3cf eigvector = solver.eigenvectors().col(maxidx);
            glm::vec3 splitNormal(std::abs(eigvector(0)), std::abs(eigvector(1)), std::abs(eigvector(2)));
            splitNormal = glm::normalize(splitNormal);
            //toFracture[tip] = splitNormal;
            //m_isCrackTip[tip] = false;
            bestTip = tip;
            bestSplit = splitNormal;
            printf("Cracking at point %d!\n", tip);
        }
    }
    if(maxEig == -1)
        return;
    toFracture[bestTip] = bestSplit;
    m_isCrackTip[bestTip] = false;
    // go through all the points to be fractured and do it
    for(auto it = toFracture.begin(); it != toFracture.end(); it++) {
        int tip = it->first;
        glm::vec3 splitNormal = it->second;
        // make a copy of the point which will be on the negative side of the fracture plane after frac


        // now iterate through tets touching the point, separate into those on + side and those on - side
        const std::unordered_set<glm::ivec2, ivec2_hash> *tetsTouching = &m_pToTMap[tip];
        std::vector<glm::ivec2> plusSide, minusSide;
        for(auto it = tetsTouching->begin(); it != tetsTouching->end(); it++) {
            glm::ivec2 tup = *it;
            int tetidx = tup.x;
            tet_t tet = m_tets[tetidx];
            if(glm::dot(splitNormal, getCenter(m_points, tet) - m_points[tip]) < 0) {
                // behind plane, swap to other point
                /*if(tet.p1 == tip)
                    m_tets[tetidx].p1 = pminus;
                if(tet.p2 == tip)
                    m_tets[tetidx].p2 = pminus;
                if(tet.p3 == tip)
                    m_tets[tetidx].p3 = pminus;
                if(tet.p4 == tip)
                    m_tets[tetidx].p4 = pminus;
                tetsTouching->erase(tetsTouching->begin() + i);
                i--;
                m_pToTMap[pminus].push_back(tetidx);*/
                minusSide.push_back(tup);
            }
            else {
                plusSide.push_back(tup);
            }
        }
        if(minusSide.size() == 0 || plusSide.size() == 0)
            continue;
        int pminus = addNewPoint();
        m_points[pminus] = m_points[tip];
        m_vels[pminus] = m_vels[tip];
        m_vels[tip] += maxEig * splitNormal / 2.f / 50000.f;
        m_vels[pminus] -= maxEig * splitNormal / 2.f / 50000.f;
        m_pointMasses[pminus] = m_pointMasses[tip] / 2;
        m_pointMasses[tip] = m_pointMasses[pminus];
        //m_isCrackTip[pminus] = true;
        // now we have to find all pairs of tets on plus and minus side that share a face that includes
        // the tip. This is really annoying; better way???
        std::unordered_set<glm::ivec2, ivec2_hash> plusFaces;
        for(long unsigned int i = 0; i < plusSide.size(); i++) {
            glm::ivec2 tup = plusSide[i];
            int tetidx = tup.x;
            int pidx = tup.y;
            tet_t ti = m_tets[tetidx];
            assert(ti[pidx] == tip);

            addTouchingFaces(plusFaces, ti, pidx);
        }
        for(unsigned long i = 0; i < minusSide.size(); i++) {
            glm::ivec2 tup = minusSide[i];
            int tetidx = tup.x;
            int pidx = tup.y;
            tet_t ti = m_tets[tetidx];
            assert(ti[pidx] == tip);

            markTouchingFaces(plusFaces, ti, pidx);
            m_tets[tetidx][pidx] = pminus;
            m_pToTMap[tip].erase(tup);
            m_pToTMap[pminus].insert(tup);
        }
    }
    if(toFracture.size()) {
        this->calcFaces();
    }
}

void TetMesh::update(float timestep) {
    // step 1: compute all fractures.
    //computeFracturing();

    std::vector<glm::vec3> forces(m_points.size()),
            dxk1(m_points.size()),
            dxk2(m_points.size()),
            dxk3(m_points.size()),
            dxk4(m_points.size()),
            xnext(m_points.size()),
            dvk1(m_points.size()),
            dvk2(m_points.size()),
            dvk3(m_points.size()),
            dvk4(m_points.size()),
            vnext(m_points.size());
    // We use RK4, which is an advanced explicit integration technique in which we find derivatives
    // at multiple sample points and average them to get the final derivative we use to move the
    // simulation forward one timestep.

    // P1: Start by calculating derivatives at current pos+velocity
    computeAllForcesFrom(forces, m_points, m_vels);
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk1[i] = m_vels[i];
        dvk1[i] = accel; // compute k1 + p
        //vtemp[i] = m_vels[i] + velIncrement
        xnext[i] = m_points[i] + dxk1[i] * 0.5f * timestep;
        vnext[i] = m_vels[i] + dvk1[i] * 0.5f * timestep;
    }
    // P2: Move pos+velocity half of a timestep from orig using derivatives from P1, calculate derivatives
    computeAllForcesFrom(forces, xnext, vnext);
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk2[i] = vnext[i];
        dvk2[i] = accel;
        xnext[i] = m_points[i] + dxk2[i] * 0.5f * timestep;
        vnext[i] = m_vels[i] + dvk2[i] * 0.5f * timestep;
    }
    // P3: Move pos+velocity half of a timestep from orig using derivatives from P2, calculate derivatives
    computeAllForcesFrom(forces, xnext, vnext);
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk3[i] = vnext[i];
        dvk3[i] = accel;
        xnext[i] = m_points[i] + dxk3[i] * 1.0f * timestep;
        vnext[i] = m_vels[i] + dvk3[i] * 1.0f * timestep;
    }
    // P4: Move pos+velocity a full timestep from orig using derivatives from P3, calculate derivatives
    computeAllForcesFrom(forces, xnext, vnext);
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk4[i] = vnext[i];
        dvk4[i] = accel;
    }
    // Take final derivatives to be (derivs(P1) + 2*derivs(P2) + 2*derivs(P3) + derivs(P4))/6, i.e.
    // a weighted average, then move pos+velocity a full timestep from the orig using those derivatives
    for(long unsigned int i = 0;i < m_points.size(); i++) {
        m_points[i] += timestep * (dxk1[i] + dxk4[i] + 2.f*(dxk2[i] + dxk3[i])) / 6.f;
        m_vels[i] += timestep * (dvk1[i] + dvk4[i] + 2.f*(dvk2[i] + dvk3[i])) / 6.f;
    }

    calcNorms();
}

void getNormalsFromFaces(const std::unordered_map<glm::ivec3, bool, ivec3_hash>& faces, const std::vector<glm::vec3>& points, std::vector<glm::vec3>& norms) {
    std::fill(norms.begin(), norms.end(), glm::vec3());
    for(auto it = faces.begin(); it != faces.end(); it++) {
        // tris (oriented s.t. pointing outwards) are:
        // 124, 234, 314, 132
        // to get cross of tri XYZ, we do Z-Y x Y-X
        // by crossing w/o normalizing, we weight by SA

        int p1idx = it->first.x;
        int p2idx = it->first.y;
        int p3idx = it->first.z;

        auto p1 = points[p1idx];
        auto p2 = points[p2idx];
        auto p3 = points[p3idx];

        auto orientedSA = glm::cross(p3 - p2, p1 - p2);

        norms[p1idx] += orientedSA;
        norms[p2idx] += orientedSA;
        norms[p3idx] += orientedSA;
    }
    // XXX For now, we don't normalize norms b/c GL is fine with it
}

void TetMesh::calcBaryTransforms() {
    // calculate the barycentric coordinate transform (from point in mat space to point in tetra's bary coordinate space)
    assert(m_baryTransforms.size() == m_tets.size());
    for(long unsigned int i = 0;i < m_tets.size(); i++) {
        auto tet = m_tets[i];
        auto v1 = m_points[tet.p1];
        auto v2 = m_points[tet.p2];
        auto v3 = m_points[tet.p3];
        auto v4 = m_points[tet.p4];
        m_baryTransforms[i] = glm::inverse(glm::mat3x3(v1 - v4, v2 - v4, v3 - v4));
    }
}

#define MAT_DENSITY 1200

void TetMesh::calcPointMasses() {
    // calculate masses for each point based on tet volumes
    assert(m_pointMasses.size() == m_points.size());
    std::fill(m_pointMasses.begin(), m_pointMasses.end(), 0);
    float maxvol = -1, minvol = INFINITY;
    for(long unsigned int i = 0;i < m_tets.size(); i++) {
        // let density = 4000 (i.e. water*4), then we can add tet vol to each node mass
        tet_t tet = m_tets[i];
        auto p1 = m_points[tet.p1];
        auto p2 = m_points[tet.p2];
        auto p3 = m_points[tet.p3];
        auto p4 = m_points[tet.p4];
        float volume = MAT_DENSITY*glm::length(glm::dot(p1 - p2, glm::cross(p4 - p2, p3 - p2)))/4;
        if(volume > maxvol)
            maxvol = volume;
        if(volume < minvol) {
            minvol = volume;
            printf("at min:\np1: %f,%f,%f\np2: %f,%f,%f\np3: %f,%f,%f\np4: %f,%f,%f\n",
                   p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, p4.x, p4.y, p4.z);
        }
        m_pointMasses[tet.p1] += volume;
        m_pointMasses[tet.p2] += volume;
        m_pointMasses[tet.p3] += volume;
        m_pointMasses[tet.p4] += volume;
    }
    float sum = 0;
    float minm = INFINITY;
    for(long unsigned int i = 0;i < m_pointMasses.size(); i++) {
        sum += m_pointMasses[i];
        if(m_pointMasses[i] < minm)
            minm = m_pointMasses[i];

    }
    printf("Total mass is %f, smallest mass is %f w/ inverse %f\nMax volume is %f, min volume is %f", sum, minm, 1.f/minm, maxvol, minvol);
    /*float min_allowed_mass = sum / m_points.size() / 100;
    for(long unsigned int i = 0;i < m_pointMasses.size(); i++) {
        if(m_pointMasses[i] < min_allowed_mass) {
            sum += min_allowed_mass - m_pointMasses[i];
            m_pointMasses[i] = min_allowed_mass;
        }
    }
    printf("After culling low masses, new total mass is %f\n", sum);*/
}

bool setIfContains(std::unordered_map<glm::ivec3, bool, ivec3_hash>& map, glm::ivec3 vec) {
    bool good = true;
#define V(a, b, c) glm::ivec3(vec[a], vec[b], vec[c])
    if(map.count(vec))
        map[vec] = good = false;
    if(map.count(V(0, 2, 1)))
        map[V(0, 2, 1)] = good = false;
    if(map.count(V(1, 0, 2)))
        map[V(1, 0, 2)] = good = false;
    if(map.count(V(1, 2, 0)))
        map[V(1, 2, 0)] = good = false;
    if(map.count(V(2, 0, 1)))
        map[V(2, 0, 1)] = good = false;
    if(map.count(V(2, 1, 0)))
        map[V(2, 1, 0)] = good = false;
    if(good)
        map[vec] = true;
#undef V
    return good;
}



void TetMesh::calcFacesAndNorms() {
    calcFaces();
    calcNorms();
}

void TetMesh::calcFaces() {
    m_faces.clear();
    // we will extract the surface mesh by looking at every face and finding which are occluded
    for(long unsigned int i = 0; i < m_tets.size(); i++) {
        tet_t tet = m_tets[i];
        glm::ivec3 f1(tet.p1, tet.p2, tet.p4),
                f2(tet.p2, tet.p3, tet.p4),
                f3(tet.p3, tet.p1, tet.p4),
                f4(tet.p1, tet.p3, tet.p2);
        setIfContains(m_faces, f1);
        setIfContains(m_faces, f2);
        setIfContains(m_faces, f3);
        setIfContains(m_faces, f4);
    }
    for(auto it = m_faces.begin(); it != m_faces.end();) {
        if(it->second)
            it++;
        else {
            it = m_faces.erase(it); // kill non-outward faces
        }
    }
}

void TetMesh::calcNorms() {
    getNormalsFromFaces(m_faces, m_points, m_norms);
}

void TetMesh::draw() {
    //update(1);
    std::vector<GLfloat> vertexData;
    for(auto it = m_faces.begin(); it != m_faces.end(); it++) {
        if(!it->second)
            continue;
        int p1idx = it->first.x;
        int p2idx = it->first.y;
        int p3idx = it->first.z;
#define pushXYZ(vd, v) vd.push_back(v.x); \
        vd.push_back(v.y); \
        vd.push_back(v.z);

        pushXYZ(vertexData, m_points[p1idx]);
        pushXYZ(vertexData, m_norms[p1idx]);
        pushXYZ(vertexData, m_points[p2idx]);
        pushXYZ(vertexData, m_norms[p2idx]);
        pushXYZ(vertexData, m_points[p3idx]);
        pushXYZ(vertexData, m_norms[p3idx]);
#undef pushXYZ
    }
    OpenGLShape shape;
    shape.setVertexData(vertexData.data(), vertexData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, vertexData.size() / 6);
    shape.setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    shape.setAttribute(ShaderAttrib::NORMAL, 3, sizeof(GLfloat) * 3, VBOAttribMarker::DATA_TYPE::FLOAT, true);
    shape.buildVAO();
    shape.draw();
}
