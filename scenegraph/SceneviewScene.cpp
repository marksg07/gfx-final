
#include "SceneviewScene.h"
#include "GL/glew.h"
#include <QGLWidget>
#include <QApplication>
#include "Camera.h"
#include "Settings.h"
#include "SupportCanvas3D.h"
#include "ResourceLoader.h"
#include "gl/shaders/CS123Shader.h"
#include "gl/shaders/Shader.h"
#include "CubeMap.h"
using namespace CS123::GL;
#include "shapes/tetmesh.h"


double fps = 0;
int n = 1;

std::vector<double> pastFrames;
double approxRollingAverage(double new_sample) {

    pastFrames.push_back(new_sample);

    float avg = 0;
    int start = std::max((int) (pastFrames.size() - 101), 0);
    for(size_t i = start; i < pastFrames.size(); i++)
    {
        avg += pastFrames[i];
    }
    avg = avg / (pastFrames.size() - start);

    if (pastFrames.size() == 1000) {
        std::vector<double> nFrames;

        size_t start = pastFrames.size() - 101;
        for(size_t i = start; i < pastFrames.size(); i++)
        {
            nFrames.push_back(pastFrames[i]);
        }
        pastFrames = nFrames;
    }

    return avg;
}


SceneviewScene::SceneviewScene()
{
    initializationMutex.lock();
    // TODO: [SCENEVIEW] Set up anything you need for your Sceneview scene here...
    m_running = false;
    loadPhongShader();
    loadWireframeShader();
    loadNormalsShader();
    loadNormalsArrowShader();
    loadShadowShader();
    loadShadowPointShader();
    loadShadowMapShader();
    loadSkyboxShader();

    //glShadeModel(GL_SMOOTH);

    settings.useLighting = true;

    m_fsq = std::make_unique<CS123::GL::FullScreenQuad>();

    m_skybox = std::make_unique<CubeMap>(":/resources/skybox", ".jpg");

}

void SceneviewScene::parsingDone() {
    printf("Parsing done called\n");
    fflush(stdout);

    // Setup for after we finish parsing.
    printf("mnodes size is %lu\n\n\n\n", m_nodes.size());
    fflush(stdout);
    for(unsigned long i = 0; i < m_nodes.size(); i++) {
        CS123ScenePrimitive prim = m_nodes[i].primitive;
        fflush(stdout);
        if(prim.type == PrimitiveType::PRIMITIVE_MESH) {
            printf("Mesh found: %s\n", prim.meshfile.c_str());
            fflush(stdout);
            TetMesh mesh(m_nodes[i], m_meshTemplateCache);
            m_meshes.push_back(mesh);
        }
        else if(prim.type == PrimitiveType::PRIMITIVE_SPHERE) {
            object_node_t node = m_nodes[i];
            node.primitive.meshfile = "example-meshes/sphere.mesh";
            TetMesh mesh(node, m_meshTemplateCache);
            m_meshes.push_back(mesh);
        }
        else if(prim.type == PrimitiveType::PRIMITIVE_CUBE) {
            object_node_t node = m_nodes[i];
            node.primitive.meshfile = "example-meshes/cube.mesh";
            TetMesh mesh(node, m_meshTemplateCache);
            m_meshes.push_back(mesh);
        }
        else if(prim.type == PrimitiveType::PRIMITIVE_CONE) {
            object_node_t node = m_nodes[i];
            node.primitive.meshfile = "example-meshes/cone.mesh";
            TetMesh mesh(node, m_meshTemplateCache);
            m_meshes.push_back(mesh);
        }
        //mesh.buildShape();
        //m_mesh[prim.meshfile] = std::move(mesh.getOpenGLShape());
    }
    m_ready = 1;

    for(auto l : m_lights)
    {
        std::cout << "added light! " <<(int) l.type << std::endl;
        m_shadowMaps.push_back(std::make_shared<ShadowMap>(m_shadowShader, m_shadowPointShader, m_shadowMapShader, l, this));
    }

    initializationMutex.unlock();
}

SceneviewScene::~SceneviewScene()
{
}

void SceneviewScene::loadPhongShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/default.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/default.frag");
    m_phongShader = std::make_unique<CS123Shader>(vertexSource, fragmentSource);
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

void SceneviewScene::loadShadowShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow.frag");
    m_shadowShader = std::make_shared<CS123::GL::Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadSkyboxShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/skybox.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/skybox.frag");
    m_skyboxShader = std::make_shared<CS123::GL::Shader>(vertexSource, fragmentSource);
}

void SceneviewScene::loadShadowPointShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shadowPoint.vert");
    std::string geometrySource = ResourceLoader::loadResourceFileToString(":/shaders/shadowPoint.gsh");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shadowPoint.frag");
    m_shadowPointShader = std::make_shared<CS123::GL::Shader>(vertexSource, geometrySource, fragmentSource);
}


void SceneviewScene::loadShadowMapShader() {
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow_map.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shadow_map.frag");
    m_shadowMapShader = std::make_shared<CS123::GL::Shader>(vertexSource, fragmentSource);
}


int frames = 0;
QTime m_timer;

