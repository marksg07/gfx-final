#ifndef SCENEVIEWSCENE_H
#define SCENEVIEWSCENE_H

#include "OpenGLScene.h"

#include <map>
#include <memory>
#include "shapes/GLShape.h"
#include "CS123SceneData.h"
#include "gl/shaders/ShadowShader.h"
#include "gl/datatype/FBO.h"
#include "gl/util/FullScreenQuad.h"
#include "ShadowMap.h"

class ShadowMap;

namespace CS123 { namespace GL {

    class Shader;
    class CS123Shader;
    class Texture2D;
}}

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
    SceneviewScene(size_t w, size_t h);
    virtual ~SceneviewScene();

    virtual void render(SupportCanvas3D *context) override;
    virtual void settingsChanged() override;

    // Use this method to set an internal selection, based on the (x, y) position of the mouse
    // pointer.  This will be used during the "modeler" lab, so don't worry about it for now.
    void setSelection(int x, int y);


    void renderGeometryShadow();

private:

    void loadPhongShader();
    void loadWireframeShader();
    void loadNormalsShader();
    void loadNormalsArrowShader();
    void loadFragShader();
    void loadShadowShader();
    void loadShadowMapShader();

     void addLight(const CS123SceneLightData &sceneLight) override
     {
        m_lights.push_back(sceneLight);
     }

    void setSceneUniforms(SupportCanvas3D *context);
    void setMatrixUniforms(CS123::GL::Shader *shader, SupportCanvas3D *context);
    void setLights();
    void renderGeometry();

    void setupLevelOfDetail() override;

    void renderWireframePass(SupportCanvas3D *context);
    void renderGeometryAsWireframe();

    size_t countPrimitiveInstances(PrimitiveType t);

    std::unique_ptr<CS123::GL::CS123Shader> m_phongShader;
    std::unique_ptr<CS123::GL::Shader> m_wireframeShader;
    std::unique_ptr<CS123::GL::Shader> m_normalsShader;
    std::unique_ptr<CS123::GL::Shader> m_normalsArrowShader;
    std::unique_ptr<CS123::GL::Shader> m_textureShader;
    std::shared_ptr<CS123::GL::Shader> m_shadowShader;


    std::shared_ptr<CS123::GL::Shader> m_shadowMapShader;
    std::unique_ptr<CS123::GL::FullScreenQuad> m_fsq;

    std::vector<std::shared_ptr<ShadowMap>> m_shadowMaps;

    std::map<PrimitiveType, std::shared_ptr<GLShape>> m_shapes;

    std::map<PrimitiveType, std::shared_ptr<GLShape>> m_dfl_shapes;

    size_t m_width, m_height;
};

#endif // SCENEVIEWSCENE_H
