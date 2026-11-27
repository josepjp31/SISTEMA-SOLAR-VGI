
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
#include "InterficieUI.h"

// Escenari actiu i llista d'escenaris reservats a main():
// - escenaris[0]: Mode Didactic / Intuitiu
// - escenaris[1]: Mode a Escala Real
Escenari* escenariActual = nullptr;
Escenari* escenaris = nullptr;

// Estat global de la interficie d'usuari (InterficieUI.h): controla el mode actiu,
// el selector inicial, el panell de temps, les opcions visuals i la camera seleccionada.
EstatSimulacio g_EstatSimulador;
//VARIABLE GLOBALS COET
// Posició i orientació de la Milano
glm::vec3 posMilano(120.0f, 0.0f, 0.0f);
float angleMilano = 0.0f;
float velocitatMilano = 0.5f;
float inclinacioMilano = 0.0f;

void InitGL()
{
	//------ Entorn VGI: Inicialització de les variables globals de CEntornVGIView
	int i;

	// Entorn VGI: Variables de control per Menú CÀMERA: Esfèrica, Navega, Mòbil, Zoom, Satelit, Polars... 
	camera = CAM_ESFERICA;
	mobil = true;			zzoom = true;		zzoomO = false;		satelit = false;

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

	// Entorn VGI: Variables de control per Menú VISTA: Pantalla Completa, Pan, dibuixar eixos i grids 
	fullscreen = false;
	pan = false;
	eixos = true;	eixos_programID = 0;  eixos_Id = 0;
	grid.x = false;	grid.y = false;		grid.z = false;		grid.w = false;
	hgrid.x = 0.0;	hgrid.y = 0.0;		hgrid.z = 0.0;		hgrid.w = 0.0;

	// Entorn VGI: Variables opció VISTA->PAN?
	fact_pan = 1;
	tr_cpv.x = 0;	tr_cpv.y = 0;	tr_cpv.z = 0;		tr_cpvF.x = 0;	tr_cpvF.y = 0;	tr_cpvF.z = 0;

	// Entorn VGI: Variables de control per les opcions de menú Projecció, Objecte
	projeccio = CAP;	// projeccio = PERSPECT;
	ProjectionMatrix = glm::mat4(1.0);	// Inicialitzar matriu a identitat
	objecte = CAP;

	// Entorn VGI: Variables de control Skybox Cube
	SkyBoxCube = false;
	skC_programID = 0;
	skC_VAOID.vaoId = 0;	skC_VAOID.vboId = 0;	skC_VAOID.nVertexs = 0;
	cubemapTexture = 0;

	// Entorn VGI: Variables de control del menú TRANFORMA
	transf = false;		trasl = false;		rota = false;		escal = false;
	fact_Tras = 1;		fact_Rota = 90;
	TG.VTras.x = 0.0;	TG.VTras.y = 0.0;	TG.VTras.z = 0;	TGF.VTras.x = 0.0;	TGF.VTras.y = 0.0;	TGF.VTras.z = 0;
	TG.VRota.x = 0;		TG.VRota.y = 0;		TG.VRota.z = 0;	TGF.VRota.x = 0;	TGF.VRota.y = 0;	TGF.VRota.z = 0;
	TG.VScal.x = 1;		TG.VScal.y = 1;		TG.VScal.z = 1;	TGF.VScal.x = 1;	TGF.VScal.y = 1;	TGF.VScal.z = 1;
	transX = false;		transY = false;		transZ = false;
	GTMatrix = glm::mat4(1.0);		// Inicialitzar matriu a identitat

	// Entorn VGI: Variables de control per les opcions de menú OCULTACIONS
	front_faces = true;	test_vis = false;	oculta = false;		back_line = false;

	// Entorn VGI: Variables de control del menú ILUMINACIÓ
	ifixe = false;					ilum2sides = false;				ilumina = FILFERROS;
	// Reflexions actives: Ambient [1], Difusa [2] i Especular [3]. No actives: Emission [0]. 
	sw_material[0] = false;			sw_material[1] = true;			sw_material[2] = true;			sw_material[3] = true;	sw_material[4] = true;
	sw_material_old[0] = false;		sw_material_old[1] = true;		sw_material_old[2] = true;		sw_material_old[3] = true;	sw_material_old[4] = true;
	textura = false;				t_textura = CAP;
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
	shader_programIDNormalMap = 0;	// Shader de normal mapping dels planetes
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

	// Càrrega Shader de Normal Mapping (planetes amb mapa normal)
	fprintf(stderr, "Normal_mapping: \n");
	if (!shader_programIDNormalMap) shader_programIDNormalMap = shaderLightingNormalMap.loadFileShaders(".\\shaders\\normal_mapping.vert", ".\\shaders\\normal_mapping.frag");
	// Entorn VGI: FI Càrrega SHADERS ---------------------------------------------------

	// Entorn VGI: Variables de control dels botons de mouse
	m_PosEAvall.x = 0;			m_PosEAvall.y = 0;			m_PosDAvall.x = 0;			m_PosDAvall.y = 0;
	m_ButoEAvall = false;		m_ButoDAvall = false;
	m_EsfeEAvall.R = 0.0;		m_EsfeEAvall.alfa = 0.0;	m_EsfeEAvall.beta = 0.0;
	m_EsfeIncEAvall.R = 0.0;	m_EsfeIncEAvall.alfa = 0.0;	m_EsfeIncEAvall.beta = 0.0;

	// Entorn VGI: Variables que controlen paràmetres visualització: Mides finestra Windows i PV (càmera)
	w = 1280;			h = 720;			// Mides de la finestra Windows (w-amplada,h-alçada)
	width_old = 1280;	height_old = 720;	// Mides de la resolució actual de la pantalla (finestra Windows)

	// Entorn VGI: Color de fons i de l'objecte
	clear_colorB = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);	// ImGui: Color de fons
	clear_colorO = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);	// ImGui: Color de l'objecte.
	c_fons.r = clear_colorB.x;		c_fons.g = clear_colorB.y;		c_fons.b = clear_colorB.z;			c_fons.a = clear_colorB.w;
	col_obj.r = clear_colorO.x;	col_obj.g = clear_colorO.y;	col_obj.b = clear_colorO.z;		col_obj.a = clear_colorO.w;

	// Entorn VGI: Variables de visualització i altres
	pas = 64;
	mida = 1.0;
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
	// TODO: Agregue aquí su código de controlador de mensajes

	// A resize event occured; cx and cy are the window's new width and height.
	// Find the OpenGL change size function given in the Lab 1 notes and call it here

	// Entorn VGI: MODIFICACIÓ ->Establim les mides de la finestra actual
	w = width;	h = height;

	// Crida a OnPaint per a redibuixar la pantalla
	//	OnPaint();
}


// SHADER NORMAL MAPPING: (re)carrega el shader de normal mapping dels planetes
void OnShaderNormalMap()
{
	fprintf(stderr, "Normal Mapping Shader:\n");
	shaderLightingNormalMap.DeleteProgram();	shader_programIDNormalMap = 0;
	GLuint newShaderID = shaderLightingNormalMap.loadFileShaders(".\\shaders\\normal_mapping.vert", ".\\shaders\\normal_mapping.frag");
	if (newShaderID) shader_programIDNormalMap = newShaderID;
	else fprintf(stderr, "GLSL_Error. Fitxers normal_mapping amb errors\n");
}

