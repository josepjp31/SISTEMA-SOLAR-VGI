#pragma once

#include "escenari.h"
#include "CelestialBodyData.h"

// Metadades astronomiques reals de cada cos celeste per al Mode a Escala Real
struct CosRealMetadata
{
	CelestialBodyId spiceId;
	const char* nom;
	double radiEquatorialKm;
	double oblicuitatGraus;
	double semieixMajorUA;
	double inclinacioOrbitalGraus;
	double periodeOrbitalDies;
	double periodeRotacioHores;
	const char* rutaTextura;
	glm::vec3 colorOrbita;
};

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
// Sol, 8 planetes, Pluto i la Lluna amb factors d'escala en UA, metadades reals
// i estructura preparada per a la connexio completa amb efemerides SPICE (Fase 5).
class EscenariRealista : public Escenari
{
public:
	static constexpr double SCALE_UNITS_PER_AU = 500.0;
	static constexpr int NUM_COSSOS_REALISTA = 11; // Sol + 8 planetes + Pluto + Lluna

	EscenariRealista();
	virtual ~EscenariRealista() override;

	void creaEscenari(GLuint& shader_programID) override;
	void updateVariables(double dt, EstatSimulacio& estat) override;
	void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) override;
	void alliberaEscenari() override;

	double getEphemerisTimeET() const { return m_ephemerisTimeET; }
	bool areKernelsCarregats() const { return m_kernelsCarregats; }

private:
	void generaOrbitesLineStrip();
	void alliberaOrbitesLineStrip();
	void actualitzaPosicionsEsquelet(double et);

	double m_ephemerisTimeET;
	bool m_kernelsCarregats;

	std::vector<OrbitaLineVAO> m_orbitesVAO;
	static const CosRealMetadata kMetadadesReals[NUM_COSSOS_REALISTA];
};
