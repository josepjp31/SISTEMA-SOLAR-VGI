#pragma once

#include "framework.h"
#include "material.h"
#include "visualitzacio.h"
#include <vector>


#define SOL 0
#define MERCURIO 1
#define VENUS 2
#define TIERRA 3
#define MARTE 4
#define JUPITER 5
#define SATURNO 6
#define URANO 7
#define NEPTUNO 8
#define LUNA 9

// Dades individuals de cada cos celeste (Sol, planetes i Lluna) dins de l'escenari
class Object
{
public:
	Object() : m_mMatrix(glm::mat4(1.0f)), m_radiCos(0.0f), m_radiOrbita(0.0f), m_angleOrbita(0.0f), m_velAngular(0.0f) {}
	~Object() {}

	float m_radiCos;
	float m_radiOrbita;
	float m_angleOrbita;
	float m_velAngular;

	float m_angleRotacio;
	float m_velRotacio;

	GLuint m_textureID;
	GLuint m_normalMapID;

	glm::mat4 m_mMatrix;
};


// Gestió de l'escenari complet del Sistema Solar (cossos, òrbites, textures i actualització orbital)
class Escenari
{
public:
	Escenari()
	{
		// CVAO no s'inicialitza sol: posar-lo a 0 evita alliberar identificadors brossa
		m_orbitVAO = { 0, 0, 0, 0, 0 };
	}
	~Escenari() { alliberaEscenari(); }

	// No es permet copiar un Escenari (gestiona memoria dinamica i recursos OpenGL)
	Escenari(const Escenari&) = delete;
	Escenari& operator=(const Escenari&) = delete;

	// Inicialitza VAO, textures i cossos del sistema solar vinculant-los als shaders actius (cridat des de la UI en triar/canviar mode)
	void creaEscenari(GLuint& shader_programID, GLuint& shaderNormalMapID);

	// Renderitza els cossos celestes, l'anell de Saturn i les orbites (controlades pel flag mostrarOrbites de la UI)
	void dibuixaEscenari(GLuint sh_programID, GLuint shaderNormalMapID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5], bool mostrarOrbites = true);

	// Actualitza els angles orbitals i de rotacio segons els parametres temporals d'EstatSimulacio
	// (enPausa, direccioTemps [+1/-1], escalaReal i factorVelocitat del timelapse)
	void updateVariables(double dt = 0.016, bool enPausa = false, int direccioTemps = 1, bool escalaReal = false, double factorVelocitat = 60.0);
	// Allibera els objectes (i els seus VAO). Cal cridar-la ABANS de destruir el context OpenGL.
	void alliberaEscenari() {
		deleteVAOList(GLU_SPHERE);
		deleteVAOList(GLU_DISK);
		deleteVAOList(GLUT_TORUS);
		alliberaOrbita();
		for (int i = 0; i < m_objectList.size(); i++) {
			if (m_objectList[i].m_textureID) { glDeleteTextures(1, &m_objectList[i].m_textureID); m_objectList[i].m_textureID = 0; }
		}
		for (int i = 0; i < m_objectList.size(); i++) {
			if (m_objectList[i].m_normalMapID) { glDeleteTextures(1, &m_objectList[i].m_normalMapID); m_objectList[i].m_normalMapID = 0; }
		}
	}

	std::vector<Object> m_objectList;
	
	// Orbites: un unic cercle de radi 1 dibuixat com a linia (GL_LINE_LOOP). Cada orbita
	// (8 planetes + Lluna) el reutilitza escalant-lo al seu radi amb la modelMatrix.
	CVAO m_orbitVAO;

	// Mapes normals per al shader de normal mapping (index = cos; 0 Sol i 7 Urà sense mapa)

private:
	void creaOrbita();
	void alliberaOrbita() {
		if (m_orbitVAO.vaoId != 0) {
			glDeleteVertexArrays(1, &m_orbitVAO.vaoId);
			glDeleteBuffers(1, &m_orbitVAO.vboId);
			m_orbitVAO = { 0, 0, 0, 0, 0 };
		}
	}
};
