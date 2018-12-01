//
// Created by gmarks on 9/30/18.
//

#include "Cylinder.h"
#include "SquareGrid.h"
#include "CircleHelper.h"
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include <math.h>
#include <iostream>
#include <iterator>
#include <algorithm>

glm::vec4 bpToNorm(glm::vec4 bp) {
   bp.y = 0;
   bp.w = 0;
   return bp;
}

void Cylinder::buildShape() {
    SquareGrid topCirc = CircleHelper(0.5, m_tf2, m_tf1, glm::translate(glm::vec3(0, 0.5, 0))).asSquareGrid();
    // if even # points, x-axis rotation works w/ phase shift. if odd # points, z-axis rotation works w/ phase shift.
    SquareGrid botCirc = CircleHelper(0.5, m_tf2, m_tf1, glm::translate(glm::vec3(0, -0.5, 0)) * glm::rotate((GLfloat)M_PI, glm::vec3((m_tf2 + 1) % 2, 0, m_tf2 % 2))).asSquareGrid();

    auto pointToBarrel = [&](glm::vec4 point) {
        glm::vec4 out;
        out.y = point.y;
        // So the circles and the barrel have to be oriented differently
        // so that they are both CCW, so one has (0,0) -> (1,0)
        // and one has (0,0) -> (0,1). That means that they don't fit
        // exactly on top of one another, so we have to phase-shift
        // the barrel depending on if the number of points is even
        // or odd.
        out.x = 0.5*cos((point.x + 0.25 * (1./m_tf2) * (m_tf2 % 4)) * 2 * M_PI );
        out.z = 0.5*sin((point.x + 0.25 * (1./m_tf2) * (m_tf2 % 4)) * 2 * M_PI );
        out.w = point.w;
        return out;
    };
    SquareGrid barrel(m_tf2, m_tf1, glm::mat4(), glm::vec3(0, 0, 1), pointToBarrel, bpToNorm);

    topCirc.buildShape();
    botCirc.buildShape();
    barrel.buildShape();
    std::vector<GLfloat> topVD = topCirc.getVertexData(),
            barrelVD = barrel.getVertexData(),
            botVD = botCirc.getVertexData();

    // trim off first point of first thing because it's a duplicate of second
    m_vertexData.resize(topVD.size() + barrelVD.size() + botVD.size() - 6);
    std::copy(topVD.begin() + 6, topVD.end(), m_vertexData.begin());
    int offset = topVD.size() - 6;
    std::copy(barrelVD.begin(), barrelVD.end(), m_vertexData.begin() + offset);
    offset += barrelVD.size();
    std::copy(botVD.begin(), botVD.end(), m_vertexData.begin() + offset);
}
