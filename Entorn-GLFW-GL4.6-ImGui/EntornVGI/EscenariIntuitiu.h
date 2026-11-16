#pragma once

#include "escenari.h"

// Escenari del Mode Didactic / Intuitiu: orbites circulars coplanars (z = 0)
// i mides/distancies ajustades visualment sense dependencia d'efemerides SPICE.
class EscenariIntuitiu : public Escenari
{
public:
	EscenariIntuitiu();
	virtual ~EscenariIntuitiu() override;

	void creaEscenari(GLuint& shader_programID) override;
	void updateVariables(double dt, EstatSimulacio& estat) override;
	void dibuixaEscenari(GLuint& sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], const EstatSimulacio& estat) override;
	void alliberaEscenari() override;

private:
	void alliberaOrbitesTorus();

	static constexpr int NUM_COSSOS_INTUITIU = 9;
	static constexpr int NUM_ORBITES_INTUITIU = 8;

	CVAO m_orbitVAO[NUM_ORBITES_INTUITIU];
};
