#version 400 core

in vec4 shadowCoord;
in vec2 uv;


const int MAX_LIGHTS = 10;

uniform sampler2D shadowMap[MAX_LIGHTS];
uniform mat4 shadowMat[MAX_LIGHTS];
uniform samplerCube shadowCubeMap[MAX_LIGHTS];
// Light data

out vec4 fragColor;

uniform bool useLighting;

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

uniform int numLights = 0;
float bias = 0.005;

vec2 poissonDisk[4] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ));

float ShadowCalculation(vec3 fragToLight, int i)
{
    // ise the fragment to light vector to sample from the depth map
    float far_plane = 25.0f;

    float closestDepth = texture(shadowCubeMap[i], fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return closestDepth == 0.0 ? 1.0 : 0.0;
}


void main(){

    /*vec4 shadowCoord = shadowMat * m * obj_position;
    if (texture(shadowMap, shadowCoord.xy).z < shadowCoord.z) {
        //gl_FragColor = vec4(texture(shadowMap, shadowCoord.xy).z, 0, 0, 1.0);
        gl_FragColor = vec4(texture(shadowMap, shadowCoord.xy).z, 0, 0, 1.0);
    } else {
        //gl_FragColor = vec4(texture(shadowMap, shadowCoord.xy).z, 0, 0, 1.0);
        gl_FragColor = vec4(texture(shadowMap, shadowCoord.xy).z, 0, 0, 1.0);
    }

    fragColor = vec4(texture(shadowMap, texc).r, 0, 0, 1.0);*/

    //vec3 texColor = texture(tex, texc*repeatUV).rgb;
    // gl_FragColor = vec4(clamp(texColor + vec3(1-useTexture), vec3(0), vec3(1)), 1);


    vec3 texColor = texture(tex, texc*repeatUV).rgb;
    texColor = clamp(texColor + vec3(1-useTexture), vec3(0), vec3(1));

    o_amb = vec3(0);
    o_diff = vec3(0);
    o_spec = vec3(0);

    float found = 0.0;
    if (useLighting) {
        o_amb = ambient_color.xyz; // Add ambient component


        float visibility = 1.0;
        for (int i = 0; i <= numLights; i++) {
            visibility = 1.0;


            vec4 vertexToLight = vec4(0);

            // Point Light
            if (lightTypes[i] == 0) {
                vertexToLight = normalize(v * vec4(lightPositions[i], 1) - position_cameraSpace);

                /*float shadow = ShadowCalculation(vertexToLight.xyz, i);
                if (shadow == 1.0)
                {
                    visibility = 0.0;
                }*/

            } else if (lightTypes[i] == 1) {
                // Dir Light

                vertexToLight = normalize(v * vec4(-lightDirections[i], 0));


                bias = clamp(0.005 * tan(acos(clamp(dot(normal_cameraSpace, -vertexToLight), 0, 1))), 0.0, 0.01);
                vec4 shadowCoord = shadowMat[i] * m * obj_position;
                /*if (texture(shadowMap[i], shadowCoord.xy).r < shadowCoord.z - bias) {
                    visibility = 0.5;
                } else {
                     visibility = 1.0;
                }*/
                for (int s = 0; s < 4; s++){
                    if (texture(shadowMap[i], shadowCoord.xy + poissonDisk[s] / 700.0).r < shadowCoord.z - bias) {
                        visibility -= 0.2;
                    }
                }

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
