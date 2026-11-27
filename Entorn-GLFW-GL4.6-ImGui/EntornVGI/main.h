//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// main.h : Interfície i declaracions globals de main.cpp (bucle GLFW, càmera, il·luminació i callbacks)
//
/////////////////////////////////////////////////////////////////////////////


// Entorn VGI: Llibreries i constants Constants de l'aplicació EntornVGI
#include "framework.h"
#include "objLoader.h"	// COBJModel (nau Milano)

//-------------- Entorn VGI: Variables globals de main

// Retrieving main monitor
GLFWmonitor* primary;

// To get current video mode of a monitor
const GLFWvidmode* mode;

// Entorn VGI: Finestra principal 
GLFWwindow* window;

// EntornVGI.ImGui: Gestió de finestres de menú ImGui
bool show_demo_window = false;
bool show_another_window = false;

// Entorn VGI: Variables de control per Menú CAMERA: Esfèrica, Navega, Mòbil, Zoom, Satelit, Polars... 
char camera;	// Variable que controla el tipus de càmera segons valors definits en constants.h
bool mobil;		// Opció canvi de Punt de Vista interactiu (mobil) [0:NO,1:SI]
bool zzoom;		// Opció Zoom interactiu (zoom) [0:NO,1:SI]
bool zzoomO;	// Opció Zoom en Projecció Ortogràfica adaptant Volum Visualització [0:NO,1:SI]
bool satelit;	// Opció de navegació animada al volt de l'objecte centrat definint increment per mouse (SATÈLIT)

// Entorn VGI: Variables de control de l'opció CAMERA->ESFÈRICA?
float cam_Esferica[3];		// ImGui: Coordenades Esfèriques Càmera
CEsfe3D OPV;				// Paràmetres camera en coord. esfèriques (R,alfa,beta)
char Vis_Polar;				// Variable que controla orientació dels eixos en Visualització Interactiva (POLARZ,POLARY,POLARX)
glm::mat4 ViewMatrix(1.0);	// Matriu de Càmera a passar al shader

// Entorn VGI: Variables de control de l'opció CÀMERA->NAVEGA?
GLdouble n[3];				// Punt cap on mira.
CPunt3D opvN;				// Definició Orígen Punt de Vista (en coordenades món)
double angleZ;				// angle de rotació de la trajectòria.

// Entorn VGI: Variables de control per Menú VISTA: fullsecreen, pan, dibuixar eixos i grid. 
bool fullscreen;	// Opció booleana per Pantalla Completal [0:NO,1:SI]
bool pan;			// Opcio Pan. Desplaçament del centre de l'esfera de Vista [0:NO,1:SI]
bool eixos;			// Opció per dibuixar els eixos X,Y,Z (Eixos?) [0:NO,1:SI]
Shader shaderEixos;	// Ientificador pel shader d'eixos.
GLuint eixos_programID;
GLuint eixos_Id;	// Identificador del VBO per a dibuixar els eixos.
CMask3D grid;		// Opció per a dibuixar grids (.x: grid XY, .y:grid XZ, .z: grid YZ, .w:grid XYZ)
CPunt3D hgrid;		// Alçada de cada grid (.x: grid XY, .y:grid XZ, .z: grid YZ, .w:grid XYZ)

// Entorn VGI: Variables de l'opció VISTA->PAN?
double fact_pan;	// Factor de desplaçament de la càmara (opció pan).
CPunt3D tr_cpv;		// Vector de Traslació del Centre del Punt de Vista actiu dins opció pan.
CPunt3D tr_cpvF;	// Vector de Traslació del Centre del Punt de Vista (fixat amb INSERT dins opció pan) 
//       i actiu fora l'opció pan.

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
Shader shaderSkyBox;		// Shader SkyBox

// Entorn VGI: Variables de control del menú TRANSFORMA
bool transf;	// Booleana per activar Transformacions (true) o no (false).
bool trasl;		// Booleana d'activació de la Traslació (true) o no (false).
bool rota;		// Booleana d'activació de la Rotació (true) o no (false).
bool escal;		// Booleana d'activació de l'Escalatge (true) o no (false).
double fact_Tras, fact_Rota;	// Increments de Traslació i Rotació.
INSTANCIA TG;	// Estructura que conté TG del menú Transforma actiu dins qualsevol opció de Transforma 
//      (Traslació Rotació i Escalatge).
INSTANCIA TGF;	// Estructura que conté TG del menú Transforma fixat amb INSERT dins les opcions de Transforma
//     i actiu fora l'opció Transforma.
bool transX;	// Opció Mobil Eix X?: TG interactives per l'eix X via mouse [F:NO,T:SI].
bool transY;	// Opció Mobil Eix Y?: TG interactives per l'eix Y via mouse [F:NO,T:SI].
bool transZ;	// Opció Mobil Eix Z?: TG interactives per l'eix Z via mouse [F:NO,T:SI].
glm::mat4 GTMatrix; // Matriu de Transformacions Geomètriques a passar al shader

