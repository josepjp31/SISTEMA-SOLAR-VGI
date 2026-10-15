#include "escenari.h"
// NOTA: no s'ha d'incloure "main.cpp" ni "main.h" aqui (definirien totes les variables
// globals dues vegades -> error de linker LNK2005). El shader es rep com a parametre.

void Escenari::creaEscenari()
{

}

void Escenari::dibuixaEscenari(GLuint sh_programID)
{
	if (m_objectList == nullptr) return;

	// Nomes es dibuixa si l'objecte te un model .OBJ carregat
	if (!m_objectList[0].teModel()) return;

	//Le hago cosas al model matrix con los (ahora is Identidad)

	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &m_objectList[0].m_mMatrix[0][0]);

	m_objectList[0].m_model.draw_TriVAO_OBJ(sh_programID);
}

void Escenari::updateVariables()
{

}