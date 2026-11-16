// CatalegAstronomic.h — Arquitectura: Catàleg Astronòmic
// Defineix les dades físiques i visuals oficials (NASA/JPL) compartides entre EscenariRealista i InterficieUI.

#pragma once

#include <glm/glm.hpp>
#include "CelestialBodyData.h"

// Entrada unificada del catàleg astronòmic (valors oficials NASA/JPL de docs/planetes/planetes.json)
struct DadesCosAstronomic
{
    CelestialBodyId spiceId;             // Identificador NAIF/SPICE (CB_SUN, CB_EARTH, CB_MOON, ...)
    int parentIndex;                     // Índex del cos pare al catàleg [-1: Sol/centre, 0: Sol, 3: Terra per a la Lluna]
    const char* nom;                     // Nom en català ("Sol", "Mercuri", ..., "Lluna")
    const char* tipus;                   // Classificació ("Estrella", "Planeta", "Planeta nan", "Satèl·lit natural")
    double radiMitjaKm;                  // [km] Radi volumètric mitjà (> 0.0; usat per a l'esfera 3D i la fitxa HUD)
    double radiEquatorialKm;             // [km] Radi equatorial (> 0.0; usat per a referència física i anells)
    double distMitjanaRefKm;             // [km] Distància mitjana de referència al cos pare (>= 0.0)
    double distMitjanaRefUA;             // [UA] Distància mitjana de referència al cos pare (>= 0.0)
    double gravetatMS2;                  // [m/s^2] Gravetat equatorial superficial o de referència (> 0.0)
    double periodeRotacioHores;          // [hores] Període de rotació sidèria (signe negatiu = rotació retrògrada)
    double periodeOrbitalDies;           // [dies] Període orbital al voltant del cos pare (0.0 per al Sol, > 0.0 per a la resta)
    double oblicuitatGraus;              // [graus] Inclinació axial / obliqüitat [0.0..180.0]
    double inclinacioOrbitalGraus;       // [graus] Inclinació orbital respecte al pla de referència [0.0..180.0]
    const char* rutaTextura;             // Ruta relativa a la textura albedo (nullptr si no n'hi ha)
    glm::vec3 colorCaracteristic;        // [RGB normalitzat, 0.0..1.0] Color per a òrbites, marcadors i fallback sense textura
};

// Catàleg astronòmic únic compartit entre la simulació realista i la interfície d'usuari
class CatalegAstronomic
{
public:
    static constexpr int NUM_COSSOS = 11;              // Nombre total de cossos: Sol (0), 8 planetes (1..8), Plutó (9), Lluna (10)
    static constexpr double KM_PER_AU = 149597870.7;   // [km/UA] Factor oficial IAU d'Unitat Astronòmica

    // Precondició: cap (índexs fora de [0, NUM_COSSOS - 1] retornen l'entrada del Sol per seguretat).
    // Paràmetres: index [0..NUM_COSSOS - 1].
    // Retorn: referència constant a les dades astronòmiques del cos sol·licitat.
    static const DadesCosAstronomic& GetCos(int index);

    // Paràmetres: spiceId — identificador NAIF/SPICE a cercar.
    // Retorn: índex dins [0, NUM_COSSOS - 1] si existeix al catàleg, o -1 en cas contrari.
    static int FindIndexBySpiceId(CelestialBodyId spiceId);

    // Retorn: punter constant a l'array estàtic de NUM_COSSOS elements del catàleg.
    static const DadesCosAstronomic* GetAll();
};
