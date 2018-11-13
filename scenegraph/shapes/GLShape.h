#pragma once

#include "GL/glew.h"

#include <memory>
#include <vector>

#include "gl/datatype/VBO.h"
#include "gl/datatype/VBOAttribMarker.h"

namespace CS123 { namespace GL {

class VAO;

}}

using namespace CS123::GL;

class GLShape {
public:

    GLShape(int t1, int t2, float t3);

    ~GLShape();

    /** Draw the initialized geometry. */
    void draw();

protected:
    /** Initialize the VBO with the given vertex data. */
    void setVertexData(GLfloat *data, int size, VBO::GEOMETRY_LAYOUT drawMode, int numVertices);

    /** Enables the specified attribute and calls glVertexAttribPointer with the given arguments. */
    void setAttribute(GLuint index, GLuint numElementsPerVertex, int offset, VBOAttribMarker::DATA_TYPE type,
                      bool normalize);

    /** Build the VAO given the specified vertex data and atrributes */
    void buildVAO();

    int m_t1;
    int m_t2;
    float m_t3;

private:
    GLfloat *m_data;                            /// vector of floats containing the vertex data.
    GLsizeiptr m_size;                          /// size of the data array, in bytes.
    VBO::GEOMETRY_LAYOUT m_drawMode;            /// drawing mode
    int m_numVertices;                          /// number of vertices to be rendered
    std::vector<VBOAttribMarker> m_markers;     /// list of VBOAttribMarkers that describe how the data is laid out.
    std::unique_ptr<CS123::GL::VAO> m_VAO;      /// a wrapper for the vertex array object (VAO)

};
