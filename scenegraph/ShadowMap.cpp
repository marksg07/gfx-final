#include "ShadowMap.h"
#include "Settings.h"
#include "gl/textures/DepthCubeTexture.h"

std::unique_ptr<CS123::GL::FullScreenQuad> ShadowMap::fsq = nullptr;

ShadowMap::ShadowMap(std::shared_ptr<CS123::GL::Shader> shader, std::shared_ptr<CS123::GL::Shader> dbg_shader, CS123SceneLightData light, SceneviewScene* scene)
    : m_shadowShader(shader), m_dbgShader(dbg_shader), m_light(light), m_scene(scene)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (ShadowMap::fsq == nullptr)
    {
        ShadowMap::fsq = std::make_unique<CS123::GL::FullScreenQuad>();
    }

    m_biasMatrix = glm::mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    );


    m_dfbo = std::make_unique<DepthFBO>(m_width, m_height);

    std::shared_ptr<DepthTexture> tex;
    if (m_light.type == LightType::LIGHT_DIRECTIONAL) {
        tex = std::make_shared<DepthTexture>(m_width, m_height);
    } else if (m_light.type == LightType::LIGHT_POINT) {
        tex = std::make_shared<DepthCubeTexture>(m_width, m_height);
    }


    m_dfbo->attachTexture(tex);

}

std::vector<glm::vec3> getFrustumPoints(glm::mat4 projection)
{

    std::vector<glm::vec3> corners;
    glm::vec4 hcorners[8];

    // near
    hcorners[0] = glm::vec4(-1, 1, 1, 1);
    hcorners[1] = glm::vec4(1, 1, 1, 1);
    hcorners[2] = glm::vec4(1, -1, 1, 1);
    hcorners[3] = glm::vec4(-1, -1, 1, 1);

    // far
    hcorners[4] = glm::vec4(-1, 1, -1, 1);
    hcorners[5] = glm::vec4(1, 1, -1, 1);
    hcorners[6] = glm::vec4(1, -1, -1, 1);
    hcorners[7] = glm::vec4(-1, -1, -1, 1);

    glm::mat4 inverseProj = glm::inverse(projection);
    for (int i = 0; i < 8; i++) {
        hcorners[i] = inverseProj * hcorners[i];
        hcorners[i] /= hcorners[i].w;

        corners.push_back(glm::vec3(hcorners[i]));
    }

    return corners;
}

void ShadowMap::updateMat(Camera* camera)
{
    std::vector<glm::vec3> corners = getFrustumPoints(camera->getProjectionMatrix());

    glm::vec3 min = corners[0];
    glm::vec3 max = corners[7];

    for(auto c : corners)
    {
        //std::cout << glm::to_string(c) << std::endl;
        for(int i = 0; i < 3; i++)
        {

            if (c[i] < min[i])
            {
                min[i] = c[i];
            }

            if (c[i] > max[i])
            {
                max[i] = c[i];
            }
        }
    }

    float tmp = -max.z;
    max.z = -min.z;
    min.z = tmp;

    max /= 2.0f;
    min /= 2.0f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::mat4 v = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), m_light.dir.xyz(), glm::vec3(0.0, 1.0, 0.0));

    // This is a hack...
    glm::mat4 p = glm::ortho(min.x, max.x, min.y, max.y, -5.0f, max.z);

    m_MVP = p * v * glm::mat4(1.0);
    m_biasMVP = m_biasMatrix * m_MVP;
}

void ShadowMap::renderDirectional()
{
    m_shadowShader->bind();

    m_shadowShader->setUniform("shadowMat", m_MVP);

    m_dfbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    m_scene->renderGeometryShadow();
    m_dfbo->unbind();

    m_shadowShader->unbind();
}

void ShadowMap::renderPoint()
{
    m_shadowShader->bind();

    m_shadowShader->setUniform("shadowMat", m_MVP);

    m_dfbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    m_scene->renderGeometryShadow();
    m_dfbo->unbind();

    m_shadowShader->unbind();
}

void ShadowMap::update(Camera* camera)
{
    updateMat(camera);

    if (m_light.type == LightType::LIGHT_DIRECTIONAL) {
        renderDirectional();
    } else if (m_light.type == LightType::LIGHT_POINT) {
        renderPoint();
    }
}

void ShadowMap::drawDBG()
{
    /*m_dbgShader->bind();
    m_dbgShader->setTexture("shadowMap", texture());

    ShadowMap::fsq->draw();

    m_dbgShader->unbind();*/
}
