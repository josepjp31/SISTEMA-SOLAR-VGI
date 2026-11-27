//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (EE-UAB)
//******** Entorn bàsic VS2026 amb interficie MFC/GLFW i Status Bar
//******** Enric Marti (Setembre 2026)
// normal_mapping.vert: Vertex Program en GLSL en versió OpenGL 3.3 o 4.60 per a implementar:
//     a) Iluminació de Gouraud + normal Mapping
//     b) Fonts de llum puntuals o direccionals
//     c) Fonts de llum restringides
//     d) Atenuació de fonts de llum
//     e) Modus d'assignació de textura MODULATE o DECAL
//     f) Calcul component a de transparència segons color sigui provinent de material o de VAO
//     g) Algorisme incremental  iluminació Gouraud encapsulat en gouraudModel()

//#version 330 core
#version 460 core

#define MaxLights 8

struct Light
{
    bool sw_light;
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 attenuation;
    bool restricted;
    vec3 spotDirection;
    float spotCosCutoff;
    float spotExponent;
};

struct Material
{
    vec4 emission;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec2 in_TexCoord;
layout(location = 4) in vec3 in_Tangent;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform bool useNormalMatrixPV = false;
uniform mat3 uNormalMatrixPV;

uniform sampler2D texture0;
uniform sampler2D normalMap;

uniform bool textur;
uniform bool flag_invert_y;
uniform bool modulate;
uniform bool fixedLight;

uniform bool sw_material;
uniform bvec4 sw_intensity;

uniform vec4 LightModelAmbient;
uniform Light LightSource[MaxLights];
uniform Material material;

out vec3 vertexPV;
out vec3 vertexNormalPV;
out vec2 vertexTexCoord;
out vec4 vertexColor;

out mat3 TBN_PV;

void main()
{
    mat4 modelView = viewMatrix * modelMatrix;

    // Posición en coordenadas de vista
    vertexPV = vec3(modelView * vec4(in_Vertex, 1.0));

    // Reutilitza la matriu normal precalculada a CPU quan s'indica des de l'aplicacio
    mat3 normalMatrixPV = useNormalMatrixPV
        ? uNormalMatrixPV
        : transpose(inverse(mat3(modelView)));

    // Normal
    vec3 N = normalize(normalMatrixPV * in_Normal);

    // Tangente
    vec3 T = normalize(mat3(modelView) * in_Tangent);

    // Ortogonalizar tangente respecto a normal
    T = normalize(T - dot(T, N) * N);

    // Bitangente
    vec3 B = normalize(cross(N, T));

    // Matriz TBN
    TBN_PV = mat3(T, B, N);

    // Coordenadas de textura
    if (flag_invert_y)
        vertexTexCoord = vec2(in_TexCoord.x, 1.0 - in_TexCoord.y);
    else
        vertexTexCoord = in_TexCoord;

    vertexNormalPV = N;
    vertexColor = in_Color;

    gl_Position =
        projectionMatrix * modelView * vec4(in_Vertex, 1.0);
}
