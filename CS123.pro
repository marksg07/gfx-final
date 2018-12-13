# -------------------------------------------------
# Project created by QtCreator 2010-08-22T14:12:19
# -------------------------------------------------
QT += opengl xml
TARGET = CS123
TEMPLATE = app

QMAKE_CXXFLAGS += -Ofast -std=c++14
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
LIBS += tetgen/libtet.a
libtet.target = tetgen/libtet.a
libtet.commands = cd tetgen && make -f makefile
QMAKE_EXTRA_TARGETS += libtet
OBJECTS += tetgen/libtet.a
SOURCES += \
    camera/OrbitingCamera.cpp \
    camera/CamtransCamera.cpp \
    camera/QuaternionCamera.cpp \
    scenegraph/Scene.cpp \
    scenegraph/OpenGLScene.cpp \
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
    gl/datatype/VBO.cpp \
    gl/datatype/IBO.cpp \
    gl/datatype/VAO.cpp \
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
    shapes/openglshape.cpp \
    gl/util/errorchecker.cpp \
    intersect/implicitshape.cpp \
    intersect/kdtree.cpp \
    shapes/tetmesh.cpp \
    shapes/tetmeshparser.cpp \
    shapes/timing.cpp \
    gl/textures/DepthCubeTexture.cpp \
    gl/textures/DepthTexture.cpp \
    scenegraph/CubeMap.cpp \
    scenegraph/ShadowMap.cpp \
    gl/datatype/DepthFBO.cpp

HEADERS += \
    camera/Camera.h \
    camera/OrbitingCamera.h \
    camera/CamtransCamera.h \
    camera/QuaternionCamera.h \
    scenegraph/Scene.h \
    scenegraph/OpenGLScene.h \
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
    shapes/openglshape.h \
    gl/util/errorchecker.h \
    intersect/implicitshape.h \
    intersect/kdtree.h \
    shapes/tetmesh.h \
    tetgen/tetgen.h \
    shapes/tetmeshparser.h \
    shapes/timing.h \
    gl/textures/DepthCubeTexture.h \
    gl/textures/DepthTexture.h \
    scenegraph/CubeMap.h \
    scenegraph/ShadowMap.h \
    gl/datatype/DepthFBO.h


FORMS += ui/mainwindow.ui
INCLUDEPATH += glm camera lib scenegraph ui glew-1.10.0/include tetgen Eigen
DEPENDPATH += glm camera lib scenegraph ui glew-1.10.0/include tetgen
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
    shaders/fullscreenquad/fullscreenquad.frag \
    shaders/fullscreenquad/fullscreenquad.vert \
    shaders/shadowPoint.gsh \
    shaders/shadow_map.frag \
    shaders/shadow.frag \
    shaders/shadowPoint.frag \
    shaders/skybox.frag \
    shaders/solid.frag \
    shaders/texture.frag \
    shaders/shadow_map.vert \
    shaders/shadow.vert \
    shaders/shadowPoint.vert \
    shaders/skybox.vert \
    shaders/solid.vert

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
