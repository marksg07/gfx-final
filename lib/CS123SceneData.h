/**
 * @file CS123SceneData.h
 *
 * Header file containing scene data structures.
 */

#ifndef __CS123_SCENE_DATA__
#define __CS123_SCENE_DATA__

#include "GL/glew.h"
#include <vector>
#include <string>
#include <QImage>
#include <QGLWidget>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/textures/Texture2D.h"
#include "gl/textures/TextureParameters.h"
#include "gl/textures/TextureParametersBuilder.h"
#include "IlluminateData.h"
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <QRgb>
#include <glm/gtx/string_cast.hpp>

const float EPSILON = 5e-4;

class CS123Renderable;
class CS123SceneMaterial;

class IlluminateData
{
public:

    IlluminateData(glm::vec4 ray, CS123Renderable* renderable);

    IlluminateData(const IlluminateData& rhs)
    {
        m_inter = rhs.m_inter;
        m_world_inter = rhs.m_world_inter;
        m_renderable = rhs.m_renderable;
    }

    inline glm::vec4& ray()
    {
        return m_inter;
    }

    inline CS123Renderable* renderable()
    {
        return m_renderable;
    }

    inline float getT()
    {
        return m_inter.w;
    }

    inline glm::vec4 intersection()
    {
        return m_world_inter;
    }

    glm::vec4 normal();

    glm::vec4 texture();


private:

    glm::vec4 m_inter;
    glm::vec4 m_world_inter;
    CS123Renderable* m_renderable;

};


enum class LightType {
    LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT, LIGHT_AREA
};

enum class PrimitiveType {
    PRIMITIVE_CUBE,
    PRIMITIVE_CONE,
    PRIMITIVE_CYLINDER,
    PRIMITIVE_TORUS,
    PRIMITIVE_SPHERE,
    PRIMITIVE_MESH
};

// Enumeration for types of transformations that can be applied to objects, lights, and cameras.
enum TransformationType {
    TRANSFORMATION_TRANSLATE, TRANSFORMATION_SCALE, TRANSFORMATION_ROTATE, TRANSFORMATION_MATRIX
};

enum Axis
{
    X, Y, Z
};

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type< Enumeration >::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

// Struct to store a RGBA color in floats [0,1]
using CS123SceneColor = glm::vec4;

// Scene global color coefficients
struct CS123SceneGlobalData  {
    float ka;  // global ambient coefficient
    float kd;  // global diffuse coefficient
    float ks;  // global specular coefficient
    float kt;  // global transparency coefficient
};

// Data for a single light
struct CS123SceneLightData {
    int id;
    LightType type;

    CS123SceneColor color;
    glm::vec3 function;  // Attenuation function

    glm::vec4 pos;       // Not applicable to directional lights
    glm::vec4 dir;       // Not applicable to point lights

    float radius;        // Only applicable to spot lights
    float penumbra;      // Only applicable to spot lights
    float angle;         // Only applicable to spot lights

    float width, height; // Only applicable to area lights
};

// Data for scene camera
struct CS123SceneCameraData {
    glm::vec4 pos;
    glm::vec4 look;
    glm::vec4 up;

    float heightAngle;
    float aspectRatio;

    float aperture;      // Only applicable for depth of field
    float focalLength;   // Only applicable for depth of field
};

// Data for file maps (ie: texture maps)
struct CS123SceneFileMap {
    //    CS123SceneFileMap() : texid(0) {}
    bool isUsed;
    std::string filename;
    float repeatU;
    float repeatV;

    void clear() {
        isUsed = false;
        repeatU = 0.0f;
        repeatV = 0.0f;
        filename = std::string();
    }
};

// Data for scene materials
struct CS123SceneMaterial {
    // This field specifies the diffuse color of the object. This is the color you need to use for
    // the object in sceneview. You can get away with ignoring the other color values until
    // intersect and ray.
    //   CS123SceneMaterial() {}
    CS123SceneColor cDiffuse;

    CS123SceneColor cAmbient;
    CS123SceneColor cReflective;
    CS123SceneColor cSpecular;
    CS123SceneColor cTransparent;
    CS123SceneColor cEmissive;

