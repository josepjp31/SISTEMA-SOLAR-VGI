//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// main.h — Arquitectura: Nucli d'Aplicació / Bucle GLFW
// Declaració de variables globals d'estat OpenGL, càmera, il·luminació, finestres i callbacks d'entrada/renderitzat.
//
/////////////////////////////////////////////////////////////////////////////


// Entorn VGI: Llibreries i constants de l'aplicació EntornVGI
#include "framework.h"

//-------------- Entorn VGI: Variables globals de main

// Monitor principal detectat per GLFW
GLFWmonitor* primary;

// Mode de vídeo actual del monitor principal
const GLFWvidmode* mode;

// Entorn VGI: Finestra principal GLFW
GLFWwindow* window;

// Entorn VGI: Variables de control per Menú CAMERA: Esfèrica, Navega, Mòbil, Zoom, Polars... 
char camera;	// Tipus de càmera actiu (CAM_ESFERICA, CAM_NAVEGA o CAP)
bool mobil;		// Permet rotació interactiva del punt de vista amb ratolí [true/false]
bool zzoom;		// Permet zoom interactiu amb roda/arrossegament [true/false]

// Entorn VGI: Variables de control de l'opció CAMERA->ESFÈRICA?
float cam_Esferica[3];		// [0]=R [unitats OpenGL], [1]=alfa [graus], [2]=beta [graus]
CEsfe3D OPV;				// Paràmetres de càmera en coordenades esfèriques (R [u], alfa [graus], beta [graus])
char Vis_Polar;				// Orientació dels eixos en vista esfèrica (POLARZ, POLARY, POLARX)
glm::mat4 ViewMatrix(1.0);	// Matriu de Vista 4x4 enviada al shader

// Entorn VGI: Variables de control de l'opció CÀMERA->NAVEGA?
GLdouble n[3];				// [unitats OpenGL] Punt 3D cap on mira la càmera en mode Navega
CPunt3D opvN;				// [unitats OpenGL] Posició de l'observador en coordenades món (Navega)
double angleZ;				// [graus] Angle de gir en pla horitzontal durant la navegació lliure
double fact_pan;			// [unitats OpenGL/pas] Velocitat de desplaçament en navegació lliure

// Entorn VGI: Variables de control per Menú VISTA: fullscreen, dibuixar eixos i grid. 
bool fullscreen;	// Estat de pantalla completa [true/false]
bool eixos;			// Activa o desactiva el dibuix dels eixos de coordenades [true/false]
Shader shaderEixos;	// Shader dedicat al renderitzat dels eixos de coordenades
GLuint eixos_programID; // Identificador OpenGL del programa shader d'eixos
GLuint eixos_Id;	// Identificador VAO dels eixos de coordenades
CMask3D grid;		// Màscara booleana de reixes actives (XY, XZ, YZ, XYZ)
CPunt3D hgrid;		// [unitats OpenGL] Alçada/desplaçament de cada reixa

// Entorn VGI: Variables de control per les opcions de menú PROJECCIÓ, OBJECTE
char projeccio;					// Tipus de projecció activa (PERSPECT o CAP)
glm::mat4 ProjectionMatrix(1.0);// Matriu de Projecció 4x4 enviada al shader
char objecte;					// Tipus d'escena/objecte actiu (SISTEMA_SOLAR o CAP)

// Entorn VGI: Variables de control Skybox Cube
bool SkyBoxCube;			// Controla si es renderitza el cubemap Skybox [true/false]
Shader shader_SkyBoxC;		// Shader dedicat al cub Skybox
GLuint skC_programID;		// Identificador OpenGL del programa shader Skybox
CVAO skC_VAOID;				// Descriptor VAO/VBO del cub Skybox
GLuint cubemapTexture;		// Identificador OpenGL de la textura cubemap estel·lar

// Entorn VGI: Variables de control per les opcions de menú OCULTACIONS
bool test_vis;  // Activa [true] o desactiva [false] el backface culling (GL_CULL_FACE)
bool oculta;    // Activa [true] o desactiva [false] el test de profunditat Z-buffer (GL_DEPTH_TEST)

// Entorn VGI: Variables de control del menú ILUMINACIÓ
bool ifixe;         // Il·luminació fixa en coordenades món [true] o lligada al punt de vista [false]
char ilumina;		// Tipus d'il·luminació/poligonització (FILFERROS, PLANA, SUAU)
bool sw_material[5];// Interruptors de material: [0]=emission, [1]=ambient, [2]=diffuse, [3]=specular, [4]=sw_material
bool tFlag_invert_Y;// Inverteix la coordenada V de textura (1.0 - v) per compatibilitat SOIL2 [true/false]

