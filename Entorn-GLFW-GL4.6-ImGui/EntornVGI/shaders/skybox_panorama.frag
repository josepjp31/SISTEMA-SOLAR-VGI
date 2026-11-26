#version 460 core

in vec3 direction;

uniform sampler2D panorama;

out vec4 FragColor;

const float PI = 3.14159265359;

void main()
{
    vec3 dir = normalize(direction);

    // Convertir dirección 3D a coordenadas equirectangulares
    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;

    float v = asin(clamp(dir.y, -1.0, 1.0)) / PI + 0.5;

    vec2 uv = vec2(u, v);

    FragColor = texture(panorama, uv);
}