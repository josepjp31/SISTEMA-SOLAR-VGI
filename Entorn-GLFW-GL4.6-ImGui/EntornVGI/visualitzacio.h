//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// visualitzacio.h — Arquitectura: Pipeline Render / Visualització
// Funcions de càmera (esfèrica i navegació), projecció perspectiva, il·luminació multi-llum, eixos i càrrega de textures.

#ifndef GLVIS_H
#define GLVIS_H

/* ------------------------------------------------------------------------- */
/*                                Funcions                                   */
/* ------------------------------------------------------------------------- */

// Precondició: `sh_programID` actiu amb glUseProgram; `lumin` array de NUM_MAX_LLUMS elements.
// Paràmetres: `sh_programID`, `ilumin` (PUNTS/FILFERROS/PLANA/SUAU), `ifix` (true=llum fixa món, false=llum mòbil),
// `ll_amb` (activa llum ambient global), `lumin` vector de fonts de llum.
void Iluminacio(GLint sh_programID, char ilumin, bool ifix, bool ll_amb, LLUM* lumin);

// ----------- Entorn VGI: PROJECCIONS
// Precondició: `sh_programID` actiu amb glUseProgram; `w > 0`, `h >= 0` (si h==0 s'ajusta a 1).
// Paràmetres: `minx`, `miny`, `w`, `h` [píxels] del viewport; `zoom` (no usat internament, conservat per firma).
// Retorn: matriu de projecció perspectiva 4x4 (FOV vertical 60 graus, plans `p_near` i `p_far`).
glm::mat4 Projeccio_Perspectiva(GLuint sh_programID, int minx, int miny, GLsizei w, GLsizei h, double zoom);

// ----------- Entorn VGI: CÀMERES
// Precondició: `sh_programID` actiu amb glUseProgram.
// Paràmetres: `opv` coordenades esfèriques (R [u], alfa [graus], beta [graus]), `VPol` (POLARX/POLARY/POLARZ),
// `col_fons` [RGBA 0..1], `oculta` (Z-buffer), `testv` (culling), `iluminacio`, `llum_amb`, `lumin`, `ifix`.
// Retorn: matriu de vista 4x4 centrada a l'origen (0,0,0).
glm::mat4 Vista_Esferica(GLuint sh_programID, CEsfe3D opv, char VPol,
				 CColor col_fons, bool oculta, bool testv,
				 char iluminacio, bool llum_amb, LLUM* lumin, bool ifix);

// Precondició: `sh_programID` actiu amb glUseProgram.
// Paràmetres: `pv` posició de la càmera [u], `n[3]` punt objectiu [u], `v[3]` vector up, i opcions d'escena/il·luminació.
// Retorn: matriu de vista 4x4 en mode càmera lliure (Navega).
glm::mat4 Vista_Navega(GLuint sh_programID, CPunt3D pv,
				  GLdouble n[3], GLdouble v[3], CColor col_fons,
				  bool oculta, bool testv,
				  char iluminacio, bool llum_amb, LLUM* lumin, bool ifix);

// Precondició: context OpenGL actiu.
// Retorn: identificador VAO dels 3 eixos de coordenades (X vermell, Y verd, Z blau, longitud 300 u).
GLuint deixos(void);

// Precondició: `vaoId` creat per `deixos()` i shader d'eixos actiu.
void draw_Eixos(GLuint vboId);

// Neteja els buffers de color i profunditat amb el color `int_fons` [RGB 0..1].
void Fons(CColor int_fons);

// Precondició: context OpenGL actiu.
// Paràmetres: `filename` ruta al fitxer d'imatge suportat per SOIL2.
// Retorn: identificador OpenGL de la textura 2D generada (> 0), o 0 (false) si falla l'obertura.
GLint loadIMA_SOIL(const char *filename);

// Precondició: context OpenGL actiu; `faces` amb 6 rutes (+X, -X, +Y, -Y, +Z, -Z).
// Retorn: identificador OpenGL de la textura `GL_TEXTURE_CUBE_MAP` (> 0), o 0 si falla alguna cara.
unsigned int loadCubemap(std::vector<std::string> faces);

// Precondició: context OpenGL actiu i textura `IDText` vàlida.
// Paràmetres: `num` unitat de textura [0..15], `IDText`, `extensT` (REPEAT vs CLAMP), `filterT` (LINEAR vs NEAREST),
// `blendT` (legacy), `genMipmpT` (generació de mipmaps).
void SetTextureParameters(GLint num, GLint IDText, bool extensT, bool filterT, bool blendT, bool genMipmpT);
#endif