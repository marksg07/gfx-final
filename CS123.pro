# -------------------------------------------------
# Project created by QtCreator 2010-08-22T14:12:19
# -------------------------------------------------
QT += opengl xml widgets
TARGET = CS123
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++14 -march=native
CONFIG += c++14

unix:!macx {
    LIBS += -lGLU
}
macx {
    QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
    QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
    CONFIG += c++11
}
win32 {
    DEFINES += GLEW_STATIC
    LIBS += -lopengl32 -lglu32
}

SOURCES += \
    brush/Brush.cpp \
    brush/ConstantBrush.cpp \
    brush/LinearBrush.cpp \
    brush/QuadraticBrush.cpp \
    brush/SmudgeBrush.cpp \
    camera/OrbitingCamera.cpp \
    camera/CamtransCamera.cpp \
    camera/QuaternionCamera.cpp \
    scenegraph/Scene.cpp \
    scenegraph/OpenGLScene.cpp \
    scenegraph/ShapesScene.cpp \
    scenegraph/SceneviewScene.cpp \
    scenegraph/RayScene.cpp \
    ui/Canvas2D.cpp \
    ui/SupportCanvas2D.cpp \
    ui/SupportCanvas3D.cpp \
    ui/Settings.cpp \
    ui/mainwindow.cpp \
    ui/Databinding.cpp \
    lib/BGRA.cpp \
    lib/CS123XmlSceneParser.cpp \
    lib/ResourceLoader.cpp \
    gl/shaders/Shader.cpp \
    gl/GLDebug.cpp \
    gl/datatype/VBOAttribMarker.cpp \
    gl/datatype/IBO.cpp \
    gl/datatype/FBO.cpp \
    gl/textures/Texture.cpp \
    gl/textures/Texture2D.cpp \
    gl/textures/TextureParameters.cpp \
    gl/textures/TextureParametersBuilder.cpp \
    gl/textures/RenderBuffer.cpp \
    gl/textures/DepthBuffer.cpp \
    gl/shaders/CS123Shader.cpp \
    gl/util/FullScreenQuad.cpp \
    main.cpp \
    glew-1.10.0/src/glew.c \
    brush/BrushManager.cpp \
    brush/SnakeBrush.cpp \
    brush/Snake.cpp \
    brush/FoodBrush.cpp \
    gl/datatype/VAO.cpp \
    gl/datatype/VBO.cpp \
    scenegraph/shapes/GLShape.cpp \
    scenegraph/shapes/GLCube.cpp \
    scenegraph/shapes/GLSphere.cpp \
    scenegraph/shapes/GLVertex.cpp \
    scenegraph/shapes/GLTriangle.cpp \
    scenegraph/shapes/GLCone.cpp \
    scenegraph/shapes/GLCylinder.cpp \
    scenegraph/Utils.cpp \
    scenegraph/shapes/GLTorus.cpp \
    scenegraph/shapes/GLCap.cpp \
    scenegraph/shapes/GLTransformable.cpp \
    filter/Filter.cpp \
    filter/FilterEdge.cpp \
    filter/FilterBlur.cpp \
    filter/Kernel.cpp \
    filter/FilterUtils.cpp \
    filter/FilterScale.cpp \
    filter/FilterRotate.cpp \
    filter/FilterSharpen.cpp \
    filter/FilterUnsharp.cpp \
    scenegraph/IlluminateData.cpp \
    scenegraph/IntersectionManager.cpp \
    scenegraph/KDTree.cpp \
    scenegraph/ShadowMap.cpp \
    gl/shaders/ShadowShader.cpp \
    gl/textures/DepthTexture.cpp \
    gl/datatype/DepthFBO.cpp \
    scenegraph/CubeMap.cpp \
    gl/textures/DepthCubeTexture.cpp


