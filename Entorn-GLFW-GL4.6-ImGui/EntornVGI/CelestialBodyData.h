// CelestialBodyData.h — Arquitectura: Bridge NASA CSPICE
// Encapsula la càrrega de kernels SPICE, el càlcul d'efemèrides en J2000, les matrius de rotació IAU i la conversió UTC -> TDB.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

// Identificadors NAIF/SPICE dels cossos i baricentres del Sistema Solar
enum CelestialBodyId {
    CB_SOLAR_SYSTEM_BARYCENTER = 0,
    CB_SUN = 10,
    // --- INNER PLANETS ---
    CB_MERCURY = 199,
    CB_VENUS = 299,
    CB_EARTH = 399,
    CB_MARS = 4,            //Look out for non-hundred id, these mean they are BARYCENTER id.
    // --- OUTER PLANETS---
    CB_JUPITER = 5,
    CB_SATURN = 6,
    CB_URANUS = 7,
    CB_NEPTUNE = 8,
    // --- DWARF PLANETS ---
    CB_CERES = 2000001,
    CB_PLUTO = 999,
    CB_PLUTO_BARYCENTER = 9,
    // --- SYSTEM MOONS ---
    CB_MOON = 301,       // Earth
    //CB_PHOBOS,     // Mars
    //CB_DEIMOS,     // Mars
    //CB_IO,         // Jupiter
    //CB_EUROPA,
    //CB_GANYMEDE,
    //CB_CALLISTO,
    //CB_TITAN,      // Saturn
    //CB_ENCELADUS,
    //CB_TRITON,     // Neptune
    CB_CHARON = 901,     // Pluto
    //CB_NIX = 902,
    //CB_HYDRA = 903,
    //CB_KERBEROS = 904,
    //CB_STYX = 905,
    //OTHER
    //CB_HALLEY = 1000036,
    CB_VOYAGER1 = -31,
    //CB_VOYAGER2 = -32
    CB_NONE = -99999
};

// Metadades NAIF/SPICE d'un cos celeste
struct CB {
    std::string m_name;                      // Nom oficial NAIF del cos (en anglès)
    std::string m_id;                        // Identificador NAIF en format text per a crides CSPICE (p. ex. "399")
    bool m_hasRotFrame = false;              // Indica si el cos disposa de marc de rotació propi al kernel PCK
    std::string m_rotationFrame;             // Nom del marc de referència rotacional CSPICE (p. ex. "IAU_EARTH")
    CelestialBodyId m_orbitedObject;         // Cos pare al voltant del qual orbita (CB_NONE si és origen/arrel)
};

const double KM_TO_AU = 1.0 / 149597870.7;   // [UA/km] Factor de conversió de quilòmetres a Unitats Astronòmiques
extern CelestialBodyId globalSystemOrigin;   // Origen per defecte del sistema de referència (CB_SUN)
extern std::unordered_map<CelestialBodyId, CB> cbList; // Taula de correspondència CelestialBodyId -> metadades SPICE

// Vector d'estat cartesià 3D (posició i velocitat) en el marc J2000.
// Unitats: [UA] i [UA/s] en funcions públiques (getCBstate, getOrbitStates); [km] i [km/s] en consultes internes km.
struct CState3D {
	double x, y, z;       // [UA] o [km] Coordenades cartesianes en J2000
	double vx, vy, vz;    // [UA/s] o [km/s] Components de velocitat en J2000

	// Inicialitza el vector d'estat a zero.
	CState3D() : x(0), y(0), z(0), vx(0), vy(0), vz(0) {}
	// Inicialitza des d'un array de 6 doubles [x, y, z, vx, vy, vz] retornat per spkezr_c/conics_c.
    CState3D(double* state) : x(state[0]), y(state[1]), z(state[2]), vx(state[3]), vy(state[4]), vz(state[5]) {}
	// Inicialitza explícitament totes 6 components.
    CState3D(double x, double y, double z, double vx, double vy, double vz) : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz) {}
	// Constructor de còpia.
    CState3D(const CState3D& state) : x(state.x), y(state.y), z(state.z), vx(state.vx), vy(state.vy), vz(state.vz) {}
};

// Escala totes les components (posició i velocitat) de `state` pel factor `val` i retorna una còpia escalada.
CState3D mulState(CState3D& state, double val);

// Precondició: kernels CSPICE carregats (loadCspiceKernels).
// Paràmetres: body (cos objectiu), origin (cos d'origen), et [s TDB des de J2000].
// Retorn: estat cartesià en [UA] i [UA/s] dins del marc inercial J2000 (vector nul si falla la consulta).
CState3D getCBstate(CelestialBodyId body, CelestialBodyId origin, double et);

// Precondició: kernels CSPICE carregats.
// Paràmetres: body, origin, et [s TDB des de J2000].
// Retorn: matriu de translació 4x4 amb la posició de `body` respecte a `origin` en [UA].
glm::mat4 getCBposMat(CelestialBodyId body, CelestialBodyId origin, double et);

// Precondició: kernels CSPICE (PCK) carregats.
// Paràmetres: body (cos amb marc IAU_*), et [s TDB des de J2000].
// Retorn: matriu de rotació 4x4 (column-major GLM) del marc propi IAU_* cap a J2000 (identitat si no té marc o falla).
glm::mat4 getCBrotMat(CelestialBodyId body, double et);

// Precondició: kernels CSPICE carregats (efemèrides + gm_de440.tpc).
// Paràmetres: body (cos amb m_orbitedObject != CB_NONE), et [s TDB des de J2000], nStates (> 0).
// Retorn: vector de nStates+1 estats en [UA] i [UA/s] relatius a m_orbitedObject cobrint un període orbital complet.
std::vector<CState3D> getOrbitStates(CelestialBodyId body, double et, int nStates);

// Carrega tots els kernels SPICE de `cbdata\` (amb fallback relatiu `..\..\cbdata\`). Operació idempotent.
void loadCspiceKernels();

// Allibera tots els kernels SPICE carregats en memòria mitjançant `kclear_c()`.
void unloadCspiceKernels();

// Precondició: carrega automàticament els kernels si encara no ho estan (necessita naif0012.tls).
// Paràmetres: data/hora UTC gregoriana (any [1550..2649], mes [1..12], dia [1..31], hora [0..23], minut [0..59], segon [0..59]).
// Retorn: temps d'efemèrides en [s TDB des de J2000] (0.0 si la conversió falla).
double utcToET(int any, int mes, int dia, int hora, int minut, int segon);