double toJ2000Seconds(int year, int month, int day, int hour = 0, int minute = 0, double second = 0.0);

// Carrega el model 3D de la nau Milano si encara no es a memoria (usat en Pilota coet i quan s'enfoca CB_MILANO)
static void AssegurarCoetCarregat()
{
	if (ObOBJ == NULL)
	{
		ObOBJ = new COBJModel;
		char nomFitxer[] = "textures/coet/milano_gotg.obj";
		int error = ObOBJ->LoadModel(nomFitxer);
		if (error == 0) {
			printf("Milano carregada correctament!\n");
		} else {
			printf("Error carregant la Milano: %d\n", error);
		}
	}
}

// Resol la posicio en coordenades del mon, el radi visual d'enquadrament i el nom
// de qualsevol astre natural (dins escenariActual) o artificial (ex. coet Milano).
static bool ObtenirDadesEnfocament(const Escenari* escenari, CelestialBodyId id,
                                   glm::vec3& outPosicio, double& outRadi, std::string* outNom = nullptr)
{
	if (id == CB_MILANO) {
		outPosicio = posMilano;
		outRadi = 6.0; // Radi aproximat d'enquadrament visual de la nau Milano (escala 0.5)
		if (outNom) *outNom = "Nau Milano (Coet)";
		return true;
	}
	if (escenari != nullptr) {
		const Object* obj = escenari->obtenirObjecte(id);
		if (obj != nullptr && obj->m_enfocable) {
			outPosicio = glm::vec3(obj->m_trMatrix[3]);
			outRadi = obj->m_scale;
			if (outNom) *outNom = obj->m_nom;
			return true;
		}
	}
	return false;
}

// Calcula la distancia per defecte, el zoom minim, el zoom maxim i el pas de zoom
// de forma proporcional al radi de l'objecte enfocat (valid tant per INTUITIU com ESCALA_REAL).
struct LimitsCameraCos {
	double distDefecte;
	double minR;
	double maxR;
	double pasZoom;
};

static LimitsCameraCos CalcularLimitsCameraPerCos(CelestialBodyId id, double radiCos)
{
	// Proteccio davant radis molt petits o nuls (ex. sondes o escala real en UA)
	const double radiSegur = std::max(radiCos, 0.0005);

	LimitsCameraCos lim;
	if (id == CB_SUN) {
		lim.distDefecte = std::max(radiSegur * 8.5, 140.0);
		lim.minR        = std::max(radiSegur * 2.2, 2.0);
		lim.maxR        = std::max(radiSegur * 60.0, 1200.0);
		lim.pasZoom     = std::max(radiSegur * 0.35, 0.5);
	} else {
		lim.distDefecte = radiSegur * 4.5;
		lim.minR        = std::max(radiSegur * 1.8, p_near * 10.0);
		lim.maxR        = std::max(radiSegur * 250.0, 800.0);
		lim.pasZoom     = std::max(radiSegur * 0.35, 0.02);
	}
	return lim;
}

static double LimitarRang(double valor, double minim, double maxim)
{
	return std::min(maxim, std::max(minim, valor));
}

