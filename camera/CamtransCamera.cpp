/**
 * @file   CamtransCamera.cpp
 *
 * This is the perspective camera class you will need to fill in for the Camtrans lab.  See the
 * lab handout for more details.
 */

#include "CamtransCamera.h"
#include <Settings.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

CamtransCamera::CamtransCamera()
{
    // @TODO Task 3: [CAMTRANS] Set up the default camera settings to match the demo...

    orientLook(glm::vec4(2, 2, 2, 0), glm::vec4(-2, -2, -2, 0), glm::vec4(0, 1, 0, 0));


    setAspectRatio(1.0);
    setHeightAngle(60);
    setClip(1, 30);

    updateViewMatrix();
    updateProjectionMatrix();
}

void CamtransCamera::setAspectRatio(float a)
{
    m_aspectRatio = a;
    setHeightAngle(glm::degrees(m_thetaH));
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

float CamtransCamera::getAspectRatio() const {
    return m_aspectRatio;
}

float CamtransCamera::getHeightAngle() const {
    return m_thetaH;
}

void CamtransCamera::orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up) {

    m_eye = eye;

    m_w = -glm::normalize(look);

    m_v = glm::normalize(up - m_w * glm::dot(m_w, up));

    m_u = glm::vec4(glm::cross(glm::vec3(m_v), glm::vec3(m_w)), 0);

    updateProjectionMatrix();
    updateViewMatrix();
}

void CamtransCamera::setHeightAngle(float h) {
    m_thetaH = glm::radians(h);

    float h2 = 2 * (m_far * tan(m_thetaH / 2));

    //m_aspectRatio = w/h
    float w = m_aspectRatio * h2;
    w /= 2;
    w /= m_far;

    m_thetaW = atan(w) * 2;
    updateProjectionMatrix();
}

void CamtransCamera::translate(const glm::vec4 &v) {
    m_eye += v;

    updateViewMatrix();
}

void CamtransCamera::rotateU(float degrees) {
    float theta = glm::radians(degrees);
    float st = glm::sin(theta);
    float ct = glm::cos(theta);

    glm::vec4 v = m_v * ct + m_w * st;
    glm::vec4 w = -m_v * st + m_w * ct;

    m_v = v;
    m_w = w;

    updateViewMatrix();
}

void CamtransCamera::rotateV(float degrees) {
    float theta = glm::radians(degrees);
    float st = glm::sin(theta);
    float ct = glm::cos(theta);

    glm::vec4 u = m_u * ct - m_w * st;
    glm::vec4 w = m_u * st + m_w * ct;

    m_u = u;
    m_w = w;

    updateViewMatrix();
}

void CamtransCamera::rotateW(float degrees) {
    float theta = glm::radians(degrees);
    float st = glm::sin(theta);
    float ct = glm::cos(theta);

    glm::vec4 v = m_v * ct - m_u * st;
    glm::vec4 u = m_v * st + m_u * ct;

    m_u = u;
    m_v = v;

    updateViewMatrix();
}

void CamtransCamera::setClip(float nearPlane, float farPlane) {
    m_near = nearPlane;
    m_far = farPlane;

    updateProjectionMatrix();
}

// @TODO Task 1: Define the helper methods for updating the matrices here...void CamtransCamera::updateProjectionMatrix()
void CamtransCamera::updatePerspectiveMatrix()
{
    float c = -m_near/m_far;

    float data[16]
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1/(c+1), c/(c+1),
        0, 0, -1, 0
    };

    m_perspectiveTransformation = glm::transpose(glm::make_mat4x4(data));
}

void CamtransCamera::updateProjectionMatrix()
{
    updateScaleMatrix();
    updatePerspectiveMatrix();
}

void CamtransCamera::updateScaleMatrix()
{
    float data[16]
    {
        1.0 / (m_far * tan(m_thetaW / 2.0)), 0, 0, 0,
        0, 1.0 / (m_far * tan(m_thetaH / 2.0)), 0, 0,
        0, 0, 1.0 / m_far, 0,
        0, 0, 0, 1
    };

    m_scaleMatrix = glm::transpose(glm::make_mat4x4(data));
}

void CamtransCamera::updateViewMatrix()
{
    updateTranslationMatrix();
    updateRotationMatrix();
}

void CamtransCamera::updateRotationMatrix()
{
    float data[16]
    {
        m_u.x, m_u.y, m_u.z, 0,
                m_v.x, m_v.y, m_v.z, 0,
                m_w.x, m_w.y, m_w.z, 0,
                0, 0, 0, 1
    };

    m_rotationMatrix = glm::transpose(glm::make_mat4x4(data));
}

void CamtransCamera::updateTranslationMatrix()
{
    float data[16]
    {
        1, 0, 0, -m_eye.x,
        0, 1, 0, -m_eye.y,
        0, 0, 1, -m_eye.z,
        0, 0, 0, 1
    };

    m_translationMatrix = glm::transpose(glm::make_mat4x4(data));
}


// @TODO Task 2: Fill in the helper methods you created for updating the matrices...
glm::mat4x4 CamtransCamera::getProjectionMatrix() const {
    return getPerspectiveMatrix() * getScaleMatrix();
}

// Returns the view matrix given the current camera settings.
glm::mat4x4 CamtransCamera::getViewMatrix() const
{
    return m_rotationMatrix * m_translationMatrix;
}

// Returns the matrix that scales down the perspective view volume into the canonical
// perspective view volume, given the current camera settings.
glm::mat4x4 CamtransCamera::getScaleMatrix() const
{
    return m_scaleMatrix;
}

// Returns the matrix the unhinges the perspective view volume, given the current camera
// settings.
glm::mat4x4 CamtransCamera::getPerspectiveMatrix() const
{
    return m_perspectiveTransformation;
}
