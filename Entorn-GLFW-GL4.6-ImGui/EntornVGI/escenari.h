#pragma once

#include "framework.h"
#include "material.h"
#include "visualitzacio.h"

struct EstatSimulacio;

// Configuracio de parametres de camera i escala propis de cada mode d'escenari
struct ConfigCamera
{
	float distInicialSol = 140.0f;
	float angleInclinacioInicial = 25.0f;
	float globalSistemaR = 550.0f;
	float angleGlobal = 65.0f;
	float minDistSol = 42.0f;
	bool  minDistSolFixa = true;
	float maxR = 800.0f;
	float factorFocusPlaneta = 4.5f;
	float minDistFocusPlaneta = 14.0f;
	float minDistFocusZoom = 2.5f;
	float factorRadiMinZoom = 2.2f;
	float pasZoomSol = 5.0f;
	float minPasZoomPlaneta = 1.0f;
	float factorPasZoomPlaneta = 0.5f;
	bool  zoomLogaritmic = false;
	float factorZoomLogaritmic = 0.12f;
	float minPasZoomLogaritmic = 0.0005f;
	float navPosX = 110.0f;
	float navPosY = -110.0f;
	float navPosZ = 30.0f;
	float navPas = 2.5f;
	float atenuacioQuad = 0.00004f;

	float getMinR(int cosId, float radiVisual) const
	{
		if (cosId == 0 && minDistSolFixa)
			return minDistSol;
		return glm::max(minDistFocusZoom, radiVisual * factorRadiMinZoom);
	}

	float getDistDefecte(int cosId, float radiVisual) const
	{
		if (cosId == 0)
			return distInicialSol;
		return glm::max(minDistFocusPlaneta, radiVisual * factorFocusPlaneta);
	}

	float getPasZoom(int cosId, float radiVisual, float distActualR) const
	{
		if (zoomLogaritmic)
			return glm::max(minPasZoomLogaritmic, distActualR * factorZoomLogaritmic);
		if (cosId == 0)
			return pasZoomSol;
		return glm::max(minPasZoomPlaneta, radiVisual * factorPasZoomPlaneta);
	}
};

// Dades individuals de cada cos celeste dins de l'escenari
class Object
{
public:
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

	std::string m_nom;
	float m_radiCos;
	float m_radiOrbita;
	float m_angleOrbita;
	float m_velAngular;

	float m_angleRotacio;
	float m_velRotacio;

	glm::vec3 m_posicio;
	glm::mat4 m_mMatrix;
};

// Classe base polimorfica per als escenaris del Sistema Solar (Intuitiu vs Escala Real)
class Escenari
{
public:
	Escenari();
	virtual ~Escenari();

	// No es permet copiar un Escenari (gestiona recursos OpenGL)
	Escenari(const Escenari&) = delete;
	Escenari& operator=(const Escenari&) = delete;

	virtual void creaEscenari(GLuint& shader_programID) = 0;
	virtual void updateVariables(double dt, EstatSimulacio& estat) = 0;
	virtual void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) = 0;

	// Allibera els recursos comuns (textures). Les subclasses han d'alliberar els seus VAOs propis.
	virtual void alliberaEscenari();

	// Getters comuns per a camera, focus, marcadors i etiquetes 2D
	int getNumCossos() const { return static_cast<int>(m_cossos.size()); }
	glm::vec3 getPosicioCos(int idx) const;
	virtual glm::vec3 getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const;
	virtual glm::vec3 getColorCos(int idx) const;
	float getRadiCos(int idx) const;
	std::string getNomCos(int idx) const;
	const ConfigCamera& getConfigCamera() const { return m_configCamera; }

protected:
	GLuint carregaTextura2D(const char* rutaFitxer);
	void alliberaTextures();

	std::vector<Object> m_cossos;
	GLuint m_texturesID[NUM_MAX_TEXTURES];
	CColor m_colObj;
	ConfigCamera m_configCamera;
};
