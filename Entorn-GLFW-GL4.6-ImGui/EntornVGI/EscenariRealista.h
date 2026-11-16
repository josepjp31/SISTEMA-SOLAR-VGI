// EscenariRealista.h — Arquitectura: Escenari Realista
// Implementa el mode científic a escala real (1 UA = 1000 u) connectat al bridge NASA CSPICE.

#pragma once

#include "escenari.h"
#include "CatalegAstronomic.h"

// VAO de línia 3D (GL_LINE_STRIP) per a una òrbita del mode realista
struct OrbitaLineVAO
{
	GLuint vaoId = 0;                                // Identificador del Vertex Array Object OpenGL (0 = no inicialitzat)
	GLuint vboId = 0;                                // Identificador del Vertex Buffer Object OpenGL (0 = no inicialitzat)
	GLint nVertexs = 0;                              // Nombre de vèrtexs de la polilínia orbital (>= 0)
	int cosPareIdx = 0;                              // Índex del cos pare: 0 heliocèntric (Sol), 3 geocèntric (Terra per a la Lluna)
	glm::vec3 color = glm::vec3(0.55f, 0.72f, 0.90f); // [RGB normalitzat 0..1] Color base de la línia orbital
};

// Escenari del Mode a Escala Real (Científic):
// Sol, 8 planetes, Plutó i la Lluna connectats amb efemèrides reals NASA CSPICE,
// òrbites 3D reals (GL_LINE_STRIP), rotació sidèria + obliqüitat axial (IAU_*) i temps real.
class EscenariRealista : public Escenari
{
public:
	// Unitats astronòmiques i factor d'escala espacial OpenGL:
	// 1 UA = 149.597.870,7 km equival a 1.000,0 unitats d'escena (Terra a ~1.000 u, Neptú a ~30.000 u).
	static constexpr double KM_PER_AU = CatalegAstronomic::KM_PER_AU; // [km/UA]
	static constexpr double SCALE_UNITS_PER_AU = 1000.0;              // [unitats OpenGL / UA]
	static constexpr float MIN_RADI_VISUAL = 1e-4f;                   // [unitats OpenGL] Llindar mínim de seguretat del radi
	static constexpr int NUM_COSSOS_REALISTA = CatalegAstronomic::NUM_COSSOS; // Nombre de cossos: Sol + 8 planetes + Plutó + Lluna (11)
	static constexpr int NUM_ORBIT_SEGMENTS = 240;                    // Nombre de segments de mostreig per període orbital

	// Marc de referència inercial de les efemèrides NASA CSPICE:
	// "J2000" (Equador Mitjà Terrestre J2000 / ICRF) és mútua i físicament consistent entre
	// posicions (spkezr_c), òrbites 3D (getOrbitStates) i matrius de rotació (pxform_c IAU_* -> J2000).
	static constexpr const char* MARC_REFERENCIA_SPICE = "J2000";

	// Inicialitza els buffers en doble precisió i configura la càmera per a escala astronòmica (zoom logarítmic).
	EscenariRealista();
	// Allibera recursos OpenGL i descarrega els kernels CSPICE si romanen carregats.
	virtual ~EscenariRealista() override;

	// Precondició: context OpenGL actiu i `shader_programID` enllaçat.
	// Crea l'esfera i el disc de Saturn, carrega textures, escala els radis físics i carrega els kernels CSPICE.
	void creaEscenari(GLuint& shader_programID) override;

	// Precondició: `creaEscenari` executat prèviament.
	// Paràmetres: `dt` [s] delta temporal (limitat a [0.0, 0.25] s), `estat` estat del simulador.
	// Avança el temps d'efemèrides `m_ephemerisTimeET`, regenera òrbites quan cal i actualitza estats CSPICE.
	void updateVariables(double dt, EstatSimulacio& estat) override;

	// Paràmetres: `sh_programID`, `MatriuVista`, `MatriuTG`, `sw_mat[5]`, `estat`.
	// Dibuixa les òrbites 3D, els 11 cossos celestes i l'anell de Saturn usant origen relatiu si escau.
	void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) override;

	// Allibera VAOs/VBOs de primitives i òrbites, textures comunes i descarrega els kernels CSPICE.
	void alliberaEscenari() override;

	// Retorn: temps d'efemèrides actual en [s TDB des de J2000].
	double getEphemerisTimeET() const { return m_ephemerisTimeET; }

	// Retorn: true si els kernels NASA CSPICE han estat carregats mitjançant `loadCspiceKernels()`.
	bool areKernelsCarregats() const { return m_kernelsCarregats; }

	// Paràmetres: `idx`, `cosEnfocatId`, `origenRelatiu`.
	// Retorn: posició [unitats OpenGL] restant en `double` abans de convertir a `float` si `origenRelatiu` és actiu.
	glm::vec3 getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const override;

	// Paràmetres: `idx` índex dins [0, NUM_COSSOS_REALISTA - 1].
	// Retorn: color característic oficial del catàleg [RGB 0..1].
	glm::vec3 getColorCos(int idx) const override;

private:
	// Recalcula la polilínia orbital del cos `idx` a l'època `et` [s TDB des de J2000].
	void recalcularOrbitaCos(int idx, double et);
	// Recalcula totes les òrbites 3D (índexs 1..NUM_COSSOS_REALISTA - 1) a l'època `et` [s TDB des de J2000].
	void recalcularOrbites3D(double et);
	// Allibera tots els VAOs/VBOs de `m_orbitesVAO`.
	void alliberaOrbitesLineStrip();
	// Consulta a CSPICE les posicions [UA] i matrius de rotació J2000 dels 11 cossos a l'època `et`.
	void actualitzaCossosSPICE(double et);

	double m_ephemerisTimeET;                        // [s TDB des de J2000] Època física actual de la simulació
	bool m_kernelsCarregats;                         // Indica si els kernels SPICE són actius en memòria
	bool m_primerFrame;                              // Forza la inicialització d'època i òrbites al primer updateVariables
	glm::dvec3 m_posicionsAU[NUM_COSSOS_REALISTA];   // [UA] Posicions heliocèntriques en doble precisió dins de J2000
	double m_etOrbita[NUM_COSSOS_REALISTA];          // [s TDB des de J2000] Època de referència en què es va generar cada òrbita

	OrbitaLineVAO m_orbitesVAO[NUM_COSSOS_REALISTA]; // VAOs/VBOs de les òrbites 3D
};
