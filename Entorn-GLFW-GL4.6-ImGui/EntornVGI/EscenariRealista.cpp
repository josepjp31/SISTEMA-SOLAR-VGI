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

EscenariRealista::EscenariRealista()
	: m_ephemerisTimeET(0.0)
	, m_kernelsCarregats(false)
	, m_primerFrame(true)
{
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		m_posicionsAU[i] = glm::dvec3(0.0);
		m_etOrbita[i] = 0.0;
	}

	// Paràmetres de càmera adaptats a l'escala astronòmica (1 UA = 1000 unitats OpenGL):
	// aquest factor manté els radis planetaris (Terra ~0,0426 u, Lluna ~0,0116 u) lluny dels límits
	// de precisió float32, alhora que encabeix Neptú (~30.000 u) i Plutó (~40.000 u) dins del frustum.
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
	// Atenuació quadràtica suau (1e-10) en lloc de 1/r^2 físic pur: a 30-40 UA (30.000-40.000 u),
	// una caiguda 1/r^2 real enfosquiria completament Neptú i Plutó respecte a la Terra; amb 1e-10
	// el factor f_att = 1 / (1 + 1e-10 * d^2) es manté prop de 0.86-0.91 per garantir visibilitat.
	m_configCamera.atenuacioQuad = 1.0e-10f;
}

EscenariRealista::~EscenariRealista()
{
	alliberaEscenari();
}

void EscenariRealista::alliberaOrbitesLineStrip()
{
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		OrbitaLineVAO& orb = m_orbitesVAO[i];
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
		m_etOrbita[i] = 0.0;
	}
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

void EscenariRealista::recalcularOrbitaCos(int idx, double et)
{
	if (idx < 1 || idx >= NUM_COSSOS_REALISTA)
		return;

	OrbitaLineVAO& orb = m_orbitesVAO[idx];
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
	m_etOrbita[idx] = et;

	const DadesCosAstronomic& cos = CatalegAstronomic::GetCos(idx);
	if (cos.parentIndex < 0)
		return;

	std::vector<CState3D> estatsOrbita = getOrbitStates(cos.spiceId, et, NUM_ORBIT_SEGMENTS);
	if (estatsOrbita.empty())
		return;

	std::vector<VertexOrbitaLine> vertexs;
	vertexs.reserve(estatsOrbita.size());

	for (const CState3D& st : estatsOrbita)
	{
		VertexOrbitaLine v{};
		v.x = static_cast<float>(st.x * SCALE_UNITS_PER_AU);
		v.y = static_cast<float>(st.y * SCALE_UNITS_PER_AU);
		v.z = static_cast<float>(st.z * SCALE_UNITS_PER_AU);
		v.r = cos.colorCaracteristic.r;
		v.g = cos.colorCaracteristic.g;
		v.b = cos.colorCaracteristic.b;
		v.a = 1.0f;
		v.nx = 0.0f;
		v.ny = 0.0f;
		v.nz = 1.0f;
		v.u = 0.0f;
		v.v = 0.0f;
		vertexs.push_back(v);
	}

	orb.nVertexs = static_cast<GLint>(vertexs.size());
	orb.cosPareIdx = cos.parentIndex;
	orb.color = cos.colorCaracteristic;

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
}

void EscenariRealista::recalcularOrbites3D(double et)
{
	alliberaOrbitesLineStrip();

	for (int i = 1; i < NUM_COSSOS_REALISTA; i++)
	{
		recalcularOrbitaCos(i, et);
	}
}

void EscenariRealista::actualitzaCossosSPICE(double et)
{
	if (static_cast<int>(m_cossos.size()) < NUM_COSSOS_REALISTA) return;

	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		const DadesCosAstronomic& cos = CatalegAstronomic::GetCos(i);
		if (cos.parentIndex < 0)
		{
			m_posicionsAU[i] = glm::dvec3(0.0);
		}
		else if (cos.parentIndex > 0)
		{
			const DadesCosAstronomic& pare = CatalegAstronomic::GetCos(cos.parentIndex);
			CState3D stRel = getCBstate(cos.spiceId, pare.spiceId, et);
			m_posicionsAU[i] = m_posicionsAU[cos.parentIndex] + glm::dvec3(stRel.x, stRel.y, stRel.z);
		}
		else
		{
			CState3D st = getCBstate(cos.spiceId, CB_SUN, et);
			m_posicionsAU[i] = glm::dvec3(st.x, st.y, st.z);
		}

		m_cossos[i].m_posicio = glm::vec3(m_posicionsAU[i] * SCALE_UNITS_PER_AU);

		// Rotacio 3D real en J2000 (oblicuitat axial real + rotacio siderea exacta a l'instant et)
		m_cossos[i].m_mMatrix = getCBrotMat(cos.spiceId, et);
	}
}

