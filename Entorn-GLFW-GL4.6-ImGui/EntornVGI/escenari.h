#pragma once

// IMPORTANT: framework.h s'ha d'incloure ABANS d'objLoader.h, perque objLoader.h
// necessita els tipus d'OpenGL (GLuint, GLint, uint), CVAO (constants.h) i GLM.
#include "framework.h"
#include "objLoader.h"
#include "material.h"
#include "visualitzacio.h"
//#include "shader.h"
//#include "main.h"

// Nombre maxim d'objectes per escenari
#define MAX_OBJECTES_ESCENARI 10

// Object s'ha de declarar ABANS d'Escenari, perque Escenari en crea un vector (new Object[...]).
class Object
{
public:
	Object() : m_mMatrix(glm::mat4(1.0f)), m_modelCarregat(false) {}	//Usar InitObject de COBJModel en el constructor
	~Object() {}	//Usar UnInitObject de COBJModel en el destructor (COBJModel ja allibera els seus VAO)

	// Carrega un fitxer .OBJ al model. Retorna true si s'ha carregat correctament
	// (COBJModel::LoadModel retorna 0 si tot va be, o el codi errno si falla).
	bool carregaModel(char* fitxer) { m_modelCarregat = (m_model.LoadModel(fitxer) == 0); return m_modelCarregat; }
	bool teModel() const { return m_modelCarregat; }

	float m_radiCos;
	float m_radiOrbita;
	float m_angleOrbita;
	float m_velAngular;
	//Vector4D m_position;
	//Vector4D m_rotation;
	//Vector4D m_scale;
	glm::mat4 m_mMatrix;
	COBJModel m_model;
private:
	bool m_modelCarregat;
};


class Escenari
{
public:
	Escenari() { m_objectList = new Object[MAX_OBJECTES_ESCENARI]; }
	~Escenari() { alliberaEscenari(); }

	// No es permet copiar un Escenari (gestiona memoria dinamica i recursos OpenGL)
	Escenari(const Escenari&) = delete;
	Escenari& operator=(const Escenari&) = delete;

	void creaEscenari();
	void dibuixaEscenari(GLuint sh_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, char objecte,
		CColor col_object, bool sw_mat[5],
		bool textur, GLuint texturID[NUM_MAX_TEXTURES], bool textur_map, bool flagInvertY,
		int nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat pasCS, bool sw_PC, bool dib_TFrenet,
		COBJModel* objecteOBJ,
		//glm::mat4 MatriuVista, 
		glm::mat4 MatriuTG);
	void updateVariables();

	// Allibera els objectes (i els seus VAO). Cal cridar-la ABANS de destruir el context OpenGL.
	void alliberaEscenari() { delete[] m_objectList; m_objectList = nullptr; }

	Object* m_objectList;

private:

	//camera

};

//#~#~#~# Ponemos el OnPaint() en el while en el OnTimer()???