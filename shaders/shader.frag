#version 400 core

in vec4 shadowCoord;
in vec2 uv;


const int MAX_LIGHTS = 10;

uniform sampler2DShadow shadowMap[MAX_LIGHTS];
uniform mat4 shadowMat[MAX_LIGHTS];
uniform samplerCube shadowCubeMap[MAX_LIGHTS];
// Light data

out vec4 fragColor;

uniform bool useLighting;
uniform int useShadowMapping;

uniform int useTexture = 0;
uniform vec2 repeatUV;

uniform int lightTypes[MAX_LIGHTS];         // 0 for point, 1 for directional
uniform vec3 lightPositions[MAX_LIGHTS];    // For point lights
uniform vec3 lightDirections[MAX_LIGHTS];   // For directional lights
//uniform vec3 lightAttenuations[MAX_LIGHTS]; // Constant, linear, and quadratic term


uniform vec3 lightColors[MAX_LIGHTS];
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float shininess;
uniform mat4 p;
uniform mat4 v;
uniform mat4 m;

in vec4 position_cameraSpace;
in vec4 normal_cameraSpace;
in vec4 obj_position;

vec3 o_amb;
vec3 o_diff;
vec3 o_spec;


uniform sampler2D tex;
in vec2 texc;

//uniform sampler2D tex;

uniform int numLights;
float bias = 0.005;

/*
vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
vec2( 0.94558609, -0.76890725 ),
vec2( -0.094184101, -0.92938870 ),
vec2( 0.34495938, 0.29387760 ));
*/

vec2 poissonDisk2[16] = vec2[](
            vec2(0.97957f, 0.2811f),
            vec2(0.794353f, 0.837025f),
            vec2(0.117349f, 0.505758f),
            vec2(0.468528f, 0.0111598f),
            vec2(0.569335f, 0.3809f),
            vec2(0.387103f, 0.809421f),
            vec2(0.024733f, 0.927193f),
            vec2(0.0644649f, 0.0364376f),
            vec2(0.857662f, 0.0151163f),
            vec2(0.873436f, 0.553479f),
            vec2(0.302805f, 0.254103f),
            vec2(0.336668f, 0.594892f),
            vec2(0.691506f, 0.148172f),
            vec2(0.624169f, 0.980038f),
            vec2(0.0466257f, 0.262733f),
            vec2(0.646728f, 0.665895f));

vec3 poissonDisk3[] = vec3[] (
        vec3(0.802456f, 0.679957f, 0.145147f),
        vec3(0.248366f, 0.147665f, 0.824477f),
        vec3(0.857692f, 0.687459f, 0.663825f),
        vec3(0.28284f, 0.960429f, 0.143881f),
        vec3(0.842143f, 0.198338f, 0.853133f),
        vec3(0.105744f, 0.858962f, 0.617928f),
        vec3(0.47324f, 0.0397311f, 0.201728f),
        vec3(0.00417795f, 0.432789f, 0.130358f),
        vec3(0.443836f, 0.641571f, 0.976557f),
        vec3(0.967166f, 0.00945927f, 0.0471094f),
        vec3(0.327304f, 0.485439f, 0.372597f),
        vec3(0.0930765f, 0.164078f, 0.483188f),
        vec3(0.481253f, 0.931533f, 0.724106f),
        vec3(0.581023f, 0.0125562f, 0.646484f),
        vec3(0.0114362f, 0.59184f, 0.979364f),
        vec3(0.992244f, 0.220386f, 0.508227f));
int poissonDisk3Samples = 16;


float dirShadow(vec4 vertexToLight, int i)
{
    float visibility = 1.0;

    if (useShadowMapping == 0) {
        return visibility;
    }

    bias = clamp(0.005 * tan(acos(clamp(dot(normal_cameraSpace, -vertexToLight), 0, 1))), 0.0, 0.01);
    vec4 shadowCoord = shadowMat[i] * m * obj_position;
    for (int s = 0; s < 16; s++){
        visibility -= (.8 / 16) *(1.0-(texture(shadowMap[i], vec3(shadowCoord.xy + poissonDisk2[s]/700.0, (shadowCoord.z-bias)/shadowCoord.w))));
    }

    return visibility;
}

float pointShadow(vec3 fragToLight, int i)
{
    float far_plane = 25.0f;
    float currentDepth = length(fragToLight);

    float visibility = 1.0;
    for (int s = 0; s < poissonDisk3Samples; s++){
        float closestDepth = texture(shadowCubeMap[i], fragToLight + poissonDisk3[s] / 700.0).r;
        closestDepth *= far_plane;

        float bias = 0.05;
        if (currentDepth - bias > closestDepth) {
            visibility -= (0.8 / poissonDisk3Samples);
        }
    }
    return visibility;

}


void main() {


    vec3 texColor = texture(tex, texc*repeatUV).rgb;
    texColor = clamp(texColor + vec3(1-useTexture), vec3(0), vec3(1));

    o_amb = vec3(0);
    o_diff = vec3(0);
    o_spec = vec3(0);

    float found = 0.0;
    if (true) {
        o_amb = ambient_color.xyz; // Add ambient component


        float visibility = 1.0;
        for (int i = 0; i < numLights; i++) {
            visibility = 1.0;


            if (lightColors[i] == vec3(0)) {
                continue;
            }

            vec4 vertexToLight = vec4(0);

            // Point Light
            if (lightTypes[i] == 0) {
                vertexToLight = normalize(v * vec4(lightPositions[i], 1) - position_cameraSpace);

                visibility = pointShadow(((m * obj_position).xyz - lightPositions[i]), i);

            } else if (lightTypes[i] == 1) {
                // Dir Light

                vertexToLight = normalize(v * vec4(-lightDirections[i], 0));

                visibility = dirShadow(vertexToLight, i);

            }



            // Add diffuse component
            float diffuseIntensity = max(0.0, dot(vertexToLight, normal_cameraSpace));
            o_diff += visibility * max(vec3(0), lightColors[i] * diffuse_color * diffuseIntensity);

            // Add specular component
            vec4 lightReflection = normalize(-reflect(vertexToLight, normal_cameraSpace));
            vec4 eyeDirection = normalize(vec4(0,0,0,1) - position_cameraSpace);
            float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
            o_spec += visibility * max (vec3(0), lightColors[i] * specular_color * specIntensity);

        }
    } else {
        //color = ambient_color + diffuse_color;
        o_amb = ambient_color;
        o_diff = diffuse_color;
        o_spec = vec3(0);
    }

    vec3 color = clamp(o_amb + o_diff + o_spec, vec3(0), vec3(1));

    fragColor = vec4(color * texColor, 1);

}
