#pragma once
#ifndef IMPLICITSHAPE_H
#define IMPLICITSHAPE_H
#include "CS123SceneData.h"

enum ISPlace {
    UNDEF,
    SPHERE,
    CONE_BODY,
    CONE_CAP,
    CYL_TOP,
    CYL_BOT,
    CYL_BODY,
    CUBE_F,
    CUBE_B,
    CUBE_L,
    CUBE_R,
    CUBE_U,
    CUBE_D,
    N_PLACE
};

struct tAndPlace {
    double t;
    ISPlace pl;
};

extern const char *placestring[N_PLACE+1];

class ImplicitShape
{
public:
    ImplicitShape();
    static struct tAndPlace getIntersectT(PrimitiveType type, glm::vec4 P, glm::vec4 d);
    static struct tAndPlace coneIntersectT(glm::vec4 P, glm::vec4 d);
    static struct tAndPlace cylinderIntersectT(glm::vec4 P, glm::vec4 d);
    static struct tAndPlace cubeIntersectT(glm::vec4 P, glm::vec4 d);
    static struct tAndPlace cubeIntersectT_neg(glm::vec4 P, glm::vec4 d);
    static float AABBIntersectT(glm::vec4 P, glm::vec4 d, glm::vec4 invd, glm::bvec3 dsigns, glm::bvec3 idsigns, glm::vec3 mn, glm::vec3 mx);
    static struct tAndPlace sphereIntersectT(glm::vec4 P, glm::vec4 d);
    static glm::vec4 getNormal(ISPlace place, glm::vec4 p);
    static glm::vec4 getTangent(ISPlace place, glm::vec4 p);
    static bool checkCorrect(ISPlace place, glm::vec4 p);
    static glm::vec2 getTexCoords(ISPlace place, glm::vec4 p);
};

#endif // IMPLICITSHAPE_H
