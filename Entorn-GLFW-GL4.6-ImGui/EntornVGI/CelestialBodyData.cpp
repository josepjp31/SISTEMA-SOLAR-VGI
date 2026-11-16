#include <cstdio>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CelestialBodyData.h"

// NASA CSPICE C API
extern "C" {
#include "cspice\SpiceUsr.h"
}

namespace
{
    bool s_kernelsLoaded = false;

    // Consulta directa en [km] i [km/s] a CSPICE (sense convertir a UA) perquè oscltx_c i conics_c
    // exigeixen unitats mètriques coherents amb el paràmetre gravitatori GM [km^3/s^2] de gm_de440.tpc.
    CState3D getCBstateKm(CelestialBodyId body, CelestialBodyId origin, double et, bool* ok = nullptr)
    {
        double state[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        double lt = 0.0;

        auto itBody = cbList.find(body);
        auto itOrigin = cbList.find(origin);
        if (itBody == cbList.end() || itOrigin == cbList.end())
        {
            if (ok) *ok = false;
            return CState3D();
        }

        spkezr_c(itBody->second.m_id.c_str(), et, "J2000", "NONE", itOrigin->second.m_id.c_str(), state, &lt);
        if (failed_c())
        {
            reset_c();
            // Fallback per a Plutó fora de la finestra 1800-2199: el kernel plu060.bsp només cobreix
            // el centre del cos (999) en aquest rang, mentre que el baricentre (9) dins de440.bsp
            // cobreix tot el rang de dates del simulador (1550-2649) amb error negligible a escala orbital.
            if (body == CB_PLUTO)
            {
                spkezr_c("9", et, "J2000", "NONE", itOrigin->second.m_id.c_str(), state, &lt);
                if (!failed_c())
                {
                    if (ok) *ok = true;
                    return CState3D(state);
                }
                reset_c();
            }
            if (ok) *ok = false;
            return CState3D();
        }

        if (ok) *ok = true;
        return CState3D(state);
    }
}

CelestialBodyId globalSystemOrigin = CB_SUN;

std::unordered_map<CelestialBodyId, CB> cbList = {

{CB_SOLAR_SYSTEM_BARYCENTER, {"SOLAR SYSTEM BARYCENTER","0", false, "", CB_NONE}},
{CB_PLUTO_BARYCENTER, {"PLUTO BARYCENTER","9", false, "", CB_SUN}},

{CB_SUN, {"SUN","10", true, "IAU_SUN", CB_NONE}},
{CB_MERCURY, {"MERCURY","199", true, "IAU_MERCURY", CB_SUN}},
{CB_VENUS, {"VENUS","299", true, "IAU_VENUS", CB_SUN}},
{CB_EARTH, {"EARTH","399", true, "IAU_EARTH", CB_SUN}},
{CB_MARS, {"MARS","4", true, "IAU_MARS", CB_SUN}},
{CB_JUPITER, {"JUPITER","5", true, "IAU_JUPITER", CB_SUN}},
{CB_SATURN, {"SATURN","6", true, "IAU_SATURN", CB_SUN}},
{CB_URANUS, {"URANUS","7", true, "IAU_URANUS", CB_SUN}},
{CB_NEPTUNE, {"NEPTUNE","8", true, "IAU_NEPTUNE", CB_SUN}},
{CB_PLUTO, {"PLUTO","999", true, "IAU_PLUTO", CB_SUN}},
{CB_CERES, {"CERES","2000001", true, "IAU_CERES", CB_SUN}},
{CB_MOON, {"MOON","301", true, "IAU_MOON", CB_EARTH}},
{CB_CHARON, {"CHARON","901", true, "IAU_CHARON", CB_PLUTO_BARYCENTER}},

{CB_VOYAGER1, {"VOYAGER1","-31", false, "", CB_NONE}},

};

std::string kernels[] = { 
    "cbdata\\naif0012.tls",
    "cbdata\\pck00011.tpc",
    "cbdata\\de440.bsp",
    "cbdata\\1000036.bsp",
    "cbdata\\codes_300ast_20100725.bsp",
    "cbdata\\plu060.bsp",
    "cbdata\\vgr1.x2100.bsp",
    "cbdata\\vgr2.x2100.bsp",
    "cbdata\\gm_de440.tpc"
};

void loadCspiceKernels()
{
    if (s_kernelsLoaded)
        return;

    erract_c("SET", 0, (char*)"RETURN");
    errprt_c("SET", 0, (char*)"NONE");

    for (const std::string& file : kernels)
    {
        furnsh_c(file.c_str());
        if (failed_c())
        {
            reset_c();
            std::string fallback = "..\\..\\" + file;
            furnsh_c(fallback.c_str());
            if (failed_c())
                reset_c();
        }
    }

    s_kernelsLoaded = true;
}

void unloadCspiceKernels()
{
    if (!s_kernelsLoaded)
        return;

    kclear_c();
    if (failed_c())
        reset_c();

    s_kernelsLoaded = false;
}

double utcToET(int any, int mes, int dia, int hora, int minut, int segon)
{
    if (!s_kernelsLoaded)
        loadCspiceKernels();

    char utcStr[32];
    std::snprintf(utcStr, sizeof(utcStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                  any, mes, dia, hora, minut, segon);

    double et = 0.0;
    str2et_c(utcStr, &et);
    if (failed_c())
    {
        reset_c();
        return 0.0;
    }

    return et;
}

CState3D getCBstate(CelestialBodyId body, CelestialBodyId origin, double et)
{
    CState3D res = getCBstateKm(body, origin, et);
    return mulState(res, KM_TO_AU);
}

CState3D mulState(CState3D &state, double val)
{
	state.x *= val; state.y *= val; state.z *= val;
	state.vx *= val; state.vy *= val; state.vz *= val;
	return state;
}

glm::mat4 getCBposMat(CelestialBodyId body, CelestialBodyId origin, double et)
{
    CState3D state = getCBstate(body, origin, et);
    glm::vec3 v(state.x, state.y, state.z);
    return glm::translate(glm::mat4(1.0f), v);
}

glm::mat4 getCBrotMat(CelestialBodyId body, double et)
{
    double rot[3][3] = {};
    glm::mat4 rotationMatrix = glm::mat4(1.0f);

    auto it = cbList.find(body);
    if (it == cbList.end() || !it->second.m_hasRotFrame)
        return rotationMatrix;

    // Transformacio de vectors del marc propi (IAU_*) cap al marc inercial J2000
    pxform_c(it->second.m_rotationFrame.c_str(), "J2000", et, rot);
    if (failed_c())
    {
        reset_c();
        return rotationMatrix;
    }

    // CSPICE emmagatzema rot[3][3] en format row-major (C), mentre que glm::make_mat3 interpreta
    // els 9 doubles seqüencials com a columnes (column-major); transpose converteix la disposició
    // de memòria de row-major a column-major sense invertir la transformació física.
    rotationMatrix = glm::mat4(glm::transpose(glm::make_mat3(&rot[0][0])));

    return rotationMatrix;
}

std::vector<CState3D> getOrbitStates(CelestialBodyId body, double et, int nStates)
{
    std::vector<CState3D> states;
    if (nStates <= 0)
        return states;

    auto it = cbList.find(body);
    if (it == cbList.end())
        return states;

    const CB& props = it->second;
    if (props.m_orbitedObject == CB_NONE)
        return states;

    // Cal consultar l'estat inicial en [km] i [km/s] mitjançant getCBstateKm() perquè bodvrd_c
    // retorna GM en [km^3/s^2] i oscltx_c necessita posició/velocitat en km per calcular el període orbital oscReturns[10].
    bool okInit = false;
    CState3D initialState = getCBstateKm(body, props.m_orbitedObject, et, &okInit);
    if (!okInit)
        return states;

    double actualState[6] = {
        initialState.x, initialState.y, initialState.z,
        initialState.vx, initialState.vy, initialState.vz
    };
    double GM = 0.0;
    SpiceInt dim = 0;

    bodvrd_c(cbList[props.m_orbitedObject].m_id.c_str(), "GM", 1, &dim, &GM);
    if (failed_c())
    {
        reset_c();
        return states;
    }

    double oscReturns[SPICE_OSCLTX_NELTS];
    oscltx_c(actualState, et, GM, oscReturns);
    if (failed_c() || oscReturns[10] <= 0.0)
    {
        reset_c();
        return states;
    }

    // Mostreig d'un periode orbital complet en UA relatiu al cos orbitat (Sol o Terra per a la Lluna)
    double tStep = oscReturns[10] / static_cast<double>(nStates);
    double time = et;
    states.reserve(static_cast<size_t>(nStates) + 1);

    for (int i = 0; i <= nStates; i++)
    {
        bool okSample = false;
        CState3D state = getCBstateKm(body, props.m_orbitedObject, time, &okSample);
        if (!okSample)
        {
            double conicState[6] = { 0.0 };
            conics_c(oscReturns, time, conicState);
            if (failed_c())
            {
                reset_c();
                continue;
            }
            state = CState3D(conicState);
        }

        mulState(state, KM_TO_AU);
        states.push_back(state);
        time += tStep;
    }

    return states;
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
