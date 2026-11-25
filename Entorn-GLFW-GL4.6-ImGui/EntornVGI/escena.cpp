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
	Escenari* escenariActual, COBJModel* objecteOBJ, COBJModel* sondes[], bool mostrarOrbites, bool mostrarMilano)
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

		glUniformMatrix4fv(
			glGetUniformLocation(sh_programID, "modelMatrix"),
			1, GL_FALSE, &ModelMatrix[0][0]
		);

		objecteOBJ->draw_TriVAO_OBJ(sh_programID);
	}
	//SONDES
	if (sondes != nullptr && escenariActual != nullptr && escenariActual->m_objectList != nullptr)
	{
		// Centre de cada model (per portar-lo a l'origen) i escala (totes ~1.5 unitats; la Terra te radi 3.7)
		const glm::vec3 centreSonda[NUM_SONDES] = {
			{ 0.00f,  1.49f, -3.88f },	// ISS
			{ 0.05f, -3.28f, -0.02f },	// Hubble
			{ 0.00f,  2.47f, -0.10f },	// JWST
			{ 2.71f, -0.23f, -0.01f },	// Juno
			{-0.01f, -2.44f, -4.52f },	// Cassini
			{ 0.00f,  0.00f,  1.83f },	// Parker
			{ 0.00f,  3.48f,  4.25f },	// Voyager
			{ 0.00f, -2.65f,  0.00f }	// Pioneer
		};
		const float escalaSonda[NUM_SONDES] = { 0.06f, 0.003f, 0.07f, 0.08f, 0.07f, 0.25f, 0.10f, 0.20f };

		// Posicions (mateixa formula que escenari.cpp: pla XY, angle en graus)
		Object* obj = escenariActual->m_objectList;
		auto puntOrbita = [](float radi, float angleGraus) {
			float a = glm::radians(angleGraus);
			return glm::vec3(radi * cos(a), radi * sin(a), 0.0f);
			};
		glm::vec3 terra = puntOrbita(obj[3].m_radiOrbita, obj[3].m_angleOrbita);
		glm::vec3 jupiter = puntOrbita(obj[5].m_radiOrbita, obj[5].m_angleOrbita);
		glm::vec3 saturn = puntOrbita(obj[6].m_radiOrbita, obj[6].m_angleOrbita);

		// Els angles depenen dels dels planetes: es paren i acceleren amb el control de temps
		glm::vec3 posSonda[NUM_SONDES];
		posSonda[SONDA_ISS] = terra + puntOrbita(5.5f, obj[3].m_angleOrbita * 40.0f);
		posSonda[SONDA_HUBBLE] = terra + puntOrbita(6.5f, obj[3].m_angleOrbita * 30.0f + 180.0f);
		posSonda[SONDA_JWST] = terra + glm::normalize(terra) * 9.0f;	// L2: darrere la Terra, oposat al Sol
		posSonda[SONDA_JUNO] = jupiter + puntOrbita(13.0f, obj[5].m_angleOrbita * 15.0f);
		posSonda[SONDA_CASSINI] = saturn + puntOrbita(15.0f, obj[6].m_angleOrbita * 10.0f + 90.0f);
		posSonda[SONDA_PARKER] = puntOrbita(21.0f, obj[1].m_angleOrbita * 1.5f);	// entre el Sol i Mercuri
		posSonda[SONDA_VOYAGER] = puntOrbita(90.0f, 35.0f);	// provisional: entre Mart (70) i Jupiter (105) per veure-la be
		posSonda[SONDA_PIONEER] = puntOrbita(235.0f, 200.0f);

		// Els OBJ no defineixen material.emission: sense aixo hereten el blanc emissiu
		// que deixa dibuixaEscenari (SeleccionaColorMaterial amb col_blanc) i surten blanques.
		// Tambe activem modulate perque les textures rebin la llum del Sol.
		glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), GL_FALSE, sw_mat[1], sw_mat[2], sw_mat[3]);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);

		for (int i = 0; i < NUM_SONDES; i++)
		{
			if (sondes[i] == nullptr) continue;

			ModelMatrix = glm::translate(MatriuTG, posSonda[i]);
			// Els models s'han exportat amb Y amunt; l'escena te Z amunt
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(escalaSonda[i]));
			// Portar el centre del model a l'origen
			ModelMatrix = glm::translate(ModelMatrix, -centreSonda[i]);

			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			sondes[i]->draw_TriVAO_OBJ(sh_programID);
		}

		// Restaurar l estat dels uniforms
		glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mat[0], sw_mat[1], sw_mat[2], sw_mat[3]);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_FALSE);
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
	}
}