// SincronitzarEstatSimuladorAmbOpenGL: Pont entre la interficie ImGui (g_EstatSimulador)
// i les variables globals d'OpenGL/EntornVGI. S'executa a cada frame abans de renderitzar:
// - Bloc 1 (configuracioInicialPendent): configura projeccio, shaders, llum solar #0, escenari i skybox.
// - Bloc 1b (canviModeDirectePendent): commuta d'escenari en directe conservant vista, angle i zoom relatiu.
// - Bloc 1c (recalculEfemeridesNecessari): recalcula posicions encara que estigui en pausa.
// - Bloc 2 (continu): aplica en temps real les opcions del menu "Visualitzacio".
// - Bloc 3 (canviCameraPendent): canvia entre CAM_ESFERICA (orbital/25°/global) i CAM_NAVEGA (Pilota coet).
void SincronitzarEstatSimuladorAmbOpenGL()
{
	// No inicialitzar ni sincronitzar escena mentre el selector inicial de modes esta obert
	if (g_EstatSimulador.mostrarSelectorModes) return;

	// 1. Configuracio inicial d'escena, il-luminacio i shaders quan s'inicia o es commuta de mode
	if (g_EstatSimulador.configuracioInicialPendent)
	{
		OnProjeccioPerspectiva();

		// Carregar i activar Shader Phong amb il-luminacio Suau si no esta actiu
		if (shader != PHONG_SHADER) OnShaderPhong();
		// Carregar el shader de normal mapping si encara no esta carregat
		if (!shader_programIDNormalMap) OnShaderNormalMap();
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
		for (int k = 0; k < 5; k++) sw_material_old[k] = sw_material[k];

		textura = false;
		t_textura = TEXTURA_FITXERIMA;
		tFlag_invert_Y = true;

		llum_ambient = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;

		llumGL[0].encesa = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;
		llumGL[0].posicio.x = 0.0;		llumGL[0].posicio.y = 0.0;		llumGL[0].posicio.z = 0.0;		llumGL[0].posicio.w = 1.0;
		llumGL[0].difusa.r = 1.0f;		llumGL[0].difusa.g = 1.0f;		llumGL[0].difusa.b = 1.0f;		llumGL[0].difusa.a = 1.0f;
		llumGL[0].especular.r = 1.0f;	llumGL[0].especular.g = 1.0f;	llumGL[0].especular.b = 1.0f;	llumGL[0].especular.a = 1.0f;
		llumGL[0].atenuacio.a = 0.00004; llumGL[0].atenuacio.b = 0.0;	llumGL[0].atenuacio.c = 1.0;
		llumGL[0].restringida = false;
		for (int i = 1; i < NUM_MAX_LLUMS; i++) llumGL[i].encesa = false;

		glUseProgram(shader_programID);
		glUniform1i(glGetUniformLocation(shader_programID, "texture0"), GLint(0));
		glUniform1i(glGetUniformLocation(shader_programID, "flag_invert_y"), tFlag_invert_Y);

		glUseProgram(shader_programIDNormalMap);
		glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "texture0"), 0);
		glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "normalMap"), 1);
		glUniform1i(glGetUniformLocation(shader_programIDNormalMap, "flag_invert_y"), tFlag_invert_Y);
		glUseProgram(shader_programID);

		// Assegurar que l'escenari te els VAOs i textures vinculats als shaders actius
		if (escenariActual != nullptr)
		{
			if (!escenariActual->esInicialitzat())
				escenariActual->creaEscenari(shader_programID, shader_programIDNormalMap);
			g_EstatSimulador.recalculEfemeridesNecessari = true;
		}

		// Assegurar que el Skybox i els Eixos estan carregats
		if (!eixos_Id) eixos_Id = deixos();
		SkyBoxCube = g_EstatSimulador.opcionsVisualitzacio.mostrarSkybox;
		if (SkyBoxCube) OnVistaSkyBox();

		g_EstatSimulador.configuracioInicialPendent = false;
	}

	// 1b. Canvi de mode en directe ("Canviar" o drecera Tab): commuta d'escenari conservant vista i camera
	if (g_EstatSimulador.canviModeDirectePendent)
	{
		// Pas 1: Capturar la vista de l'escenari sortint abans de canviar el punter escenariActual
		const Escenari* escenariAnterior = escenariActual;
		EstatSimulacio::EstatVistaTransicio transicio;
		transicio.valid = true;
		transicio.cosAnterior = g_EstatSimulador.cosEnfocatId;
		transicio.modeCameraAnterior = g_EstatSimulador.cameraConfig.mode;
		transicio.alfa = OPV.alfa;
		transicio.beta = OPV.beta;
		transicio.distAbsoluta = OPV.R;
		transicio.radiSistemaAnterior = (escenariAnterior != nullptr)
			? std::max(escenariAnterior->obtenirRadiReferenciaSistema(), 1e-6)
			: 210.0;

		glm::vec3 posOrigen(0.0f);
		double radiOrigen = 16.0;
		if (!ObtenirDadesEnfocament(escenariAnterior, transicio.cosAnterior, posOrigen, radiOrigen)) {
			radiOrigen = 16.0;
		}
		transicio.radiCosAnterior = radiOrigen;
		transicio.ratioZoomRelatiu = (radiOrigen > 1e-6) ? (OPV.R / radiOrigen) : 4.5;

		// Pas 2: Commutar el punter d'escenari actiu i inicialitzar-lo si es el primer cop
		if (escenaris != nullptr) {
			int idxNou = (g_EstatSimulador.modeActual == ModeSimulacio::INTUITIU) ? 0 : 1;
			escenariActual = &escenaris[idxNou];
			if (!escenariActual->esInicialitzat()) {
				escenariActual->creaEscenari(shader_programID, shader_programIDNormalMap);
			}
		}

		// Forcar actualitzacio immediata a l'instant temporal actual fins i tot si la simulacio es en pausa
		if (escenariActual != nullptr && objecte == SISTEMA_SOLAR) {
			bool escalaReal = (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL);
			double tJ2000 = toJ2000Seconds(
				g_EstatSimulador.tempsSimulacio.dataHora.any,
				g_EstatSimulador.tempsSimulacio.dataHora.mes,
				g_EstatSimulador.tempsSimulacio.dataHora.dia,
				g_EstatSimulador.tempsSimulacio.dataHora.hora,
				g_EstatSimulador.tempsSimulacio.dataHora.minut,
				g_EstatSimulador.tempsSimulacio.dataHora.segon
			);
			escenariActual->updateVariables(
				tJ2000,
				0.0,
				g_EstatSimulador.tempsSimulacio.enPausa,
				g_EstatSimulador.tempsSimulacio.direccioTemps,
				escalaReal,
				g_EstatSimulador.tempsSimulacio.factorVelocitatSegonsPerSegon,
				true // forcarActualitzacio: m_trMatrix sincronitzada abans de posicionar la camera
			);
			g_EstatSimulador.recalculEfemeridesNecessari = false;
		}

		// Pas 3: Restaurar/adaptar la camera i l'objecte enfocat al nou escenari
		double radiSistemaDesti = (escenariActual != nullptr)
			? std::max(escenariActual->obtenirRadiReferenciaSistema(), 1e-6)
			: 210.0;
		double factorSistema = radiSistemaDesti / transicio.radiSistemaAnterior;

		CelestialBodyId cidDesti = transicio.cosAnterior;
		glm::vec3 posDesti(0.0f);
		double radiDesti = 16.0;
		if (!ObtenirDadesEnfocament(escenariActual, cidDesti, posDesti, radiDesti)) {
			// Si no existeix al nou escenari, provar el seu pare o fer fallback a CB_SUN
			CelestialBodyId fallbackId = CB_SUN;
			if (escenariAnterior != nullptr) {
				const Object* objOrig = escenariAnterior->obtenirObjecte(cidDesti);
				if (objOrig != nullptr && objOrig->m_parent != nullptr &&
					ObtenirDadesEnfocament(escenariActual, objOrig->m_parent->m_id, posDesti, radiDesti))
				{
					fallbackId = objOrig->m_parent->m_id;
				}
			}
			cidDesti = fallbackId;
			ObtenirDadesEnfocament(escenariActual, cidDesti, posDesti, radiDesti);
			g_EstatSimulador.cosEnfocatId = cidDesti;
			g_EstatSimulador.origenReferenciaSol = (cidDesti == CB_SUN);
		}
		InterficieUI::ActualitzarDadesCosActiu(g_EstatSimulador, escenariActual);

		if (cidDesti == CB_MILANO) {
			AssegurarCoetCarregat();
		}

		if (transicio.modeCameraAnterior == ModeCamera::ORBITAL_CENTRADOR ||
			transicio.modeCameraAnterior == ModeCamera::FIXA_INCLINADA_25)
		{
			camera = CAM_ESFERICA;
			mobil = true;   zzoom = true;   satelit = false;   pan = false;
			Vis_Polar = POLARZ;
			OPV.alfa = (transicio.modeCameraAnterior == ModeCamera::FIXA_INCLINADA_25) ? 25.0 : transicio.alfa;
			OPV.beta = transicio.beta;

			LimitsCameraCos limDesti = CalcularLimitsCameraPerCos(cidDesti, radiDesti);
			OPV.R = LimitarRang(transicio.ratioZoomRelatiu * radiDesti, limDesti.minR, limDesti.maxR);

			cam_Esferica[0] = (float)OPV.R;
			cam_Esferica[1] = (float)OPV.alfa;
			cam_Esferica[2] = (float)OPV.beta;
		}
		else if (transicio.modeCameraAnterior == ModeCamera::GLOBAL_SISTEMA)
		{
			camera = CAM_ESFERICA;
			mobil = true;   zzoom = true;   satelit = false;   pan = false;
			Vis_Polar = POLARZ;
			OPV.alfa = transicio.alfa;
			OPV.beta = transicio.beta;

			LimitsCameraCos limDesti = CalcularLimitsCameraPerCos(cidDesti, radiDesti);
			OPV.R = LimitarRang(transicio.distAbsoluta * factorSistema, limDesti.minR, limDesti.maxR);

			cam_Esferica[0] = (float)OPV.R;
			cam_Esferica[1] = (float)OPV.alfa;
			cam_Esferica[2] = (float)OPV.beta;
		}
		else if (transicio.modeCameraAnterior == ModeCamera::COET_PRIMERA_PERSONA)
		{
			AssegurarCoetCarregat();
			camera = CAM_NAVEGA;
			mobil = false;  zzoom = false;  satelit = false;
			Vis_Polar = POLARZ;

			posMilano *= (float)factorSistema;
			glm::vec3 posSol(0.0f);
			double radiSol = 16.0;
			if (ObtenirDadesEnfocament(escenariActual, CB_SUN, posSol, radiSol)) {
				glm::vec3 deltaSol = posMilano - posSol;
				float distSol = glm::length(deltaSol);
				float minDistSol = (float)(radiSol * 1.5);
				if (distSol < 1e-4f) {
					posMilano = posSol + glm::vec3(minDistSol, 0.0f, 0.0f);
				} else if (distSol < minDistSol) {
					posMilano = posSol + glm::normalize(deltaSol) * minDistSol;
				}
			}
		}

		g_EstatSimulador.transicioVista = transicio;
		g_EstatSimulador.canviModeDirectePendent = false;
	}

	// 1b. Recalcular immediatament les posicions orbitals si s'ha canviat data/hora o creat l'escenari
	// (funciona tambe amb la simulacio en pausa i abans de construir ViewMatrix)
	if (g_EstatSimulador.recalculEfemeridesNecessari && escenariActual != nullptr && objecte == SISTEMA_SOLAR)
	{
		bool escalaReal = (g_EstatSimulador.modeActual == ModeSimulacio::ESCALA_REAL);
		double tJ2000 = toJ2000Seconds(
			g_EstatSimulador.tempsSimulacio.dataHora.any,
			g_EstatSimulador.tempsSimulacio.dataHora.mes,
			g_EstatSimulador.tempsSimulacio.dataHora.dia,
			g_EstatSimulador.tempsSimulacio.dataHora.hora,
			g_EstatSimulador.tempsSimulacio.dataHora.minut,
			g_EstatSimulador.tempsSimulacio.dataHora.segon
		);
		escenariActual->updateVariables(
			tJ2000,
			0.0,
			g_EstatSimulador.tempsSimulacio.enPausa,
			g_EstatSimulador.tempsSimulacio.direccioTemps,
			escalaReal,
			g_EstatSimulador.tempsSimulacio.factorVelocitatSegonsPerSegon,
			true // forcarActualitzacio: reflecteix el canvi de data encara que estigui en pausa
		);
		g_EstatSimulador.recalculEfemeridesNecessari = false;
	}

	// 2. Sincronitzacio continua a cada frame de les opcions commutables del menu "Visualitzacio"
	if (objecte == SISTEMA_SOLAR)
	{
		SkyBoxCube = g_EstatSimulador.opcionsVisualitzacio.mostrarSkybox;
		if (SkyBoxCube && skC_VAOID.vaoId == 0) OnVistaSkyBox();

		eixos = g_EstatSimulador.opcionsVisualitzacio.mostrarEixos;

		// Llum directa del Sol (emissio del Sol + font punt #0) i mode Vista Clara (ambient)
		sw_material[0] = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;
		llumGL[0].encesa = g_EstatSimulador.opcionsVisualitzacio.iluminacioSolarDirecta;

		llum_ambient = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;
		sw_material[1] = g_EstatSimulador.opcionsVisualitzacio.vistaClaraAmbient;
	}

	// 3. Aplicacio puntual dels canvis de mode de camera o d'astre enfocant demanats des de la UI o dreceres
	if (g_EstatSimulador.canviCameraPendent)
	{
		CelestialBodyId cid = g_EstatSimulador.cosEnfocatId;
		if (cid == CB_NONE) cid = CB_SUN;

		if (cid == CB_MILANO) {
			AssegurarCoetCarregat();
		}

		// Distancia de camera proporcional al radi de l'objecte enfocat (natural o artificial)
		glm::vec3 posObjecte(0.0f);
		double radiObjecte = 16.0;
		if (!ObtenirDadesEnfocament(escenariActual, cid, posObjecte, radiObjecte)) {
			cid = CB_SUN;
			ObtenirDadesEnfocament(escenariActual, cid, posObjecte, radiObjecte);
		}
		LimitsCameraCos lim = CalcularLimitsCameraPerCos(cid, radiObjecte);
		double distPerDefecte = lim.distDefecte;

		if (g_EstatSimulador.cameraConfig.mode == ModeCamera::FIXA_INCLINADA_25)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;	satelit = false;	pan = false;
			Vis_Polar = POLARZ;
			OPV.alfa = 25.0; // Inclinacio vertical blocada a 25° (veure OnMouseMove)
			OPV.R = distPerDefecte;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::ORBITAL_CENTRADOR)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;	satelit = false;	pan = false;
			Vis_Polar = POLARZ;
			if (OPV.alfa == 0.0) OPV.alfa = 25.0;
			OPV.R = distPerDefecte;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::GLOBAL_SISTEMA)
		{
			camera = CAM_ESFERICA;
			mobil = true;	zzoom = true;	satelit = false;	pan = false;
			Vis_Polar = POLARZ;
			g_EstatSimulador.cosEnfocatId = CB_SUN;
			OPV.R = 550.0;
			OPV.alfa = 65.0;
			OPV.beta = 0.0;
		}
		else if (g_EstatSimulador.cameraConfig.mode == ModeCamera::COET_PRIMERA_PERSONA)
		{
			// Mode "Pilota coet": activa CAM_NAVEGA, carrega el model de la Milano i situa la camera
			OnCameraNavega();
			Vis_Polar = POLARZ;
			
			// Situar la camera lliure en una posicio inicial privilegiada mirant cap al centre del Sistema Solar
			/**/
			// Posició de la càmera darrere de la Milano
			opvN.x = 50.0;
			opvN.y = -25.0;
			opvN.z = 15.0;

			// Punt cap on mira: centre de la Milano
			n[0] = 30.0;
			n[1] = 0.0;
			n[2] = 0.0;
			angleZ = 0.0;
			fact_pan = 0.5;
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
	// TODO: Agregue aquí su código de controlador de mensajes
	GLdouble vpv[3] = { 0.0, 0.0, 1.0 };

	// Neteja del framebuffer durant la visualitzacio del Selector Inicial de Modes:
	// evita dibuixar l'escena 3D darrere les dues targetes inicials.
	if (g_EstatSimulador.mostrarSelectorModes)
	{
		glDisable(GL_SCISSOR_TEST);
		glViewport(0, 0, w, h);
		glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	// Sincronitzar les opcions de la interficie ImGui amb l'estat d'OpenGL abans de construir ViewMatrix
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

		// Entorn VGI: Definició de Viewport, Projecció i Càmera
		ProjectionMatrix = Projeccio_Perspectiva(shader_programID, 0, 0, w, h, OPV.R);

		// Entorn VGI: Definició de la càmera.
		if (camera == CAM_ESFERICA) {
			n[0] = 0;		n[1] = 0;		n[2] = 0;
			ViewMatrix = Vista_Esferica(shader_programID, OPV, Vis_Polar, pan, tr_cpv, tr_cpvF, c_fons, col_obj, objecte, mida, pas,
				front_faces, oculta, test_vis, back_line,
				ilumina, llum_ambient, llumGL, ifixe, ilum2sides,
				eixos, grid, hgrid);

			// Quan hi ha un cos enfocat des del menu (natural o artificial), traslladem la vista
			// cap a la seva posicio orbital perque la camera orbiti al seu voltant
			if (objecte == SISTEMA_SOLAR && g_EstatSimulador.cosEnfocatId != CB_NONE)
			{
				glm::vec3 posObjecte(0.0f);
				double radiObjecte = 1.0;
				if (ObtenirDadesEnfocament(escenariActual, g_EstatSimulador.cosEnfocatId, posObjecte, radiObjecte))
				{
					ViewMatrix = glm::translate(ViewMatrix, -posObjecte);
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
				n, vpv, pan, tr_cpv, tr_cpvF, c_fons, col_obj, objecte, true, pas,
				front_faces, oculta, test_vis, back_line,
				ilumina, llum_ambient, llumGL, ifixe, ilum2sides,
				eixos, grid, hgrid);
			// Càmera de seguiment de la Milano (tercera persona)
			float angleRad = glm::radians(angleMilano);
			//COET
			// Direcció cap a la qual apunta la nau
			glm::vec3 direccio(
				cos(angleRad),
				sin(angleRad),
				0.0f
			);

			// Posició de la càmera: darrere i una mica per sobre
			glm::vec3 cameraPos =
				posMilano - direccio * 35.0f + glm::vec3(0.0f, 0.0f, 5.0f);

			// Punt cap on mira la càmera
			glm::vec3 puntMirada = posMilano + direccio * 8.0f + glm::vec3(0.0f, 0.0f, 2.0f);

			// Crear la nova càmera
			ViewMatrix = glm::lookAt(
				cameraPos,
				puntMirada,
				glm::vec3(0.0f, 0.0f, 1.0f)
			);

			// Actualitzar la matriu de càmera al shader
			glUseProgram(shader_programID);
			glUniformMatrix4fv(
				glGetUniformLocation(shader_programID, "viewMatrix"),
				1, GL_FALSE, &ViewMatrix[0][0]
			);
		}

		// Entorn VGI: Dibuix de l'Objecte o l'Escena
		configura_Escena();     // Aplicar Transformacions Geometriques segons persiana Transformacio i configurar objectes.
		dibuixa_Escena();		// Dibuix geometria de l'escena amb comandes GL.
		break;

	default:
		// Entorn VGI: Càrrega SHADERS
		// Entorn VGI: Càrrega Shader Eixos
		if (!eixos_programID) eixos_programID = shaderEixos.loadFileShaders(".\\shaders\\eixos.VERT", ".\\shaders\\eixos.FRAG");

		// Entorn VGI: Càrrega Shader de Phong
		if (!shader_programID) shader_programID = shaderLighting.loadFileShaders(".\\shaders\\phong_shdrML.vert", ".\\shaders\\phong_shdrML.frag");
		if (!shader_programIDNormalMap) shader_programIDNormalMap = shaderLightingNormalMap.loadFileShaders(".\\shaders\\normal_mapping.vert", ".\\shaders\\normal_mapping.frag");

		//#~#~#~#~# Crear un altre shader
		if (!shader_programID) instancing_programID = shaderLighting.loadFileShaders(".\\shaders\\gouraud_shdrML_inst.vert", ".\\shaders\\gouraud_shdrML_inst.frag");

		// Entorn VGI: Creació de la llista que dibuixarà els eixos Coordenades Món. Funció on està codi per dibuixar eixos	
		if (!eixos_Id) eixos_Id = deixos();						// Funció que defineix els Eixos Coordenades Món com un VAO.

		Fons(c_fons);
		break;
	}
}

// configura_Escena: Funcio que configura els parametres de Model i dibuixa les
//                   primitives OpenGL dins classe Model
void configura_Escena() {

	// Aplicar Transformacions Geometriques segons persiana Transformacio i Quaternions
	GTMatrix = instancia(transf, TG, TGF);
}

// dibuixa_Escena: Funcio que crida al dibuix dels diferents elements de l'escana
void dibuixa_Escena() {

	//glUseProgram(shader_programID);

//	Dibuix SkyBox Cúbic.
	if (SkyBoxCube) dibuixa_Skybox(skC_programID, cubemapTexture, Vis_Polar, ProjectionMatrix, ViewMatrix);

	//	Dibuix Coordenades Món i Reixes.
	dibuixa_Eixos(eixos_programID, eixos, eixos_Id, grid, hgrid, ProjectionMatrix, ViewMatrix);

	// Escalat d'objectes, per adequar-los a les vistes ortogràfiques (Pràctica 2)
	//	GTMatrix = glm::scale();

	//	Dibuix geometria de l'escena amb comandes GL.
	bool mostrarMilano = (camera == CAM_NAVEGA) || (g_EstatSimulador.cosEnfocatId == CB_MILANO);
	dibuixa_EscenaGL(shader_programID, shader_programIDNormalMap, ViewMatrix, GTMatrix, sw_material,
		escenariActual,
		ObOBJ,				// COET: model OBJ de la nau Milano
		g_EstatSimulador.opcionsVisualitzacio.mostrarOrbites,
		mostrarMilano);
}


/* ------------------------------------------------------------------------- */
/*                            MENUS ImGui                                    */
/* ------------------------------------------------------------------------- */

// Entorn VGI: Construeix tot el frame de Dear ImGui (selector, menus, panells i etiquetes 2D)
void draw_Menu_ImGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Renderitza la interficie principal del simulador (selector modal, barra de menu, panell de temps i HUD)
	InterficieUI::Renderitzar(g_EstatSimulador, escenariActual, escenaris, shader_programID, shader_programIDNormalMap);

	// Projeccio dinamica de les etiquetes 2D de tots els objectes observables sobre la pantalla:
	// nomes s'executa amb la simulacio activa (selector tancat) i l'opcio del menu activada.
	if (!g_EstatSimulador.mostrarSelectorModes &&
		g_EstatSimulador.opcionsVisualitzacio.mostrarEtiquetes &&
		objecte == SISTEMA_SOLAR &&
		escenariActual != nullptr &&
		!escenariActual->m_objectList.empty())
	{
		glm::vec4 viewport(0.0f, 0.0f, (float)w, (float)h);
		std::vector<const Object*> enfocables = escenariActual->obtenirObjectesEnfocables();
		for (const Object* obj : enfocables)
		{
			if (obj == nullptr || !obj->m_mostrarEtiqueta) continue;

			// Amagar etiquetes de satel-lits naturals en vista molt allunyada excepte si estan enfocats
			if (obj->m_categoria == CategoriaObjecte::SATELLIT_NATURAL &&
				obj->m_id != g_EstatSimulador.cosEnfocatId &&
				obj->m_parent != nullptr)
			{
				glm::vec3 posSat = glm::vec3(obj->m_trMatrix[3]);
				glm::vec3 posPare = glm::vec3(obj->m_parent->m_trMatrix[3]);
				glm::vec3 scrSat = glm::project(posSat, ViewMatrix, ProjectionMatrix, viewport);
				glm::vec3 scrPare = glm::project(posPare, ViewMatrix, ProjectionMatrix, viewport);
				if (scrSat.z > 0.0f && scrSat.z < 1.0f && scrPare.z > 0.0f && scrPare.z < 1.0f) {
					float dx = scrSat.x - scrPare.x;
					float dy = scrSat.y - scrPare.y;
					if ((dx * dx + dy * dy) < (24.0f * 24.0f)) continue;
				}
			}

			// Situa l'etiqueta lleugerament per sobre de cada cos (z = rCos * 1.15)
			glm::vec3 centreCos = glm::vec3(obj->m_trMatrix[3]);
			glm::vec3 posMon = centreCos + glm::vec3(0.0f, 0.0f, (float)(obj->m_scale * 1.15));

			InterficieUI::DibuixarEtiquetaPlaneta(obj->m_nom, posMon, ViewMatrix, ProjectionMatrix, viewport);
		}

		// Etiqueta 2D de la Nau Milano quan es visible a l'escena
		if (camera == CAM_NAVEGA || g_EstatSimulador.cosEnfocatId == CB_MILANO)
		{
			glm::vec3 posMonMilano = posMilano + glm::vec3(0.0f, 0.0f, 6.0f * 1.15f);
			InterficieUI::DibuixarEtiquetaPlaneta("Nau Milano", posMonMilano, ViewMatrix, ProjectionMatrix, viewport);
		}
	}

	// Finestres opcionals d'EntornVGI (desactivades per defecte)
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering ImGui Menus
	ImGui::Render();
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// CÀMERA--> NAVEGA:  Mode de navegació sobre un pla amb botons de teclat o de mouse (nav) (opció booleana)
void OnCameraNavega()
{
	// TODO: Agregue aquí su código de controlador de comandos
	if (projeccio != ORTO || projeccio != CAP)
	{
		camera = CAM_NAVEGA;
		// Carregar la Milano automàticament si encara no està carregada
		AssegurarCoetCarregat();
		// Desactivació de zoom, mobil, Transformacions Geomètriques via mouse i pan 
		//		si navega activat
		mobil = false;	zzoom = false;	satelit = false;
		transX = false;	transY = false;	transZ = false;
		//pan = false;
		tr_cpv.x = 0.0;		tr_cpv.y = 0.0;		tr_cpv.z = 0.0;		// Inicialitzar a 0 desplaçament de pantalla
		tr_cpvF.x = 0.0;	tr_cpvF.y = 0.0;	tr_cpvF.x = 0.0;	// Inicialitzar a 0 desplaçament de pantalla

		// Incialitzar variables Navega segons configuració eixos en Polars
		if (Vis_Polar == POLARZ) { /*COET*/
			// Posició inicial de la càmera darrere de la Milano
			opvN.x = 50.0;
			opvN.y = -25.0;
			opvN.z = 15.0;

			// Punt cap on mira la càmera: posició de la Milano
			n[0] = 30.0;
			n[1] = 0.0;
			n[2] = 0.0;
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
	// TODO: Agregue aquí su código de controlador de comandos
		//SkyBoxCube = !SkyBoxCube;

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
	// TODO: Agregue aquí su código de controlador de comandos
	projeccio = PERSPECT;
	mobil = true;			zzoom = true;		zzoomO = false;
}

// SHADER PHONG
void OnShaderPhong()
{
	GLuint newShaderID = 0;

	// TODO: Agregue aquí su código de controlador de comandos
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
		// Bloquejar dreceres i moviment d'escena mentre el selector modal inicial esta obert
		if (g_EstatSimulador.mostrarSelectorModes) return;

		// Dreceres globals del simulador sincronitzades amb els menus de la interficie:
		// Espai -> Pausa/Reprendre, Backspace -> Invertir temps, R -> Reenquadrar vista, Home -> Vista Global, Tab -> Canviar mode
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
				g_EstatSimulador.cosEnfocatId = CB_SUN;
				g_EstatSimulador.origenReferenciaSol = true;
				InterficieUI::ActualitzarDadesCosActiu(g_EstatSimulador, escenariActual);
				g_EstatSimulador.canviCameraPendent = true;
				return;
			}
			else if (key == GLFW_KEY_TAB) {
				ModeSimulacio nouMode = (g_EstatSimulador.modeActual == ModeSimulacio::INTUITIU)
					? ModeSimulacio::ESCALA_REAL
					: ModeSimulacio::INTUITIU;
				InterficieUI::SolicitarCanviModeDirecte(g_EstatSimulador, nouMode);
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
			// Adapta el zoom minim i pas de distancia segons el radi del cos enfocat (natural o artificial)
			CelestialBodyId cosId = g_EstatSimulador.cosEnfocatId;
			glm::vec3 posObj(0.0f);
			double scale = 16.0;
			ObtenirDadesEnfocament(escenariActual, cosId, posObj, scale);
			LimitsCameraCos lim = CalcularLimitsCameraPerCos(cosId, scale);
			const double pasDistancia = std::max(lim.pasZoom, OPV.R * 0.08);

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
			if (OPV.R < lim.minR)
				OPV.R = lim.minR;
			if (OPV.R > lim.maxR)
				OPV.R = lim.maxR;

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

void OnKeyUp(GLFWwindow* window, int key, int scancode, int action, int mods)
{}

void OnTextDown(GLFWwindow* window, unsigned int codepoint)
{}

// Teclat_Navega: Teclat pels moviments de navegació.
void Teclat_Navega(int key, int action)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	float pas = 2.0f;
	float gir = 5.0f;

	float angleRad = glm::radians(angleMilano);
	float inclinacioRad = glm::radians(inclinacioMilano);

	// Direcció cap on apunta la nau
	glm::vec3 direccio(
		cos(angleRad) * cos(inclinacioRad),
		sin(angleRad) * cos(inclinacioRad),
		sin(inclinacioRad)
	);

	switch (key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		posMilano += direccio * pas;
		break;

	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		posMilano -= direccio * pas;
		break;

	case GLFW_KEY_LEFT:
	case GLFW_KEY_A:
		angleMilano += gir;
		break;

	case GLFW_KEY_RIGHT:
	case GLFW_KEY_D:
		angleMilano -= gir;
		break;

	case GLFW_KEY_Q:
		posMilano.z += pas;
		break;

	case GLFW_KEY_E:
		posMilano.z -= pas;
		break;
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
	// TODO: Agregue aquí su código de controlador de mensajes o llame al valor predeterminado
	// Get the cursor position when the mouse key has been pressed or released.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// EntornVGI.ImGui: Test si events de mouse han sigut filtrats per ImGui (io.WantCaptureMouse)
	// (1) ALWAYS forward mouse data to ImGui! This is automatic with default backends. With your own backend:
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action);

	// (2) Nomes reenviar clics de ratoli a l'escena 3D si ImGui no captura el punter i el selector modal esta tancat
	if (!io.WantCaptureMouse && !g_EstatSimulador.mostrarSelectorModes) { //<Tractament mouse de l'aplicació>}
		// OnLButtonDown
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{	// Entorn VGI: Detectem en quina posició s'ha apretat el botó esquerra del
			//				mouse i ho guardem a la variable m_PosEAvall i activem flag m_ButoEAvall
			m_ButoEAvall = true;
			m_PosEAvall.x = xpos;	m_PosEAvall.y = ypos;
			m_EsfeEAvall = OPV;
		}
		// OnLButtonUp: Funció que es crida quan deixem d'apretar el botó esquerra del mouse.
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{	// Entorn VGI: Desactivem flag m_ButoEAvall quan deixem d'apretar botó esquerra del mouse.
			m_ButoEAvall = false;

			// OPCIÓ VISTA-->SATÈLIT: Càlcul increment desplaçament del Punt de Vista
			if ((satelit) && (projeccio != ORTO))
			{	//m_EsfeIncEAvall.R = m_EsfeEAvall.R - OPV.R;
				m_EsfeIncEAvall.alfa = 0.01f * (OPV.alfa - m_EsfeEAvall.alfa); //if (abs(m_EsfeIncEAvall.alfa)<0.01) { if ((m_EsfeIncEAvall.alfa)>0.0) m_EsfeIncEAvall.alfa = 0.01 else m_EsfeIncEAvall.alfa=0.01}
				m_EsfeIncEAvall.beta = 0.01f * (OPV.beta - m_EsfeEAvall.beta);
				if (abs(m_EsfeIncEAvall.beta) < 0.01)
				{
					if ((m_EsfeIncEAvall.beta) > 0.0) m_EsfeIncEAvall.beta = 0.01;
					else m_EsfeIncEAvall.beta = 0.01;
				}
				//if ((m_EsfeIncEAvall.R == 0.0) && (m_EsfeIncEAvall.alfa == 0.0) && (m_EsfeIncEAvall.beta == 0.0)) KillTimer(WM_TIMER);
				//else SetTimer(WM_TIMER, 10, NULL);
			}
		}
		// OnRButtonDown
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{	// Entorn VGI: Detectem en quina posició s'ha apretat el botó esquerra del
			//				mouse i ho guardem a la variable m_PosEAvall i activem flag m_ButoEAvall
			m_ButoDAvall = true;
			//m_PosDAvall = point;
			m_PosDAvall.x = xpos;	m_PosDAvall.y = ypos;
		}
		// OnLButtonUp: Funció que es crida quan deixem d'apretar el botó esquerra del mouse.
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		{	// Entorn VGI: Desactivem flag m_ButoEAvall quan deixem d'apretar botó esquerra del mouse.
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
	// Ignorar moviment del ratoli a l'escena 3D mentre el selector modal inicial esta obert
	if (g_EstatSimulador.mostrarSelectorModes) return;

	// TODO: Agregue aquí su código de controlador de mensajes o llame al valor predeterminado
	double modul = 0;
	GLdouble vdir[3] = { 0, 0, 0 };
	CSize gir = { 0,0 }, girn = { 0,0 }, girT = { 0,0 }, zoomincr = { 0,0 };

	// TODO: Add your message handler code here and/or call default
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
	else if (m_ButoEAvall && (camera == CAM_NAVEGA) && (projeccio != CAP && projeccio != ORTO)) // Opció Navegació
	{	// Entorn VGI: Canviar orientació en opció de Navegació
		girn.cx = m_PosEAvall.x - xpos;
		girn.cy = m_PosEAvall.y - ypos;

		// Gir horitzontal
		angleMilano -= girn.cx * 0.2f;

		// Inclinació vertical
		inclinacioMilano += girn.cy * 0.2f;
		
		// Limitar la inclinació vertical
		if (inclinacioMilano > 60.0f)
			inclinacioMilano = 60.0f;

		if (inclinacioMilano < -60.0f)
			inclinacioMilano = -60.0f;

		m_PosEAvall.x = xpos;
		m_PosEAvall.y = ypos;
	}

	// Entorn VGI: Transformació Geomètrica interactiva pels eixos X,Y boto esquerra del mouse.
	else {
		bool transE = transX || transY;
		if (m_ButoEAvall && transE && transf)
		{	// Calcular increment
			girT.cx = m_PosEAvall.x - xpos;		girT.cy = m_PosEAvall.y - ypos;
			if (transX)
			{
				long int incrT = girT.cx;
				if (trasl)
				{
					TG.VTras.x += incrT * fact_Tras;
					if (TG.VTras.x < -100000.0) TG.VTras.x = 100000.0;
					if (TG.VTras.x > 100000.0) TG.VTras.x = 100000.0;
				}
				else if (rota)
				{
					TG.VRota.x += incrT * fact_Rota;
					while (TG.VRota.x >= 360.0) TG.VRota.x -= 360.0;
					while (TG.VRota.x < 0.0) TG.VRota.x += 360.0;
				}
				else if (escal)
				{
					if (incrT < 0) incrT = -1 / incrT;
					TG.VScal.x = TG.VScal.x * incrT;
					if (TG.VScal.x < 0.25) TG.VScal.x = 0.25;
					if (TG.VScal.x > 8192.0) TG.VScal.x = 8192.0;
				}
			}
			if (transY)
			{
				long int incrT = girT.cy;
				if (trasl)
				{
					TG.VTras.y += incrT * fact_Tras;
					if (TG.VTras.y < -100000) TG.VTras.y = 100000;
					if (TG.VTras.y > 100000) TG.VTras.y = 100000;
				}
				else if (rota)
				{
					TG.VRota.y += incrT * fact_Rota;
					while (TG.VRota.y >= 360.0) TG.VRota.y -= 360.0;
					while (TG.VRota.y < 0.0) TG.VRota.y += 360.0;
				}
				else if (escal)
				{
					if (incrT <= 0.0)
					{
						if (incrT >= -2) incrT = -2;
						incrT = 1 / static_cast<long int>(log2(static_cast<double>(-incrT)));
					}
					else incrT = static_cast<long int>(log2(static_cast<double>(incrT)));
					TG.VScal.y = TG.VScal.y * incrT;
					if (TG.VScal.y < 0.25) TG.VScal.y = 0.25;
					if (TG.VScal.y > 8192.0) TG.VScal.y = 8192.0;
				}
			}
			m_PosEAvall.x = xpos;	m_PosEAvall.y = ypos;
			// Crida a OnPaint() per redibuixar l'escena
			//InvalidateRect(NULL, false);
			//OnPaint(windows);
		}
	}

	// Entorn VGI: Determinació del desplaçament del pan segons l'increment
	//				vertical de la posició del mouse (tecla dreta apretada).
	if (m_ButoDAvall && pan && (projeccio != CAP && projeccio != ORTO))
	{
		//CSize zoomincr = m_PosDAvall - point;
		zoomincr.cx = m_PosDAvall.x - xpos;		zoomincr.cy = m_PosDAvall.y - ypos;
		long int incrx = zoomincr.cx;
		long int incry = zoomincr.cy;

		// Desplaçament pan vertical
		tr_cpv.y -= incry * fact_pan;
		if (tr_cpv.y > 100000.0) tr_cpv.y = 100000.0;
		else if (tr_cpv.y < -100000.0) tr_cpv.y = -100000.0;

		// Desplaçament pan horitzontal
		tr_cpv.x += incrx * fact_pan;
		if (tr_cpv.x > 100000.0) tr_cpv.x = 100000.0;
		else if (tr_cpv.x < -100000.0) tr_cpv.x = -100000.0;

		//m_PosDAvall = point;
		m_PosDAvall.x = xpos;	m_PosDAvall.y = ypos;
		// Crida a OnPaint() per redibuixar l'escena
		//OnPaint(window);
	}
	// Determinació del paràmetre R segons l'increment
	//   vertical de la posició del mouse (tecla dreta apretada)
		//else if (m_ButoDAvall && zzoom && (projeccio!=CAP && projeccio!=ORTO))
	else if (m_ButoDAvall && zzoom && (projeccio != CAP))
	{	//CSize zoomincr = m_PosDAvall - point;
		zoomincr.cx = m_PosDAvall.x - xpos;		zoomincr.cy = m_PosDAvall.y - ypos;
		long int incr = zoomincr.cy / 1.0;

		if (camera == CAM_ESFERICA) {	// Càmera Esfèrica
			OPV.R = OPV.R + incr;
			//if (OPV.R < 0.25) OPV.R = 0.25;
			if (OPV.R < p_near) OPV.R = p_near;
			if (OPV.R > p_far) OPV.R = p_far;
		}

		//m_PosDAvall = point;
		m_PosDAvall.x = xpos;				m_PosDAvall.y = ypos;
		// Crida a OnPaint() per redibuixar l'escena
		//OnPaint(window);
	}
	else if (m_ButoDAvall && (camera == CAM_NAVEGA) && (projeccio != CAP && projeccio != ORTO))
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
			// Crida a OnPaint() per redibuixar l'escena
			//OnPaint(window);
		}
	}

	// Entorn VGI: Transformació Geomètrica interactiva per l'eix Z amb boto dret del mouse.
	else if (m_ButoDAvall && transZ && transf)
	{	// Calcular increment
		girT.cx = m_PosDAvall.x - xpos;		girT.cy = m_PosDAvall.y - ypos;
		long int incrT = girT.cy;
		if (trasl)
		{
			TG.VTras.z += incrT * fact_Tras;
			if (TG.VTras.z < -100000.0) TG.VTras.z = 100000.0;
			if (TG.VTras.z > 100000.0) TG.VTras.z = 100000.0;
		}
		else if (rota)
		{
			incrT = girT.cx;
			TG.VRota.z += incrT * fact_Rota;
			while (TG.VRota.z >= 360.0) TG.VRota.z -= 360.0;
			while (TG.VRota.z < 0.0) TG.VRota.z += 360.0;
		}
		else if (escal)
		{
			if (incrT <= 0) {
				if (incrT >= -2.0) incrT = -2.0;
				incrT = 1.0 / static_cast<double>(static_cast<int>(log2(static_cast<double>(-incrT))));
			}
			else incrT = static_cast<long int>(log2(static_cast<double>(incrT)));
			TG.VScal.z = TG.VScal.z * incrT;
			if (TG.VScal.z < 0.25) TG.VScal.z = 0.25;
			if (TG.VScal.z > 8192.0) TG.VScal.z = 8192.0;
		}
		//m_PosDAvall = point;
		m_PosDAvall.x = xpos;	m_PosDAvall.y = ypos;
		// Crida a OnPaint() per redibuixar l'escena
		//OnPaint(window);
	}
}

// OnMouseWheel: Funció que es crida quan es mou el rodet del mouse. La utilitzem per la 
//				  Visualització Interactiva per modificar el paràmetre R de P.V. (R,angleh,anglev) 
//				  segons el moviment del rodet del mouse.
//      PARAMETRES: -  (xoffset,yoffset): Estructura (x,y) que dóna la posició del mouse 
//							 (coord. pantalla) quan el botó s'ha apretat.
void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
	// TODO: Agregue aquí su código de controlador de mensajes o llame al valor predeterminado
	double modul = 0;
	GLdouble vdir[3] = { 0, 0, 0 };

	// EntornVGI.ImGui: Test si events de mouse han sigut filtrats per ImGui (io.WantCaptureMouse)
	// (1) ALWAYS forward mouse data to ImGui! This is automatic with default backends. With your own backend:
	ImGuiIO& io = ImGui::GetIO();
	//io.AddMouseButtonEvent(button, true);

// (2) Nomes aplicar zoom amb el rodet si ImGui no captura el ratoli i el selector inicial esta tancat
	if (!io.WantCaptureMouse && !g_EstatSimulador.mostrarSelectorModes) { // <Tractament mouse de l'aplicació>}
		// Funció de zoom quan està activada la funció pan o les T. Geomètriques
		if ((zzoom || zzoomO) || (transX) || (transY) || (transZ))
		{
			if (camera == CAM_ESFERICA)
			{	// Càmera Esfèrica: limits i velocitat de zoom adaptats al cos enfocat (natural o artificial)
				CelestialBodyId cosId = g_EstatSimulador.cosEnfocatId;
				glm::vec3 posObj(0.0f);
				double scale = 16.0;
				ObtenirDadesEnfocament(escenariActual, cosId, posObj, scale);
				LimitsCameraCos lim = CalcularLimitsCameraPerCos(cosId, scale);
				double pasZoom = std::max(lim.pasZoom, OPV.R * 0.08);

				OPV.R = OPV.R - yoffset * pasZoom;
				if (OPV.R < lim.minR)
					OPV.R = lim.minR;
				if (OPV.R > lim.maxR)
					OPV.R = lim.maxR;

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
			double pasNav = yoffset * 2.5;
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

#include <ctime>
double toJ2000Seconds(int year, int month, int day, int hour, int minute, double second) {
	if (month <= 2) {
		year -= 1;
		month += 12;
	}

	int a = year / 100;
	int b = 2 - a + (a / 4);

	// Days since J2000.0 (2000-01-01 12:00:00)
	// -1524.5 (JD offset) - 2451545.0 (J2000 epoch) = -2453069.5
	double daysSinceJ2000 = std::floor(365.25 * (year + 4716))
		+ std::floor(30.6001 * (month + 1))
		+ day
		+ b
		- 2453069.5;

	return (daysSinceJ2000 * 86400.0) + (hour * 3600.0) + (minute * 60.0) + second;
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

	// Antialiasing (MSAA x4): el framebuffer de la finestra guarda 4 mostres per pixel.
	// Ha d'anar ABANS de glfwCreateWindow: els hints nomes afecten les finestres que es creen despres.
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	// GLFW_SAMPLES s'ha mogut abans de glfwCreateWindow (aqui ja no tenia efecte).
	glEnable(GL_MULTISAMPLE);	// Activar el multisampling (antialiasing de linies i vores dels poligons)
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
	//glfwSetCharCallback(window, OnTextDown);										// - Directly redirect GLFW char events
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

// Entorn VGI.ImGui: Configuracio de tema fosc espacial i carrega de fonts (InterficieUI)
	InterficieUI::InicialitzarEstil();

	// Entorn VGI.ImGui: Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	// Entorn VGI.ImGui: End Setup Dear ImGui context

	// Reserva de l'array d'escenaris (0: Mode Didactic, 1: Mode a Escala Real).
	// L'escena del sistema solar NO es carrega ni es dibuixa mentre el selector inicial de modes estigui obert.
	static constexpr int NUM_ESCENARIS = 2;
	escenaris = new Escenari[NUM_ESCENARIS];
	escenariActual = &escenaris[0];
	objecte = CAP;

	objecte = SISTEMA_SOLAR;
	camera = CAM_ESFERICA;		Vis_Polar = POLARZ;
	t_textura = TEXTURA_FITXERIMA;	tFlag_invert_Y = true;	textura = true;
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
				// En mode didàctic el rellotge avança a ritme moderat; en escala real segons el timelapse (factorVelocitatSegonsPerSegon)
				//double deltaRellotge = escalaReal ? (double)delta : ((double)delta * 0.25);
				double deltaRellotge = delta;
				g_EstatSimulador.tempsSimulacio.AvancarSegons(deltaRellotge);
			}

			// Propaga pausa, direccio, mode i velocitat del timelapse cap a l'Escenari actiu
			if (escenariActual != nullptr && objecte == SISTEMA_SOLAR) {
				escenariActual->updateVariables(
					toJ2000Seconds(g_EstatSimulador.tempsSimulacio.dataHora.any, 
						g_EstatSimulador.tempsSimulacio.dataHora.mes,
						g_EstatSimulador.tempsSimulacio.dataHora.dia,
						g_EstatSimulador.tempsSimulacio.dataHora.hora,
						g_EstatSimulador.tempsSimulacio.dataHora.minut,
						g_EstatSimulador.tempsSimulacio.dataHora.segon
					),
					//Lo es de abajo no deberian estar, pero bueno
					(double)delta,
					g_EstatSimulador.tempsSimulacio.enPausa,
					g_EstatSimulador.tempsSimulacio.direccioTemps,
					escalaReal,
					g_EstatSimulador.tempsSimulacio.factorVelocitatSegonsPerSegon
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

	delete[] escenaris;
	escenaris = nullptr;
	escenariActual = nullptr;

	glfwDestroyWindow(window);

	// Terminating GLFW: Destroy the windows, monitors and cursor objects
	glfwTerminate();

	if (shaderLighting.getProgramID() != -1) shaderLighting.DeleteProgram();
	if (shaderSkyBox.getProgramID() != -1) shaderSkyBox.DeleteProgram();
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
