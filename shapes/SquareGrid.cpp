//
// Created by gmarks on 9/30/18.
//

//
// Created by gmarks on 9/30/18.
//

#include "SquareGrid.h"
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include "glm/gtx/constants.hpp"

SquareGrid::SquareGrid(int nx, int ny, glm::mat4 trans, glm::vec3 normal) {
    m_col = nx;
    m_row = ny;
    m_trans = trans;
    m_N = normal;
    m_grid_fn = NULL;
    m_norm_fn = NULL;
    m_grid = makeGrid();
}

SquareGrid::SquareGrid(int nx, int ny, glm::mat4 trans, glm::vec3 normal, std::function<glm::vec4(glm::vec4)> grid_f, std::function<glm::vec4(glm::vec4)> norm_f) {
    m_col = nx;
    m_row = ny;
    m_trans = trans;
    m_N = normal;
    m_grid_fn = grid_f;
    m_norm_fn = norm_f;
    m_grid = makeGrid();
}

void SquareGrid::concatR(SquareGrid other) {
    assert(m_row == other.m_row);
    for(int y = m_row; y >= 0; y--) {
        m_grid.insert(m_grid.begin() + (m_col+1) * (y + 1), other.m_grid.begin() + (m_col+1) * y, other.m_grid.begin() + (m_col+1) * (y + 1));
    }
    m_col += other.m_col;
}

void SquareGrid::concatD(SquareGrid other) {
    assert(m_col == other.m_col);
    m_grid.insert(m_grid.end(), other.m_grid.begin(), other.m_grid.end());
    m_row += other.m_row;
}

void SquareGrid::buildShape() {
    m_vertexData.resize(6 * (2 * m_row + 4 * m_row * m_col));
    glm::vec4 dup;
    int offset = 0;
    for(int y = 0; y < m_row; y++) {
        dup = std::get<0>(m_grid[y * (m_col + 1)]);

        m_vertexData[offset] = dup.x;
        m_vertexData[offset + 1] = dup.y;
        m_vertexData[offset + 2] = dup.z;
        offset += 6;

        for(int x = 0; x < m_col; x++, offset += 24) {
            std::tuple<glm::vec4, glm::vec4> tl = m_grid[y * (m_col + 1) + x];
            std::tuple<glm::vec4, glm::vec4> tr = m_grid[y * (m_col + 1) + x + 1];
            std::tuple<glm::vec4, glm::vec4> bl = m_grid[(y + 1) * (m_col + 1) + x];
            std::tuple<glm::vec4, glm::vec4> br = m_grid[(y + 1) * (m_col + 1) + x + 1];
#define doThing(idx, off) 0;\
                m_vertexData[off] = std::get<idx>(tl).x; \
                m_vertexData[off + 1] = std::get<idx>(tl).y; \
                m_vertexData[off + 2] = std::get<idx>(tl).z; \
\
                m_vertexData[off + 6] = std::get<idx>(bl).x;\
                m_vertexData[off + 7] = std::get<idx>(bl).y;\
                m_vertexData[off + 8] = std::get<idx>(bl).z;\
\
                m_vertexData[off + 12] = std::get<idx>(tr).x;\
                m_vertexData[off + 13] = std::get<idx>(tr).y;\
                m_vertexData[off + 14] = std::get<idx>(tr).z;\
\
                m_vertexData[off + 18] = std::get<idx>(br).x;\
                m_vertexData[off + 19] = std::get<idx>(br).y;\
                m_vertexData[off + 20] = std::get<idx>(br).z;
            doThing(0, offset);
            doThing(1, offset + 3);
        }
        m_vertexData[offset] = m_vertexData[offset - 6];
        m_vertexData[offset + 1] = m_vertexData[offset - 5];
        m_vertexData[offset + 2] = m_vertexData[offset - 4];
        offset += 6;
    }
}

std::vector<std::tuple<glm::vec4, glm::vec4>> SquareGrid::makeGrid() {
    std::vector<std::tuple<glm::vec4, glm::vec4>> grid;
    grid.resize((m_row + 1) * (m_col + 1));
    for(int y = 0; y < m_row + 1; y++) {
        for(int x = 0; x < m_col + 1; x++) {
            std::tuple<glm::vec4, glm::vec4> *el = &grid[y * (m_col + 1) + x];
            std::get<0>(*el) = m_trans * glm::vec4(x / GLfloat(m_col) - 0.5, y / GLfloat(m_row) - 0.5, 0, 1.f);
            if(m_grid_fn != NULL)
                std::get<0>(*el) = m_grid_fn(std::get<0>(*el));
            if(m_norm_fn == NULL)
                std::get<1>(*el) = glm::vec4(m_N, 0.);
            else
                std::get<1>(*el) = m_norm_fn(std::get<0>(*el));
        }
    }
    return grid;
}
