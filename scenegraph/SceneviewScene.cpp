#include "SceneviewScene.h"
#include "GL/glew.h"
#include <QGLWidget>
#include <iostream>
#include <sstream>
#include <QGuiApplication>
#include "Camera.h"
#include "gl/util/FullScreenQuad.h"
#include "glm/gtx/string_cast.hpp"
#include "gl/textures/DepthBuffer.h"

#include "Settings.h"
#include "SupportCanvas3D.h"
#include "ResourceLoader.h"
#include "gl/shaders/CS123Shader.h"

#include "shapes/GLCube.h"
#include "shapes/GLCone.h"
#include "shapes/GLCylinder.h"
#include "shapes/GLSphere.h"
#include "shapes/GLTorus.h"

using namespace CS123::GL;

SceneviewScene::SceneviewScene(size_t w, size_t h)
    : m_width(w), m_height(h)
{
    // TODO: [SCENEVIEW] Set up anything you need for your Sceneview scene here...
    loadPhongShader();
    loadWireframeShader();
    loadNormalsShader();
    loadNormalsArrowShader();
    loadShadowShader();
    loadShadowMapShader();

    m_dfl_shapes[PrimitiveType::PRIMITIVE_CUBE] = std::make_shared<GLCube>(2, 2, 255.0);
    m_dfl_shapes[PrimitiveType::PRIMITIVE_CONE] = std::make_shared<GLCone>(10, 10, 255.0);
    m_dfl_shapes[PrimitiveType::PRIMITIVE_CYLINDER] = std::make_shared<GLCylinder>(20, 20, 255.0);
    m_dfl_shapes[PrimitiveType::PRIMITIVE_SPHERE] = std::make_shared<GLSphere>(20, 20, 255.0);

    // Has broken normals...
    m_dfl_shapes[PrimitiveType::PRIMITIVE_TORUS] = std::make_shared<GLTorus>(20, 20, 255.0);


}

SceneviewScene::~SceneviewScene()
{
}

void SceneviewScene::loadPhongShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/default.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/default.frag");
    m_phongShader = std::make_unique<CS123Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadShadowShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow.frag");
    m_shadowShader = std::make_shared<CS123::GL::Shader>(vertexSource, fragmentSource);
}


void SceneviewScene::loadShadowMapShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow_map.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow_map.frag");
    m_shadowMapShader = std::make_shared<CS123::GL::Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadWireframeShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/wireframe.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/wireframe.frag");
    m_wireframeShader = std::make_unique<Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadNormalsShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/normals.vert");
    std::string geometrySource = ResourceLoader::loadResourceFileToString(":/shaders/normals.gsh");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/normals.frag");
    m_normalsShader = std::make_unique<Shader>(vertexSource, geometrySource, fragmentSource);
}

void SceneviewScene::loadNormalsArrowShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.vert");
    std::string geometrySource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.gsh");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/normalsArrow.frag");
    m_normalsArrowShader = std::make_unique<Shader>(vertexSource, geometrySource, fragmentSource);
}

void SceneviewScene::render(SupportCanvas3D *context) {
    setClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // shadow mapping

    int light = -1;
    for (int i = 0; i < m_lights.size(); i++)
    {
        CS123SceneLightData* l = &m_lights[i];
        if (l->type == LightType::LIGHT_DIRECTIONAL)
        {
            light = i;
            break;
        }
    }

    // For now just get first directional light
    if (light == -1)
    {
        return;

    }
    // shadow mapping end

    for(auto& m : m_shadowMaps)
    {
        m->update(context->getCamera());
    }


    // Restore...
    float ratio = static_cast<QGuiApplication *>(QCoreApplication::instance())->devicePixelRatio();
    glViewport(0, 0, m_width * ratio, m_height * ratio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_BACK);


    std::shared_ptr<ShadowMap> m = m_shadowMaps[light];
    if (settings.useKDTree)
    {
        m->drawDBG();

        return;

    }

    m_phongShader->bind();

    for(size_t i = 0; i < m_lights.size(); i++)
    {
        m_phongShader->setUniformArrayByIndex("shadowMat", m_shadowMaps[i]->biasMVP(), i);
        m_phongShader->setTexture("shadowMap[" + std::to_string(i) + "]", m_shadowMaps[i]->texture());
    }

    setSceneUniforms(context);
    setLights();
    renderGeometry();


    m_phongShader->unbind();

}

void SceneviewScene::setSceneUniforms(SupportCanvas3D *context) {
    Camera *camera = context->getCamera();
    m_phongShader->setUniform("useLighting", settings.useLighting);
    //m_phongShader->setUniform("useArrowOffsets", false);
    m_phongShader->setUniform("p" , camera->getProjectionMatrix());
    m_phongShader->setUniform("v", camera->getViewMatrix());
}

void SceneviewScene::setMatrixUniforms(Shader *shader, SupportCanvas3D *context) {
    shader->setUniform("p", context->getCamera()->getProjectionMatrix());
    shader->setUniform("v", context->getCamera()->getViewMatrix());
}

