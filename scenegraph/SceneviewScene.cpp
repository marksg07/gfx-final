#include "SceneviewScene.h"
#include "GL/glew.h"
#include <QGLWidget>
#include "Camera.h"

#include "Settings.h"
#include "SupportCanvas3D.h"
#include "ResourceLoader.h"
#include "gl/shaders/CS123Shader.h"
using namespace CS123::GL;
#include "shapes/tetmesh.h"

SceneviewScene::SceneviewScene()
{
    // TODO: [SCENEVIEW] Set up anything you need for your Sceneview scene here...
    m_running = false;
    loadPhongShader();
    loadWireframeShader();
    loadNormalsShader();
    loadNormalsArrowShader();
    //glShadeModel(GL_SMOOTH);
}

void SceneviewScene::parsingDone() {
    // Setup for after we finish parsing.
    printf("mnodes size is %d\n\n\n\n", m_nodes.size());
    fflush(stdout);
    for(int i = 0; i < m_nodes.size(); i++) {
        CS123ScenePrimitive prim = m_nodes[i].primitive;
        printf("Checking prim %d\n");
        fflush(stdout);
        if(prim.type != PrimitiveType::PRIMITIVE_MESH)
            continue;
        printf("Mesh found: %s\n", prim.meshfile.c_str());
        fflush(stdout);
        TetMesh mesh(m_nodes[i], m_meshTemplateCache);
        m_meshes.push_back(mesh);
        //mesh.buildShape();
        //m_mesh[prim.meshfile] = std::move(mesh.getOpenGLShape());
    }
    //m_ready = 1;
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

void SceneviewScene::render(SupportCanvas3D *context) {
    setClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_phongShader->bind();
    setSceneUniforms(context);
    setLights();
    renderGeometry();
    glBindTexture(GL_TEXTURE_2D, 0);
    m_phongShader->unbind();

}

void SceneviewScene::setSceneUniforms(SupportCanvas3D *context) {
    Camera *camera = context->getCamera();
    m_phongShader->setUniform("useLighting", settings.useLighting);
    m_phongShader->setUniform("useArrowOffsets", false);
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
    for(int i = 0; i < m_lights.size(); i++) {
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

void SceneviewScene::renderGeometry() {
    //while(!m_ready);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(int i = 0; i < m_meshes.size(); i++) {
        TetMesh& tetmesh = m_meshes[i];
        auto onode = tetmesh.getONode();
        m_phongShader->setUniform("m", onode.trans);
        m_phongShader->applyMaterial(onode.primitive.material);
        if(m_running)
            tetmesh.update(settings.femTimeStep);
        tetmesh.draw();
    }
}
void SceneviewScene::settingsChanged() {
    // TODO: [SCENEVIEW] Fill this in if applicable.
}

