#ifndef RAYSCENE_H
#define RAYSCENE_H

#include "Scene.h"
#include "Camera.h"
#include "Canvas2D.h"
#include "glm.hpp"
#include "IntersectionManager.h"
#include <vector>


/**
 * @class RayScene
 *
 *  Students will implement this class as necessary in the Ray project.
 */
class RayScene : public Scene {
public:
    RayScene(Scene &scene, Camera* camera);
    virtual ~RayScene();

    void render(Canvas2D* canvas, int _w, int _h);

    void eval(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c);

    void evaluateRay(std::vector<glm::vec3>& buffer, size_t w, size_t h, size_t r, size_t c, glm::vec4 p, glm::vec4 d);

    inline glm::vec4 screenToFilm(float r, float c, float w, float h)
    {
        return glm::vec4((2 * c / w) - 1, 1 - ((2 * r) / h), -1, 1);
    }

private:
    Camera* m_camera;

    glm::mat4 m_film_to_world;

    glm::vec4 m_eye;

    std::unique_ptr<IntersectionManager> m_intersectionManager;

};

#endif // RAYSCENE_H
