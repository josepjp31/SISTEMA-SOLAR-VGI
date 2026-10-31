#include "escenari.h"
#include <cmath>

// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005). El shader es rep com a parametre.

void Escenari::creaEscenari(GLuint& shader_programID)
{
	m_fitxers[0] = (char*)".\\textures\\sun\\2k_sun.jpg";					// 0: Sol
	m_fitxers[1] = (char*)".\\textures\\planet\\Mercury\\2k_mercury.jpg";	// 1: Mercuri
	m_fitxers[2] = (char*)".\\textures\\planet\\Venus\\2k_venus_surface.jpg"; // 2: Venus
	m_fitxers[3] = (char*)".\\textures\\planet\\Earth\\2k_earth_daymap.jpg";  // 3: Terra
	m_fitxers[4] = (char*)".\\textures\\planet\\Mars\\2k_mars.jpg";			// 4: Mart
	m_fitxers[5] = (char*)".\\textures\\planet\\Jupiter\\2k_jupiter.jpg";	// 5: Jupiter
	m_fitxers[6] = (char*)".\\textures\\planet\\Saturn\\2k_saturn.jpg";		// 6: Saturn
	m_fitxers[7] = (char*)".\\textures\\planet\\Uranus\\2k_uranus.jpg";		// 7: Ura
	m_fitxers[8] = (char*)".\\textures\\planet\\Neptune\\2k_neptune.jpg";	// 8: Neptu

	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;

	glUseProgram(shader_programID);

	netejaVAOList();
	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120));	// Anell fi de radi 1 (orbites)

	for (int i = 0; i < NUM_MAX_TEXTURES; i++)
		if (m_texturesID[i]) { glDeleteTextures(1, &m_texturesID[i]); m_texturesID[i] = 0; }
	for (int i = 0; i < 9; i++)
	{
		m_texturesID[i] = loadIMA_SOIL(m_fitxers[i]);
		if (m_texturesID[i] == 0) fprintf(stderr, "No s'ha pogut carregar: %s\n", m_fitxers[i]);
	}

	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), GL_TRUE);

	// Propietats astronomiques d'escala didactica
	const float radiCos[9]     = { 20.0f,  3.0f,  5.0f,  5.0f,  4.0f, 12.0f, 10.0f,  7.0f,  7.0f };
	const float radiOrbita[9]  = {  0.0f, 40.0f, 60.0f, 80.0f,100.0f,135.0f,175.0f,210.0f,240.0f };
	const float angleOrbita[9] = {  0.0f,  0.0f, 40.0f, 80.0f,120.0f,160.0f,200.0f,240.0f,280.0f };

	for (int i = 0; i < 9; i++)
	{
		m_objectList[i].m_radiCos = radiCos[i];
		m_objectList[i].m_radiOrbita = radiOrbita[i];
		m_objectList[i].m_angleOrbita = angleOrbita[i];
	}

	// Velocitats angulars orbitals en graus per segon (molt mes suaus i proporcionals a la distancia al Sol)
	m_objectList[0].m_velAngular =  0.0f; // Sol (estatic al centre)
	m_objectList[1].m_velAngular = 12.0f; // Mercuri
	m_objectList[2].m_velAngular =  8.5f; // Venus
	m_objectList[3].m_velAngular =  6.0f; // Terra
	m_objectList[4].m_velAngular =  4.5f; // Mart
	m_objectList[5].m_velAngular =  2.5f; // Jupiter
	m_objectList[6].m_velAngular =  1.6f; // Saturn
	m_objectList[7].m_velAngular =  1.0f; // Ura
	m_objectList[8].m_velAngular =  0.6f; // Neptu
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
	if (mostrarOrbites)
	{
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat);
		for (i = 1; i < 9; i++)
		{
			ModelMatrix = glm::scale(MatriuTG, vec3(m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_TORUS);
		}
	}

	// 2. Cossos: esfera amb la seva textura, situada sobre la seva orbita
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), flagInvertY);
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

	for (i = 0; i < 9; i++)
	{
		if (m_texturesID[i] != 0) SetTextureParameters(0, m_texturesID[i], true, true, textur_map, false);

		float x = m_objectList[i].m_radiOrbita * (float)cos(m_objectList[i].m_angleOrbita * PI / 180.0);
		float y = m_objectList[i].m_radiOrbita * (float)sin(m_objectList[i].m_angleOrbita * PI / 180.0);
		ModelMatrix = glm::translate(MatriuTG, vec3(x, y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(m_objectList[i].m_radiCos, m_objectList[i].m_radiCos, m_objectList[i].m_radiCos));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		// El Sol (i == 0) brilla amb llum propia (emissio activa si sw_mat[0] es true);
		// els planetes (i > 0) no emeten llum propia i reben nomes la llum difusa/ambient del Sol.
		glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), (i == 0 && sw_mat[0]), sw_mat[1], sw_mat[2], sw_mat[3]);
		draw_TriEBO_Object(GLU_SPHERE);
	}

	// Restaurar l'estat dels uniforms de material i textura
	glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), textur);
}

void Escenari::updateVariables(double dt, bool enPausa, int direccioTemps, bool escalaReal, double factorVelocitat)
{
	if (m_objectList == nullptr || enPausa) return;

	// Limit de seguretat per a dt en cas de bloqueig temporal de finestra
	if (dt < 0.0) dt = 0.0;
	if (dt > 0.1) dt = 0.1;

	float factorTemps = 1.0f;
	if (escalaReal)
	{
		// Escalat progressiu segons el selector de timelapse (60 s/s = 1 min/s fins a 1728000 s/s = 20 dies/s)
		factorTemps = (float)std::sqrt(factorVelocitat / 300.0);
	}

	for (int i = 1; i < 9; i++)
	{
		float increment = m_objectList[i].m_velAngular * (float)dt * (float)direccioTemps * factorTemps;
		m_objectList[i].m_angleOrbita += increment;
		while (m_objectList[i].m_angleOrbita >= 360.0f) m_objectList[i].m_angleOrbita -= 360.0f;
		while (m_objectList[i].m_angleOrbita < 0.0f)    m_objectList[i].m_angleOrbita += 360.0f;
	}
}