void SceneviewScene::render(SupportCanvas3D *context) {

    m_timer.start();

    setClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_lights.size() == 0){
        return;
    }
    // shadow mapping

    if (settings.useShadowMapping) {
        for(auto& m : m_shadowMaps)
        {
            m->update(context->getCamera());
        }
    }

    m_fbo->bind();

    // Restore...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_BACK);


    m_phongShader->bind();

    m_phongShader->setUniform("numLights", (int) m_lights.size());

    for(size_t i = 0; i < m_lights.size(); i++)
    {
        m_shadowMaps[i]->prepareShader(m_phongShader.get(), "shadow", i);
    }

    setSceneUniforms(context);
    setLights();
    renderGeometry();


    m_phongShader->unbind();

    glDepthFunc(GL_LEQUAL);
    m_skyboxShader->bind();

    //m_skybox->textureId()

    m_skyboxShader->setUniform("v", glm::mat4(glm::mat3(context->getCamera()->getViewMatrix())));
    m_skyboxShader->setUniform("p", context->getCamera()->getProjectionMatrix());
    m_skyboxShader->setTexture("skybox", GL_TEXTURE_CUBE_MAP, m_skybox->textureId());

    m_skybox->draw();

    m_skyboxShader->unbind();
    glDepthFunc(GL_LESS);


    m_fbo->unbind();


    float ratio = static_cast<QGuiApplication *>(QCoreApplication::instance())->devicePixelRatio();
    glViewport(0, 0, context->width() * ratio, context->height() * ratio);

    // m_shadowMapShader is actually the FXAA shader lol
    m_shadowMapShader->bind();
    float w = context->width() * ratio;
    float h = context->height() * ratio;
    m_shadowMapShader->setUniform("showFXAAEdges", settings.showFXAAEdges);
    m_shadowMapShader->setUniform("inverseScreenSize", glm::vec2(1.0 / w, 1.0 / h));
    m_fbo->getColorAttachment(0).bind();
    m_fsq->draw();
    m_shadowMapShader->unbind();

    fps = approxRollingAverage(1.0 / (float(m_timer.elapsed()) / 1000.0f));
    frames++;

    if (frames % 100 == 0) {
        printf("FPS: %f\n", fps);
        context->setFPS(fps);
        frames = 0;
    }
}

void SceneviewScene::setSceneUniforms(SupportCanvas3D *context) {
    Camera *camera = context->getCamera();
    //m_phongShader->setUniform("useLighting", settings.useLighting);
    //m_phongShader->setUniform("useArrowOffsets", false);
    m_phongShader->setUniform("p" , camera->getProjectionMatrix());
    m_phongShader->setUniform("v", camera->getViewMatrix());
}

void SceneviewScene::setMatrixUniforms(Shader *shader, SupportCanvas3D *context) {
    shader->setUniform("p", context->getCamera()->getProjectionMatrix());
    shader->setUniform("v", context->getCamera()->getViewMatrix());
}

void SceneviewScene::setLights()
{
    // TODO: [SCENEVIEW] Fill this in...
    //
    // Set up the lighting for your scene using m_phongShader.
    // The lighting information will most likely be stored in CS123SceneLightData structures.
    //
    for(unsigned long i = 0; i < m_lights.size(); i++) {
        m_phongShader->setLight(m_lights[i]);
    }
}

/*void SceneviewScene::renderGeometry() {
    //while(!m_ready);
    printf("mnodes size is %d\n\n\n\n", m_nodes.size());
    fflush(stdout);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#define SWMAP(type, inst) \
    case PrimitiveType::PRIMITIVE_ ## type : \
        inst->draw(); \
        break;

    for(int i = 0; i < m_nodes.size(); i++) {
        CS123ScenePrimitive prim = m_nodes[i].primitive;
        //printf("Drawing primitive of type %d\n", prim.type);
        glm::mat4x4 trans = m_nodes[i].trans;
        m_phongShader->setUniform("m", trans);
        m_phongShader->applyMaterial(prim.material);


        switch(prim.type) {
            SWMAP(CUBE, m_cube)
            SWMAP(SPHERE, m_sphere)
            SWMAP(CYLINDER, m_cylinder)
            SWMAP(CONE, m_cone)
            case PrimitiveType::PRIMITIVE_MESH:
                //assert(m_meshes.count(prim.meshfile) != 0);
                //m_meshes[prim.meshfile]->update(1);
                //m_meshes[prim.meshfile]->draw();
                break;
            default:
                break;
        }
    }

#undef SWMAP

    // TODO: [SCENEVIEW] Fill this in...
    // You shouldn't need to write *any* OpenGL in this class!
    //
    //
    // This is where you should render the geometry of the scene. Use what you
    // know about OpenGL and leverage your Shapes classes to get the job done.

}*/

void SceneviewScene::renderGeometry(CS123::GL::Shader* shader) {
    //while(!m_ready);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(unsigned long i = 0; i < m_meshes.size(); i++) {
        TetMesh& tetmesh = m_meshes[i];
        auto onode = tetmesh.getONode();
        shader->setUniform("m", glm::mat4(1.0f));
        //shader->applyMaterial(onode.primitive.material);
        tetmesh.draw();
    }
}

void SceneviewScene::renderGeometry() {
    //while(!m_ready);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(unsigned long i = 0; i < m_meshes.size(); i++) {
        TetMesh& tetmesh = m_meshes[i];
        auto onode = tetmesh.getONode();
        m_phongShader->setUniform("m", glm::mat4(1.0f));
        m_phongShader->applyMaterial(onode.primitive.material);
        if(m_running) {
            float timePerStep = settings.femTimeStep / settings.femStepsPerFrame;
            for(int j = 0; j < settings.femStepsPerFrame; j++) {
                tetmesh.update(timePerStep);
            }
        }
        tetmesh.draw();
    }
}
void SceneviewScene::settingsChanged() {
    // TODO: [SCENEVIEW] Fill this in if applicable.
}

void SceneviewScene::onResize(int width, int height) {
    int a = std::max(width, height);
    m_fbo = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, a, a,
                                  TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE);
}

