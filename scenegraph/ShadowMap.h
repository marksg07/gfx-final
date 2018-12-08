#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "CS123SceneData.h"
#include "SceneviewScene.h"
#include "gl/textures/DepthTexture.h"
#include "gl/datatype/DepthFBO.h"
#include "glm/gtx/string_cast.hpp"
#include "Camera.h"

class SceneviewScene;



class ShadowMap
{
public:
    ShadowMap(std::shared_ptr<CS123::GL::Shader> shader, std::shared_ptr<CS123::GL::Shader> dbg_shader, CS123SceneLightData light, SceneviewScene* scene);

    void updateMat(Camera* camera);
    void update(Camera* camera);

    void drawDBG();


    DepthTexture& texture()
    {
        return m_dfbo->texture();
    }

    glm::mat4& mat()
    {
        return m_MVP;
    }

    glm::mat4& biasMVP()
    {
        return m_biasMVP;
    }

    void bindTexture()
    {
        m_dfbo->bindTexture();
    }

    std::unique_ptr<DepthFBO> m_dfbo;


private:

    static std::unique_ptr<CS123::GL::FullScreenQuad> fsq;

    glm::mat4 m_MVP;
    glm::mat4 m_biasMatrix;
    glm::mat4 m_biasMVP;

    CS123SceneLightData m_light;
    SceneviewScene* m_scene;
    std::shared_ptr<CS123::GL::Shader> m_shadowShader;
    std::shared_ptr<CS123::GL::Shader> m_dbgShader;


    size_t m_width = 1024, m_height = 1024;

    unsigned int depthMapFBO;
    unsigned int depthMap;

};

#endif // SHADOWMAP_H
