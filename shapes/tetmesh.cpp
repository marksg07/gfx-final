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
#include <unordered_set>
#include "Settings.h"

std::size_t hash_ivec3_fn(const glm::ivec3& v) {
    hashh<int, int, int> hasher;
    return hasher(v.x, v.y, v.z);

}

inline double get_time(void) {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_usec * 0.000001 + t.tv_sec);
}

inline glm::vec3 getPoint(const tetgenio& t, int i) {
    assert(i < t.numberofpoints);
    return glm::vec3(t.pointlist[3 * i], t.pointlist[3 * i + 1], t.pointlist[3 * i + 2]);
}

std::vector<glm::vec3> computeNorms(const tetgenio& t) {
    // This should happen pre-getTets
    printf("Computing per-point norms\n");
    std::vector<glm::vec3> norms;
    norms.resize(t.numberofpoints);
    // we go through each tetrahedron and for each triangle, add its contribution (surface area of triangle) to constituent points
    for(int i = 0; i < t.numberoftetrahedra; i++) {
        // tris (oriented s.t. pointing outwards) are:
        // 124, 234, 314, 132
        // to get cross of tri XYZ, we do Z-Y x Y-X
        // by crossing w/o normalizing, we weight by SA
        int p1idx = t.tetrahedronlist[i * 4];
        int p2idx = t.tetrahedronlist[i * 4 + 1];
        int p3idx = t.tetrahedronlist[i * 4 + 2];
        int p4idx = t.tetrahedronlist[i * 4 + 3];
        printf("npoints: %d, pidxs: %d, %d, %d, %d\n", t.numberofpoints, p1idx, p2idx, p3idx, p4idx);
        fflush(stdout);
        auto p1 = getPoint(t, p1idx);
        auto p2 = getPoint(t, p2idx);
        auto p3 = getPoint(t, p3idx);
        auto p4 = getPoint(t, p4idx);

        auto orientedSA124 = glm::cross(p4 - p2, p1 - p2);
        auto orientedSA234 = glm::cross(p4 - p3, p2 - p3);
        auto orientedSA314 = glm::cross(p4 - p1, p3 - p1);
        auto orientedSA132 = glm::cross(p2 - p3, p1 - p3);

        norms[p1idx] += orientedSA124 + orientedSA314 + orientedSA132;
        norms[p2idx] += orientedSA124 + orientedSA234 + orientedSA132;
        norms[p3idx] += orientedSA234 + orientedSA314 + orientedSA132;
        norms[p4idx] += orientedSA124 + orientedSA234 + orientedSA314;
    }
    // XXX For now, we don't normalize norms b/c GL is fine with it
    return norms;
}

/*std::vector<Tetrahedron> getTets(const tetgenio& t, std::vector<glm::vec3> norms) {
    // assume has loaded nodes and eles
    printf("Getting tets from tetgenio\n");
    printf("%d tetrahedra\n", t.numberoftetrahedra);
    assert(t.numberofcorners == 4);
    std::vector<Tetrahedron> tets;
    tets.reserve(t.numberoftetrahedra);
    for(int i = 0; i < t.numberoftetrahedra; i++) {
        int p1idx = t.tetrahedronlist[i * 4];
        int p2idx = t.tetrahedronlist[i * 4 + 1];
        int p3idx = t.tetrahedronlist[i * 4 + 2];
        int p4idx = t.tetrahedronlist[i * 4 + 3];

        auto p1 = getPoint(t, p1idx);
        auto p2 = getPoint(t, p2idx);
        auto p3 = getPoint(t, p3idx);
        auto p4 = getPoint(t, p4idx);

        auto n1 = norms[p1idx];
        auto n2 = norms[p2idx];
        auto n3 = norms[p3idx];
        auto n4 = norms[p4idx];

        tets.push_back(Tetrahedron(p1, p2, p3, p4, n1, n2, n3, n4));
    }
    return tets;
}*/

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

