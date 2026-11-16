
//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2026)
// main.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//    Versió 0.5:	- Interficie ImGui
//					- Per a dialeg de cerca de fitxers, s'utilitza la llibreria NativeFileDialog
//

// Entorn VGI.ImGui: Includes llibreria ImGui
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_glfw.h"
#include "ImGui\imgui_impl_opengl3.h"

#include "framework.h"
#include "shader.h"
#include "visualitzacio.h"
#include "escena.h"
#include "main.h"
#include "escenari.h"
#include "EscenariIntuitiu.h"
#include "EscenariRealista.h"
#include "InterficieUI.h"

// Escenari actiu i llista d'escenaris polimorfics (0: Mode Didactic, 1: Mode a Escala Real)
Escenari* escenariActual = nullptr;
Escenari* escenaris[2] = { nullptr, nullptr };

// Estat global de la interficie d'usuari del simulador
EstatSimulacio g_EstatSimulador;

void InitGL()
{
	//------ Entorn VGI: Inicialització de les variables globals de CEntornVGIView
	int i;

	// Entorn VGI: Variables de control per Menú CÀMERA: Esfèrica, Navega, Mòbil, Zoom, Polars... 
	camera = CAM_ESFERICA;
	mobil = true;			zzoom = true;

	// Entorn VGI: Variables de control de l'opció CAMERA->ESFÈRICA?
	//(NOA)
	cam_Esferica[0] = 100.0f;	
	cam_Esferica[1] = 25.0f;			
	cam_Esferica[2] = 0.0f;

	OPV.R = cam_Esferica[0];	
	OPV.alfa = cam_Esferica[1];		
	OPV.beta = cam_Esferica[2];		// Origen PV en esfèriques
	Vis_Polar = POLARZ;
	ViewMatrix = glm::mat4(1.0);		// Inicialitzar a identitat

	// Entorn VGI: Variables de control de l'opció CÀMERA->NAVEGA?
	n[0] = 0.0;		n[1] = 0.0;		n[2] = 0.0;
	opvN.x = 10.0;	opvN.y = 0.0;		opvN.z = 0.0;
	angleZ = 0.0;
	fact_pan = 1;

	// Entorn VGI: Variables de control per Menú VISTA: Pantalla Completa, dibuixar eixos i grids 
	fullscreen = false;
	eixos = true;	eixos_programID = 0;  eixos_Id = 0;
	grid.x = false;	grid.y = false;		grid.z = false;		grid.w = false;
	hgrid.x = 0.0;	hgrid.y = 0.0;		hgrid.z = 0.0;		hgrid.w = 0.0;

	// Entorn VGI: Variables de control per les opcions de menú Projecció, Objecte
	projeccio = CAP;	// projeccio = PERSPECT;
	ProjectionMatrix = glm::mat4(1.0);	// Inicialitzar matriu a identitat
	objecte = CAP;

	// Entorn VGI: Variables de control Skybox Cube
	SkyBoxCube = false;
	skC_programID = 0;
	skC_VAOID.vaoId = 0;	skC_VAOID.vboId = 0;	skC_VAOID.nVertexs = 0;
	cubemapTexture = 0;

	// Entorn VGI: Variables de control per les opcions de menú OCULTACIONS
	test_vis = false;	oculta = false;

	// Entorn VGI: Variables de control del menú ILUMINACIÓ
	ifixe = false;					ilumina = FILFERROS;
	// Reflexions actives: Ambient [1], Difusa [2] i Especular [3]. No actives: Emission [0]. 
	sw_material[0] = false;			sw_material[1] = true;			sw_material[2] = true;			sw_material[3] = true;	sw_material[4] = true;
	tFlag_invert_Y = false;

	// ----------------- Entorn VGI: Variables de control del menú LLUMS
		// Entorn VGI: Inicialització variables Llums
	llum_ambient = true;		llumGL[0].encesa = true;
	for (i = 1; i < NUM_MAX_LLUMS; i++) llumGL[i].encesa = false;
	for (i = 0; i < NUM_MAX_LLUMS; i++)
	{
		llumGL[i].posicio.x = 0.0;		llumGL[i].posicio.y = 0.0;		llumGL[i].posicio.z = 0.0;		llumGL[i].posicio.w = 1.0;
		llumGL[i].difusa.r = 1.0f;		llumGL[i].difusa.g = 1.0f;		llumGL[i].difusa.b = 1.0f;		llumGL[i].difusa.a = 1.0f;
		llumGL[i].especular.r = 1.0f;	llumGL[i].especular.g = 1.0f;	llumGL[i].especular.b = 1.0f;	llumGL[i].especular.a = 1.0f;
		llumGL[i].atenuacio.a = 0.0;	llumGL[i].atenuacio.b = 0.0;	llumGL[i].atenuacio.c = 1.0;
		llumGL[i].restringida = false;
		llumGL[i].spotdirection.x = 0.0;	llumGL[i].spotdirection.y = 0.0;	llumGL[i].spotdirection.z = 0.0; llumGL[i].spotdirection.w = 0.0;
		llumGL[i].spotcoscutoff = 0.0;		llumGL[i].spotexponent = 0.0;
	}

	// ---------------- LLUM #0 - (+Z) no restringida, amb atenuació constant (a,b,c) = (0,0,1)
	// Posició de la font de llum (x,y,z)=(0,200,0):
	llumGL[0].posicio.x = 0.0;			llumGL[0].posicio.y = 0.0;			llumGL[0].posicio.z = 200.0;	llumGL[0].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,1,1):
	llumGL[0].difusa.r = 1.0f;			llumGL[0].difusa.g = 1.0f;			llumGL[0].difusa.b = 1.0f;		llumGL[0].difusa.a = 1.0f;
	llumGL[0].especular.r = 1.0f;		llumGL[0].especular.g = 1.0f;		llumGL[0].especular.b = 1.0f;	llumGL[0].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[0].atenuacio.a = 0.0;		llumGL[0].atenuacio.b = 0.0;		llumGL[0].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[0].restringida = false;
	llumGL[0].spotdirection.x = 0.0;	llumGL[0].spotdirection.y = 0.0;	llumGL[0].spotdirection.z = -1.0;
	llumGL[0].spotcoscutoff = cos(25.0 * PI / 180);		llumGL[0].spotexponent = 1.0;		// llumGL[0].spotexponent = 45.0; Model de Warn (10, 500)

	// Activació font de llum: ENCESA
	llumGL[0].encesa = true;

	// ---------------- LLUM #1 - (+X) no restringida, amb atenuació constant (a,b,c) = (0,0,1)
	// Posició de la font de llum (x,y,z)=(75,0,0):
	llumGL[1].posicio.x = 75.0;			llumGL[1].posicio.y = 0.0;			llumGL[1].posicio.z = 0.0;		llumGL[1].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,1,1):
	llumGL[1].difusa.r = 1.0f;			llumGL[1].difusa.g = 1.0f;			llumGL[1].difusa.b = 1.0f;		llumGL[1].difusa.a = 1.0f;
	llumGL[1].especular.r = 1.0f;		llumGL[1].especular.g = 1.0f;		llumGL[1].especular.b = 1.0f;	llumGL[1].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[1].atenuacio.a = 0.0;		llumGL[1].atenuacio.b = 0.0;		llumGL[1].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[1].restringida = false;
	llumGL[1].spotdirection.x = 0.0;	llumGL[1].spotdirection.y = 0.0;	llumGL[1].spotdirection.z = 0.0;
	llumGL[1].spotcoscutoff = 0.0;		llumGL[1].spotexponent = 0.0;

	// Activació font de llum: APAGADA
	llumGL[1].encesa = false;

	// ---------------- LLUM #2 - (+Y) no restringida, amb atenuació constant (a,b,c) = (0,0,1)
	// Posició de la font de llum (x,y,z)=(0,75,0):
	llumGL[2].posicio.x = 0.0;			llumGL[2].posicio.y = 75.0;			llumGL[2].posicio.z = 0.0;		llumGL[2].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,1,1):
	llumGL[2].difusa.r = 1.0f;			llumGL[2].difusa.g = 1.0f;			llumGL[2].difusa.b = 1.0f;		llumGL[2].difusa.a = 1.0f;
	llumGL[2].especular.r = 1.0f;		llumGL[2].especular.b = 1.0f;		llumGL[2].especular.b = 1.0f;	llumGL[2].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum amb atenuació per distància (a,b,c)=(0,0.025,1):
	llumGL[2].atenuacio.a = 0.0;		llumGL[2].atenuacio.b = 0.0;		llumGL[2].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[2].restringida = false;
	llumGL[2].spotdirection.x = 0.0;	llumGL[2].spotdirection.y = -1.0;	llumGL[2].spotdirection.z = 0.0;
	llumGL[2].spotcoscutoff = cos(2.5 * PI / 180);							llumGL[2].spotexponent = 1.0;

	// Activació font de llum: APAGADA
	llumGL[2].encesa = false;

	// ---------------- LLUM #3 - (Y=X), restringida amb 25 graus obertura i exponent 45, amb atenuació constant (a,b,c) = (0,0,1)
	// Posició de la font de llum (x,y,z)=(75,75,75):
	llumGL[3].posicio.x = 75.0;			llumGL[3].posicio.y = 75.0;			llumGL[3].posicio.z = 75.0;		llumGL[3].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (0,1,0):
	llumGL[3].difusa.r = 0.0f;			llumGL[2].difusa.g = 1.0f;			llumGL[3].difusa.b = 0.0f;		llumGL[3].difusa.a = 1.0f;
	llumGL[3].especular.r = 0.0f;		llumGL[2].especular.g = 1.0f;		llumGL[3].especular.b = 0.0f;	llumGL[3].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[3].atenuacio.a = 0.0;		llumGL[3].atenuacio.b = 0.0;		llumGL[3].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[3].restringida = true;
	llumGL[3].spotdirection.x = -1.0;	llumGL[3].spotdirection.y = -1.0;	llumGL[3].spotdirection.z = -1.0;
	llumGL[3].spotcoscutoff = cos(25.0 * PI / 180);							llumGL[3].spotexponent = 45.0;

	// Activació font de llum: APAGADA
	llumGL[3].encesa = false;

	// ---------------- LLUM #4 - (-Z), no restringida, amb atenuació constant (a,b,c) = (0,0,1)
	// Posició de la font de llum (x,y,z)=(0,0,-75):
	llumGL[4].posicio.x = 0.0;			llumGL[4].posicio.y = 0.0;			llumGL[4].posicio.z = -75.0;	llumGL[4].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,1,1):
	llumGL[4].difusa.r = 1.0f;			llumGL[4].difusa.g = 1.0f;			llumGL[4].difusa.b = 1.0f;		llumGL[4].difusa.a = 1.0f;
	llumGL[4].especular.r = 1.0f;		llumGL[4].especular.g = 1.0f;		llumGL[4].especular.b = 1.0f;	llumGL[4].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[4].atenuacio.a = 0.0;		llumGL[4].atenuacio.b = 0.0;		llumGL[4].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[4].restringida = false;
	llumGL[4].spotdirection.x = 0.0;	llumGL[4].spotdirection.y = 0.0;	llumGL[4].spotdirection.z = -1.0;
	llumGL[4].spotcoscutoff = cos(5 * PI / 180);							llumGL[4].spotexponent = 30.0;

	// Activació font de llum: APAGADA
	llumGL[4].encesa = false;

	// ---------------- LLUM #5 - (-X,-Y,-Z), direccional, no restringida, amb atenuació constant (a,b,c) = (0,0,1)
	// Vector de la font de llum direccional (x,y,z)=(-1,-1,-1):
	llumGL[5].posicio.x = -1.0;			llumGL[5].posicio.y = -1.0;			llumGL[5].posicio.z = -1.0;		llumGL[5].posicio.w = 0.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,0,0):
	llumGL[5].difusa.r = 1.0f;			llumGL[5].difusa.g = 0.0f;			llumGL[5].difusa.b = 0.0f;		llumGL[5].difusa.a = 1.0f;
	llumGL[5].especular.r = 1.0f;		llumGL[5].especular.g = 0.0f;		llumGL[5].especular.b = 0.0f;	llumGL[5].especular.a = 1.0f;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[5].atenuacio.a = 0.0;		llumGL[5].atenuacio.b = 0.0;		llumGL[5].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[5].restringida = false;
	llumGL[5].spotdirection.x = 0.0;	llumGL[5].spotdirection.y = 0.0;	llumGL[5].spotdirection.z = 0.0;
	llumGL[5].spotcoscutoff = 0.0;		llumGL[5].spotexponent = 0.0;

	// Activació font de llum: APAGADA
	llumGL[5].encesa = false;

	// ---------------- LLUM #6 - Llum Vaixell, configurada a la funció vaixell() en escena.cpp.
	// Posició de la font de llum (x,y,z)=(-75,75,75):
	llumGL[6].posicio.x = -75.0;		llumGL[6].posicio.y = 75.0;			llumGL[6].posicio.z = 75.0;		llumGL[6].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (1,1,1):
	llumGL[6].difusa.r = 1.0f;			llumGL[6].difusa.g = 1.0f;			llumGL[6].difusa.b = 1.0f;		llumGL[6].difusa.a = 1.0f;
	llumGL[6].especular.r = 1.0f;		llumGL[6].especular.g = 1.0f;		llumGL[6].especular.b = 1.0f;	llumGL[6].especular.a = 1;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[6].atenuacio.a = 0.0;		llumGL[6].atenuacio.b = 0.0;		llumGL[6].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[6].restringida = false;
	llumGL[6].spotdirection.x = 1.0;	llumGL[6].spotdirection.y = -1.0;	llumGL[6].spotdirection.z = -1.0;
	llumGL[6].spotcoscutoff = sqrt(2.0) / 2.0;		llumGL[6].spotexponent = 10.0;

	// Activació font de llum: APAGADA
	llumGL[6].encesa = false;

	// ---------------- LLUM #7 - Llum Far, configurada a la funció faro() en escena.cpp.
	// Posició de la font de llum (x,y,z)=(50,50,-50), però cap posició definida, definida en funció faro() en escena.cpp:
	llumGL[7].posicio.x = 50.0;			llumGL[7].posicio.y = 50.0;			llumGL[7].posicio.z = -50.0;	llumGL[7].posicio.w = 1.0;

	// Intensitats difusa i especular de la font de llum (r,g,b) = (0,1,1):
	llumGL[7].difusa.r = 0.0f;			llumGL[7].difusa.g = 1.0f;			llumGL[7].difusa.b = 1.0f;		llumGL[7].difusa.a = 1.0f;
	llumGL[7].especular.r = 0.0f;		llumGL[7].especular.g = 1.0f;		llumGL[7].especular.b = 1.0f;	llumGL[7].especular.a = 1;

	// Coeficients factor atenuació f_att=1/(ad2+bd+c). Llum sense atenuació per distància (a,b,c)=(0,0,1):
	llumGL[7].atenuacio.a = 0.0;		llumGL[7].atenuacio.b = 0.0;		llumGL[7].atenuacio.c = 1.0;

	// Paràmetres font de llum restringida:
	llumGL[7].restringida = true;
	llumGL[7].spotdirection.x = -1.0;	llumGL[7].spotdirection.y = -1.0;	llumGL[7].spotdirection.z = 1.0;
	llumGL[7].spotcoscutoff = 0.5;		llumGL[7].spotexponent = 5.0;

	// Activació font de llum: APAGADA
	llumGL[7].encesa = false;
	// ---------------- FI DEFINICIÓ LLUMS

	// Entorn VGI: Variables de control del menú SHADERS
	shader = CAP_SHADER;
	shader_programID = 0;
	shaderLighting.releaseAllShaders();

	// Entorn VGI: Inicialitzar llista de VAO's ABANS de carregar el VAO del Skybox per no esborrar-lo
	initVAOList();

	// Entorn VGI: Càrrega SHADERS ---------------------------------------------------
	// Càrrega Shader Eixos
	fprintf(stderr, "Eixos: \n");
	if (!eixos_programID) eixos_programID = shaderEixos.loadFileShaders(".\\shaders\\eixos.VERT", ".\\shaders\\eixos.FRAG");

	// Entorn VGI: Càrrega Shader Skybox
	fprintf(stderr, "SkyBox: \n");
	if (!skC_programID) skC_programID = shader_SkyBoxC.loadFileShaders(".\\shaders\\skybox.VERT", ".\\shaders\\skybox.FRAG");

	// Entorn VGI: Càrrega VAO Skybox Cube
	if (skC_VAOID.vaoId == 0) skC_VAOID = loadCubeSkybox_VAO();
	Set_VAOList(CUBE_SKYBOX, skC_VAOID);

	if (!cubemapTexture)
	{	// load Skybox textures
		// -------------
		std::vector<std::string> faces =
		{ ".\\textures\\skybox\\right.png",
			".\\textures\\skybox\\left.png",
			".\\textures\\skybox\\top.png",
			".\\textures\\skybox\\bottom.png",
			".\\textures\\skybox\\front.png",
			".\\textures\\skybox\\back.png"
		};
		cubemapTexture = loadCubemap(faces);
	}

	// Càrrega Shader de Phong
	shader_programID = 0;
	fprintf(stderr, "Phong_shdrML: \n");
	if (!shader_programID) shader_programID = shaderLighting.loadFileShaders(".\\shaders\\phong_shdrML.vert", ".\\shaders\\phong_shdrML.frag");
	shader = PHONG_SHADER;
	// Entorn VGI: FI Càrrega SHADERS ---------------------------------------------------

	// Entorn VGI: Variables de control dels botons de mouse
	m_PosEAvall.x = 0;			m_PosEAvall.y = 0;			m_PosDAvall.x = 0;			m_PosDAvall.y = 0;
	m_ButoEAvall = false;		m_ButoDAvall = false;

	// Entorn VGI: Variables que controlen paràmetres visualització: Mides finestra Windows i PV (càmera)
	w = 1280;			h = 720;			// Mides de la finestra Windows (w-amplada,h-alçada)
	width_old = 1280;	height_old = 720;	// Mides de la resolució actual de la pantalla (finestra Windows)

	// Entorn VGI: Color de fons
	c_fons.r = 0.0;		c_fons.g = 0.0;		c_fons.b = 0.0;		c_fons.a = 1.0;
}


