//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// escena.h — Arquitectura: Pipeline Render / Visualització
// Punt d'entrada de dibuix per a eixos de coordenades, skybox cubemap i delegació del renderitzat a Escenari.
//

#ifndef GLSOURCE_H
#define GLSOURCE_H

#include "escenari.h"

struct EstatSimulacio;

/* ------------------------------------------------------------------------- */
/*                            Funcions de les escenes                        */
/* ------------------------------------------------------------------------- */

// Precondició: context OpenGL actiu, `ax_programID` i `axis_Id` creats.
// Paràmetres: `ax_programID` (shader d'eixos), `eix` (activar/desactivar), `axis_Id` (VAO dels eixos),
// `reixa` i `hreixa` [unitats OpenGL], `MatriuProjeccio` i `MatriuVista`.
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);

// Precondició: context OpenGL actiu; si algun identificador o VAO de Skybox és 0 retorna sense dibuixar.
// Paràmetres: `sk_programID` (shader skybox), `cmTexture` (cubemap OpenGL), `eix_Polar` (POLARX/POLARY/POLARZ),
// `MatriuProjeccio` i `MatriuVista` (se n'elimina la translació per mantenir el skybox infinit).
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);

// Precondició: context OpenGL actiu i `sh_programID` enllaçat.
// Paràmetres: `sh_programID` (shader Phong), `MatriuVista`, `MatriuTG`, `sw_mat[5]` (interruptors de material),
// `escenariActual` (punter a l'escenari actiu o nullptr) i `estat` (estat global de simulació).
void dibuixa_EscenaGL(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5],
	Escenari* escenariActual, const EstatSimulacio& estat);

#endif
