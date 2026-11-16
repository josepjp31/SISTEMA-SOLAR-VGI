// EscenariIntuitiu.h — Arquitectura: Escenari Intuïtiu
// Implementa el mode didàctic amb òrbites circulars coplanars (z = 0) i mides/distàncies ajustades visualment sense SPICE.

#pragma once

#include "escenari.h"

// Escenari del Mode Didàctic / Intuïtiu: òrbites circulars coplanars (z = 0)
// i mides/distàncies ajustades visualment sense dependència d'efemèrides SPICE.
class EscenariIntuitiu : public Escenari
{
public:
	// Inicialitza els descriptors VAO de les òrbites a zero i configura els paràmetres de càmera didàctics.
	EscenariIntuitiu();
	// Allibera les primitives geomètriques, les òrbites toroïdals i les textures associades.
	virtual ~EscenariIntuitiu() override;

	// Precondició: context OpenGL actiu i `shader_programID` enllaçat.
	// Carrega textures, construeix l'esfera/disc/torus base i inicialitza els 9 cossos i les 8 òrbites.
	void creaEscenari(GLuint& shader_programID) override;

	// Paràmetres: `dt` [s] delta temporal del frame (limitat a [0.0, 0.1] s), `estat` estat del simulador.
	// Integra l'angle orbital i de rotació axial dels cossos si la simulació no està en pausa.
	void updateVariables(double dt, EstatSimulacio& estat) override;

	// Paràmetres: `sh_programID`, `MatriuVista`, `MatriuTG`, `sw_mat[5]`, `estat`.
	// Dibuixa les òrbites toroïdals, les 9 esferes texturades i l'anell translúcid de Saturn.
	void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) override;

	// Allibera els VAOs/EBOs de primitives i òrbites, i delega la neteja de textures a Escenari.
	void alliberaEscenari() override;

private:
	// Elimina els VAOs/VBOs/EBOs toroïdals emmagatzemats a `m_orbitVAO`.
	void alliberaOrbitesTorus();

	static constexpr int NUM_COSSOS_INTUITIU = 9;    // Nombre de cossos en mode didàctic: Sol + 8 planetes
	static constexpr int NUM_ORBITES_INTUITIU = 8;   // Nombre d'òrbites circulars (Mercuri .. Neptú)

	CVAO m_orbitVAO[NUM_ORBITES_INTUITIU];           // VAOs toroïdals de radi unitari per a cadascuna de les 8 òrbites
};