void GetGLVersion(int* major, int* minor)
{
	// for all versions
	char* ver = (char*)glGetString(GL_VERSION); // ver = "3.2.0"

	*major = ver[0] - '0';
	if (*major >= 3)
	{
		// for GL 3.x
		glGetIntegerv(GL_MAJOR_VERSION, major);		// major = 3
		glGetIntegerv(GL_MINOR_VERSION, minor);		// minor = 2
	}
	else
	{
		*minor = ver[2] - '0';
	}

	// GLSL
	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);	// 1.50 NVIDIA via Cg compiler
}

void OnSize(GLFWwindow* window, int width, int height)
{
	// Entorn VGI: MODIFICACIÓ ->Establim les mides de la finestra actual
	w = width;	h = height;
}


// SincronitzarEstatSimuladorAmbOpenGL: Sincronitza les opcions del menu ImGui (InterficieUI)
// amb les variables internes d'OpenGL (il-luminacio del Sol, Skybox, eixos, camera i focus).
void SincronitzarEstatSimuladorAmbOpenGL()
{
	if (g_EstatSimulador.mostrarSelectorModes) return;

	// 1. Configuracio inicial d'escena, il-luminacio i shaders quan s'inicia o es commuta de mode
	if (g_EstatSimulador.configuracioInicialPendent)
	{
		OnProjeccioPerspectiva();

		// Carregar i activar Shader Phong amb il-luminacio Suau si no esta actiu
		if (shader != PHONG_SHADER) OnShaderPhong();
		ilumina = SUAU;
		test_vis = false;
		oculta = true;

		objecte = SISTEMA_SOLAR;

		// Configuracio dels parametres d'il-luminacio del sistema solar:
		// - Font de llum fixa (#0) situada a l'origen (centre del Sol).
		// - Model de reflexio Phong: emissio propia per al Sol i reflexio difusa per als planetes.
		// - Texturat individual per a cada astre amb inversio d'eix vertical (tFlag_invert_Y).
		// - Vista clara (llum ambient) desactivada per defecte.
		ifixe = true;
		sw_material[4] = true;
		sw_material[0] = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;
		sw_material[1] = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;
		sw_material[2] = true;
		sw_material[3] = false;

		tFlag_invert_Y = true;

		llum_ambient = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;

		llumGL[0].encesa = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;
		llumGL[0].posicio.x = 0.0;		llumGL[0].posicio.y = 0.0;		llumGL[0].posicio.z = 0.0;		llumGL[0].posicio.w = 1.0;
		llumGL[0].difusa.r = 1.0f;		llumGL[0].difusa.g = 1.0f;		llumGL[0].difusa.b = 1.0f;		llumGL[0].difusa.a = 1.0f;
		llumGL[0].especular.r = 1.0f;	llumGL[0].especular.g = 1.0f;	llumGL[0].especular.b = 1.0f;	llumGL[0].especular.a = 1.0f;
		llumGL[0].atenuacio.a = (escenariActual != nullptr)
			? static_cast<GLdouble>(escenariActual->getConfigCamera().atenuacioQuad)
			: ((g_EstatSimulador.modeActual == ModeSimulacio::INTUITIU) ? 0.00004 : 1.0e-10);
		llumGL[0].atenuacio.b = 0.0;	llumGL[0].atenuacio.c = 1.0;
		llumGL[0].restringida = false;
		for (int i = 1; i < NUM_MAX_LLUMS; i++) llumGL[i].encesa = false;

		glUseProgram(shader_programID);
		glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
		glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), tFlag_invert_Y);

		// Assegurar que l'escenari te els VAOs i textures vinculats al shader Phong actual
		if (escenariActual != nullptr)
		{
			escenariActual->creaEscenari(shader_programID);
		}

		// Assegurar que el Skybox i els Eixos estan carregats
		if (!eixos_Id) eixos_Id = deixos();
		SkyBoxCube = g_EstatSimulador.opcionsVisualitzacio.mostrarSkybox;
		if (SkyBoxCube) OnVistaSkyBox();

		g_EstatSimulador.configuracioInicialPendent = false;
	}

	// 2. Sincronitzacio continua de les opcions de Visualitzacio del menu superior
	if (objecte == SISTEMA_SOLAR)
	{
		SkyBoxCube = g_EstatSimulador.opcionsVisualitzacio.mostrarSkybox;
		if (SkyBoxCube && skC_VAOID.vaoId == 0) OnVistaSkyBox();

		eixos = g_EstatSimulador.opcionsVisualitzacio.mostrarEixos;

		sw_material[0] = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;
		llumGL[0].encesa = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;

		llum_ambient = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;
		sw_material[1] = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;
	}

	// 3. Aplicacio dels canvis de mode de camera o d'astre enfocat des del menu
	if (g_EstatSimulador.canviCameraPendent && escenariActual != nullptr)
	{
		const ConfigCamera& cfgCam = escenariActual->getConfigCamera();
		int numCossos = escenariActual->getNumCossos();
		int cid = g_EstatSimulador.cosEnfocatId;
		if (cid < 0 || cid >= numCossos) cid = 0;

		float distPerDefecte = cfgCam.getDistDefecte(cid, escenariActual->getRadiCos(cid));

		if (g_EstatSimulador.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;
			Vis_Polar = POLARZ;
			OPV.alfa = cfgCam.angleInclinacioInicial;
			OPV.R = distPerDefecte;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;
			Vis_Polar = POLARZ;
			if (OPV.alfa == 0.0) OPV.alfa = cfgCam.angleInclinacioInicial;
			OPV.R = distPerDefecte;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;
			Vis_Polar = POLARZ;
			g_EstatSimulador.cosEnfocatId = 0;
			OPV.R = cfgCam.globalSistemaR;
			OPV.alfa = cfgCam.angleGlobal;
			OPV.beta = 0.0;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)
		{
			// Camera Lliure implementada a l'opcio Navega (CAM_NAVEGA)
			OnCameraNavega();
			Vis_Polar = POLARZ;
			fact_pan = cfgCam.navPas;
			// Situar la camera lliure en una posicio inicial privilegiada mirant cap al centre del Sistema Solar
			opvN.x = cfgCam.navPosX;	opvN.y = cfgCam.navPosY;	opvN.z = cfgCam.navPosZ;
			n[0] = 0.0;		n[1] = 0.0;			n[2] = 0.0;
			angleZ = 0.0;
		}

		cam_Esferica[0] = (float)OPV.R;
		cam_Esferica[1] = (float)OPV.alfa;
		cam_Esferica[2] = (float)OPV.beta;
		g_EstatSimulador.canviCameraPendent = false;
	}
}

