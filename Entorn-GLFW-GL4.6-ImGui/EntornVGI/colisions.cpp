
#include "colisions.h"
#include <limits>

BoundingSphere creaBoundingSphere(const glm::mat4& ModelMatrix,float radiLocal)
{
    BoundingSphere esfera;

    // Calcular el centre de l'objecte en coordenades del mon
    esfera.centre = glm::vec3(ModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    // Calcular l'escala aplicada als tres eixos
    glm::vec3 escala;

    escala.x = glm::length(glm::vec3(ModelMatrix[0]));
    escala.y = glm::length(glm::vec3(ModelMatrix[1]));
    escala.z = glm::length(glm::vec3(ModelMatrix[2]));

    // Agafar l'escala mes gran per garantir que l'esfera envolta l'objecte
    esfera.radi = radiLocal * glm::max(escala.x, glm::max(escala.y, escala.z));

    return esfera;


}

BoundingBox creaBoundingBox(const glm::mat4& ModelMatrix,const glm::vec3& minLocal,const glm::vec3& maxLocal)
{
    BoundingBox caixa;

    caixa.min = glm::vec3(std::numeric_limits<float>::max()); //(+inf, +inf, +inf)
	caixa.max = glm::vec3(std::numeric_limits<float>::lowest()); //(-inf, -inf, -inf)

    // Recorrem les 8 combinacions de coordenades
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {

                // Creem un vèrtex amb coordenades locals
                glm::vec3 vertex(
                    x ? maxLocal.x : minLocal.x,
                    y ? maxLocal.y : minLocal.y,
                    z ? maxLocal.z : minLocal.z
                );

                // Transformem el vèrtex a coordenades del món
                glm::vec3 vertexWorld =
                    glm::vec3(ModelMatrix * glm::vec4(vertex, 1.0f));

                // Actualitzem els extrems de la caixa
                caixa.min = glm::min(caixa.min, vertexWorld);
                caixa.max = glm::max(caixa.max, vertexWorld);
            }
        }
    }

    return caixa;
}

bool colisioAABBEsfera(const BoundingBox& caixa,const BoundingSphere& esfera)
{
    // Trobar el punt de la caixa més proper al centre de l'esfera
    glm::vec3 puntProper = glm::clamp(esfera.centre,caixa.min,caixa.max);

    // Distància entre el punt proper i el centre de l'esfera
    glm::vec3 diferencia = esfera.centre - puntProper;

    float distanciaQuadrada = glm::dot(diferencia, diferencia);

    // Comprovar si la distància és menor o igual que el radi
    return distanciaQuadrada <= esfera.radi * esfera.radi;
}
