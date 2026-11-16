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

	// Aproximacio de segons transcorreguts des de J2000 (2000-01-01 12:00:00 UTC)
	// fins que a la Fase 5 es connecti amb str2et_c de CSPICE.
	double calculaSegonsDesDeJ2000(const DataHoraSimulacio& dh)
	{
		int a = dh.any;
		int m = dh.mes;
		if (m <= 2)
		{
			a -= 1;
			m += 12;
		}
		long long A = a / 100;
		long long B = 2 - A + (A / 4);
		double jd = std::floor(365.25 * (a + 4716))
			+ std::floor(30.6001 * (m + 1))
			+ static_cast<double>(dh.dia)
			+ static_cast<double>(B)
			- 1524.5;
		double fraccioDia = (dh.hora - 12.0) / 24.0 + dh.minut / 1440.0 + dh.segon / 86400.0;
		return ((jd + fraccioDia) - 2451545.0) * 86400.0;
	}

	glm::vec3 calculaPosicioOrbital3D(double radiUA, double inclinacioGraus, double angleRad, double scaleUnitsPerAU)
	{
		double r = radiUA * scaleUnitsPerAU;
		double incRad = glm::radians(inclinacioGraus);
		double x = r * std::cos(angleRad);
		double yOrb = r * std::sin(angleRad);
		double y = yOrb * std::cos(incRad);
		double z = yOrb * std::sin(incRad);
		return glm::vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}
}

// Taula de metadades reals: Sol, 8 planetes, Pluto i Lluna
const CosRealMetadata EscenariRealista::kMetadadesReals[NUM_COSSOS_REALISTA] = {
	{ CB_SUN,     "Sol",     696340.0,   7.25,  0.000000,  0.000,     0.00,    609.12,  ".\\textures\\sun\\2k_sun.jpg",                   glm::vec3(1.00f, 0.90f, 0.40f) },
	{ CB_MERCURY, "Mercuri",   2439.7,   0.03,  0.387098,  7.005,    87.97,   1407.60,  ".\\textures\\planet\\Mercury\\2k_mercury.jpg",   glm::vec3(0.65f, 0.65f, 0.70f) },
	{ CB_VENUS,   "Venus",     6051.8, 177.36,  0.723332,  3.395,   224.70,  -5832.50,  ".\\textures\\planet\\Venus\\2k_venus_surface.jpg", glm::vec3(0.90f, 0.75f, 0.45f) },
	{ CB_EARTH,   "Terra",     6371.0,  23.44,  1.000000,  0.000,   365.25,     23.93,  ".\\textures\\planet\\Earth\\2k_earth_daymap.jpg",  glm::vec3(0.30f, 0.68f, 0.95f) },
	{ CB_MARS,    "Mart",      3389.5,  25.19,  1.523679,  1.850,   686.98,     24.62,  ".\\textures\\planet\\Mars\\2k_mars.jpg",         glm::vec3(0.88f, 0.42f, 0.30f) },
	{ CB_JUPITER, "Júpiter",  69911.0,   3.13,  5.204400,  1.303,  4332.59,      9.93,  ".\\textures\\planet\\Jupiter\\2k_jupiter.jpg",   glm::vec3(0.85f, 0.70f, 0.55f) },
	{ CB_SATURN,  "Saturn",   58232.0,  26.73,  9.582600,  2.485, 10759.22,     10.70,  ".\\textures\\planet\\Saturn\\2k_saturn.jpg",     glm::vec3(0.90f, 0.82f, 0.58f) },
	{ CB_URANUS,  "Urà",      25362.0,  97.77, 19.218400,  0.773, 30688.50,    -17.24,  ".\\textures\\planet\\Uranus\\2k_uranus.jpg",     glm::vec3(0.55f, 0.85f, 0.90f) },
	{ CB_NEPTUNE, "Neptú",    24622.0,  28.32, 30.110387,  1.770, 60195.00,     16.11,  ".\\textures\\planet\\Neptune\\2k_neptune.jpg",   glm::vec3(0.35f, 0.55f, 0.95f) },
	{ CB_PLUTO,   "Plutó",     1188.3, 122.53, 39.482000, 17.160, 90560.00,   -153.29,  nullptr,                                          glm::vec3(0.72f, 0.66f, 0.60f) },
	{ CB_MOON,    "Lluna",     1737.4,   6.68,  0.002570,  5.145,    27.32,    655.72,  nullptr,                                          glm::vec3(0.78f, 0.78f, 0.82f) }
};

