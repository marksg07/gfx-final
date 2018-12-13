#version 330 core
layout (location = 0) in vec3 position;

out vec3 texc;

uniform mat4 p;
uniform mat4 v;

void main()
{
    texc = position;
    vec4 pos = p * v * vec4(position, 1.0);
    gl_Position = pos.xyww;
}
