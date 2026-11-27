//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW  3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// escena.h : Interfície de escena.cpp (eixos, skybox i renderitzat de l'escena del Sistema Solar)
//

#ifndef GLSOURCE_H
#define GLSOURCE_H

#include "escenari.h"
#include "objLoader.h"	// COBJModel (model OBJ de la nau Milano)

/* ------------------------------------------------------------------------- */
/*                            Funcions de les escenes                        */
/* ------------------------------------------------------------------------- */

// Dibuixa els eixos de coordenades del món i les reixes actives
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);

// Dibuixa el cubemap Skybox de fons
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);

// Dibuixa l'escena OpenGL principal delegant el Sistema Solar a Escenari
// (shader_programIDNormalMap: shader de normal mapping dels planetes; objecteOBJ: model de la nau Milano)
void dibuixa_EscenaGL(GLuint sh_programID, GLuint shader_programIDNormalMap, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5],
	Escenari* escenariActual, COBJModel* objecteOBJ, bool mostrarOrbites = true, bool mostrarMilano = true/*COET*/, const ParametresIluminacioEscenari* ilum = nullptr);

//VARIABLES GLOBALS COET
extern glm::vec3 posMilano;
extern float angleMilano;
extern float inclinacioMilano;

#endif
