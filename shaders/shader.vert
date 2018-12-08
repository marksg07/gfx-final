#version 330 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex
layout(location = 5) in vec2 texCoord; // UV texture coordinates
layout(location = 10) in float arrowOffset; // Sideways offset for billboarded normal arrows

//out vec3 color; // Computed color for this vertex
out vec2 texc;

// Transformation matrices
uniform mat4 p;
uniform mat4 v;
uniform mat4 m;
uniform sampler2D shadowMap;
uniform mat4 shadowMat;

//out vec4 shadowCoord;


// Material data


out vec4 position_cameraSpace;
out vec4 normal_cameraSpace;
out vec4 obj_position;
out float bias;

void main() {
    //texc = texCoord * repeatUV;
    texc = texCoord;

    bias = 0.005;

    obj_position = vec4(position, 1.0);
    position_cameraSpace = v * m * vec4(position, 1.0);
    normal_cameraSpace = vec4(normalize(mat3(transpose(inverse(v * m))) * normal), 0);

    vec4 position_worldSpace = m * vec4(position, 1.0);
    vec4 normal_worldSpace = vec4(normalize(mat3(transpose(inverse(m))) * normal), 0);


    gl_Position = p * position_cameraSpace;
    //gl_Position = obj_position;


    //color = clamp(color, 0.0, 1.0);
}
