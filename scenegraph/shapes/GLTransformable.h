#ifndef GLTRANSFORMABLE_H
#define GLTRANSFORMABLE_H

#include "glm/gtx/transform.hpp"
#include "GL/glew.h"
#include "glm/glm.hpp"

class GLTransformable
{
public:
    GLTransformable();

    virtual GLTransformable& reflect(GLfloat x, GLfloat y, GLfloat z)
    {
        return reflect(glm::vec3(x, y, z));
    }

    virtual GLTransformable& reflect(glm::vec3 v) = 0;

    virtual GLTransformable& rotate(float angle, GLfloat x, GLfloat y, GLfloat z)
    {
        return rotate(angle, glm::vec3(x, y, z));
    }

    virtual GLTransformable& rotate(float angle, glm::vec3 v) = 0;

    virtual GLTransformable& apply(glm::mat4 t) = 0;

    virtual GLTransformable& translate(GLfloat x, GLfloat y, GLfloat z)
    {
        return translate(glm::vec3(x, y, z));
    }

    virtual GLTransformable& translate(glm::vec3 v) = 0;
};

#endif // GLTRANSFORMABLE_H
