// escenari.h — Arquitectura: Escenari Base
// Defineix la classe base polimòrfica Escenari, la configuració de càmera per mode i la gestió comuna de textures i cossos.

#pragma once

#include "framework.h"
#include "material.h"
#include "visualitzacio.h"

struct EstatSimulacio;

// Configuració de paràmetres de càmera, zoom i atenuació lumínica propis de cada mode d'escenari
struct ConfigCamera
{
	float distInicialSol = 140.0f;         // [unitats OpenGL] Distància inicial al Sol en mode Fixa/Orbital (> 0)
	float angleInclinacioInicial = 25.0f;  // [graus] Elevació inicial de la càmera esfèrica [-89..89]
	float globalSistemaR = 550.0f;         // [unitats OpenGL] Radi esfèric en vista Global del Sistema (> 0)
	float angleGlobal = 65.0f;             // [graus] Elevació de la càmera en vista Global [-89..89]
	float minDistSol = 42.0f;              // [unitats OpenGL] Distància mínima de zoom centrats al Sol (> 0)
	bool  minDistSolFixa = true;           // Si true usa minDistSol fixa; si false escala amb factorRadiMinZoom
	float maxR = 800.0f;                   // [unitats OpenGL] Distància màxima permesa de la càmera esfèrica (> minDistSol)
	float factorFocusPlaneta = 4.5f;       // [adimensional] Multiplicador sobre el radi visual per al zoom inicial en enfocar un planeta
	float minDistFocusPlaneta = 14.0f;     // [unitats OpenGL] Distància mínima de càmera en seleccionar un planeta (> 0)
	float minDistFocusZoom = 2.5f;         // [unitats OpenGL] Límit absolut inferior de zoom fent scroll (> 0)
	float factorRadiMinZoom = 2.2f;        // [adimensional] Multiplicador mínim sobre el radi visual per evitar travessar l'esfera
	float pasZoomSol = 5.0f;               // [unitats OpenGL/pas] Increment lineal de zoom enfocant el Sol
	float minPasZoomPlaneta = 1.0f;        // [unitats OpenGL/pas] Increment mínim lineal de zoom enfocant un planeta
	float factorPasZoomPlaneta = 0.5f;     // [adimensional] Fracció del radi visual aplicada al pas de zoom lineal
	bool  zoomLogaritmic = false;          // Activa el zoom proporcional a la distància actual (escala real)
	float factorZoomLogaritmic = 0.12f;    // [adimensional] Fracció de distActualR per pas quan zoomLogaritmic == true
	float minPasZoomLogaritmic = 0.0005f;  // [unitats OpenGL/pas] Pas mínim en mode logarítmic (> 0)
	float navPosX = 110.0f;                // [unitats OpenGL] Coordenada X inicial en càmera lliure (Navega)
	float navPosY = -110.0f;               // [unitats OpenGL] Coordenada Y inicial en càmera lliure (Navega)
	float navPosZ = 30.0f;                 // [unitats OpenGL] Coordenada Z inicial en càmera lliure (Navega)
	float navPas = 2.5f;                   // [unitats OpenGL/pulsació] Velocitat de desplaçament en càmera lliure (> 0)
	float atenuacioQuad = 0.00004f;        // [1/unitats^2] Coeficient d'atenuació quadràtica per a LightSource[0]

	// Paràmetres: cosId (0 = Sol, > 0 = planeta/satèl·lit), radiVisual [unitats OpenGL] (>= 0).
	// Retorn: distància mínima de seguretat R [unitats OpenGL] per al zoom esfèric.
	float getMinR(int cosId, float radiVisual) const
	{
		if (cosId == 0 && minDistSolFixa)
			return minDistSol;
		return glm::max(minDistFocusZoom, radiVisual * factorRadiMinZoom);
	}

	// Paràmetres: cosId (0 = Sol, > 0 = cos enfocat), radiVisual [unitats OpenGL] (>= 0).
	// Retorn: distància per defecte R [unitats OpenGL] en enfocar el cos celeste.
	float getDistDefecte(int cosId, float radiVisual) const
	{
		if (cosId == 0)
			return distInicialSol;
		return glm::max(minDistFocusPlaneta, radiVisual * factorFocusPlaneta);
	}

	// Paràmetres: cosId, radiVisual [unitats OpenGL], distActualR [unitats OpenGL] (>= 0).
	// Retorn: increment de radi deltaR [unitats OpenGL] per a un esdeveniment de roda del ratolí.
	float getPasZoom(int cosId, float radiVisual, float distActualR) const
	{
		if (zoomLogaritmic)
			return glm::max(minPasZoomLogaritmic, distActualR * factorZoomLogaritmic);
		if (cosId == 0)
			return pasZoomSol;
		return glm::max(minPasZoomPlaneta, radiVisual * factorPasZoomPlaneta);
	}
};