// OnPaint: Funció de dibuix i visualització en frame buffer del frame
void OnPaint(GLFWwindow* window)
{
	GLdouble vpv[3] = { 0.0, 0.0, 1.0 };

	// Neteja del framebuffer durant la visualitzacio del Selector Inicial de Modes (per no mostrar el sistema)
	if (g_EstatSimulador.mostrarSelectorModes)
	{
		glDisable(GL_SCISSOR_TEST);
		glViewport(0, 0, w, h);
		glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	// Sincronitzar les opcions de la interficie ImGui amb l'estat d'OpenGL
	SincronitzarEstatSimuladorAmbOpenGL();

	// Entorn VGI: Cridem a les funcions de l'escena i la projecció segons s'hagi 
	// seleccionat una projecció o un altra
	switch (projeccio)
	{
	case PERSPECT:
		// Entorn VGI: PROJECCIÓ PERSPECTIVA
		glDisable(GL_SCISSOR_TEST);		// Desactivació del retall de pantalla

		// Entorn VGI: Activar shader Visualització Escena
		glUseProgram(shader_programID);

		// Calcul dinamic de plans near/far en Escala Real per evitar near-clipping i Z-fighting
		if (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL)
		{
			if (camera == CAM_NAVEGA)
			{
				p_near = 0.01;
				p_far = 100000.0;
			}
			else
			{
				p_near = glm::clamp((float)OPV.R * 0.01f, 0.0001f, 10.0f);
				p_far = glm::max(100000.0f, (float)OPV.R * 4.0f);
			}
		}
		else
		{
			p_near = 0.01;
			p_far = 50000.0;
		}

		// Entorn VGI: Definició de Viewport, Projecció i Càmera
		ProjectionMatrix = Projeccio_Perspectiva(shader_programID, 0, 0, w, h, OPV.R);

		// Entorn VGI: Definició de la càmera.
		if (camera == CAM_ESFERICA) {
			n[0] = 0;		n[1] = 0;		n[2] = 0;
			ViewMatrix = Vista_Esferica(shader_programID, OPV, Vis_Polar, c_fons,
				oculta, test_vis, ilumina, llum_ambient, llumGL, ifixe);

			// Quan hi ha un astre enfocat (cosEnfocatId > 0), centrar la camera sobre la posicio 3D de l'astre
			if (objecte == SISTEMA_SOLAR && escenariActual != nullptr &&
				g_EstatSimulador.cosEnfocatId > 0 && g_EstatSimulador.cosEnfocatId < escenariActual->getNumCossos())
			{
				glm::vec3 pos = escenariActual->getPosicioCos(g_EstatSimulador.cosEnfocatId);
				if (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL)
				{
					// Origen relatiu al cos enfocat (evita jitter en float32 a 30.000-40.000 u):
					// la camera orbita (0,0,0) i el Sol (Llum #0) es situa a -pos
					glUseProgram(shader_programID);
					glUniform4f(glGetUniformLocation(shader_programID, "LightSource[0].position"), -pos.x, -pos.y, -pos.z, 1.0f);
				}
				else
				{
					ViewMatrix = glm::translate(ViewMatrix, -pos);
					glUseProgram(shader_programID);
					glUniformMatrix4fv(glGetUniformLocation(shader_programID, "viewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
				}
			}
		}
		else if (camera == CAM_NAVEGA) {
			if (Vis_Polar == POLARZ) {
				vpv[0] = 0.0;	vpv[1] = 0.0;	vpv[2] = 1.0;
			}
			else if (Vis_Polar == POLARY) {
				vpv[0] = 0.0;	vpv[1] = 1.0;	vpv[2] = 0.0;
			}
			else if (Vis_Polar == POLARX) {
				vpv[0] = 1.0;	vpv[1] = 0.0;	vpv[2] = 0.0;
			}
			ViewMatrix = Vista_Navega(shader_programID, opvN,
				n, vpv, c_fons, oculta, test_vis,
				ilumina, llum_ambient, llumGL, ifixe);
		}

		// Entorn VGI: Dibuix de l'Objecte o l'Escena
		dibuixa_Escena();		// Dibuix geometria de l'escena amb comandes GL.
		break;

	default:
		// Entorn VGI: Càrrega SHADERS
		// Entorn VGI: Càrrega Shader Eixos
		if (!eixos_programID) eixos_programID = shaderEixos.loadFileShaders(".\\shaders\\eixos.VERT", ".\\shaders\\eixos.FRAG");

		// Entorn VGI: Càrrega Shader de Phong
		if (!shader_programID) shader_programID = shaderLighting.loadFileShaders(".\\shaders\\phong_shdrML.vert", ".\\shaders\\phong_shdrML.frag");

		// Entorn VGI: Creació de la llista que dibuixarà els eixos Coordenades Món. Funció on està codi per dibuixar eixos	
		if (!eixos_Id) eixos_Id = deixos();						// Funció que defineix els Eixos Coordenades Món com un VAO.

		Fons(c_fons);
		break;
	}
}

// dibuixa_Escena: Funcio que crida al dibuix dels diferents elements de l'escana
void dibuixa_Escena() {

	//	Dibuix SkyBox Cúbic.
	if (SkyBoxCube) dibuixa_Skybox(skC_programID, cubemapTexture, Vis_Polar, ProjectionMatrix, ViewMatrix);

	//	Dibuix Coordenades Món i Reixes.
	dibuixa_Eixos(eixos_programID, eixos, eixos_Id, grid, hgrid, ProjectionMatrix, ViewMatrix);

	//	Dibuix geometria de l'escena amb comandes GL.
	dibuixa_EscenaGL(shader_programID, ViewMatrix, glm::mat4(1.0f), sw_material,
		escenariActual,
		g_EstatSimulador);
}


/* ------------------------------------------------------------------------- */
/*                            MENUS ImGui                                    */
/* ------------------------------------------------------------------------- */

// Entorn VGI: Dibuixa el menú principal ImGui del Simulador del Sistema Solar
void draw_Menu_ImGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Renderitzar la interficie principal del Simulador del Sistema Solar (Selector inicial, barra superior, panell de temps, HUD)
	InterficieUI::Renderitzar(g_EstatSimulador, escenariActual, escenaris, shader_programID);

	// Projeccio de les etiquetes i marcadors 2D dels astres sobre la pantalla quan la simulacio esta activa
	if (!g_EstatSimulador.mostrarSelectorModes &&
		objecte == SISTEMA_SOLAR &&
		escenariActual != nullptr)
	{
		const int numCossos = escenariActual->getNumCossos();
		const bool esEscalaReal = (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL);
		const bool mostrarMarcadors = esEscalaReal && g_EstatSimulador.opcionsVisualitzacio.mostrarMarcadorsPetits;
		const bool mostrarEtiquetes = g_EstatSimulador.opcionsVisualitzacio.mostrarEtiquetes;

		if (mostrarMarcadors || mostrarEtiquetes)
		{
			const glm::vec4 viewport(0.0f, 0.0f, (float)w, (float)h);
			const bool origenRelatiu = (esEscalaReal && camera == CAM_ESFERICA &&
				g_EstatSimulador.cosEnfocatId > 0 && g_EstatSimulador.cosEnfocatId < numCossos);

			for (int i = 0; i < numCossos; i++)
			{
				glm::vec3 pos = escenariActual->getPosicioVista(i, g_EstatSimulador.cosEnfocatId, origenRelatiu);
				float rCos = escenariActual->getRadiCos(i);

				if (mostrarMarcadors)
				{
					InterficieUI::DibuixarMarcadorPlaneta(
						escenariActual->getNomCos(i),
						pos,
						rCos,
						escenariActual->getColorCos(i),
						i == g_EstatSimulador.cosEnfocatId,
						mostrarEtiquetes,
						ViewMatrix,
						ProjectionMatrix,
						viewport
					);
				}
				else
				{
					glm::vec3 posMon(pos.x, pos.y, pos.z + rCos * 1.15f);
					InterficieUI::DibuixarEtiquetaPlaneta(escenariActual->getNomCos(i), posMon, ViewMatrix, ProjectionMatrix, viewport);
				}
			}
		}
	}

	// Rendering ImGui Menus
	ImGui::Render();
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// CÀMERA--> NAVEGA:  Mode de navegació sobre un pla amb botons de teclat o de mouse (nav) (opció booleana)
void OnCameraNavega()
{
	if (projeccio != CAP)
	{
		camera = CAM_NAVEGA;
		// Desactivació de zoom i mòbil si navega activat
		mobil = false;	zzoom = false;

		// Incialitzar variables Navega segons configuració eixos en Polars
		if (Vis_Polar == POLARZ) {
			opvN.x = 10.0;	opvN.y = 0.0;	opvN.z = 0.0; // opvN = (10,0,0)
			n[0] = 0.0;		n[1] = 0.0;		n[2] = 0.0;
			angleZ = 0.0;
		}
		else if (Vis_Polar == POLARY) {
			opvN.x = 0.0;	opvN.y = 0.0;	opvN.z = 10.0; // opvN = (10,0,0)
			n[0] = 0.0;		n[1] = 0.0;		n[2] = 0.0;
			angleZ = 0.0;
		}
		else if (Vis_Polar == POLARX) {
			opvN.x = 0.0;	opvN.y = 10.0;	opvN.z = 0.0;	 // opvN = (0,10,0)
			n[0] = 0.0;		n[1] = 0.0;		n[2] = 0.0;
			angleZ = 0.0;
		}
	}
}

// SKYBOX: Visualitzar Skybox en l'escena (opció booleana)
void OnVistaSkyBox()
{
	// Càrrega Shader Skybox
	if (!skC_programID) skC_programID = shader_SkyBoxC.loadFileShaders(".\\shaders\\skybox.VERT", ".\\shaders\\skybox.FRAG");

	// Càrrega VAO Skybox Cube (verificant si el VAO es valid en OpenGL)
	if (skC_VAOID.vaoId == 0 || !glIsVertexArray(skC_VAOID.vaoId)) skC_VAOID = loadCubeSkybox_VAO();
	Set_VAOList(CUBE_SKYBOX, skC_VAOID);

	if (!cubemapTexture)
	{	// load Skybox textures
		// -------------
		std::vector<std::string> faces =
		{ ".\\textures\\skybox\\right.png",
			".\\textures\\skybox\\left.png",
			".\\textures\\skybox\\top.png",
			".\\textures\\skybox\\bottom.png",
			".\\textures\\skybox\\front.png",
			".\\textures\\skybox\\back.png"
		};
		cubemapTexture = loadCubemap(faces);
	}
}

/* ------------------------------------------------------------------------- */
/*					6. PROJECCIÓ                                             */
/* ------------------------------------------------------------------------- */

// PROJECCIÓ: Perspectiva
void OnProjeccioPerspectiva()
{
	projeccio = PERSPECT;
	mobil = true;			zzoom = true;
}

// SHADER PHONG
void OnShaderPhong()
{
	GLuint newShaderID = 0;

	shader = PHONG_SHADER;	ilumina = SUAU;
	test_vis = false;		oculta = true;

	// Càrrega Shader de Phong
	fprintf(stderr, "Phong Shader: \n");

	// Elimina shader anterior
	shaderLighting.DeleteProgram();	shader_programID = 0;
	// Càrrega Phong Shader
	newShaderID = shaderLighting.loadFileShaders(".\\shaders\\phong_shdrML.vert", ".\\shaders\\phong_shdrML.frag");

	// Càrrega shaders dels fitxers
	if (newShaderID) {
		shader_programID = newShaderID;
		// Activa shader.
		glUseProgram(shader_programID); // shaderLighting.use();
	}
	else fprintf(stderr, "%s", "GLSL_Error. Fitxers .vert o .frag amb errors"); // AfxMessageBox(_T("GLSL_Error. Fitxers .vert o .frag amb errors"));
}


/* ------------------------------------------------------------------------- */
/*                            FI MENUS ImGui                                 */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/*                           CONTROL DEL TECLAT                              */
/* ------------------------------------------------------------------------- */

// OnKeyDown: Funció de tractament de teclat (funció que es crida quan es prem una tecla)
//   PARÀMETRES:
//    - key: Codi del caracter seleccionat
//    - scancode: Nombre de vegades que s'ha apretat la tecla (acceleració)
//    - action: Acció de la tecla: GLFW_PRESS (si s'ha apretat), GLFW_REPEAT, si s'ha repetit pressió i GL_RELEASE, si es deixa d'apretar.
//    - mods: Variable que identifica si la tecla s'ha pulsat directa (mods=0), juntament amb la tecla Shift (mods=1) o la tecla Ctrl (mods=2).
static void CalculaLimitsZoomCamera(int cosId, double& outMinR, double& outMaxR, double& outPasZoom)
{
	float radiCos = 35.0f;
	outMinR = 42.0;
	outMaxR = 800.0;
	outPasZoom = 5.0;

	if (escenariActual != nullptr)
	{
		const ConfigCamera& cfgCam = escenariActual->getConfigCamera();
		int numCossos = escenariActual->getNumCossos();
		if (cosId >= 0 && cosId < numCossos)
		{
			radiCos = escenariActual->getRadiCos(cosId);
		}
		outMinR = static_cast<double>(cfgCam.getMinR(cosId, radiCos));
		outMaxR = static_cast<double>(cfgCam.maxR);
		outPasZoom = static_cast<double>(cfgCam.getPasZoom(cosId, radiCos, static_cast<float>(OPV.R)));
	}
}

void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// EntornVGI.ImGui: Test si events de teclat han sigut filtrats per ImGui (io.WantCaptureKeyboard)
	ImGuiIO& io = ImGui::GetIO();

	if (!io.WantCaptureKeyboard) {
		// EntornVGI: Si tecla pulsada és ESCAPE, tancar finestres i aplicació.
		if (mods == 0 && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
			return;
		}
		// Bloquejar teclat si el selector inicial de modes està obert
		if (g_EstatSimulador.mostrarSelectorModes) return;

		// Dreceres de control del simulador (Pausa, Invertir temps, Reenquadrar vista, Vista Global)
		if (mods == 0 && action == GLFW_PRESS) {
			if (key == GLFW_KEY_SPACE) {
				g_EstatSimulador.tempsSimulacio.enPausa = !g_EstatSimulador.tempsSimulacio.enPausa;
				return;
			}
			else if (key == GLFW_KEY_BACKSPACE) {
				g_EstatSimulador.tempsSimulacio.direccioTemps = -g_EstatSimulador.tempsSimulacio.direccioTemps;
				return;
			}
			else if (key == GLFW_KEY_R) {
				g_EstatSimulador.canviCameraPendent = true;
				return;
			}
			else if (key == GLFW_KEY_HOME) {
				g_EstatSimulador.cameraConfig.mode = ModeCamera::GLOBAL_SISTEMA;
				g_EstatSimulador.cosEnfocatId = 0;
				InterficieUI::ActualitzarDadesCosActiu(g_EstatSimulador);
				g_EstatSimulador.canviCameraPendent = true;
				return;
			}
		}

		//(NOA)
		if (camera == CAM_ESFERICA &&
			(action == GLFW_PRESS || action == GLFW_REPEAT) &&
			(key == GLFW_KEY_LEFT ||
				key == GLFW_KEY_RIGHT ||
				key == GLFW_KEY_UP ||
				key == GLFW_KEY_DOWN))
		{
			const double pasAngle = 5.0;
			int cosId = g_EstatSimulador.cosEnfocatId;
			double minR = 42.0, maxR = 800.0, pasDistancia = 5.0;
			CalculaLimitsZoomCamera(cosId, minR, maxR, pasDistancia);

			// Gir horitzontal al voltant del cos enfocat
			if (key == GLFW_KEY_LEFT)
				OPV.beta -= pasAngle;

			else if (key == GLFW_KEY_RIGHT)
				OPV.beta += pasAngle;

			// Zoom: apropar-se o allunyar-se
			else if (key == GLFW_KEY_UP)
				OPV.R -= pasDistancia;

			else if (key == GLFW_KEY_DOWN)
				OPV.R += pasDistancia;

			// Evitem que la càmera travessi el cos
			if (OPV.R < minR)
				OPV.R = minR;
			if (OPV.R > maxR)
				OPV.R = maxR;

			// Mantenim l'angle horitzontal entre 0 i 360 graus
			while (OPV.beta >= 360.0)
				OPV.beta -= 360.0;

			while (OPV.beta < 0.0)
				OPV.beta += 360.0;

			cam_Esferica[0] = (float)OPV.R;
			cam_Esferica[1] = (float)OPV.alfa;
			cam_Esferica[2] = (float)OPV.beta;
		}
		else if (camera == CAM_NAVEGA)
			Teclat_Navega(key, action);
	}
}

// Teclat_Navega: Teclat pels moviments de navegació.
void Teclat_Navega(int key, int action)
{
	GLdouble vdir[3] = { 0, 0, 0 };
	double modul = 0;

	// Entorn VGI: Controls de moviment de navegació
	vdir[0] = n[0] - opvN.x;
	vdir[1] = n[1] - opvN.y;
	vdir[2] = n[2] - opvN.z;
	modul = sqrt(vdir[0] * vdir[0] + vdir[1] * vdir[1] + vdir[2] * vdir[2]);
	vdir[0] = vdir[0] / modul;
	vdir[1] = vdir[1] / modul;
	vdir[2] = vdir[2] / modul;

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		double nx = 0.0, ny = 0.0, nz = 0.0;
		switch (key)
		{	// Tecla cursor amunt o W
		case GLFW_KEY_UP:
		case GLFW_KEY_W:
			if (Vis_Polar == POLARZ)
			{  // (X,Y,Z)
				opvN.x += fact_pan * vdir[0];
				opvN.y += fact_pan * vdir[1];
				n[0] += fact_pan * vdir[0];
				n[1] += fact_pan * vdir[1];
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				opvN.x += fact_pan * vdir[0];
				opvN.z += fact_pan * vdir[2];
				n[0] += fact_pan * vdir[0];
				n[2] += fact_pan * vdir[2];
			}
			else if (Vis_Polar == POLARX)
			{	//(X,Y,Z) --> (Y,Z,X)
				opvN.y += fact_pan * vdir[1];
				opvN.z += fact_pan * vdir[2];
				n[1] += fact_pan * vdir[1];
				n[2] += fact_pan * vdir[2];
			}
			break;

			// Tecla cursor avall o S
		case GLFW_KEY_DOWN:
		case GLFW_KEY_S:
			if (Vis_Polar == POLARZ)
			{ // (X,Y,Z)
				opvN.x -= fact_pan * vdir[0];
				opvN.y -= fact_pan * vdir[1];
				n[0] -= fact_pan * vdir[0];
				n[1] -= fact_pan * vdir[1];
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				opvN.x -= fact_pan * vdir[0];
				opvN.z -= fact_pan * vdir[2];
				n[0] -= fact_pan * vdir[0];
				n[2] -= fact_pan * vdir[2];
			}
			else if (Vis_Polar == POLARX)
			{ //(X,Y,Z) --> (Y,Z,X)
				opvN.y -= fact_pan * vdir[1];
				opvN.z -= fact_pan * vdir[2];
				n[1] -= fact_pan * vdir[1];
				n[2] -= fact_pan * vdir[2];
			}
			break;

			// Tecla cursor esquerra o A
		case GLFW_KEY_LEFT:
		case GLFW_KEY_A:
			angleZ = fact_pan;
			if (Vis_Polar == POLARZ)
			{ // (X,Y,Z)
				nx = vdir[0] * cos(angleZ * PI / 180.0) - vdir[1] * sin(angleZ * PI / 180.0);
				ny = vdir[0] * sin(angleZ * PI / 180.0) + vdir[1] * cos(angleZ * PI / 180.0);
				n[0] = nx + opvN.x;
				n[1] = ny + opvN.y;
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				nz = vdir[2] * cos(angleZ * PI / 180.0) - vdir[0] * sin(angleZ * PI / 180.0);
				nx = vdir[2] * sin(angleZ * PI / 180.0) + vdir[0] * cos(angleZ * PI / 180.0);
				n[2] = nz + opvN.z;
				n[0] = nx + opvN.x;
			}
			else if (Vis_Polar == POLARX)
			{ //(X,Y,Z) --> (Y,Z,X)
				ny = vdir[1] * cos(angleZ * PI / 180.0) - vdir[2] * sin(angleZ * PI / 180.0);
				nz = vdir[1] * sin(angleZ * PI / 180.0) + vdir[2] * cos(angleZ * PI / 180.0);
				n[1] = ny + opvN.y;
				n[2] = nz + opvN.z;
			}
			break;

			// Tecla cursor dret o D
		case GLFW_KEY_RIGHT:
		case GLFW_KEY_D:
			angleZ = 360.0 - fact_pan;
			if (Vis_Polar == POLARZ)
			{ // (X,Y,Z)
				nx = vdir[0] * cos(angleZ * PI / 180.0) - vdir[1] * sin(angleZ * PI / 180.0);
				ny = vdir[0] * sin(angleZ * PI / 180.0) + vdir[1] * cos(angleZ * PI / 180.0);
				n[0] = nx + opvN.x;
				n[1] = ny + opvN.y;
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				nz = vdir[2] * cos(angleZ * PI / 180.0) - vdir[0] * sin(angleZ * PI / 180.0);
				nx = vdir[2] * sin(angleZ * PI / 180.0) + vdir[0] * cos(angleZ * PI / 180.0);
				n[2] = nz + opvN.z;
				n[0] = nx + opvN.x;
			}
			else if (Vis_Polar == POLARX)
			{ //(X,Y,Z) --> (Y,Z,X)
				ny = vdir[1] * cos(angleZ * PI / 180.0) - vdir[2] * sin(angleZ * PI / 180.0);
				nz = vdir[1] * sin(angleZ * PI / 180.0) + vdir[2] * cos(angleZ * PI / 180.0);
				n[1] = ny + opvN.y;
				n[2] = nz + opvN.z;
			}
			break;

			// Tecla Inicio ('7') o Q (Pujar)
		case GLFW_KEY_KP_7: //GLFW_KEY_HOME:
		case GLFW_KEY_Q:
			if (Vis_Polar == POLARZ) {
				opvN.z += fact_pan;
				n[2] += fact_pan;
			}
			else if (Vis_Polar == POLARY) {
				opvN.y += fact_pan;
				n[1] += fact_pan;
			}
			else if (Vis_Polar == POLARX) {
				opvN.x += fact_pan;
				n[0] += fact_pan;
			}
			break;

			// Tecla Fin o E (Baixar)
		case GLFW_KEY_END:
		case GLFW_KEY_E:
			if (Vis_Polar == POLARZ) {
				opvN.z -= fact_pan;
				n[2] -= fact_pan;
			}
			else if (Vis_Polar == POLARY) {
				opvN.y -= fact_pan;
				n[1] -= fact_pan;
			}
			else if (Vis_Polar == POLARX) {
				opvN.x -= fact_pan;
				n[0] -= fact_pan;
			}
			break;

			// Tecla PgUp ('9)
		case GLFW_KEY_KP_9: //GLFW_KEY_PAGE_UP:
			fact_pan /= 2;
			if (fact_pan < 0.125) fact_pan = 0.125;
			break;

			// Tecla PgDown ('3')
		case GLFW_KEY_KP_3: //GLFW_KEY_PAGE_DOWN:
			fact_pan *= 2;
			if (fact_pan > 2048) fact_pan = 2048;
			break;

		default:
			break;
		}
	}
}


/* ------------------------------------------------------------------------- */
/*                           CONTROL DEL RATOLI                              */
/* ------------------------------------------------------------------------- */

// OnMouseButton: Funció que es crida quan s'apreta algun botó (esquerra o dreta) del mouse.
//      PARAMETRES: - window: Finestra activa
//					- button: Botó seleccionat (GLFW_MOUSE_BUTTON_LEFT o GLFW_MOUSE_BUTTON_RIGHT)
//					- action: Acció de la tecla: GLFW_PRESS (si s'ha apretat), GLFW_REPEAT, si s'ha repetit pressió i GL_RELEASE, si es deixa d'apretar.
void OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	// Get the cursor position when the mouse key has been pressed or released.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// EntornVGI.ImGui: Test si events de mouse han sigut filtrats per ImGui (io.WantCaptureMouse)
	// (1) ALWAYS forward mouse data to ImGui! This is automatic with default backends. With your own backend:
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action);

	// (2) ONLY forward mouse data to your underlying app/game.
	if (!io.WantCaptureMouse && !g_EstatSimulador.mostrarSelectorModes) { //<Tractament mouse de l'aplicació>}
		// OnLButtonDown
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{	// Entorn VGI: Detectem en quina posició s'ha apretat el botó esquerra del
			//				mouse i ho guardem a la variable m_PosEAvall i activem flag m_ButoEAvall
			m_ButoEAvall = true;
			m_PosEAvall.x = xpos;	m_PosEAvall.y = ypos;
		}
		// OnLButtonUp: Funció que es crida quan deixem d'apretar el botó esquerra del mouse.
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{	// Entorn VGI: Desactivem flag m_ButoEAvall quan deixem d'apretar botó esquerra del mouse.
			m_ButoEAvall = false;
		}
		// OnRButtonDown
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{	// Entorn VGI: Detectem en quina posició s'ha apretat el botó dret del
			//				mouse i ho guardem a la variable m_PosDAvall i activem flag m_ButoDAvall
			m_ButoDAvall = true;
			m_PosDAvall.x = xpos;	m_PosDAvall.y = ypos;
		}
		// OnRButtonUp: Funció que es crida quan deixem d'apretar el botó dret del mouse.
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		{	// Entorn VGI: Desactivem flag m_ButoDAvall quan deixem d'apretar botó dret del mouse.
			m_ButoDAvall = false;
		}
	}
}

