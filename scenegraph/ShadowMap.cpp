#include "ShadowMap.h"
#include "Settings.h"

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

#if 0
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    TextureParametersBuilder().build().applyTo(depthMapTexture);


    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    m_dfbo = std::make_unique<DepthFBO>(m_width, m_height);

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

    //std::cout << "min: " << glm::to_string(min) << ", max: " << glm::to_string(max) << std::endl;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::mat4 v = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), m_light.dir.xyz(), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 p = glm::ortho(min.x, max.x, min.y, max.y, -5.0f, max.z);

    /*glm::vec3 lightInvDir = glm::vec3(0.5f,2,2);
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
    m_MVP = depthProjectionMatrix * depthViewMatrix;*/

    //m_biasMatrix *
    m_MVP = p * v * glm::mat4(1.0);
    m_biasMVP = m_biasMatrix * m_MVP;
}

void ShadowMap::update(Camera* camera)
{
    if (m_light.type != LightType::LIGHT_DIRECTIONAL)
    {
        return;
    }

    updateMat(camera);

    m_shadowShader->bind();

    m_shadowShader->setUniform("shadowMat", m_MVP);

    m_dfbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    m_scene->renderGeometryShadow();
    m_dfbo->unbind();

    m_shadowShader->unbind();
}

void ShadowMap::drawDBG()
{
    m_dbgShader->bind();
    m_dbgShader->setTexture("shadowMap", texture());

    ShadowMap::fsq->draw();

    m_dbgShader->unbind();
}
