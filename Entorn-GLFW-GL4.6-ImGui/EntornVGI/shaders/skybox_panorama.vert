#version 460 core

layout(location = 0) in vec3 in_Vertex;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 direction;

void main()
{
    // Dirección del vértice del cubo
    direction = mat3(modelMatrix) * in_Vertex;

    vec4 pos = projectionMatrix *
               viewMatrix *
               modelMatrix *
               vec4(in_Vertex, 1.0);

    // El skybox siempre queda en el fondo
    gl_Position = pos.xyww;
}