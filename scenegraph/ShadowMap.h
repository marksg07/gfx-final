#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "CS123SceneData.h"
#include "SceneviewScene.h"
#include "gl/textures/DepthTexture.h"
#include "gl/datatype/DepthFBO.h"
#include "glm/gtx/string_cast.hpp"
#include "Camera.h"
#include "gl/util/FullScreenQuad.h"
#include "gl/shaders/Shader.h"
#include <glm/gtc/matrix_transform.hpp>

class SceneviewScene;



class ShadowMap
{
public:
    ShadowMap(std::shared_ptr<CS123::GL::Shader> shader, std::shared_ptr<CS123::GL::Shader> pointShader, std::shared_ptr<CS123::GL::Shader> dbg_shader, CS123SceneLightData light, SceneviewScene* scene);
    void updateMat(Camera* camera);
    void update(Camera* camera);

    void drawDBG();


    GLuint textureID()
    {
        return m_dfbo->textureID();
    }

    glm::mat4& mat()
    {
        return m_MVP;
    }

    glm::mat4& biasMVP()
    {
        return m_biasMVP;
    }

    void prepareShader(CS123::GL::Shader* shader, std::string base, size_t i)
    {
        if (m_light.type == LightType::LIGHT_DIRECTIONAL) {
            shader->setUniformArrayByIndex(base + "Mat", biasMVP(), i);
            shader->setTexture(base + "Map[" + std::to_string(i) + "]",  GL_TEXTURE_2D, textureID());
        } else if (m_light.type == LightType::LIGHT_POINT) {
            shader->setTexture(base + "CubeMap[" + std::to_string(i) + "]",  GL_TEXTURE_CUBE_MAP, textureID());
        }
    }

    void renderDirectional(Camera* camera);

    void renderPoint(Camera* camera);

    void getMaxMin(Camera* camera, glm::vec3& min, glm::vec3& max);

    std::unique_ptr<DepthFBO> m_dfbo;

private:

    static std::unique_ptr<CS123::GL::FullScreenQuad> fsq;

    glm::mat4 m_MVP;
    glm::mat4 m_biasMatrix;
    glm::mat4 m_biasMVP;

    CS123SceneLightData m_light;
    SceneviewScene* m_scene;
    std::shared_ptr<CS123::GL::Shader> m_shadowShader;
    std::shared_ptr<CS123::GL::Shader> m_shadowPointShader;
    std::shared_ptr<CS123::GL::Shader> m_dbgShader;


    size_t m_width = 1024, m_height = 1024;

    unsigned int depthMapFBO;
    unsigned int depthMap;

};

#endif // SHADOWMAP_H
