#pragma once

#include <glm/glm.hpp>
#include "CelestialBodyData.h"

// Entrada unificada del cataleg astronomic (valors oficials NASA/JPL de docs/planetes/planetes.json)
struct DadesCosAstronomic
{
    CelestialBodyId spiceId;             // Identificador NAIF/SPICE (CB_SUN, CB_EARTH, CB_MOON, ...)
    int parentIndex;                     // Index del cos pare (-1: Sol/centre, 0: Sol, 3: Terra per a la Lluna)
    const char* nom;                     // Nom en catala ("Sol", "Mercuri", ..., "Lluna")
    const char* tipus;                   // Classificacio ("Estrella", "Planeta", "Planeta nan", "Satèl·lit natural")
    double radiMitjaKm;                  // Radi volumetric mitja [km] (usat per a l'esfera 3D i la fitxa HUD)
    double radiEquatorialKm;             // Radi equatorial [km] (usat per a referencia fisica i anells)
    double distMitjanaRefKm;             // Distancia mitjana de referencia al cos pare [km]
    double distMitjanaRefUA;             // Distancia mitjana de referencia al cos pare [UA]
    double gravetatMS2;                  // Gravetat equatorial superficial o de referencia [m/s^2]
    double periodeRotacioHores;          // Periode de rotacio siderea [hores] (signe negatiu = rotacio retrograda)
    double periodeOrbitalDies;           // Periode orbital al voltant del cos pare [dies]
    double oblicuitatGraus;              // Inclinacio axial / oblicuitat [graus]
    double inclinacioOrbitalGraus;       // Inclinacio orbital respecte al pla de referencia [graus]
    const char* rutaTextura;             // Ruta relativa a la textura albedo (nullptr si no n'hi ha)
    glm::vec3 colorCaracteristic;        // Color RGB caracteristic per a orbites, marcadors i fallback sense textura
};

// Cataleg astronomic unic compartit entre la simulacio realista i la interficie d'usuari
class CatalegAstronomic
{
public:
    static constexpr int NUM_COSSOS = 11; // Sol + 8 planetes + Pluto + Lluna
    static constexpr double KM_PER_AU = 149597870.7;

    static const DadesCosAstronomic& GetCos(int index);
    static int FindIndexBySpiceId(CelestialBodyId spiceId);
    static const DadesCosAstronomic* GetAll();
};
