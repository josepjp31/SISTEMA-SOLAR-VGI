#include "EscenariIntuitiu.h"
#include "InterficieUI.h"
#include <cmath>

EscenariIntuitiu::EscenariIntuitiu()
{
	for (int i = 0; i < NUM_ORBITES_INTUITIU; i++)
	{
		m_orbitVAO[i] = { 0, 0, 0, 0, 0 };
	}

	// Parametres de camera adaptats al Mode Didactic / Intuitiu
	m_configCamera.distInicialSol = 140.0f;
	m_configCamera.angleInclinacioInicial = 25.0f;
	m_configCamera.globalSistemaR = 550.0f;
	m_configCamera.angleGlobal = 65.0f;
	m_configCamera.minDistSol = 42.0f;
	m_configCamera.minDistSolFixa = true;
	m_configCamera.maxR = 800.0f;
	m_configCamera.factorFocusPlaneta = 4.5f;
	m_configCamera.minDistFocusPlaneta = 14.0f;
	m_configCamera.minDistFocusZoom = 2.5f;
	m_configCamera.factorRadiMinZoom = 2.2f;
	m_configCamera.pasZoomSol = 5.0f;
	m_configCamera.minPasZoomPlaneta = 1.0f;
	m_configCamera.factorPasZoomPlaneta = 0.5f;
	m_configCamera.zoomLogaritmic = false;
	m_configCamera.navPosX = 110.0f;
	m_configCamera.navPosY = -110.0f;
	m_configCamera.navPosZ = 30.0f;
	m_configCamera.navPas = 2.5f;
	m_configCamera.atenuacioQuad = 0.00004f;
}

EscenariIntuitiu::~EscenariIntuitiu()
{
	alliberaEscenari();
}

void EscenariIntuitiu::alliberaOrbitesTorus()
{
	for (int i = 0; i < NUM_ORBITES_INTUITIU; i++)
	{
		if (m_orbitVAO[i].vaoId != 0)
		{
			glDeleteVertexArrays(1, &m_orbitVAO[i].vaoId);
			glDeleteBuffers(1, &m_orbitVAO[i].vboId);
			glDeleteBuffers(1, &m_orbitVAO[i].eboId);
			m_orbitVAO[i] = { 0, 0, 0, 0, 0 };
		}
	}
}

void EscenariIntuitiu::alliberaEscenari()
{
	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	alliberaOrbitesTorus();
	Escenari::alliberaEscenari();
}