    CS123SceneFileMap textureMap;
    float blend;

    CS123SceneFileMap bumpMap;

    float shininess;

    float ior; // index of refraction

    void clear() {
        cAmbient.r = 0.0f; cAmbient.g = 0.0f; cAmbient.b = 0.0f; cAmbient.a = 0.0f;
        cDiffuse.r = 0.0f; cDiffuse.g = 0.0f; cDiffuse.b = 0.0f; cDiffuse.a = 0.0f;
        cSpecular.r = 0.0f; cSpecular.g = 0.0f; cSpecular.b = 0.0f; cSpecular.a = 0.0f;
        cReflective.r = 0.0f; cReflective.g = 0.0f; cReflective.b = 0.0f; cReflective.a = 0.0f;
        cTransparent.r = 0.0f; cTransparent.g = 0.0f; cTransparent.b = 0.0f; cTransparent.a = 0.0f;
        cEmissive.r = 0.0f; cEmissive.g = 0.0f; cEmissive.b = 0.0f; cEmissive.a = 0.0f;
        textureMap.clear();
        bumpMap.clear();
        blend = 0.0f;
        shininess = 0.0f;
        ior = 0.0;
    }
};

struct CS123ScenePrimitive {

    PrimitiveType type;
    std::string meshfile;     // Only applicable to meshes
    CS123SceneMaterial material;
};

struct TexCoord
{
    TexCoord(size_t u, size_t v)
        : u(u), v(v)
    {

    }

    size_t u;
    size_t v;
};

// Data for transforming a scene object. Aside from the TransformationType, the remaining of the
// data in the struct is mutually exclusive.
struct CS123SceneTransformation {
    TransformationType type;

    glm::vec3 translate; // The translation vector. Only valid if transformation is a translation.
    glm::vec3 scale;     // The scale vector. Only valid if transformation is a scale.
    glm::vec3 rotate;    // The axis of rotation. Only valid if the transformation is a rotation.
    float angle;         // The rotation angle in RADIANS. Only valid if transformation is a
    // rotation.

    glm::mat4x4 matrix;  // The matrix for the transformation. Only valid if the transformation is
    // a custom matrix.
};

// Structure for non-primitive scene objects
struct CS123SceneNode {
    std::vector<CS123SceneTransformation*> transformations;

    std::vector<CS123ScenePrimitive*> primitives;

    std::vector<CS123SceneNode*> children;
};

inline glm::mat4x4 transformToMatrix(CS123SceneTransformation* st)
{
    if (st->type == TRANSFORMATION_TRANSLATE)
    {
        return glm::translate(st->translate);
    } else if (st->type == TRANSFORMATION_ROTATE) {
        return glm::rotate(st->angle, st->rotate);
    } else if (st->type == TRANSFORMATION_SCALE) {
        return glm::scale(st->scale);
    }

    return st->matrix;
}
class AABB
{
public:

    void minmax(glm::vec4 corner, glm::vec2& x, glm::vec2& y, glm::vec2& z)
    {
        x.x = std::min(corner.x, x.x);
        x.y = std::max(corner.x, x.y);

        y.x = std::min(corner.y, y.x);
        y.y = std::max(corner.y, y.y);

        z.x = std::min(corner.z, z.x);
        z.y = std::max(corner.z, z.y);
    }

    AABB(const AABB& rhs)
    {
        m_max = rhs.m_max;
        m_min = rhs.m_min;
    }

    AABB()
        : m_min(-glm::vec3(INFINITY, INFINITY, INFINITY)),
          m_max(glm::vec3(INFINITY, INFINITY, INFINITY))
    {
    }

    AABB(glm::vec3 min, glm::vec3 max)
        : m_min(min), m_max(max)
    {
    }

