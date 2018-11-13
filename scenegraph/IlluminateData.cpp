#include "IlluminateData.h"

IlluminateData::IlluminateData(glm::vec4 ray, CS123Renderable* renderable)
{
    m_inter = ray;
    m_renderable = renderable;

    // Find the intersection in world space
    m_world_inter = (m_renderable->transform * glm::vec4(m_inter.xyz(), 1));
}

glm::vec4 IlluminateData::normal()
{
    glm::mat3x3 m(m_renderable->inv_transform);

    m = glm::transpose(m);

    // Find the normal vector in world space
    return glm::normalize(glm::vec4(m * m_renderable->normal(m_inter).xyz(), 0.0));
}
