#version 410

in vec4 positionEye;
in vec3 normalEye;
in vec2 texCoord;
in vec4 positionLight;

out vec4 fragmentColour;

//Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

//Lights
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 baseColor;
uniform vec3 viewPosEye;
uniform vec3 lightPosEye; //point light position in eye coordinates
uniform vec3 pointLightColor;

//Textures
uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.05f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.20f;

float computeFog()
{
    float fogDensity = 0.01f;
    float fragmentDistance = length(positionEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

vec4 BlinnPhongLightingPoint()
{
    //compute light direction
    vec3 lightDirN = normalize(lightPosEye - positionEye.xyz);

    //compute distance to light
    float dist = length(lightPosEye - positionEye.xyz);

    //compute attenuation
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    //compute ambient light
    ambient = att * ambientStrength * pointLightColor * texture(ambientTexture, texCoord).rgb;

    //compute diffuse
    diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor * texture(diffuseTexture, texCoord).rgb;

    //compute specular
    //compute view direction in eye coordinates
    vec3 viewDirN = normalize(viewPosEye - positionEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular = specCoeff * pointLightColor * texture(specularTexture, texCoord).rgb;

    return vec4(min(ambient + diffuse + specular, 1.0), 1.0f);
}

float computeShadow()
{
    //perform pespective divide
    vec3 projCoords = positionLight.xyz / positionLight.w;

    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;

    float bias = 0.005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

vec4 BlinnPhongLightingDirectional()
{
    //compute ambient light
    ambient = ambientStrength * lightColor * texture(ambientTexture, texCoord).rgb;

    //normalize the light's direction in eye coordinates
    vec3 lightDirN = normalize(lightDir);

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * texture(diffuseTexture, texCoord).rgb;

    //compute view direction in eye coordinates
    vec3 viewDirN = normalize(viewPosEye - positionEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    //compute specular light
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular = specCoeff * lightColor * texture(specularTexture, texCoord).rgb;

    float shadow = computeShadow();
    //if (shadow == 1.0)
        //return vec4(1.0, 0.0, 0.0, 1.0);

    return vec4(min(ambient + (1 - shadow) * (diffuse + specular), 1.0), 1.0);

    //debug
    //vec4 debugCol = vec4(0.0, 0.0, 0.0, 1.0);
    //if (lightColor.x > 0.9)
        //debugCol += vec4(1.0, 0.0, 0.0, 1.0);
    //if (lightDir.z < -0.9)
        //debugCol += vec4(0.0, 1.0, 0.0, 1.0);
    //if (diffuse.b > 0)
        //debugCol += vec4(1.0, 0.0, 0.0, 1.0);
    //if (texture(shadowMap, vec2(0.0, 0.0)).r != 0.0)
        //debugCol += vec4(1.0, 0.0, 0.0, 0.0);
    //return debugCol;
}

void main() {
    //fragmentColour = BlinnPhongLightingDirectional();
    //fragmentColour += BlinnPhongLightingPoint();
    //fragmentColour = min(fragmentColour, 1.0);

    vec4 lightingCol = min(BlinnPhongLightingDirectional() + BlinnPhongLightingPoint(), 1.0);
    
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    fragmentColour = mix(fogColor, lightingCol, fogFactor);
}