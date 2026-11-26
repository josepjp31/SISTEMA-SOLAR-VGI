#pragma once

#include "framework.h"

// Bounding Sphere: bounding box pero per esferes.
struct BoundingSphere
{
    glm::vec3 centre;
    float radi;
};

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

bool colisioAABBEsfera(const BoundingBox& caixa,const BoundingSphere& esfera);

// Crea una Bounding Sphere a partir de la matriu de l'objecte
BoundingSphere creaBoundingSphere(const glm::mat4& ModelMatrix,float radiLocal = 1.0f);
BoundingBox creaBoundingBox(const glm::mat4& ModelMatrix, const glm::vec3& minLocal, const glm::vec3& maxLocal);