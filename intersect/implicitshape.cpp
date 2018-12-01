#include "implicitshape.h"
#include <algorithm>

const char *placestring[N_PLACE+1] = {"undefined", "sphere", "cone body", "cone cap",
                             "cyl top", "cyl bot", "cyl body", "cube front",
                             "cube back", "cube left", "cube right", "cube up",
                              "cube down", "really undefined"
                            };

ImplicitShape::ImplicitShape()
{

}

struct tAndPlace ImplicitShape::getIntersectT(PrimitiveType type, glm::vec4 P, glm::vec4 d) {
    switch(type) {
    case PrimitiveType::PRIMITIVE_CONE:
        ////printf("cone\n");
        return coneIntersectT(P, d);
    case PrimitiveType::PRIMITIVE_CUBE:
        ////printf("cube\n");
        return cubeIntersectT(P, d);
    case PrimitiveType::PRIMITIVE_CYLINDER:
        ////printf("cylinder\n");
        return cylinderIntersectT(P, d);
    case PrimitiveType::PRIMITIVE_SPHERE:
        ////printf("sphere\n");
        return sphereIntersectT(P, d);
    default:
        return {INFINITY, UNDEF};
    }
}
struct tAndPlace ImplicitShape::coneIntersectT(glm::vec4 P, glm::vec4 d) {
    // cap
    double capT = (-0.5 - P.y) / d.y;
    double x = P.x + capT * d.x;
    double z = P.z + capT * d.z;
    if(x * x + z * z > 0.25 || capT < 0)
        capT = INFINITY;
    // body
    double A = d.x * d.x + d.z * d.z - 0.25 * d.y * d.y;
    double B = 2 * P.x * d.x + 2 * P.z * d.z - 0.5 * P.y * d.y + 0.25 * d.y;
    double C = P.x * P.x + P.z * P.z - 0.25 * P.y * P.y + 0.25 * P.y - 0.25 * 0.25;
    double D = B * B - 4 * A * C;
    if(D >= 0 && A != 0) {
        double t1 = (-B - glm::sqrt(D)) / 2 / A;
        if(t1 < 0 || fabs(P.y + t1 * d.y) > 0.5)
            t1 = INFINITY;
        double t2 = (-B + glm::sqrt(D)) / 2 / A;
        if(t2 < 0 || fabs(P.y + t2 * d.y) > 0.5)
            t2 = INFINITY;
        double cb_min = std::min(t1, t2);
        if(cb_min < capT)
            return {cb_min, CONE_BODY};
        return {capT, CONE_CAP};
    }
    return {capT, CONE_CAP};

}

struct tAndPlace ImplicitShape::sphereIntersectT(glm::vec4 P, glm::vec4 d) {
    // (P + td) . (P + td) = 0.25
    // d.d * t^2  + 2 * P.d * t + P . P - 0.25 = 0
    double A = glm::dot(d, d); // = sum of coord squares
    double B = 2 * glm::dot(P, d);
    // P is a point not a vector
    double C = glm::dot(P.xyz(), P.xyz()) - 0.25;
    double D = B * B - 4 * A * C;
    if(D >= 0 && A != 0) {
        double t1 = (-B - glm::sqrt(D)) / 2 / A;

        if(t1 < 0)
            t1 = INFINITY;
        double t2 = (-B + glm::sqrt(D)) / 2 / A;

        if(t2 < 0)
            t2 = INFINITY;
        return {std::min(t1, t2), SPHERE};
    }
    return {INFINITY, SPHERE};
}

