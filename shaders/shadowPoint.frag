#version 330 core

in vec4 fragPos;

uniform vec3 lightPos;
uniform float farPlane = 25.0f;

void main()
{
    float lightDistance = length(fragPos.xyz - lightPos);

    lightDistance = lightDistance / farPlane;

    gl_FragDepth = lightDistance;
}