std::unordered_map<int, std::vector<int>> tetsTouchingPoint(const std::vector<tet_t>& tets) {
    std::unordered_map<int, std::vector<int>> pToTMap;
    for(int i = 0; i < tets.size(); i++) {
        tet_t tet = tets[i];
        for(int j = 0; j < 4; j++) {
            int pidx = tet[j];
            if(pToTMap.count(pidx) == 0) {
                pToTMap[pidx] = std::vector<int>();
            }
            pToTMap[pidx].push_back(i);
        }
    }
    return pToTMap;
}

std::vector<glm::vec3> getPoints(const tetgenio& t) {
    std::vector<glm::vec3> ret;
    ret.reserve(t.numberofpoints);
    for(int i = 0; i < t.numberofpoints; i++) {
        ret.push_back(getPoint(t, i));
    }
    return ret;
}

bool strIsIntable(const std::string& str) {
    if(str.length() == 0)
        return false;
    if(str[0] == '-' || str[0] == '+') {
        return strIsIntable(str.substr(1));
    }
    return str.find_first_not_of("0123456789") == std::string::npos; // i.e. no non-digits
}

int getFirstIntFromVTN(const std::string& vtn) {
    if(strIsIntable(vtn))
        return std::stoi(vtn);
    else {
        std::string v = vtn.substr(0, vtn.find('/'));
        assert(strIsIntable(v));
        return std::stoi(v);
    }
}

tetgenio convertObjFile(std::string objfilename) {
    std::ifstream infile(objfilename);
    std::string line;
    std::vector<REAL> vertCoords;
    std::vector<int> faceIdxs;
    while(std::getline(infile, line)) {
        line.erase(0, line.find_first_not_of(" \n\r\t"));
        if(line[0] == '#')
            continue;
        else if(line[0] == 'v') {
            if(line[1] == 'n' || line[1] == 't' || line[1] == 'p') {
                // texcors, normals, or param space all ignored
                continue;
            }
            // now we make a stream of floats representing the x,y,z[,w] and read from it
            std::istringstream strm(&line[1]);
            float x = 99, y = 99, z = 99;
            if(!(strm >> x >> y >> z)) {
                printf("Attempted to read line:\n%s\nAs string stream of x,y,z, but failed.\n", line.data());
                printf("Read: %f, %f, %f\n", x, y, z);
                continue;
            }
            vertCoords.push_back(x);
            vertCoords.push_back(y);
            vertCoords.push_back(z);
        }
        else if(line[0] == 'f') {
            // we make a stream of ints representing the triangle corners
            std::istringstream strm(&line[1]);
            std::string v1, v2, v3, tmp;
            int i1, i2, i3;
            if(!(strm >> v1 >> v2 >> v3)) {
                printf("Attempted to read line:\n%s\nAs string stream of v1,v2,v3, but failed.\n", line.data());
                continue;
            }
            if(strm >> tmp) {
                // > 3 verts
                printf("Line parsed as:\n%s\nHas >3 vertices listed. Ignoring addl verts.\n", line.data());
            }
            i1 = getFirstIntFromVTN(v1);
            i2 = getFirstIntFromVTN(v2);
            i3 = getFirstIntFromVTN(v3);
            // -1 because .obj indexes from 1
            faceIdxs.push_back(i1 - 1);
            faceIdxs.push_back(i2 - 1);
            faceIdxs.push_back(i3 - 1);
        }
    }
    // XXX The reason we are not directly initializing a tetgenio object is because there's a bunch of random stuff that has to be initialized.
    std::ofstream ofile("tmp.smesh");
    ofile << "# tmp.smesh autogenerated by convertObjFile(" << objfilename << ")" << std::endl;
    // number nodes, number dims (always 3), number attributes (currently 0), has boundary marker (no)
    ofile << vertCoords.size() / 3 << " " << 3 << " " << 0 << " " << 0 << std::endl;
    ofile << std::endl;
    for(int i = 0; i < vertCoords.size() / 3; i++) {
        ofile << i << " ";
        ofile << vertCoords[i * 3] << " ";
        ofile << vertCoords[i * 3 + 1] << " ";
        ofile << vertCoords[i * 3 + 2];
        ofile << std::endl;
    }
    ofile << std::endl;
    // number facets, has boundary marker (no)
    ofile << faceIdxs.size() / 3 << " " << 0 << std::endl;
    ofile << std::endl;
    for(int i = 0; i < faceIdxs.size() / 3; i++) {
        ofile << 3 << " ";
        ofile << faceIdxs[i * 3] << " ";
        ofile << faceIdxs[i * 3 + 1] << " ";
        ofile << faceIdxs[i * 3 + 2];
        ofile << std::endl;
    }
    ofile << std::endl;
    // number holes (0)
    ofile << 0 << std::endl;
    tetgenio tgio;
    printf("About to tgio.load_poly...\n");
    fflush(stdout);
    tgio.load_poly("tmp");
    printf("Done with tgio.load_poly.\n");
    fflush(stdout);
    return tgio;
}

