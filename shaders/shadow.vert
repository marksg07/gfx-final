#version 330 core

layout(location = 0) in vec3 position; // Position of the vertex

// Transformation matrices
uniform mat4 m;
uniform mat4 shadowMat;

void main() {
    gl_Position = shadowMat * m * vec4(position, 1.0);
}
