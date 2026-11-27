//******** PRACTICA VISUALITZACIO GRAFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn basic VS2026 MONOFINESTRA amb OpenGL 4.6, interficie GLFW 3.4, ImGui i llibreries GLM
//******** Enric Marti (Setembre 2026)
// objLoader.h: Interface of the class COBJModel.
//
//	  Versio 3.0:	- Carregador rapid d'una passada, cache binaria .mbin, VBO GL_FLOAT + EBO
//					  i suport de precarrega en segon pla (LoadToCPU / UploadFromCPU).
//////////////////////////////////////////////////////////////////////////////////////

#ifndef OBJLOADER_H
#define OBJLOADER_H

#define OBJLOADER_CLASS_DECL     __declspec(dllexport)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <vector>

// Maxima mida vector VAOList
#define MAX_SIZE_VAOLIST 125

// Needed structures
struct Vector3D
{
	float fX;
	float fY;
	float fZ;
};

struct Vector2D
{
	float fX;
	float fY;
};

struct OBJFileInfo
{
	unsigned int iVertexCount;
	unsigned int iTexCoordCount;
	unsigned int iNormalCount;
	unsigned int iFaceCount;
	unsigned int iMaterialCount;
};

struct Face
{
	unsigned int	iNumVertices;
	unsigned int	iMaterialIndex;
	Vector3D		*pVertices;
	Vector3D		*pNormals;
	Vector2D		*pTexCoords;
};

struct Material
{
	char	szName[1024];		 // Name of the material
	char	szTexture[_MAX_PATH];// Filename of the texture
	uint	iTextureID;			 // OpenGL name of the texture
	float fDiffuse[3];		 // Diffuse component
	float fAmbient[3];		 // Ambient component
	float fSpecular[3];		 // Specular component
	float fEmmissive[3];		 // Emmissive component
	float fShininess;			 // Specular exponent
};

// Compact interleaved vertex format (32 bytes/vertex)
struct VertexOBJ
{
	float px, py, pz;   // location 0: in_Vertex
	float nx, ny, nz;   // location 2: in_Normal
	float u, v;         // location 3: in_TexCoord
};

struct OBJSubMeshCPU
{
	int materialIndex;
	std::vector<VertexOBJ> vertices;
	std::vector<unsigned int> indices;
};

struct OBJTextureCPU
{
	unsigned char* pixels;
	int width;
	int height;
	int channels;
	OBJTextureCPU() : pixels(NULL), width(0), height(0), channels(0) {}
};

struct OBJPreloadData
{
	std::vector<Material> materials;
	std::vector<OBJSubMeshCPU> submeshes;
	std::vector<OBJTextureCPU> textures;
	bool valid;
	OBJPreloadData() : valid(false) {}
};

class OBJLOADER_CLASS_DECL COBJModel
{
  public:
	void _stdcall DrawModel(int prim_Id);
	int _stdcall LoadModel(char* szFileName);
	_stdcall COBJModel();
	virtual _stdcall ~COBJModel();
	void _stdcall EliminaLlista(int prim_Id);

	// Precarrega desacoplada CPU -> GPU (thread-safe per LoadToCPU)
	static bool _stdcall LoadToCPU(const char* szFileName, OBJPreloadData& outData);
	int _stdcall UploadFromCPU(OBJPreloadData& data);
	static void _stdcall FreePreloadData(OBJPreloadData& data);

	// Funcions CVAO
	void _stdcall netejaVAOList_OBJ();
	void _stdcall netejaTextures_OBJ();
	void _stdcall draw_TriVAO_OBJ(GLuint sh_programID);

  private:
	void _stdcall UseMaterial_ShaderID(GLuint sh_programID, const Material& pMaterial);
	void _stdcall updateUniformCache(GLuint sh_programID);

	// CVAO
	GLint numMaterials;
	GLint numSubmeshes;
	int vector_Materials[MAX_SIZE_VAOLIST];
	CVAO VAOList_OBJ[MAX_SIZE_VAOLIST];
	Material vMaterials[MAX_SIZE_VAOLIST];

	// Cache de localitzacions d'uniforms
	GLuint cachedProgramID;
	GLint loc_texture0;
	GLint loc_textur;
	GLint loc_matAmbient;
	GLint loc_matDiffuse;
	GLint loc_matSpecular;
	GLint loc_matShininess;

	// Funcions CVAO
	void _stdcall initVAOList_OBJ();
	void _stdcall Set_VAOList_OBJ(GLint k, CVAO auxVAO);
	void _stdcall deleteVAOList_OBJ(GLint k);
	void _stdcall draw_TriVAO_Object_OBJ(GLint k);
};

// Returns a newly created COBJModel object.
OBJLOADER_CLASS_DECL COBJModel* _stdcall InitObject();

// Destroys a COBJModel object
OBJLOADER_CLASS_DECL void _stdcall UnInitObject(COBJModel *obj);

#endif // OBJLOADER_H
