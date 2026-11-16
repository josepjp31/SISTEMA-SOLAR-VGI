//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// material.h — Arquitectura: Pipeline Render / Visualització
// Configuració de les propietats de reflectivitat del material (emissió, ambient, difusa, especular) al shader GLSL.

#ifndef GLMATERIAL_H
#define GLMATERIAL_H

#include "framework.h"

/* ------------------------------------------------------------------------- */
/*                            Funcions de material                           */
/* ------------------------------------------------------------------------- */

// Precondició: context OpenGL actiu i `sh_programID` vinculat amb glUseProgram.
// Paràmetres:
//   - sh_programID: identificador del programa shader Phong actiu.
//   - c_obj: color base [RGBA 0.0..1.0] aplicat a les components actives.
//   - sw_mater[5]: interruptors [0]=emission, [1]=ambient, [2]=diffuse, [3]=specular (shininess=32), [4]=sw_material.
void SeleccionaColorMaterial(GLuint sh_programID, CColor c_obj, bool sw_mater[5]);

#endif