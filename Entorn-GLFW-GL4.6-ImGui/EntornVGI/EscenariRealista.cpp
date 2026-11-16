#include "EscenariRealista.h"
#include "InterficieUI.h"
#include <cmath>
#include <cstddef>

namespace
{
	struct VertexOrbitaLine
	{
		float x, y, z;
		float r, g, b, a;
		float nx, ny, nz;
		float u, v;
	};
}

// Taula de metadades reals (radis mitjans en km segons docs/planetes/planetes.json):
// Sol, 8 planetes, Pluto i Lluna
const CosRealMetadata EscenariRealista::kMetadadesReals[NUM_COSSOS_REALISTA] = {
	{ CB_SUN,     "Sol",     695700.0,    7.25,  0.000000,  0.000,     0.00,    609.12,  ".\\textures\\sun\\2k_sun.jpg",                   glm::vec3(1.00f, 0.90f, 0.40f) },
	{ CB_MERCURY, "Mercuri",   2439.4,    0.03,  0.387098,  7.005,    87.97,   1407.60,  ".\\textures\\planet\\Mercury\\2k_mercury.jpg",   glm::vec3(0.65f, 0.65f, 0.70f) },
	{ CB_VENUS,   "Venus",     6051.8,  177.36,  0.723332,  3.395,   224.70,  -5832.50,  ".\\textures\\planet\\Venus\\2k_venus_surface.jpg", glm::vec3(0.90f, 0.75f, 0.45f) },
	{ CB_EARTH,   "Terra",     6371.0084, 23.44, 1.000000,  0.000,   365.25,     23.93,  ".\\textures\\planet\\Earth\\2k_earth_daymap.jpg",  glm::vec3(0.30f, 0.68f, 0.95f) },
	{ CB_MARS,    "Mart",      3389.5,   25.19,  1.523679,  1.850,   686.98,     24.62,  ".\\textures\\planet\\Mars\\2k_mars.jpg",         glm::vec3(0.88f, 0.42f, 0.30f) },
	{ CB_JUPITER, "Júpiter",  69911.0,    3.13,  5.204400,  1.303,  4332.59,      9.93,  ".\\textures\\planet\\Jupiter\\2k_jupiter.jpg",   glm::vec3(0.85f, 0.70f, 0.55f) },
	{ CB_SATURN,  "Saturn",   58232.0,   26.73,  9.582600,  2.485, 10759.22,     10.70,  ".\\textures\\planet\\Saturn\\2k_saturn.jpg",     glm::vec3(0.90f, 0.82f, 0.58f) },
	{ CB_URANUS,  "Urà",      25362.0,   97.77, 19.218400,  0.773, 30688.50,    -17.24,  ".\\textures\\planet\\Uranus\\2k_uranus.jpg",     glm::vec3(0.55f, 0.85f, 0.90f) },
	{ CB_NEPTUNE, "Neptú",    24622.0,   28.32, 30.110387,  1.770, 60195.00,     16.11,  ".\\textures\\planet\\Neptune\\2k_neptune.jpg",   glm::vec3(0.35f, 0.55f, 0.95f) },
	{ CB_PLUTO,   "Plutó",     1188.3,  122.53, 39.482000, 17.160, 90560.00,   -153.29,  nullptr,                                          glm::vec3(0.72f, 0.66f, 0.60f) },
	{ CB_MOON,    "Lluna",     1737.4,    6.68,  0.002570,  5.145,    27.32,    655.72,  nullptr,                                          glm::vec3(0.78f, 0.78f, 0.82f) }
};

EscenariRealista::EscenariRealista()
	: m_ephemerisTimeET(0.0)
	, m_kernelsCarregats(false)
	, m_primerFrame(true)
{
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		m_posicionsAU[i] = glm::dvec3(0.0);
	}

	// Parametres de camera adaptats a l'escala astronomica en UA (SCALE_UNITS_PER_AU = 1000)
	m_configCamera.distInicialSol = 1500.0f;
	m_configCamera.angleInclinacioInicial = 25.0f;
	m_configCamera.globalSistemaR = 35000.0f;
	m_configCamera.angleGlobal = 65.0f;
	m_configCamera.minDistSol = 0.0005f;
	m_configCamera.minDistSolFixa = false;
	m_configCamera.maxR = 80000.0f;
	m_configCamera.factorFocusPlaneta = 4.5f;
	m_configCamera.minDistFocusPlaneta = 0.002f;
	m_configCamera.minDistFocusZoom = 0.0005f;
	m_configCamera.factorRadiMinZoom = 2.2f;
	m_configCamera.pasZoomSol = 30.0f;
	m_configCamera.minPasZoomPlaneta = 0.0005f;
	m_configCamera.factorPasZoomPlaneta = 0.12f;
	m_configCamera.zoomLogaritmic = true;
	m_configCamera.factorZoomLogaritmic = 0.12f;
	m_configCamera.minPasZoomLogaritmic = 0.0005f;
	m_configCamera.navPosX = 900.0f;
	m_configCamera.navPosY = -900.0f;
	m_configCamera.navPosZ = 280.0f;
	m_configCamera.navPas = 16.0f;
	m_configCamera.atenuacioQuad = 1.0e-10f;
}

