// GestorRecursos.h : Repositori compartit de textures, skybox, VAOs i cache d'uniforms
//
// Objectius de disseny:
// - Evitar duplicacio de textures en VRAM entre escenaris[0] (Intuitiu) i escenaris[1] (Escala Real).
// - Precarregar i descodificar imatges a RAM en segon pla (std::thread) per arrencada instantania.
// - Pujar textures a GPU progressivament (2-3 per frame) sense micro-congelacions.
// - Cachejar ubicacions d'uniforms de shaders per eliminar glGetUniformLocation / glGetUniform* per frame.

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "framework.h"
#include "constants.h"

// Dades d'una imatge descodificada a RAM (sense crides OpenGL)
struct ImatgeCPU {
	unsigned char* pixels = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	std::string rutaNormalitzada;
};

// Parametres d'il-luminacio passats directament des de CPU per sincronitzar shaders sense glGetUniform*
struct ParametresIluminacioEscenari {
	glm::mat4 matriuProjeccio = glm::mat4(1.0f);
	const LLUM* llums = nullptr;
	int numLlums = 0;
	bool llumAmbient = false;
	bool llumFixa = true;
};

// Cache d'ubicacions d'uniforms per evitar consultes al driver OpenGL en cada fotograma
struct CacheUniformsShader {
	GLuint programID = 0;
	GLint locProjectionMatrix = -1;
	GLint locViewMatrix = -1;
	GLint locModelMatrix = -1;
	GLint locNormalMatrix = -1;
	GLint locUseNormalMatrixPV = -1;
	GLint locUNormalMatrixPV = -1;
	GLint locTexture0 = -1;
	GLint locNormalMap = -1;
	GLint locTextur = -1;
	GLint locModulate = -1;
	GLint locFlagInvertY = -1;
	GLint locSwMaterial = -1;
	GLint locSwIntensity = -1;
	GLint locSwTwoSided = -1;
	GLint locLightModelAmbient = -1;
	GLint locFixedLight = -1;
	GLint locMatEmission = -1;
	GLint locMatAmbient = -1;
	GLint locMatDiffuse = -1;
	GLint locMatSpecular = -1;
	GLint locMatShininess = -1;

	struct LocLlum {
		GLint swLight = -1;
		GLint position = -1;
		GLint ambient = -1;
		GLint diffuse = -1;
		GLint specular = -1;
		GLint attenuation = -1;
		GLint restricted = -1;
		GLint spotDirection = -1;
		GLint spotCosCutoff = -1;
		GLint spotExponent = -1;
	};
	LocLlum locLlums[8];

	// Es calcula un sol cop quan canvia programID
	void ActualitzarSiCal(GLuint id);
};

class GestorRecursos
{
public:
	// Inicia el fil secundari de descodificacio de textures planetaries i Skybox a RAM
	static void IniciarPrecarregaEscena();

	// Puja fins a maxTexturesPerFrame textures ja descodificades de RAM a VRAM (cridar al fil principal)
	static void ProcessarCuaPujadaGPU(int maxTexturesPerFrame = 3);

	// Espera el fil de precarrega si encara esta actiu i puja totes les textures pendents a GPU
	static void FinalitzarPrecarregaGPU();

	// Retorna una textura 2D compartida (del cache GPU, de la cua CPU o carregant-la si cal)
	static GLuint ObtenirTextura2D(const char* ruta);

	// Retorna la textura 1x1 plana (0, 0, 1) unica i compartida per a cossos sense mapa normal
	static GLuint ObtenirNormalMapPerDefecte();

	// Retorna la textura cubemap compartida del Skybox (puja des de CPU o carrega de disc si cal)
	static GLuint ObtenirTexturaSkybox();

	// Retorna true si el cubemap del Skybox ja es troba llest a GPU
	static bool SkyboxLlestAGPU();

	// Assegura que els VAOs compartits del Sistema Solar (GLU_SPHERE, GLUT_TORUS) estan creats un sol cop
	static void AssegurarVAOsEscenari();

	// Atura el fil de precarrega, allibera buffers CPU pendents, textures OpenGL i VAOs compartits
	// Cal cridar-ho al tancament de main() ABANS de glfwDestroyWindow().
	static void AlliberarTot();
};