// OnMouseMove: Funció que es crida quan es mou el mouse. La utilitzem per la 
//				  Visualització Interactiva amb les tecles del mouse apretades per 
//				  modificar els paràmetres de P.V. (R,angleh,anglev) segons els 
//				  moviments del mouse.
//      PARAMETRES: - window: Finestra activa
//					- xpos: Posició X del cursor del mouse (coord. pantalla) quan el botó s'ha apretat.
//					- ypos: Posició Y del cursor del mouse(coord.pantalla) quan el botó s'ha apretat.
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	if (g_EstatSimulador.mostrarSelectorModes) return;

	double modul = 0;
	GLdouble vdir[3] = { 0, 0, 0 };
	CSize gir = { 0,0 }, girn = { 0,0 }, zoomincr = { 0,0 };

	if (m_ButoEAvall && mobil && projeccio != CAP)
	{	// Entorn VGI: Determinació dels angles (en graus) segons l'increment
		//				horitzontal i vertical de la posició del mouse.
		gir.cx = m_PosEAvall.x - xpos;		gir.cy = m_PosEAvall.y - ypos;
		m_PosEAvall.x = xpos;				m_PosEAvall.y = ypos;
		if (camera == CAM_ESFERICA)
		{	// Càmera Esfèrica
			OPV.beta = OPV.beta - gir.cx / 2.0;

			// Si estem en mode Càmera Fixa (25º), mantenim alfa fix a 25.0.
			// En la resta de modes de càmera orbital/global, permetem variar l'angle vertical alfa.
			if (g_EstatSimulador.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25) {
				OPV.alfa = 25.0;
			}
			else {
				OPV.alfa = OPV.alfa + gir.cy / 2.0;
				while (OPV.alfa >= 360.0)	OPV.alfa = OPV.alfa - 360.0;
				while (OPV.alfa < 0.0)		OPV.alfa = OPV.alfa + 360.0;
			}

			while (OPV.beta >= 360.0)		OPV.beta = OPV.beta - 360.0;
			while (OPV.beta < 0.0)			OPV.beta = OPV.beta + 360.0;

			cam_Esferica[0] = (float)OPV.R;
			cam_Esferica[1] = (float)OPV.alfa;
			cam_Esferica[2] = (float)OPV.beta;
		}
		// Crida a OnPaint() per redibuixar l'escena
		//OnPaint(window);
	}
	else if (m_ButoEAvall && (camera == CAM_NAVEGA) && (projeccio != CAP)) // Opció Navegació
	{	// Entorn VGI: Canviar orientació en opció de Navegació
		girn.cx = m_PosEAvall.x - xpos;		girn.cy = m_PosEAvall.y - ypos;
		angleZ = girn.cx / 2.0;
		// Entorn VGI: Control per evitar el creixement desmesurat dels angles.
		while (angleZ >= 360.0) angleZ = angleZ - 360.0;
		while (angleZ < 0.0)	angleZ = angleZ + 360.0;

		// Entorn VGI: Segons orientació dels eixos Polars (Vis_Polar)
		if (Vis_Polar == POLARZ)
		{ // (X,Y,Z)
			double dx = n[0] - opvN.x;
			double dy = n[1] - opvN.y;
			n[0] = (dx * cos(angleZ * PI / 180.0) - dy * sin(angleZ * PI / 180.0)) + opvN.x;
			n[1] = (dx * sin(angleZ * PI / 180.0) + dy * cos(angleZ * PI / 180.0)) + opvN.y;
			n[2] = n[2] + girn.cy * 0.01;
		}
		else if (Vis_Polar == POLARY)
		{ //(X,Y,Z) --> (Z,X,Y)
			double dz = n[2] - opvN.z;
			double dx = n[0] - opvN.x;
			n[2] = (dz * cos(angleZ * PI / 180.0) - dx * sin(angleZ * PI / 180.0)) + opvN.z;
			n[0] = (dz * sin(angleZ * PI / 180.0) + dx * cos(angleZ * PI / 180.0)) + opvN.x;
			n[1] = n[1] + girn.cy * 0.01;
		}
		else if (Vis_Polar == POLARX)
		{ //(X,Y,Z) --> (Y,Z,X)
			double dy = n[1] - opvN.y;
			double dz = n[2] - opvN.z;
			n[1] = (dy * cos(angleZ * PI / 180.0) - dz * sin(angleZ * PI / 180.0)) + opvN.y;
			n[2] = (dy * sin(angleZ * PI / 180.0) + dz * cos(angleZ * PI / 180.0)) + opvN.z;
			n[0] = n[0] + girn.cy * 0.01;
		}

		m_PosEAvall.x = xpos;		m_PosEAvall.y = ypos;
	}

	// Determinació del paràmetre R segons l'increment
	//   vertical de la posició del mouse (tecla dreta apretada)
	if (m_ButoDAvall && zzoom && (projeccio != CAP))
	{	//CSize zoomincr = m_PosDAvall - point;
		zoomincr.cx = m_PosDAvall.x - xpos;		zoomincr.cy = m_PosDAvall.y - ypos;
		long int incr = zoomincr.cy / 1.0;

		if (camera == CAM_ESFERICA) {	// Càmera Esfèrica
			if (escenariActual != nullptr && escenariActual->getConfigCamera().zoomLogaritmic) {
				double minR = 0.0005, maxR = 80000.0, pasZoom = 0.0;
				CalculaLimitsZoomCamera(g_EstatSimulador.cosEnfocatId, minR, maxR, pasZoom);
				OPV.R = OPV.R * exp(static_cast<double>(zoomincr.cy) * 0.01);
				if (OPV.R < minR) OPV.R = minR;
				if (OPV.R > maxR) OPV.R = maxR;
			}
			else {
				OPV.R = OPV.R + incr;
				//if (OPV.R < 0.25) OPV.R = 0.25;
				if (OPV.R < p_near) OPV.R = p_near;
				if (OPV.R > p_far) OPV.R = p_far;
			}
		}

		//m_PosDAvall = point;
		m_PosDAvall.x = xpos;				m_PosDAvall.y = ypos;
		// Crida a OnPaint() per redibuixar l'escena
		//OnPaint(window);
	}
	else if (m_ButoDAvall && (camera == CAM_NAVEGA) && (projeccio != CAP))
	{	// Avançar en opció de Navegació
		if ((m_PosDAvall.x != xpos) && (m_PosDAvall.y != ypos))
		{
			//CSize zoomincr = m_PosDAvall - point;
			zoomincr.cx = m_PosDAvall.x - xpos;		zoomincr.cy = m_PosDAvall.y - ypos;
			double incr = zoomincr.cy / 2.0;
			//	long int incr=zoomincr.cy/2.0;  // Causa assertion en "afx.inl" lin 169
			vdir[0] = n[0] - opvN.x;
			vdir[1] = n[1] - opvN.y;
			vdir[2] = n[2] - opvN.z;
			modul = sqrt(vdir[0] * vdir[0] + vdir[1] * vdir[1] + vdir[2] * vdir[2]);
			vdir[0] = vdir[0] / modul;
			vdir[1] = vdir[1] / modul;
			vdir[2] = vdir[2] / modul;

			// Entorn VGI: Segons orientació dels eixos Polars (Vis_Polar)
			if (Vis_Polar == POLARZ)
			{ // (X,Y,Z)
				opvN.x += incr * vdir[0];
				opvN.y += incr * vdir[1];
				n[0] += incr * vdir[0];
				n[1] += incr * vdir[1];
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				opvN.z += incr * vdir[2];
				opvN.x += incr * vdir[0];
				n[2] += incr * vdir[2];
				n[0] += incr * vdir[0];
			}
			else if (Vis_Polar == POLARX)
			{ //(X,Y,Z) --> (Y,Z,X)
				opvN.y += incr * vdir[1];
				opvN.z += incr * vdir[2];
				n[1] += incr * vdir[1];
				n[2] += incr * vdir[2];
			}

			//m_PosDAvall = point;
			m_PosDAvall.x = xpos;				m_PosDAvall.y = ypos;
		}
	}
}

