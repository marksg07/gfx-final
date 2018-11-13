#include "RayScene.h"
#include "Settings.h"
#include "Ray.h"
#include "CS123SceneData.h"
#include "IlluminateData.h"
#include "IntersectionManager.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <thread>

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

void RayScene::evaluateRay(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c, glm::vec4 p, glm::vec4 d)
{
    std::vector<IlluminateData> intersections = m_intersectionManager->getIntersections(p, d);
    if (intersections.size() != 0)
    {
        // Find the nearest intersection by checking the distance between the point from where we shot the ray to the point of intersection.
        IlluminateData inter = *std::min_element(intersections.begin(), intersections.end(), [p](IlluminateData a, IlluminateData b) {
            return glm::distance(p, a.intersection()) < glm::distance(p, b.intersection());
        });

        CS123SceneMaterial* material = &inter.renderable()->primitive.material;
        buffer[(r * w) + c] += glm::vec3(material->cAmbient.xyz());

        for(CS123SceneLightData light : m_lights)
        {
            if (light.type != LightType::LIGHT_POINT)
            {
                continue;
            }

            glm::vec4 lightDirWorld = glm::vec4(glm::vec3(light.pos - inter.intersection()), 0.0);

            glm::vec4 l = glm::normalize(lightDirWorld);
            buffer[(r * w) + c] += glm::vec3(light.color * material->cDiffuse * glm::clamp(glm::dot(inter.normal(), l), 0.0f, 1.0f));
        }
    }
}

void RayScene::eval(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c)
{
    glm::vec4 p = m_eye;

    if (!settings.useSuperSampling)
    {
        glm::vec4 d = glm::normalize(glm::vec4(((m_film_to_world * screenToFilm(r + .5, c + .5, w, h)) - m_eye).xyz(), 0));
        evaluateRay(buffer, w, h, r, c, p, d);

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
            evaluateRay(buffer, w, h, r, c, p, d);
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
    }



    for(size_t r = 0; r < h; r++)
    {
        for(size_t c = 0; c < w; c++)
        {
            toBGRA(&pix[(r * w) + c], buffer[(r * w) + c]);
        }
    }


}