// Dades individuals d'estat cinemàtic i visual d'un cos celeste dins de l'escenari
class Object
{
public:
	// Inicialitza un cos celeste amb valors nuls i matriu de rotació identitat.
	Object()
		: m_nom("")
		, m_radiCos(0.0f)
		, m_radiOrbita(0.0f)
		, m_angleOrbita(0.0f)
		, m_velAngular(0.0f)
		, m_angleRotacio(0.0f)
		, m_velRotacio(0.0f)
		, m_posicio(0.0f, 0.0f, 0.0f)
		, m_mMatrix(glm::mat4(1.0f))
	{}
	~Object() {}

	std::string m_nom;       // Nom visible del cos celeste
	float m_radiCos;         // [unitats OpenGL] Radi visual de l'esfera 3D (> 0)
	float m_radiOrbita;      // [unitats OpenGL] Radi orbital mitjà de referència (>= 0)
	float m_angleOrbita;     // [graus, 0..360) Angle orbital instantani (mode intuïtiu)
	float m_velAngular;      // [graus/s] Velocitat angular orbital (mode intuïtiu)

	float m_angleRotacio;    // [graus, 0..360) Angle de rotació sobre el propi eix (mode intuïtiu)
	float m_velRotacio;      // [graus/s] Velocitat de rotació axial (mode intuïtiu; negatiu = retrògrad)

	glm::vec3 m_posicio;     // [unitats OpenGL] Posició cartesiana en coordenades de món
	glm::mat4 m_mMatrix;     // Matriu d'orientació/rotació 3D local -> món (usada per SPICE en escala real)
};

// Classe base polimòrfica per als escenaris del Sistema Solar (Intuïtiu vs Escala Real)
class Escenari
{
public:
	// Inicialitza l'array de textures a 0 i el color base a blanc.
	Escenari();
	// Allibera les textures comunes i buida la llista de cossos.
	virtual ~Escenari();

	// No es permet copiar un Escenari (gestiona recursos OpenGL).
	Escenari(const Escenari&) = delete;
	Escenari& operator=(const Escenari&) = delete;

	// Precondició: context OpenGL actiu i `shader_programID` compilat/enllaçat.
	// Inicialitza VAOs, textures i cossos celestes de l'escenari.
	virtual void creaEscenari(GLuint& shader_programID) = 0;

	// Precondició: `creaEscenari` executat prèviament.
	// Paràmetres: `dt` [s] delta temporal del frame (>= 0), `estat` estat global mutable de la simulació.
	virtual void updateVariables(double dt, EstatSimulacio& estat) = 0;

	// Precondició: context OpenGL i shader `sh_programID` vàlids.
	// Paràmetres: `MatriuVista`, `MatriuTG` (transformació arrel), `sw_mat[5]` (interruptors d'emissió/ambient/difusa/especular/material), `estat`.
	virtual void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) = 0;

	// Allibera els recursos comuns (textures). Les subclasses han d'alliberar els seus VAOs propis.
	virtual void alliberaEscenari();

	// Retorn: nombre total de cossos celestes registrats a `m_cossos` (>= 0).
	int getNumCossos() const { return static_cast<int>(m_cossos.size()); }

	// Paràmetres: `idx` índex dins [0, getNumCossos() - 1].
	// Retorn: posició absoluta [unitats OpenGL] del cos (o (0,0,0) si l'índex és invàlid).
	glm::vec3 getPosicioCos(int idx) const;

	// Paràmetres: `idx`, `cosEnfocatId`, `origenRelatiu` (si true, la càmera orbita al voltant del cos enfocat).
	// Retorn: posició del cos [unitats OpenGL] en l'espai de vista usat per al renderitzat i projecció 2D.
	virtual glm::vec3 getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const;

	// Paràmetres: `idx` índex del cos celeste.
	// Retorn: color característic [RGB normalitzat 0..1] per a marcadors 2D i òrbites.
	virtual glm::vec3 getColorCos(int idx) const;

	// Paràmetres: `idx` índex dins [0, getNumCossos() - 1].
	// Retorn: radi visual [unitats OpenGL] (>= 0.0; 0.0 si l'índex és invàlid).
	float getRadiCos(int idx) const;

	// Paràmetres: `idx` índex dins [0, getNumCossos() - 1].
	// Retorn: nom del cos celeste (cadena buida si l'índex és invàlid).
	std::string getNomCos(int idx) const;

	// Retorn: referència constant a la configuració de càmera i atenuació pròpia de l'escenari.
	const ConfigCamera& getConfigCamera() const { return m_configCamera; }

protected:
	// Precondició: context OpenGL actiu.
	// Paràmetres: `rutaFitxer` ruta relativa al fitxer d'imatge.
	// Retorn: identificador de textura OpenGL (> 0), o 0 si falla la càrrega.
	GLuint carregaTextura2D(const char* rutaFitxer);

	// Elimina totes les textures OpenGL no nul·les de `m_texturesID` i les reinicia a 0.
	void alliberaTextures();

	std::vector<Object> m_cossos;            // Vector de cossos celestes de l'escenari
	GLuint m_texturesID[NUM_MAX_TEXTURES];   // Identificadors OpenGL de textures 2D [0 = sense textura]
	CColor m_colObj;                         // [RGBA 0..1] Color base per defecte dels objectes
	ConfigCamera m_configCamera;             // Paràmetres de càmera, zoom i atenuació propis de l'escenari
};
