//
// Created by gmarks on 9/29/18.
//
#include "CircleHelper.h"
#include "SquareGrid.h"
#include <math.h>
#include <assert.h>
#include <vector>

CircleHelper::CircleHelper(GLfloat radius, int npoints, int tf) {
    m_radius = radius;
    m_npoints = npoints;
    m_tf = tf;
    m_trans = glm::mat4();
}

CircleHelper::CircleHelper(GLfloat radius, int npoints, int tf, glm::mat4 trans) {
    m_radius = radius;
    m_npoints = npoints;
    m_tf = tf;
    m_trans = trans;
}

SquareGrid CircleHelper::asSquareGrid() {
    auto transformToVerticalCircle = [&](glm::vec4 v){
        // x->place on circle, y->radius
        glm::vec4 new_v;
        new_v.w = v.w;
        new_v.y = 0;
        new_v.x = m_radius * (v.y+0.5) * sin(v.x * 2 * M_PI);
        new_v.z = m_radius * (v.y+0.5) * cos(v.x * 2 * M_PI);
        return m_trans * new_v;
    };
    auto norm = [&](glm::vec4 _) { // straight up
        return m_trans * glm::vec4(0, 1, 0, 0);
    };

    SquareGrid grid(m_npoints, m_tf, glm::mat4(), glm::vec3(0, 0, 0), transformToVerticalCircle, norm);
    return grid;
}

void CircleHelper::buildShape() {
    SquareGrid grid = asSquareGrid();
    grid.buildShape();
    m_vertexData = grid.getVertexData();
}
