/**
 * @file   CamtransCamera.cpp
 *
 * This is the perspective camera class you will need to fill in for the Camtrans lab.  See the
 * lab handout for more details.
 */

#include "CamtransCamera.h"
#include <Settings.h>

// lazy function for converting to vec3, crossing, back to vec4.
// assumes w=0.
static inline glm::vec4 crossVec4(glm::vec4 a, glm::vec4 b) {
    return glm::vec4(glm::cross(glm::vec3(a), glm::vec3(b)), 0.);
}

CamtransCamera::CamtransCamera()
{
    // @TODO Task 3: [CAMTRANS] Set up the default camera settings to match the demo...
    m_near = 1;
    m_far = 30;
    m_thetaW = 60;
    m_aspectRatio = 1;
    m_thetaH = 60; // easy b/c AR = 1:1
    m_eye = glm::vec4(2., 2., 2., 1.);
    m_w = glm::normalize(m_eye); // -m_eye is vector from eye to origin = look. -(-m_eye) = w
    m_up = glm::vec4(0., 1., 0., 0.);
    m_v = glm::normalize(m_up - glm::dot(m_up, m_w) * m_w);
    m_u = crossVec4(m_v, m_w);
}

void CamtransCamera::setAspectRatio(float a)
{
    m_aspectRatio = a;
    updateProjectionMatrix();
}

glm::mat4x4 CamtransCamera::getProjectionMatrix() const {
    return m_perspectiveTransformation * m_scaleMatrix;
}

glm::mat4x4 CamtransCamera::getViewMatrix() const {
    return m_rotationMatrix * m_translationMatrix;
}

glm::mat4x4 CamtransCamera::getScaleMatrix() const {
    return m_scaleMatrix;
}

glm::mat4x4 CamtransCamera::getPerspectiveMatrix() const {
    return m_perspectiveTransformation;
}

glm::vec4 CamtransCamera::getPosition() const {
    return m_eye;
}

glm::vec4 CamtransCamera::getU() const {
    return m_u;
}

glm::vec4 CamtransCamera::getV() const {
    return m_v;
}

glm::vec4 CamtransCamera::getW() const {
    return m_w;
}

glm::vec4 CamtransCamera::getLook() const {
    return -m_w;
}

glm::vec4 CamtransCamera::getUp() const {
    return m_up;
}

float CamtransCamera::getAspectRatio() const {
    return m_aspectRatio;
}

float CamtransCamera::getHeightAngle() const {
    return m_thetaH;
}

void CamtransCamera::orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up) {

      m_eye = eye;
      m_up = up;
      m_w = -glm::normalize(look);
      m_v = glm::normalize(m_up - glm::dot(m_up, m_w) * m_w);
      m_u = crossVec4(m_v, m_w);
      updateViewMatrix();
      updateProjectionMatrix();
}

void CamtransCamera::setHeightAngle(float h) {
      m_thetaH = h;
      updateProjectionMatrix();
}

void CamtransCamera::translate(const glm::vec4 &v) {
    m_eye += v;
    updateViewMatrix();
}

void CamtransCamera::rotateU(float degrees) {
    float rads = glm::radians(degrees);
    float s = glm::sin(rads);
    float c = glm::cos(rads);

    glm::vec4 newv = m_v * c + m_w * s;
    m_w = -m_v * s + m_w * c;
    m_v = newv;
    updateViewMatrix();
}

void CamtransCamera::rotateV(float degrees) {
    float rads = glm::radians(degrees);
    float s = glm::sin(rads);
    float c = glm::cos(rads);

    glm::vec4 newu = m_u * c - m_w * s;
    m_w = m_u * s + m_w * c;
    m_u = newu;
    updateViewMatrix();
}

void CamtransCamera::rotateW(float degrees) {
    float rads = glm::radians(degrees);
    float s = glm::sin(rads);
    float c = glm::cos(rads);

    glm::vec4 newv = m_v * c - m_u * s;
    m_u = m_v * s + m_u * c;
    m_v = newv;
    updateViewMatrix();
}

void CamtransCamera::setClip(float nearPlane, float farPlane) {
    m_near = nearPlane;
    m_far = farPlane;
    updateProjectionMatrix();
}

// @TODO Task 1: Define the helper methods for updating the matrices here...
// @TODO Task 2: Fill in the helper methods you created for updating the matrices...
void CamtransCamera::updateProjectionMatrix() {
    updatePerspectiveMatrix();
    updateScaleMatrix();
}

void CamtransCamera::updatePerspectiveMatrix() {
    float c = -m_near / m_far;
    m_perspectiveTransformation = glm::transpose(glm::mat4x4(1, 0, 0, 0,
                                                             0, 1, 0, 0,
                                                             0, 0, -1/(c+1), c/(c+1),
                                                             0, 0, -1, 0));
}

void CamtransCamera::updateScaleMatrix() {
    float invfar = 1 / m_far;
    float yscale = invfar / tan(glm::radians(m_thetaH / 2.));
    // note: we don't have to calc thetaW b/c we know AR
    float xscale = yscale / m_aspectRatio;
    m_scaleMatrix = glm::mat4x4(xscale, 0, 0, 0,
                                0, yscale, 0, 0,
                                0, 0, invfar, 0,
                                0, 0, 0, 1);
}

void CamtransCamera::updateViewMatrix() {
    updateRotationMatrix();
    updateTranslationMatrix();
}

void CamtransCamera::updateRotationMatrix() {
    m_rotationMatrix = glm::mat4x4(m_u.x, m_u.y, m_u.z, 0,
                                   m_v.x, m_v.y, m_v.z, 0,
                                   m_w.x, m_w.y, m_w.z, 0,
                                   0, 0, 0, 1);
}

void CamtransCamera::updateTranslationMatrix() {
    m_translationMatrix = glm::transpose(glm::mat4x4(1, 0, 0, -m_eye.x,
                                                     0, 1, 0, -m_eye.y,
                                                     0, 0, 1, -m_eye.z,
                                                     0, 0, 0, 1));
}
