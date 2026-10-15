#pragma once

#include <unordered_map>

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
    // --- SYSTEM MOONS ---
    CB_MOON = 301       // Earth
    //CB_PHOBOS,     // Mars
    //CB_DEIMOS,     // Mars
    //CB_IO,         // Jupiter
    //CB_EUROPA,
    //CB_GANYMEDE,
    //CB_CALLISTO,
    //CB_TITAN,      // Saturn
    //CB_ENCELADUS,
    //CB_TRITON,     // Neptune
    //CB_CHARON = 901,     // Pluto
    //CB_NIX = 902,
    //CB_HYDRA = 903,
    //CB_KERBEROS = 904,
    //CB_STYX = 905,
    //OTHER
    //CB_HALLEY = 1000036,
    //CB_VOYAGER1 = -31,
    //CB_VOYAGER2 = -32
};

const double KM_TO_AU = 1.0 / 149597870.7;

struct CB {
    std::string m_name;
    std::string m_id;
    bool m_hasRotFrame = false;
    std::string m_rotationFrame;
};

glm::mat4 getCB_posMat(CelestialBodyId body, CelestialBodyId origin, double et);
glm::mat4 getCB_rotMat(CelestialBodyId body, double et);
glm::mat4 getCB_radii(CelestialBodyId body);