size_t SceneviewScene::countPrimitiveInstances(PrimitiveType t)
{
    size_t count = 0;
    for(auto r : m_renderables)
    {
        if (r.primitive.type == t)
        {
            count++;
        }
    }

    return count;
}

inline std::shared_ptr<GLShape> makeShape(PrimitiveType t, int t1, int t2, float t3)
{
    switch(t) {
        case PrimitiveType::PRIMITIVE_CUBE:
            return std::make_shared<GLCube>(t1, t2, t3);
        case PrimitiveType::PRIMITIVE_SPHERE:
            return std::make_shared<GLSphere>(t1, t2, t3);
        case PrimitiveType::PRIMITIVE_CONE:
            return std::make_shared<GLCone>(t1, t2, t3);
        case PrimitiveType::PRIMITIVE_CYLINDER:
            return std::make_shared<GLCylinder>(t1, t2, t3);
        case PrimitiveType::PRIMITIVE_TORUS:
            return std::make_shared<GLTorus>(t1, t2, t3);
        default:
            return std::make_shared<GLCube>(t1, t2, t3);
    }
}

void SceneviewScene::setupLevelOfDetail()
{
    std::map<PrimitiveType, size_t> counts;

    for(auto it = m_dfl_shapes.begin(); it != m_dfl_shapes.end(); ++it)
    {
        counts[it->first] = countPrimitiveInstances(it->first);
    }

    for(auto it = counts.begin(); it != counts.end(); ++it)
    {
        // Render at 100% if less than 50 instances
        if (it->second < 50)
        {
            std::cout << "100: " << (int) it->first << std::endl;
            m_shapes[it->first] = makeShape(it->first, 100, 100, 1.0f);
        } else if (it->second < 100) { // Render at 50% if less than 100 instances
            m_shapes[it->first] = makeShape(it->first, 50, 50, 1.0f);
        } else if (it->second < 150) { // Render at 25% if less than 150 instances
            m_shapes[it->first] = makeShape(it->first, 25, 25, 1.0f);
        } else { // Render with defaults.
            m_shapes[it->first] = m_dfl_shapes[it->first];
        }
    }


    for(auto l : m_lights)
    {
        m_shadowMaps.push_back(std::make_shared<ShadowMap>(m_shadowShader, m_shadowMapShader, l, this));
    }
}

void SceneviewScene::setLights()
{
    // TODO: [SCENEVIEW] Fill this in...
    //
    // Set up the lighting for your scene using m_phongShader.
    // The lighting information will most likely be stored in CS123SceneLightData structures.
    //
    for (int i = 0; i < MAX_NUM_LIGHTS; i++) {
        std::ostringstream os;
        os << i;
        std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.
        m_phongShader->setUniform("lightColors" + indexString, glm::vec3(0.0f, 0.0f, 0.0f));
    }

    for(auto light : m_lights)
    {
        m_phongShader->setLight(light);
    }

    m_phongShader->setUniform("numLights", (int) m_lights.size());
}

#include "shapes/GLSphere.h"
void SceneviewScene::renderGeometryShadow() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // TODO: [SCENEVIEW] Fill this in...
    // You shouldn't need to write *any* OpenGL in this class!
    //
    //
    // This is where you should render the geometry of the scene. Use what you
    // know about OpenGL and leverage your Shapes classes to get the job done.
    //

    for(auto r : m_renderables)
    {
        //std::cout << glm::to_string(r.transform) << std::endl;
        //std::cout << "ayy: "<< glm::to_string(glm::mat4(1.0f)) << std::endl;

        m_shadowShader->setUniform("m", r.transform);
        if (m_shapes.count(r.primitive.type))
        {
            m_shapes[r.primitive.type]->draw();
        }
    }
}


void SceneviewScene::renderGeometry() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // TODO: [SCENEVIEW] Fill this in...
    // You shouldn't need to write *any* OpenGL in this class!
    //
    //
    // This is where you should render the geometry of the scene. Use what you
    // know about OpenGL and leverage your Shapes classes to get the job done.
    //

    for(auto r : m_renderables)
    {
        //std::cout << glm::to_string(r.transform) << std::endl;
        //std::cout << "ayy: "<< glm::to_string(glm::mat4(1.0f)) << std::endl;

        m_phongShader->setUniform("m", r.transform);
        m_phongShader->applyMaterial(r.primitive.material);


        if (r.primitive.material.textureMap.isUsed && 0)
        {
            Texture2D& tex = *r.tex.get();

            m_phongShader->setTexture("tex", tex);
            m_phongShader->setUniform("useTexture", 1);
            m_phongShader->setUniform("repeatUV", glm::vec2(r.primitive.material.textureMap.repeatU, r.primitive.material.textureMap.repeatV));

        } else {
            //m_phongShader->setUniform("useTexture", 0);
        }

        // Don't render shapes we don't have
        if (m_shapes.count(r.primitive.type))
        {
            m_shapes[r.primitive.type]->draw();
        }
    }
}

void SceneviewScene::settingsChanged() {
    // TODO: [SCENEVIEW] Fill this in if applicable.


}

