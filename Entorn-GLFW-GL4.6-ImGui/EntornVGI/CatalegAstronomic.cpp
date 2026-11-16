#include "CatalegAstronomic.h"

namespace
{
    // Font oficial: docs/planetes/planetes.json (NASA/JPL Solar System Dynamics i NSSDCA)
    // Ordre dels indexs: 0=Sol, 1..8=Mercuri..Neptu, 9=Pluto, 10=Lluna
    const DadesCosAstronomic kCatalegOficial[CatalegAstronomic::NUM_COSSOS] = {
        {
            CB_SUN, -1, "Sol", "Estrella",
            695700.0, 696000.0,
            0.0, 0.0,
            274.0, 609.12, 0.0, 7.25, 0.0,
            ".\\textures\\sun\\2k_sun.jpg",
            glm::vec3(1.00f, 0.90f, 0.40f)
        },
        {
            CB_MERCURY, 0, "Mercuri", "Planeta",
            2439.4, 2440.53,
            57900000.0, 57900000.0 / CatalegAstronomic::KM_PER_AU,
            3.7, 1407.6, 88.0, 0.034, 7.0,
            ".\\textures\\planet\\Mercury\\2k_mercury.jpg",
            glm::vec3(0.65f, 0.65f, 0.70f)
        },
        {
            CB_VENUS, 0, "Venus", "Planeta",
            6051.8, 6051.8,
            108200000.0, 108200000.0 / CatalegAstronomic::KM_PER_AU,
            8.87, -5832.5, 224.7, 177.4, 3.4,
            ".\\textures\\planet\\Venus\\2k_venus_surface.jpg",
            glm::vec3(0.90f, 0.75f, 0.45f)
        },
        {
            CB_EARTH, 0, "Terra", "Planeta",
            6371.0084, 6378.1366,
            149600000.0, 149600000.0 / CatalegAstronomic::KM_PER_AU,
            9.8, 23.9345, 365.2, 23.4, 0.0,
            ".\\textures\\planet\\Earth\\2k_earth_daymap.jpg",
            glm::vec3(0.30f, 0.68f, 0.95f)
        },
        {
            CB_MARS, 0, "Mart", "Planeta",
            3389.5, 3396.19,
            228000000.0, 228000000.0 / CatalegAstronomic::KM_PER_AU,
            3.71, 24.623, 687.0, 25.2, 1.8,
            ".\\textures\\planet\\Mars\\2k_mars.jpg",
            glm::vec3(0.88f, 0.42f, 0.30f)
        },
        {
            CB_JUPITER, 0, "Júpiter", "Planeta",
            69911.0, 71492.0,
            778500000.0, 778500000.0 / CatalegAstronomic::KM_PER_AU,
            24.79, 9.925, 4331.0, 3.1, 1.3,
            ".\\textures\\planet\\Jupiter\\2k_jupiter.jpg",
            glm::vec3(0.85f, 0.70f, 0.55f)
        },
        {
            CB_SATURN, 0, "Saturn", "Planeta",
            58232.0, 60268.0,
            1432000000.0, 1432000000.0 / CatalegAstronomic::KM_PER_AU,
            10.44, 10.65624, 10747.0, 26.7, 2.5,
            ".\\textures\\planet\\Saturn\\2k_saturn.jpg",
            glm::vec3(0.90f, 0.82f, 0.58f)
        },
        {
            CB_URANUS, 0, "Urà", "Planeta",
            25362.0, 25559.0,
            2867000000.0, 2867000000.0 / CatalegAstronomic::KM_PER_AU,
            8.87, -17.24, 30589.0, 97.8, 0.8,
            ".\\textures\\planet\\Uranus\\2k_uranus.jpg",
            glm::vec3(0.55f, 0.85f, 0.90f)
        },
        {
            CB_NEPTUNE, 0, "Neptú", "Planeta",
            24622.0, 24764.0,
            4515000000.0, 4515000000.0 / CatalegAstronomic::KM_PER_AU,
            11.15, 16.11, 59800.0, 28.3, 1.8,
            ".\\textures\\planet\\Neptune\\2k_neptune.jpg",
            glm::vec3(0.35f, 0.55f, 0.95f)
        },
        {
            CB_PLUTO, 0, "Plutó", "Planeta nan",
            1188.3, 1188.3,
            5906400000.0, 5906400000.0 / CatalegAstronomic::KM_PER_AU,
            0.62, -153.3, 90560.0, 119.5, 17.2,
            nullptr,
            glm::vec3(0.72f, 0.66f, 0.60f)
        },
        {
            CB_MOON, 3, "Lluna", "Satèl·lit natural",
            1737.4, 1738.1,
            384400.0, 384400.0 / CatalegAstronomic::KM_PER_AU,
            1.62, 655.72, 27.3217, 6.68, 5.145,
            ".\\textures\\moon\\2k_moon.jpg",
            glm::vec3(0.78f, 0.78f, 0.82f)
        }
    };
}

const DadesCosAstronomic& CatalegAstronomic::GetCos(int index)
{
    if (index < 0 || index >= NUM_COSSOS)
    {
        return kCatalegOficial[0];
    }
    return kCatalegOficial[index];
}

int CatalegAstronomic::FindIndexBySpiceId(CelestialBodyId spiceId)
{
    for (int i = 0; i < NUM_COSSOS; ++i)
    {
        if (kCatalegOficial[i].spiceId == spiceId)
        {
            return i;
        }
    }
    return -1;
}

const DadesCosAstronomic* CatalegAstronomic::GetAll()
{
    return kCatalegOficial;
}
