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

// Copia matrius i llums del shader Phong al shader de normal mapping (perque tots dos il.luminin igual)
static void copiaUniformsShader(GLuint src, GLuint dst)
{
	auto locs = [&](const char* nom, GLint& ls, GLint& ld) {
		ls = glGetUniformLocation(src, nom);
		ld = glGetUniformLocation(dst, nom);
		return ls >= 0 && ld >= 0;
		};
	auto copiaMat4 = [&](const char* nom) {
		GLint ls, ld; GLfloat m[16];
		if (!locs(nom, ls, ld)) return;
		glGetUniformfv(src, ls, m);
		glProgramUniformMatrix4fv(dst, ld, 1, GL_FALSE, m);
		};
	auto copiaF = [&](const char* nom, int n) {	// vecN de floats (n = 1..4)
		GLint ls, ld; GLfloat v[4];
		if (!locs(nom, ls, ld)) return;
		glGetUniformfv(src, ls, v);
		if (n == 1) glProgramUniform1fv(dst, ld, 1, v);
		else if (n == 3) glProgramUniform3fv(dst, ld, 1, v);
		else glProgramUniform4fv(dst, ld, 1, v);
		};
	auto copiaI = [&](const char* nom) {		// int / bool
		GLint ls, ld, v;
		if (!locs(nom, ls, ld)) return;
		glGetUniformiv(src, ls, &v);
		glProgramUniform1i(dst, ld, v);
		};

	copiaMat4("projectionMatrix");
	copiaMat4("viewMatrix");

	// normalMatrix (solo la usa el frag para spots): inversa-transpuesta de la vista
	GLint lsV = glGetUniformLocation(src, "viewMatrix");
	GLint ldN = glGetUniformLocation(dst, "normalMatrix");
	if (lsV >= 0 && ldN >= 0) {
		glm::mat4 V; glGetUniformfv(src, lsV, &V[0][0]);
		glm::mat4 NM = glm::transpose(glm::inverse(V));
		glProgramUniformMatrix4fv(dst, ldN, 1, GL_FALSE, &NM[0][0]);
	}

	copiaF("LightModelAmbient", 4);
	copiaI("fixedLight");

	char n[64];
	for (int i = 0; i < 8; i++) {
		sprintf_s(n, "LightSource[%d].sw_light", i);		copiaI(n);
		sprintf_s(n, "LightSource[%d].position", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].ambient", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].diffuse", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].specular", i);		copiaF(n, 4);
		sprintf_s(n, "LightSource[%d].attenuation", i);	copiaF(n, 3);
		sprintf_s(n, "LightSource[%d].restricted", i);		copiaI(n);
		sprintf_s(n, "LightSource[%d].spotDirection", i);	copiaF(n, 3);
		sprintf_s(n, "LightSource[%d].spotCosCutoff", i);	copiaF(n, 1);
		sprintf_s(n, "LightSource[%d].spotExponent", i);	copiaF(n, 1);
	}
}

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

	// Parametrització inicial de textures
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
	glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_FALSE);
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), GL_TRUE);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sh_programID, 0, "in_Vertex");		// Vèrtexs
	glBindAttribLocation(sh_programID, 1, "in_Color");		// Color
	glBindAttribLocation(sh_programID, 2, "in_Normal");		// Normals
	glBindAttribLocation(sh_programID, 3, "in_TexCoord");	// Textura

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
			glm::radians(angleMilano),
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

		glUniformMatrix4fv(
			glGetUniformLocation(sh_programID, "modelMatrix"),
			1, GL_FALSE, &ModelMatrix[0][0]
		);

		if (shader_programIDNormalMap != 0 && objecteOBJ->iNormalMapID != 0)
		{
			CColor col_blanc;
			col_blanc.r = 1.0; col_blanc.g = 1.0; col_blanc.b = 1.0; col_blanc.a = 1.0;
			
			glUseProgram(shader_programIDNormalMap);
			copiaUniformsShader(sh_programID, shader_programIDNormalMap);	// llums, càmera, etc.
			glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "texture0"), 0);
			glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "textur"), GL_TRUE);
			glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "modulate"), GL_TRUE);
			glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "flag_invert_y"), GL_TRUE);
			SeleccionaColorMaterial(shader_programIDNormalMap, col_blanc, sw_mat);
			glUniformMatrix4fv(glGetUniformLocation(shader_programIDNormalMap, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniform4i(glGetUniformLocation(shader_programIDNormalMap, "sw_intensity"),sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
			glUniform1f(glGetUniformLocation(shader_programIDNormalMap, "emissionScale"), 0.3f);
			glUniform1f(glGetUniformLocation(shader_programIDNormalMap, "bumpStrength"), 1.0f);
			
			objecteOBJ->draw_TriVAO_OBJ_NormalMap(shader_programIDNormalMap);
			
			glUniform1f(glGetUniformLocation(shader_programIDNormalMap, "emissionScale"), 1.0f);
			glUniform1f(glGetUniformLocation(shader_programIDNormalMap, "bumpStrength"), 0.3f);
			glUseProgram(sh_programID);
		}
		else {
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			objecteOBJ->draw_TriVAO_OBJ(sh_programID);		// fallback sin normal map	}

		}
	}
}

