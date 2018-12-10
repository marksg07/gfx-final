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
//#include <Eigen/Dense>

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

void convertObjFile(std::string objfilename, tetgenio *out) {
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
    printf("About to tgio.load_poly...\n");
    fflush(stdout);
    out->load_poly("tmp");
    printf("Done with tgio.load_poly.\n");
    fflush(stdout);
}

void convertMeshFile(std::string meshFileName, tetgenio *out) {
    std::ifstream in(meshFileName);
    std::string line;
    std::vector<float> vertCoords;
    std::vector<int> tetCoords;
    while(std::getline(in, line)) {
        if(line[0] == 'v') {
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
        else if(line[0] == 't') {
            std::istringstream strm(&line[1]);
            int x = 99, y = 99, z = 99, w = 99;;
            if(!(strm >> x >> y >> z >> w)) {
                printf("Attempted to read line:\n%s\nAs string stream of x,y,z,w ints but failed.\n", line.data());
                printf("Read: %d, %d, %d, %d\n", x, y, z, w);
                continue;
            }
            tetCoords.push_back(x);
            tetCoords.push_back(y);
            tetCoords.push_back(z);
            tetCoords.push_back(w);
        }
    }
    std::ofstream ofile("tmp.ele");
    // first ele. Line is num tets, num verts/shape (4 for tet), 0 boundary
    ofile << tetCoords.size()/4 << " " << 4 << " " << 0;
    ofile << std::endl;
    for(int i = 0; i < tetCoords.size()/4; i++) {
        ofile << i << " ";
        ofile << tetCoords[i*4] << " ";
        ofile << tetCoords[i*4+1] << " ";
        ofile << tetCoords[i*4+2] << " ";
        ofile << tetCoords[i*4+3];
        ofile << std::endl;
    }
    std::ofstream onodes("tmp.node");
    // Line is num nodes, dimension, num attributes, 0 for boundary
    onodes << vertCoords.size()/3 << " " << 3 << " " << 0 << " " << 0;
    onodes << std::endl;
    for(int i = 0; i < vertCoords.size()/3; i++) {
        onodes << i << " ";
        onodes << vertCoords[i*3] << " ";
        onodes << vertCoords[i*3+1] << " ";
        onodes << vertCoords[i*3+2] << " ";
        onodes << std::endl;
    }

    printf("About to tgio.load_node...\n");
    fflush(stdout);
    out->load_node("tmp");
    printf("Done with tgio.load_node.\n");
    printf("About to tgio.load_tet...\n");
    out->load_tet("tmp");
    printf("Done with tgio.load_tet.\n");
    fflush(stdout);
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
    tetgenio out;
    if(ext.compare(".obj") == 0) {
        // TODO: parse obj, create tetgenio, tetrahedralize
        tetgenio in;
        convertObjFile(filename, &in);
        printf("About to tetrahedralize...\n");
        fflush(stdout);
        double start = get_time();
        tetrahedralize(&tet_behavior, &in, &out);
        out.save_elements("tmp");
        printf("Done tetrahedralizing. Took %f secs.\n", get_time() - start);
        fflush(stdout);
    }
    else if(ext.compare(".smesh") == 0) {
        // TODO: convert to .ele
        tetgenio in;
        in.load_poly((char *)noext.c_str());
        tetrahedralize(&tet_behavior, &in, &out);
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
        out.load_tet((char*)noext.c_str());
        int nodeext_idx = nodefile.rfind('.');
        if(nodeext_idx == -1) {
            printf("Warning: file %s opened as nodefile, but no extension found. Not loading.\n", nodefile.data());
            return;
        }
        out.load_node((char*)nodefile.substr(0, nodeext_idx).c_str());
    }
    else if(ext.compare(".mesh") == 0) {
        convertMeshFile(filename, &out);
    }
    else if(ext.compare(".node") == 0) {
        printf("Warning: file %s opened as TetMesh, but is a .node file (list of points). You might have meant to open the associated .poly (for 2D mesh) or .ele (for 3D tetmesh) file.\n", filename.data());
        return;
    }
    else {
        printf("Warning: file %s opened as TetMesh, but could not be read. Not loading.\n", filename.data());
    }

    m_tets = getTets(out);
    m_baryTransforms.resize(m_tets.size());
    m_points = getPoints(out);
    m_norms.resize(m_points.size());
    m_vels.resize(m_points.size());
    m_pointMasses.resize(m_points.size());
    m_pToTMap = tetsTouchingPoint(m_tets);
    calcFacesAndNorms();

    printf("Tets loaded: %d\n", m_tets.size());
    printf("m_faces size is now %d\n", m_faces.size());
    calcBaryTransforms();
    calcPointMasses();
    srand(time(NULL));
    /*for(int i = 0; i < 1; i++) {
        auto it = m_faces.begin(); // get random pt
        int id = rand() % m_faces.size();
        std::advance(it, id);
        printf("%d\n", id);
        assert(it->second);
        int p1idx = it->first.x;
        m_points[p1idx] += glm::normalize(m_norms[p1idx]) / 1.f;// * 5.f;
    }*/
    // balloon everything out a bit
    for(int i = 0; i < m_points.size(); i++) {
        //m_points[i] *= 1.2;
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
    m_pointMasses = copyFrom->m_pointMasses;
    m_vels = copyFrom->m_vels;
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

bool tetInverted(const std::vector<glm::vec3> &points, tet_t tet) {
    auto p1 = points[tet.p1];
    auto p2 = points[tet.p2];
    auto p3 = points[tet.p3];
    auto p4 = points[tet.p4];
    glm::vec3 cross123 = glm::cross(p2 - p1, p3 - p1);
    return glm::dot(cross123, p4 - p1) < 0;
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
    for(int i = 0; i < m_tets.size(); i++) {
        auto tet = m_tets[i];
        if(tetInverted(points, tet)) {
            printf("Inverted tet (#%d) found!\n", i);
            fflush(stdout);
        }
        auto p1 = points[tet.p1];
        auto p2 = points[tet.p2];
        auto p3 = points[tet.p3];
        auto p4 = points[tet.p4];
        if(p1 == p2 || p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4 || p3 == p4)
            continue;

        glm::mat3x3 P(p1 - p4, p2 - p4, p3 - p4);
        glm::mat3x3 dx = P * m_baryTransforms[i];
        glm::mat3x3 dxT = dx;
        dx = glm::transpose(dx);
        glm::mat3x3 strain = dxT * dx - id; //- id;

        //glm::mat3x3 stress_elastic = /*m_material.incompressibility*/ settings.femIncompressibility * (strain[0][0] + strain[1][1] + strain[2][2]) * id
        //        + 2 * settings.femRigidity /*m_material.rigidity*/ * strain;
        glm::mat3x3 trace_matrix = (strain[0][0] + strain[1][1] + strain[2][2]) * id;
        glm::mat3x3 stress_elastic = 2 * settings.femRigidity * (strain - 1/3.f * trace_matrix)
                + settings.femIncompressibility * trace_matrix;
        auto v1 = vels[tet.p1];
        auto v2 = vels[tet.p2];
        auto v3 = vels[tet.p3];
        auto v4 = vels[tet.p4];

        glm::mat3x3 V(v1 - v4, v2 - v4, v3 - v4);

        glm::mat3x3 dv = V * m_baryTransforms[i];
        glm::mat3x3 dvT = dv;
        dv = glm::transpose(dv);

        glm::mat3x3 strain_rate = dxT * dv + dvT * dx;

        //glm::mat3x3 stress_viscous = /*m_material.viscous1*/ settings.femBulkViscosity * (strain_rate[0][0] + strain_rate[1][1] + strain_rate[2][2]) * id
        //        + 2 * settings.femShearViscosity /*m_material.viscous2*/ * strain_rate;
        glm::mat3x3 sr_trace_matrix = (strain_rate[0][0] + strain_rate[1][1] + strain_rate[2][2]) * id;
        glm::mat3x3 stress_viscous = 2 * settings.femShearViscosity * (strain_rate - 1/3.f * sr_trace_matrix)
                + settings.femIncompressibility * sr_trace_matrix;
        glm::mat3x3 stress_total = stress_elastic + stress_viscous;
        glm::mat3x3 stress_t_ws = dx * stress_total;
        glm::vec3 cross234 = glm::cross(p4 - p2, p3 - p2);
        glm::vec3 cross134 = glm::cross(p4 - p3, p1 - p3);
        glm::vec3 cross124 = glm::cross(p4 - p1, p2 - p1);
        glm::vec3 cross123 = glm::cross(p2 - p1, p3 - p1);
        float epsilon = -1;
        glm::vec3 p1force;
        glm::vec3 p2force;
        glm::vec3 p3force;
        glm::vec3 p4force;
        if(glm::length(cross234) > epsilon)
            p1force = stress_t_ws * cross234;
        if(glm::length(cross134) > epsilon)
            p2force = stress_t_ws * cross134;
        if(glm::length(cross124) > epsilon)
            p3force = stress_t_ws * cross124;
        if(glm::length(cross123) > epsilon)
            p4force = stress_t_ws * cross123;
        if(glm::length(p1force) > 1000 || glm::length(p2force) > 1000 || glm::length(p3force) > 1000 || glm::length(p4force) > 1000) {
            /*printf("forces real bigg, ls:\n");
            printf("%f, %f, %f at %f, %f, %f\n", p1force.x, p1force.y, p1force.z, (p1-p4).x, (p1-p4).y, (p1-p4).z);
            printf("%f, %f, %f at %f, %f, %f\n", p2force.x, p2force.y, p2force.z, (p2-p4).x, (p2-p4).y, (p2-p4).z);
            printf("%f, %f, %f at %f, %f, %f\n", p3force.x, p3force.y, p3force.z, (p3-p4).x, (p3-p4).y, (p3-p4).z);
            printf("%f, %f, %f at %f, %f, %f\n", p4force.x, p4force.y, p4force.z, 0.f, 0.f, 0.f);
            fflush(stdout);*/

        }
        forcePerNode[tet.p1] -= p1force;
        forcePerNode[tet.p2] -= p2force;
        forcePerNode[tet.p3] -= p3force;
        forcePerNode[tet.p4] -= p4force;
    }
}

bool detectInversion(std::vector<glm::vec3> points, std::vector<glm::vec3> tets) {

}

// number of newtons to apply when penetrated 1  meter^2
#define PENALTY_ACCEL_K 50000.f

void TetMesh::computeCollisionForces(std::vector<glm::vec3> &forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels, float floorY) {
    for(int i = 0; i < points.size(); i++) {
        if(points[i].y < floorY) {
            glm::vec3 force_normal = m_pointMasses[i] * glm::vec3(0, PENALTY_ACCEL_K * (floorY - points[i].y), 0);
            //m_vels[i].y = (floorY - m_points[i].y);
            //float friction_mag = glm::length(0.1 * m_pointMasses[i]) * 0.4;

            forcePerNode[i] += force_normal; /* + friction_mag * glm::normalize(-(forcePerNode[i] - m_pointMasses[i] * glm::vec3(0, -0.1, 0)));*/
            //m_points[i].y = floorY;
            //m_vels[i].y = 0;
        }
    }
}

void TetMesh::computeAllForces(std::vector<glm::vec3> &forcePerNode) {
    std::fill(forcePerNode.begin(), forcePerNode.end(), glm::vec3());
    // first add grav
    for(int i = 0; i < m_points.size(); i++) {
        forcePerNode[i] += glm::vec3(0, -0.1, 0) * m_pointMasses[i];
    }
    computeStressForces(forcePerNode, m_points, m_vels);
    computeCollisionForces(forcePerNode, m_points, m_vels, -2);
}

void TetMesh::computeAllForcesFrom(std::vector<glm::vec3> &forcePerNode, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& vels) {
    std::fill(forcePerNode.begin(), forcePerNode.end(), glm::vec3());
    // first add grav
    for(int i = 0; i < points.size(); i++) {
        forcePerNode[i] += glm::vec3(0, -0.1, 0) * m_pointMasses[i];
    }
    computeStressForces(forcePerNode, points, vels);
    computeCollisionForces(forcePerNode, points, vels, -2);
}

void TetMesh::update(float timestep) {
    // step 1: get all forces.
#if 0
    static glm::vec3 forcePerNode[m_points.size()];

    computeAllForces(forcePerNode);
    // midpoint method:
    // copy old points and vels for final calc

    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forcePerNode[i] / m_pointMasses[i];
        glm::vec3 velIncrement = accel * timestep;
        m_points[i] += m_vels[i] * timestep + velIncrement;// / 2.f;
        m_vels[i] += velIncrement;
    }
#else
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
    computeAllForcesFrom(forces, m_points, m_vels);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk1[i] = m_vels[i];
        dvk1[i] = accel; // compute k1 + p
        //vtemp[i] = m_vels[i] + velIncrement
        xnext[i] = m_points[i] + dxk1[i] * 0.5f * timestep;
        vnext[i] = m_vels[i] + dvk1[i] * 0.5f * timestep;
    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk2[i] = vnext[i];
        dvk2[i] = accel;
        xnext[i] = m_points[i] + dxk2[i] * 0.5f * timestep;
        vnext[i] = m_vels[i] + dvk2[i] * 0.5f * timestep;
    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk3[i] = vnext[i];
        dvk3[i] = accel;
        xnext[i] = m_points[i] + dxk3[i] * 1.0f * timestep;
        vnext[i] = m_vels[i] + dvk3[i] * 1.0f * timestep;
    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dxk4[i] = vnext[i];
        dvk4[i] = accel;
    }
    for(int i = 0; i < m_points.size(); i++) {
        m_points[i] += timestep * (dxk1[i] + dxk4[i] + 2.f*(dxk2[i] + dxk3[i])) / 6.f;
        m_vels[i] += timestep * (dvk1[i] + dvk4[i] + 2.f*(dvk2[i] + dvk3[i])) / 6.f;
    }
    /* computeAllForcesFrom(forces, m_points, m_vels);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dvk1[i] = accel; // compute k1 + p
        vnext[i] = m_vels[i] + dvk1[i] * 0.5f * timestep;
        dxk1[i] = vnext[i];
        //vtemp[i] = m_vels[i] + velIncrement
        xnext[i] = m_points[i] + dxk1[i] * 0.5f * timestep;

    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dvk2[i] = accel;
        vnext[i] = m_vels[i] + dvk2[i] * 0.5f * timestep;
        dxk2[i] = vnext[i];
        xnext[i] = m_points[i] + dxk2[i] * 0.5f * timestep;

    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dvk3[i] = accel;
        vnext[i] = m_vels[i] + dvk3[i] * 1.0f * timestep;
        dxk3[i] = vnext[i];
        xnext[i] = m_points[i] + dxk3[i] * 1.0f * timestep;

    }
    computeAllForcesFrom(forces, xnext, vnext);
    for(int i = 0; i < m_points.size(); i++) {
        glm::vec3 accel = forces[i] / m_pointMasses[i];
        dvk4[i] = accel;
        dxk4[i] = m_vels[i] + dvk4[i] * 1.0f * timestep;//vnext[i];
    }
    for(int i = 0; i < m_points.size(); i++) {
        m_points[i] += timestep * (dxk1[i] + dxk4[i] + 2.f*(dxk2[i] + dxk3[i])) / 6.f;
        m_vels[i] += timestep * (dvk1[i] + dvk4[i] + 2.f*(dvk2[i] + dvk3[i])) / 6.f;
    }*/

#endif
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

#define MAT_DENSITY 1200

void TetMesh::calcPointMasses() {
    // calculate masses for each point based on tet volumes
    assert(m_pointMasses.size() == m_points.size());
    std::fill(m_pointMasses.begin(), m_pointMasses.end(), 0);
    float maxvol = -1, minvol = INFINITY;
    for(int i = 0; i < m_tets.size(); i++) {
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
    for(int i = 0; i < m_pointMasses.size(); i++) {
        sum += m_pointMasses[i];
        if(m_pointMasses[i] < minm)
            minm = m_pointMasses[i];

    }
    printf("Total mass is %f, smallest mass is %f w/ inverse %f\nMax volume is %f, min volume is %f", sum, minm, 1.f/minm, maxvol, minvol);
    /*float min_allowed_mass = sum / m_points.size() / 100;
    for(int i = 0; i < m_pointMasses.size(); i++) {
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
