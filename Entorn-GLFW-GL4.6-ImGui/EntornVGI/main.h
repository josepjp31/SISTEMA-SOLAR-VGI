//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// main.h : Interfície i declaracions globals de main.cpp (bucle GLFW, càmera, il·luminació i callbacks)
//
/////////////////////////////////////////////////////////////////////////////


// Entorn VGI: Llibreries i constants Constants de l'aplicació EntornVGI
#include "framework.h"

//-------------- Entorn VGI: Variables globals de main

// Retrieving main monitor
GLFWmonitor* primary;

// To get current video mode of a monitor
const GLFWvidmode* mode;

// Entorn VGI: Finestra principal 
GLFWwindow* window;

// Entorn VGI: Variables de control per Menú CAMERA: Esfèrica, Navega, Mòbil, Zoom, Polars... 
char camera;	// Variable que controla el tipus de càmera segons valors definits en constants.h
bool mobil;		// Opció canvi de Punt de Vista interactiu (mobil) [0:NO,1:SI]
bool zzoom;		// Opció Zoom interactiu (zoom) [0:NO,1:SI]

// Entorn VGI: Variables de control de l'opció CAMERA->ESFÈRICA?
float cam_Esferica[3];		// ImGui: Coordenades Esfèriques Càmera
CEsfe3D OPV;				// Paràmetres camera en coord. esfèriques (R,alfa,beta)
char Vis_Polar;				// Variable que controla orientació dels eixos en Visualització Interactiva (POLARZ,POLARY,POLARX)
glm::mat4 ViewMatrix(1.0);	// Matriu de Càmera a passar al shader

// Entorn VGI: Variables de control de l'opció CÀMERA->NAVEGA?
GLdouble n[3];				// Punt cap on mira.
CPunt3D opvN;				// Definició Orígen Punt de Vista (en coordenades món)
double angleZ;				// angle de rotació de la trajectòria.
double fact_pan;			// Factor de desplaçament de la càmera en navegació lliure.

// Entorn VGI: Variables de control per Menú VISTA: fullscreen, dibuixar eixos i grid. 
bool fullscreen;	// Opció booleana per Pantalla Completa [0:NO,1:SI]
bool eixos;			// Opció per dibuixar els eixos X,Y,Z (Eixos?) [0:NO,1:SI]
Shader shaderEixos;	// Identificador pel shader d'eixos.
GLuint eixos_programID;
GLuint eixos_Id;	// Identificador del VBO per a dibuixar els eixos.
CMask3D grid;		// Opció per a dibuixar grids (.x: grid XY, .y:grid XZ, .z: grid YZ, .w:grid XYZ)
CPunt3D hgrid;		// Alçada de cada grid (.x: grid XY, .y:grid XZ, .z: grid YZ, .w:grid XYZ)

// Entorn VGI: Variables de control per les opcions de menú PROJECCIÓ, OBJECTE
char projeccio;					// Menú Projecció: Tipus de projeccio
glm::mat4 ProjectionMatrix(1.0);// Matriu de Projecció a passar al shader
char objecte;					// Menú Objecte: Tipus d'Objecte

// Entorn VGI: Variables de control Skybox Cube
bool SkyBoxCube;			// Booleana que controla si es visualitza Skybox [TRUE] o no [FALSE].
Shader shader_SkyBoxC;		// Shader Skybox Cub
GLuint skC_programID;		// Identificador program Skybox Cube
CVAO skC_VAOID;				// Identificador VAO List (vaoId, vboId, nVertexs) per a Skybox Cube
GLuint cubemapTexture;		// Identificador textura cubemap.

// Entorn VGI: Variables de control per les opcions de menú OCULTACIONS
bool test_vis;  // Menú ocultacions: Activa [true] o desactiva [false] Test Visibilitat.
bool oculta;    // Menú ocultacions: Activa [true] o desactiva [false] Ocultacions (Z-buffer).

// Entorn VGI: Variables de control del menú ILUMINACIÓ
bool ifixe;         // Iluminació fixe independent del PV (ifixe=1) 
// o depenent (mobil) respecte el PV (casc miner) (ifixe=0)
char ilumina;		// Tipus d'il.luminació [FILFERROS 'w',PLANA 'f',SUAU 's']
bool sw_material[5];// Variable que controla els coeficients de reflectivitat del material [TRUE] o no [FALSE]:
// [0]: emission, [1]: ambient, [2]: difusa, [3] especular.
bool tFlag_invert_Y; // Booleana que activa la inversió coordenada textura t (o Y) a 1.0-cty segons llibreria SOIL (TRUE) o no (FALSE).

// Entorn VGI: Variables de control del menú LLUMS
bool llum_ambient;		// Booleana que controla la llum ambient (SI/NO).
LLUM llumGL[NUM_MAX_LLUMS];		// Vector de llums d'OpenGL

// Entorn VGI: Variables de control del menú SHADER
char shader;				// Tipus de shader [FLAT, GOURAUD, PHONG, FILE, PROG_BINARY_SHADER]
GLuint shader_programID;	// Shader Program que conté el Vertex i Fragment program.
Shader shaderLighting;		// Shader que implementa els codis GLSL d'il.luminació.

// Entorn VGI: Variables de control dels botons de mouse
CPoint m_PosEAvall, m_PosDAvall;	// Coordenades del cursor quan el boto esquerre(E) o dret(D) del 
//    mouse ha estat clicat.
bool m_ButoEAvall, m_ButoDAvall;	// TRUE si el boto esquerre(E) o dret(D) del mouse esta clicat.

// Entorn VGI: Variables que controlen paràmetres visualització: Mides finestra Windows i PV (càmera)
int w, h;				// Mides de la finestra Windows (w-amplada,h-alçada)
int width_old, height_old;	// Mides de la resolució actual de la pantalla (finestra Windows)

// Entorn VGI: Color de fons
CColor c_fons;				// Intensitat de color de fons.
//-------------- Entorn VGI: Fi De Variables globals de main

//-------------- Entorn VGI: Declaració funcions main

// Inicialització variables de control
void InitGL();

void GetGLVersion(int* major, int* minor);

// Entorn VGI: Control de l'EVENT ONSIZE
void OnSize(GLFWwindow* window, int width, int height);

void OnPaint(GLFWwindow* window);
void dibuixa_Escena();

// EntornVGI: Funcions de mostrar Finestres ImGui
void draw_Menu_ImGui();

// EntornVGI: RECURSOS DE MENU (persianes) DE L'APLICACIO:
void OnCameraNavega();
void OnVistaSkyBox();
void OnProjeccioPerspectiva();
void OnShaderPhong();

// Entorn VGI: Control de l'EVENT TECLAT
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mods);

// Entorn VGI: Funcions de tractament de teclat en diferents modus
void Teclat_Navega(int key, int action);

// Entorn VGI: Control de l'EVENT MOUSE
void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
void OnMouseMove(GLFWwindow* window, double xpos, double ypos);
void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset);

// ---------------- Entorn VGI: Funcions locals a main.cpp
void OnFull_Screen(GLFWmonitor* monitor, GLFWwindow* window);
// ---------------- Entorn VGI: Tractament d'errors
void error_callback(int code, const char* description);
GLenum glCheckError_(const char* file, int line);
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam);
