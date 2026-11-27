#pragma once

#include "framework.h"
#include "material.h"
#include "visualitzacio.h"
#include <vector>
#include "CelestialBodyData.h"

// Dades individuals de cada cos qualsevol dins de l'escenari
class Object
{
public:
	Object() {}
	Object(const std::string& nom, double scale) : m_nom(nom), m_scale(scale) {}
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;
	virtual ~Object() {
		unloadTexture();
		unloadNormMap();
		for (Object* o : m_sons)
			delete o;
		m_sons.clear();
	}

	//REALMENTE lo correcto seria que las texturas se guardasen en las herencias determinadas, ya que hay objetos que no tienen. pero como es codigo exactamente igual lo dejamos aqui de momento
	void unloadTexture() { if (m_textureID != 0) { glDeleteTextures(1, &m_textureID); m_textureID = 0; } }
	void unloadNormMap() { if (m_normalMapID != 0) { glDeleteTextures(1, &m_normalMapID); m_normalMapID = 0; } }
	GLuint loadTexture(const char* filepath) {
		unloadTexture();
		if (filepath == nullptr) return 0;
		m_textureID = loadIMA_SOIL(filepath);
		if (m_textureID == 0)
			fprintf(stderr, "No s'ha pogut carregar: %s\n", filepath);
		else {
			glBindTexture(GL_TEXTURE_2D, m_textureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.0f);
		}
		return m_textureID;
	}
	GLuint loadNormMap(const char* filepath) {
		unloadNormMap();
		if (filepath == nullptr) return 0;
		m_normalMapID = loadIMA_SOIL(filepath);
		if (m_normalMapID == 0)
			fprintf(stderr, "No s'ha pogut carregar: %s\n", filepath);
		else {
			glBindTexture(GL_TEXTURE_2D, m_normalMapID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.0f);
		}
		return m_normalMapID;
	}
	bool addSon(Object* s) {
		if (s != nullptr && s->m_parent == nullptr && s != this) {
			s->m_parent = this;
			m_sons.push_back(s);
			return true;
		}
		return false;
	}

	virtual void drawObject(GLuint& shaderActual, glm::mat4& matriuTG) = 0;
	virtual void update(double T) { for (Object* son : m_sons) if (son) son->update(T); }

	GLuint m_textureID = 0;
	GLuint m_normalMapID = 0;

	glm::mat4 m_trMatrix = glm::mat4(1.0f);
	double m_scale = 1.0f; //Scale / radi del objecte, s'utilitza per la camara, hauria de canviar el nom pero bue. Més tard ja veig

	std::string m_nom = "";

	std::vector<Object*> m_sons;
	Object* m_parent = nullptr;

	bool m_sw_mat[5] = { false, true, true, true, true };
};

class EsferaInt : public Object
{
public:
	double& m_radiCos = m_scale;
	double m_radiOrbita = 0.0f;

	double m_angleOrbitaInicial = 0.0f;
	double m_angleOrbita = 0.0f;
	double m_velAngular = 0.0f;

	double m_angleRotacioInicial = 0.0f;
	double m_angleRotacio = 0.0f;
	double m_velRotacio = 0.0f;

	double m_inclinacioEix = 0.0;
	double m_inclinacioOrbita = 0.0;

	EsferaInt() {}
	EsferaInt(const std::string& nom, double radiCos, double radiOrbita, double angleOrbita, double velAngular, double angleRotacio, double velRotacio, double inclinacioEix = 0.0, double inclinacioOrbita = 0.0) :
		Object(nom, radiCos),
		m_radiOrbita(radiOrbita),
		m_angleOrbita(angleOrbita),
		m_angleOrbitaInicial(angleOrbita),
		m_velAngular(velAngular),
		m_angleRotacio(angleRotacio),
		m_angleRotacioInicial(angleRotacio),
		m_velRotacio(velRotacio),
		m_inclinacioEix(inclinacioEix),
		m_inclinacioOrbita(inclinacioOrbita) {}

	~EsferaInt() {}

	void drawObject(GLuint& shaderActual, glm::mat4& matriuTG) override;
	void update(double T) override;

};

// Gestió de l'escenari complet del Sistema Solar (cossos, òrbites, textures i actualització orbital)
class Escenari
{
public:
	Escenari() {}
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
	void updateVariables(double et = 0, double dt = 0.016, bool enPausa = false, int direccioTemps = 1, bool escalaReal = false, double factorVelocitat = 60.0);
	// Allibera els objectes (i els seus VAO). Cal cridar-la ABANS de destruir el context OpenGL.
	void alliberaEscenari() {
		deleteVAOList(GLU_SPHERE);
		deleteVAOList(GLU_DISK);
		deleteVAOList(GLUT_TORUS);

		std::vector<Object*> roots;
		for (auto& p : m_objectList)
			if (p.second->m_parent == nullptr)
				roots.push_back(p.second);

		m_objectList.clear();

		for (Object* r : roots)
			delete r;
		roots.clear();
	}

	std::unordered_map<CelestialBodyId, Object*> m_objectList;
};
