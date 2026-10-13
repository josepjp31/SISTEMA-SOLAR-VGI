#pragma once
#include "objLoader.h"
#include "escenari.cpp"

class Escenari
{
public:
	void creaEscenari();
	void dibuixaEscenari();
	void updateVariables();

private:
	Escenari() { m_objectList = new Object[10]; }

	//camera
	Object* m_objectList;

};


class Object
{
public:
	Object();//Usar InitObject de COBJModel en el constructor
	~Object();//Usar UnInitObject de COBJModel en el destructor

private:
	//Vector4D m_position;
	//Vector4D m_rotation;
	//Vector4D m_scale;
	COBJModel m_model;
};

//#~#~#~# Ponemos el OnPaint() en el while en el OnTimer()???