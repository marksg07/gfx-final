#version 330 core

in vec2 uv;
uniform sampler2D shadowMap;

void main()
{
    float depth_value = texture(shadowMap, uv).r;
    fragColor = vec4(vec3(depth_value), 1.0); // orthographic
}