struct tAndPlace ImplicitShape::cylinderIntersectT(glm::vec4 P, glm::vec4 d) {
    // caps
    double cap1T = (-0.5 - P.y) / d.y;
    double x = P.x + cap1T * d.x;
    double z = P.z + cap1T * d.z;
    //printf("c1t=%f\n", cap1T);
    if(x * x + z * z > 0.25 || cap1T < 0)
        cap1T = INFINITY;
    //printf("c1t=%f\n", cap1T);
    double cap2T = (0.5 - P.y) / d.y;
    x = P.x + cap2T * d.x;
    z = P.z + cap2T * d.z;
    //printf("c2t=%f\n", cap2T);
    if(x * x + z * z > 0.25 || cap2T < 0)
        cap2T = INFINITY;
    //printf("c2t=%f\n", cap2T);
    // body:
    /* (P.xz + t*d.xz) . (P.xz + t*d.xz) = 0.25
     * d.xz . d.xz * t^2 + 2 * P.xz . d.xz * t + P.xz . P.xz - 0.25 = 0
     */
    auto dxz = d.xz();
    auto Pxz = P.xz();
    double A = glm::dot(dxz, dxz);
    double B = 2 * glm::dot(Pxz, dxz);
    double C = glm::dot(Pxz, Pxz) - 0.25;
    double D = B * B - 4 * A * C;
    if(D >= 0 && A != 0) {
        double t1 = (-B - glm::sqrt(D)) / 2 / A;
        //printf("t1=%f\n", t1);
        if(t1 < 0 || fabs(P.y + t1 * d.y) > 0.5)
            t1 = INFINITY;
        //printf("t1=%f\n", t1);
        double t2 = (-B + glm::sqrt(D)) / 2 / A;
        //printf("t2=%f\n", t2);
        if(t2 < 0 || fabs(P.y + t2 * d.y) > 0.5)
            t2 = INFINITY;
        //printf("t2=%f\n", t2);
        double cb_min = std::min(t1, t2);
        if(cb_min <= std::min(cap1T, cap2T))
            return {cb_min, CYL_BODY};
    }
    ISPlace mincap;
    if(cap1T < cap2T) {
        return {cap1T, CYL_BOT};
    }
    return {cap2T, CYL_TOP};

}

bool intersectsPlane(glm::vec2 P, glm::vec2 d, double t) {
    glm::vec2 point = P + d * glm::vec2(t, t);
    return point.x >= -0.5 && point.x <= 0.5 && point.y >= -0.5 && point.y <= 0.5;
}

ISPlace facePlaces[] = {CUBE_F, CUBE_B, CUBE_L, CUBE_R, CUBE_U, CUBE_D};

struct tAndPlace ImplicitShape::cubeIntersectT(glm::vec4 P, glm::vec4 d) {
    // front = xy plane at +0.5
    double tF = (0.5 - P.z) / d.z;
    if(tF < 0 || !intersectsPlane(P.xy(), d.xy(), tF))
        tF = INFINITY;
    double tB = (-0.5 - P.z) / d.z;
    if(tB < 0 || !intersectsPlane(P.xy(), d.xy(), tB))
        tB = INFINITY;
    double tL = (-0.5 - P.x) / d.x;
    if(tL < 0|| !intersectsPlane(P.yz(), d.yz(), tL))
        tL = INFINITY;
    double tR = (0.5 - P.x) / d.x;
    if(tR < 0 || !intersectsPlane(P.yz(), d.yz(), tR))
        tR = INFINITY;
    double tU = (0.5 - P.y) / d.y;
    if(tU < 0 || !intersectsPlane(P.xz(), d.xz(), tU))
        tU = INFINITY;
    double tD = (-0.5 - P.y) / d.y;
    if(tD < 0 || !intersectsPlane(P.xz(), d.xz(), tD))
        tD = INFINITY;

    double v[6] = {tF, tB, tL, tR, tU, tD};
    auto it = std::min_element(v, v+6);
    return {*it, facePlaces[it - v]};
}

struct tAndPlace ImplicitShape::cubeIntersectT_neg(glm::vec4 P, glm::vec4 d) {
    // front = xy plane at +0.5
    double tF = (0.5 - P.z) / d.z;
    if(!intersectsPlane(P.xy(), d.xy(), tF))
        tF = INFINITY;
    double tB = (-0.5 - P.z) / d.z;
    if(!intersectsPlane(P.xy(), d.xy(), tB))
        tB = INFINITY;
    double tL = (-0.5 - P.x) / d.x;
    if(!intersectsPlane(P.yz(), d.yz(), tL))
        tL = INFINITY;
    double tR = (0.5 - P.x) / d.x;
    if(!intersectsPlane(P.yz(), d.yz(), tR))
        tR = INFINITY;
    double tU = (0.5 - P.y) / d.y;
    if(!intersectsPlane(P.xz(), d.xz(), tU))
        tU = INFINITY;
    double tD = (-0.5 - P.y) / d.y;
    if(!intersectsPlane(P.xz(), d.xz(), tD))
        tD = INFINITY;

    double v[6] = {tF, tB, tL, tR, tU, tD};
    auto it = std::min_element(v, v+6);
    return {*it, facePlaces[it - v]};
}