EscenariRealista::~EscenariRealista()
{
	alliberaEscenari();
}

void EscenariRealista::alliberaOrbitesLineStrip()
{
	for (OrbitaLineVAO& orb : m_orbitesVAO)
	{
		if (orb.vaoId != 0)
		{
			glDeleteVertexArrays(1, &orb.vaoId);
			orb.vaoId = 0;
		}
		if (orb.vboId != 0)
		{
			glDeleteBuffers(1, &orb.vboId);
			orb.vboId = 0;
		}
		orb.nVertexs = 0;
	}
	m_orbitesVAO.clear();
}

void EscenariRealista::alliberaEscenari()
{
	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	alliberaOrbitesLineStrip();
	Escenari::alliberaEscenari();
	if (m_kernelsCarregats)
	{
		unloadCspiceKernels();
		m_kernelsCarregats = false;
	}
}

void EscenariRealista::recalcularOrbites3D(double et)
{
	alliberaOrbitesLineStrip();

	for (int i = 1; i < NUM_COSSOS_REALISTA; i++)
	{
		const CosRealMetadata& meta = kMetadadesReals[i];
		std::vector<CState3D> estatsOrbita = getOrbitStates(meta.spiceId, et, NUM_ORBIT_SEGMENTS);
		if (estatsOrbita.empty())
			continue;

		std::vector<VertexOrbitaLine> vertexs;
		vertexs.reserve(estatsOrbita.size());

		for (const CState3D& st : estatsOrbita)
		{
			VertexOrbitaLine v{};
			v.x = static_cast<float>(st.x * SCALE_UNITS_PER_AU);
			v.y = static_cast<float>(st.y * SCALE_UNITS_PER_AU);
			v.z = static_cast<float>(st.z * SCALE_UNITS_PER_AU);
			v.r = meta.colorOrbita.r;
			v.g = meta.colorOrbita.g;
			v.b = meta.colorOrbita.b;
			v.a = 1.0f;
			v.nx = 0.0f;
			v.ny = 0.0f;
			v.nz = 1.0f;
			v.u = 0.0f;
			v.v = 0.0f;
			vertexs.push_back(v);
		}

		OrbitaLineVAO orb;
		orb.nVertexs = static_cast<GLint>(vertexs.size());
		orb.cosPareIdx = (meta.spiceId == CB_MOON) ? 3 : 0; // 3 = Terra
		orb.color = meta.colorOrbita;

		glGenVertexArrays(1, &orb.vaoId);
		glGenBuffers(1, &orb.vboId);

		glBindVertexArray(orb.vaoId);
		glBindBuffer(GL_ARRAY_BUFFER, orb.vboId);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexs.size() * sizeof(VertexOrbitaLine)), vertexs.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOrbitaLine), reinterpret_cast<void*>(offsetof(VertexOrbitaLine, x)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexOrbitaLine), reinterpret_cast<void*>(offsetof(VertexOrbitaLine, r)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOrbitaLine), reinterpret_cast<void*>(offsetof(VertexOrbitaLine, nx)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexOrbitaLine), reinterpret_cast<void*>(offsetof(VertexOrbitaLine, u)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_orbitesVAO.push_back(orb);
	}
}

void EscenariRealista::actualitzaCossosSPICE(double et)
{
	if (static_cast<int>(m_cossos.size()) < NUM_COSSOS_REALISTA) return;

	// Posicio heliocentrica de la Terra per situar la Lluna calculada relativament a CB_EARTH
	CState3D stTerra = getCBstate(CB_EARTH, CB_SUN, et);

	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		const CosRealMetadata& meta = kMetadadesReals[i];
		if (meta.spiceId == CB_SUN)
		{
			m_posicionsAU[i] = glm::dvec3(0.0);
		}
		else if (meta.spiceId == CB_MOON)
		{
			CState3D stMoonRel = getCBstate(CB_MOON, CB_EARTH, et);
			m_posicionsAU[i] = glm::dvec3(
				stTerra.x + stMoonRel.x,
				stTerra.y + stMoonRel.y,
				stTerra.z + stMoonRel.z
			);
		}
		else
		{
			CState3D st = getCBstate(meta.spiceId, CB_SUN, et);
			m_posicionsAU[i] = glm::dvec3(st.x, st.y, st.z);
		}

		m_cossos[i].m_posicio = glm::vec3(m_posicionsAU[i] * SCALE_UNITS_PER_AU);

		// Rotacio 3D real en J2000 (oblicuitat axial real + rotacio siderea exacta a l'instant et)
		m_cossos[i].m_mMatrix = getCBrotMat(meta.spiceId, et);
	}
}

