#include "escenari.h"
//#include "main.h"

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

	

	glUseProgram(shader_programID);

	netejaVAOList();
	SetColor4d(m_colObj.r, m_colObj.g, m_colObj.b, m_colObj.a);
	Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	Set_VAOList(GLU_DISK, loadgluDisk_EBO(8.0f, 12.0f, 80, 5)); //anell saturn

	for (int i = 0; i < NUM_MAX_TEXTURES; i++)
		if (m_texturesID[i]) { glDeleteTextures(1, &m_texturesID[i]); m_texturesID[i] = 0; }

	//canviar al añadir una textura
	for (int i = 0; i < 10; i++)
	{
		m_texturesID[i] = loadIMA_SOIL(m_fitxers[i]);
		if (m_texturesID[i] == 0) fprintf(stderr, "No s'ha pogut carregar: %s\n", m_fitxers[i]);
	}

	glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), false);

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
	m_objectList[0].m_velAngular = 0.0f;  // Sol
	m_objectList[1].m_velAngular = 0.12f; // Mercurio
	m_objectList[2].m_velAngular = 0.09f; // Venus
	m_objectList[3].m_velAngular = 0.075f;// Tierra
	m_objectList[4].m_velAngular = 0.06f; // Marte
	m_objectList[5].m_velAngular = 0.035f;// Júpiter
	m_objectList[6].m_velAngular = 0.025f;// Saturno
	m_objectList[7].m_velAngular = 0.018f;// Urano
	m_objectList[8].m_velAngular = 0.012f;// Neptuno

	// Velocitats de rotació sobre el propi eix
	m_objectList[0].m_velRotacio = 0.0f;  // Sol
	m_objectList[1].m_velRotacio = 0.25f; // Mercurio
	m_objectList[2].m_velRotacio = 0.15f; // Venus
	m_objectList[3].m_velRotacio = 0.8f;  // Tierra
	m_objectList[4].m_velRotacio = 0.7f;  // Marte
	m_objectList[5].m_velRotacio = 1.0f;  // Júpiter
	m_objectList[6].m_velRotacio = 0.9f;  // Saturno
	m_objectList[7].m_velRotacio = 0.5f;  // Urano
	m_objectList[8].m_velRotacio = 0.6f;  // Neptuno
}

void Escenari::dibuixaEscenari(GLuint& sh_programID, bool& eix, GLuint& axis_Id, CMask3D& reixa, CPunt3D& hreixa, char& objecte,
	CColor& col_object, bool sw_mat[5],
	bool& textur, bool& textur_map, bool& flagInvertY,
	int& nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat& pasCS, bool& sw_PC, bool& dib_TFrenet,
	COBJModel* objecteOBJ,
	//glm::mat4 MatriuVista, 
	glm::mat4& MatriuTG)
{

	CColor col_blanc, col_orbita;
	col_blanc.r = 1.0;	col_blanc.g = 1.0;	col_blanc.b = 1.0;	col_blanc.a = 1.0;
	col_orbita.r = 0.7;	col_orbita.g = 0.7;	col_orbita.b = 0.7;	col_orbita.a = 1.0;

	glm::mat4 ModelMatrix(1.0);
	int i;

	// 1. Òrbites: anell fi sense textura, escalat al radi de cada òrbita
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
	SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat);
	for (i = 1; i < 9; i++)
	{
		ModelMatrix = glm::scale(MatriuTG, vec3(m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita, m_objectList[i].m_radiOrbita)); //€~€~€~€~€
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		CVAO& orbita = m_orbitVAO[i - 1];
		if (orbita.vaoId != 0)
		{
			glBindVertexArray(orbita.vaoId);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbita.eboId);
			glDrawElements(GL_TRIANGLES,orbita.nIndices,GL_UNSIGNED_INT,nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
	}
	//2.Anell saturn
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
		SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);

		if (m_texturesID[9] != 0) SetTextureParameters(0, m_texturesID[9], true, true, textur_map, false);

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
	SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);
	for (i = 0; i < 9; i++)
	{
		if (m_texturesID[i] != 0) SetTextureParameters(0, m_texturesID[i], true, true, textur_map, false);

		float x = m_objectList[i].m_radiOrbita * (float)cos(m_objectList[i].m_angleOrbita * PI / 180.0); //€~€~€~€~€
		float y = m_objectList[i].m_radiOrbita * (float)sin(m_objectList[i].m_angleOrbita * PI / 180.0); //€~€~€~€~€

		ModelMatrix = glm::translate(MatriuTG, vec3(x, y, 0.0f));

		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(m_objectList[i].m_angleRotacio), vec3(0.0f, 0.0f, 1.0f));

		ModelMatrix = glm::scale(ModelMatrix, vec3(m_objectList[i].m_radiCos, m_objectList[i].m_radiCos, m_objectList[i].m_radiCos)); //€~€~€~€~€

		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		// El Sol brilla amb llum pròpia (emissió)
		glUniform1i(glGetUniformLocation(sh_programID, "sw_intensity[0]"), (i == 0));
		draw_TriEBO_Object(GLU_SPHERE);
	}

	glUniform1i(glGetUniformLocation(sh_programID, "sw_intensity[0]"), sw_mat[0]);
}

void Escenari::updateVariables()
{
	if (m_objectList == nullptr) return;
	for (int i = 0; i < 9; i++)
	{
		//gir sobre punt de origen (sol)
		m_objectList[i].m_angleOrbita = m_objectList[i].m_angleOrbita + m_objectList[i].m_velAngular;
		if (m_objectList[i].m_angleOrbita > 360) m_objectList[i].m_angleOrbita -= 360;

		//gir sobre mateix cos
		m_objectList[i].m_angleRotacio += m_objectList[i].m_velRotacio;
		if (m_objectList[i].m_angleRotacio >= 360.0f) m_objectList[i].m_angleRotacio -= 360.0f;
	}


}
