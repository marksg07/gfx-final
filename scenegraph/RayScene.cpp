#include "RayScene.h"
#include "Settings.h"
#include "CS123SceneData.h"
#include "IlluminateData.h"
#include "IntersectionManager.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <thread>
#include "ThreadPool.h"

RayScene::RayScene(Scene &scene, Camera *camera) :
    Scene(scene)
{
    // TODO [INTERSECT]
    // Remember that any pointers or OpenGL objects (e.g. texture IDs) will
    // be deleted when the old scene is deleted (assuming you are managing
    // all your memory properly to prevent memory leaks).  As a result, you
    // may need to re-allocate some things here.

    m_camera = camera;

    m_film_to_world = glm::inverse(camera->getViewMatrix()) * glm::inverse(camera->getScaleMatrix());
    m_eye = m_film_to_world * glm::vec4(0, 0, 0, 1);
    m_intersectionManager = std::make_unique<IntersectionManager>(m_renderables);
}

RayScene::~RayScene()
{
}


inline unsigned char REAL2byte(float f) {
    int i = static_cast<int>((f * 255.0 + 0.5));
    return (i < 0) ? 0 : (i > 255) ? 255 : i;
}

inline void toBGRA(BGRA* pix, glm::vec3 c) {
    *pix = BGRA(REAL2byte(c.x), REAL2byte(c.y), REAL2byte(c.z));
}

inline float fatt(float d, glm::vec3 att)
{
    return std::min(1.0, 1.0/(att.x + (att.y * d) + (att.z * d * d)));
}


bool RayScene::occluded(glm::vec4 pos, glm::vec4 lightDir, CS123SceneLightData& light)
{

    std::vector<IlluminateData> linter = m_intersectionManager->getIntersections(pos + (EPSILON * lightDir), lightDir);

    if (light.type == LightType::LIGHT_DIRECTIONAL)
    {
        return linter.size() != 0;
    }

    for(auto& ill : linter)
    {
        glm::vec4 posToLight = light.pos - pos;

        // Check if we hit an object before we hit the light.
        if (glm::distance(pos, ill.intersection()) < glm::length(posToLight))
        {
            return true;
        }
    }

    return false;
}

glm::vec3 RayScene::computePointLight(CS123SceneLightData& light, glm::vec4 d, IlluminateData& inter, glm::vec4 tex)
{
    CS123SceneMaterial* material = &inter.renderable()->primitive.material;
    glm::vec4 inter_pos = inter.intersection() - (d * EPSILON);
    glm::vec4 n = inter.normal();

    glm::vec4 lightDirWorld = glm::vec4(glm::vec3(light.pos - inter.intersection()), 0.0);
    glm::vec4 l = glm::normalize(lightDirWorld);

    if (settings.useShadows && occluded(inter_pos, l, light))
    {
        return glm::vec3(0);
    }

    glm::vec4 diffuse = material->cDiffuse;
    if (material->textureMap.isUsed && settings.useTextureMapping)
    {
        diffuse = glm::mix(material->cDiffuse, tex, material->blend);
    }

    glm::vec4 dif = glm::vec4(diffuse * glm::clamp(glm::dot(n, l), 0.0f, 1.0f));

    glm::vec4 spec = material->cSpecular * glm::pow(glm::clamp(glm::dot(glm::reflect(l, n), d), 0.0f, 1.0f), material->shininess);

    return (fatt(glm::length(lightDirWorld), light.function) * light.color * (dif + spec)).xyz();
}

glm::vec3 RayScene::computeDirectionalLight(CS123SceneLightData& light, glm::vec4 d, IlluminateData& inter, glm::vec4 tex)
{
    CS123SceneMaterial* material = &inter.renderable()->primitive.material;

    glm::vec4 inter_pos = inter.intersection() - (d * EPSILON);
    glm::vec4 n = inter.normal();
    glm::vec4 l = -glm::normalize(light.dir);

    if (settings.useShadows && occluded(inter_pos, l, light))
    {
        return glm::vec3(0);
    }


    glm::vec4 diffuse = material->cDiffuse;
    if (material->textureMap.isUsed && settings.useTextureMapping)
    {
        diffuse = glm::mix(material->cDiffuse, tex, material->blend);
    }

    glm::vec4 dif = glm::vec4(diffuse * glm::clamp(glm::dot(n, l), 0.0f, 1.0f));

    glm::vec4 spec = material->cSpecular * glm::pow(glm::clamp(glm::dot(glm::reflect(l, n), d), 0.0f, 1.0f), material->shininess);

    return (light.color * (dif + spec)).xyz();
}

glm::vec3 RayScene::computeAreaLight(CS123SceneLightData& light, glm::vec4 d, IlluminateData& inter, glm::vec4 tex)
{

}

glm::vec3 RayScene::computeSpotLight(CS123SceneLightData& light, glm::vec4 d, IlluminateData& inter, glm::vec4 tex)
{

}


