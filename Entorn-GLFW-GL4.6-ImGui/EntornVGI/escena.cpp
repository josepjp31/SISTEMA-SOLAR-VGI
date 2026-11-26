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


BoundingBox boundingBoxMilano;//COLISIONS

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
void dibuixa_EscenaGL(GLuint sh_programID, GLuint shader_programIDNormalMap, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5],
	Escenari* escenariActual, COBJModel* objecteOBJ, bool mostrarOrbites, bool mostrarMilano)
{
	glm::mat4 ModelMatrix(1.0);

	// Shader Visualització Objectes
	glUseProgram(sh_programID);

	static GLuint s_cachedProgramID = 0;
	static GLint s_locTextur = -1;
	static GLint s_locModulate = -1;
	static GLint s_locFlagInvertY = -1;
	static GLint s_locModelMatrix = -1;
	static GLint s_locUseNormalMatrixPV = -1;
	static GLint s_locUNormalMatrixPV = -1;
	if (s_cachedProgramID != sh_programID)
	{
		s_cachedProgramID = sh_programID;
		s_locTextur = glGetUniformLocation(sh_programID, "textur");
		s_locModulate = glGetUniformLocation(sh_programID, "modulate");
		s_locFlagInvertY = glGetUniformLocation(sh_programID, "flag_invert_y");
		s_locModelMatrix = glGetUniformLocation(sh_programID, "modelMatrix");
		s_locUseNormalMatrixPV = glGetUniformLocation(sh_programID, "useNormalMatrixPV");
		s_locUNormalMatrixPV = glGetUniformLocation(sh_programID, "uNormalMatrixPV");
	}

	// Parametrització inicial de textures
	if (s_locTextur != -1) glUniform1i(s_locTextur, GL_FALSE);
	if (s_locModulate != -1) glUniform1i(s_locModulate, GL_FALSE);
	if (s_locFlagInvertY != -1) glUniform1i(s_locFlagInvertY, GL_TRUE);

	if (escenariActual != nullptr)
		escenariActual->dibuixaEscenari(sh_programID, shader_programIDNormalMap, MatriuVista, MatriuTG, sw_mat, mostrarOrbites);

	//COET
	if (mostrarMilano && objecteOBJ != nullptr)
	{
		// Centre geomètric del model
		glm::vec3 centreModel(5.05f, 0.71f, -7.55f);

		ModelMatrix = glm::translate(MatriuTG, posMilano);

		// Portar el centre del model a l'origen
		ModelMatrix = glm::translate(ModelMatrix, centreModel);

		// Calcular l'eix lateral de la nau
		float angleRad = glm::radians(angleMilano);

		glm::vec3 eixPitch(
			-sin(angleRad),
			cos(angleRad),
			0.0f
		);

		// Inclinació vertical de la nau
		ModelMatrix = glm::rotate(
			ModelMatrix,
			glm::radians(-inclinacioMilano),
			eixPitch
		);

		// Gir horitzontal de la nau
		ModelMatrix = glm::rotate(
			ModelMatrix,
			glm::radians(angleMilano ),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);

		// Recuperar la posició original dels vèrtexs
		ModelMatrix = glm::translate(ModelMatrix, -centreModel);

		// Orientació fixa del model (sobre z)
		ModelMatrix = glm::rotate(
			ModelMatrix,
			glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);

		// Gir de 180 graus sobre Y
		ModelMatrix = glm::rotate(
			ModelMatrix,
			glm::radians(180.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		//morro baix
		ModelMatrix = glm::rotate(
			ModelMatrix,
			glm::radians(-90.0f),
			glm::vec3(1.0f, 0.0f, 0.0f)
		);

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));

		const glm::vec3 minMilano(-6.3877f, -2.90362f, -12.7207f);
		const glm::vec3 maxMilano(16.4814f, 4.31772f, -2.36932f);

		boundingBoxMilano = creaBoundingBox(ModelMatrix, minMilano, maxMilano);//COLISIONS

		if (s_locModelMatrix != -1)
			glUniformMatrix4fv(s_locModelMatrix, 1, GL_FALSE, &ModelMatrix[0][0]);

		if (s_locUseNormalMatrixPV != -1 && s_locUNormalMatrixPV != -1)
		{
			glm::mat3 normalMatPV = glm::mat3(glm::transpose(glm::inverse(MatriuVista * ModelMatrix)));
			glUniform1i(s_locUseNormalMatrixPV, GL_TRUE);
			glUniformMatrix3fv(s_locUNormalMatrixPV, 1, GL_FALSE, &normalMatPV[0][0]);
		}

		GLboolean prevCullEnabled = glIsEnabled(GL_CULL_FACE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		objecteOBJ->draw_TriVAO_OBJ(sh_programID);

		if (!prevCullEnabled) glDisable(GL_CULL_FACE);
		if (s_locUseNormalMatrixPV != -1)
			glUniform1i(s_locUseNormalMatrixPV, GL_FALSE);
	}
}
