//
// Created by gmarks on 9/30/18.
//

#include "Cube.h"
#include "SquareGrid.h"
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include "glm/gtx/constants.hpp"

void Cube::buildShape() {
    int tf = m_tf1;
    m_vertexData.resize(6 * (6 * (2 * tf + 4 * tf * tf)));
    glm::mat4 m1 = glm::mat4(), m2 = glm::mat4();
    int offset = 0;
    GLfloat hpi = glm::half_pi<GLfloat>();

    // front face
    SquareGrid *grid;
    SquareGrid gr(tf, tf, glm::translate(glm::vec3(0, 0, 0.5)), glm::vec3(0, 0, 1));
    gr.buildShape();
    std::vector<GLfloat> toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
    offset += toAdd.size();

    // back face
    gr = SquareGrid(tf, tf, glm::translate(glm::vec3(0, 0, -0.5)) * glm::rotate(hpi * 2, glm::vec3(1.f, 0, 0)), glm::vec3(0, 0, -1));
    gr.buildShape();
    toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
    offset += toAdd.size();

    // top face
    gr = SquareGrid(tf, tf, glm::translate(glm::vec3(0, 0.5, 0)) * glm::rotate(hpi * 3, glm::vec3(1.f, 0, 0)), glm::vec3(0, 1, 0));
    gr.buildShape();
    toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
    offset += toAdd.size();

    // bottom face
    gr = SquareGrid(tf, tf, glm::translate(glm::vec3(0, -0.5, 0)) * glm::rotate(hpi, glm::vec3(1.f, 0, 0)), glm::vec3(0, -1, 0));
    gr.buildShape();
    toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
    offset += toAdd.size();

    // left face
    gr = SquareGrid(tf, tf, glm::translate(glm::vec3(-0.5, 0, 0)) * glm::rotate(hpi * 3, glm::vec3(0, 1.f, 0)), glm::vec3(-1, 0, 0));
    gr.buildShape();
    toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
    offset += toAdd.size();

    // right face
    gr = SquareGrid(tf, tf, glm::translate(glm::vec3(0.5, 0, 0)) * glm::rotate(hpi, glm::vec3(0, 1.f, 0)), glm::vec3(1, 0, 0));
    gr.buildShape();
    toAdd = gr.getVertexData();
    std::copy(&toAdd[0], &toAdd[toAdd.size()], &m_vertexData[offset]);
}