EscenariRealista::EscenariRealista()
	: m_ephemerisTimeET(0.0)
	, m_kernelsCarregats(false)
{
	// Parametres de camera adaptats a l'escala astronomica en UA (SCALE_UNITS_PER_AU = 500)
	m_configCamera.distInicialSol = 380.0f;
	m_configCamera.angleInclinacioInicial = 25.0f;
	m_configCamera.distGlobal = 6500.0f;
	m_configCamera.angleGlobal = 65.0f;
	m_configCamera.minDistSol = 5.0f;
	m_configCamera.maxDistCamera = 42000.0f;
	m_configCamera.factorFocusPlaneta = 5.0f;
	m_configCamera.minDistFocusPlaneta = 0.04f;
	m_configCamera.minDistFocusZoom = 0.02f;
	m_configCamera.factorRadiMinZoom = 2.2f;
	m_configCamera.pasZoomSol = 15.0f;
	m_configCamera.minPasZoomPlaneta = 0.01f;
	m_configCamera.factorPasZoomPlaneta = 0.45f;
	m_configCamera.navPosX = 450.0f;
	m_configCamera.navPosY = -450.0f;
	m_configCamera.navPosZ = 140.0f;
	m_configCamera.navPas = 8.0f;
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
}

void EscenariRealista::generaOrbitesLineStrip()
{
	alliberaOrbitesLineStrip();

	const int numSegments = 256;
	for (int i = 1; i < NUM_COSSOS_REALISTA; i++)
	{
		const CosRealMetadata& meta = kMetadadesReals[i];
		std::vector<VertexOrbitaLine> vertexs;
		vertexs.reserve(numSegments + 1);

		for (int s = 0; s <= numSegments; s++)
		{
			double ang = (2.0 * PI * static_cast<double>(s)) / static_cast<double>(numSegments);
			glm::vec3 p = calculaPosicioOrbital3D(meta.semieixMajorUA, meta.inclinacioOrbitalGraus, ang, SCALE_UNITS_PER_AU);

			VertexOrbitaLine v{};
			v.x = p.x;
			v.y = p.y;
			v.z = p.z;
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
		orb.cosPareIdx = (meta.spiceId == CB_MOON) ? 3 : 0;
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

void EscenariRealista::actualitzaPosicionsEsquelet(double et)
{
	if (static_cast<int>(m_cossos.size()) < NUM_COSSOS_REALISTA) return;

	for (int i = 0; i < NUM_COSSOS_REALISTA; i++)
	{
		const CosRealMetadata& meta = kMetadadesReals[i];
		if (i == 0)
		{
			m_cossos[i].m_posicio = glm::vec3(0.0f);
			m_cossos[i].m_angleOrbita = 0.0f;
		}
		else
		{
			double periodeSegons = meta.periodeOrbitalDies * 86400.0;
			double angRad = (periodeSegons > 0.0) ? ((2.0 * PI * et) / periodeSegons) : 0.0;
			glm::vec3 posRel = calculaPosicioOrbital3D(meta.semieixMajorUA, meta.inclinacioOrbitalGraus, angRad, SCALE_UNITS_PER_AU);

			if (meta.spiceId == CB_MOON)
			{
				// La Lluna orbita al voltant de la Terra (index 3)
				m_cossos[i].m_posicio = m_cossos[3].m_posicio + posRel;
			}
			else
			{
				m_cossos[i].m_posicio = posRel;
			}

			double angGraus = std::fmod(glm::degrees(angRad), 360.0);
			if (angGraus < 0.0) angGraus += 360.0;
			m_cossos[i].m_angleOrbita = static_cast<float>(angGraus);
		}

		if (std::fabs(meta.periodeRotacioHores) > 1e-4)
		{
			double rotSegons = meta.periodeRotacioHores * 3600.0;
			double rotGraus = std::fmod((360.0 * et) / rotSegons, 360.0);
			if (rotGraus < 0.0) rotGraus += 360.0;
			m_cossos[i].m_angleRotacio = static_cast<float>(rotGraus);
		}
		else
		{
			m_cossos[i].m_angleRotacio = 0.0f;
		}
	}
}

void EscenariRealista::creaEscenari(GLuint& shader_programID)
{
	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;

	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);

	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5));

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
		m_cossos[i].m_radiCos = static_cast<float>(meta.radiEquatorialKm * KM_TO_AU * SCALE_UNITS_PER_AU);
		m_cossos[i].m_radiOrbita = static_cast<float>(meta.semieixMajorUA * SCALE_UNITS_PER_AU);
		m_cossos[i].m_angleOrbita = 0.0f;
		m_cossos[i].m_angleRotacio = 0.0f;
		m_cossos[i].m_velAngular = 0.0f;
		m_cossos[i].m_velRotacio = 0.0f;
	}

	// Esquelet de Fase 4: els kernels CSPICE complets es connectaran a la Fase 5
	m_kernelsCarregats = false;
	m_ephemerisTimeET = 0.0;
	actualitzaPosicionsEsquelet(m_ephemerisTimeET);
	generaOrbitesLineStrip();
}

