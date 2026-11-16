//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Enric Martí (Setembre 2026)
// shader.h — Arquitectura: Pipeline Render / Shaders
// Gestió del cicle de vida de programes GLSL (càrrega des de fitxer, compilació, enllaç i pas d'uniforms).

#ifndef SHADER_H
#define SHADER_H

// Entorn VGI: Constants i tipus de l'aplicació entorn VGI
#include "framework.h"

// Encapsula un programa shader OpenGL (Vertex/Fragment i opcionalment Geometry Shader)
class Shader
{

public:
//-------------- Entorn VGI: Variables globals de la classe Shader
	unsigned int programID; // Identificador OpenGL del programa GLSL (-1/UINT_MAX abans de carregar)
//-------------- Entorn VGI: Fi De Variables globals de la classe Shader 

//-------------- Entorn VGI: Funcions (mètodes) de de la classe Shader 
	// Inicialitza `programID` a -1 sense carregar cap shader.
	Shader();

	// Allibera el programa GLSL associat amb `glDeleteProgram`.
	~Shader();

	// Precondició: context OpenGL actiu; rutes vàlides a fitxers GLSL.
	// Compila i enllaça Vertex + Fragment shader. Retorn: `programID` del programa creat (0 si error).
	GLuint loadFileShaders(std::string file_Vert, std::string file_Frag);

	// Precondició: context OpenGL actiu; rutes vàlides a fitxers GLSL.
	// Compila i enllaça Vertex + Fragment + Geometry shader. Retorn: `programID` creat (0 si error).
	GLuint loadFileShadersG(std::string file_Vert, std::string file_Frag, std::string file_Geom);

	// Mostra per sortida estàndard/error un missatge d'error de compilació o lectura del fitxer `fileShader`.
	int DisplayMessageBox(std::string mess, std::string fileShader);

	// Llegeix tot el contingut del fitxer de text `fn` en un buffer dinàmic (responsabilitat del cridador alliberar-lo).
	char* textFileRead(const char* fn);

	// Allibera recursos de shaders enllaçats al programa actual.
	void releaseAllShaders();

	// Retorn: identificador OpenGL `programID` del programa shader actual.
	GLuint getProgramID();

	// Activa el programa shader actual mitjançant `glUseProgram(programID)`.
	void use();

	// Pas d'uniforms escalars al shader actiu (precondició: programa vinculat amb `use()`).
	void setBool(const std::string &name, bool value);
	void setInt(const std::string &name, int value);
	void setFloat(const std::string &name, float value);
	void setDouble(const std::string &name, double value);

	// Pas d'uniforms vectorials de 3 components (vec3 / ivec3 / bvec3 / dvec3).
	void setBool3(const std::string &name, bool value1, bool value2, bool value3);
	void setInt3(const std::string &name, int value1, int value2, int value3);
	void setFloat3(const std::string &name, float value1, float value2, float value3);
	void setDouble3(const std::string &name, double value1, double value2, double value3);

	// Pas d'uniforms vectorials de 4 components (vec4 / dvec4).
	void setFloat4(const std::string &name, float value1, float value2, float value3, float value4);
	void setDouble4(const std::string &name, double value1, double value2, double value3, double value4);
	
	// Pas d'uniforms matricials 3x3 (normals) i 4x4 (Model/View/Projection).
	void setMatrix3fv(const std::string &name, glm::mat3 trans);
	void setMatrix4fv(const std::string &name, glm::mat4 trans);

	// Elimina explícitament el programa shader amb `glDeleteProgram(programID)`.
	void DeleteProgram();

//-------------- Entorn VGI: Fi Funcions (mètodes) de de la classe Shader 
};
#endif