glm::vec3 EscenariRealista::getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const
{
	if (idx < 0 || idx >= NUM_COSSOS_REALISTA || idx >= static_cast<int>(m_cossos.size()))
		return glm::vec3(0.0f);

	if (origenRelatiu && cosEnfocatId > 0 && cosEnfocatId < NUM_COSSOS_REALISTA)
	{
		// Resta en doble precisio abans de passar a float per evitar jitter en cossos llunyans
		return glm::vec3((m_posicionsAU[idx] - m_posicionsAU[cosEnfocatId]) * SCALE_UNITS_PER_AU);
	}

	return m_cossos[idx].m_posicio;
}

glm::vec3 EscenariRealista::getColorCos(int idx) const
{
	if (idx < 0 || idx >= NUM_COSSOS_REALISTA)
		return glm::vec3(0.0f, 0.78f, 1.0f);
	return kMetadadesReals[idx].colorOrbita;
}

void EscenariRealista::creaEscenari(GLuint& shader_programID)
{
	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;

	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);

	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));
	// Disc unitari per a l'anell de Saturn: radi interior ~1.24 * R_Saturno, exterior ~2.27 * R_Saturno
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(1.24, 2.27, 80, 5));

	alliberaTextures();
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		if (kMetadadesReals[i].rutaTextura != nullptr)
		{
			m_texturesID[i] = carregaTextura2D(kMetadadesReals[i].rutaTextura);
		}
	}
	// Index 11: textura dels anells de Saturn
	m_texturesID[11] = carregaTextura2D(".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png");

	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	m_cossos.clear();
	m_cossos.resize(NUM_COSSOS_REALISTA);
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		const CosRealMetadata& meta = kMetadadesReals[i];
		m_cossos[i].m_nom = meta.nom;
		float radiVisual = static_cast<float>((meta.radiEquatorialKm / KM_PER_AU) * SCALE_UNITS_PER_AU);
		if (radiVisual < MIN_RADI_VISUAL) radiVisual = MIN_RADI_VISUAL;
		m_cossos[i].m_radiCos = radiVisual;
		m_cossos[i].m_radiOrbita = static_cast<float>(meta.semieixMajorUA * SCALE_UNITS_PER_AU);
		m_cossos[i].m_angleOrbita = 0.0f;
		m_cossos[i].m_angleRotacio = 0.0f;
		m_cossos[i].m_velAngular = 0.0f;
		m_cossos[i].m_velRotacio = 0.0f;
		m_cossos[i].m_mMatrix = glm::mat4(1.0f);
	}

	// Carrega de kernels NASA CSPICE; el calcul inicial d'efemerides i orbites 3D es fa una sola vegada a updateVariables()
	loadCspiceKernels();
	m_kernelsCarregats = true;
	m_primerFrame = true;
}

void EscenariRealista::updateVariables(double dt, EstatSimulacio& estat)
{
	if (m_cossos.empty()) return;

	if (m_primerFrame || estat.recalculEfemeridesNecessari)
	{
		const DataHoraSimulacio& dh = estat.tempsSimulacio.dataHora;
		m_ephemerisTimeET = utcToET(dh.any, dh.mes, dh.dia, dh.hora, dh.minut, dh.segon);
		recalcularOrbites3D(m_ephemerisTimeET);
		estat.recalculEfemeridesNecessari = false;
		m_primerFrame = false;
	}
	else if (!estat.tempsSimulacio.enPausa)
	{
		double dtClamped = dt;
		if (dtClamped < 0.0) dtClamped = 0.0;
		if (dtClamped > 0.25) dtClamped = 0.25;
		m_ephemerisTimeET += dtClamped
			* static_cast<double>(estat.tempsSimulacio.direccioTemps)
			* estat.tempsSimulacio.factorVelocitatSegonsPerSegon;
	}

	actualitzaCossosSPICE(m_ephemerisTimeET);
}