// Entorn VGI: Variables de control del menú LLUMS
bool llum_ambient;		// Activa [true] o desactiva [false] la llum ambient global
LLUM llumGL[NUM_MAX_LLUMS];		// Vector de fins a 8 fonts de llum configurades al shader

// Entorn VGI: Variables de control del menú SHADER
char shader;				// Tipus de shader actiu (PHONG_SHADER o CAP_SHADER)
GLuint shader_programID;	// Identificador del programa shader Phong principal
Shader shaderLighting;		// Gestor GLSL del shader d'il·luminació Phong multi-llum

// Entorn VGI: Variables de control dels botons de mouse
CPoint m_PosEAvall, m_PosDAvall;	// [píxels] Darrer punt clicat amb el botó esquerre (E) o dret (D)
bool m_ButoEAvall, m_ButoDAvall;	// true mentre el botó esquerre (E) o dret (D) del ratolí roman premut

// Entorn VGI: Variables que controlen paràmetres visualització: Mides finestra Windows i PV (càmera)
int w, h;				// [píxels] Amplada i alçada actuals de la finestra / framebuffer
int width_old, height_old;	// [píxels] Amplada i alçada prèvies abans de passar a pantalla completa

// Entorn VGI: Color de fons
CColor c_fons;				// [RGBA 0.0..1.0] Color de neteja del fons OpenGL
//-------------- Entorn VGI: Fi De Variables globals de main

//-------------- Entorn VGI: Declaració funcions main

// Precondició: context OpenGL creat. Inicialitza càmera, llums, shaders i variables globals per defecte.
void InitGL();

// Llegeix la versió OpenGL suportada pel context actiu i la desa a `*major` i `*minor`.
void GetGLVersion(int* major, int* minor);

// Callback GLFW de redimensionament: actualitza les variables globals `w` i `h` [píxels].
void OnSize(GLFWwindow* window, int width, int height);

// Precondició: finestra i context OpenGL actius. Sincronitza l'estat, calcula projecció/vista i dibuixa el frame 3D.
void OnPaint(GLFWwindow* window);

// Invoca el dibuix seqüencial de Skybox, eixos de coordenades i escena 3D (`dibuixa_EscenaGL`).
void dibuixa_Escena();

// Precondició: `OnPaint` ja ha actualitzat `ViewMatrix` i `ProjectionMatrix`.
// Construeix el frame ImGui, dibuixa els menús/panells i projecta etiquetes i marcadors 2D dels astres.
void draw_Menu_ImGui();

// Activa el mode de càmera lliure (`CAM_NAVEGA`) i inicialitza el punt de vista.
void OnCameraNavega();

// Activa el cubemap Skybox i carrega les 6 textures de fons estel·lar si encara no s'han carregat.
void OnVistaSkyBox();

// Activa la projecció perspectiva (`PERSPECT`) i crida `OnPaint`.
void OnProjeccioPerspectiva();

// Carrega i activa el shader Phong multi-llum (`phong_shdrML.vert` / `phong_shdrML.frag`).
void OnShaderPhong();

// Callback GLFW de teclat per a dreceres globals (Fullscreen, Pausa, Eixos, Càmera) i moviment en mode Navega.
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mods);

// Processa les tecles de desplaçament i gir quan la càmera activa és `CAM_NAVEGA`.
void Teclat_Navega(int key, int action);

// Callback GLFW de botons del ratolí (ignora l'esdeveniment si ImGui captura el ratolí).
void OnMouseButton(GLFWwindow* window, int button, int action, int mods);

// Callback GLFW de moviment del ratolí: orbita la càmera esfèrica en arrossegar amb el botó esquerre.
void OnMouseMove(GLFWwindow* window, double xpos, double ypos);

// Callback GLFW de la roda del ratolí: aplica zoom lineal o logarítmic segons `ConfigCamera`.
void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset);

// ---------------- Entorn VGI: Funcions locals a main.cpp
// Commuta entre finestra i pantalla completa conservant `width_old`/`height_old`.
void OnFull_Screen(GLFWmonitor* monitor, GLFWwindow* window);
// ---------------- Entorn VGI: Tractament d'errors
// Callback d'errors interns de GLFW.
void error_callback(int code, const char* description);
// Comprova `glGetError()` i informa per consola de l'arxiu i línia on s'ha produït. Retorn: codi GLenum.
GLenum glCheckError_(const char* file, int line);
// Callback de missatges de depuració del context OpenGL 4.6 (`GL_DEBUG_OUTPUT`).
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam);