//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// escena.cpp : Aqui es on ha d'anar el codi de les funcions que 
//              dibuixin les escenes.
//
//    Versió 2.0:	- Objectes Cub, Esfera, Tetera (primitives libreria GLUT)
//
//	  Versió 2.2:	- Objectes Cub, Esfera, Tetera definides en fitxer font glut_geometry amb altres primitives GLUT
//
//	  Versió 2.5:	- Objectes cubRGB i Tie (nau Star Wars fet per alumnes)
//

#include "escenari.h"

#include "framework.h"
#include "material.h"
#include "visualitzacio.h"
#include "escena.h"

// Dibuixa Eixos Coordenades Món i Reixes, activant un shader propi.
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa,
	glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	// Visualització Eixos Coordenades Mòn
	glUseProgram(ax_programID);

	// Pas Matrius Projecció i Vista Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(ax_programID, 0, "in_Vertex"); // Vèrtexs
	glBindAttribLocation(ax_programID, 1, "in_Color");	// Color

	//  Dibuix dels eixos
	if (eix) draw_Eixos(axis_Id);

	// Dibuixa el grid actiu
	if ((reixa.x) || (reixa.y) || (reixa.z) || (reixa.w)) draw_Grid(reixa, hreixa);
}

// Dibuixa Skybox en forma de Cub, activant un shader propi.
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	if (!sk_programID || !cmTexture || Get_VAOId(CUBE_SKYBOX) == 0) return;

	glm::mat4 ModelMatrix(1.0);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE); // Optimitzacio: no escriure al buffer de profunditat

	// Activacio shader per a cub skybox
	glUseProgram(sk_programID);

	// Pas Matrius Projeccio i Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glm::mat4 viewSenseTranslacio = glm::mat4(glm::mat3(MatriuVista));
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "viewMatrix"), 1, GL_FALSE, &viewSenseTranslacio[0][0]);

	// Rotar skyBox per a orientar sobre eix superior Z o X en Vista Esferica (POLARX, POLARY, POLARZ)
	if (eix_Polar == POLARZ) ModelMatrix = glm::rotate(ModelMatrix, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	else if (eix_Polar == POLARX) ModelMatrix = glm::rotate(ModelMatrix, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));

	// Escalar Cub Skybox a 5000 per encabir objectes escena a l'interior
	ModelMatrix = glm::scale(ModelMatrix, vec3(5000.0f, 5000.0f, 5000.0f));
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	// Activar textura cubemaps del Skybox per encabir objectes escena
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmTexture);
	glUniform1i(glGetUniformLocation(sk_programID, "skybox"), 0);

	//  Dibuix del Skybox
	drawCubeSkybox();

	glDepthMask(GL_TRUE); // Restaurar escriptura de profunditat
	glDepthFunc(GL_LESS); // set depth function back to default
}



// dibuixa_EscenaGL: Dibuix de l'escena amb comandes GL
void dibuixa_EscenaGL(GLuint sh_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, char objecte,
	CColor col_object, bool sw_mat[5],
	bool textur, GLuint texturID[NUM_MAX_TEXTURES], bool textur_map, bool flagInvertY,
	int nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat pasCS, bool sw_PC, bool dib_TFrenet,
	COBJModel* objecteOBJ,
	glm::mat4 MatriuTG,
	Escenari* escenariActual, bool mostrarOrbites)
{
	// Shader Visualització Objectes
	glUseProgram(sh_programID);

	// Parametrització i activació/desactivació de textures
	if (texturID[0] != 0) SetTextureParameters(0, texturID[0], true, true, textur_map, false);
	if (textur) {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);
	}
	else {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_FALSE);
	}
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), flagInvertY);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sh_programID, 0, "in_Vertex");		// Vèrtexs
	glBindAttribLocation(sh_programID, 1, "in_Color");		// Color
	glBindAttribLocation(sh_programID, 2, "in_Normal");		// Normals
	glBindAttribLocation(sh_programID, 3, "in_TexCoord");	// Textura

	// Definició propietats de reflexió (emissió, ambient, difusa, especular) del material.
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);

	switch (objecte)
	{
	case SISTEMA_SOLAR:
		if (escenariActual != nullptr)
			escenariActual->dibuixaEscenari(sh_programID, eix, axis_Id, reixa, hreixa, objecte, col_object, sw_mat, textur, textur_map, flagInvertY, nptsU, PC_u, pasCS, sw_PC, dib_TFrenet, objecteOBJ, MatriuTG, mostrarOrbites);
		break;

	default:
		break;
	}
}
