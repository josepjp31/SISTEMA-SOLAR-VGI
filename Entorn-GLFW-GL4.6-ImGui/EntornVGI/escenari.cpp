#include "escenari.h"
#include <cmath>

// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005). El shader es rep com a parametre.

void Escenari::creaEscenari(GLuint& shader_programID)
{

	m_fitxers[0] = (char*)".\\textures\\sun\\\\2k_sun.jpg";
	m_fitxers[1] = (char*)".\\textures\\planet\\Mercury\\2k_mercury.jpg";	// 1
	m_fitxers[2] = (char*)".\\textures\\planet\\Venus\\2k_venus_surface.jpg";		// 2
	m_fitxers[3] = (char*)".\\textures\\planet\\Earth\\2k_earth_daymap.jpg";		// 3
	m_fitxers[4] = (char*)".\\textures\\planet\\Mars\\2k_mars.jpg";		// 4
	m_fitxers[5] = (char*)".\\textures\\planet\\Jupiter\\2k_jupiter.jpg";	// 5
	m_fitxers[6] = (char*)".\\textures\\planet\\Saturn\\2k_saturn.jpg";	// 6
	m_fitxers[7] = (char*)".\\textures\\planet\\Uranus\\2k_uranus.jpg";	// 7
	m_fitxers[8] = (char*)".\\textures\\planet\\Neptune\\2k_neptune.jpg";		// 8
	m_fitxers[9] = (char*)".\\textures\\planet\\Saturn\\2k_saturn_ring_alpha.png";	// 6

	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;


	glUseProgram(shader_programID);

	deleteVAOList(GLU_SPHERE);
	deleteVAOList(GLU_DISK);
	deleteVAOList(GLUT_TORUS);
	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5)); //anell saturn
	Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120));	// Anell fi de radi 1 (orbites)


	for (int i = 0; i < NUM_MAX_TEXTURES; i++)
		if (m_texturesID[i]) { glDeleteTextures(1, &m_texturesID[i]); m_texturesID[i] = 0; }

	// Carregar textures i configurar parametres de filtratge i repeticio un sol cop
	for (int i = 0; i < 10; i++)
	{
		m_texturesID[i] = loadIMA_SOIL(m_fitxers[i]);
		if (m_texturesID[i] == 0) {
			fprintf(stderr, "No s'ha pogut carregar: %s\n", m_fitxers[i]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	//Initialize class variables
	const float radiCos[9] = {
		16.0f,  // Sol
		2.0f,   // Mercurio
		3.5f,   // Venus
		3.7f,   // Tierra
		2.8f,   // Marte
		9.0f,   // Júpiter
		7.5f,   // Saturno
		5.5f,   // Urano
		5.3f    // Neptuno
	};

	const float radiOrbita[9] = {
		0.0f,   // Sol
		27.0f,  // Mercurio
		40.0f,  // Venus
		55.0f,  // Tierra
		70.0f,  // Marte
		105.0f, // Júpiter
		140.0f, // Saturno
		175.0f, // Urano
		210.0f  // Neptuno
	};

	const float angleOrbita[9] = { 0.0f,  0.0f, 40.0f, 80.0f,120.0f,160.0f,200.0f,240.0f,280.0f };

	for (int i = 0; i < 9; i++)
	{
		m_objectList[i].m_radiCos = radiCos[i];
		m_objectList[i].m_radiOrbita = radiOrbita[i];
		m_objectList[i].m_angleOrbita = angleOrbita[i];
	}

	//torus
	const float gruixOrbita = 0.03f;
	for (int i = 1; i < 9; i++)
	{
		float radi = m_objectList[i].m_radiOrbita;
		m_orbitVAO[i - 1] = loadglutSolidTorus_EBO(gruixOrbita / radi,1.0f,8,120);
	}

	for (int i = 0; i < 9; i++)
	{
		m_objectList[i].m_angleRotacio = 0.0f;
	}

	//velocitats al voltant de sol
	float x = 3650;
	// Velocitats angulars orbitals en graus per segon (molt mes suaus i proporcionals a la distancia al Sol)
	m_objectList[0].m_velAngular = 0.0f; // Sol (estatic al centre)
	m_objectList[1].m_velAngular = x / 88.0f; // Mercuri
	m_objectList[2].m_velAngular = x / 225.0f; // Venus
	m_objectList[3].m_velAngular = x / 365.0f; // Terra
	m_objectList[4].m_velAngular = x / 687.0f; // Mart
	m_objectList[5].m_velAngular = x / 4333.0f; // Jupiter
	m_objectList[6].m_velAngular = x / 10756.0f; // Saturn
	m_objectList[7].m_velAngular = x / 30687.0f; // Ura
	m_objectList[8].m_velAngular = x / 60190.0f; // Neptu

	float factorRotacio = 1.0f;
	// Velocitats de rotació sobre el propi eix
	m_objectList[0].m_velRotacio = 0.0f;
	m_objectList[1].m_velRotacio = 6.136f * factorRotacio;     // Mercuri
	m_objectList[2].m_velRotacio = -1.481f * factorRotacio;    // Venus
	m_objectList[3].m_velRotacio = 361.015f * factorRotacio;   // Terra
	m_objectList[4].m_velRotacio = 351.220f * factorRotacio;   // Mart
	m_objectList[5].m_velRotacio = 871.249f * factorRotacio;   // Jupiter
	m_objectList[6].m_velRotacio = 818.182f * factorRotacio;   // Saturn
	m_objectList[7].m_velRotacio = -501.393f * factorRotacio;  // Urà
	m_objectList[8].m_velRotacio = 545.455f * factorRotacio;   // Neptú

}



void Escenari::dibuixaEscenari(GLuint& sh_programID, bool& eix, GLuint& axis_Id, CMask3D& reixa, CPunt3D& hreixa, char& objecte,
	CColor& col_object, bool sw_mat[5],
	bool& textur, bool& textur_map, bool& flagInvertY,
	int& nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat& pasCS, bool& sw_PC, bool& dib_TFrenet,
	COBJModel* objecteOBJ,
	glm::mat4& MatriuTG,
	bool mostrarOrbites)
{
	CColor col_blanc, col_orbita;
	col_blanc.r = 1.0;	col_blanc.g = 1.0;	col_blanc.b = 1.0;	col_blanc.a = 1.0;
	col_orbita.r = 0.7;	col_orbita.g = 0.7;	col_orbita.b = 0.7;	col_orbita.a = 1.0;

	glm::mat4 ModelMatrix(1.0);
	int i;

	// 1. Orbites: anell fi sense textura, escalat al radi de cada orbita (condicionat a mostrarOrbites)
	if (mostrarOrbites) {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat);
		GLint locModelMatOrb = glGetUniformLocation(sh_programID, "modelMatrix");
		for (i = 1; i < 9; i++)
		{
			ModelMatrix = glm::scale(MatriuTG, vec3(m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita));
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
	//2.Anell saturn
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
		SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

		if (m_texturesID[9] != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texturesID[9]);
		}

		float x = m_objectList[6].m_radiOrbita * (float)cos(m_objectList[6].m_angleOrbita * PI / 180.0f);

		float y = m_objectList[6].m_radiOrbita * (float)sin(m_objectList[6].m_angleOrbita * PI / 180.0f);

		ModelMatrix = glm::translate(MatriuTG, vec3(x, y, 0.0f));

		// Inclinar ligeramente los anillos
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), vec3(1.0f, 0.0f, 0.0f));

		// Escalar los anillos
		ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		draw_TriEBO_Object(GLU_DISK);

	// 3. Cossos: esfera amb la seva textura, situada sobre la seva òrbita

	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), flagInvertY);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	GLint locModelMatCos = glGetUniformLocation(sh_programID, "modelMatrix");
	GLint locSwInt = glGetUniformLocation(sh_programID, "sw_intensity");

	for (i = 0; i < 9; i++)
	{
		if (m_texturesID[i] != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
		}

		float x = m_objectList[i].m_radiOrbita * (float)cos(m_objectList[i].m_angleOrbita * PI / 180.0);
		float y = m_objectList[i].m_radiOrbita * (float)sin(m_objectList[i].m_angleOrbita * PI / 180.0);

		ModelMatrix = glm::translate(MatriuTG, vec3(x, y, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_objectList[i].m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_objectList[i].m_radiCos, m_objectList[i].m_radiCos, m_objectList[i].m_radiCos));

		glUniformMatrix4fv(locModelMatCos, 1, GL_FALSE, &ModelMatrix[0][0]);

		// El Sol (i == 0) brilla amb llum propia (emissio activa si sw_mat[0] es true);
		// els planetes (i > 0) no emeten llum propia i reben nomes la llum difusa/ambient del Sol.
		glUniform4i(locSwInt, (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
		draw_TriEBO_Object(GLU_SPHERE);
	}


	// Restaurar l'estat dels uniforms de material i textura
	glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), textur);

}