void EscenariRealista::updateVariables(double dt, EstatSimulacio& estat)
{
	(void)dt;
	if (m_cossos.empty()) return;

	m_ephemerisTimeET = calculaSegonsDesDeJ2000(estat.tempsSimulacio.dataHora);
	actualitzaPosicionsEsquelet(m_ephemerisTimeET);

	if (estat.recalculEfemeridesNecessari)
	{
		estat.recalculEfemeridesNecessari = false;
	}
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

	// 1. Orbites 3D amb GL_LINE_STRIP
	if (estat.opcionsVisualitzacio.mostrarOrbites)
	{
		glUniform1i(locTextur, GL_FALSE);
		bool sw_mat_orbita[5] = { true, false, false, false, true };

		for (const OrbitaLineVAO& orb : m_orbitesVAO)
		{
			if (orb.vaoId == 0 || orb.nVertexs <= 0) continue;

			CColor col_orbita{ orb.color.r, orb.color.g, orb.color.b, 1.0 };
			SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat_orbita);

			glm::mat4 ModelMatrix = MatriuTG;
			if (orb.cosPareIdx > 0 && orb.cosPareIdx < static_cast<int>(m_cossos.size()))
			{
				ModelMatrix = glm::translate(MatriuTG, m_cossos[orb.cosPareIdx].m_posicio);
			}
			glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelMatrix[0][0]);

			glBindVertexArray(orb.vaoId);
			glDrawArrays(GL_LINE_STRIP, 0, orb.nVertexs);
			glBindVertexArray(0);
		}
	}

	// 2. Anells de Saturn escalats proporcionalment al radi real de Saturn (index 6)
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

	float escalaAnellSaturn = m_cossos[6].m_radiCos / 7.5f;
	glm::mat4 ModelAnell = glm::translate(MatriuTG, m_cossos[6].m_posicio);
	ModelAnell = glm::rotate(ModelAnell, glm::radians(static_cast<float>(kMetadadesReals[6].oblicuitatGraus)), vec3(1.0f, 0.0f, 0.0f));
	ModelAnell = glm::scale(ModelAnell, vec3(escalaAnellSaturn, escalaAnellSaturn, escalaAnellSaturn));
	glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelAnell[0][0]);

	GLint locTwoSided = glGetUniformLocation(sh_programID, "sw_twoSided");
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_TriEBO_Object(GLU_DISK);
	glDisable(GL_BLEND);
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_FALSE);

	// 3. Cossos celestes (Sol, 8 planetes, Pluto i Lluna)
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

		glm::mat4 ModelMatrix = glm::translate(MatriuTG, m_cossos[i].m_posicio);
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(static_cast<float>(kMetadadesReals[i].oblicuitatGraus)), vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_cossos[i].m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_cossos[i].m_radiCos, m_cossos[i].m_radiCos, m_cossos[i].m_radiCos));

		glUniformMatrix4fv(locModelMat, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform4i(locSwInt, (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
		draw_TriEBO_Object(GLU_SPHERE);
	}

	glUniform4i(locSwInt, sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(locTextur, GL_FALSE);
}
