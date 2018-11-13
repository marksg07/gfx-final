#include "Scene.h"
#include "Camera.h"
#include <iostream>
#include "CS123ISceneParser.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"


Scene::Scene()
{
}

Scene::Scene(Scene &scene)
{
    // We need to set the global constants to one when we duplicate a scene,
    // otherwise the global constants will be double counted (squared)
    CS123SceneGlobalData global = { 1, 1, 1, 1};
    setGlobal(global);

    // TODO [INTERSECT]
    // Make sure to copy over the lights and the scenegraph from the old scene,
    // as well as any other member variables your new scene will need.
    m_renderables = scene.m_renderables;
    m_lights = scene.m_lights;
    m_cameraData = scene.m_cameraData;
    m_globalData = scene.m_globalData;
}

Scene::~Scene()
{
    // Do not delete m_camera, it is owned by SupportCanvas3D
}

void Scene::parseNode(CS123SceneNode* node, glm::mat4x4 transform, CS123SceneGlobalData& global)
{
    for(auto t : node->transformations)
    {
        glm::mat4x4 m = transformToMatrix(t);

        transform *= m;
    }

    //std::cout << glm::to_string(transform) << std::endl;

    for(auto primative : node->primitives)
    {
        m_renderables.push_back(CS123Renderable(transform, primative, global));
    }

    for(auto c : node->children)
    {
        parseNode(c, transform, global);
    }
}

void Scene::parse(Scene *sceneToFill, CS123ISceneParser *parser) {
    // TODO: load scene into sceneToFill using setGlobal(), addLight(), addPrimitive(), and
    //   finishParsing()

    // Global
    CS123SceneGlobalData global;
    parser->getGlobalData(global);
    sceneToFill->setGlobal(global);


    // Add nodes
    CS123SceneNode* root = parser->getRootNode();
    sceneToFill->parseNode(root, glm::mat4(1.0f), global);

    // Add lights
    size_t num_lights = parser->getNumLights();
    for(size_t i = 0; i < num_lights; i++)
    {
        CS123SceneLightData light;

        parser->getLightData(i, light);

        sceneToFill->addLight(light);
    }

    sceneToFill->setupLevelOfDetail();
}



void Scene::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const glm::mat4x4 &matrix) {

}

void Scene::addLight(const CS123SceneLightData &sceneLight) {

    m_lights.push_back(sceneLight);
}

void Scene::setGlobal(const CS123SceneGlobalData &global) {
    m_globalData = global;
}

