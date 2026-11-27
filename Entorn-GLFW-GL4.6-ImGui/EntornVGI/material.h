//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// material.h : Interfície de material.cpp (configuració de color i reflectivitat de materials al shader)
//

#ifndef GLMATERIAL_H
#define GLMATERIAL_H

#include "framework.h"

/* ------------------------------------------------------------------------- */
/*                            Funcions de material                           */
/* ------------------------------------------------------------------------- */

// Configura i envia al shader la reflectivitat del material (emissió, ambient, difusa, especular) segons un color base
void SeleccionaColorMaterial(GLuint sh_programID, CColor c_obj, bool sw_mater[5]);

#endif