void Escenari::updateVariables(double dt, bool enPausa, int direccioTemps, bool escalaReal, double factorVelocitat)
{
	if (m_objectList == nullptr || enPausa) return;

	// Limit de seguretat per a dt
	if (dt < 0.0) dt = 0.0;
	if (dt > 0.1) dt = 0.1;

	// Factor de temps per al moviment orbital
	float factorTemps = 1.0f;
	if (escalaReal) factorTemps = (float)std::sqrt(factorVelocitat / 300.0);

	for (int i = 0; i < 9; i++)
	{
		// Gir orbital al voltant del Sol
		if (i > 0)
		{
			float incrementOrbita = m_objectList[i].m_velAngular * (float)dt * (float)direccioTemps * factorTemps;
			m_objectList[i].m_angleOrbita += incrementOrbita;
			while (m_objectList[i].m_angleOrbita >= 360.0f) m_objectList[i].m_angleOrbita -= 360.0f;
			while (m_objectList[i].m_angleOrbita < 0.0f) m_objectList[i].m_angleOrbita += 360.0f;
		}

		m_objectList[i].m_angleRotacio += m_objectList[i].m_velRotacio * (float)dt * (float)direccioTemps;
		while (m_objectList[i].m_angleRotacio >= 360.0f) m_objectList[i].m_angleRotacio -= 360.0f;
		while (m_objectList[i].m_angleRotacio < 0.0f) m_objectList[i].m_angleRotacio += 360.0f;
	}
}