HEADERS += \
    brush/Brush.h \
    brush/ConstantBrush.h \
    brush/LinearBrush.h \
    brush/QuadraticBrush.h \
    brush/SmudgeBrush.h \
    camera/Camera.h \
    camera/OrbitingCamera.h \
    camera/CamtransCamera.h \
    camera/QuaternionCamera.h \
    scenegraph/Scene.h \
    scenegraph/OpenGLScene.h \
    scenegraph/ShapesScene.h \
    scenegraph/SceneviewScene.h \
    scenegraph/RayScene.h \
    ui/Canvas2D.h \
    ui/SupportCanvas2D.h \
    ui/SupportCanvas3D.h \
    ui/Settings.h \
    ui/mainwindow.h \
    ui/Databinding.h \
    ui_mainwindow.h \
    gl/shaders/Shader.h \
    gl/GLDebug.h \
    gl/shaders/ShaderAttribLocations.h \
    gl/datatype/VBOAttribMarker.h \
    gl/datatype/VBO.h \
    gl/datatype/IBO.h \
    gl/datatype/VAO.h \
    gl/datatype/FBO.h \
    gl/textures/Texture.h \
    gl/textures/Texture2D.h \
    gl/textures/TextureParameters.h \
    gl/textures/TextureParametersBuilder.h \
    gl/textures/RenderBuffer.h \
    gl/textures/DepthBuffer.h \
    gl/shaders/CS123Shader.h \
    gl/util/FullScreenQuad.h \
    lib/BGRA.h \
    lib/CS123XmlSceneParser.h \
    lib/CS123SceneData.h \
    lib/CS123ISceneParser.h \
    lib/ResourceLoader.h \
    glew-1.10.0/include/GL/glew.h \
    brush/BrushManager.h \
    brush/SnakeBrush.h \
    brush/Snake.h \
    brush/FoodBrush.h \
    scenegraph/shapes/GLShape.h \
    scenegraph/shapes/GLCube.h \
    scenegraph/shapes/GLSphere.h \
    scenegraph/shapes/GLVertex.h \
    scenegraph/shapes/GLTriangle.h \
    scenegraph/shapes/GLCone.h \
    scenegraph/shapes/GLCylinder.h \
    scenegraph/shapes/Utils.h \
    scenegraph/Utils.h \
    scenegraph/shapes/GLTorus.h \
    scenegraph/shapes/GLCap.h \
    scenegraph/shapes/GLTransformable.h \
    filter/Filter.h \
    filter/FilterEdge.h \
    filter/FilterBlur.h \
    filter/FilterUtils.h \
    filter/Kernel.h \
    filter/FilterScale.h \
    filter/FilterRotate.h \
    filter/FilterSharpen.h \
    filter/FilterUnsharp.h \
    scenegraph/Ray.h \
    scenegraph/IlluminateData.h \
    scenegraph/IntersectionManager.h \
    scenegraph/ThreadPool.h \
    scenegraph/KDTree.h \
    scenegraph/ShadowMap.h \
    gl/shaders/ShadowShader.h \
    gl/textures/DepthTexture.h \
    gl/datatype/DepthFBO.h \
    gl/textures/DepthCubeTexture.h \
    scenegraph/CubeMap.h


FORMS += ui/mainwindow.ui
INCLUDEPATH += glm brush camera lib scenegraph ui filter glew-1.10.0/include
DEPENDPATH += glm brush camera lib scenegraph ui glew-1.10.0/include
DEFINES += _USE_MATH_DEFINES
DEFINES += TIXML_USE_STL
DEFINES += GLM_SWIZZLE GLM_FORCE_RADIANS
OTHER_FILES += shaders/shader.frag \
    shaders/shader.vert \
    shaders/wireframe/wireframe.vert \
    shaders/wireframe/wireframe.frag \
    shaders/normals/normals.vert \
    shaders/normals/normals.gsh \
    shaders/normals/normals.frag \
    shaders/normals/normalsArrow.vert \
    shaders/normals/normalsArrow.gsh \
    shaders/normals/normalsArrow.frag \
    shaders/deferredlighting/gbuffer/gbuffer.frag \
    shaders/deferredlighting/gbuffer/gbuffer.vert \
    shaders/deferredlighting/lighting/lighting.frag \
    shaders/deferredlighting/lighting/lighting.vert \
    shaders/deferredlighting/compositing/compositing.frag \
    shaders/deferredlighting/compositing/compositing.vert \
    README.txt \
    shaders/test/shader.frag \
    shaders/test/shader.vert \
    Shapes_README.txt \
    Filter_README.txt \
    shaders/texture.frag \
    Sceneview_README.txt \
    Intersect_README.txt \
    Ray_README.txt \
    shaders/shadow.frag \
    shaders/shadow.vert \
    shaders/shadow_map.frag \
    shaders/shadow_map.vert \
    shaders/shadowPoint.frag \
    shaders/shadowPoint.vert \
    shaders/shadowPoint.gsh \
    shaders/skybox.vert \
    shaders/skybox.frag

# Don't add the -pg flag unless you know what you are doing. It makes QThreadPool freeze on Mac OS X
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_WARN_ON -= -Wall
QMAKE_CXXFLAGS_WARN_ON += -Waddress -Warray-bounds -Wc++0x-compat -Wchar-subscripts -Wformat\
                          -Wmain -Wmissing-braces -Wparentheses -Wreorder -Wreturn-type \
                          -Wsequence-point -Wsign-compare -Wstrict-overflow=1 -Wswitch \
                          -Wtrigraphs -Wuninitialized -Wunused-label -Wunused-variable \
                          -Wvolatile-register-var -Wno-extra

QMAKE_CXXFLAGS += -g

# QMAKE_CXX_FLAGS_WARN_ON += -Wunknown-pragmas -Wunused-function -Wmain

macx {
    QMAKE_CXXFLAGS_WARN_ON -= -Warray-bounds -Wc++0x-compat
}

RESOURCES += \
    resources.qrc

DISTFILES += \
    shaders/normals/normals.vert \
    shaders/normals/normals.frag \
    shaders/normals/normals.gsh \
    shaders/normals/normalsArrow.gsh \
    shaders/normals/normalsArrow.frag \
    shaders/normals/normalsArrow.vert
