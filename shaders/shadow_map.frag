#version 330 core

in vec2 texc;
uniform sampler2D shadowMap;

out vec4 fragColor;

void main()
{
    float depth_value = texture(shadowMap, texc).r;
    fragColor = vec4(vec3(depth_value), 1.0);
}