// very fast version of box intersect that doesn't transform to OS, assumes box is axis aligned, doesn't cull negative t's, and
// doesn't find ISPlace of intersection. This is optimized heavily for kd-tree traversal.
float ImplicitShape::AABBIntersectT(glm::vec4 P, glm::vec4 d, glm::vec4 invd, glm::bvec3 dsigns, glm::bvec3 idsigns, glm::vec3 mn, glm::vec3 mx) {
    // essentially, the idea of this is that we want to have all the times of entrance to the box
    // be less than all the times of exit. This means that there is some period of time at which all coordinates
    // are within the constraints of the box. The min time this happens is max(t entrance x, t entrance y, t entrance z).
    // The condition that this happens is that that time is <= min(t exit x, t exit y, t exit z).
    float temp, t1x, t2x, t1y, t2y, t1z, t2z;
    // precomputing invdir both saves time and solves a problem where the ray's direction.x is +/- 0.
    // we also eliminate branches by using signs as indices.
    float tx[] = {(mn.x - P.x)*invd.x,
                (mx.x - P.x)*invd.x};
    float ty[] = {(mn.y - P.y)*invd.y,
                (mx.y - P.y)*invd.y};
    t1x = tx[idsigns.x];
    t2x = tx[dsigns.x];
    t1y = ty[idsigns.y];
    t2y = ty[dsigns.y];

    if(t1x > t2y || t1y > t2x) { // either of these mean that we exit one dimension of the rectangle before we enter the next.
        // i.e., the x dimension is already outside of rect bounds by the time the y dimension is in bounds, or other way.
        return INFINITY;
    }

    float tmin = std::max(t1x, t1y); // This definitely hits the xy-plane if we ignore z-coords. We need the max so that we get
    // the time when the second coordinate enters the box. If we did it when the first coordinate enters, i.e. the min, the
    // second coordinate wouldn't have entered yet.
    float tmax = std::min(t2x, t2y); // similar to above

    float tz[] = {(mn.z - P.z)*invd.z, (mx.z - P.z)*invd.z};
    t1z = tz[idsigns.z];
    t2z = tz[dsigns.z];

    if(t1z > tmax || t2z < tmin) {// similar test, we exit one of the dimensions before we enter z or we exit z before we enter
        // one of the dimensions
        return INFINITY;
    }
    // So we do hit the box.
    return std::max(t1z, tmin);
}

// we dont need to normalize in this fxn because we have to take invT(mat) * normal
// and then normalize that anyways
glm::vec4 ImplicitShape::getNormal(ISPlace place, glm::vec4 p) {
    p.w = 0;
    switch(place) {
    case SPHERE:
        return p;
    case CONE_BODY:
        return glm::vec4(p.x, glm::sqrt(p.x * p.x + p.z * p.z) / 2, p.z, 0);
    case CYL_BODY:
        p.y = 0;
        return p;
    case CONE_CAP:
    case CYL_BOT:
    case CUBE_D:
        return glm::vec4(0, -1, 0, 0);
    case CYL_TOP:
    case CUBE_U:
        return glm::vec4(0, 1, 0, 0);
    case CUBE_F:
        return glm::vec4(0, 0, 1, 0);
    case CUBE_B:
        return glm::vec4(0, 0, -1, 0);
    case CUBE_L:
        return glm::vec4(-1, 0, 0, 0);
    case CUBE_R:
        return glm::vec4(1, 0, 0, 0);
    default:
        return glm::vec4(0, 0, 0, 0);
    }
}

// should point in advancing u direction corresponding with texcors.
glm::vec4 ImplicitShape::getTangent(ISPlace place, glm::vec4 p) {
    p.w = 0;
    switch(place) {
    case SPHERE:
        if(p.x == 0 && p.z == 0)
            return glm::vec4(1, 0, 0, 0);
        return glm::normalize(glm::vec4(p.z, 0, -p.x, 0.f));
    case CONE_BODY:
        if(p.x == 0 && p.z == 0)
            return glm::vec4(1, 0, 0, 0);
        return glm::normalize(glm::vec4(p.z, 0, -p.x, 0.f));
    case CYL_BODY:
        if(p.x == 0 && p.z == 0)
            return glm::vec4(1, 0, 0, 0);
        return glm::normalize(glm::vec4(p.z, 0, -p.x, 0.f));
        /*case CONE_CAP:
    case CYL_BOT:
    case CUBE_D:
        return glm::vec2(p.x + 0.5, -p.z + 0.5);
    case CYL_TOP:
    case CUBE_U:
        return glm::vec2(p.x + 0.5, p.z + 0.5);
    case CUBE_F:
        return glm::vec2(p.x + 0.5, -p.y + 0.5);
    case CUBE_B:
        return glm::vec2(-p.x + 0.5, -p.y + 0.5);
    case CUBE_L:
        return glm::vec2(p.z + 0.5, -p.y + 0.5);
    case CUBE_R:
        return glm::vec2(-p.z + 0.5, -p.y + 0.5);*/
    case CONE_CAP:
    case CYL_BOT:
    case CUBE_D:
        return glm::vec4(1, 0, 0, 0);
    case CYL_TOP:
    case CUBE_U:
        return glm::vec4(1, 0, 0, 0);
    case CUBE_F:
        return glm::vec4(1, 0, 0, 0);
    case CUBE_B:
        return glm::vec4(-1, 0, 0, 0);
    case CUBE_L:
        return glm::vec4(0, 0, 1, 0);
    case CUBE_R:
        return glm::vec4(0, 0, -1, 0);
    default:
        return glm::vec4(0, 0, 0, 0);
    }
}

