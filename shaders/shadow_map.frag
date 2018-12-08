#version 330 core

in vec2 uv;
uniform sampler2D depth_map;

void main()
{
    float depth_value = texture(depth_map, uv).r;
    gl_FragColor = vec4(vec3(depth_value), 1.0); // orthographic
}
