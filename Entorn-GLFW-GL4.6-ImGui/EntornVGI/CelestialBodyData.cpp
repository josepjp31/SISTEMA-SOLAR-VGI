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
{CB_PLUTO, {"PLUTO","999", true, "IAU_PLUTO", CB_PLUTO_BARYCENTER}},
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
    for (std::string &file : kernels)
        furnsh_c(file.c_str());
    char action[] = "RETURN";
    erract_c("SET", 0, action);
}

void unloadCspiceKernels()
{
    kclear_c();
}

CState3D getCBstate(CelestialBodyId body, CelestialBodyId origin, double et)
{
    double state[6] = {0,0,0,0,0,0}, lt;
    spkezr_c(cbList[body].m_id.c_str(), et, "J2000", "NONE", cbList[origin].m_id.c_str(), state, &lt);
    if (failed_c()) {
        reset_c();
        return CState3D(state);
    }
	CState3D res = CState3D(state);
  //  while (origin != globalSystemOrigin && origin != CB_NONE) {
		//body = origin;
  //      origin = cbList[origin].m_orbitedObject;
  //      spkezr_c(cbList[body].m_id.c_str(), et, "J2000", "NONE", cbList[origin].m_id.c_str(), state, &lt);
  //      if (failed_c()) {
  //          reset_c();
  //          return res;
  //      }
		//res.x += state[0]; res.y += state[1]; res.z += state[2];
  //  }

    return res;
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
    
    if (!cbList[body].m_hasRotFrame)
        return rotationMatrix;

    pxform_c("J2000", cbList[body].m_rotationFrame.c_str(), et, rot);
    if (failed_c())
    {
        reset_c();
        return rotationMatrix;
    }

	rotationMatrix = glm::mat4(glm::transpose(glm::make_mat3(&rot[0][0])));

    return rotationMatrix;
}

std::vector<CState3D> getOrbitStates(CelestialBodyId body, double et, int nStates)
{
	CB &props = cbList[body];
	std::vector<CState3D> states;

    if (props.m_orbitedObject == CB_NONE)
        return states;

    //get orbit T
    CState3D initialState = getCBstate(body, props.m_orbitedObject, et);
	double actualState[6] = { initialState.x, initialState.y, initialState.z, initialState.vx, initialState.vy, initialState.vz };
    double GM; SpiceInt dim;

	bodvrd_c(cbList[props.m_orbitedObject].m_id.c_str(), "GM", 1, &dim, &GM);
	if (failed_c())
	{
		reset_c();
		return states;
	}

    double oscReturns[SPICE_OSCLTX_NELTS];
	oscltx_c(actualState, et, GM, oscReturns);
    if (failed_c())
    {
        reset_c();
        return states;
    }
    //sample nStates states
	double t = oscReturns[10] / nStates;
    double vconst = oscReturns[10] / (nStates-1);
    double time = et;
    CState3D state, tmp, currentOriginState = getCBstate(cbList[body].m_orbitedObject, globalSystemOrigin, time);
    for (int i = 0; i < nStates; i++)
	{
		state = getCBstate(body, props.m_orbitedObject, time); 
        state = getCBstate(body, props.m_orbitedObject, time);// if (body == CB_MOON) {state.x *= 10.0; state.y *= 10.0; state.z *= 10.0;}
        state.x += currentOriginState.x; state.y += currentOriginState.y; state.z += currentOriginState.z;
		mulState(state, KM_TO_AU); 
		state.vx *= vconst; state.vy *= vconst; state.vz *= vconst;
		states.push_back(state);
		time += t;
	}
    state = getCBstate(body, props.m_orbitedObject, time); 
    state = getCBstate(body, props.m_orbitedObject, time);// if (body == CB_MOON) { state.x *= 10.0; state.y *= 10.0; state.z *= 10.0; }
    state.x += currentOriginState.x; state.y += currentOriginState.y; state.z += currentOriginState.z;
    mulState(state, KM_TO_AU);
    state.vx *= vconst; state.vy *= vconst; state.vz *= vconst;
    states.push_back(state);

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