bool ImplicitShape::checkCorrect(ISPlace place, glm::vec4 p) {
    double err = .002;
    switch(place) {
    case SPHERE:
        return fabs(glm::dot(p.xyz(), p.xyz()) - 0.25) <= err;
    case CONE_BODY:
        return fabs(glm::dot(p.xz(), p.xz()) - ((0.5 - p.y) / 2) * ((0.5 - p.y) / 2)) <= err
                && p.y >= -0.5 - err && p.y <= 0.5 + err;
    case CYL_BODY:
        return true;//fabs(glm::dot(p.xz(), p.xz()) - 0.25) <= err
                //&& p.y >= -0.5 - err && p.y <= 0.5 + err;
    case CONE_CAP:
    case CYL_BOT:
        return glm::dot(p.xz(), p.xz()) <= 0.25 + err
                && fabs(p.y + 0.5) <= err;
    case CYL_TOP:
        return glm::dot(p.xz(), p.xz()) <= 0.25 + err
                && fabs(p.y - 0.5) <= err;
    case CUBE_D:
        return fabs(p.x) <= 0.5 + err && fabs(p.z) <= 0.5 + err
                && fabs(p.y + 0.5) <= err;
    case CUBE_U:
        return fabs(p.x) <= 0.5 + err && fabs(p.z) <= 0.5 + err
                && fabs(p.y - 0.5) <= err;
    case CUBE_F:
        return fabs(p.x) <= 0.5 + err && fabs(p.y) <= 0.5 + err
                && fabs(p.z - 0.5) <= err;
    case CUBE_B:
        return fabs(p.x) <= 0.5 + err && fabs(p.y) <= 0.5 + err
                && fabs(p.z + 0.5) <= err;
    case CUBE_L:
        return fabs(p.y) <= 0.5 + err && fabs(p.z) <= 0.5 + err
                && fabs(p.x + 0.5) <= err;
    case CUBE_R:
        return fabs(p.y) <= 0.5 + err && fabs(p.z) <= 0.5 + err
                && fabs(p.x - 0.5) <= err;
    default:
        return false;
    }

}

glm::vec2 ImplicitShape::getTexCoords(ISPlace place, glm::vec4 p) {
    float u, v, theta, phi;
    switch(place) {
    case SPHERE:
        theta = std::atan2(p.z, p.x);
        u = -theta / (2 * M_PI);
        if(theta >= 0)
            u += 1;
        phi = std::asin(p.y*2);
        v = 1 - (phi / M_PI + 0.5);
        return glm::vec2(u, v);
    case CONE_BODY:
    case CYL_BODY:
        theta = std::atan2(p.z, p.x);
        u = -theta / (2 * M_PI);
        if(theta >= 0)
            u += 1;
        v = 1 - (p.y + 0.5);
        return glm::vec2(u, v);
    case CONE_CAP:
    case CYL_BOT:
    case CUBE_D:
        return glm::vec2(p.x + 0.5, -p.z + 0.5);
    case CYL_TOP:
    case CUBE_U:
        return glm::vec2(p.x + 0.5, p.z + 0.5);
    case CUBE_F:
        return glm::vec2(p.x + 0.5, -p.y + 0.5);
    case CUBE_B:
        return glm::vec2(-p.x + 0.5, -p.y + 0.5);
    case CUBE_L:
        return glm::vec2(p.z + 0.5, -p.y + 0.5);
    case CUBE_R:
        return glm::vec2(-p.z + 0.5, -p.y + 0.5);
    default:
        return glm::vec2(0, 0);
    }
}