void EscenariIntuitiu::creaEscenari(GLuint& shader_programID)
{
	static const char* kRutesTextures[10] = {
		".\\textures\\sun\\2k_sun.jpg",
		".\\textures\\planet\\Mercury\\2k_mercury.jpg",
		".\\textures\\planet\\Venus\\2k_venus_surface.jpg",
		".\\textures\\planet\\Earth\\2k_earth_daymap.jpg",
		".\\textures\\planet\\Mars\\2k_mars.jpg",
		".\\textures\\planet\\Jupiter\\2k_jupiter.jpg",
		".\\textures\\planet\\Saturn\\2k_saturn.jpg",
		".\\textures\\planet\\Uranus\\2k_uranus.jpg",
		".\\textures\\planet\\Neptune\\2k_neptune.jpg",
		".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png"
	};

	static const char* kNomsCossos[NUM_COSSOS_INTUITIU] = {
		"Sol", "Mercuri", "Venus", "Terra", "Mart", "Júpiter", "Saturn", "Urà", "Neptú"
	};

	static const float kRadiCos[NUM_COSSOS_INTUITIU] = {
		16.0f,  // Sol
		2.0f,   // Mercuri
		3.5f,   // Venus
		3.7f,   // Terra
		2.8f,   // Mart
		9.0f,   // Jupiter
		7.5f,   // Saturn
		5.5f,   // Ura
		5.3f    // Neptu
	};

	static const float kRadiOrbita[NUM_COSSOS_INTUITIU] = {
		0.0f,   // Sol
		27.0f,  // Mercuri
		40.0f,  // Venus
		55.0f,  // Terra
		70.0f,  // Mart
		105.0f, // Jupiter
		140.0f, // Saturn
		175.0f, // Ura
		210.0f  // Neptu
	};

	static const float kAngleInicial[NUM_COSSOS_INTUITIU] = {
		0.0f, 0.0f, 40.0f, 80.0f, 120.0f, 160.0f, 200.0f, 240.0f, 280.0f
	};

	static const float kPeriodesOrbitalsDies[NUM_COSSOS_INTUITIU] = {
		0.0f, 88.0f, 225.0f, 365.0f, 687.0f, 4333.0f, 10756.0f, 30687.0f, 60190.0f
	};

	static const float kVelRotacioBase[NUM_COSSOS_INTUITIU] = {
		0.0f, 6.136f, -1.481f, 361.015f, 351.220f, 871.249f, 818.182f, -501.393f, 545.455f
	};

	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;

	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	alliberaOrbitesTorus();

	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));              // Esfera de radi 1 (cossos)
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5));          // Anell de Saturn
	Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120)); // Anell fi de radi 1 (orbites)

	alliberaTextures();
	for (int i = 0; i < 10; i++)
	{
		m_texturesID[i] = carregaTextura2D(kRutesTextures[i]);
	}

	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	m_cossos.clear();
	m_cossos.resize(NUM_COSSOS_INTUITIU);

	const float factorVelocitatOrbital = 3650.0f;
	for (int i = 0; i < NUM_COSSOS_INTUITIU; i++)
	{
		m_cossos[i].m_nom = kNomsCossos[i];
		m_cossos[i].m_radiCos = kRadiCos[i];
		m_cossos[i].m_radiOrbita = kRadiOrbita[i];
		m_cossos[i].m_angleOrbita = kAngleInicial[i];
		m_cossos[i].m_angleRotacio = 0.0f;
		m_cossos[i].m_velAngular = (i > 0 && kPeriodesOrbitalsDies[i] > 0.0f)
			? (factorVelocitatOrbital / kPeriodesOrbitalsDies[i])
			: 0.0f;
		m_cossos[i].m_velRotacio = kVelRotacioBase[i];

		float angRad = m_cossos[i].m_angleOrbita * static_cast<float>(PI) / 180.0f;
		m_cossos[i].m_posicio = glm::vec3(
			m_cossos[i].m_radiOrbita * std::cos(angRad),
			m_cossos[i].m_radiOrbita * std::sin(angRad),
			0.0f
		);
	}

	// VAOs toroidals per a les 8 orbites circulars
	const float gruixOrbita = 0.03f;
	for (int i = 1; i < NUM_COSSOS_INTUITIU; i++)
	{
		float radi = m_cossos[i].m_radiOrbita;
		m_orbitVAO[i - 1] = loadglutSolidTorus_EBO(gruixOrbita / radi, 1.0f, 8, 120);
	}
}

void EscenariIntuitiu::updateVariables(double dt, EstatSimulacio& estat)
{
	if (m_cossos.empty() || estat.tempsSimulacio.enPausa) return;

	if (dt < 0.0) dt = 0.0;
	if (dt > 0.1) dt = 0.1;

	const float direccio = static_cast<float>(estat.tempsSimulacio.direccioTemps);
	const float fdt = static_cast<float>(dt);

	for (int i = 0; i < static_cast<int>(m_cossos.size()); i++)
	{
		if (i > 0)
		{
			m_cossos[i].m_angleOrbita += m_cossos[i].m_velAngular * fdt * direccio;
			while (m_cossos[i].m_angleOrbita >= 360.0f) m_cossos[i].m_angleOrbita -= 360.0f;
			while (m_cossos[i].m_angleOrbita < 0.0f) m_cossos[i].m_angleOrbita += 360.0f;
		}

		m_cossos[i].m_angleRotacio += m_cossos[i].m_velRotacio * fdt * direccio;
		while (m_cossos[i].m_angleRotacio >= 360.0f) m_cossos[i].m_angleRotacio -= 360.0f;
		while (m_cossos[i].m_angleRotacio < 0.0f) m_cossos[i].m_angleRotacio += 360.0f;

		float angRad = m_cossos[i].m_angleOrbita * static_cast<float>(PI) / 180.0f;
		m_cossos[i].m_posicio = glm::vec3(
			m_cossos[i].m_radiOrbita * std::cos(angRad),
			m_cossos[i].m_radiOrbita * std::sin(angRad),
			0.0f
		);
	}
}

