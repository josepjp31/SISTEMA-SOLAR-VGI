#pragma once
#include "objLoader.h"

class Escenari
{
public:
	Escenari() { m_objectList = new Object[10]; }

	void creaEscenari();
	void dibuixaEscenari();
	void updateVariables();

private:

	//camera
	Object* m_objectList;

};


class Object
{
public:
	Object() { m_mMatrix = glm::mat4(1.0); }//Usar InitObject de COBJModel en el constructor
	~Object();//Usar UnInitObject de COBJModel en el destructor

	//Vector4D m_position;
	//Vector4D m_rotation;
	//Vector4D m_scale;
	glm::mat4 m_mMatrix;
	COBJModel m_model;
private:
};

//#~#~#~# Ponemos el OnPaint() en el while en el OnTimer()???