glm::vec3 EscenariRealista::getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const
{
	if (idx < 0 || idx >= NUM_COSSOS_REALISTA || idx >= static_cast<int>(m_cossos.size()))
		return glm::vec3(0.0f);

	if (origenRelatiu && cosEnfocatId > 0 && cosEnfocatId < NUM_COSSOS_REALISTA)
	{
		// Resta en doble precisió (double, [UA]) abans de multiplicar per SCALE_UNITS_PER_AU i truncar a float32:
		// evita pèrdua de dígits significatius (cancel·lació catastròfica) i jitter de càmera/lluna quan
		// s'observa un planeta llunyà (p. ex. Neptú o Plutó a 30.000-40.000 u amb radis de ~0,01-0,16 u).
		return glm::vec3((m_posicionsAU[idx] - m_posicionsAU[cosEnfocatId]) * SCALE_UNITS_PER_AU);
	}

	return m_cossos[idx].m_posicio;
}

glm::vec3 EscenariRealista::getColorCos(int idx) const
{
	if (idx < 0 || idx >= NUM_COSSOS_REALISTA)
		return glm::vec3(0.0f, 0.78f, 1.0f);
	return CatalegAstronomic::GetCos(idx).colorCaracteristic;
}

void EscenariRealista::creaEscenari(GLuint& shader_programID)
{
	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;

	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);

	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));
	// Disc unitari per a l'anell de Saturn: radi interior ~1.24 * R_eq_Saturn, exterior ~2.27 * R_eq_Saturn
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(1.24, 2.27, 80, 5));

	alliberaTextures();
	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		const DadesCosAstronomic& cos = CatalegAstronomic::GetCos(i);
		if (cos.rutaTextura != nullptr)
		{
			m_texturesID[i] = carregaTextura2D(cos.rutaTextura);
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
		const DadesCosAstronomic& cos = CatalegAstronomic::GetCos(i);
		m_cossos[i].m_nom = cos.nom;
		float radiVisual = static_cast<float>((cos.radiMitjaKm / KM_PER_AU) * SCALE_UNITS_PER_AU);
		if (radiVisual < MIN_RADI_VISUAL) radiVisual = MIN_RADI_VISUAL;
		m_cossos[i].m_radiCos = radiVisual;
		m_cossos[i].m_radiOrbita = static_cast<float>(cos.distMitjanaRefUA * SCALE_UNITS_PER_AU);
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

		// Regeneracio selectiva de cada orbita 3D quan l'epoca avanca > 5% del periode orbital (> 1.5 dies per a la Lluna)
		constexpr double SEGONS_PER_DIA = 86400.0;
		for (int i = 1; i < NUM_COSSOS_REALISTA; i++)
		{
			const DadesCosAstronomic& cos = CatalegAstronomic::GetCos(i);
			if (cos.periodeOrbitalDies <= 0.0) continue;

			const double llindarSegons = (cos.spiceId == CB_MOON)
				? (1.5 * SEGONS_PER_DIA)
				: (0.05 * cos.periodeOrbitalDies * SEGONS_PER_DIA);

			if (std::abs(m_ephemerisTimeET - m_etOrbita[i]) > llindarSegons)
			{
				recalcularOrbitaCos(i, m_ephemerisTimeET);
			}
		}
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

		for (int i = 1; i < NUM_COSSOS_REALISTA; i++)
		{
			const OrbitaLineVAO& orb = m_orbitesVAO[i];
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
			const glm::vec3& col = CatalegAstronomic::GetCos(i).colorCaracteristic;
			glUniform1i(locTextur, GL_FALSE);
			CColor colCos{ col.r, col.g, col.b, 1.0 };
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

	// 3. Anells de Saturn escalats respecte al radi equatorial real (60.268,0 km -> 74.732 km .. 136.808 km)
	//    i orientats amb la matriu de rotació 3D real de SPICE (IAU_SATURN -> J2000, inclinació ~26.73 deg).
	//    Es dibuixen després de totes les esferes amb glDepthMask(GL_FALSE) per respectar el Z-buffer del planeta
	//    sense escriure profunditat des dels píxels semitransparents (evita oclusions errònies pel canal alfa).
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

	const DadesCosAstronomic& saturn = CatalegAstronomic::GetCos(6);
	float radiEquatorialSaturn = static_cast<float>((saturn.radiEquatorialKm / KM_PER_AU) * SCALE_UNITS_PER_AU);
	glm::vec3 posSaturn = getPosicioVista(6, estat.cosEnfocatId, origenRelatiu);
	glm::mat4 ModelAnell = glm::translate(MatriuTG, posSaturn);
	ModelAnell = ModelAnell * m_cossos[6].m_mMatrix;
	ModelAnell = glm::scale(ModelAnell, vec3(radiEquatorialSaturn, radiEquatorialSaturn, radiEquatorialSaturn));
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