// OnMouseWheel: Funció que es crida quan es mou el rodet del mouse. La utilitzem per la 
//				  Visualització Interactiva per modificar el paràmetre R de P.V. (R,angleh,anglev) 
//				  segons el moviment del rodet del mouse.
//      PARAMETRES: -  (xoffset,yoffset): Estructura (x,y) que dóna la posició del mouse 
//							 (coord. pantalla) quan el botó s'ha apretat.
void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
	double modul = 0;
	GLdouble vdir[3] = { 0, 0, 0 };

	// EntornVGI.ImGui: Test si events de mouse han sigut filtrats per ImGui (io.WantCaptureMouse)
	// (1) ALWAYS forward mouse data to ImGui! This is automatic with default backends. With your own backend:
	ImGuiIO& io = ImGui::GetIO();
	//io.AddMouseButtonEvent(button, true);

// (2) ONLY forward mouse data to your underlying app/game.
	if (!io.WantCaptureMouse && !g_EstatSimulador.mostrarSelectorModes) { // <Tractament mouse de l'aplicació>}
		// Funció de zoom en Càmera Esfèrica
		if (zzoom)
		{
			if (camera == CAM_ESFERICA)
			{	// Càmera Esfèrica
				int cosId = g_EstatSimulador.cosEnfocatId;
				double minR = 42.0, maxR = 800.0, pasZoom = 5.0;
				CalculaLimitsZoomCamera(cosId, minR, maxR, pasZoom);

				if (escenariActual != nullptr && escenariActual->getConfigCamera().zoomLogaritmic)
				{
					double factor = static_cast<double>(escenariActual->getConfigCamera().factorZoomLogaritmic);
					OPV.R = OPV.R * pow(1.0 - factor, yoffset);
				}
				else
				{
					OPV.R = OPV.R - yoffset * pasZoom;
				}
				if (OPV.R < minR)
					OPV.R = minR;
				if (OPV.R > maxR)
					OPV.R = maxR;

				cam_Esferica[0] = (float)OPV.R;
				cam_Esferica[1] = (float)OPV.alfa;
				cam_Esferica[2] = (float)OPV.beta;
				//InvalidateRect(NULL, false);
			}
		}
		else if (camera == CAM_NAVEGA && !io.WantCaptureMouse)
		{
			vdir[0] = n[0] - opvN.x;
			vdir[1] = n[1] - opvN.y;
			vdir[2] = n[2] - opvN.z;
			modul = sqrt(vdir[0] * vdir[0] + vdir[1] * vdir[1] + vdir[2] * vdir[2]);
			vdir[0] = vdir[0] / modul;
			vdir[1] = vdir[1] / modul;
			vdir[2] = vdir[2] / modul;

			// Entorn VGI: Segons orientació dels eixos Polars (Vis_Polar)
			double pasNav = yoffset * fact_pan;
			if (Vis_Polar == POLARZ)
			{ // (X,Y,Z)
				opvN.x += pasNav * vdir[0];
				opvN.y += pasNav * vdir[1];
				n[0] += pasNav * vdir[0];
				n[1] += pasNav * vdir[1];
			}
			else if (Vis_Polar == POLARY)
			{ //(X,Y,Z) --> (Z,X,Y)
				opvN.z += pasNav * vdir[2];
				opvN.x += pasNav * vdir[0];
				n[2] += pasNav * vdir[2];
				n[0] += pasNav * vdir[0];
			}
			else if (Vis_Polar == POLARX)
			{ //(X,Y,Z) --> (Y,Z,X)
				opvN.y += pasNav * vdir[1];
				opvN.z += pasNav * vdir[2];
				n[1] += pasNav * vdir[1];
				n[2] += pasNav * vdir[2];
			}
		}
	}
}