    AABB(glm::mat4x4& transform)
    {
        std::vector<glm::vec4> corners =
        {
            glm::vec4(.5, .5, .5, 1),
            glm::vec4(.5, -.5, -.5, 1),
            glm::vec4(.5, -.5, -.5, 1),
            glm::vec4(.5, .5, .5, 1),

            glm::vec4(-.5, -.5, -.5, 1),
            glm::vec4(-.5, -.5, .5, 1),
            glm::vec4(-.5, .5, -.5, 1),
            glm::vec4(-.5, .5, .5, 1)
        };

        for(size_t i = 0; i < corners.size(); i++)
        {
            corners[i] = transform * corners[i];
        }

        glm::vec2 x(INFINITY, -INFINITY);
        glm::vec2 y(INFINITY, -INFINITY);
        glm::vec2 z(INFINITY, -INFINITY);

        for(auto corner : corners)
        {
            minmax(corner, x, y, z);
        }

        m_min = glm::vec3(x.x, y.x, z.x);
        m_max = glm::vec3(x.y, y.y, z.y);
    }

    inline glm::vec3 min()
    {
        return m_min;
    }

    inline glm::vec3 max()
    {
        return m_max;
    }

    inline float sa()
    {
        glm::vec3 dims = (m_max - m_min);

        return 2.0 * (dims.x * dims.y) + 2.0 * (dims.x * dims.z) + 2.0 * (dims.y * dims.z);
    }

private:

    glm::vec3 m_min, m_max;
};


class CS123Renderable
{
public:

    CS123Renderable(const CS123Renderable& rhs)
    {
        this->transform = rhs.transform;
        this->primitive = rhs.primitive;
        this->inv_transform = rhs.inv_transform;
        this->tex = rhs.tex;
        this->m_aabb = rhs.m_aabb;
    }

    CS123Renderable(glm::mat4x4 transform, CS123ScenePrimitive* primitive, CS123SceneGlobalData& global)
        : m_aabb(AABB(transform))
    {
        this->transform = transform;

        this->inv_transform = glm::inverse(transform);
        this->primitive = *primitive;

        this->primitive.material.cAmbient *= global.ka;
        this->primitive.material.cDiffuse *= global.kd;
        this->primitive.material.cSpecular *= global.ks;
        this->primitive.material.cReflective *= global.ks;
        this->primitive.material.cTransparent *= global.kt;


        // Calculate bounding box

        if (this->primitive.material.textureMap.isUsed)
        {

            std::string path = this->primitive.material.textureMap.filename;

            if (m_textures.find(path) == m_textures.end())
            {
                QImage img(path.c_str());

                m_textures[path] = img; //.convertToFormat(QImage::Format_RGB32);
                m_texturesGLFormat[path] = QGLWidget::convertToGLFormat(m_textures[path]);
            }

            //QImage* img = &m_texturesGLFormat[path];

            //CS123::GL::TextureParametersBuilder builder;
            //CS123::GL::TextureParameters parameters = builder.build();

            //tex = std::make_shared<CS123::GL::Texture2D>(img->bits(), img->width(), img->height());

            //parameters.applyTo(*tex.get());

        } else {
            tex = nullptr;
        }
    }

    bool approxEqual(float x, float y)
    {
        float diff = fabs(x - y);

        return (diff <= EPSILON);
    }


    glm::vec2 repeatMap(float u, float v, size_t w, size_t h)
    {
        int s = ((int) (u * primitive.material.textureMap.repeatU * w)) % w;
        int t = ((int) (v * primitive.material.textureMap.repeatV * h)) % h;

        return glm::vec2(s, t);
    }


    glm::vec2 repeatMap(glm::vec2 v, size_t w, size_t h)
    {
        return repeatMap(v.x, v.y, w, h);
    }

    AABB aabb()
    {
        return m_aabb;
    }

    float computeU(glm::vec4 pos)
    {
        float theta = atan2(pos.z, pos.x);

        float u = -theta / (2 * M_PI);
        if (theta >= 0)
        {
            u += 1;
        }

        return u;
    }

    glm::vec4 sampleTex(glm::vec2 v)
    {
        return sampleTex(v.x, v.y);
    }

