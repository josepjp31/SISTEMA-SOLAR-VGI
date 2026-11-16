//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// constants.h — Arquitectura: Constants i Tipus Compartits
// Definició de constants simbòliques, límits de visualització i estructures bàsiques de càmera, color, VAO i il·luminació.

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
extern double p_near; // [unitats OpenGL] Distància del pla de retall proper (> 0.0, dinàmic en Escala Real)
extern double p_far;  // [unitats OpenGL] Distància del pla de retall llunyà (> p_near)

// -------------- VGI: CONSTANTS TEXTURES
// Nombre màxim de textures per escenari
#define NUM_MAX_TEXTURES 15

// --------------  VGI: NOMBRE DE LLUMS: Nombre de Llums de l'aplicació, les d'OpenGL
const int NUM_MAX_LLUMS = 8;

// -------------- VGI: SHADERS --> Tipus de Shaders
#define CAP_SHADER ' '
#define PHONG_SHADER 'P'

//--------------- VGI: Valor constant de pi
const double PI = 3.14159;        // [radians] Constant Pi aproximada
const double TWOPI = 2 * PI;      // [radians] 2*Pi (circumferència completa)
const double PID2 = PI / 2;       // [radians] Pi/2 (90 graus)

// --------------- VGI: Estructura tamany 2D (Pre-definida en Visual Studio 2019)
struct CSize
{
	GLint cx; // [píxels] Amplada (>= 0)
	GLint cy; // [píxels] Alçada (>= 0)
};

// --------------- VGI: Estructura coordenada 2D (Ja definida en Visual Studio 2010)
struct CPoint
{
	GLint x; // [píxels] Coordenada horitzontal de pantalla
	GLint y; // [píxels] Coordenada vertical de pantalla
};

// --------------- VGI: Estructura coordenada 3D homogenia
struct CPunt3D
{
	GLdouble x; // [unitats OpenGL] Coordenada X (o component X de direcció)
	GLdouble y; // [unitats OpenGL] Coordenada Y (o component Y de direcció)
	GLdouble z; // [unitats OpenGL] Coordenada Z (o component Z de direcció)
	GLdouble w; // [adimensional] Component homogènia (1.0 = posició puntual, 0.0 = direcció)
};

// --------------- GMS: Màscara booleana sobre coordenades 3D
struct CMask3D
{
	bool x; // Activar/desactivar component o reixa X
	bool y; // Activar/desactivar component o reixa Y
	bool z; // Activar/desactivar component o reixa Z
	bool w; // Activar/desactivar component o reixa global XYZ
};

// --------------- VGI: Estructura de color R,G,B,A
struct CColor
{
	GLdouble r; // [0.0..1.0] Canal vermell
	GLdouble g; // [0.0..1.0] Canal verd
	GLdouble b; // [0.0..1.0] Canal blau
	GLdouble a; // [0.0..1.0] Canal alfa (opacitat)
};

// --------------- VGI: Estructura coordenada Esfèrica 3D
struct CEsfe3D
{
	GLdouble R;    // [unitats OpenGL] Distància radial de la càmera al centre d'òrbita (> 0.0)
	GLdouble alfa; // [graus] Angle d'elevació vertical [-90..90]
	GLdouble beta; // [graus] Angle azimutal horitzontal [0..360)
};

// --------------- VGI: Estructura LLista VAO
struct CVAO
{
	GLuint vaoId;   // Identificador del Vertex Array Object (0 = buit)
	GLuint vboId;   // Identificador del Vertex Buffer Object (0 = buit)
	GLuint eboId;   // Identificador de l'Element Buffer Object d'índexs (0 = sense EBO)
	GLint nVertexs; // Nombre de vèrtexs emmagatzemats (>= 0)
	GLint nIndices; // Nombre d'índexs per a glDrawElements (>= 0)
};

// --------------- VGI: Coeficients equació d'atenuació de la llum fatt=1/(ad2+bd+c)
struct CAtenua
{
	GLdouble a; // [1/unitats^2] Coeficient d'atenuació quadràtica (>= 0.0)
	GLdouble b; // [1/unitats] Coeficient d'atenuació lineal (>= 0.0)
	GLdouble c; // [adimensional] Coeficient d'atenuació constant (>= 0.0; 1.0 sense atenuació)
};

// --------------- VGI: Estructura de coeficients de reflectivitat de materials
struct MATERIAL
{
	GLfloat ambient[4];  // [RGBA 0.0..1.0] Reflectivitat ambient
	GLfloat diffuse[4];  // [RGBA 0.0..1.0] Reflectivitat difusa
	GLfloat specular[4]; // [RGBA 0.0..1.0] Reflectivitat especular
	GLfloat emission[4]; // [RGBA 0.0..1.0] Emissió pròpia del material
	GLfloat shininess;   // [0.0..128.0] Exponent especular de Phong
};

// --------------- VGI: Estructura font de llum
struct LLUM
{
	bool encesa;            // Estat de la font de llum [true = encesa, false = apagada]
	CPunt3D posicio;        // [unitats OpenGL] Posició (w=1.0) o direcció (w=0.0) de la llum
	CColor difusa;          // [RGBA 0.0..1.0] Intensitat difusa de la font de llum
	CColor especular;       // [RGBA 0.0..1.0] Intensitat especular de la font de llum
	CAtenua atenuacio;      // Coeficients de l'equació d'atenuació fatt = 1 / (a*d^2 + b*d + c)
	bool restringida;       // Booleana que indica si és focus focal/spot [true] o omnidireccional [false]
	CPunt3D spotdirection;  // [vector normalitzable] Vector de direcció del focus restringit
	GLfloat spotcoscutoff;  // [cos(theta), -1.0..1.0] Cosinus de l'angle d'obertura del focus
	GLfloat spotexponent;   // [0.0..128.0] Exponent d'atenuació angular de Warn
};

#endif