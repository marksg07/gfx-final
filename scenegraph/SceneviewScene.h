#ifndef SCENEVIEWSCENE_H
#define SCENEVIEWSCENE_H

#include "OpenGLScene.h"
#include "shapes/openglshape.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include "gl/util/FullScreenQuad.h"
#include "CubeMap.h"
#include "ShadowMap.h"
#include "shapes/tetmesh.h"
#include "gl/util/FullScreenQuad.h"
#include "gl/datatype/FBO.h"

namespace CS123 { namespace GL {

    class Shader;
    class CS123Shader;
    class Texture2D;
}}

class ShadowMap;

/**
 *
 * @class SceneviewScene
 *
 * A complex scene consisting of multiple objects. Students will implement this class in the
 * Sceneview assignment.
 *
 * Here you will implement your scene graph. The structure is up to you - feel free to create new
 * classes and data structures as you see fit. We are providing this SceneviewScene class for you
 * to use as a stencil if you like.
 *
 * Keep in mind that you'll also be rendering entire scenes in the next two assignments, Intersect
 * and Ray. The difference between this assignment and those that follow is here, we are using
 * OpenGL to do the rendering. In Intersect and Ray, you will be responsible for that.
 */
class SceneviewScene : public OpenGLScene {
public:
    SceneviewScene();
    virtual ~SceneviewScene();

    void parsingDone(); // essentially a callback for when initialization w/ Scene::parse is done

    virtual void render(SupportCanvas3D *context) override;
    virtual void settingsChanged() override;

    // Use this method to set an internal selection, based on the (x, y) position of the mouse
    // pointer.  This will be used during the "modeler" lab, so don't worry about it for now.
    void setSelection(int x, int y);
    void startSimulation() {
        m_running = true;
    }
    void stopSimulation() {
        m_running = false;
    }
    void stepSimulation(SupportCanvas3D *context) {
        bool restore = m_running;
        m_running = true;
        render(context);
        m_running = restore;
    }

    void restartSimulation() {

    }


    void renderGeometry(CS123::GL::Shader* shader);
    void onResize(int, int);
    void delete_all();
    void create_random();

private:


private:
    void loadPhongShader();
    void loadWireframeShader();
    void loadNormalsShader();
    void loadNormalsArrowShader();
    void loadFragShader();
    void loadShadowShader();
    void loadShadowMapShader();
    void loadShadowPointShader();
    void loadSkyboxShader();

    void setSceneUniforms(SupportCanvas3D *context);
    void setMatrixUniforms(CS123::GL::Shader *shader, SupportCanvas3D *context);
    void setLights();
    void renderGeometry();

    std::unique_ptr<CS123::GL::CS123Shader> m_phongShader;
    std::unique_ptr<CS123::GL::Shader> m_wireframeShader;
    std::unique_ptr<CS123::GL::Shader> m_normalsShader;
    std::unique_ptr<CS123::GL::Shader> m_normalsArrowShader;
    std::unique_ptr<CS123::GL::Shader> m_textureShader;
    std::shared_ptr<CS123::GL::Shader> m_shadowShader;
    std::shared_ptr<CS123::GL::Shader> m_shadowPointShader;
    std::shared_ptr<CS123::GL::Shader> m_skyboxShader;
    std::shared_ptr<CS123::GL::Shader> m_shadowMapShader;
    std::unique_ptr<CS123::GL::FullScreenQuad> m_fsq;
    std::unique_ptr<CubeMap> m_skybox = nullptr;
    std::unique_ptr<FBO> m_fbo;


    std::vector<std::shared_ptr<ShadowMap>> m_shadowMaps;


    std::unordered_map<std::string, std::unique_ptr<TetMesh>> m_meshTemplateCache;
    std::vector<std::unique_ptr<TetMesh>> m_meshes;
    bool m_running;
    bool m_ready;
    std::mutex initializationMutex;
};

#endif // SCENEVIEWSCENE_H