void EscenariIntuitiu::dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat)
{
	(void)MatriuVista;
	if (m_cossos.size() < NUM_COSSOS_INTUITIU) return;

	CColor col_blanc, col_orbita;
	col_blanc.r = 1.0;  col_blanc.g = 1.0;  col_blanc.b = 1.0;  col_blanc.a = 1.0;
	col_orbita.r = 0.7; col_orbita.g = 0.7; col_orbita.b = 0.7; col_orbita.a = 1.0;

	glm::mat4 ModelMatrix(1.0f);
	bool sw_mat_no_emisio[5] = { false, sw_mat[1], sw_mat[2], sw_mat[3], sw_mat[4] };

	// 1. Orbites toroidals (condicionat a mostrarOrbites)
	if (estat.opcionsVisualitzacio.mostrarOrbites)
	{
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat_no_emisio);
		GLint locModelMatOrb = glGetUniformLocation(sh_programID, "modelMatrix");
		for (int i = 1; i < NUM_COSSOS_INTUITIU; i++)
		{
			float rOrb = m_cossos[i].m_radiOrbita;
			ModelMatrix = glm::scale(MatriuTG, vec3(rOrb, rOrb, rOrb));
			glUniformMatrix4fv(locModelMatOrb, 1, GL_FALSE, &ModelMatrix[0][0]);
			CVAO& orbita = m_orbitVAO[i - 1];
			if (orbita.vaoId != 0)
			{
				glBindVertexArray(orbita.vaoId);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbita.eboId);
				glDrawElements(GL_TRIANGLES, orbita.nIndices, GL_UNSIGNED_INT, nullptr);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}
		}
	}

	// 2. Cossos: esfera amb textura situada sobre la seva orbita
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	GLint locModelMatCos = glGetUniformLocation(sh_programID, "modelMatrix");
	GLint locSwInt = glGetUniformLocation(sh_programID, "sw_intensity");

	for (int i = 0; i < NUM_COSSOS_INTUITIU; i++)
	{
		if (m_texturesID[i] != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
		}

		ModelMatrix = glm::translate(MatriuTG, m_cossos[i].m_posicio);
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_cossos[i].m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_cossos[i].m_radiCos, m_cossos[i].m_radiCos, m_cossos[i].m_radiCos));

		glUniformMatrix4fv(locModelMatCos, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform4i(locSwInt, (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
		draw_TriEBO_Object(GLU_SPHERE);
	}

	// 3. Anell de Saturn (despres de les esferes per oclusio correcta i sense escriure al Z-buffer)
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat_no_emisio);

	if (m_texturesID[9] != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texturesID[9]);
	}

	ModelMatrix = glm::translate(MatriuTG, m_cossos[6].m_posicio);
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(locModelMatCos, 1, GL_FALSE, &ModelMatrix[0][0]);

	GLint locTwoSided = glGetUniformLocation(sh_programID, "sw_twoSided");
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	draw_TriEBO_Object(GLU_DISK);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	if (locTwoSided != -1) glUniform1i(locTwoSided, GL_FALSE);

	// Restaurar estat de uniforms
	glUniform4i(locSwInt, sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
}
