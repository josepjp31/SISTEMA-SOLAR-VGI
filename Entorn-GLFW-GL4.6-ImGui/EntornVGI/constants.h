//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// constants.h : Definició de constants compartides
//				 visualitzacio.cpp, escena.cpp i main.cpp


#ifndef CONST_H
#define CONST_H

//--------------- VGI: Tipus de Càmera
#define CAP ' '
#define CAM_ESFERICA 'E'
#define CAM_NAVEGA 'N'

//--------------- VGI: Tipus de Projecció
#define PERSPECT 'P'

//--------------- VGI: Tipus de Polars (per la Visualització Interactiva)
#define POLARZ 'Z'
#define POLARY 'Y'
#define POLARX 'X'

//-------------- VGI: Tipus d'Objectes
#define SISTEMA_SOLAR '$'

//-------------- VGI: Tipus d'Iluminacio
#define PUNTS 'P'
#define FILFERROS 'w'
#define PLANA 'f'
#define SUAU 's'

// -------------- VGI: Definició dels valors del pla near i far del Volum de Visualització en Perspectiva
extern double p_near;
extern double p_far;

// -------------- VGI: CONSTANTS TEXTURES
// Nombre màxim de textures
#define NUM_MAX_TEXTURES 15

// --------------  VGI: NOMBRE DE LLUMS: Nombre de Llums de l'aplicació, les d'OpenGL
const int NUM_MAX_LLUMS = 8;

// -------------- VGI: SHADERS --> Tipus de Shaders
#define CAP_SHADER ' '
#define PHONG_SHADER 'P'

//--------------- VGI: Valor constant de pi
const double PI = 3.14159;
const double TWOPI = 2 * PI;
const double PID2 = PI / 2;

// --------------- VGI: Estructura tamany 2D (Pre-definida en Visual Studio 2019)
struct CSize
{
	GLint cx;
	GLint cy;
};

// --------------- VGI: Estructura coordenada 2D (Ja definida en Visual Studio 2010)
struct CPoint
{
	GLint x;
	GLint y;
};

// --------------- VGI: Estructura coordenada 3D
struct CPunt3D
{
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;
};

// --------------- GMS: 3Màscara booleana sobre coordenades 3D
struct CMask3D
{
	bool x;
	bool y;
	bool z;
	bool w;
};

// --------------- VGI: Estructura de color R,G,B,A
struct CColor
{
	GLdouble r;
	GLdouble g;
	GLdouble b;
	GLdouble a;
};

// --------------- VGI: Estructura coordenada Esfèrica 3D
struct CEsfe3D
{
	GLdouble R;
	GLdouble alfa;
	GLdouble beta;
};

// --------------- VGI: Estructura LLista VAO
struct CVAO
{
	GLuint vaoId;
	GLuint vboId;
	GLuint eboId;
	GLint nVertexs;
	GLint nIndices;
};

// --------------- VGI: Coeficients equació d'atenuació de la llum fatt=1/(ad2+bd+c)
struct CAtenua
{
	GLdouble a;
	GLdouble b;
	GLdouble c;
};

// --------------- VGI: Estructura de coeficients de reflectivitat de materials
struct MATERIAL
{
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
	GLfloat shininess;
};

// --------------- VGI: Estructura font de llum
struct LLUM
{
	bool encesa;				// Booleana que controla si la llum és encesa [TRUE] o no [FALSE]
	CPunt3D posicio;		// Posició (x,y,z) de la font de llum en coordenades esfèriques.
	CColor difusa;			// Intensitat difusa de la font de llum (r,g,b,a)
	CColor especular;		// Intensitat especular de la font de llum (r,g,b,a)
	CAtenua atenuacio;		// Coeficients de l'equació d'atenuació de la llum fatt=1/(ad2+bd+c)
	bool restringida;		// Booleana que indica si la font de llum és restringida [TRUE] i per tant són vàlids els coeficients posteriors o no [FALSE].
	CPunt3D spotdirection;	// Vector de direció de la font de llum restringida (x,y,z).
	GLfloat spotcoscutoff;	// Coseno de l'angle d'obertura de la font de llum restringida.
	GLfloat spotexponent;	// Exponent que indica l'atenuació de la font del centre de l'eix a l'exterior, segons model de Warn.
};

#endif
