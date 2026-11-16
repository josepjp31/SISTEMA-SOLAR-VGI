//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2026 MONOFINESTRA amb OpenGL 4.6, interfície GLFW 3.4, ImGui i llibreries GLM
//******** Enric Martí (Setembre 2026)
// objLoader.h — Arquitectura: Pipeline Render / Carregador OBJ
// Carregador de models Wavefront (.obj / .mtl) amb un VAO per cada material definit al fitxer.
//
//	  Versió 2.0:	- Adaptació funcions a crear un VAO per a cada material del fitxer
//////////////////////////////////////////////////////////////////////////////////////

#ifndef OBJLOADER_H
#define OBJLOADER_H

#define OBJLOADER_CLASS_DECL     __declspec(dllexport)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Màxima mida vector VAOList
#define MAX_SIZE_VAOLIST 125

// Vector 3D en coma flotant (posicions o normals [unitats del model OBJ])
struct Vector3D
{
	float fX; // Component X
	float fY; // Component Y
	float fZ; // Component Z
};

// Vector 2D per a coordenades de textura UV [0.0..1.0]
struct Vector2D
{
	float fX; // Component U horitzontal
	float fY; // Component V vertical
};

// Comptadors d'elements llegits de la capçalera d'un fitxer .obj
struct OBJFileInfo
{
	unsigned int iVertexCount;   // Nombre de vèrtexs geomètrics (>= 0)
	unsigned int iTexCoordCount; // Nombre de coordenades UV (>= 0)
	unsigned int iNormalCount;   // Nombre de normals (>= 0)
	unsigned int iFaceCount;     // Nombre de cares triangulars/poligonals (>= 0)
	unsigned int iMaterialCount; // Nombre de materials MTL referenciats (>= 0)
};

// Definició d'una cara poligonal d'un model OBJ
struct Face
{
	unsigned int	iNumVertices;   // Nombre de vèrtexs de la cara (>= 3)
	unsigned int	iMaterialIndex; // Índex del material associat dins vMaterials [0..MAX_SIZE_VAOLIST-1]
	Vector3D		*pVertices;     // Array dinàmic de posicions dels vèrtexs de la cara
	Vector3D		*pNormals;      // Array dinàmic de normals per vèrtex
	Vector2D		*pTexCoords;    // Array dinàmic de coordenades de textura UV
};

// Propietats òptiques i textura d'un material Wavefront (.mtl)
struct Material
{
  char	szName[1024];		 // Nom identificador del material dins el .mtl
  char	szTexture[_MAX_PATH];// Ruta al fitxer de textura albedo (map_Kd)
  uint	iTextureID;			 // Identificador OpenGL de la textura carregada (0 = sense textura)
  float fDiffuse[3];		 // [RGB 0.0..1.0] Component difusa (Kd)
  float fAmbient[3];		 // [RGB 0.0..1.0] Component ambient (Ka)
  float fSpecular[3];		 // [RGB 0.0..1.0] Component especular (Ks)
  float fEmmissive[3];		 // [RGB 0.0..1.0] Component emissiva (Ke)
  float fShininess;			 // [0.0..128.0] Exponent especular (Ns)
};

// Carregador i renderitzador de malles Wavefront .obj agrupades per material en VAOs OpenGL
class OBJLOADER_CLASS_DECL COBJModel  
{
  public:
	// Dibuixa la primitiva identificada per `prim_Id` dins la llista de VAOs.
	void _stdcall DrawModel(int prim_Id);
	// Precondició: context OpenGL actiu i `szFileName != nullptr`.
	// Carrega el fitxer .obj, el seu .mtl associat i genera els VAOs per material. Retorn: 0 si èxit, codi `errno` si error.
	int _stdcall LoadModel(char* szFileName);
	// Inicialitza les llistes de VAOs i materials buides.
	_stdcall COBJModel();
	// Allibera tots els VAOs i textures carregades pel model.
	virtual _stdcall ~COBJModel();
	// Elimina el VAO associat a `prim_Id`.
	void _stdcall EliminaLlista(int prim_Id);

// Funcions CVAO
	// Elimina tots els VAOs/VBOs actius a `VAOList_OBJ`.
	void _stdcall netejaVAOList_OBJ();
	// Allibera totes les textures OpenGL referenciades a `vMaterials`.
	void _stdcall netejaTextures_OBJ();
	// Precondició: `sh_programID` actiu. Dibuixa totes les sub-malles del model aplicant el material i textura de cadascuna.
	void _stdcall draw_TriVAO_OBJ(GLuint sh_programID);

private:
	void _stdcall ReadNextString(char szString[], FILE *hStream);
	  int _stdcall LoadTexture2(const char szFileName[_MAX_PATH]);
	  void _stdcall UseMaterial(const Material *pMaterial);
	  void _stdcall UseMaterial_ShaderID(GLuint sh_programID, Material pMaterial);
	  void _stdcall GetTokenParameter(char szString[], const unsigned int iStrSize, FILE *hFile);
	  void _stdcall MakePath(char szFileAndPath[]);
	  bool _stdcall LoadMaterialLib(const char szFileName[], Material *pMaterials,
				unsigned int *iCurMaterialIndex, char szBasePath[]);
	CVAO _stdcall RenderToVAOList(const Face* pFaces, const unsigned int iFaceCount,
				const Material *pMaterials);
	void _stdcall loadToVAOList(const Face* pFaces, const unsigned int iFaceCount,
				const Material* pMaterials);
	void _stdcall GetFaceNormal(float fNormalOut[3], const Face *pFace);
	void _stdcall ParseFaceString(char szFaceString[], Face *FaceOut, const Vector3D *pVertices,
			  const Vector3D *pNormals, const Vector2D *pTexCoords, const unsigned int iMaterialIndex);
	void _stdcall GetFileInfo(FILE *hStream, OBJFileInfo *Stat, const char szConstBasePath[]);
 	void _stdcall GenTexCoords();

// CVAO
	GLint numMaterials = 0;                   // Nombre de materials carregats [0..MAX_SIZE_VAOLIST]
	int vector_Materials[MAX_SIZE_VAOLIST];   // Taula d'índexs de material per cada entrada VAO
	CVAO VAOList_OBJ[MAX_SIZE_VAOLIST];       // VAOs agrupats per material
	Material vMaterials[MAX_SIZE_VAOLIST];    // Taula de materials llegits del fitxer .mtl

// Funcions CVAO
	void _stdcall initVAOList_OBJ();
	void _stdcall Set_VAOList_OBJ(GLint k, CVAO auxVAO);
	void _stdcall deleteVAOList_OBJ(GLint k);
	void _stdcall draw_TriVAO_Object_OBJ(GLint k);
};

// Callback function for comparing two faces with qsort
static int CompareFaceByMaterial(const void *Arg1, const void *Arg2);

// Crea i retorna una nova instància de COBJModel.
OBJLOADER_CLASS_DECL COBJModel* _stdcall InitObject();

// Allibera una instància de COBJModel prèviament creada amb InitObject().
OBJLOADER_CLASS_DECL void _stdcall UnInitObject(COBJModel *obj);

#endif // !defined(AFX_OBJMODEL_H__32C5F722_AD3D_11D1_8F4D_E0B57CC10800__INCLUDED_)