tetgenbehavior tet_behavior;
bool baba = tet_behavior.parse_commandline("p");

TetMesh::TetMesh(std::string filename, std::string nodefile) {
    m_material = {0.16, 0.003, 0.16, 0.003};
    assert(baba);
    int ext_idx = filename.rfind('.');
    if(ext_idx == -1) {
        printf("Warning: file %s opened as TetMesh, but could not be read. Not loading.\n", filename.data());
        return;
    }
    std::string ext = filename.substr(ext_idx);
    std::string noext = filename.substr(0, ext_idx);
    if(ext.compare(".obj") == 0) {
        // TODO: parse obj, create tetgenio, tetrahedralize
        tetgenio in = convertObjFile(filename);
        tetgenio out;
        printf("About to tetrahedralize...\n");
        fflush(stdout);
        double start = get_time();
        tetrahedralize(&tet_behavior, &in, &out);
        out.save_elements("tmp");
        printf("Done tetrahedralizing. Took %f secs.\n", get_time() - start);
        fflush(stdout);
        m_tets = getTets(out);
        m_baryTransforms.resize(m_tets.size());
        m_points = getPoints(out);
        m_norms.resize(m_points.size());
        m_vels.resize(m_points.size());
        m_pointInvMasses.resize(m_points.size());
        m_pToTMap = tetsTouchingPoint(m_tets);
        calcFacesAndNorms();
    }
    else if(ext.compare(".smesh") == 0) {
        // TODO: convert to .ele

        tetgenio in;
        in.load_poly((char *)noext.c_str());
        tetgenio out;

        tetrahedralize(&tet_behavior, &in, &out);
        m_tets = getTets(out);
        m_baryTransforms.resize(m_tets.size());
        m_points = getPoints(out);
        m_norms.resize(m_points.size());
        m_vels.resize(m_points.size());
        m_pointInvMasses.resize(m_points.size());
        m_pToTMap = tetsTouchingPoint(m_tets);
        calcFacesAndNorms();

    }
    else if(ext.compare(".ele") == 0) {
        // TODO: read file
        if(nodefile.length() == 0) {
            nodefile = filename.substr(0, ext_idx) + ".node";
            printf("Nodefile not given for file %s, assuming nodefile is %s.\n", filename.data(), nodefile.data());
            FILE *f = fopen(nodefile.c_str(), "r");
            if(!f) {
                printf("Assumption failed as %s does not exist.\n", nodefile.data());
                return;
            }
            fclose(f);
        }
        tetgenio tets;
        tets.load_tet((char*)noext.c_str());
        int nodeext_idx = nodefile.rfind('.');
        if(nodeext_idx == -1) {
            printf("Warning: file %s opened as nodefile, but no extension found. Not loading.\n", nodefile.data());
            return;
        }
        tets.load_node((char*)nodefile.substr(0, nodeext_idx).c_str());
        m_tets = getTets(tets);
        m_baryTransforms.resize(m_tets.size());
        m_points = getPoints(tets);
        m_norms.resize(m_points.size());
        m_vels.resize(m_points.size());
        m_pointInvMasses.resize(m_points.size());
        m_pToTMap = tetsTouchingPoint(m_tets);
        calcFacesAndNorms();
    }
    else if(ext.compare(".node") == 0) {
        printf("Warning: file %s opened as TetMesh, but is a .node file (list of points). You might have meant to open the associated .poly (for 2D mesh) or .ele (for 3D tetmesh) file.\n", filename.data());
        return;
    }
    else {
        printf("Warning: file %s opened as TetMesh, but could not be read. Not loading.\n", filename.data());
    }
    printf("Tets loaded: %d\n", m_tets.size());
    printf("m_faces size is now %d\n", m_faces.size());
    calcBaryTransforms();
    calcPointInvMasses();
    srand(time(NULL));
    for(int i = 0; i < 1; i++) {
        auto it = m_faces.begin(); // get random pt
        int id = rand() % m_faces.size();
        std::advance(it, id);
        printf("%d\n", id);
        assert(it->second);
        int p1idx = it->first.x;
        m_points[p1idx] += glm::normalize(m_norms[p1idx]) / 1.f;// * 5.f;
    }
    calcNorms();
    printf("N surface faces: %d\n", m_faces.size());
    for(int i = 0; i < m_points.size(); i++) {
        //printf("Point is at: %f,%f,%f, Norm is at: %f,%f,%f\n", m_points[i].x, m_points[i].y, m_points[i].z, m_norms[i].x, m_norms[i].y, m_norms[i].z);
    }
}



