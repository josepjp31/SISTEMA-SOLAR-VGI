//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// material.cpp : Funcions de definició de les propietats de reflectivitat dels objectes.

#include "framework.h"
#include "material.h"


/******************************************************************************\
|* SeleccionaColorMaterial                                                    *|
|* Selecciona el Color del material                                           *|
\******************************************************************************/
void SeleccionaColorMaterial(GLuint sh_programID, CColor c_obj, bool sw_mater[5])
{
	//const float zeros[4]={0.0f,0.0f,0.0f,1.0f};
	//float color[4] = { c_obj.r, c_obj.g, c_obj.b, c_obj.a };
	MATERIAL material;

	SetColor4d(c_obj.r, c_obj.g, c_obj.b, c_obj.a);		// Definir color a primitives GLUT.

// Pas vector booleans sw_materials al Shader
	glUniform4i(glGetUniformLocation(sh_programID, "sw_intensity"), sw_mater[0], sw_mater[1], sw_mater[2], sw_mater[3]);
	glUniform1i(glGetUniformLocation(sh_programID, "sw_material"), sw_mater[4]);

// Component de reflectivitat d'emissió de llum del material
//	glColorMaterial(GL_FRONT, GL_EMISSION);	// Codi OpenGL 1.1
	if (sw_mater[0]) {	//glColor4fv(color);	// Codi OpenGL 1.1
						//glMaterialfv(GL_FRONT, GL_EMISSION, color);	// Codi OpenGL 1.1
						material.emission[0] = c_obj.r;	
						material.emission[1] = c_obj.g;	
						material.emission[2] = c_obj.b;	
						material.emission[3] = c_obj.a;
					}
		else {	//glColor4fv(zeros);
				//glMaterialfv(GL_FRONT, GL_EMISSION, zeros);	// Codi OpenGL 1.1
				material.emission[0] = 0.0f;
				material.emission[1] = 0.0f;
				material.emission[2] = 0.0f;
				material.emission[3] = 1.0f;
			}
// Pas component emission del material al Shader
	glUniform4f(glGetUniformLocation(sh_programID, "material.emission"), material.emission[0], material.emission[1], material.emission[2], material.emission[3]);

// Component de reflectivitat ambient del material
//	glColorMaterial(GL_FRONT, GL_AMBIENT);	// Codi OpenGL 1.1
	if (sw_mater[1]) {	//glColor4fv(color);	// Codi OpenGL 1.1
						//glMaterialfv(GL_FRONT, GL_AMBIENT, color);	// Codi OpenGL 1.1
						material.ambient[0] = c_obj.r;
						material.ambient[1] = c_obj.g;
						material.ambient[2] = c_obj.b;
						material.ambient[3] = c_obj.a;
					}
		else {	//glColor4fv(zeros);	// Codi OpenGL 1.1
				//glMaterialfv(GL_FRONT, GL_AMBIENT, zeros);	// Codi OpenGL 1.1
				material.ambient[0] = 0.0f;
				material.ambient[1] = 0.0f;
				material.ambient[2] = 0.0f;
				material.ambient[3] = 1.0f;
			}

// Pas component ambient del material al Shader
	glUniform4f(glGetUniformLocation(sh_programID, "material.ambient"), material.ambient[0], material.ambient[1], material.ambient[2], material.ambient[3]);

// Component de reflectivitat difusa del material
//	glColorMaterial(GL_FRONT, GL_DIFFUSE);	// Codi OpenGL 1.1
	if (sw_mater[2]) {	//glColor4fv(color);	// Codi OpenGL 1.1
						//glMaterialfv(GL_FRONT, GL_DIFFUSE, color);	// Codi OpenGL 1.1
						material.diffuse[0] = c_obj.r;
						material.diffuse[1] = c_obj.g;
						material.diffuse[2] = c_obj.b;
						material.diffuse[3] = c_obj.a;
					}
		else {	//glColor4fv(zeros);	// Codi OpenGL 1.1
				//glMaterialfv(GL_FRONT, GL_DIFFUSE, zeros);	// Codi OpenGL 1.1
				material.diffuse[0] = 0.0f;
				material.diffuse[1] = 0.0f;
				material.diffuse[2] = 0.0f;
				material.diffuse[3] = 1.0f;
			}

// Pas component diffuse del material al Shader
	glUniform4f(glGetUniformLocation(sh_programID, "material.diffuse"), material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]);

// Component de reflectivitat especular del material
//	glColorMaterial(GL_FRONT, GL_SPECULAR);	// Codi OpenGL 1.1
	if (sw_mater[3]) {	//glColor4fv(color);	// Codi OpenGL 1.1
						//glMaterialfv(GL_FRONT, GL_SPECULAR, color);	// Codi OpenGL 1.1
						//glMaterialf(GL_FRONT, GL_SHININESS, 128);	// Codi OpenGL 1.1
						material.specular[0] = c_obj.r;
						material.specular[1] = c_obj.g;
						material.specular[2] = c_obj.b;
						material.specular[3] = c_obj.a;
						material.shininess = 32.0f;
					}
		else {	//glColor4fv(zeros);	// Codi OpenGL 1.1
				//glMaterialfv(GL_FRONT, GL_SPECULAR, zeros);	// Codi OpenGL 1.1
				//glMaterialf(GL_FRONT, GL_SHININESS, 0);	// Codi OpenGL 1.1
				material.specular[0] = 0.0f;
				material.specular[1] = 0.0f;
				material.specular[2] = 0.0f;
				material.specular[3] = 1.0f;
				material.shininess = 0.0f;
			}
// Pas component specular i shininess del material al Shader
	glUniform4f(glGetUniformLocation(sh_programID, "material.specular"), material.specular[0], material.specular[1], material.specular[2], material.specular[3]);
	glUniform1f(glGetUniformLocation(sh_programID, "material.shininess"), material.shininess);
}
