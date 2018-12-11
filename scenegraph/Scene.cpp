#include "Scene.h"
#include "Camera.h"
#include "CS123ISceneParser.h"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/norm.hpp"
#include <algorithm>
Scene::Scene()
{
}

Scene::Scene(Scene &scene) :
    m_textures()
{
    // We need to set the global constants to one when we duplicate a scene,
    // otherwise the global constants will be double counted (squared)
    CS123SceneGlobalData global = { 1, 1, 1, 1};
    setGlobal(global);

    // TODO [INTERSECT]
    // Make sure to copy over the lights and the scenegraph from the old scene,
    // as well as any other member variables your new scene will need.

}

Scene::~Scene()
{
    // Do not delete m_camera, it is owned by SupportCanvas3D
}

glm::mat4x4 getRealMat(CS123SceneTransformation tr) {
   glm::mat4x4 ret;
   switch(tr.type) {
   case TRANSFORMATION_TRANSLATE:
       ret = glm::translate(tr.translate);
       break;
   case TRANSFORMATION_SCALE:
       ret = glm::scale(tr.scale);
       break;
   case TRANSFORMATION_ROTATE:
       ret = glm::rotate(tr.angle, tr.rotate);
       break;
   case TRANSFORMATION_MATRIX:
       ret = tr.matrix;
       break;
   default:
       ret = glm::mat4x4();
   }
   return ret;
}

void Scene::traverseAndAddPrimitives(Scene *scene, CS123SceneNode *root, glm::mat4x4 trans) {
    //printf("new traversal node reached, root = %p\n", root);
    for(unsigned long i = 0; i < root->transformations.size(); i++) {
        trans *= getRealMat(*root->transformations[i]);
    }
    for(unsigned long i = 0; i < root->primitives.size(); i++) {
        scene->addPrimitive(*root->primitives[i], trans);
    }
    for(unsigned long i = 0; i < root->children.size(); i++) {
        traverseAndAddPrimitives(scene, root->children[i], trans);
    }
}

void Scene::parse(Scene *sceneToFill, CS123ISceneParser *parser) {
    // TODO: load scene into sceneToFill using setGlobal(), addLight(), addPrimitive(), and
    //   finishParsing()
    CS123SceneGlobalData global;
    parser->getGlobalData(global);
    sceneToFill->setGlobal(global);
    int nLights = parser->getNumLights();
    sceneToFill->m_lights.reserve(sceneToFill->m_lights.size() + nLights);
    for(int i = 0; i < nLights; i++) {
        CS123SceneLightData light;
        parser->getLightData(i, light);
        sceneToFill->addLight(light);
    }
    CS123SceneNode *root = parser->getRootNode();
    printf("Starting scenegraph traversal...\n");
    traverseAndAddPrimitives(sceneToFill, root, glm::mat4x4());

}

void Scene::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const glm::mat4x4 &matrix) {
    // this operation copies b/c we are making a new struct
    // we will apply the global now
    CS123ScenePrimitive prim(scenePrimitive);
    if(prim.material.textureMap.isUsed) {
        prim.material.textureMap.filename = prim.material.textureMap.filename.replace(0, strlen("/course/cs123/data/image"), "image");
        auto fname = prim.material.textureMap.filename;
        if(!m_textures.count(fname)) {
            m_textures[fname] = std::make_unique<QImage>(fname.data());
            printf("Setting %s to %p, width = %d\n", fname.data(), m_textures[fname].get() , m_textures[fname]->width());
        }
    }
    if(prim.material.bumpMap.isUsed) {
        printf("bump map used\n");
        prim.material.bumpMap.filename = prim.material.bumpMap.filename.replace(0, strlen("/course/cs123/data/image"), "image");
        auto fname = prim.material.bumpMap.filename;
        if(!m_textures.count(fname)) {
            m_textures[fname] = std::make_unique<QImage>(fname.data());
            printf("Setting %s to %p, width = %d\n", fname.data(), m_textures[fname].get() , m_textures[fname]->width());
        }
    }
    prim.material.cDiffuse *= m_global.kd;
    prim.material.cAmbient *= m_global.ka;
    prim.material.cSpecular *= m_global.ks;
    prim.material.cReflective *= m_global.ks;
    prim.material.cTransparent *= m_global.kt;
    // to get aabb, take bounding box and transform(yes this is lazy)
    std::vector<double> xs, ys, zs;
    xs.reserve(8);
    ys.reserve(8);
    zs.reserve(8);
    for(float x = -0.5; x < 1; x++) {
        for(float y = -0.5; y < 1; y++) {
            for(float z = -0.5; z < 1; z++) {
                glm::vec4 tr = matrix * glm::vec4(x, y, z, 1);
                xs.push_back(tr.x);
                ys.push_back(tr.y);
                zs.push_back(tr.z);
            }
        }
    }
    glm::vec3 minbound = glm::vec3(*std::min_element(xs.begin(), xs.end()), *std::min_element(ys.begin(), ys.end()), *std::min_element(zs.begin(), zs.end()));
    glm::vec3 maxbound = glm::vec3(*std::max_element(xs.begin(), xs.end()), *std::max_element(ys.begin(), ys.end()), *std::max_element(zs.begin(), zs.end()));
    object_node_t node = {prim, matrix, glm::inverse(matrix), minbound, maxbound};
    m_nodes.push_back(node);
}

// we use copy constructors for the other 2
void Scene::addLight(const CS123SceneLightData &sceneLight) {
    CS123SceneLightData light(sceneLight);
    light.angle = glm::radians(light.angle);
    light.penumbra = glm::radians(light.penumbra);
    m_lights.push_back(light);
}

void Scene::setGlobal(const CS123SceneGlobalData &global) {
    m_global = CS123SceneGlobalData(global);
}