// Entorn VGI. OnFull_Screen: Funció per a pantalla completa
void OnFull_Screen(GLFWmonitor* monitor, GLFWwindow* window)
{
	//int winPosX, winPosY;
	//winPosX = 0;	winPosY = 0;

	fullscreen = !fullscreen;

	if (fullscreen)
	{	// backup window position and window size
		//glfwGetWindowPos(window, &winPosX, &winPosY);
		//glfwGetWindowSize(window, &width_old, &height_old);

		// Get resolution of monitor
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		w = mode->width;	h = mode->height;
		// Switch to full screen
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {	// Restore last window size and position
		glfwSetWindowMonitor(window, nullptr, 216, 239, 640, 480, mode->refreshRate);
	}
}

// -------------------- TRACTAMENT ERRORS
// error_callback: Displaia error que es pugui produir
void error_callback(int code, const char* description)
{
	//const char* descripcio;
	//int codi = glfwGetError(&descripcio);

 //   display_error_message(code, description);
	fprintf(stderr, "Codi Error: %i", code);
	fprintf(stderr, "Descripcio: %s\n", description);
}


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		fprintf(stderr, "ERROR %s | File: %s | Line ( %3i ) \n", error.c_str(), file, line);
		//fprintf(stderr, "ERROR %s | ", error.c_str());
		//fprintf(stderr, "File: %s | ", file);
		//fprintf(stderr, "Line ( %3i ) \n", line);
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

	fprintf(stderr, "---------------\n");
	fprintf(stderr, "Debug message ( %3i %s \n", id, message);

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             fprintf(stderr, "Source: API \n"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   fprintf(stderr, "Source: Window System \n"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: fprintf(stderr, "Source: Shader Compiler \n"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     fprintf(stderr, "Source: Third Party \n"); break;
	case GL_DEBUG_SOURCE_APPLICATION:     fprintf(stderr, "Source: Application \n"); break;
	case GL_DEBUG_SOURCE_OTHER:           fprintf(stderr, "Source: Other \n"); break;
	} //std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               fprintf(stderr, "Type: Error\n"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: fprintf(stderr, "Type: Deprecated Behaviour\n"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  fprintf(stderr, "Type: Undefined Behaviour\n"); break;
	case GL_DEBUG_TYPE_PORTABILITY:         fprintf(stderr, "Type: Portability\n"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:         fprintf(stderr, "Type: Performance\n"); break;
	case GL_DEBUG_TYPE_MARKER:              fprintf(stderr, "Type: Marker\n"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          fprintf(stderr, "Type: Push Group\n"); break;
	case GL_DEBUG_TYPE_POP_GROUP:           fprintf(stderr, "Type: Pop Group\n"); break;
	case GL_DEBUG_TYPE_OTHER:               fprintf(stderr, "Type: Other\n"); break;
	} //std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         fprintf(stderr, "Severity: high\n"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       fprintf(stderr, "Severity: medium\n"); break;
	case GL_DEBUG_SEVERITY_LOW:          fprintf(stderr, "Severity: low\n"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: fprintf(stderr, "Severity: notification\n"); break;
	} //std::cout << std::endl;
	//std::cout << std::endl;
	fprintf(stderr, "\n");
}

//COMMMMMMMMM static Escenari* escenariActual = new Escenari();
// main(): Funció principal del programa. Inicialitza la finestra, els callbacks i el bucle principal d'operació
int main()
{
	//	std::cout << "Hello World!\n";

	//    GLFWwindow* window;
	// Entorn VGI. Timer: Variables
	float now;
	float delta;

	// glfw: initialize and configure
	// ------------------------------
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	// Retrieving main monitor
	primary = glfwGetPrimaryMonitor();

	// To get current video mode of a monitor
	mode = glfwGetVideoMode(primary);

	// Retrieving monitors
	//    int countM;
	//   GLFWmonitor** monitors = glfwGetMonitors(&countM);

	// Retrieving video modes of the monitor
	int countVM;
	const GLFWvidmode* modes = glfwGetVideoModes(primary, &countVM);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1280, 720, "Entorn Gràfic VS2022 amb GLFW i OpenGL 4.6 (Visualització Gràfica Interactiva - Grau en Enginyeria Informàtica - Escola Enginyeria - UAB)", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 4.6 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Llegir resolució actual de pantalla
	glfwGetWindowSize(window, &width_old, &height_old);

	// Initialize GLEW
	if (GLEW_VERSION_3_3) glewExperimental = GL_TRUE; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		glGetError();	// Esborrar codi error generat per bug a llibreria GLEW
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	const unsigned char* version = (unsigned char*)glGetString(GL_VERSION);

	int major, minor;
	GetGLVersion(&major, &minor);

	// ------------- Entorn VGI: Configure OpenGL context
	//	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor); // GL 4.6

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Si funcions deprecades són eliminades (no ARB_COMPATIBILITY)
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);  // Si funcions deprecades NO són eliminades (Si ARB_COMPATIBILITY)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// Creació contexte CORE
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);	// Creació contexte ARB_COMPATIBILITY
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // comment this line in a release build! 


	// ------------ - Entorn VGI : Enable OpenGL debug context if context allows for DEBUG CONTEXT (GL4.6)
	if (GLEW_VERSION_4_6)
	{
		GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


	// Initialize Application control varibles
	InitGL();

	// ------------- Entorn VGI: Callbacks
	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetWindowSizeCallback(window, OnSize);										// - Windows Size in screen Coordinates
	glfwSetFramebufferSizeCallback(window, OnSize);									// - Windows Size in Pixel Coordinates
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)OnMouseButton);			// - Directly redirect GLFW mouse button events
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)OnMouseMove);				// - Directly redirect GLFW mouse position events
	glfwSetScrollCallback(window, (GLFWscrollfun)OnMouseWheel);						// - Directly redirect GLFW mouse wheel events
	glfwSetKeyCallback(window, (GLFWkeyfun)OnKeyDown);								// - Directly redirect GLFW key events
	glfwSetErrorCallback(error_callback);											// Error callback
	glfwSetWindowRefreshCallback(window, (GLFWwindowrefreshfun)OnPaint);			// - Callback to refresh the screen

	// Entorn VGI; Timer: Lectura temps
	float previous = glfwGetTime();

	// Entorn VGI.ImGui: Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