void EscenariRealista::dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat)
{
	(void)MatriuVista;
	if (static_cast<int>(m_cossos.size()) < NUM_COSSOS_REALISTA) return;

	CColor col_blanc{ 1.0, 1.0, 1.0, 1.0 };
	GLint locModelMat = glGetUniformLocation(sh_programID, "modelMatrix");
	GLint locSwInt = glGetUniformLocation(sh_programID, "sw_intensity");
	GLint locTextur = glGetUniformLocation(sh_programID, "textur");
	GLint locModulate = glGetUniformLocation(sh_programID, "modulate");
	GLint locInvertY = glGetUniformLocation(sh_programID, "flag_invert_y");

	const bool origenRelatiu = (estat.cameraConfig.mode != ModeCamera::COET_PRIMERA_PERSONA
		&& estat.cosEnfocatId > 0
		&& estat.cosEnfocatId < NUM_COSSOS_REALISTA);

	// 1. Orbites 3D reals amb GL_LINE_STRIP
	if (estat.opcionsVisualitzacio.mostrarOrbites)
	{
		glUniform1i(locTextur, GL_FALSE);
		bool sw_mat_orbita[5] = { true, false, false, false, true };

		for (const OrbitaLineVAO& orb : m_orbitesVAO)
		{
			if (orb.vaoId == 0 || orb.nVertexs <= 0) continue;

			CColor col_orbita{ orb.color.r, orb.color.g, orb.color.b, 1.0 };
			SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat_orbita);

			glm::vec3 posPare = getPosicioVista(orb.cosPareIdx, estat.cosEnfocatId, origenRelatiu);
			glm::mat4 ModelMatrix = glm::translate(MatriuTG, posPare);
			glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelMatrix[0][0]);

			glBindVertexArray(orb.vaoId);
			glDrawArrays(GL_LINE_STRIP, 0, orb.nVertexs);
			glBindVertexArray(0);
		}
	}

	// 2. Cossos celestes (Sol, 8 planetes, Pluto i Lluna)
	glUniform1i(locModulate, GL_TRUE);
	glUniform1i(locInvertY, GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		if (m_texturesID[i] != 0)
		{
			glUniform1i(locTextur, GL_TRUE);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
			SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);
		}
		else
		{
			glUniform1i(locTextur, GL_FALSE);
			CColor colCos{ kMetadadesReals[i].colorOrbita.r, kMetadadesReals[i].colorOrbita.g, kMetadadesReals[i].colorOrbita.b, 1.0 };
			SeleccionaColorMaterial(sh_programID, colCos, sw_mat);
		}

		glm::vec3 posCos = getPosicioVista(i, estat.cosEnfocatId, origenRelatiu);
		glm::mat4 ModelMatrix = glm::translate(MatriuTG, posCos);
		ModelMatrix = ModelMatrix * m_cossos[i].m_mMatrix;
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_cossos[i].m_radiCos, m_cossos[i].m_radiCos, m_cossos[i].m_radiCos));

		glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform4i(locSwInt, (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
		draw_TriEBO_Object(GLU_SPHERE);
	}

	// 3. Anells de Saturn escalats proporcionalment al radi real de Saturn (index 6: 1.24x..2.27x R_Saturn)
	//    i orientats amb la matriu de rotacio 3D real de SPICE (IAU_SATURN -> J2000, inclinacio 26.73 deg)
	bool sw_mat_no_emisio[5] = { false, sw_mat[1], sw_mat[2], sw_mat[3], sw_mat[4] };
	glUniform1i(locTextur, GL_TRUE);
	glUniform1i(locModulate, GL_TRUE);
	glUniform1i(locInvertY, GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat_no_emisio);

	if (m_texturesID[11] != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texturesID[11]);
	}

	float radiSaturn = m_cossos[6].m_radiCos;
	glm::vec3 posSaturn = getPosicioVista(6, estat.cosEnfocatId, origenRelatiu);
	glm::mat4 ModelAnell = glm::translate(MatriuTG, posSaturn);
	ModelAnell = ModelAnell * m_cossos[6].m_mMatrix;
	ModelAnell = glm::scale(ModelAnell, vec3(radiSaturn, radiSaturn, radiSaturn));
	glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelAnell[0][0]);
	glUniform4i(locSwInt, GL_FALSE, sw_mat[1], sw_mat[2], sw_mat[3]);

	GLint locTwoSided = glGetUniformLocation(sh_programID, "sw_twoSided");
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	draw_TriEBO_Object(GLU_DISK);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_FALSE);

	glUniform4i(locSwInt, sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(locTextur, GL_FALSE);
}
