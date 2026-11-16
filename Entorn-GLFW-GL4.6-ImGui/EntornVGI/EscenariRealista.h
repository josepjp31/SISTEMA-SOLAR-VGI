#pragma once

#include "escenari.h"
#include "CatalegAstronomic.h"

// VAO de linia 3D (GL_LINE_STRIP) per a una orbita del mode realista
struct OrbitaLineVAO
{
	GLuint vaoId = 0;
	GLuint vboId = 0;
	GLint nVertexs = 0;
	int cosPareIdx = 0; // 0: heliocentric (Sol), 3: geocentric (Terra per a la Lluna)
	glm::vec3 color = glm::vec3(0.55f, 0.72f, 0.90f);
};

// Escenari del Mode a Escala Real (Cientific):
// Sol, 8 planetes, Pluto i la Lluna connectats amb efemerides reals NASA CSPICE,
// orbites 3D reals (GL_LINE_STRIP), rotacio siderea + oblicuitat axial (IAU_*) i temps real.
class EscenariRealista : public Escenari
{
public:
	// Unitats astronomiques i factor d'escala espacial OpenGL:
	// 1 UA = 149.597.870,7 km equival a 1.000,0 unitats d'escena (Terra a ~1.000 u, Neptu a ~30.000 u).
	static constexpr double KM_PER_AU = CatalegAstronomic::KM_PER_AU;
	static constexpr double SCALE_UNITS_PER_AU = 1000.0;
	static constexpr float MIN_RADI_VISUAL = 1e-4f;
	static constexpr int NUM_COSSOS_REALISTA = CatalegAstronomic::NUM_COSSOS; // Sol + 8 planetes + Pluto + Lluna
	static constexpr int NUM_ORBIT_SEGMENTS = 240;

	// Marc de referencia inercial de les efemerides NASA CSPICE:
	// "J2000" (Equador Mitja Terrestre J2000 / ICRF) es mutua i fisicament consistent entre
	// posicions (spkezr_c), orbites 3D (getOrbitStates) i matrius de rotacio (pxform_c IAU_* -> J2000).
	// Com que el pla XY del mon correspon a l'equador celeste J2000, el pla de l'ecliptica apareix
	// inclinat ~23,44 graus respecte al pla horitzontal XY (mentre l'eix de rotacio terrestre manté
	// la seva oblicuitat fisica respecte al pla orbital). Per a una futura opcio d'ecliptica horitzontal,
	// n'hi hauria prou amb commutar aquest marc a "ECLIPJ2000" de forma unificada a totes les crides SPICE.
	static constexpr const char* MARC_REFERENCIA_SPICE = "J2000";

	EscenariRealista();
	virtual ~EscenariRealista() override;

	void creaEscenari(GLuint& shader_programID) override;
	void updateVariables(double dt, EstatSimulacio& estat) override;
	void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) override;
	void alliberaEscenari() override;

	double getEphemerisTimeET() const { return m_ephemerisTimeET; }
	bool areKernelsCarregats() const { return m_kernelsCarregats; }
	glm::vec3 getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const override;
	glm::vec3 getColorCos(int idx) const override;

private:
	void recalcularOrbitaCos(int idx, double et);
	void recalcularOrbites3D(double et);
	void alliberaOrbitesLineStrip();
	void actualitzaCossosSPICE(double et);

	double m_ephemerisTimeET;
	bool m_kernelsCarregats;
	bool m_primerFrame;
	glm::dvec3 m_posicionsAU[NUM_COSSOS_REALISTA];
	double m_etOrbita[NUM_COSSOS_REALISTA];

	OrbitaLineVAO m_orbitesVAO[NUM_COSSOS_REALISTA];
};