    glm::vec4 sampleTex(float u, float v)
    {
        QImage& texImg = getTexture(primitive.material.textureMap.filename);

        int w = texImg.width();
        int h = texImg.height();

        glm::vec2 coord = repeatMap(u, v, w, h);

        int x = coord.x;
        int y = (h - 1) - coord.y;

        if (x < 0 || x >= w || x < 0 || y >= h)
        {
            return glm::vec4(0);
        }

        QColor color(texImg.pixel(x, y));

        return glm::vec4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    glm::vec3 getGetPlanarForm(glm::vec4 pos)
    {
        if (approxEqual(fabs(pos.x), .5))
        {
            pos.z *= (pos.x > 0) ? -1.0f : 1.0f;
            return pos.zyx();

        } else if (approxEqual(fabs(pos.y), .5))
        {
            pos.z *= (pos.y > 0) ? -1.0f : 1.0f;
            return pos.xzy();
        }

        pos.x *= (pos.z > 0) ? 1.0f : -1.0f;
        return pos.xyz();
    }

    glm::vec2 computeCubeUV(glm::vec4 pos)
    {
        glm::vec3 pForm = getGetPlanarForm(pos);

        return glm::vec2(pForm.x + .5, pForm.y + .5);
    }

    glm::vec4 texture(glm::vec4 pos)
    {
        if (primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
        {
            float phi = asin(pos.y / .5);

            float v = (phi / M_PI) + (1.0 / 2);
            float u = computeU(pos);

            return sampleTex(u, v);
        } else if (primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
        {
            if (approxEqual(pos.y, .5))
            {
                return sampleTex(pos.x + .5, 1 - (pos.z + .5));
            } else if (approxEqual(pos.y, -.5)) {
                return sampleTex(pos.x + .5, (pos.z + .5));
            }

            float v = pos.y + .5;
            float u = computeU(pos);

            return sampleTex(u, v);
        } else if (primitive.type == PrimitiveType::PRIMITIVE_CONE)
        {
            if (approxEqual(pos.y, -.5)) {
                return sampleTex(pos.x + .5, (pos.z + .5));
            }

            float v = pos.y + .5;
            float u = computeU(pos);

            return sampleTex(u, v);
        } else if (primitive.type == PrimitiveType::PRIMITIVE_CUBE)
        {
            glm::vec2 uv = computeCubeUV(pos);

            return sampleTex(uv);
        }

        return glm::vec4(0);
    }

    glm::vec4 normal(glm::vec4 pos)
    {
        if (primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
        {
            return glm::vec4(glm::normalize(pos.xyz()), 0);
        } else if (primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
            // Cap
            if (approxEqual(fabs(pos.y), .5))
            {
                return glm::normalize(glm::vec4(0, pos.y, 0, 0));
            } else {
                return glm::normalize(glm::vec4(pos.x, 0, pos.z, 0));
            }

        } else if (primitive.type == PrimitiveType::PRIMITIVE_CUBE) {

            if (approxEqual(fabs(pos.x), .5))
            {
                return glm::normalize(glm::vec4(pos.x, 0, 0, 0));
            } else if (approxEqual(fabs(pos.y), .5))
            {
                return glm::normalize(glm::vec4(0, pos.y, 0, 0));
            } else if (approxEqual(fabs(pos.z), .5))
            {
                return glm::normalize(glm::vec4(0, 0, pos.z, 0));
            }
        } else if (primitive.type == PrimitiveType::PRIMITIVE_CONE)
        {
            if (approxEqual(pos.y, -.5))
            {
                return glm::normalize(glm::vec4(0, -1, 0, 0));
            } else {

                // Lol this just works....
                glm::vec4 xz = glm::normalize(glm::vec4(pos.x, 0, pos.z, 0));

                return glm::normalize(glm::vec4(xz.x, .5, xz.z, 0));
            }
        }

        return glm::vec4(0, 0, 0, 0);
    }

    static void clearTextures()
    {
        m_textures.clear();
    }


    static QImage& getTexture(std::string name)
    {
        return m_textures[name];
    }

    AABB m_aabb;

    glm::mat4x4 transform;
    glm::mat4x4 inv_transform;
    CS123ScenePrimitive primitive;
    std::shared_ptr<CS123::GL::Texture2D> tex;

    static std::map<std::string, QImage> m_textures;
    static std::map<std::string, QImage> m_texturesGLFormat;
};

#endif

