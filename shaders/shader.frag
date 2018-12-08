#version 400 core

in vec4 shadowCoord;
in vec2 uv;


const int MAX_LIGHTS = 10;

uniform sampler2D shadowMap[MAX_LIGHTS];
uniform mat4 shadowMat[MAX_LIGHTS];
// Light data

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
out vec4 fragColor;

//uniform sampler2D tex;

uniform int numLights = 0;
float bias = 0.005;

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



            vec4 vertexToLight = vec4(0);

            // Point Light
            if (lightTypes[i] == 0) {
                vertexToLight = normalize(v * vec4(lightPositions[i], 1) - position_cameraSpace);

            } else if (lightTypes[i] == 1) {
                // Dir Light

                vertexToLight = normalize(v * vec4(-lightDirections[i], 0));


                bias = clamp(0.005 * tan(acos(clamp(dot(normal_cameraSpace, -vertexToLight), 0, 1))), 0, 0.01);
                vec4 shadowCoord = shadowMat[i] * m * obj_position;
                if (texture(shadowMap[i], shadowCoord.xy).r < shadowCoord.z - bias) {
                    visibility = 0.5;
                } else {
                     visibility = 1.0;
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
