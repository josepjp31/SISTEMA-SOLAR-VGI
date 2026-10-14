#include "escenari.h"
#include "main.cpp"

void Escenari::creaEscenari()
{

}

void Escenari::dibuixaEscenari()
{
	m_objectList[0].m_mMatrix;
	//Le hago cosas al model matrix con los (ahora is Identidad)

	glUniformMatrix4fv(glGetUniformLocation(shader_programID, "modelMatrix"), 1, GL_FALSE, &m_objectList[0].m_mMatrix[0][0]);

	m_objectList[0].m_model->draw_TriVAO_OBJ(shader_programID);
}

void Escenari::updateVariables()
{

}