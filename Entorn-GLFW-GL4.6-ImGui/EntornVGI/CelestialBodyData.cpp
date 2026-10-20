#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CelestialBodyData.h"

// NASA CSPICE C API
extern "C" {
#include "cspice\SpiceUsr.h"
}

std::unordered_map<CelestialBodyId, CB> cbList = {

{CB_SOLAR_SYSTEM_BARYCENTER, {"SOLAR SYSTEM BARYCENTER","0", false, ""}},
{CB_SUN, {"SUN","10", true, "IAU_SUN"}},
{CB_MERCURY, {"MERCURY","199", true, "IAU_MERCURY"}},
{CB_VENUS, {"VENUS","299", true, "IAU_VENUS"}},
{CB_EARTH, {"EARTH","399", true, "IAU_EARTH"}},
{CB_MARS, {"MARS","4", true, "IAU_MARS"}},
{CB_JUPITER, {"JUPITER","5", true, "IAU_JUPITER"}},
{CB_SATURN, {"SATURN","6", true, "IAU_SATURN"}},
{CB_URANUS, {"URANUS","7", true, "IAU_URANUS"}},
{CB_NEPTUNE, {"NEPTUNE","8", true, "IAU_NEPTUNE"}},
{CB_PLUTO, {"PLUTO","999", true, "IAU_PLUTO"}},
{CB_CERES, {"CERES","2000001", true, "IAU_CERES"}},

{CB_MOON, {"MOON","301", true, "IAU_MOON"}},

};

std::string kernels[] = { 
    "cbdata\\naif0012.tls",
    "cbdata\\pck00011.tpc",
    "cbdata\\de440.bsp",
    "cbdata\\1000036.bsp",
    "cbdata\\codes_300ast_20100725.bsp",
    "cbdata\\plu060.bsp",
    "cbdata\\vgr1.x2100.bsp",
    "cbdata\\vgr2.x2100.bsp"
};

void loadCspiceKernels()
{
    for (std::string &file : kernels)
        furnsh_c(file.c_str());
}

void unloadCspiceKernels()
{
    kclear_c();
}

glm::mat4 getCB_posMat(CelestialBodyId body, CelestialBodyId origin, double et)
{
    glm::mat4 transfMatrix = glm::mat4(1.0f);

    double state[6];
    double lt;

    spkezr_c(cbList[body].m_id.c_str(), et, "J2000", "NONE", cbList[origin].m_id.c_str(), state, &lt);
    if (failed_c())
    {
        reset_c();
        return transfMatrix;
    }
    glm::vec3 positionVector(0.0f);

    positionVector.x = static_cast<float>(state[0] * KM_TO_AU);
    positionVector.y = static_cast<float>(state[1] * KM_TO_AU);
    positionVector.z = static_cast<float>(state[2] * KM_TO_AU);

    transfMatrix = glm::translate(transfMatrix, positionVector);

    return transfMatrix;
}
glm::mat4 getCB_rotMat(CelestialBodyId body, double et)
{
    double rot[3][3] = {};
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    
    if (!cbList[body].m_hasRotFrame)
        return rotationMatrix;

    pxform_c("J2000", cbList[body].m_rotationFrame.c_str(), et, rot);
    if (failed_c())
    {
        reset_c();
        return rotationMatrix;
    }

    glm::mat3 spiceRot = glm::make_mat3(&rot[0][0]);
    rotationMatrix = glm::mat4(glm::transpose(spiceRot));

    return rotationMatrix;
}

//float getCB_radii(CelestialBodyId body) //Probably will get erased.
//{
//    const CB props = cbList[body];
//
//
//    double radii[3] = { 0.0 };
//    SpiceInt dimReturned;
//
//    bodvrd_c(props.m_name.c_str(), "RADII", 3, &dimReturned, radii);
//
//    // 5. Isolate the first element (Equatorial X radius) and convert KM to AU
//    double equatorialRadiusKm = radii[0];
//    float radiusInAU = static_cast<float>(equatorialRadiusKm * KM_TO_AU);
//
//    return radiusInAU;
//}