TetMesh::TetMesh(object_node_t node, std::unordered_map<std::string, std::unique_ptr<TetMesh>>& map) : TetMesh(node.primitive.meshfile) {
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
}

std::vector<TetMesh> TetMesh::fracture(int pointIdx, glm::vec3 fracNorm) {
    auto pt = m_points[pointIdx];
    m_points.push_back(m_points[pointIdx]);
    int point2Idx = m_points.size() - 1;
    //for()
}

inline float randFloat() {
    return ((float)rand())/RAND_MAX*2-1;
}

void TetMesh::computeStressForces(std::vector<glm::vec3>& forcePerNode) {
    // total force = gravity/other global forces + stress per element
    // stress = elastic stress + viscous stress
    // elastic stress = incompressibility * trace(strain) * ID_3x3 + 2*rigidity * strain
    // viscous stress = coeff_visc_1 * trace(strain rate) * ID_3x3 + 2*coeff_visc_2 * strain rate
    // strain = Ftrans*F - I = dx/du - I, where dx/du = P*barytrans, where P is [p1 - p4, p2 - p4, p3 - p4]
    // strain rate = (dx/du)T * (dv/du) + (dv/du)T * (dx/du), where dv/du = V*barytrans, where V is [v1 - v4, v2 - v4, v3 - v4], v velocities
    // so by computing that, we can get force for each node
    glm::mat3x3 id = glm::mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    for(int i = 0; i < m_tets.size(); i++) {
        auto tet = m_tets[i];
        auto p1 = m_points[tet.p1];
        auto p2 = m_points[tet.p2];
        auto p3 = m_points[tet.p3];
        auto p4 = m_points[tet.p4];
        if(p1 == p2 || p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4 || p3 == p4)
            continue;

        glm::mat3x3 P(p1 - p4, p2 - p4, p3 - p4);

        glm::mat3x3 dx = P * m_baryTransforms[i];
        glm::mat3x3 dxT = glm::transpose(dx);

        glm::mat3x3 strain = dxT * dx - id;

        glm::mat3x3 stress_elastic = /*m_material.incompressibility*/ settings.femIncompressibility * (strain[0][0] + strain[1][1] + strain[2][2]) * id
                + 2 * settings.femRigidity /*m_material.rigidity*/ * strain;

        auto v1 = m_vels[tet.p1];
        auto v2 = m_vels[tet.p2];
        auto v3 = m_vels[tet.p3];
        auto v4 = m_vels[tet.p4];

        glm::mat3x3 V(v1 - v4, v2 - v4, v3 - v4);

        glm::mat3x3 dv = V * m_baryTransforms[i];
        glm::mat3x3 dvT = glm::transpose(dv);

        glm::mat3x3 strain_rate = dxT * dv + dvT * dx;

        glm::mat3x3 stress_viscous = /*m_material.viscous1*/ settings.femBulkViscosity * (strain_rate[0][0] + strain_rate[1][1] + strain_rate[2][2]) * id
                + 2 * settings.femShearViscosity /*m_material.viscous2*/ * strain_rate;

        glm::mat3x3 stress_total = stress_elastic + stress_viscous;
        glm::mat3x3 stress_t_ws = dx * stress_total;
        glm::vec3 cross234 = glm::cross(p4 - p2, p3 - p2);
        float invvolume = 1.f;//glm::length(glm::dot(p1 - p2, cross234))*1000000; // 100000 is weight
        glm::vec3 p1force = stress_t_ws * cross234 * invvolume;
        glm::vec3 p2force = stress_t_ws * glm::cross(p4 - p3, p1 - p3) * invvolume;
        glm::vec3 p3force = stress_t_ws * glm::cross(p4 - p1, p2 - p1) * invvolume;
        glm::vec3 p4force = stress_t_ws * glm::cross(p2 - p1, p3 - p1) * invvolume;
        forcePerNode[tet.p1] -= p1force;
        forcePerNode[tet.p2] -= p2force;
        forcePerNode[tet.p3] -= p3force;
        forcePerNode[tet.p4] -= p4force;
    }
}

