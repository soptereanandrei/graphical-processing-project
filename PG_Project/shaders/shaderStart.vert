#version 410

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textcoord;

out vec4 positionEye;
out vec3 normalEye;
out vec2 texCoord;
out vec4 positionLight;//position in light space

//Matricies
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

void main() {
    //calculate vertex position in eye coordinates
    positionEye = view * model * vec4(vertexPosition, 1.0);

    //compute eye coordinates for normals
    normalEye = normalize(normalMatrix * vertexNormal);

    texCoord = textcoord;

    positionLight = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);

    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}
