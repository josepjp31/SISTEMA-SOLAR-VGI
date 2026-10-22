#include "escenari.h"
//#include "main.h"

// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005). El shader es rep com a parametre.

void Escenari::creaEscenari()
{
	static char* fitxers[9] = {
		(char*)".\\textures\\sun\\\\2k_sun.jpg",
		(char*)".\\textures\\planet\\Mercury\\2k_mercury.jpg",	// 1
		(char*)".\\textures\\planet\\Venus\\2k_venus_surface.jpg",		// 2
		(char*)".\\textures\\planet\\Earth\\2k_earth_daymap.jpg",		// 3
		(char*)".\\textures\\planet\\Mars\\2k_mars.jpg",		// 4
		(char*)".\\textures\\planet\\Jupiter\\2k_jupiter.jpg",	// 5
		(char*)".\\textures\\planet\\Saturn\\2k_saturn.jpg",	// 6
		(char*)".\\textures\\planet\\Uranus\\2k_uranus.jpg",	// 7
		(char*)".\\textures\\planet\\Neptune\\2k_neptune.jpg"		// 8
	};

	//OnProjeccioPerspectiva();	oProjeccio = shortCut_Projeccio();
	//OnIluminacioSuau();			oIlumina = shortCut_Iluminacio();

	//objecte = SISTEMA_SOLAR;	cosSolar = -1;
	//camera = CAM_ESFERICA;		Vis_Polar = POLARZ;
	//t_textura = TEXTURA_FITXERIMA;	tFlag_invert_Y = true;	textura = true;
	//sw_material[0] = false;

	//// Càmera allunyada i enlairada per veure les òrbites des de dalt
	//OPV.R = 550.0;	OPV.alfa = 65.0;	OPV.beta = 0.0;
	//cam_Esferica[0] = (float)OPV.R;	cam_Esferica[1] = (float)OPV.alfa;	cam_Esferica[2] = (float)OPV.beta;

	//glUseProgram(shader_programID);

	//netejaVAOList();
	//SetColor4d(col_obj.r, col_obj.g, col_obj.b, col_obj.a);
	////Set_VAOList(GLU_DISK, loadgluDisk_EBO());
	//Set_VAOList(GLU_SPHERE, loadgluSphere_EBO(1.0, 40, 40));				// Esfera de radi 1 (cossos)
	//Set_VAOList(GLUT_TORUS, loadglutSolidTorus_EBO(0.005, 1.0, 8, 120));		// Anell fi de radi 1 (òrbites)

	//for (int i = 0; i < NUM_MAX_TEXTURES; i++)
	//	if (texturesID[i]) { glDeleteTextures(1, &texturesID[i]); texturesID[i] = 0; }
	//for (int i = 0; i < 9; i++)
	//{
	//	texturesID[i] = loadIMA_SOIL(fitxers[i]);
	//	if (texturesID[i] == 0) fprintf(stderr, "No s'ha pogut carregar: %s\n", fitxers[i]);
	//}

	//glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
	//glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), tFlag_invert_Y);

	m_objectList[0].m_velAngular = 1.0f;
	m_objectList[1].m_velAngular = 0.8f;
	m_objectList[2].m_velAngular = 0.2f;
	m_objectList[3].m_velAngular = 1.1f;
	m_objectList[4].m_velAngular = 1.0f;
	m_objectList[5].m_velAngular = 1.2f;
	m_objectList[6].m_velAngular = 0.5f;
	m_objectList[7].m_velAngular = 0.1f;
	m_objectList[8].m_velAngular = 2.0f;
}

void Escenari::dibuixaEscenari(GLuint sh_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, char objecte,
	CColor col_object, bool sw_mat[5],
	bool textur, GLuint texturID[NUM_MAX_TEXTURES], bool textur_map, bool flagInvertY,
	int nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat pasCS, bool sw_PC, bool dib_TFrenet,
	COBJModel* objecteOBJ,
	//glm::mat4 MatriuVista, 
	glm::mat4 MatriuTG)
{
	//if (m_objectList == nullptr) return;

	//// Nomes es dibuixa si l'objecte te un model .OBJ carregat
	//if (!m_objectList[0].teModel()) return;

	////Le hago cosas al model matrix con los (ahora is Identidad)

	//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &m_objectList[0].m_mMatrix[0][0]);

	//m_objectList[0].m_model.draw_TriVAO_OBJ(sh_programID);

	//                              Sol    Merc   Venus  Terra  Mart   Júpit  Saturn Urà    Neptú
	const float radiCos[9] = { 20.0f,  3.0f,  5.0f,  5.0f,  4.0f, 12.0f, 10.0f,  7.0f,  7.0f };
	const float radiOrbita[9] = { 0.0f, 40.0f, 60.0f, 80.0f,100.0f,135.0f,175.0f,210.0f,240.0f };
	const float angleOrbita[9] = { 0.0f,  0.0f, 40.0f, 80.0f,120.0f,160.0f,200.0f,240.0f,280.0f };

	CColor col_blanc, col_orbita;
	col_blanc.r = 1.0;	col_blanc.g = 1.0;	col_blanc.b = 1.0;	col_blanc.a = 1.0;
	col_orbita.r = 0.7;	col_orbita.g = 0.7;	col_orbita.b = 0.7;	col_orbita.a = 1.0;

	glm::mat4 ModelMatrix(1.0);
	int i;

	//// 1. Òrbites: anell fi sense textura, escalat al radi de cada òrbita
	//glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
	//SeleccionaColorMaterial(sh_programID, col_orbita, sw_mat);
	//for (i = 1; i < 9; i++)
	//{
	//	ModelMatrix = glm::scale(MatriuTG, vec3(radiOrbita[i], radiOrbita[i], radiOrbita[i]));
	//	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	//	draw_TriEBO_Object(GLUT_TORUS);
	//}

	//// 2. Cossos: esfera amb la seva textura, situada sobre la seva òrbita
	//glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
	//SeleccionaColorMaterial(sh_programID, col_blanc, sw_mat);
	//for (i = 0; i < 9; i++)
	//{
	//	if (texturID[i] != 0) SetTextureParameters(0, texturID[i], true, true, textur_map, false);

	//	float x = radiOrbita[i] * (float)cos(angleOrbita[i] * PI / 180.0);
	//	float y = radiOrbita[i] * (float)sin(angleOrbita[i] * PI / 180.0);
	//	ModelMatrix = glm::translate(MatriuTG, vec3(x, y, 0.0f));
	//	ModelMatrix = glm::scale(ModelMatrix, vec3(radiCos[i], radiCos[i], radiCos[i]));
	//	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	//	// El Sol brilla amb llum pròpia (emissió)
	//	glUniform1i(glGetUniformLocation(sh_programID, "sw_intensity[0]"), (i == 0));
	//	draw_TriEBO_Object(GLU_SPHERE);
	//}
	//glUniform1i(glGetUniformLocation(sh_programID, "sw_intensity[0]"), sw_mat[0]);
}

void Escenari::updateVariables()
{
	for (int i = 0; i < 9; i++)
	{
		m_objectList[i].m_angleOrbita = m_objectList[i].m_angleOrbita + m_objectList[i].m_velAngular;
		if (m_objectList[i].m_angleOrbita > 360) m_objectList[i].m_angleOrbita -= 360;
	}
}
