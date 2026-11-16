//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// visualitzacio.h : Interface de visualitzacio.cpp
//

#ifndef GLVIS_H
#define GLVIS_H

/* ------------------------------------------------------------------------- */
/*                                Funcions                                   */
/* ------------------------------------------------------------------------- */

// Entorn VGI: Il.luminació
void Iluminacio(GLint sh_programID, char ilumin, bool ifix, bool ll_amb, LLUM* lumin);


// ----------- Entorn VGI: PROJECCIONS
// Entorn VGI: Projecció Perspectiva
glm::mat4 Projeccio_Perspectiva(GLuint sh_programID, int minx, int miny, GLsizei w, GLsizei h, double zoom);

// ----------- Entorn VGI: CÀMERES
// Entorn VGI: Càmera Esfèrica
glm::mat4 Vista_Esferica(GLuint sh_programID, CEsfe3D opv, char VPol,
				 CColor col_fons, bool oculta, bool testv,
				 char iluminacio, bool llum_amb, LLUM* lumin, bool ifix);
// Entorn VGI: Càmera Navega
glm::mat4 Vista_Navega(GLuint sh_programID, CPunt3D pv,
				  GLdouble n[3], GLdouble v[3], CColor col_fons,
				  bool oculta, bool testv,
				  char iluminacio, bool llum_amb, LLUM* lumin, bool ifix);

// Entorn VGI: Dibuix dels eixos de coordenades
GLuint deixos(void);
void draw_Eixos(GLuint vboId);

// Entorn VGI: Esborrat de pantalla a fons variable (Fons)
void Fons(CColor int_fons);

// Entorn VGI: Carregar fitxer imatge de formats admesos per la llibreria DevIL/OpenIL 
//				com a textura
GLint loadIMA_SOIL(const char *filename);
unsigned int loadCubemap(std::vector<std::string> faces);
void SetTextureParameters(GLint num, GLint IDText, bool extensT, bool filterT, bool blendT, bool genMipmpT);
#endif