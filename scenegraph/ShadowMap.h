#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "CS123SceneData.h"
#include "SceneviewScene.h"

class ShadowMap
{
public:
    ShadowMap(CS123SceneLightData* light, SceneviewScene* scene)
        : m_light(light), m_scene(scene)
    {

    }



private:
    CS123SceneLightData* m_light;
    SceneviewScene* m_scene;

    size_t m_width = 1024, m_height = 1024;
};

#endif // SHADOWMAP_H
