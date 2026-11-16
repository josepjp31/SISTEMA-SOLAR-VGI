#include "escenari.h"

// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005).

Escenari::Escenari()
{
	m_colObj.r = 1.0; m_colObj.g = 1.0; m_colObj.b = 1.0; m_colObj.a = 1.0;
	for (int i = 0; i < NUM_MAX_TEXTURES; i++)
	{
		m_texturesID[i] = 0;
	}
}

Escenari::~Escenari()
{
	alliberaTextures();
	m_cossos.clear();
}

void Escenari::alliberaEscenari()
{
	alliberaTextures();
	m_cossos.clear();
}

GLuint Escenari::carregaTextura2D(const char* rutaFitxer)
{
	if (rutaFitxer == nullptr || rutaFitxer[0] == '\0') return 0;

	GLint texInt = loadIMA_SOIL(rutaFitxer);
	if (texInt <= 0)
	{
		fprintf(stderr, "No s'ha pogut carregar: %s\n", rutaFitxer);
		return 0;
	}

	GLuint texID = static_cast<GLuint>(texInt);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}

void Escenari::alliberaTextures()
{
	for (int i = 0; i < NUM_MAX_TEXTURES; i++)
	{
		if (m_texturesID[i] != 0)
		{
			glDeleteTextures(1, &m_texturesID[i]);
			m_texturesID[i] = 0;
		}
	}
}

glm::vec3 Escenari::getPosicioCos(int idx) const
{
	if (idx < 0 || idx >= static_cast<int>(m_cossos.size())) return glm::vec3(0.0f);
	return m_cossos[idx].m_posicio;
}

glm::vec3 Escenari::getPosicioVista(int idx, int cosEnfocatId, bool origenRelatiu) const
{
	(void)cosEnfocatId;
	(void)origenRelatiu;
	return getPosicioCos(idx);
}

glm::vec3 Escenari::getColorCos(int idx) const
{
	(void)idx;
	return glm::vec3(0.0f, 0.78f, 1.0f);
}

float Escenari::getRadiCos(int idx) const
{
	if (idx < 0 || idx >= static_cast<int>(m_cossos.size())) return 0.0f;
	return m_cossos[idx].m_radiCos;
}

std::string Escenari::getNomCos(int idx) const
{
	if (idx < 0 || idx >= static_cast<int>(m_cossos.size())) return "";
	return m_cossos[idx].m_nom;
}
