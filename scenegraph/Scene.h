#ifndef SCENE_H
#define SCENE_H

#include "CS123SceneData.h"
#include <map>
#include <QImage>
#include <memory>

class Camera;
class CS123ISceneParser;


/**
 * @class Scene
 *
 * @brief This is the base class for all scenes. Modify this class if you want to provide
 * common functionality to all your scenes.
 */

typedef struct ObjectNode {
    CS123ScenePrimitive primitive;
    glm::mat4x4 trans;
    glm::mat4x4 invtrans;
    glm::vec3 minbound, maxbound;
} object_node_t;

class Scene {
public:
    Scene();
    Scene(Scene &scene);
    virtual ~Scene();

    virtual void settingsChanged() {}

    static void parse(Scene *sceneToFill, CS123ISceneParser *parser);

//protected:

    // Adds a primitive to the scene.
    virtual void addPrimitive(const CS123ScenePrimitive &scenePrimitive, const glm::mat4x4 &matrix);

    // Adds a light to the scene.
    virtual void addLight(const CS123SceneLightData &sceneLight);

    // Sets the global data for the scene.
    virtual void setGlobal(const CS123SceneGlobalData &global);

    std::vector<object_node_t> m_nodes;
    std::vector<CS123SceneLightData> m_lights;
    CS123SceneGlobalData m_global;
    std::map<std::string, std::unique_ptr<QImage>> m_textures;
private:
    static void traverseAndAddPrimitives(Scene *scene, CS123SceneNode *root, glm::mat4x4 trans);

};



#endif // SCENE_H