void TetMesh::update(float timestep) {
    printf("updating w/ step %f\n", timestep);
    // step 1: get all forces.
    std::vector<glm::vec3> forcePerNode;
    forcePerNode.resize(m_points.size());
    // Gravity:
    //std::fill(forcePerNode.begin(), forcePerNode.end(), glm::vec3(0, -9.81, 0));
    // Stress (elastic + viscous):
    computeStressForces(forcePerNode);
    // step 2:
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forcePerNode[i] * m_pointInvMasses[i];
        m_points[i] += m_vels[i] * timestep + (accel * timestep) / 2.f;
        m_vels[i] += accel * timestep;
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
    for(int i = 0; i < m_tets.size(); i++) {
        auto tet = m_tets[i];
        auto v1 = m_points[tet.p1];
        auto v2 = m_points[tet.p2];
        auto v3 = m_points[tet.p3];
        auto v4 = m_points[tet.p4];
        m_baryTransforms[i] = glm::inverse(glm::mat3x3(v1 - v4, v2 - v4, v3 - v4));
    }
}

void TetMesh::calcPointInvMasses() {
    // calculate masses for each point based on tet volumes
    assert(m_pointInvMasses.size() == m_points.size());
    std::fill(m_pointInvMasses.begin(), m_pointInvMasses.end(), 0);
    for(int i = 0; i < m_tets.size(); i++) {
        // let density = 4000 (i.e. water*4), then we can add tet vol to each node mass
        tet_t tet = m_tets[i];
        auto p1 = m_points[tet.p1];
        auto p2 = m_points[tet.p2];
        auto p3 = m_points[tet.p3];
        auto p4 = m_points[tet.p4];
        float volume = 1000*glm::length(glm::dot(p1 - p2, glm::cross(p4 - p2, p3 - p2)));
        m_pointInvMasses[tet.p1] += volume;
        m_pointInvMasses[tet.p2] += volume;
        m_pointInvMasses[tet.p3] += volume;
        m_pointInvMasses[tet.p4] += volume;
    }
    for(int i = 0; i < m_pointInvMasses.size(); i++) {
        m_pointInvMasses[i] = 1/m_pointInvMasses[i];
    }
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
    m_faces.clear();
    // we will extract the surface mesh by looking at every face and finding which are occluded
    printf("Hash of 1,2,3 is %d, Hash of 1,3,2 is %d\n", hash_ivec3_fn(glm::ivec3(1, 2, 3)), hash_ivec3_fn(glm::ivec3(1, 3, 2)));
    std::unordered_map<glm::ivec3, bool, ivec3_hash> abacus;
    abacus[glm::ivec3(1, 2, 3)] = false;
    abacus[glm::ivec3(1, 3, 2)] = true;
    for(int i = 0; i < m_tets.size(); i++) {
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
    calcNorms();

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