// Entorn VGI: Variables de control per les opcions de menú OCULTACIONS
bool front_faces;// Menú ocultacions: Determina si les cares visibles són les frontals [true-defecte] o les back [false] pel Test de Visibilitat.
bool test_vis;  // Menú ocultacions: Activa [true] o desactiva [false] Test Visibilitat.
bool oculta;    // Menú ocultacions: Activa [true] o desactiva [false] Ocultacions (Z-buffer).
bool back_line; // Menú ocultacions: Activa [true] o desactiva [false] dibuix cares back com a línies.

// Entorn VGI: Variables de control del menú ILUMINACIÓ
bool ifixe;         // Iluminació fixe independent del PV (ifixe=1) 
// o depenent (mobil) respecte el PV (casc miner) (ifixe=0)
bool ilum2sides;	// Iluminació de les cares FRONT i BACK [TRUE] o només les FRONT [FALSE]
char ilumina;		// Tipus d'il.luminació [FILFERROS 'f',PLANA 'c',SUAU 'c']
bool sw_material[5];// Variable que controla els coeficients de reflectivitat del material [TRUE] o no [FALSE]:
// [0]: emission, [1]: ambient, [2]: difusa, [3] especular.
bool sw_material_old[5]; // Variable que guarda els valors de sw_material mentre no hi ha reflectivitat de material (refl_material=false).
bool textura;       // Control de textures desactivades [0:NO] o activades [1:SI]
char t_textura;		// Tipus de textures (predefinides o per fitxer)
bool tFlag_invert_Y; // Booleana que activa la inversió coordenada textura t (o Y) a 1.0-cty segons llibreria SOIL (TRUE) o no (FALSE).

// Entorn VGI: Variables de control del menú LLUMS
bool llum_ambient;		// Booleana que controla la llum ambient (SI/NO).
LLUM llumGL[NUM_MAX_LLUMS];		// Vector de llums d'OpenGL

// Entorn VGI: Variables de control del menú SHADER
char shader;				// Tipus de shader [FLAT, GOURAUD, PHONG, FILE, PROG_BINARY_SHADER]
GLuint shader_programID;	// Shader Program que conté el Vertex i Fragment program.
GLuint instancing_programID;
GLuint shader_programIDNormalMap;	// Shader Program que conté el Vertex i Fragment program per a normal mapping.
Shader shaderLightingNormalMap;		// Shader que implementa els codis GLSL d'il.luminació.
Shader shaderLighting;		// Shader que implementa els codis GLSL d'il.luminació.

// COET: Model OBJ de la nau Milano (es carrega en activar la camera Navega)
COBJModel* ObOBJ = NULL;

// Entorn VGI: Variables de control dels botons de mouse
CPoint m_PosEAvall, m_PosDAvall;	// Coordenades del cursor quan el boto esquerre(E) o dret(D) del 
//    mouse ha estat clicat.
bool m_ButoEAvall, m_ButoDAvall;	// TRUE si el boto esquerre(E) o dret(D) del mouse esta clicat.
CEsfe3D m_EsfeEAvall;				// Coordenades Esfèriques del PV (OPV) quan el boto esquerre(E) o dret(D) del 
//		mouse ha estat clicat.
CEsfe3D m_EsfeIncEAvall;			// Increment de desplaçament en coordenades Esfèriques del PV (OPV).

// Entorn VGI: Variables que controlen paràmetres visualització: Mides finestra Windows i PV (càmera)
int w, h;				// Mides de la finestra Windows (w-amplada,h-alçada)
int width_old, height_old;	// Mides de la resolució actual de la pantalla (finestra Windows)

// Entorn VGI: Color de fons i de l'objecte
CColor c_fons;				// Intensitat de color de fons.
ImVec4 clear_colorB;		// ImGui: Color de fons
ImVec4 clear_colorO;		// ImGui: Color de l'objecte.
CColor col_obj;				// Color de l'objecte simple.

// Entorn VGI: Variables de visualització i altres
int pas;			// Resolució passada a les funcions de càmera.
GLdouble mida;		// Factor d'escala per calcular Volum de Visualització de l'objecte que encaixi.
//-------------- Entorn VGI: Fi De Variables globals de main

//-------------- Entorn VGI: Declaració funcions main

// Inicialització variables de control
void InitGL();

void GetGLVersion(int* major, int* minor);

// Entorn VGI: Control de l'EVENT ONSIZE
void OnSize(GLFWwindow* window, int width, int height);

void OnPaint(GLFWwindow* window);
void configura_Escena();
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
void OnKeyUp(GLFWwindow* window, int key, int scancode, int action, int mods);
void OnTextDown(GLFWwindow* window, unsigned int codepoint);

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