glm::vec3 RayScene::computeLight(std::vector<CS123SceneLightData>& lights, glm::vec4 d, IlluminateData& inter)
{
    glm::vec3 intensity(0.f, 0.f, 0.f);

    glm::vec4 tex = (inter.renderable()->primitive.material.textureMap.isUsed && settings.useTextureMapping) ? inter.texture() : glm::vec4(0);

    for(CS123SceneLightData& light : m_lights)
    {
        if (light.type == LightType::LIGHT_POINT && settings.usePointLights)
        {
            intensity += computePointLight(light, d, inter, tex);
        } else if (light.type == LightType::LIGHT_DIRECTIONAL && settings.useDirectionalLights)
        {
            intensity += computeDirectionalLight(light, d, inter, tex);
        }
    }

    return intensity;
}


glm::vec3 RayScene::computeRay(glm::vec4 p, glm::vec4 d, size_t recursion_depth)
{
    // Prevent infinite recursion
    if (recursion_depth >= MAX_RECURSION_DEPTH)
    {
        return glm::vec3(1,1,1);
    }


    std::vector<IlluminateData> intersections = m_intersectionManager->getIntersections(p, d);
    if (intersections.size() != 0)
    {
        // Find the nearest intersection by checking the distance between the point from where we shot the ray to the point of intersection.
        IlluminateData inter = *std::min_element(intersections.begin(), intersections.end(), [p](IlluminateData a, IlluminateData b) {
                return glm::distance(p, a.intersection()) < glm::distance(p, b.intersection());
    });

        CS123SceneMaterial* material = &inter.renderable()->primitive.material;

        glm::vec3 ret = material->cAmbient.xyz();

        ret += computeLight(m_lights, d, inter);

        if (settings.useReflection)
        {
            glm::vec4 interPos = inter.intersection() - (EPSILON * d);
            glm::vec4 n = inter.normal();

            ret += material->cReflective.xyz() * computeRay(interPos, glm::reflect(d, n), recursion_depth + 1);
        }

        return ret;
    }

    return glm::vec3(0, 0, 0);
}

/*
void RayScene::evaluateRay(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c, glm::vec4 p, glm::vec4 d)
{
    buffer[(r * w) + c] = computeReflectedRay()
}*/

void RayScene::eval(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c)
{
    glm::vec4 p = m_eye;

    if (!settings.useSuperSampling)
    {
        glm::vec4 d = glm::normalize(glm::vec4(((m_film_to_world * screenToFilm(r + .5, c + .5, w, h)) - m_eye).xyz(), 0));
        buffer[(r * w) + c] = computeRay(p, d);

        return;
    }

    int samples = settings.numSuperSamples;

    // top left
    glm::vec2 tl = glm::vec2(c - .5, r - .5);
    float step = 1.0 / samples;

    for(int step_row = 0; step_row < samples; step_row++)
    {
        for(int step_col = 0; step_col < samples; step_col++)
        {
            float r_sub = tl.y + (step_row * step);
            float c_sub = tl.x + (step_col * step);

            glm::vec4 d = glm::normalize(glm::vec4(((m_film_to_world * screenToFilm(r_sub, c_sub, w, h)) - m_eye).xyz(), 0));
            //evaluateRay(buffer, w, h, r, c, p, d);

            buffer[(r * w) + c] += computeRay(p, d);
        }
    }

    buffer[(r * w) + c] /= (samples * samples);
}

void RayScene::render(Canvas2D* canvas, int w, int h)
{
    canvas->resize(w, h);

    BGRA* pix = canvas->data();

    // Start with a blank canvas..

    std::vector<glm::vec3> buffer;
    buffer.resize(w * h);

    memset(&buffer[0], 0, w * h * sizeof(glm::vec3));


    if (!settings.useMultiThreading)
    {
        for(size_t r = 0; r < h; r++)
        {
            for(size_t c = 0; c < w; c++)
            {
                eval(buffer, w, h, r, c);
            }
        }
    } else {


#if 1

        for(size_t r = 0; r < h; r++)
        {
            tp.addJob([&buffer, w, h, r, this]() {
                for(size_t c = 0; c < w; c++)
                {
                    eval(buffer, w, h, r, c);
                }
            });
        }

        tp.wait();

#else
        size_t num_threads = 20;
        size_t rows_per_thread = h / num_threads;

        std::vector<std::thread> threads;
        for(size_t tid = 0; tid < num_threads; tid++)
        {
            threads.push_back(std::thread([this, &buffer, tid, w, h, rows_per_thread, num_threads]() {

                size_t max_rows = (tid + 1 == num_threads) ? h : ((tid + 1) * rows_per_thread);

                for(size_t r = (tid * rows_per_thread); r < max_rows; r++)
                {
                    for(size_t c = 0; c < w; c++)
                    {
                        eval(buffer, w, h, r, c);
                    }
                }
            }));
        }

        for(auto& t : threads)
        {
            t.join();
        }

#endif
    }



    for(size_t r = 0; r < h; r++)
    {
        for(size_t c = 0; c < w; c++)
        {
            toBGRA(&pix[(r * w) + c], buffer[(r * w) + c]);
        }
    }


}