// Entorn VGI.ImGui: Setup Dear ImGui style
	InterficieUI::InicialitzarEstil();

	// Entorn VGI.ImGui: Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	// Entorn VGI.ImGui: End Setup Dear ImGui context

	// Reserva dels 2 escenaris polimorfics disponibles (0: Intuitiu, 1: Escala Real).
	// L'escena del sistema solar NO es carrega ni es dibuixa mentre el selector inicial de modes estigui obert.
	escenaris[0] = new EscenariIntuitiu();
	escenaris[1] = new EscenariRealista();
	escenariActual = escenaris[0];

	objecte = SISTEMA_SOLAR;
	camera = CAM_ESFERICA;		Vis_Polar = POLARZ;
	tFlag_invert_Y = true;
	sw_material[0] = false;		SkyBoxCube = false;

	// Càmera allunyada i enlairada per veure les òrbites des de dalt
	OPV.R = 550.0;	OPV.alfa = 65.0;	OPV.beta = 0.0;
	cam_Esferica[0] = (float)OPV.R;	cam_Esferica[1] = (float)OPV.alfa;	cam_Esferica[2] = (float)OPV.beta;


// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{	// Poll for and process events */
		//        glfwPollEvents();

		// Entorn VGI. Timer: common part, do this only once
		now = glfwGetTime();
		delta = now - previous;
		previous = now;


		// Actualització del temps de simulació i moviment orbital dels planetes
		// (només quan s'ha tancat el selector inicial d'escenaris)
		if (!g_EstatSimulador.mostrarSelectorModes) {
			bool escalaReal = (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL);
			if (!g_EstatSimulador.tempsSimulacio.enPausa) {
				// En mode didàctic el rellotge avança a ritme moderat; en escala real segons el timelapse
				double deltaRellotge = escalaReal ? (double)delta : ((double)delta * 0.25);
				g_EstatSimulador.tempsSimulacio.AvancarSegons(deltaRellotge);
			}

			if (escenariActual != nullptr && objecte == SISTEMA_SOLAR) {
				escenariActual->updateVariables(
					(double)delta,
					g_EstatSimulador
				);
			}
		}

		// Poll for and process events
		glfwPollEvents();

		// Entorn VGI.ImGui: Dibuixa menú ImGui
		draw_Menu_ImGui();

		// Crida a OnPaint() per redibuixar l'escena
		OnPaint(window);

		// Entorn VGI.ImGui: Capta dades del menú InGui
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Entorn VGI: Activa la finestra actual
		glfwMakeContextCurrent(window);

		// Entorn VGI: Transferència del buffer OpenGL a buffer de pantalla
		glfwSwapBuffers(window);
	}

	// Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Entorn VGI.ImGui: Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	for (int i = 0; i < 2; i++)
	{
		delete escenaris[i];
		escenaris[i] = nullptr;
	}
	escenariActual = nullptr;

	glfwDestroyWindow(window);

	// Terminating GLFW: Destroy the windows, monitors and cursor objects
	glfwTerminate();

	if (shaderLighting.getProgramID() != -1) shaderLighting.DeleteProgram();
	return 0;

}

// Ejecutar programa: Ctrl + F5 o menú Depurar > Iniciar sin depurar
// Depurar programa: F5 o menú Depurar > Iniciar depuración

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de código fuente
//   3. Use la ventana de salida para ver la salida de compilación y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de código, o a Proyecto > Agregar elemento existente para agregar archivos de código existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
