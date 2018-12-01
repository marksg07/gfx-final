#ifndef RAYSCENE_H
#define RAYSCENE_H

#include "Scene.h"
#include "ui/Canvas2D.h"
#include <vector>
#include "intersect/kdtree.h"
#include <functional>

// max number of bounces. = 0 means no bounces.
const int maxRecursion = 20;
// min weight of recursion. When weight < this, recursion does not happen.
const float minWeight = 0.00001;

/**
 * @class RayScene
 *
 *  Students will implement this class as necessary in the Ray project.
 */
class RayScene : public Scene {
public:
    RayScene(Scene &scene);
    void setDrawParams(Camera *camera, int width, int height);
    void draw(Canvas2D *canvas);
    virtual ~RayScene();
    // static for ease of use with multithreading
    static void renderWithParams(RayScene *scene, BGRA *target, int ystart, int nrows, int nsamples, std::function<bool(int, int)> renderCondition);
    static glm::vec3 colorFromRay(RayScene *scene, glm::vec4 P_ws, glm::vec4 d_ws, int recurseLevel, float recurseWeight);
    static double rayIntersect(RayScene *scene, glm::vec4 P_ws, glm::vec4 d_ws);

private:
    glm::mat4x4 m_camTransform, m_invTransform;
    glm::vec4 m_eye;
    int m_width, m_height;
    std::unique_ptr<KDTree> m_kdtree;
};



#endif // RAYSCENE_H
